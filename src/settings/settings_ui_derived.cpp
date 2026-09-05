#include "settings_ui_derived.h"


////////////////////////////
/// Class Initialization ///
////////////////////////////
DialogSettings::DialogSettings(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : DialogSettingsBase(parent, id, title, pos, size, style)
{
  m_notebook1->SetSelection(0);
}

DialogSettings::~DialogSettings()
{
}

void DialogSettings::OnInit(wxInitDialogEvent& event)
{
  //Update UI
  m_checkBox_restoreWindowPos->SetValue(g_restoreWindowPos);
  m_checkBox_restoreWindowSize->SetValue(g_restoreWindowSize);
  m_checkBox_keepWindowActive->SetValue(g_keepWindowActive);

  Layout();
  Fit();
}



//////////////
/// Others ///
//////////////
void DialogSettings::updateSettings()
{
  g_restoreWindowPos = m_checkBox_restoreWindowPos->IsChecked();
  g_restoreWindowSize = m_checkBox_restoreWindowSize->IsChecked();
  g_keepWindowActive = m_checkBox_keepWindowActive->IsChecked();
}



////////////////////
/// wxGUI Events ///
////////////////////
void DialogSettings::OnOK(wxCommandEvent& event)
{
  updateSettings();
  EndModal(wxID_OK);
}

void DialogSettings::OnApply(wxCommandEvent& event)
{
  updateSettings();
}
