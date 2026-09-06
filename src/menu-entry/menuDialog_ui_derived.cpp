#include "menuDialog_ui_derived.h"

DialogMenuEntry::DialogMenuEntry(wxWindow* parent): DialogMenuEntryBase(parent)
{

}

void DialogMenuEntry::OnButtonClick_UpdateSearchPosition( wxCommandEvent& event )
{
    m_action = DialogAction::UpdateToCursor;
    EndModal(wxID_OK);
}

void DialogMenuEntry::OnButtonClick_UpdateSearchPosOnBoat(wxCommandEvent& event)
{
    m_action = DialogAction::UpdateToBoat;
    EndModal(wxID_OK);
}

