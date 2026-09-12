#include "ais_stream_client.h"

#include <cstdint>
#include <cstring>
#include <mutex>
#include <sstream>
#include <vector>

#include <wx/base64.h>
#include <wx/log.h>

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>

#include <json/json.h>


namespace {

constexpr char kAisHost[]   = "ais.openwaters.io";
constexpr int  kAisPort     = 443;
constexpr char kAisTarget[] = "/v1/stream";

// RFC 6455 magic GUID used when validating the Sec-WebSocket-Accept header.
constexpr char kWebSocketGuid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

#ifdef _WIN32
// Winsock requires an explicit WSAStartup() before any socket call and a
// matching WSACleanup(). wxSocketClient used to do this for us implicitly;
// now that we talk to Winsock directly we own this ourselves. Guarded so it
// only runs once per process regardless of how many AisStreamClient
// instances/threads are created.
void EnsureWinsockInitialized()
{
    static std::once_flag flag;
    std::call_once(flag, []()
                   {
                       WSADATA wsaData;
                       const int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
                       wxLogMessage("AisStreamClient: WSAStartup rc=%d", rc);
                       // Deliberately never call WSACleanup(): other plugins/OpenCPN core
                       // may also be using Winsock in the same process, and there is no
                       // reliable single point at which we know we're the last user.
                       // Leaving it initialized for the lifetime of the process is the
                       // standard, safe approach for a plugin.
                   });
}

void CloseSocket(socket_t s)
{
    shutdown(s, SD_BOTH);
    closesocket(s);
}
#else
void CloseSocket(socket_t s)
{
    shutdown(s, SHUT_RDWR);
    close(s);
}
#endif

void ProcessAisEvent(const Json::Value& ev, const std::function<void(const wxString&)>& sendSentence)
{
    if (!ev.isMember("type") || ev["type"].asString() != "event")
    {
        return; // welcome/control message, not an AIS report
    }

    if (!ev.isMember("nmea") || !ev["nmea"].isArray())
    {
        return;
    }

    for (const auto& sentence : ev["nmea"])
    {
        if (!sentence.isString())
        {
            continue;
        }

        wxString nmea = wxString::FromUTF8(sentence.asString().c_str());
        if (!nmea.EndsWith("\r\n"))
        {
            nmea += "\r\n";
        }

        wxLogMessage("AisStreamClient: dispatching sentence to callback: %s", nmea.Trim());
        sendSentence(nmea);
        wxLogMessage("AisStreamClient: callback returned");
    }
}

// --- Small blocking SSL helpers -------------------------------------------
// These treat SSL_read()/SSL_write() as fully blocking calls. Stop()
// unblocks the worker thread by closing the underlying socket, which makes
// the in-flight SSL_read() fail and the loop exit.

bool SslWriteAll(SSL* ssl, const char* data, size_t len)
{
    size_t sent = 0;
    while (sent < len)
    {
        const int n = SSL_write(ssl, data + sent, static_cast<int>(len - sent));
        if (n <= 0)
        {
            wxLogMessage("AisStreamClient: SSL_write failed, n=%d, SSL_get_error=%d",
                         n, SSL_get_error(ssl, n));
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

bool SslReadExact(SSL* ssl, char* buf, size_t len)
{
    size_t got = 0;
    while (got < len)
    {
        const int n = SSL_read(ssl, buf + got, static_cast<int>(len - got));
        if (n <= 0)
        {
            wxLogMessage("AisStreamClient: SSL_read failed, n=%d, SSL_get_error=%d",
                         n, SSL_get_error(ssl, n));
            return false; // connection closed / error / unblocked by Stop()
        }
        got += static_cast<size_t>(n);
    }
    return true;
}

// Reads bytes one at a time until the terminating blank line of an HTTP
// response is seen. Simple and slow, but the handshake response is tiny and
// this only runs once per connection.
bool SslReadHttpHeaders(SSL* ssl, std::string& headersOut)
{
    std::string acc;
    while (acc.size() < 8192)
    {
        char c;
        const int n = SSL_read(ssl, &c, 1);
        if (n <= 0)
        {
            wxLogMessage("AisStreamClient: SslReadHttpHeaders SSL_read failed, n=%d, SSL_get_error=%d",
                         n, SSL_get_error(ssl, n));
            return false;
        }
        acc.push_back(c);
        if (acc.size() >= 4 && acc.compare(acc.size() - 4, 4, "\r\n\r\n") == 0)
        {
            headersOut = acc;
            return true;
        }
    }
    wxLogMessage("AisStreamClient: SslReadHttpHeaders exceeded 8192 bytes without terminator");
    return false;
}

} // namespace


struct AisStreamClient::Session
{
    socket_t rawSocket = kInvalidSocket;
    SSL_CTX* sslCtx = nullptr;
    SSL* ssl = nullptr;

    ~Session()
    {
        wxLogMessage("AisStreamClient: Session::~Session begin");
        if (ssl)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        if (sslCtx)
        {
            SSL_CTX_free(sslCtx);
        }
        if (rawSocket != kInvalidSocket)
        {
            CloseSocket(rawSocket);
        }
        wxLogMessage("AisStreamClient: Session::~Session end");
    }
};


AisStreamClient::AisStreamClient() = default;

AisStreamClient::~AisStreamClient()
{
    wxLogMessage("AisStreamClient: ~AisStreamClient calling Stop()");
    Stop();
}


/////////////////////////////
/// TLS / WebSocket layer ///
/////////////////////////////
bool AisStreamClient::TlsConnect(Session& s, const std::string& host, int port)
{
    wxLogMessage("AisStreamClient: TlsConnect begin, host=%s port=%d", host.c_str(), port);

#ifdef _WIN32
    EnsureWinsockInitialized();
#endif

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* addrResult = nullptr;
    const std::string portStr = std::to_string(port);

    wxLogMessage("AisStreamClient: calling getaddrinfo");
    const int gaiRc = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &addrResult);
    wxLogMessage("AisStreamClient: getaddrinfo returned %d", gaiRc);
    if (gaiRc != 0)
    {
        return false;
    }

    for (addrinfo* p = addrResult; p != nullptr; p = p->ai_next)
    {
        wxLogMessage("AisStreamClient: creating socket");
        s.rawSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s.rawSocket == kInvalidSocket)
        {
            wxLogMessage("AisStreamClient: socket() failed, trying next addrinfo entry");
            continue;
        }
        wxLogMessage("AisStreamClient: calling connect");
        if (connect(s.rawSocket, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0)
        {
            wxLogMessage("AisStreamClient: connect succeeded");
            break; // connected
        }
        wxLogMessage("AisStreamClient: connect failed, trying next addrinfo entry");
        CloseSocket(s.rawSocket);
        s.rawSocket = kInvalidSocket;
    }
    freeaddrinfo(addrResult);

    if (s.rawSocket == kInvalidSocket)
    {
        wxLogMessage("AisStreamClient: no addrinfo entry connected successfully");
        return false;
    }

    wxLogMessage("AisStreamClient: calling SSL_CTX_new");
    s.sslCtx = SSL_CTX_new(TLS_client_method());
    if (!s.sslCtx)
    {
        wxLogMessage("AisStreamClient: SSL_CTX_new returned null");
        return false;
    }
    wxLogMessage("AisStreamClient: SSL_CTX_new ok, calling SSL_CTX_set_default_verify_paths");
    SSL_CTX_set_default_verify_paths(s.sslCtx);
    SSL_CTX_set_verify(s.sslCtx, SSL_VERIFY_PEER, nullptr);

    wxLogMessage("AisStreamClient: calling SSL_new");
    s.ssl = SSL_new(s.sslCtx);
    if (!s.ssl)
    {
        wxLogMessage("AisStreamClient: SSL_new returned null");
        return false;
    }

    // SNI, required by many TLS-terminating hosts.
    wxLogMessage("AisStreamClient: calling SSL_set_tlsext_host_name");
    SSL_set_tlsext_host_name(s.ssl, host.c_str());
    // Hostname verification against the presented certificate.
    wxLogMessage("AisStreamClient: calling SSL_set1_host");
    SSL_set1_host(s.ssl, host.c_str());

    wxLogMessage("AisStreamClient: calling SSL_set_fd");
    if (SSL_set_fd(s.ssl, static_cast<int>(s.rawSocket)) != 1)
    {
        wxLogMessage("AisStreamClient: SSL_set_fd failed");
        return false;
    }

    wxLogMessage("AisStreamClient: calling SSL_connect");
    const int connectRc = SSL_connect(s.ssl);
    wxLogMessage("AisStreamClient: SSL_connect returned %d (SSL_get_error=%d)",
                 connectRc, SSL_get_error(s.ssl, connectRc));

    return connectRc == 1;
}

bool AisStreamClient::WsHandshake(Session& s, const std::string& host, const std::string& target)
{
    wxLogMessage("AisStreamClient: WsHandshake begin");

    unsigned char keyBytes[16];
    if (RAND_bytes(keyBytes, sizeof(keyBytes)) != 1)
    {
        wxLogMessage("AisStreamClient: RAND_bytes failed");
        return false;
    }
    const wxString wsKey = wxBase64Encode(keyBytes, sizeof(keyBytes));

    const std::string req =
        "GET " + target + " HTTP/1.1\r\n"
                          "Host: " + host + "\r\n"
                 "Upgrade: websocket\r\n"
                 "Connection: Upgrade\r\n"
                 "Sec-WebSocket-Key: " + wsKey.ToStdString() + "\r\n"
                                "Sec-WebSocket-Version: 13\r\n"
                                "\r\n";

    wxLogMessage("AisStreamClient: sending WS handshake request");
    if (!SslWriteAll(s.ssl, req.data(), req.size()))
    {
        wxLogMessage("AisStreamClient: WsHandshake write failed");
        return false;
    }

    wxLogMessage("AisStreamClient: reading WS handshake response headers");
    std::string headers;
    if (!SslReadHttpHeaders(s.ssl, headers))
    {
        wxLogMessage("AisStreamClient: WsHandshake header read failed");
        return false;
    }
    wxLogMessage("AisStreamClient: received headers (%zu bytes)", headers.size());

    if (headers.find("HTTP/1.1 101") == std::string::npos)
    {
        wxLogMessage("AisStreamClient: response missing 'HTTP/1.1 101' - handshake rejected");
        return false;
    }

    // Validate Sec-WebSocket-Accept = base64(SHA1(key + GUID)).
    const std::string acceptSrc = wsKey.ToStdString() + kWebSocketGuid;
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(acceptSrc.data()), acceptSrc.size(), digest);
    const wxString expectedAccept = wxBase64Encode(digest, sizeof(digest));

    const bool ok = headers.find(expectedAccept.ToStdString()) != std::string::npos;
    wxLogMessage("AisStreamClient: WsHandshake Sec-WebSocket-Accept validation: %s", ok ? "OK" : "FAILED");
    return ok;
}

bool AisStreamClient::WsSendFrame(Session& s, WsOpcode opcode, const std::string& payload)
{
    std::vector<unsigned char> frame;
    frame.push_back(0x80 | static_cast<unsigned char>(opcode)); // FIN + opcode

    constexpr unsigned char kMaskBit = 0x80; // client->server frames must be masked
    const size_t len = payload.size();
    if (len <= 125)
    {
        frame.push_back(static_cast<unsigned char>(len) | kMaskBit);
    }
    else if (len <= 0xFFFF)
    {
        frame.push_back(126 | kMaskBit);
        frame.push_back(static_cast<unsigned char>((len >> 8) & 0xFF));
        frame.push_back(static_cast<unsigned char>(len & 0xFF));
    }
    else
    {
        frame.push_back(127 | kMaskBit);
        for (int i = 7; i >= 0; --i)
        {
            frame.push_back(static_cast<unsigned char>((len >> (i * 8)) & 0xFF));
        }
    }

    unsigned char mask[4];
    RAND_bytes(mask, sizeof(mask));
    frame.insert(frame.end(), mask, mask + 4);

    const size_t headerSize = frame.size();
    frame.resize(headerSize + len);
    for (size_t i = 0; i < len; ++i)
    {
        frame[headerSize + i] = static_cast<unsigned char>(payload[i]) ^ mask[i % 4];
    }

    wxLogMessage("AisStreamClient: WsSendFrame opcode=%d payloadLen=%zu", static_cast<int>(opcode), len);
    return SslWriteAll(s.ssl, reinterpret_cast<const char*>(frame.data()), frame.size());
}

bool AisStreamClient::WsReadFrame(Session& s, std::string& payloadOut, WsOpcode& opcodeOut)
{
    unsigned char hdr[2];
    if (!SslReadExact(s.ssl, reinterpret_cast<char*>(hdr), sizeof(hdr)))
    {
        return false;
    }

    // Fragmented messages aren't reassembled here; the AIS feed sends
    // complete JSON events per frame in practice.
    opcodeOut = static_cast<WsOpcode>(hdr[0] & 0x0F);

    const bool masked = (hdr[1] & 0x80) != 0;
    uint64_t len = hdr[1] & 0x7F;

    if (len == 126)
    {
        unsigned char ext[2];
        if (!SslReadExact(s.ssl, reinterpret_cast<char*>(ext), sizeof(ext)))
        {
            return false;
        }
        len = (static_cast<uint64_t>(ext[0]) << 8) | ext[1];
    }
    else if (len == 127)
    {
        unsigned char ext[8];
        if (!SslReadExact(s.ssl, reinterpret_cast<char*>(ext), sizeof(ext)))
        {
            return false;
        }
        len = 0;
        for (unsigned char b : ext)
        {
            len = (len << 8) | b;
        }
    }

    wxLogMessage("AisStreamClient: WsReadFrame opcode=%d masked=%d len=%llu",
                 static_cast<int>(opcodeOut), masked ? 1 : 0,
                 static_cast<unsigned long long>(len));

    unsigned char maskKey[4] = {0, 0, 0, 0};
    if (masked)
    {
        if (!SslReadExact(s.ssl, reinterpret_cast<char*>(maskKey), sizeof(maskKey)))
        {
            return false;
        }
    }

    std::string payload;
    payload.resize(static_cast<size_t>(len));
    if (len > 0 && !SslReadExact(s.ssl, &payload[0], payload.size()))
    {
        return false;
    }

    if (masked)
    {
        for (size_t i = 0; i < payload.size(); ++i)
        {
            payload[i] = static_cast<char>(static_cast<unsigned char>(payload[i]) ^ maskKey[i % 4]);
        }
    }

    payloadOut = std::move(payload);
    wxLogMessage("AisStreamClient: WsReadFrame payload received (%zu bytes)", payloadOut.size());
    return true;
}


////////////////////////
/// Public interface  ///
////////////////////////
void AisStreamClient::Start(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence)
{
    wxLogMessage("AisStreamClient: Start() called, lat=%f lon=%f box=%f",
                 latitude, longitude, boxSizeDegrees);

    if (m_streaming.load())
    {
        wxLogMessage("AisStreamClient: Start() no-op, already streaming");
        return; // already running
    }

    // The previous worker may have exited on its own (dropped connection)
    // without anyone joining it; assigning to a still-joinable std::thread
    // calls std::terminate.
    if (m_thread.joinable())
    {
        wxLogMessage("AisStreamClient: Start() joining previous worker thread");
        m_thread.join();
        wxLogMessage("AisStreamClient: Start() previous worker thread joined");
    }

    m_onSentence = std::move(onSentence);
    m_streaming = true;
    wxLogMessage("AisStreamClient: Start() spawning worker thread");
    m_thread = std::thread(&AisStreamClient::ThreadFunc, this, latitude, longitude, boxSizeDegrees);
    wxLogMessage("AisStreamClient: Start() worker thread spawned, id=%s",
                 [&]{ std::ostringstream ss; ss << m_thread.get_id(); return ss.str(); }().c_str());
}

void AisStreamClient::Stop()
{
    wxLogMessage("AisStreamClient: Stop() called");

    // No early-out on m_streaming: the worker clears that flag itself when
    // the connection drops, but the thread still needs joining.
    m_streaming = false;

    // The worker thread is blocked in a synchronous SSL_read(). shutdown()
    // followed by closesocket()/close() from this thread is the standard,
    // thread-safe way to unblock a synchronous read happening on another
    // thread - the read call returns with an error and the loop exits
    // cleanly. Unlike wxSocketClient, a raw socket handle has no thread
    // affinity, so this is safe to call from any thread.
    {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        if (m_session && m_session->rawSocket != kInvalidSocket)
        {
            wxLogMessage("AisStreamClient: Stop() closing socket to unblock worker");
            CloseSocket(m_session->rawSocket);
        }
        else
        {
            wxLogMessage("AisStreamClient: Stop() no active session/socket to close");
        }
    }

    if (m_thread.joinable())
    {
        wxLogMessage("AisStreamClient: Stop() joining worker thread");
        m_thread.join();
        wxLogMessage("AisStreamClient: Stop() worker thread joined");
    }
}

void AisStreamClient::Restart(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence)
{
    wxLogMessage("AisStreamClient: Restart() called");
    if (!m_streaming.load())
    {
        wxLogMessage("AisStreamClient: Restart() no-op, not currently streaming");
        return;
    }

    Stop();
    Start(latitude, longitude, boxSizeDegrees, std::move(onSentence));
}

bool AisStreamClient::IsStreaming() const
{
    return m_streaming.load();
}

void AisStreamClient::ThreadFunc(double latitude, double longitude, double boxSizeDegrees)
{
    wxLogMessage("AisStreamClient: ThreadFunc begin");
    try
    {
        Session* sessionPtr = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_sessionMutex);
            if (!m_streaming.load())
            {
                wxLogMessage("AisStreamClient: ThreadFunc aborting, Stop() already ran");
                return; // Stop() already ran; don't open a socket nobody can close
            }
            wxLogMessage("AisStreamClient: ThreadFunc constructing Session");
            m_session = std::make_unique<Session>();
            sessionPtr = m_session.get();
        }
        auto& session = *sessionPtr;

