cmake_minimum_required(VERSION 3.28)

project(Cutline
    VERSION 0.1.0
    DESCRIPTION "CUTLINE - video editing without the paywall"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CUTLINE_OFFICIAL_BUILD "Official download build: FFmpeg must be LGPL (no GPL, no nonfree)" OFF)
option(CUTLINE_WERROR "Treat compiler warnings as errors" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Quick QuickControls2 Svg Test)
qt_standard_project_setup(REQUIRES 6.8)

include(CutlineWarnings)
include(CutlineFFmpeg)
include(CTest)
include(CutlineTesting)

add_subdirectory(src/core)
add_subdirectory(src/media)
add_subdirectory(src/app)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
