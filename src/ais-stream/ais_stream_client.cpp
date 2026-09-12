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
        if (!nmea.EndsWith("\r\n"))
        {
            nmea += "\r\n";
        }

        sendSentence(nmea);
    }
}

} // namespace


AisStreamClient::AisStreamClient()
{
    // Required once per process before any ix::WebSocket is used; safe to
    // call more than once across multiple client instances.
    ix::initNetSystem();
}

AisStreamClient::~AisStreamClient()
{
    Stop();
    ix::uninitNetSystem();
}

std::string AisStreamClient::BuildSubscribeMessage(double latitude, double longitude, double boxSizeDegrees) const
{
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

////////////////////////
/// Public interface  ///
////////////////////////
void AisStreamClient::Start(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence)
{
    if (m_streaming.load())
    {
        return; // already running
    }

    std::lock_guard<std::mutex> lock(m_socketMutex);

    m_onSentence = std::move(onSentence);

    m_socket = std::make_unique<ix::WebSocket>();
    m_socket->setUrl(kAisUrl);

    // We manage our own Start/Stop/Restart lifecycle explicitly; don't let
    // the library reconnect behind our back.
    m_socket->disableAutomaticReconnection();

    const std::string subscribeMsg = BuildSubscribeMessage(latitude, longitude, boxSizeDegrees);

    m_socket->setOnMessageCallback([this, subscribeMsg](const ix::WebSocketMessagePtr& msg)
                                   {
                                       switch (msg->type)
                                       {
                                       case ix::WebSocketMessageType::Open:
                                       {
                                           // Send the subscribe request once the connection is up.
                                           std::lock_guard<std::mutex> lk(m_socketMutex);
                                           if (m_socket)
                                           {
                                               m_socket->send(subscribeMsg);
                                           }
                                           break;
                                       }

                                       case ix::WebSocketMessageType::Message:
                                           HandleMessage(msg->str);
                                           break;

                                       case ix::WebSocketMessageType::Error:
                                       case ix::WebSocketMessageType::Close:
                                           // Connection dropped or failed; reflect that in IsStreaming()
                                           // so the caller knows to Start() again if it wants to retry.
                                           m_streaming = false;
                                           break;

                                       default:
                                           break; // Ping/Pong/Fragment are handled internally by the library.
                                       }
                                   });

    m_streaming = true;
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