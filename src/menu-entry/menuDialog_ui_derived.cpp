#include "menuDialog_ui_derived.h"

DialogMenuEntry::DialogMenuEntry(wxWindow* parent): DialogMenuEntryBase(parent)
{

}

void DialogMenuEntry::OnButtonClick_UpdateSearchPosition( wxCommandEvent& event )
{
  EndModal(wxID_OK);
}

