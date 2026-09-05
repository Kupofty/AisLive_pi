#ifndef DIALOG_MAIN_GUI
#define DIALOG_MAIN_GUI

#include "main_ui_base.h"
#include "settings/global_settings.h"

class TemplatePlugin;

// Main class
class DialogMainGui : public DialogMainGuiBase
{
  public:
    DialogMainGui(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Template Plugin GUI"),
                  const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
    ~DialogMainGui();

    TemplatePlugin* plugin = nullptr;

  protected:
    void OnClose(wxCloseEvent& event) override;
};

#endif //DIALOG_MAIN_GUI