        wxLogMessage("AisStreamClient: ThreadFunc calling TlsConnect");
        if (!TlsConnect(session, kAisHost, kAisPort))
        {
            throw std::runtime_error("TLS connect failed");
        }
        wxLogMessage("AisStreamClient: ThreadFunc TlsConnect succeeded");

        if (!m_streaming.load())
        {
            throw std::runtime_error("stopped during connect");
        }

        wxLogMessage("AisStreamClient: ThreadFunc calling WsHandshake");
        if (!WsHandshake(session, kAisHost, kAisTarget))
        {
            throw std::runtime_error("WebSocket handshake failed");
        }
        wxLogMessage("AisStreamClient: ThreadFunc WsHandshake succeeded");

        // Search area
        Json::Value box(Json::arrayValue);
        box.append(latitude  - boxSizeDegrees / 2.0);
        box.append(longitude - boxSizeDegrees / 2.0);
        box.append(latitude  + boxSizeDegrees / 2.0);
        box.append(longitude + boxSizeDegrees / 2.0);

        Json::Value bbox(Json::arrayValue);
        bbox.append(box);

        Json::Value sub_msg;
        sub_msg["type"] = "subscribe";
        sub_msg["bbox"] = bbox;

        Json::FastWriter writer;
        std::string sub_str = writer.write(sub_msg);
        // FastWriter appends a trailing newline; trim it since this is sent
        // as a single websocket text frame.
        if (!sub_str.empty() && sub_str.back() == '\n')
        {
            sub_str.pop_back();
        }

