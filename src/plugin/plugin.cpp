//////////////////////////////////////
/// Project:  OpenCPN              ///
/// Plugin:   AisLive              ///
/// Author:   Kupofty              ///
//////////////////////////////////////


///////////////////////
/// Include Headers ///
///////////////////////
#include "plugin.h"
#include "settings/global_settings.h"
#include "settings/settings_ui_derived.h"



///////////////////////
/// Class Factories ///
///////////////////////
extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr) {
  return new Plugin(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
  delete p;
}



////////////////////////////
/// Class Initialization ///
////////////////////////////
Plugin::Plugin(void* ppimgr) : opencpn_plugin_120(ppimgr), wxEvtHandler()
{
  //Logo for plugin catalog (SVG only)
  wxString pluginFolder = GetPluginDataDir(PKG_NAME) + wxFileName::GetPathSeparator() + "data" + wxFileName::GetPathSeparator();
  g_pluginBitmap = GetBitmapFromSVGFile(pluginFolder + "icon_catalog.svg", 32, 32);

  // A flag used to indicate the toggled/untoggled state of the toolbar button
  isToolbarActive = false;
}

Plugin::~Plugin()
{
}

int Plugin::Init()
{
  //Load translations
  AddLocaleCatalog("opencpn-aislive_pi");

  //Init GUI
  parentWindow = GetOCPNCanvasWindow();
  configSettings = GetOCPNConfigObject();
  myGUI = NULL;

  //Settings
  LoadSettings();

  //Add the toolbar button (SVG only)
  wxString pluginFolder = GetPluginDataDir(PKG_NAME) + wxFileName::GetPathSeparator() + "data" + wxFileName::GetPathSeparator();
  wxString normalIcon   = pluginFolder + "icon_toolbar_default.svg";
  wxString rolloverIcon = pluginFolder + "icon_toolbar_toggled.svg"; //BUG : rollover state not working in OpenCPN
  wxString toggledIcon  = pluginFolder + "icon_toolbar_toggled.svg";
  toolbarId = InsertPlugInToolSVG("AisLive", normalIcon, rolloverIcon, toggledIcon, wxITEM_CHECK, "AisLive", "AisLive Plugin Toolbar", NULL, -1, 0, this);

  //Right-click menu entry
  wxMenu menu;
  menuID = AddCanvasContextMenuItem(new wxMenuItem(&menu, -1, _("AisLive...")), this);
  SetCanvasContextMenuItemViz(menuID, false);

  //Inform OpenCPN about the plugin capabilities and requested callbacks
  return  ( INSTALLS_TOOLBAR_TOOL  //Add toolbar icon
          | WANTS_PREFERENCES      //Add "Preferences" button in plugin catalogue
          | WANTS_CURSOR_LATLON    //Enable SetCursorLatLon()
          | WANTS_NMEA_EVENTS);    //Enable SetPositionFix
}

bool Plugin::DeInit()
{
  SaveSettings();

  if (myGUI != NULL)
  {
    myGUI->Close();
    delete myGUI;
    myGUI = NULL;

    isToolbarActive = false;
    SetToolbarItemState(toolbarId, false);
    SetCanvasContextMenuItemViz(menuID, false);
  }

  return true;
}



////////////////////////////////////////
/// OCPN Required Plugin Information ///
////////////////////////////////////////
int Plugin::GetAPIVersionMajor()
{
  return atoi(API_VERSION);
}

int Plugin::GetAPIVersionMinor()
{
  std::string v(API_VERSION);
  size_t dotpos = v.find('.');
  return atoi(v.substr(dotpos + 1).c_str());
}

int Plugin::GetPlugInVersionMajor()
{
  return PLUGIN_VERSION_MAJOR;
}

int Plugin::GetPlugInVersionMinor()
{
  return PLUGIN_VERSION_MINOR;
}

int Plugin::GetPlugInVersionPatch()
{
  return PLUGIN_VERSION_PATCH;
}

wxString Plugin::GetCommonName()
{
  return PLUGIN_API_NAME;
}

wxString Plugin::GetShortDescription()
{
  return PKG_SUMMARY;
}

wxString Plugin::GetLongDescription()
{
  return PKG_DESCRIPTION;
}

wxBitmap* Plugin::GetPlugInBitmap()
{
  return &g_pluginBitmap;
}

int Plugin::GetToolbarToolCount() {
  return 1;
}



////////////////
/// Settings ///
////////////////
void Plugin::LoadSettings()
{
  if (configSettings)
  {
    configSettings->SetPath("/PlugIns/AisLivePlugin");

    configSettings->Read("RestoreWindowSize", &g_restoreWindowSize, 1);
    configSettings->Read("WindowWidth", &g_windowWidth, 650);
    configSettings->Read("WindowHeight", &g_windowHeight, 400);

    configSettings->Read("RestoreWindowPos", &g_restoreWindowPos, 1);
    configSettings->Read("WindowPosX", &g_windowPosX, 0);
    configSettings->Read("WindowPosY", &g_windowPosY, 0);

    configSettings->Read("KeepWindowActive", &g_keepWindowActive, 0);
  }
}

void Plugin::SaveSettings()
{
  //Update settings variables
  if (myGUI != NULL)
  {
    wxSize size = myGUI->GetSize();
    g_windowWidth = size.GetWidth();
    g_windowHeight = size.GetHeight();

    wxPoint pos = myGUI->GetPosition();
    g_windowPosX = pos.x;
    g_windowPosY = pos.y;
  }

  if (configSettings)
  {
    configSettings->SetPath("/PlugIns/AisLivePlugin");

    configSettings->Write("RestoreWindowSize", g_restoreWindowSize);
    configSettings->Write("WindowWidth", g_windowWidth);
    configSettings->Write("WindowHeight", g_windowHeight);

    configSettings->Write("RestoreWindowPos", g_restoreWindowPos);
    configSettings->Write("WindowPosX", g_windowPosX);
    configSettings->Write("WindowPosY", g_windowPosY);

    configSettings->Write("KeepWindowActive", g_keepWindowActive);
  }
}


/////////////////////////////
/// OCPN Automatic Update ///
/////////////////////////////
void Plugin::SetCursorLatLon(double lat, double lon)
{
    m_cursor_lat = lat;
    m_cursor_lon = lon;
}

void Plugin::SetPositionFix(PlugIn_Position_Fix &pfix)
{
    m_boat_lat = pfix.Lat;
    m_boat_lon = pfix.Lon;

    if (myGUI != NULL)
        myGUI->updateBoatPosition(m_boat_lat, m_boat_lon);
}



/////////////////////////////////
/// OCPN Interactions Methods ///
/////////////////////////////////
void Plugin::ShowPreferencesDialog(wxWindow* parent)
{
  auto dialogSettings = std::make_unique<DialogSettings>(
    parent,
    wxID_ANY,
    _("AisLive Preferences"),
    wxDefaultPosition,
    wxDefaultSize,
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
  );

  if (wxID_OK == dialogSettings->ShowModal())
    SaveSettings();
}

void Plugin::OnToolbarToolCallback(int id)
{
  if (id != toolbarId)
    return;

  //Create GUI first time
  if(myGUI == NULL)
  {
    myGUI = new DialogMainGui(parentWindow);
    myGUI->plugin = this;

    if(g_restoreWindowSize)
      myGUI->SetSize(g_windowWidth, g_windowHeight);
    if(g_restoreWindowPos)
      myGUI->Move(wxPoint(g_windowPosX, g_windowPosY));
  }

  //Toggle UI & toolbar icon state
  isToolbarActive = !isToolbarActive;
  SetToolbarItemState(id, isToolbarActive);
  if (isToolbarActive)
  {
    myGUI->Show();
    myGUI->Raise();
    myGUI->SetFocus();
    SetCanvasContextMenuItemViz(menuID, true);
  }
  else
  {
    myGUI->Hide();

    if(!g_keepWindowActive)
    {
      SetCanvasContextMenuItemViz(menuID, false);
    }
  }

  //Refresh screen
  RequestRefresh(parentWindow);
}

void Plugin::OnContextMenuItemCallback(int id)
{
  if (myGUI == NULL)
    return;

  //Entry in right-click menu
  if(id == menuID)
  {
    DialogMenuEntry dlg(GetOCPNCanvasWindow());

    if(dlg.ShowModal() == wxID_OK)
    {
        switch(dlg.m_action)
        {
            case DialogAction::UpdateToCursor:
                myGUI->updateSearchPosition(m_cursor_lat, m_cursor_lon);
                break;

            case DialogAction::UpdateToBoat:
                myGUI->updateSearchPosition(m_boat_lat, m_boat_lon);
                break;

            case DialogAction::None:
                break;
        }


    }
  }
}


////////////////////
/// wxGUI Events ///
////////////////////
void Plugin::OnGuiClosed()
{
  isToolbarActive = false;
  SetToolbarItemState(toolbarId, false);
  myGUI->Hide();

  if(!g_keepWindowActive)
    SetCanvasContextMenuItemViz(menuID, false);

  //Refresh screen
  RequestRefresh(parentWindow);
}


void Plugin::sendNmeaSentence(wxString sentence)
{
    PushNMEABuffer(sentence); //using old API
}

