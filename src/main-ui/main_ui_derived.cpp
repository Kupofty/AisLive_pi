#include <cmath>
#include <algorithm>
#include <cstdint>

#include "main_ui_derived.h"
#include "settings/global_settings.h"
#include "plugin/plugin.h"
#include "utils/utils.h"


namespace {

constexpr double kFollowThresholdFraction = 0.3;

// Returns the re-center threshold in nm, based on the smaller (worst-case)
// dimension of the search box at the given latitude — the longitude side
// shrinks as cos(latitude), so it's the limiting factor away from the equator.
double ComputeFollowThresholdNm(double searchBoxSizeDegrees, double latitude)
{
    double halfSideLatNm = (searchBoxSizeDegrees * Utils::kNmPerDegreeLatitude) / 2.0;
    double halfSideLonNm = halfSideLatNm * std::cos(Utils::DegToRad(latitude));

    double limitingHalfSideNm = std::min(halfSideLatNm, halfSideLonNm);

    return limitingHalfSideNm * kFollowThresholdFraction;
}

// True if a non-empty (after trimming) token is configured in the settings.
bool HasToken()
{
    wxString token = g_openWatersToken;   // copy: Trim() modifies in place
    token.Trim(true).Trim(false);
    return !token.empty();
}

} // namespace


