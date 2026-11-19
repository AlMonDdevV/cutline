# Provides the INTERFACE target Cutline::FFmpeg (avformat, avcodec, avutil, swresample, swscale, avfilter).
#   - vcpkg builds (Windows, macOS): the LGPL FFmpeg from vcpkg.json, via vcpkg's FindFFMPEG wrapper.
#   - other builds (Linux distro packages): pkg-config.
if(TARGET Cutline::FFmpeg)
    return()
endif()

add_library(cutline_ffmpeg INTERFACE)

if(VCPKG_TOOLCHAIN)
    find_package(FFMPEG REQUIRED)
    target_include_directories(cutline_ffmpeg INTERFACE ${FFMPEG_INCLUDE_DIRS})
    if(FFMPEG_LIBRARY_DIRS)
        target_link_directories(cutline_ffmpeg INTERFACE ${FFMPEG_LIBRARY_DIRS})
    endif()
    target_link_libraries(cutline_ffmpeg INTERFACE ${FFMPEG_LIBRARIES})
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CUTLINE_FFMPEG REQUIRED IMPORTED_TARGET
        libavformat libavcodec libavutil libswresample libswscale libavfilter)
    target_link_libraries(cutline_ffmpeg INTERFACE PkgConfig::CUTLINE_FFMPEG)
endif()

add_library(Cutline::FFmpeg ALIAS cutline_ffmpeg)
