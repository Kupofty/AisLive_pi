# ----------------------------------------- #
# Summary:      Local, generic plugin setup #
# Copyright (c) 2026 Kupofty                #
# License:      GPLv3+                      #
# ----------------------------------------- #

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------
set(OCPN_TEST_REPO
    "kupoftyopencpn/aislive-alpha"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "kupoftyopencpn/aislive-beta"
    CACHE STRING
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "kupoftyopencpn/aislive-prod"
    CACHE STRING
    "Default repository for tagged builds not matching 'beta'"
)


# ------- Plugin setup --------
set(PKG_NAME aislive_pi)
set(PKG_VERSION  0.0.1)
set(PKG_PRERELEASE "") # Empty, or a tag like 'beta'

set(DISPLAY_NAME AisLive)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME AisLive) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "Online AIS Stream")
set(PKG_DESCRIPTION "Connects to the OpenWaters.io AIS stream and fetches live AIS targets over the internet for display in OpenCPN")
set(PKG_AUTHOR "Kupofty")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/Kupofty/AisLive_pi)
set(PKG_INFO_URL https://github.com/Kupofty/AisLive_pi) #Used in the website button

# ------- Code sources --------
set(SRC
    # OCPN plugin
    ${CMAKE_SOURCE_DIR}/src/plugin/plugin.h
    ${CMAKE_SOURCE_DIR}/src/plugin/plugin.cpp

    # Main UI
    ${CMAKE_SOURCE_DIR}/src/main-ui/main_ui_base.h
    ${CMAKE_SOURCE_DIR}/src/main-ui/main_ui_base.cpp
    ${CMAKE_SOURCE_DIR}/src/main-ui/main_ui_derived.h
    ${CMAKE_SOURCE_DIR}/src/main-ui/main_ui_derived.cpp

    # Settings UI
    ${CMAKE_SOURCE_DIR}/src/settings/settings_ui_base.h
    ${CMAKE_SOURCE_DIR}/src/settings/settings_ui_base.cpp
    ${CMAKE_SOURCE_DIR}/src/settings/settings_ui_derived.h
    ${CMAKE_SOURCE_DIR}/src/settings/settings_ui_derived.cpp

    # Shared settings variables
    ${CMAKE_SOURCE_DIR}/src/settings/global_settings.h
    ${CMAKE_SOURCE_DIR}/src/settings/global_settings.cpp

    # Menu Dialog UI
    ${CMAKE_SOURCE_DIR}/src/menu-entry/menuDialog_ui_base.h
    ${CMAKE_SOURCE_DIR}/src/menu-entry/menuDialog_ui_base.cpp
    ${CMAKE_SOURCE_DIR}/src/menu-entry/menuDialog_ui_derived.h
    ${CMAKE_SOURCE_DIR}/src/menu-entry/menuDialog_ui_derived.cpp
)
set (SOURCE_FILES ${SRC})



# ------- Settings --------

# Set API version
set(PKG_API_LIB api-20)

# Late initialization
macro(late_init)
  if (APPLE)
    target_compile_definitions(${PACKAGE_NAME} PUBLIC OCPN_GHC_FILESYSTEM)
  endif ()
endmacro ()


# ------- Libraries --------

# Add plugin libraries
#macro(add_plugin_libraries)
    #add_subdirectory(opencpn-libs/nmea0183)
    #target_link_libraries(${PACKAGE_NAME} ocpn::nmea0183)
#endmacro ()
