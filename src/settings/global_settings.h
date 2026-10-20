#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

// wxWidgets Precompiled Headers
#include "wx/wxprec.h"
#include <cstdint>

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif


/////////////////////////////////////////////////////////////////////
/// Globally accessible variables used by the plugin, dialogs etc ///
/////////////////////////////////////////////////////////////////////

extern bool g_restoreWindowSize;
extern int g_windowWidth;
extern int g_windowHeight;

extern bool g_restoreWindowPos;
extern int g_windowPosX;
extern int g_windowPosY;

extern bool g_keepWindowActive;

extern double g_lastSearchBoxSize;

extern wxString g_openWatersToken;

extern std::vector<uint32_t> g_mmsiList;
extern std::vector<wxString> g_mmsiListNames;
extern bool g_activateVesselsFollowing;


#endif //GLOBAL_SETTINGS_H
