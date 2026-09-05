#ifndef DIALOG_SETTINGS_H
#define DIALOG_SETTINGS_H

// wxWidgets Precompiled Headers
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include "settings_ui_base.h"
#include "global_settings.h"

class DialogSettings : public DialogSettingsBase
{
  public:
    DialogSettings(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Template Plugin Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
    ~DialogSettings();

  protected:
    // Overridden methods from the base class
    void OnInit(wxInitDialogEvent& event) override;
    void OnOK(wxCommandEvent& event) override;
    void OnApply(wxCommandEvent& event) override;

  private:
    void updateSettings();
};

#endif //DIALOG_SETTINGS_H
