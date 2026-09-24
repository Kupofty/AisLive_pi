#include "ais_stream_client.h"

#include <algorithm>
#include <chrono>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessageType.h>

#include <json/json.h>


namespace {

constexpr char kAisUrl[] = "wss://ais.openwaters.io/v1/stream";

// Reconnect backoff: wait kInitialBackoffSeconds after the first failure,
// double on each consecutive failure up to the cap, reset once a session
// reaches Running. A server rejection (type:"error" frame) jumps straight
// to the cap so a locked-out client doesn't hammer the server.
constexpr int kInitialBackoffSeconds = 2;
constexpr int kMaxBackoffSeconds     = 30;

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

        // Only forward actual AIS VDM sentences.
        if (!nmea.StartsWith("!AIVDM"))
        {
            continue;
        }

        if (!nmea.EndsWith("\r\n"))
        {
            nmea += "\r\n";
        }

        //Publish sentence to OpenCPN
        sendSentence(nmea);
    }
}

} //end of namespace



/////////////
/// Class ///
/////////////
AisStreamClient::AisStreamClient()
{
    // Required once per process before any ix::WebSocket is used; safe to
    // call more than once across multiple client instances. On Windows this
    // does WSAStartup().
    ix::initNetSystem();
}

AisStreamClient::~AisStreamClient()
{
    Stop();
    ix::uninitNetSystem();
}



////////////////
/// Messages ///
////////////////
std::string AisStreamClient::BuildSubscribeMessage(double latitude, double longitude, double boxSizeDegrees) const
{
    // Search area centered on search position
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
    // FastWriter appends a trailing newline; trim it since this is sent as a
    // single websocket text frame.
    if (!sub_str.empty() && sub_str.back() == '\n')
    {
        sub_str.pop_back();
    }
    return sub_str;
}

bool AisStreamClient::HandleMessage(const std::string& payload)
{
    Json::Value ev;
    Json::Reader reader;
    if (reader.parse(payload, ev))
    {
        // e.g. {"type":"error","error":"concurrent streams per address
        // exceeded"}; the server closes the connection after sending it, so
        // surface the reason and let the caller pace the retry.
        if (ev.isMember("type") && ev["type"].asString() == "error")
        {
            const wxString reason = ev["error"].isString()
                                        ? wxString::FromUTF8(ev["error"].asString().c_str())
                                        : wxString("server error");
            SetState(State::Error, reason);
            return true;
        }

        if (m_onSentence)
        {
            ProcessAisEvent(ev, m_onSentence);
        }
    }
    // else: ignore malformed frames.
    return false;
}



///////////////
/// State   ///
///////////////
void AisStreamClient::SetStateCallback(StateCallback onStateChanged)
{
    std::lock_guard<std::mutex> lock(m_stateCallbackMutex);
    m_onStateChanged = std::move(onStateChanged);
}

void AisStreamClient::SetState(State state, const wxString& detail)
{
    // Copy the callback out under the lock rather than holding the lock
    // while invoking it, so a SetStateCallback() call from another thread
    // can't block on (or deadlock with) whatever the callback itself does.
    StateCallback cb;
    {
        std::lock_guard<std::mutex> lock(m_stateCallbackMutex);
        cb = m_onStateChanged;
    }

    if (cb)
    {
        cb(state, detail);
    }
}



/////////////////////////
/// Public interface  ///
/////////////////////////
void AisStreamClient::Start(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence)
{
    if (m_streaming.load())
    {
        return; // already running
    }

    // Stop() always joins, but assigning to a still-joinable std::thread
    // calls std::terminate; make sure any previous supervisor is reaped.
    if (m_thread.joinable())
    {
        m_thread.join();
    }

    m_onSentence = std::move(onSentence);
    m_streaming = true;
    m_thread = std::thread(&AisStreamClient::SupervisorFunc, this,
                           BuildSubscribeMessage(latitude, longitude, boxSizeDegrees));
}

