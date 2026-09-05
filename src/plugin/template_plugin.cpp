//////////////////////////////////////
/// Project:  OpenCPN              ///
/// Plugin:   Template             ///
/// Author:   Kupofty              ///
//////////////////////////////////////


///////////////////////
/// Include Headers ///
///////////////////////
#include "template_plugin.h"
#include "settings/global_settings.h"
#include "settings/settings_ui_derived.h"



///////////////////////
/// Class Factories ///
///////////////////////
extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr) {
  return new TemplatePlugin(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
  delete p;
}



////////////////////////////
/// Class Initialization ///
////////////////////////////
TemplatePlugin::TemplatePlugin(void* ppimgr) : opencpn_plugin_120(ppimgr), wxEvtHandler()
{
  //Logo for plugin catalog (SVG only)
  wxString pluginFolder = GetPluginDataDir(PKG_NAME) + wxFileName::GetPathSeparator() + "data" + wxFileName::GetPathSeparator();
  g_pluginBitmap = GetBitmapFromSVGFile(pluginFolder + "icon_template_catalog.svg", 32, 32);

  // A flag used to indicate the toggled/untoggled state of the toolbar button
  isToolbarActive = false;
}

TemplatePlugin::~TemplatePlugin()
{
}

int TemplatePlugin::Init()
{
  //Load translations
  AddLocaleCatalog("opencpn-template_pi");

  //Init GUI
  parentWindow = GetOCPNCanvasWindow();
  configSettings = GetOCPNConfigObject();
  myGUI = NULL;

  //Settings
  LoadSettings();

  //Add the toolbar button (SVG only)
  wxString pluginFolder = GetPluginDataDir(PKG_NAME) + wxFileName::GetPathSeparator() + "data" + wxFileName::GetPathSeparator();
  wxString normalIcon   = pluginFolder + "icon_template_toolbar_default.svg";
  wxString rolloverIcon = pluginFolder + "icon_template_toolbar_toggled.svg"; //BUG : rollover state not working in OpenCPN
  wxString toggledIcon  = pluginFolder + "icon_template_toolbar_toggled.svg";
  toolbarId = InsertPlugInToolSVG("Template", normalIcon, rolloverIcon, toggledIcon, wxITEM_CHECK, "Template", "Template Plugin Toolbar", NULL, -1, 0, this);

  //Right-click menu entry
  wxMenu templateMenu;
  templateMenuID = AddCanvasContextMenuItem(new wxMenuItem(&templateMenu, -1, _("Template...")), this);
  SetCanvasContextMenuItemViz(templateMenuID, false);

  //Inform OpenCPN about the plugin capabilities and requested callbacks
  return  ( INSTALLS_TOOLBAR_TOOL  //Add toolbar icon
          | WANTS_PREFERENCES      //Add "Preferences" button in plugin catalogue
          | WANTS_CURSOR_LATLON);  //Enable SetCursorLatLon()
}

bool TemplatePlugin::DeInit()
{
  SaveSettings();

  if (myGUI != NULL)
  {
    myGUI->Close();
    delete myGUI;
    myGUI = NULL;

    isToolbarActive = false;
    SetToolbarItemState(toolbarId, false);
    SetCanvasContextMenuItemViz(templateMenuID, false);
  }

  return true;
}



////////////////////////////////////////
/// OCPN Required Plugin Information ///
////////////////////////////////////////
int TemplatePlugin::GetAPIVersionMajor()
{
  return atoi(API_VERSION);
}

int TemplatePlugin::GetAPIVersionMinor()
{
  std::string v(API_VERSION);
  size_t dotpos = v.find('.');
  return atoi(v.substr(dotpos + 1).c_str());
}

int TemplatePlugin::GetPlugInVersionMajor()
{
  return PLUGIN_VERSION_MAJOR;
}

int TemplatePlugin::GetPlugInVersionMinor()
{
  return PLUGIN_VERSION_MINOR;
}

int TemplatePlugin::GetPlugInVersionPatch()
{
  return PLUGIN_VERSION_PATCH;
}

wxString TemplatePlugin::GetCommonName()
{
  return PLUGIN_API_NAME;
}

wxString TemplatePlugin::GetShortDescription()
{
  return PKG_SUMMARY;
}

wxString TemplatePlugin::GetLongDescription()
{
  return PKG_DESCRIPTION;
}

wxBitmap* TemplatePlugin::GetPlugInBitmap()
{
  return &g_pluginBitmap;
}

int TemplatePlugin::GetToolbarToolCount() {
  return 1;
}



////////////////
/// Settings ///
////////////////
void TemplatePlugin::LoadSettings()
{
  if (configSettings)
  {
    configSettings->SetPath("/PlugIns/TemplatePlugin");

    configSettings->Read("RestoreWindowSize", &g_restoreWindowSize, 1);
    configSettings->Read("WindowWidth", &g_windowWidth, 650);
    configSettings->Read("WindowHeight", &g_windowHeight, 400);

    configSettings->Read("RestoreWindowPos", &g_restoreWindowPos, 1);
    configSettings->Read("WindowPosX", &g_windowPosX, 0);
    configSettings->Read("WindowPosY", &g_windowPosY, 0);

    configSettings->Read("KeepWindowActive", &g_keepWindowActive, 0);
  }
}

void TemplatePlugin::SaveSettings()
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
    configSettings->SetPath("/PlugIns/TemplatePlugin");

    configSettings->Write("RestoreWindowSize", g_restoreWindowSize);
    configSettings->Write("WindowWidth", g_windowWidth);
    configSettings->Write("WindowHeight", g_windowHeight);

    configSettings->Write("RestoreWindowPos", g_restoreWindowPos);
    configSettings->Write("WindowPosX", g_windowPosX);
    configSettings->Write("WindowPosY", g_windowPosY);

    configSettings->Write("KeepWindowActive", g_keepWindowActive);
  }
}




/////////////////////////////////
/// OCPN Interactions Methods ///
/////////////////////////////////
void TemplatePlugin::ShowPreferencesDialog(wxWindow* parent)
{
  auto dialogSettings = std::make_unique<DialogSettings>(
    parent,
    wxID_ANY,
    _("Template Preferences"),
    wxDefaultPosition,
    wxDefaultSize,
    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
  );

  if (wxID_OK == dialogSettings->ShowModal())
    SaveSettings();
}

void TemplatePlugin::OnToolbarToolCallback(int id)
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
    SetCanvasContextMenuItemViz(templateMenuID, true);
  }
  else
  {
    myGUI->Hide();

    if(!g_keepWindowActive)
    {
      SetCanvasContextMenuItemViz(templateMenuID, false);
    }
  }

  //Refresh screen
  RequestRefresh(parentWindow);
}

void TemplatePlugin::OnContextMenuItemCallback(int id)
{
  if (myGUI == NULL)
    return;

  //Entry in right-click menu
  if(id == templateMenuID)
  {
    DialogMenuEntry dlg(GetOCPNCanvasWindow());

    if(dlg.ShowModal() == wxID_OK)
    {
        //do something
    }
  }
}



////////////////////
/// wxGUI Events ///
////////////////////
void TemplatePlugin::OnGuiClosed()
{
  isToolbarActive = false;
  SetToolbarItemState(toolbarId, false);
  myGUI->Hide();

  if(!g_keepWindowActive)
    SetCanvasContextMenuItemViz(templateMenuID, false);

  //Refresh screen
  RequestRefresh(parentWindow);
}

