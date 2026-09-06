#include <cmath>
#include <ctime>
#include <functional>
#include <string>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <nlohmann/json.hpp>

#include "main_ui_derived.h"
#include "settings/global_settings.h"
#include "plugin/plugin.h"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;
using json = nlohmann::json;


namespace {

constexpr char kAisHost[] = "ais.openwaters.io";
constexpr char kAisPort[] = "443";
constexpr char kAisTarget[] = "/v1/stream";

void ProcessAisEvent(const json& ev, const std::function<void(const wxString&)>& sendSentence)
{
    if (!ev.contains("type") || ev.at("type") != "event")
    {
        return; // welcome/control message, not an AIS report
    }

    if (!ev.contains("nmea") || !ev.at("nmea").is_array())
    {
        return;
    }

    for (const auto& sentence : ev.at("nmea"))
    {
        if (!sentence.is_string())
        {
            continue;
        }

        wxString nmea = wxString::FromUTF8(sentence.get<std::string>().c_str());
        if (!nmea.EndsWith("\r\n"))
        {
            nmea += "\r\n";
        }

        sendSentence(nmea);
    }
}

}

struct DialogMainGui::AisStreamSession
{
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    std::unique_ptr<websocket::stream<beast::ssl_stream<tcp::socket>>> ws;

    AisStreamSession() { ctx.set_default_verify_paths(); }
};



////////////////////////////
/// Class Initialization ///
////////////////////////////
DialogMainGui::DialogMainGui(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : DialogMainGuiBase( parent )
{

}

DialogMainGui::~DialogMainGui()
{
    StopAisStream();
}



/////////////////////
/// Input updates ///
/////////////////////
void DialogMainGui::updateSearchPosition(double lat, double lon)
{
    //Update search position
    m_searchLatitude = lat;
    m_searchLongitude = lon;

    //Update labels
    const wxString latDir = lat >= 0.0 ? "N" : "S";
    const wxString lonDir = lon >= 0.0 ? "E" : "W";

    m_staticText_searchLatitude->SetLabel(
        wxString::Format("%.6f°%s", std::abs(lat), latDir)
        );

    m_staticText_searchLongitude->SetLabel(
        wxString::Format("%.6f°%s", std::abs(lon), lonDir)
        );

    //Refresh stream with updated search location
    RestartAisStream();
}

void DialogMainGui::updateSearchBoxSize(double degrees)
{
    m_slider_searchBoxSize->SetValue(degrees);

    m_searchBoxSize = degrees;
    m_staticText_searchBoxSize->SetLabel(
        wxString::Format("%.0f° x %.0f°", degrees, degrees)
        );

    RestartAisStream();
}

void DialogMainGui::updateBoatPosition(double lat, double lon)
{
    m_boatLatitude = lat;
    m_boatLongitude = lon;

    // Use the first valid boat position as the initial AIS search position.
    if (!m_initialBoatPositionSet)
    {
        m_initialBoatPositionSet = true;
        updateSearchPosition(lat, lon);
    }
}



///////////////
/// Getters ///
///////////////
double DialogMainGui::getSearchBoxSize()
{
    return m_searchBoxSize;
}



/////////////////
/// UI events ///
/////////////////
void DialogMainGui::OnClose(wxCloseEvent& event)
{
    StopAisStream();

    if (plugin)
    {
        plugin->OnGuiClosed();

    }
}

void DialogMainGui::OnButtonClick_startStream(wxCommandEvent& event)
{
    StartAisStream();
}

void DialogMainGui::OnButtonClick_stopStream(wxCommandEvent& event)
{
    StopAisStream();
}

void DialogMainGui::OnScroll_UpdateSearchBoxSize(wxScrollEvent& event)
{
    double degrees = m_slider_searchBoxSize->GetValue();
    updateSearchBoxSize(degrees);
}



////////////////////
/// AIS streaming ///
////////////////////
void DialogMainGui::StartAisStream()
{
    if (m_streaming.load())
    {
        return; // already running
    }

    m_streaming = true;
    m_streamThread = std::thread(&DialogMainGui::AisStreamThreadFunc, this);

    m_staticText_streamState->SetLabel(_("Running"));
}

void DialogMainGui::StopAisStream()
{
    if (!m_streaming.load())
    {
        return;
    }

    m_streaming = false;
    m_staticText_streamState->SetLabel(_("Stopped"));

    // The worker thread is blocked in a synchronous ws.read(). Closing the
    // underlying socket from this thread is the standard way to unblock a
    // synchronous Asio/Beast read happening on another thread - the read
    // call returns with an error and the loop exits cleanly.
    if (m_aisSession && m_aisSession->ws)
    {
        boost::system::error_code ec;
        beast::get_lowest_layer(*m_aisSession->ws).close(ec);
    }

    if (m_streamThread.joinable())
    {
        m_streamThread.join();
    }
}

void DialogMainGui::RestartAisStream()
{
    const bool wasStreaming = m_streaming.load();

    if (wasStreaming)
    {
        StopAisStream();
        StartAisStream();
    }
}

void DialogMainGui::AisStreamThreadFunc()
{
    try
    {
        m_aisSession = std::make_unique<AisStreamSession>();
        auto& ioc = m_aisSession->ioc;
        auto& ctx = m_aisSession->ctx;

        m_aisSession->ws = std::make_unique<websocket::stream<beast::ssl_stream<tcp::socket>>>(ioc, ctx);
        auto& ws = *m_aisSession->ws;

        // SNI is required by many TLS-terminating hosts.
        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), kAisHost))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        tcp::resolver resolver{ioc};
        auto const results = resolver.resolve(kAisHost, kAisPort);
        auto ep = net::connect(beast::get_lowest_layer(ws), results);

        const std::string host_header = std::string(kAisHost) + ":" + std::to_string(ep.port());

        ws.next_layer().handshake(ssl::stream_base::client);

        ws.handshake(host_header, kAisTarget);

        // Search area
        const json sub_msg = {
            {"type", "subscribe"},
            {"bbox", json::array({json::array({m_searchLatitude  - m_searchBoxSize/2.0,
                                               m_searchLongitude - m_searchBoxSize/2.0,
                                               m_searchLatitude  + m_searchBoxSize/2.0,
                                               m_searchLongitude + m_searchBoxSize/2.0})})}
        };
        const std::string sub_str = sub_msg.dump();
        ws.write(net::buffer(sub_str));

        beast::flat_buffer buffer;
        while (m_streaming.load())
        {
            buffer.clear();
            beast::error_code ec;
            ws.read(buffer, ec);

            if (ec)
            {
                // Either StopAisStream() closed the socket, or the connection
                // dropped. Either way, stop reading.
                break;
            }

            const std::string raw = beast::buffers_to_string(buffer.data());

            try
            {
                const json ev = json::parse(raw);
                ProcessAisEvent(ev, [this](const wxString& sentence)
                {
                    if (plugin)
                    {
                        plugin->sendNmeaSentence(sentence);
                    }
                });
            }
            catch (const json::parse_error&)
            {
                // Ignore malformed frames.
            }
        }
    }

    catch (const std::exception& e)
    {
        // Ignore
    }

    m_streaming = false;
    m_aisSession.reset();
}