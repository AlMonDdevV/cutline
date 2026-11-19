qt_add_executable(cutline WIN32 MACOSX_BUNDLE main.cpp)

set_target_properties(cutline PROPERTIES
    OUTPUT_NAME cutline
    MACOSX_BUNDLE_BUNDLE_NAME CUTLINE
    MACOSX_BUNDLE_GUI_IDENTIFIER org.cutline.Cutline
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)

target_link_libraries(cutline PRIVATE cutline_ui cutline_uiplugin Qt6::Quick Qt6::QuickControls2)
cutline_set_warnings(cutline)
