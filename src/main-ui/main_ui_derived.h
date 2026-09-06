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

protected:
    void OnClose(wxCloseEvent& event) override;
    void OnButtonClick_startStream(wxCommandEvent& event) override;
    void OnButtonClick_stopStream(wxCommandEvent& event) override;

private:
    struct AisStreamSession;
    std::unique_ptr<AisStreamSession> m_aisSession;

    std::thread m_streamThread;
    std::atomic<bool> m_streaming{false};

    void StartAisStream();
    void StopAisStream();
    void AisStreamThreadFunc();
};

#endif //DIALOG_MAIN_GUI