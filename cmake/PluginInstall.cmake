# ~~~
# Summary       Installation items and layout
# Author:       Pavel Kalian (Based on the work of Sean D'Epagnier)
# License:      GPLv3+
# Copyright (c) 2014 Pavel Kallian
#               2021 Alec Leamas
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

include(Metadata)

if (APPLE)
  install(
    TARGETS ${PACKAGE_NAME}
    RUNTIME LIBRARY DESTINATION OpenCPN.app/Contents/PlugIns
  )
  if (EXISTS ${PROJECT_SOURCE_DIR}/data)
    install(
      DIRECTORY data
      DESTINATION OpenCPN.app/Contents/SharedSupport/plugins/${PACKAGE_NAME}
    )
  endif ()

elseif (WIN32)
  message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
  if (CMAKE_CROSSCOMPILING)
    install(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION "plugins")
  else ()
    install(TARGETS ${PACKAGE_NAME} RUNTIME DESTINATION "plugins")
  endif ()
  if (EXISTS ${PROJECT_SOURCE_DIR}/data)
    install(DIRECTORY data DESTINATION "plugins/${PACKAGE_NAME}")
  endif ()

  # OpenSSL is linked dynamically on Windows (IXWebSocket's TLS backend
  # there); there's no system OpenSSL to rely on like on Linux, so the
  # matching DLLs have to ship next to the plugin. OPENSSL_INCLUDE_DIR is
  # a cache variable set by FindOpenSSL and stays visible here even though
  # find_package(OpenSSL) itself ran deep inside IXWebSocket's CMakeLists.
  if (DEFINED OPENSSL_INCLUDE_DIR)
    if (DEFINED OPENSSL_ROOT_DIR)
      set(_openssl_root "${OPENSSL_ROOT_DIR}")
    else ()
      get_filename_component(_openssl_root "${OPENSSL_INCLUDE_DIR}/.." ABSOLUTE)
    endif ()
    file(GLOB _openssl_dlls
      "${_openssl_root}/bin/libcrypto*.dll"
      "${_openssl_root}/bin/libssl*.dll"
    )
    if (_openssl_dlls)
      message(STATUS "Packaging OpenSSL DLLs: ${_openssl_dlls}")
      install(FILES ${_openssl_dlls} DESTINATION "plugins")
    else ()
      message(WARNING "OpenSSL found at ${_openssl_root} but no DLLs matched "
                       "${_openssl_root}/bin/lib{crypto,ssl}*.dll -- the "
                       "plugin will fail to load without them at runtime.")
    endif ()
  endif ()

elseif (UNIX)
  install(
    TARGETS ${PACKAGE_NAME}
    RUNTIME LIBRARY DESTINATION lib/opencpn
  )
  if (EXISTS ${PROJECT_SOURCE_DIR}/data)
    install(DIRECTORY data DESTINATION share/opencpn/plugins/${PACKAGE_NAME})
  endif ()
endif ()

# Hardcoded, absolute destination for tarball generation
if (${BUILD_TYPE} STREQUAL "tarball" OR ${BUILD_TYPE} STREQUAL "flatpak")
  install(CODE "
    configure_file(
      ${CMAKE_BINARY_DIR}/${pkg_displayname}.xml.in
      ${CMAKE_BINARY_DIR}/app/files/metadata.xml
      @ONLY
    )
  ")
endif()

# On macos, fix paths which points to the build environment, make sure they
# refers to runtime locations
if (${BUILD_TYPE} STREQUAL "tarball" AND APPLE)
  install(CODE
    "execute_process(
      COMMAND bash -c ${PROJECT_SOURCE_DIR}/cmake/fix-macos-libs.sh
    )"
  )
endif()

if (CMAKE_BUILD_TYPE MATCHES "Release|MinSizeRel")
  if (APPLE)
    set(_striplib OpenCPN.app/Contents/PlugIns/lib${PACKAGE_NAME}.dylib)
  elseif (MINGW)
    set(_striplib plugins/lib${PACKAGE_NAME}.dll)
  elseif (UNIX AND NOT CMAKE_CROSSCOMPILING AND NOT DEFINED ENV{FLATPAK_ID})
    # Plain, native linux
    set(_striplib lib/opencpn/lib${PACKAGE_NAME}.so)
  endif ()
  if (BUILD_TYPE STREQUAL "tarball" AND DEFINED _striplib)
    find_program(STRIP_UTIL NAMES strip REQUIRED)
    if (APPLE)
      set(STRIP_UTIL "${STRIP_UTIL} -x")
    endif ()
    install(CODE "message(STATUS \"Stripping ${_striplib}\")")
    install(CODE "
      execute_process(
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${STRIP_UTIL} app/files/${_striplib}
      )
    ")
  endif ()
endif ()