        wxLogMessage("AisStreamClient: ThreadFunc sending subscribe message: %s", sub_str.c_str());
        if (!WsSendFrame(session, WsOpcode::Text, sub_str))
        {
            throw std::runtime_error("Failed to send subscribe message");
        }
        wxLogMessage("AisStreamClient: ThreadFunc subscribe message sent, entering read loop");

        while (m_streaming.load())
        {
            std::string payload;
            WsOpcode opcode;
            if (!WsReadFrame(session, payload, opcode))
            {
                // Either Stop() closed the socket, or the connection
                // dropped. Either way, stop reading.
                wxLogMessage("AisStreamClient: ThreadFunc WsReadFrame failed, exiting read loop");
                break;
            }

            if (opcode == WsOpcode::Close)
            {
                wxLogMessage("AisStreamClient: ThreadFunc received Close frame, exiting read loop");
                break;
            }

            if (opcode == WsOpcode::Ping)
            {
                // Keep the connection alive.
                wxLogMessage("AisStreamClient: ThreadFunc received Ping, sending Pong");
                WsSendFrame(session, WsOpcode::Pong, payload);
                continue;
            }

            if (opcode != WsOpcode::Text)
            {
                wxLogMessage("AisStreamClient: ThreadFunc ignoring frame opcode=%d", static_cast<int>(opcode));
                continue; // ignore binary/pong/continuation frames
            }

            wxLogMessage("AisStreamClient: ThreadFunc parsing JSON payload (%zu bytes)", payload.size());
            Json::Value ev;
            Json::Reader reader;
            if (reader.parse(payload, ev))
            {
                wxLogMessage("AisStreamClient: ThreadFunc JSON parsed ok, calling ProcessAisEvent");
                if (m_onSentence)
                {
                    ProcessAisEvent(ev, m_onSentence);
                }
                wxLogMessage("AisStreamClient: ThreadFunc ProcessAisEvent returned");
            }
            else
            {
                wxLogMessage("AisStreamClient: ThreadFunc JSON parse failed, ignoring malformed frame");
            }
        }
    }
    catch (const std::exception& ex)
    {
        wxLogMessage("AisStreamClient: ThreadFunc caught exception: %s", ex.what());
    }

    wxLogMessage("AisStreamClient: ThreadFunc cleaning up session");
    m_streaming = false;

    // Tear down unlocked: ~Session's SSL_shutdown blocks, and Stop() must never wait on it.
    std::unique_ptr<Session> session_out;
    {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        session_out = std::move(m_session);
    }
    if (session_out && session_out->rawSocket != kInvalidSocket)
    {
        // Close first so SSL_shutdown fails fast instead of writing to a dead peer.
        wxLogMessage("AisStreamClient: ThreadFunc closing socket before Session teardown");
        CloseSocket(session_out->rawSocket);
    }
    wxLogMessage("AisStreamClient: ThreadFunc end (session_out destructor runs next)");
}