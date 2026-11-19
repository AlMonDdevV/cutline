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

# Windows: bake the app icon from the SVG mark at build time and embed it in cutline.exe.
if(WIN32)
    set(CUTLINE_ICO "${CMAKE_CURRENT_BINARY_DIR}/cutline.ico")
    set(_mark "${PROJECT_SOURCE_DIR}/src/app/icons/cutline-mark.svg")
    get_target_property(_qmake Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin "${_qmake}" DIRECTORY)
    add_custom_command(
        OUTPUT "${CUTLINE_ICO}"
        COMMAND ${CMAKE_COMMAND} -E env --modify "PATH=path_list_prepend:${_qt_bin}"
                $<TARGET_FILE:iconbake> "${_mark}" "${CUTLINE_ICO}" "${CMAKE_CURRENT_BINARY_DIR}/icons"
        DEPENDS iconbake "${_mark}"
        COMMENT "Baking the CUTLINE app icon"
        VERBATIM)
    configure_file(cutline.rc.in "${CMAKE_CURRENT_BINARY_DIR}/cutline.rc" @ONLY)
    target_sources(cutline PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/cutline.rc" "${CUTLINE_ICO}")
    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/cutline.rc" PROPERTIES OBJECT_DEPENDS "${CUTLINE_ICO}")
endif()
