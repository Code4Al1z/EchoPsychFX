cmake_minimum_required(VERSION 3.24)
project(EchoPsychFX VERSION 0.9.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(modules/JUCE)

juce_add_plugin(EchoPsychFX
    COMPANY_NAME "TrailblaiZ"
    PLUGIN_MANUFACTURER_CODE AL1Z
    PLUGIN_CODE AE01
    VERSION "${PROJECT_VERSION}"
    DESCRIPTION "Tricks your brain into feeling space, distance, emotion, and intimacy through carefully crafted phase, stereo, and EQ illusions."
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "EchoPsychFX"
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
)
target_sources(EchoPsychFX PRIVATE ${SOURCES})

target_link_libraries(EchoPsychFX PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_plugin_client
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_opengl
)

target_compile_definitions(EchoPsychFX PRIVATE
    JUCE_USE_OGGVORBIS=0
    JUCE_USE_FLAC=0
    JUCE_USE_MP3AUDIOFORMAT=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_features(EchoPsychFX PUBLIC cxx_std_17)

juce_generate_juce_header(EchoPsychFX)

# Optional developer tests (off by default so normal plugin builds are unchanged):
#   cmake -B build -DECHOPSYCHFX_BUILD_TESTS=ON && cmake --build build --target EchoPsychFXTests
option(ECHOPSYCHFX_BUILD_TESTS "Build the description-consistency test" OFF)
if(ECHOPSYCHFX_BUILD_TESTS)
    enable_testing()
    juce_add_console_app(EchoPsychFXTests PRODUCT_NAME "EchoPsychFXTests")
    target_sources(EchoPsychFXTests PRIVATE tests/DescriptionConsistencyTest.cpp)
    target_include_directories(EchoPsychFXTests PRIVATE src)
    target_link_libraries(EchoPsychFXTests PRIVATE
        EchoPsychFX
        juce::juce_audio_processors juce::juce_dsp juce::juce_gui_basics juce::juce_gui_extra
        juce::juce_data_structures juce::juce_audio_utils juce::juce_audio_devices
        juce::juce_recommended_config_flags)
    add_test(NAME DescriptionConsistency COMMAND EchoPsychFXTests)

    juce_add_console_app(EchoPsychFXMeasure PRODUCT_NAME "EchoPsychFXMeasure")
    target_sources(EchoPsychFXMeasure PRIVATE tools/calibration/MeasureSettings.cpp)
    target_include_directories(EchoPsychFXMeasure PRIVATE src)
    target_link_libraries(EchoPsychFXMeasure PRIVATE
        EchoPsychFX
        juce::juce_audio_processors juce::juce_dsp juce::juce_gui_basics juce::juce_gui_extra
        juce::juce_data_structures juce::juce_audio_utils juce::juce_audio_devices
        juce::juce_recommended_config_flags)
endif()
