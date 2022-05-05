# ~~~
# Summary:      Tests build script
# Copyright (c) 2022 Pavel Kalian
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required( VERSION 3.0 )

project( DashboardSKTests CXX )

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.0.0-preview5)

FetchContent_MakeAvailable(Catch2)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${Catch2_SOURCE_DIR}/extras")

set( SOURCES_TESTS
        001-DashboardSK.cpp
        002-Dashboard.cpp
        003-SimpleNumberInstrument.cpp
        opencpn_mock.cpp
        ${SRC_DASHBOARD} )

include_directories("${CMAKE_SOURCE_DIR}/include")

add_executable(tests ${SOURCES_TESTS})
if(WIN32)
    target_include_directories(tests PRIVATE "${CMAKE_SOURCE_DIR}/opencpn-libs/WindowsHeaders/include")
    #add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/plugingl")
    target_link_libraries(tests ocpn::plugingl)

    #add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxJSON")
    target_link_libraries(tests ocpn::wxjson)

    target_link_libraries(tests ${wxWidgets_LIBRARIES})
endif()
target_link_libraries(tests Catch2::Catch2WithMain)
add_subdirectory("opencpn-libs/plugingl")
target_link_libraries(tests ocpn::plugingl)
target_link_libraries(tests OpenGL::GL)
add_subdirectory("opencpn-libs/${PKG_API_LIB}")
target_link_libraries(tests ocpn::api)
add_subdirectory("opencpn-libs/wxJSON")
target_link_libraries(tests ocpn::wxjson)

include(CTest)
include(Catch)
catch_discover_tests(tests)

add_custom_command(TARGET tests PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tests/samples ${CMAKE_CURRENT_BINARY_DIR}/samples)
