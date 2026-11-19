qt_add_library(cutline_ui STATIC
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
)

qt_add_resources(cutline_ui "cutline_theme"
    PREFIX "/cutline/theme"
    BASE theme
    FILES
        theme/dark.json
        theme/light.json
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
)
cutline_set_warnings(cutline_ui)

add_subdirectory(main)
