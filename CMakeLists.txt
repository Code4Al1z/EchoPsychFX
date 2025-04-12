cmake_minimum_required(VERSION 3.24)
project(EchoPsychFX VERSION 0.0.1) # Add this line back, in the correct spot

# JUCE module paths (ensure JUCE is correctly included)
add_subdirectory(modules/JUCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the plugin target
juce_add_plugin(EchoPsychFX
    COMPANY_NAME "Trailblaiz"
    PLUGIN_MANUFACTURER_CODE AL1Z
    PLUGIN_CODE AE01
    VERSION "${PROJECT_VERSION}" # use the project version variable.
    DESCRIPTION "Tricks your brain into feeling space, distance, emotion, and intimacy through carefully crafted phase, stereo, and EQ illusions."
    FORMATS AU VST3 AAX Standalone
    PRODUCT_NAME "EchoPsychFX"
)


# Globbing files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
)

target_sources(EchoPsychFX PRIVATE ${SOURCES})

# Include JUCE modules you need
target_link_libraries(EchoPsychFX PRIVATE
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_data_structures
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_graphics
    juce::juce_events
    juce::juce_opengl
)

# Set the standard C++ version (make sure to use at least C++17 for JUCE projects)
target_compile_features(EchoPsychFX PUBLIC cxx_std_17)

# Include JUCE modules you need
juce_generate_juce_header(EchoPsychFX)

# Specify the location of the plugin's resources (icons, etc.)
target_include_directories(EchoPsychFX PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
)
