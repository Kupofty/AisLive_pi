#ifndef __menuDialog_ui_derived__
#define __menuDialog_ui_derived__

#include "menuDialog_ui_base.h"


class DialogMenuEntry : public DialogMenuEntryBase
{
  public:
    DialogMenuEntry( wxWindow* parent );

  protected:
    void OnButtonClick_UpdateDummy( wxCommandEvent& event ) override;
};

#endif // __menuDialog_ui_derived__
