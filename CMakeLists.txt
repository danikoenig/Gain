cmake_minimum_required(VERSION 3.22)

project(Gain VERSION 1.5.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: lokaler Pfad (-DJUCE_PATH=...) oder automatischer Download
if(DEFINED JUCE_PATH)
    add_subdirectory(${JUCE_PATH} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(JUCE)
endif()

# CLAP (Bitwig u.a.) ueber clap-juce-extensions
include(FetchContent)
FetchContent_Declare(
    clap-juce-extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG main
    GIT_SUBMODULES_RECURSE TRUE
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(clap-juce-extensions)

juce_add_plugin(Gain
    COMPANY_NAME "Dani Koenig"
    BUNDLE_ID "ch.danikoenig.gain"
    PLUGIN_MANUFACTURER_CODE Dkng
    PLUGIN_CODE Gan1
    FORMATS VST3 AU Standalone
    AU_MAIN_TYPE kAudioUnitType_Effect
    PRODUCT_NAME "Gain"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Tools)

target_sources(Gain PRIVATE
    Source/Engine.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Look.cpp)

target_compile_definitions(Gain PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Gain
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

clap_juce_extensions_plugin(TARGET Gain
    CLAP_ID "ch.danikoenig.gain"
    CLAP_FEATURES audio-effect utility stereo mono)
