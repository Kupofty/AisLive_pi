#include "ais_stream_client.h"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessageType.h>

#include <json/json.h>


namespace {

constexpr char kAisUrl[] = "wss://ais.openwaters.io/v1/stream";

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

void AisStreamClient::HandleMessage(const std::string& payload)
{
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



///////////////
/// State   ///
///////////////
void AisStreamClient::SetStateCallback(StateCallback onStateChanged)
{
    std::lock_guard<std::mutex> lock(m_stateCallbackMutex);
    m_onStateChanged = std::move(onStateChanged);
}

void AisStreamClient::SetState(State state)
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
        cb(state);
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

    m_onSentence = std::move(onSentence);

    auto socket = std::make_unique<ix::WebSocket>();
    socket->setUrl(kAisUrl);

    // We manage our own Start/Stop/Restart lifecycle explicitly; don't let
    // the library reconnect behind our back.
    socket->disableAutomaticReconnection();

    const std::string subscribeMsg = BuildSubscribeMessage(latitude, longitude, boxSizeDegrees);

    socket->setOnMessageCallback([this, subscribeMsg](const ix::WebSocketMessagePtr& msg)
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
                                         SetState(State::Running);
                                         break;
                                     }

                                     case ix::WebSocketMessageType::Message:
                                         HandleMessage(msg->str);
                                         break;

                                     case ix::WebSocketMessageType::Error:
                                         // Connection failed unexpectedly; reflect that in
                                         // IsStreaming() so the caller knows to Start() again
                                         // if it wants to retry.
                                         m_streaming = false;
                                         SetState(State::Error);
                                         break;

                                     case ix::WebSocketMessageType::Close:
                                         m_streaming = false;
                                         SetState(State::Stopped);
                                         break;

                                     default:
                                         break; // Ping/Pong/Fragment are handled internally by the library.
                                     }
                                 });

    {
        // Publish m_socket only briefly under the lock, so the Open/Message
        // callback above (which can in principle fire from the background
        // thread before socket->start() below returns) can never block on a
        // mutex still held by this thread while it's inside socket->start().
        std::lock_guard<std::mutex> lock(m_socketMutex);
        m_socket = std::move(socket);
    }

    m_streaming = true;
    SetState(State::Connecting);
    m_socket->start();
}

void AisStreamClient::Stop()
{
    // No early-out on m_streaming: the message callback clears that flag
    // itself when the connection drops, but the socket still needs to be
    // torn down and its thread joined.
    m_streaming = false;

    std::unique_ptr<ix::WebSocket> socket;
    {
        std::lock_guard<std::mutex> lock(m_socketMutex);
        socket = std::move(m_socket);
    }

    if (socket)
    {
        // Blocks until the connection is closed and IXWebSocket's background
        // thread has fully exited.
        socket->stop();
    }

    SetState(State::Stopped);
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