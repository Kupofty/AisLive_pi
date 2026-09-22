#ifndef DIALOG_MAIN_GUI
#define DIALOG_MAIN_GUI

#include "main_ui_base.h"
#include "settings/global_settings.h"
#include "ais-stream/ais_stream_client.h"

class Plugin;

// Main class
class DialogMainGui : public DialogMainGuiBase
{
    public:
        DialogMainGui(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("AisLive Plugin GUI"),
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
        ~DialogMainGui();
        Plugin* plugin = nullptr;

        void activateFollowBoatMode();
        void manualUpdateSearchPosition(double lat, double lon);
        void updateBoatPosition(double lat, double lon);
        void updateSearchBoxSize(double degrees);
        double getSearchBoxSize();
        void StopAisStream();
        bool isStreamingData();

    protected:
        void OnClose(wxCloseEvent& event) override;
        void OnButtonClick_startStream(wxCommandEvent& event) override;
        void OnButtonClick_stopStream(wxCommandEvent& event) override;
        void OnScroll_UpdateSearchBoxSize(wxScrollEvent& event) override;
        void OnCheckBox_FollowBoatMode(wxCommandEvent& event) override;

    private:
        AisStreamClient m_aisStream;

        void StartAisStream();
        void RestartAisStream();
        void OnAisStreamStateChanged(AisStreamClient::State state);
        void updateSearchPosition(double lat, double lon);

        bool m_initialBoatPositionSet = false;
        bool m_followBoatMode = true;

        double m_boatLatitude = 0;
        double m_boatLongitude = 0;
        double m_searchLatitude = 0;
        double m_searchLongitude = 0;
        double m_searchBoxSize = 1.0;
};

#endif //DIALOG_MAIN_GUI
