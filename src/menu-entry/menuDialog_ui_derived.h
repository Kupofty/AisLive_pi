#ifndef __menuDialog_ui_derived__
#define __menuDialog_ui_derived__

#include "menuDialog_ui_base.h"

enum class DialogAction
{
    None = 0,
    UpdateToBoat = 1,
    UpdateToCursor = 2
};

class DialogMenuEntry : public DialogMenuEntryBase
{
  public:
    DialogMenuEntry( wxWindow* parent );
    DialogAction m_action = DialogAction::None;

  protected:
    void OnButtonClick_UpdateSearchPosition(wxCommandEvent& event) override;
    void OnButtonClick_UpdateSearchPosOnBoat(wxCommandEvent& event) override;
};

#endif // __menuDialog_ui_derived__
