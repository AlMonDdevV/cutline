add_library(cutline_media STATIC
    FfmpegInfo.h
    FfmpegInfo.cpp
)

target_link_libraries(cutline_media PUBLIC cutline_core PRIVATE Cutline::FFmpeg)
cutline_set_warnings(cutline_media)
