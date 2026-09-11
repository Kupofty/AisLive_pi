#include "ais_stream_client.h"

#include <cstdint>
#include <vector>

#include <wx/base64.h>
#include <wx/socket.h>

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

        sendSentence(nmea);
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
            return false;
        }
        acc.push_back(c);
        if (acc.size() >= 4 && acc.compare(acc.size() - 4, 4, "\r\n\r\n") == 0)
        {
            headersOut = acc;
            return true;
        }
    }
    return false;
}

} // namespace


struct AisStreamClient::Session
{
    wxSocketClient sock;
    SSL_CTX* sslCtx = nullptr;
    SSL* ssl = nullptr;

    ~Session()
    {
        if (ssl)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        if (sslCtx)
        {
            SSL_CTX_free(sslCtx);
        }
    }
};


AisStreamClient::AisStreamClient() = default;

AisStreamClient::~AisStreamClient()
{
    Stop();
}


/////////////////////////////
/// TLS / WebSocket layer ///
/////////////////////////////
bool AisStreamClient::TlsConnect(Session& s, const std::string& host, int port)
{
    wxIPV4address addr;
    addr.Hostname(host);
    addr.Service(port);

    // Blocking flags: safe for use off the GUI thread, and lets us reuse the
    // "close the socket from another thread to unblock" shutdown strategy.
    s.sock.SetFlags(wxSOCKET_BLOCK | wxSOCKET_WAITALL);
    if (!s.sock.Connect(addr, true /*wait*/))
    {
        return false;
    }

    s.sslCtx = SSL_CTX_new(TLS_client_method());
    if (!s.sslCtx)
    {
        return false;
    }
    SSL_CTX_set_default_verify_paths(s.sslCtx);
    SSL_CTX_set_verify(s.sslCtx, SSL_VERIFY_PEER, nullptr);

    s.ssl = SSL_new(s.sslCtx);
    if (!s.ssl)
    {
        return false;
    }

    // SNI, required by many TLS-terminating hosts.
    SSL_set_tlsext_host_name(s.ssl, host.c_str());
    // Hostname verification against the presented certificate.
    SSL_set1_host(s.ssl, host.c_str());

    if (SSL_set_fd(s.ssl, static_cast<int>(s.sock.GetSocket())) != 1)
    {
        return false;
    }

    return SSL_connect(s.ssl) == 1;
}

bool AisStreamClient::WsHandshake(Session& s, const std::string& host, const std::string& target)
{
    unsigned char keyBytes[16];
    if (RAND_bytes(keyBytes, sizeof(keyBytes)) != 1)
    {
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

    if (!SslWriteAll(s.ssl, req.data(), req.size()))
    {
        return false;
    }

    std::string headers;
    if (!SslReadHttpHeaders(s.ssl, headers))
    {
        return false;
    }

    if (headers.find("HTTP/1.1 101") == std::string::npos)
    {
        return false;
    }

    // Validate Sec-WebSocket-Accept = base64(SHA1(key + GUID)).
    const std::string acceptSrc = wsKey.ToStdString() + kWebSocketGuid;
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(acceptSrc.data()), acceptSrc.size(), digest);
    const wxString expectedAccept = wxBase64Encode(digest, sizeof(digest));

    return headers.find(expectedAccept.ToStdString()) != std::string::npos;
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
    return true;
}


////////////////////////
/// Public interface  ///
////////////////////////
void AisStreamClient::Start(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence)
{
    if (m_streaming.load())
    {
        return; // already running
    }

    // The previous worker may have exited on its own (dropped connection)
    // without anyone joining it; assigning to a still-joinable std::thread
    // calls std::terminate.
    if (m_thread.joinable())
    {
        m_thread.join();
    }

    m_onSentence = std::move(onSentence);
    m_streaming = true;
    m_thread = std::thread(&AisStreamClient::ThreadFunc, this, latitude, longitude, boxSizeDegrees);
}

void AisStreamClient::Stop()
{
    // No early-out on m_streaming: the worker clears that flag itself when
    // the connection drops, but the thread still needs joining.
    m_streaming = false;

    // The worker thread is blocked in a synchronous SSL_read(). Closing the
    // underlying socket from this thread is the standard way to unblock a
    // synchronous read happening on another thread - the read call returns
    // with an error and the loop exits cleanly.
    if (m_session)
    {
        m_session->sock.Close();
    }

    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void AisStreamClient::Restart(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence)
{
    if (!m_streaming.load())
    {
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
    try
    {
        m_session = std::make_unique<Session>();
        auto& session = *m_session;

        if (!TlsConnect(session, kAisHost, kAisPort))
        {
            throw std::runtime_error("TLS connect failed");
        }

        if (!WsHandshake(session, kAisHost, kAisTarget))
        {
            throw std::runtime_error("WebSocket handshake failed");
        }

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

        if (!WsSendFrame(session, WsOpcode::Text, sub_str))
        {
            throw std::runtime_error("Failed to send subscribe message");
        }

        while (m_streaming.load())
        {
            std::string payload;
            WsOpcode opcode;
            if (!WsReadFrame(session, payload, opcode))
            {
                // Either Stop() closed the socket, or the connection
                // dropped. Either way, stop reading.
                break;
            }

            if (opcode == WsOpcode::Close)
            {
                break;
            }

            if (opcode == WsOpcode::Ping)
            {
                // Keep the connection alive.
                WsSendFrame(session, WsOpcode::Pong, payload);
                continue;
            }

            if (opcode != WsOpcode::Text)
            {
                continue; // ignore binary/pong/continuation frames
            }

            Json::Value ev;
            Json::Reader reader;
            if (reader.parse(payload, ev))
            {
                if (m_onSentence)
                {
                    ProcessAisEvent(ev, m_onSentence);
                }
            }
            // else: ignore malformed frames.
        }
    }
    catch (const std::exception&)
    {
        // Ignore
    }

    m_streaming = false;
    m_session.reset();
}