# ~~~
# Summary:      Local, non-generic plugin setup
# Copyright (c) 2020-2021 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------

set(OCPN_TEST_REPO
        "nohal/opencpn-plugins"
        CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
        "nohal/opencpn-plugins"
        CACHE STRING
        "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
        "nohal/opencpn-plugins"
        CACHE STRING
        "Default repository for tagged builds not matching 'beta'"
)

#
#
# -------  Plugin setup --------
#
set(PKG_NAME DashboardSK_pi)
set(PKG_VERSION  0.0.1)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME DashboardSK)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME DashboardSK) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "DashboardSK plugin for OpenCPN")
set(PKG_DESCRIPTION [=[
DashboardSK plugin for OpenCPN
]=])

set(PKG_AUTHOR "Pavel Kalian")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/nohal/dashboardsk_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/dashboardsk.html)

add_definitions(-DDASHBOARDSK_USE_SVG)
add_definitions(-DocpnUSE_GL)

set(HDR_DASHBOARD
        ${CMAKE_SOURCE_DIR}/include/dashboardsk.h
        ${CMAKE_SOURCE_DIR}/include/dashboard.h
        ${CMAKE_SOURCE_DIR}/include/instrument.h
        ${CMAKE_SOURCE_DIR}/include/simplenumberinstrument.h
        ${CMAKE_SOURCE_DIR}/include/simplegaugeinstrument.h
        ${CMAKE_SOURCE_DIR}/include/simpletextinstrument.h
        ${CMAKE_SOURCE_DIR}/include/simplepositioninstrument.h
        ${CMAKE_SOURCE_DIR}/include/zone.h
        ${CMAKE_SOURCE_DIR}/include/displayscale.h
        )
set(SRC_DASHBOARD
        ${CMAKE_SOURCE_DIR}/src/dashboardsk.cpp
        ${CMAKE_SOURCE_DIR}/src/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/instrument.cpp
        ${CMAKE_SOURCE_DIR}/src/simplenumberinstrument.cpp
        ${CMAKE_SOURCE_DIR}/src/simplegaugeinstrument.cpp
        ${CMAKE_SOURCE_DIR}/src/simpletextinstrument.cpp
        ${CMAKE_SOURCE_DIR}/src/simplepositioninstrument.cpp
        )


set(SRC
        ${HDR_DASHBOARD}
        ${SRC_DASHBOARD}
        ${CMAKE_SOURCE_DIR}/include/dashboardsk_pi.h
        ${CMAKE_SOURCE_DIR}/src/dashboardsk_pi.cpp
        ${CMAKE_SOURCE_DIR}/include/dashboardskgui.h
        ${CMAKE_SOURCE_DIR}/src/dashboardskgui.cpp
        ${CMAKE_SOURCE_DIR}/include/dashboardskguiimpl.h
        ${CMAKE_SOURCE_DIR}/src/dashboardskguiimpl.cpp
)

set(PKG_API_LIB api-18)  #  A dir in opencpn-libs/ e. g., api-17 or api-16

macro(late_init)
        # Perform initialization after the PACKAGE_NAME library, compilers
        # and ocpn::api is available.

        # Fix OpenGL deprecated warnings in Xcode
        target_compile_definitions(${PACKAGE_NAME} PRIVATE GL_SILENCE_DEPRECATION)
        # Prepare doxygen config
        configure_file(${CMAKE_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile)
        configure_file(${CMAKE_SOURCE_DIR}/doc/header.html.in ${CMAKE_BINARY_DIR}/header.html)
        # Prepare asciidoxy
        configure_file(${CMAKE_SOURCE_DIR}/doc/api.adoc.in ${CMAKE_BINARY_DIR}/api.adoc @ONLY)
        configure_file(${CMAKE_SOURCE_DIR}/doc/packages.toml ${CMAKE_BINARY_DIR}/packages.toml)
        configure_file(${CMAKE_SOURCE_DIR}/doc/contents.toml ${CMAKE_BINARY_DIR}/contents.toml)
endmacro()

macro(add_plugin_libraries)
        # Add libraries required by this plugin
        add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/plugingl")
        target_link_libraries(${PACKAGE_NAME} ocpn::plugingl)

        add_subdirectory("opencpn-libs/wxJSON")
        target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)
endmacro()
