#ifndef AIS_STREAM_CLIENT_H
#define AIS_STREAM_CLIENT_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include <wx/string.h>

namespace ix
{
class WebSocket;
}

// Connects to the OpenWaters.io AIS websocket feed for a given search area
// and delivers decoded NMEA sentences via callback until stopped.
//
// Uses IXWebSocket internally, which owns and manages its own background
// connection thread (including TLS, the WebSocket handshake, framing, and
// ping/pong keepalives). Start()/Stop()/Restart() are safe to call from the
// GUI thread. The sentence callback is invoked from IXWebSocket's background
// thread, so callers must marshal back to the GUI thread themselves if they
// touch wx widgets from it (e.g. via wxTheApp->CallAfter or a wx event).
class AisStreamClient
{
public:
    using SentenceCallback = std::function<void(const wxString&)>;

    AisStreamClient();
    ~AisStreamClient();

    AisStreamClient(const AisStreamClient&) = delete;
    AisStreamClient& operator=(const AisStreamClient&) = delete;

    // Starts streaming for the given search area. No-op if already running.
    void Start(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence);

    // Stops streaming and blocks until the underlying connection thread
    // has exited. No-op if not running.
    void Stop();

    // Stop() + Start() with new parameters; no-op (does not start) if not
    // already streaming.
    void Restart(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence);

    bool IsStreaming() const;

private:
    void HandleMessage(const std::string& payload);
    std::string BuildSubscribeMessage(double latitude, double longitude, double boxSizeDegrees) const;

    // Guards m_socket against concurrent access from the calling thread
    // (Start/Stop) and IXWebSocket's own background thread (the message
    // callback).
    std::mutex m_socketMutex;
    std::unique_ptr<ix::WebSocket> m_socket;
    std::atomic<bool> m_streaming{false};
    SentenceCallback m_onSentence;
};

#endif // AIS_STREAM_CLIENT_H