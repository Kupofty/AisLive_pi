#include <cmath>

#include "main_ui_derived.h"
#include "settings/global_settings.h"
#include "plugin/plugin.h"


////////////////////////////
/// Class Initialization ///
////////////////////////////
DialogMainGui::DialogMainGui(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : DialogMainGuiBase( parent )
{
    // Runs on the stream's worker thread; CallAfter hops to the GUI thread.
    // Queued on this window (not wxTheApp) so pending updates are discarded
    // if the dialog is destroyed before they run; the destructor's Stop()
    // joins the worker, so no new ones arrive after that.
    m_aisStream.SetStatusCallback(
        [this](AisStreamClient::Status status, const wxString& detail)
        {
            const wxString detailCopy = detail.Clone(); // deep copy: wxString is not thread-safe to share
            CallAfter([this, status, detailCopy]()
            {
                UpdateStreamStatusLabel(status, detailCopy);
            });
        });
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
    if (m_aisStream.IsStreaming())
    {
        return; // already running
    }

    // NOTE: this callback runs on AisStreamClient's background thread, not
    // the GUI thread - same as the previous implementation. If
    // plugin->sendNmeaSentence() or anything downstream of it ever touches
    // wx widgets directly, it should marshal back via wxTheApp->CallAfter().
    m_aisStream.Start(m_searchLatitude, m_searchLongitude, m_searchBoxSize,
        [this](const wxString& sentence)
        {
            if (plugin)
            {
                plugin->sendNmeaSentence(sentence);
            }
        });
}

void DialogMainGui::StopAisStream()
{
    if (!m_aisStream.IsStreaming())
    {
        return;
    }

    m_aisStream.Stop();
}

void DialogMainGui::UpdateStreamStatusLabel(AisStreamClient::Status status, const wxString& detail)
{
    wxString label;
    switch (status)
    {
        case AisStreamClient::Status::Connecting:
            label = _("Connecting...");
            break;
        case AisStreamClient::Status::Running:
            label = _("Running");
            break;
        case AisStreamClient::Status::Error:
            label = detail.empty() ? _("Error")
                                   : wxString::Format(_("Error: %s"), detail);
            break;
        case AisStreamClient::Status::Stopped:
            label = _("Stopped");
            break;
    }
    m_staticText_streamState->SetLabel(label);
}

void DialogMainGui::RestartAisStream()
{
    if (!m_aisStream.IsStreaming())
    {
        return;
    }

    m_aisStream.Restart(m_searchLatitude, m_searchLongitude, m_searchBoxSize,
        [this](const wxString& sentence)
        {
            if (plugin)
            {
                plugin->sendNmeaSentence(sentence);
            }
        });
}
