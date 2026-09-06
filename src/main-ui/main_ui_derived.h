#ifndef DIALOG_MAIN_GUI
#define DIALOG_MAIN_GUI

#include <atomic>
#include <memory>
#include <thread>

#include "main_ui_base.h"
#include "settings/global_settings.h"

class Plugin;

// Main class
class DialogMainGui : public DialogMainGuiBase
{
public:
    DialogMainGui(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("AisLive Plugin GUI"),
                  const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
    ~DialogMainGui();

    Plugin* plugin = nullptr;
    void updateSearchPosition(double lat, double lon);
    void updateBoatPosition(double lat, double lon);
    void updateSearchBoxSize(double degrees);

protected:
    void OnClose(wxCloseEvent& event) override;
    void OnButtonClick_startStream(wxCommandEvent& event) override;
    void OnButtonClick_stopStream(wxCommandEvent& event) override;
    void OnScroll_UpdateSearchBoxSize(wxScrollEvent& event) override;

private:
    struct AisStreamSession;
    std::unique_ptr<AisStreamSession> m_aisSession;

    std::thread m_streamThread;
    std::atomic<bool> m_streaming{false};

    void StartAisStream();
    void StopAisStream();
    void RestartAisStream();
    void AisStreamThreadFunc();

    bool m_initialBoatPositionSet = false;
    double m_boatLatitude = 0;
    double m_boatLongitude = 0;

    double m_searchLatitude = 0;
    double m_searchLongitude = 0;
    double m_searchBoxSize = 1.0;
};

#endif //DIALOG_MAIN_GUI