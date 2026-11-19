qt_add_library(cutline_ui STATIC)

qt_add_qml_module(cutline_ui
    URI Cutline
    VERSION 1.0
    QML_FILES
        qml/Main.qml
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
