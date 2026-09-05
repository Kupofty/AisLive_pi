#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

// wxWidgets Precompiled Headers
#include "wx/wxprec.h"

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

#endif //GLOBAL_SETTINGS_H
