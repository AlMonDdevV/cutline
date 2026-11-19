cutline_add_test(NAME tst_time SOURCES core/tst_time.cpp LIBS cutline_core)

cutline_add_test(NAME tst_ffmpeginfo SOURCES media/tst_ffmpeginfo.cpp LIBS cutline_media)
if(CUTLINE_OFFICIAL_BUILD)
    target_compile_definitions(tst_ffmpeginfo PRIVATE CUTLINE_OFFICIAL_BUILD)
endif()

cutline_add_test(NAME tst_app_smoke SOURCES ui/tst_app_smoke.cpp LIBS cutline_ui cutline_uiplugin GUI)
