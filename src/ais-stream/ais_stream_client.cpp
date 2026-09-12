#include "ais_stream_client.h"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessageType.h>

#include <json/json.h>

#include <wx/log.h>


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
    wxLogMessage("AisStreamClient: ctor - calling ix::initNetSystem()");
    // Required once per process before any ix::WebSocket is used; safe to
    // call more than once across multiple client instances. On Windows this
    // does WSAStartup().
    const bool ok = ix::initNetSystem();
    wxLogMessage("AisStreamClient: ctor - ix::initNetSystem() returned %s", ok ? "true" : "false");
}

AisStreamClient::~AisStreamClient()
{
    wxLogMessage("AisStreamClient: dtor - calling Stop()");
    Stop();
    wxLogMessage("AisStreamClient: dtor - calling ix::uninitNetSystem()");
    ix::uninitNetSystem();
    wxLogMessage("AisStreamClient: dtor - done");
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
    else
    {
        wxLogMessage("AisStreamClient: HandleMessage() - failed to parse frame as JSON (%zu bytes)", payload.size());
    }
}

////////////////////////
/// Public interface  ///
////////////////////////
void AisStreamClient::Start(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence)
{
    wxLogMessage("AisStreamClient: Start() called (lat=%f, lon=%f, box=%f)", latitude, longitude, boxSizeDegrees);

    if (m_streaming.load())
    {
        wxLogMessage("AisStreamClient: Start() - already streaming, ignoring");
        return; // already running
    }

    m_onSentence = std::move(onSentence);

    wxLogMessage("AisStreamClient: Start() - constructing ix::WebSocket");
    auto socket = std::make_unique<ix::WebSocket>();
    wxLogMessage("AisStreamClient: Start() - ix::WebSocket constructed OK");

    socket->setUrl(kAisUrl);
    wxLogMessage("AisStreamClient: Start() - URL set to %s", kAisUrl);

    // We manage our own Start/Stop/Restart lifecycle explicitly; don't let
    // the library reconnect behind our back.
    socket->disableAutomaticReconnection();

    const std::string subscribeMsg = BuildSubscribeMessage(latitude, longitude, boxSizeDegrees);
    wxLogMessage("AisStreamClient: Start() - subscribe message built (%zu bytes)", subscribeMsg.size());

    socket->setOnMessageCallback([this, subscribeMsg](const ix::WebSocketMessagePtr& msg)
                                 {
                                     switch (msg->type)
                                     {
                                     case ix::WebSocketMessageType::Open:
                                     {
                                         wxLogMessage("AisStreamClient: callback - OPEN (uri=%s)", msg->openInfo.uri.c_str());
                                         // Send the subscribe request once the connection is up.
                                         std::lock_guard<std::mutex> lk(m_socketMutex);
                                         if (m_socket)
                                         {
                                             wxLogMessage("AisStreamClient: callback - sending subscribe message");
                                             m_socket->send(subscribeMsg);
                                             wxLogMessage("AisStreamClient: callback - subscribe message sent");
                                         }
                                         else
                                         {
                                             wxLogMessage("AisStreamClient: callback - OPEN fired but m_socket is null (Stop() raced us)");
                                         }
                                         break;
                                     }

                                     case ix::WebSocketMessageType::Message:
                                         wxLogMessage("AisStreamClient: callback - MESSAGE (%zu bytes)", msg->str.size());
                                         HandleMessage(msg->str);
                                         break;

                                     case ix::WebSocketMessageType::Error:
                                         wxLogMessage(
                                             "AisStreamClient: callback - ERROR reason='%s' http_status=%d retries=%d wait_time_ms=%d decompression_error=%s",
                                             msg->errorInfo.reason.c_str(),
                                             msg->errorInfo.http_status,
                                             msg->errorInfo.retries,
                                             static_cast<int>(msg->errorInfo.wait_time),
                                             msg->errorInfo.decompressionError ? "true" : "false");
                                         // Connection dropped or failed; reflect that in IsStreaming()
                                         // so the caller knows to Start() again if it wants to retry.
                                         m_streaming = false;
                                         break;

                                     case ix::WebSocketMessageType::Close:
                                         wxLogMessage("AisStreamClient: callback - CLOSE code=%d reason='%s'",
                                                      msg->closeInfo.code, msg->closeInfo.reason.c_str());
                                         m_streaming = false;
                                         break;

                                     default:
                                         wxLogMessage("AisStreamClient: callback - other message type=%d", static_cast<int>(msg->type));
                                         break; // Ping/Pong/Fragment are handled internally by the library.
                                     }
                                 });
    wxLogMessage("AisStreamClient: Start() - message callback registered");

    {
        // Publish m_socket only briefly under the lock, so the Open/Message
        // callback above (which can in principle fire from the background
        // thread before socket->start() below returns) can never block on a
        // mutex still held by this thread while it's inside socket->start().
        std::lock_guard<std::mutex> lock(m_socketMutex);
        m_socket = std::move(socket);
    }

    m_streaming = true;
    wxLogMessage("AisStreamClient: Start() - calling m_socket->start()");
    m_socket->start();
    wxLogMessage("AisStreamClient: Start() - m_socket->start() returned, Start() complete");
}

void AisStreamClient::Stop()
{
    wxLogMessage("AisStreamClient: Stop() called");

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
        wxLogMessage("AisStreamClient: Stop() - calling socket->stop()");
        // Blocks until the connection is closed and IXWebSocket's background
        // thread has fully exited.
        socket->stop();
        wxLogMessage("AisStreamClient: Stop() - socket->stop() returned");
    }
    else
    {
        wxLogMessage("AisStreamClient: Stop() - no socket to stop");
    }
}

void AisStreamClient::Restart(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence)
{
    wxLogMessage("AisStreamClient: Restart() called");

    if (!m_streaming.load())
    {
        wxLogMessage("AisStreamClient: Restart() - not currently streaming, ignoring");
        return;
    }

    Stop();
    Start(latitude, longitude, boxSizeDegrees, std::move(onSentence));
}

bool AisStreamClient::IsStreaming() const
{
    return m_streaming.load();
}