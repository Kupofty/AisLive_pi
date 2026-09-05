#ifndef PLUGIN_H
#define PLUGIN_H


///////////////
/// Headers ///
///////////////

// Pre compiled headers 
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

// wxWidgets
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/fileconf.h>

// Defines version numbers, names etc. for this plugin
// This is automagically constructed via config.h.in from CMakeLists.txt
#include "config.h"

// OpenCPN Plugin header
#include "ocpn_plugin.h"

// Bundled OpenCPN NMEA 0183 libraries
//#include "nmea0183.h"

//GUI
#include "main-ui/main_ui_derived.h"
#include "menu-entry/menuDialog_ui_derived.h"



///////////////
/// Classes ///
//////////////

//Main plugin class
class Plugin : public opencpn_plugin_120, public wxEvtHandler
{
  public:
    // Constructor
    Plugin(void* ppimgr);
    ~Plugin();
    int Init() override;
    bool DeInit() override;

    //Plugin gets
    int GetAPIVersionMajor() override;
    int GetAPIVersionMinor() override;
    int GetPlugInVersionMajor() override;
    int GetPlugInVersionMinor() override;
    int GetPlugInVersionPatch() override;
    wxString GetCommonName() override;
    wxString GetShortDescription() override;
    wxString GetLongDescription() override;
    wxBitmap* GetPlugInBitmap() override;
    int GetToolbarToolCount() override;

    //OCPN Interactions
    void ShowPreferencesDialog(wxWindow* parent) override;
    void OnToolbarToolCallback(int id) override;
    void OnContextMenuItemCallback(int id) override;

    //Main GUI wxEvent
    void OnGuiClosed();

  private:
    //GUI
    DialogMainGui *myGUI;
    wxWindow *parentWindow;
    wxFileConfig *configSettings;

    //Settings
    void LoadSettings();
    void SaveSettings();

    // Plugin icon
    wxBitmap g_pluginBitmap;

    // Toolbar button Id & state
    int toolbarId;
    bool isToolbarActive;
    int menuID;
};


#endif //PLUGIN_H

