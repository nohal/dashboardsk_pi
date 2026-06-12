# ~~~
# Summary:      Tests build script
# Copyright (c) 2022 Pavel Kalian
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.

cmake_minimum_required(VERSION 3.12.0)

project(DashboardSKTests CXX)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.7.1)

FetchContent_MakeAvailable(Catch2)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${Catch2_SOURCE_DIR}/extras")

set(SOURCES_TESTS
    001-DashboardSK.cpp
    002-Dashboard.cpp
    003-SimpleNumberInstrument.cpp
    004-DividerInstrument.cpp
    005-SpacerInstrument.cpp
    opencpn_mock.cpp
    ${SRC_DASHBOARD})

include_directories("${CMAKE_SOURCE_DIR}/include")

add_executable(tests ${SOURCES_TESTS})
if(WIN32)
  target_include_directories(
    tests PRIVATE "${CMAKE_SOURCE_DIR}/opencpn-libs/WindowsHeaders/include")
endif()
target_link_libraries(tests Catch2::Catch2WithMain)
# The libraries are normally already added by the plugin build itself, only add
# them when configuring the tests standalone
if(NOT TARGET ocpn::plugin-dc)
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/plugin_dc"
                   "${CMAKE_CURRENT_BINARY_DIR}/plugin_dc")
endif()
target_link_libraries(tests ocpn::plugin-dc)
target_link_libraries(tests OpenGL::GL)
if(NOT TARGET ocpn::api)
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/${PKG_API_LIB}"
                   "${CMAKE_CURRENT_BINARY_DIR}/${PKG_API_LIB}")
endif()
target_link_libraries(tests ocpn::api)
if(NOT TARGET ocpn::wxjson)
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxJSON"
                   "${CMAKE_CURRENT_BINARY_DIR}/wxJSON")
endif()
target_link_libraries(tests ocpn::wxjson)
target_link_libraries(tests ${wxWidgets_LIBRARIES})
if(GIOMM_FOUND)
  target_link_libraries(tests ${GIOMM_LIBRARIES})
endif()

include(CTest)
include(Catch)
catch_discover_tests(tests)

add_custom_command(
  TARGET tests
  PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tests/samples
          ${CMAKE_CURRENT_BINARY_DIR}/samples)
