#ifndef AIS_STREAM_CLIENT_H
#define AIS_STREAM_CLIENT_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <wx/string.h>

namespace ix
{
class WebSocket;
}

// Connects to the OpenWaters.io AIS websocket feed for a given search area
// and delivers decoded NMEA sentences via callback until stopped. If the
// connection fails, drops, or is rejected by the server while streaming is
// wanted, a supervisor thread reconnects with capped exponential backoff
// and re-sends the subscription.
//
// Uses IXWebSocket for the protocol work (TLS, the WebSocket handshake,
// framing, ping/pong keepalives); each connection runs on the supervisor
// thread, which owns retry pacing. Start()/Stop()/Restart() are safe to
// call from the GUI thread. The sentence and state callbacks are invoked
// from the supervisor thread, so callers must marshal back to the GUI
// thread themselves if they touch wx widgets from them (e.g. via
// wxTheApp->CallAfter or a wx event).
class AisStreamClient
{
public:
    using SentenceCallback = std::function<void(const wxString&)>;

    // Lifecycle state of the underlying connection. Reported via
    // StateCallback so a UI can display it without polling IsStreaming().
    enum class State
    {
        Stopped,     // Not connected, no connection attempt in progress.
        Connecting,  // Start()/Restart() called, socket handshake in progress.
        Running,     // Connected and subscribed; sentences may be arriving.
        Error        // Connection dropped, failed, or rejected by the
                     // server; a reconnect attempt follows automatically.
    };

    // detail is a short human-readable reason, only meaningful for Error
    // (e.g. the server's own error string on a rejected subscribe).
    using StateCallback = std::function<void(State, const wxString& detail)>;

    AisStreamClient();
    ~AisStreamClient();

    AisStreamClient(const AisStreamClient&) = delete;
    AisStreamClient& operator=(const AisStreamClient&) = delete;

    // Registers a callback invoked whenever the connection state changes.
    // Like SentenceCallback, this may be invoked from AisStreamClient's
    // background thread (e.g. on an unexpected Error/Close), so callers
    // touching wx widgets from it must marshal back to the GUI thread
    // themselves. Call this before Start() to avoid missing the initial
    // transition; safe to call at any other time too.
    void SetStateCallback(StateCallback onStateChanged);

    // Starts streaming for the given search area. No-op if already running.
    void Start(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence);

    // Stops streaming and blocks until the supervisor thread has exited.
    // No-op if not running.
    void Stop();

    // Stop() + Start() with new parameters; no-op (does not start) if not
    // already streaming.
    void Restart(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence);

    // True from Start() until Stop(), including while reconnecting.
    bool IsStreaming() const;

private:
    // Connect/subscribe/read loop with reconnect pacing; runs on m_thread.
    void SupervisorFunc(std::string subscribeMsg);

    // Returns true if the payload was a server type:"error" frame (already
    // surfaced via the state callback).
    bool HandleMessage(const std::string& payload);

    std::string BuildSubscribeMessage(double latitude, double longitude, double boxSizeDegrees) const;
    void SetState(State state, const wxString& detail = wxString());

    // Sleeps up to the given duration, returning early once Stop() runs.
    void SleepWhileStreaming(int seconds);

    // Guards m_socket against concurrent access from the calling thread
    // (Stop) and the supervisor thread that owns the socket.
    std::mutex m_socketMutex;
    std::unique_ptr<ix::WebSocket> m_socket;
    std::thread m_thread;
    std::atomic<bool> m_streaming{false};
    SentenceCallback m_onSentence;

    // Guards m_onStateChanged against SetStateCallback() racing with a
    // state transition fired from the background thread.
    std::mutex m_stateCallbackMutex;
    StateCallback m_onStateChanged;
};

#endif // AIS_STREAM_CLIENT_H