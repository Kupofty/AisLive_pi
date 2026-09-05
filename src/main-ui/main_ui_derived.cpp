#include <cmath>
#include <ctime>
#include <wx/hyperlink.h>
#include <wx/clipbrd.h>

#include "main_ui_derived.h"
#include "settings/global_settings.h"
#include "plugin/template_plugin.h"


////////////////////////////
/// Class Initialization ///
////////////////////////////
DialogMainGui::DialogMainGui(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : DialogMainGuiBase( parent )
{

}

DialogMainGui::~DialogMainGui()
{

}




//////////////
/// Others ///
//////////////
void DialogMainGui::OnClose(wxCloseEvent& event)
{
  if (plugin)
  {
    plugin->OnGuiClosed();

  }
}



