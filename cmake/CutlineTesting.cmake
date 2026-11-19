# cutline_add_test(NAME <name> SOURCES <files...> [LIBS <targets...>] [GUI])
#
# Builds a Qt Test executable and registers it with CTest under the same name.
# GUI tests run headless: offscreen platform plugin and the software Qt Quick backend.
# On Windows the Qt bin directory is prepended to PATH so the tests find Qt's DLLs.
function(cutline_add_test)
    cmake_parse_arguments(ARG "GUI" "NAME" "SOURCES;LIBS" ${ARGN})
    if(NOT ARG_NAME OR NOT ARG_SOURCES)
        message(FATAL_ERROR "cutline_add_test needs NAME and SOURCES")
    endif()

    qt_add_executable(${ARG_NAME} ${ARG_SOURCES})
    target_link_libraries(${ARG_NAME} PRIVATE Qt6::Test ${ARG_LIBS})
    cutline_set_warnings(${ARG_NAME})
    set_target_properties(${ARG_NAME} PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)

    add_test(NAME ${ARG_NAME} COMMAND ${ARG_NAME})

    set(env "")
    if(ARG_GUI)
        list(APPEND env "QT_QPA_PLATFORM=offscreen" "QT_QUICK_BACKEND=software")
    endif()
    if(env)
        set_tests_properties(${ARG_NAME} PROPERTIES ENVIRONMENT "${env}")
    endif()

    if(WIN32)
        get_target_property(_qmake Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(_qt_bin "${_qmake}" DIRECTORY)
        set_tests_properties(${ARG_NAME} PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${_qt_bin}")
    endif()
endfunction()
