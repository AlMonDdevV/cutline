configure_file(Version.h.in "${CMAKE_CURRENT_BINARY_DIR}/generated/cutline/Version.h" @ONLY)

add_library(cutline_core STATIC
    Result.h
    Time.h
    Time.cpp
)

target_include_directories(cutline_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
)
target_link_libraries(cutline_core PUBLIC Qt6::Core)
cutline_set_warnings(cutline_core)
