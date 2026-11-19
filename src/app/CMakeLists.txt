add_library(cutline_ico STATIC icons/IcoWriter.h icons/IcoWriter.cpp)
target_include_directories(cutline_ico PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(cutline_ico PUBLIC Qt6::Gui)
cutline_set_warnings(cutline_ico)

qt_add_library(cutline_ui STATIC
    layout/WorkspaceLayout.h
    layout/WorkspaceLayout.cpp
    layout/PanelRegistry.h
    layout/PanelRegistry.cpp
    layout/WorkspaceManager.h
    layout/WorkspaceManager.cpp
    actions/Contexts.h
    actions/Action.h
    actions/Action.cpp
    actions/ActionRegistry.h
    actions/ActionRegistry.cpp
    actions/ShortcutProfile.h
    actions/ShortcutProfile.cpp
    actions/ShortcutRouter.h
    actions/ShortcutRouter.cpp
    actions/ShortcutRecorder.h
    actions/ShortcutRecorder.cpp
    icons/IconProvider.h
    icons/IconProvider.cpp
    theme/Palette.h
    theme/Palette.cpp
    theme/ThemeLibrary.h
    theme/ThemeLibrary.cpp
    theme/ThemeController.h
    theme/ThemeController.cpp
    theme/Fonts.h
    theme/Fonts.cpp
)

qt_add_qml_module(cutline_ui
    URI Cutline
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/controls/Icon.qml
)

qt_add_resources(cutline_ui "cutline_theme"
    PREFIX "/cutline/theme"
    BASE theme
    FILES
        theme/dark.json
        theme/light.json
)

file(GLOB _icon_svgs RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/icons/svg/*.svg")
qt_add_resources(cutline_ui "cutline_icons"
    PREFIX "/cutline/icons"
    BASE icons/svg
    FILES ${_icon_svgs}
)

set(_fonts_dir "${PROJECT_SOURCE_DIR}/resources/fonts")
qt_add_resources(cutline_ui "cutline_fonts"
    PREFIX "/cutline/fonts"
    BASE "${_fonts_dir}"
    FILES
        "${_fonts_dir}/Inter-Regular.ttf"
        "${_fonts_dir}/Inter-Medium.ttf"
        "${_fonts_dir}/Inter-SemiBold.ttf"
        "${_fonts_dir}/JetBrainsMono-Regular.ttf"
        "${_fonts_dir}/JetBrainsMono-Medium.ttf"
)

target_link_libraries(cutline_ui
    PUBLIC
        cutline_core
        cutline_media
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Svg
        cutline_ico
)
# qmltyperegistrar includes QML_ELEMENT headers by bare file name.
target_include_directories(cutline_ui PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/actions"
    "${CMAKE_CURRENT_SOURCE_DIR}/controllers"
    "${CMAKE_CURRENT_SOURCE_DIR}/layout"
    "${CMAKE_CURRENT_SOURCE_DIR}/theme"
    "${CMAKE_CURRENT_SOURCE_DIR}/icons"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
cutline_set_warnings(cutline_ui)

add_subdirectory(main)
