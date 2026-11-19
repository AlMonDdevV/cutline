qt_add_executable(iconbake main.cpp)
target_link_libraries(iconbake PRIVATE cutline_ico Qt6::Svg)
set_target_properties(iconbake PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
cutline_set_warnings(iconbake)
