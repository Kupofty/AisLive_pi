#include "global_settings.h"
#include <cstdint>


//Windows size & pos
bool g_restoreWindowSize = 1;
bool g_restoreWindowPos = 1;

int g_windowWidth = 0;
int g_windowHeight = 0;
int g_windowPosX = 0;
int g_windowPosY = 0;

bool g_keepWindowActive = 1;

double g_lastSearchBoxSize = 1; //degree

wxString g_openWatersToken = "";

std::vector<uint32_t> g_mmsiList = {};
std::vector<wxString> g_mmsiListNames = {};
bool g_activateVesselsFollowing = 1;
