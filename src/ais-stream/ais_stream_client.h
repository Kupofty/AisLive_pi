#ifndef AIS_STREAM_CLIENT_H
#define AIS_STREAM_CLIENT_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <wx/string.h>

// Connects to the OpenWaters.io AIS websocket feed for a given search area
// and delivers decoded NMEA sentences via callback until stopped.
//
// Owns its own background thread; Start()/Stop()/Restart() are safe to call
// from the GUI thread. The sentence callback is invoked from the background
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

        // Stops streaming and blocks until the worker thread has exited. No-op
        // if not running.
        void Stop();

        // Stop() + Start() with new parameters; no-op (does not start) if not
        // already streaming.
        void Restart(double latitude, double longitude, double boxSizeDegrees, SentenceCallback onSentence);

        bool IsStreaming() const;

    private:
        struct Session;

        // Minimal RFC 6455 opcodes used by the frame read/write helpers.
        enum class WsOpcode : unsigned char
        {
            Continuation = 0x0,
            Text         = 0x1,
            Binary       = 0x2,
            Close        = 0x8,
            Ping         = 0x9,
            Pong         = 0xA
        };

        void ThreadFunc(double latitude, double longitude, double boxSizeDegrees);

        bool TlsConnect(Session& s, const std::string& host, int port);
        bool WsHandshake(Session& s, const std::string& host, const std::string& target);
        bool WsSendFrame(Session& s, WsOpcode opcode, const std::string& payload);
        bool WsReadFrame(Session& s, std::string& payloadOut, WsOpcode& opcodeOut);

        // Guards the m_session pointer against Stop()/ThreadFunc() races.
        std::mutex m_sessionMutex;
        std::unique_ptr<Session> m_session;
        std::thread m_thread;
        std::atomic<bool> m_streaming{false};
        SentenceCallback m_onSentence;
};

#endif // AIS_STREAM_CLIENT_H