////////////////////////////
/// Class Initialization ///
////////////////////////////
DialogMainGui::DialogMainGui(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : DialogMainGuiBase( parent )
{
    //Always open on first tab
    m_notebook->SetSelection(0);

    // Init wx widgets
    updateSearchBoxSize(g_lastSearchBoxSize);
    m_checkBox_activateVesselFollowing->SetValue(g_activateVesselsFollowing);

    // Configure columns &  Populate the MMSI list
    m_dataViewListCtrl_followedVessels->AppendTextColumn(
        "MMSI", wxDATAVIEW_CELL_EDITABLE, 90, wxALIGN_LEFT
        );
    m_dataViewListCtrl_followedVessels->AppendTextColumn(
        "Name", wxDATAVIEW_CELL_EDITABLE, 130, wxALIGN_LEFT
        );

    for (size_t i = 0; i < g_mmsiList.size(); ++i)
    {
        wxVector<wxVariant> row;

        //Load MMSI
        const uint32_t mmsi = g_mmsiList[i];
        row.push_back(wxVariant(wxString::Format("%09u", static_cast<unsigned int>(mmsi))));

        // Load the corresponding comment if available
        const wxString comment = (i < g_mmsiListNames.size()) ? g_mmsiListNames[i] : "";
        row.push_back(wxVariant(comment));

        m_dataViewListCtrl_followedVessels->AppendItem(row);
    }

    m_textCtrl_mmsiComment->SetHint("Vessel name...");

    // AisStreamClient may invoke this from its own background thread (e.g.
    // an unexpected disconnect), so marshal back to the GUI thread. Using
    // this->CallAfter() (rather than wxTheApp->CallAfter()) ties the
    // pending call to this dialog's event handler: if the dialog is
    // destroyed (see ~DialogMainGui -> StopAisStream()) before the call
    // runs, wxWidgets drops it instead of invoking a lambda that captured
    // a now-dangling `this`.
    m_aisStream.SetStateCallback(
        [this](AisStreamClient::State state, const wxString& detail)
        {
            const wxString detailCopy = detail.Clone(); // deep copy: wxString is not thread-safe to share
            this->CallAfter([this, state, detailCopy]()
                            {
                                OnAisStreamStateChanged(state, detailCopy);
                            });
        });

    Layout();
}

DialogMainGui::~DialogMainGui()
{
    StopAisStream();
}



/////////////////////
/// Input updates ///
/////////////////////
void DialogMainGui::updateSearchZoneOnCoord(double lat, double lon)
{
    //Deactivate follow boat mode
    m_followBoatMode = false;
    m_checkBox_followBoatMode->SetValue(false);

    //Update to manually selected position
    updateSearchPosition(lat, lon);
}

void DialogMainGui::activateFollowBoatMode()
{
    m_followBoatMode = true;
    m_checkBox_followBoatMode->SetValue(true);
    updateSearchPosition(m_boatLatitude, m_boatLongitude);
}

void DialogMainGui::updateSearchPosition(double lat, double lon)
{
    //Update search position
    m_searchLatitude = lat;
    m_searchLongitude = lon;

    //Update labels
    const wxString latDir = lat >= 0.0 ? "N" : "S";
    const wxString lonDir = lon >= 0.0 ? "E" : "W";

    const wxString degreeSymbol(wxUniChar(0x00B0));

    m_staticText_searchLatitude->SetLabel(
        wxString::Format("%.6f", std::abs(lat)) + degreeSymbol + " " + latDir
        );

    m_staticText_searchLongitude->SetLabel(
        wxString::Format("%.6f", std::abs(lon)) + degreeSymbol + " " + lonDir
        );

    //Refresh stream with updated search location
    RestartAisStream();
}

void DialogMainGui::updateSearchBoxSize(double degrees)
{
    //Max range depends on whether a token is configured
    const int maxValue = HasToken() ? 20 : 10;
    const int minValue = 0;

    m_slider_searchBoxSize->SetRange(minValue, maxValue);
    degrees = std::clamp(degrees, static_cast<double>(minValue), static_cast<double>(maxValue));

    m_slider_searchBoxSize->SetValue(static_cast<int>(degrees));

    m_searchBoxSize = degrees;

    const wxString degreeSymbol(wxUniChar(0x00B0));
    m_staticText_searchBoxSize->SetLabel(
        wxString::Format("%.0f", degrees) + degreeSymbol +
        " x " +
        wxString::Format("%.0f", degrees) + degreeSymbol
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
        Layout();
    }

    //Update search position if boat moved far enough
    if (m_followBoatMode)
    {
        if(m_searchBoxSize == 0)
            return;

        double distanceNm = Utils::GreatCircleDistanceNm(m_searchLatitude, m_searchLongitude,
                                                         m_boatLatitude, m_boatLongitude);

        double thresholdNm = ComputeFollowThresholdNm(m_searchBoxSize, m_boatLatitude);

        if (distanceNm >= thresholdNm)
        {
            updateSearchPosition(m_boatLatitude, m_boatLongitude);
        }
    }
}



///////////////
/// Getters ///
///////////////
double DialogMainGui::getSearchBoxSize()
{
    return m_searchBoxSize;
}

bool DialogMainGui::isStreamingData()
{
    return m_aisStream.IsStreaming();
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

    const wxString degreeSymbol(wxUniChar(0x00B0));
    m_staticText_searchBoxSize->SetLabel(
        wxString::Format("%.0f", degrees) + degreeSymbol +
        " x " +
        wxString::Format("%.0f", degrees) + degreeSymbol
        );

}

void DialogMainGui::OnScrollThumbRelease_UpdateSearchBoxSize(wxScrollEvent& event)
{
    double degrees = m_slider_searchBoxSize->GetValue();
    updateSearchBoxSize(degrees);
}

void DialogMainGui::OnCheckBox_FollowBoatMode(wxCommandEvent& event)
{
    m_followBoatMode = event.IsChecked();

    // Snap to the boat immediately when the user turns it on
    if (m_followBoatMode)
    {
        updateSearchPosition(m_boatLatitude, m_boatLongitude);
    }
}

void DialogMainGui::OnButtonClick_AddMmsiToList(wxCommandEvent& event)
{
    const int value = m_spinCtrl_followedMMSI->GetValue();
    const uint32_t mmsi = static_cast<uint32_t>(value);
    const wxString comment = m_textCtrl_mmsiComment->GetValue();

    // Prevent duplicates
    if (std::find(g_mmsiList.begin(), g_mmsiList.end(), mmsi)
        != g_mmsiList.end())
    {
        return;
    }

    // Update MMSI list
    g_mmsiList.push_back(mmsi);
    g_mmsiListNames.push_back(comment);

    // Add MMSI and comment to the two-column widget
    wxVector<wxVariant> row;
    row.push_back(wxVariant(wxString::Format("%09u", static_cast<unsigned int>(mmsi))));
    row.push_back(wxVariant(comment));
    m_dataViewListCtrl_followedVessels->AppendItem(row);

    //Clear comment/name for next one
    m_textCtrl_mmsiComment->Clear();

    Layout();

    // Restart stream with newest MMSI list
    RestartAisStream();
}

void DialogMainGui::OnButtonClick_DeleteMmsiFromList(wxCommandEvent& event)
{
    const int selectedIndex = m_dataViewListCtrl_followedVessels->GetSelectedRow();

    // Ensure an item is selected
    if (selectedIndex == wxNOT_FOUND)
        return;

    // Remove the corresponding MMSI & name from the vector
    g_mmsiList.erase(g_mmsiList.begin() + selectedIndex);
    g_mmsiListNames.erase(g_mmsiListNames.begin() + selectedIndex);

    // Remove the selected row (MMSI and comment)
    m_dataViewListCtrl_followedVessels->DeleteItem(selectedIndex);

    Layout();

    // Restart stream with updated MMSI list
    RestartAisStream();
}

void DialogMainGui::OnButtonClick_ClearMmsiList(wxCommandEvent& event)
{
    // Clear MMSI data
    g_mmsiList.clear();
    g_mmsiListNames.clear();

    // Clear the GUI (MMSI and comments)
    m_dataViewListCtrl_followedVessels->DeleteAllItems();

    Layout();

    // Restart stream with empty MMSI list
    RestartAisStream();
}

void DialogMainGui::OnDataViewListCtrlItemValueChanged_updateMmsiList(wxDataViewEvent& event)
{
    const int row = m_dataViewListCtrl_followedVessels->ItemToRow(event.GetItem());

    if (row == wxNOT_FOUND)
        return;

    const int column = event.GetColumn();

    wxVariant value;
    m_dataViewListCtrl_followedVessels->GetValue(value, row, column);

    // Column 0: MMSI
    if (column == 0)
    {
        unsigned long parsedMmsi = 0;
        const wxString text = value.GetString();

        if (!text.ToULong(&parsedMmsi)
            || parsedMmsi < 100000000UL
            || parsedMmsi > 999999999UL)
        {
            goto restore_mmsi;
        }

        // Prevent duplicate MMSIs
        for (size_t i = 0; i < g_mmsiList.size(); ++i)
        {
            if (i != static_cast<size_t>(row)
                && g_mmsiList[i] == parsedMmsi)
            {
                goto restore_mmsi;
            }
        }

        g_mmsiList[row] = static_cast<uint32_t>(parsedMmsi);
        RestartAisStream();
        return;

    restore_mmsi:
        m_dataViewListCtrl_followedVessels->SetValue(
            wxVariant(wxString::Format(
                "%09u",
                static_cast<unsigned int>(g_mmsiList[row]))),
            row,
            column);
    }
    // Column 1: Name / Comment
    else if (column == 1)
    {
        if (row >= static_cast<int>(g_mmsiListNames.size()))
            g_mmsiListNames.resize(g_mmsiList.size());

        g_mmsiListNames[row] = value.GetString();
    }

}

void DialogMainGui::m_checkBox_activateVesselFollowingOnCheckBox(wxCommandEvent& event)
{
    g_activateVesselsFollowing = m_checkBox_activateVesselFollowing->GetValue();
    RestartAisStream();
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

    //Update token
    m_aisStream.SetToken(g_openWatersToken);
    m_aisStream.SetTokenEnabled(HasToken());

    // This callback runs on AisStreamClient's background thread; the OpenCPN
    // plugin API is not thread-safe, so marshal the push to the GUI thread.
    // Queue on this dialog (not wxTheApp) so pending events are discarded
    // when the dialog is destroyed - Plugin::DeInit() deletes the dialog
    // (joining the stream thread) before destroy_pi() frees the plugin, so
    // a queued event can never run against a dead plugin.
    m_aisStream.Start(m_searchLatitude, m_searchLongitude, m_searchBoxSize, g_activateVesselsFollowing, g_mmsiList, m_getSnapshot,
                      [this](const wxString& sentence)
                      {
                          CallAfter([this, sentence]()
                                    {
                                        if (plugin)
                                        {
                                            plugin->sendNmeaSentence(sentence);
                                        }
                                    });
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

void DialogMainGui::RestartAisStream()
{
    if (!m_aisStream.IsStreaming())
    {
        return;
    }

    // Same marshaling as StartAisStream: the callback runs on the websocket
    // thread and the plugin API must only be called from the GUI thread.
    m_aisStream.Restart(m_searchLatitude, m_searchLongitude, m_searchBoxSize, g_activateVesselsFollowing, g_mmsiList, m_getSnapshot,
                        [this](const wxString& sentence)
                        {
                            CallAfter([this, sentence]()
                                      {
                                          if (plugin)
                                          {
                                              plugin->sendNmeaSentence(sentence);
                                          }
                                      });
                        });
}

void DialogMainGui::OnAisStreamStateChanged(AisStreamClient::State state, const wxString& detail)
{
    switch (state)
    {
    case AisStreamClient::State::Stopped:
        m_staticText_streamState->SetLabel(_("Stopped"));
        break;

    case AisStreamClient::State::Connecting:
        m_staticText_streamState->SetLabel(_("Connecting..."));
        break;

    case AisStreamClient::State::Running:
        m_staticText_streamState->SetLabel(_("Running"));
        break;

    case AisStreamClient::State::Error:
        m_staticText_streamState->SetLabel(
            detail.empty()
                ? _("Unknown error")
                : wxString(_("Error")) + " (" + detail + ")");
        break;
    }
}

void DialogMainGui::refreshTokenState()
{
    m_aisStream.SetToken(g_openWatersToken);
    m_aisStream.SetTokenEnabled(HasToken());

    updateSearchBoxSize(m_slider_searchBoxSize->GetValue());
}