void AisStreamClient::SupervisorFunc(std::string subscribeMsg)
{
    int backoffSeconds = kInitialBackoffSeconds;

    while (m_streaming.load())
    {
        SetState(State::Connecting);

        // Per-session flags. Only this thread touches them: the socket's
        // read loop runs on it (run() below), so the message callback does
        // too.
        bool reachedRunning = false;
        bool serverRejected = false;
        bool errorReported  = false;

        auto socket = std::make_unique<ix::WebSocket>();
        socket->setUrl(kAisUrl);

        // This loop owns retry pacing. The library's automatic reconnection
        // is no help here: it only backs off between consecutive failed
        // handshakes and reconnects instantly after a session that connected
        // and then dropped - which is exactly what a rejected subscribe
        // looks like (the server completes the handshake, sends a
        // type:"error" frame, and closes).
        socket->disableAutomaticReconnection();

        socket->setOnMessageCallback(
            [this, &subscribeMsg, &reachedRunning, &serverRejected, &errorReported](const ix::WebSocketMessagePtr& msg)
            {
                switch (msg->type)
                {
                case ix::WebSocketMessageType::Open:
                {
                    // Send the subscribe request once the connection is up.
                    // Scoped so the lock is released before SetState() below
                    // invokes the (user-supplied) state callback.
                    {
                        std::lock_guard<std::mutex> lk(m_socketMutex);
                        if (m_socket)
                        {
                            m_socket->send(subscribeMsg);
                        }
                    }
                    reachedRunning = true;
                    SetState(State::Running);
                    break;
                }

                case ix::WebSocketMessageType::Message:
                    if (HandleMessage(msg->str))
                    {
                        // Server rejected the subscribe (type:"error" frame,
                        // already surfaced by HandleMessage); it closes the
                        // connection right after.
                        serverRejected = true;
                        errorReported = true;
                    }
                    break;

                case ix::WebSocketMessageType::Error:
                    errorReported = true;
                    SetState(State::Error, wxString::FromUTF8(msg->errorInfo.reason.c_str()));
                    break;

                case ix::WebSocketMessageType::Close:
                    // A close from our own Stop()/Restart() arrives with
                    // m_streaming already false; the supervisor reports
                    // Stopped when it exits. Don't overwrite a more specific
                    // reason already shown for this session.
                    if (m_streaming.load() && !errorReported)
                    {
                        SetState(State::Error, wxString("connection lost"));
                    }
                    break;

                default:
                    break; // Ping/Pong/Fragment are handled internally by the library.
                }
            });

        ix::WebSocket* raw = nullptr;
        {
            // Publish the socket so Stop() can close() it from another
            // thread. Recheck the flag under the same lock so the socket can
            // never appear after Stop() has already looked for one to close.
            std::lock_guard<std::mutex> lock(m_socketMutex);
            if (!m_streaming.load())
            {
                break;
            }
            m_socket = std::move(socket);
            raw = m_socket.get();
        }

        // Blocks on this thread until the session ends: connect failure,
        // drop, server close, or Stop() closing the socket (close() also
        // cancels an in-flight handshake).
        raw->run();

        // Tear down unlocked so Stop() never waits on the destructor.
        std::unique_ptr<ix::WebSocket> finished;
        {
            std::lock_guard<std::mutex> lock(m_socketMutex);
            finished = std::move(m_socket);
        }
        finished.reset();

        if (!m_streaming.load())
        {
            break;
        }

        if (serverRejected)
        {
            backoffSeconds = kMaxBackoffSeconds; // explicit rejection; don't hammer the server
        }
        else if (reachedRunning)
        {
            backoffSeconds = kInitialBackoffSeconds; // had a good session; retry soon
        }
        SleepWhileStreaming(backoffSeconds);
        backoffSeconds = std::min(backoffSeconds * 2, kMaxBackoffSeconds);
    }

    SetState(State::Stopped);
}

void AisStreamClient::SleepWhileStreaming(int seconds)
{
    // Chunked so Stop() never waits out a full backoff interval.
    for (int i = 0; i < seconds * 10 && m_streaming.load(); ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void AisStreamClient::Stop()
{
    // m_streaming means "streaming wanted": clearing it ends the supervisor
    // loop, cuts any backoff sleep short, and tells the Close handler this
    // teardown is deliberate, not a drop.
    m_streaming = false;

    {
        std::lock_guard<std::mutex> lock(m_socketMutex);
        if (m_socket)
        {
            // Unblocks the supervisor's run(); the supervisor owns teardown.
            m_socket->close();
        }
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