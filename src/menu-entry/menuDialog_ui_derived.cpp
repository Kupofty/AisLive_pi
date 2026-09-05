#include "menuDialog_ui_derived.h"

DialogMenuEntry::DialogMenuEntry(wxWindow* parent): DialogMenuEntryBase(parent)
{

}

void DialogMenuEntry::OnButtonClick_UpdateDummy( wxCommandEvent& event )
{
  EndModal(wxID_OK);
}

