#include <cmath>

#include <wx/app.h>

#include "main_ui_derived.h"
#include "settings/global_settings.h"
#include "plugin/plugin.h"


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

    // This callback runs on AisStreamClient's background thread; the OpenCPN
    // plugin API is not thread-safe, so marshal the push to the GUI thread.
    // The plugin pointer and sentence are captured by value so the queued
    // call stays valid even if this dialog is destroyed first.
    m_aisStream.Start(m_searchLatitude, m_searchLongitude, m_searchBoxSize,
        [this](const wxString& sentence)
        {
            wxTheApp->CallAfter([plugin = this->plugin, sentence]()
            {
                if (plugin)
                {
                    plugin->sendNmeaSentence(sentence);
                }
            });
        });

    m_staticText_streamState->SetLabel(_("Running"));
}

void DialogMainGui::StopAisStream()
{
    if (!m_aisStream.IsStreaming())
    {
        return;
    }

    m_aisStream.Stop();
    m_staticText_streamState->SetLabel(_("Stopped"));
}

void DialogMainGui::RestartAisStream()
{
    if (!m_aisStream.IsStreaming())
    {
        return;
    }

    // Same marshaling as StartAisStream: the callback runs on the websocket
    // thread and the plugin API must only be called from the GUI thread.
    m_aisStream.Restart(m_searchLatitude, m_searchLongitude, m_searchBoxSize,
        [this](const wxString& sentence)
        {
            wxTheApp->CallAfter([plugin = this->plugin, sentence]()
            {
                if (plugin)
                {
                    plugin->sendNmeaSentence(sentence);
                }
            });
        });
}
