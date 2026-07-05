cmake_minimum_required(VERSION 3.24)
project(EchoPsychFX VERSION 0.0.1)
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
    FORMATS AU VST3 AAX Standalone
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

target_include_directories(EchoPsychFX PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
)