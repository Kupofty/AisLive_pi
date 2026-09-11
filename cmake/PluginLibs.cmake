# ~~~
# Summary:      Find and link general plugin libraries
# License:      GPLv3+
# Copyright (c) 2021 Alec Leamas
#
# Find and link general libraries to use: gettext, wxWidgets and OpenGL
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

find_package(Gettext REQUIRED)

#
# Windows environment check
#
set(_bad_win_env_msg [=[
%WXWIN% is not present in environment, win_deps.bat has not been run.
Build might work, but most likely fail when not finding wxWidgets.
Run buildwin\win_deps.bat or set %WXWIN% to mute this message.
]=])

if (WIN32 AND NOT DEFINED ENV{WXWIN})
  message(WARNING ${_bad_win_env_msg})
endif ()

#
# OpenGL
#
# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")

find_package(OpenGL)
if (TARGET OpenGL::GL)
  target_link_libraries(${PACKAGE_NAME} OpenGL::GL)
else ()
  message(WARNING "Cannot locate usable OpenGL libs and headers.")
endif ()
if (NOT OPENGL_GLU_FOUND)
  message(WARNING "Cannot find OpenGL GLU extension.")
endif ()
if (APPLE)
  # As of 3.19.2, cmake's FindOpenGL does not link to the directory
  # containing gl.h. cmake bug? Intended due to missing subdir GL/gl.h?
  find_path(GL_H_DIR NAMES gl.h)
  if (GL_H_DIR)
    target_include_directories(${PACKAGE_NAME} PRIVATE "${GL_H_DIR}")
  else ()
    message(WARNING "Cannot locate OpenGL header file gl.h")
  endif ()
endif ()
if (WIN32)
  if (EXISTS "${PROJECT_SOURCE_DIR}/opencpn-libs/WindowsHeaders")
    add_subdirectory("${PROJECT_SOURCE_DIR}/opencpn-libs/WindowsHeaders")
    target_link_libraries(${PACKAGE_NAME} windows::headers)
  else ()
    message(STATUS
      "WARNING: WindowsHeaders library is missing, OpenGL unavailable"
    )
  endif ()
endif ()

#
# wxWidgets
#
set(wxWidgets_USE_DEBUG OFF)
set(wxWidgets_USE_UNICODE ON)
set(wxWidgets_USE_UNIVERSAL OFF)
set(wxWidgets_USE_STATIC OFF)

set(WX_COMPONENTS base core net xml html adv stc aui)
if (TARGET OpenGL::OpenGL OR TARGET OpenGL::GL)
  list(APPEND WX_COMPONENTS gl)
endif ()

find_package(wxWidgets REQUIRED ${WX_COMPONENTS})
include(${wxWidgets_USE_FILE})
target_link_libraries(${PACKAGE_NAME} ${wxWidgets_LIBRARIES})

#
# OpenSSL (TLS transport for the AIS websocket client)
#
# NOTE: OpenSSL is linked dynamically here (default FindOpenSSL behavior).
# Do NOT switch this to OPENSSL_USE_STATIC_LIBS on Windows: OpenCPN plugins
# are loaded/unloaded at runtime via LoadLibrary/FreeLibrary, and OpenSSL
# 1.1.x keeps process-global state (RNG pool, locks, ENGINE tables). A
# statically-linked copy inside plugin.dll creates a second, independent
# copy of that global state alongside whatever OpenCPN core / other
# plugins already loaded, which caused hard crashes in testing. Dynamic
# linking keeps a single shared OpenSSL instance in the process, which is
# the safe configuration - we just need to make sure the DLL is present
# on the end user's machine (see the WIN32 install() block below).
#
find_package(OpenSSL REQUIRED)
target_link_libraries(${PACKAGE_NAME} OpenSSL::SSL OpenSSL::Crypto)

#
# Bundle the OpenSSL runtime DLLs on Windows.
#
# find_package(OpenSSL) on the AppVeyor image resolves against a
# pre-installed OpenSSL (currently C:/OpenSSL-Win32, version 1.1.1w) that
# is NOT present on end-user machines. Since the plugin links dynamically
# against it, libssl-*.dll / libcrypto-*.dll must ship next to the plugin
# binary itself, in the same flat "plugins" destination used by
# PluginInstall.cmake, or the plugin fails/crashes at load time on users'
# systems that don't happen to already have a compatible OpenSSL DLL
# elsewhere in the OpenCPN plugins search path.
#
# Globbing (rather than hardcoding libssl-1_1.dll / libcrypto-1_1.dll)
# means a future OpenSSL version change on the CI image (e.g. a bump to
# the 3.x series) is picked up automatically instead of silently
# resolving to nothing.
#
if (WIN32)
  if (EXISTS "${PROJECT_SOURCE_DIR}/opencpn-libs/WindowsHeaders")
    add_subdirectory("${PROJECT_SOURCE_DIR}/opencpn-libs/WindowsHeaders")
    target_link_libraries(${PACKAGE_NAME} windows::headers)
  else ()
    message(STATUS
      "WARNING: WindowsHeaders library is missing, OpenGL unavailable"
    )
  endif ()
  target_link_libraries(${PACKAGE_NAME} ws2_32)

  if (NOT DEFINED OPENSSL_ROOT_DIR OR OPENSSL_ROOT_DIR STREQUAL "")
    get_filename_component(OPENSSL_ROOT_DIR "${OPENSSL_INCLUDE_DIR}/.." ABSOLUTE)
    message(STATUS "OPENSSL_ROOT_DIR not set by FindOpenSSL, derived as ${OPENSSL_ROOT_DIR}")
  endif ()

  file(GLOB _openssl_dlls
    "${OPENSSL_ROOT_DIR}/bin/libssl-*.dll"
    "${OPENSSL_ROOT_DIR}/bin/libcrypto-*.dll"
  )

  if (NOT _openssl_dlls)
    message(FATAL_ERROR
      "OpenSSL runtime DLLs not found under ${OPENSSL_ROOT_DIR}/bin - "
      "the plugin would fail/crash on end-user machines without them."
    )
  endif ()

  message(STATUS "Bundling OpenSSL runtime DLLs: ${_openssl_dlls}")
  install(FILES ${_openssl_dlls} DESTINATION "plugins")
endif ()