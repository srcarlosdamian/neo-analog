cmake_minimum_required(VERSION 3.22)

# Set deployment target before project() for macOS
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum OS X deployment target")
endif()

project(LadderMono VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PRODUCT_NAME "Neo Analog" CACHE STRING "Plugin product name")
set(COMPANY_NAME "Neo" CACHE STRING "Manufacturer name")
set(PLUGIN_CODE "NeAn" CACHE STRING "4-character plugin code")
set(MANUFACTURER_CODE "NeoA" CACHE STRING "4-character manufacturer code")

# Add JUCE
add_subdirectory(libs/JUCE EXCLUDE_FROM_ALL)

# Add clap-juce-extensions
add_subdirectory(libs/clap-juce-extensions EXCLUDE_FROM_ALL)

# Target plugin formats
set(PLUGIN_FORMATS Standalone VST3)
if (APPLE)
    list(APPEND PLUGIN_FORMATS AU)
endif()

juce_add_plugin(LadderMono
    COMPANY_NAME "${COMPANY_NAME}"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE "${MANUFACTURER_CODE}"
    PLUGIN_CODE "${PLUGIN_CODE}"
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "${PRODUCT_NAME}"
    BUNDLE_ID "com.neoapps.neoanalog"
)

target_sources(LadderMono PRIVATE
    src/PluginProcessor.h
    src/PluginProcessor.cpp
    src/PluginEditor.h
    src/PluginEditor.cpp
    src/dsp/Oscillator.h
    src/dsp/Oscillator.cpp
    src/dsp/LadderFilter.h
    src/dsp/LadderFilter.cpp
    src/dsp/Envelope.h
    src/dsp/Glide.h
    src/dsp/NoiseGen.h
    src/dsp/Arpeggiator.h
    src/dsp/Voice.h
    src/dsp/Voice.cpp
    src/params/ParameterIDs.h
    src/params/ParameterLayout.cpp
    src/presets/PresetManager.h
    src/presets/PresetManager.cpp
    src/ui/MinimalistLookAndFeel.h
    src/ui/VirtualKeyboard.h
)

target_include_directories(LadderMono PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(LadderMono PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(LadderMono PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Enable CLAP target via clap-juce-extensions
if (COMMAND clap_juce_extensions_plugin)
    clap_juce_extensions_plugin(TARGET LadderMono
        CLAP_ID "com.yourname.laddermono"
    )
endif()

# Automated Test Target
add_executable(LadderMonoTests
    tests/TestMain.cpp
    src/dsp/Oscillator.h
    src/dsp/Oscillator.cpp
    src/dsp/LadderFilter.h
    src/dsp/LadderFilter.cpp
    src/dsp/Envelope.h
    src/dsp/Glide.h
    src/dsp/NoiseGen.h
    src/dsp/Arpeggiator.h
    src/dsp/Voice.h
    src/dsp/Voice.cpp
)

target_include_directories(LadderMonoTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(LadderMonoTests PRIVATE
    juce::juce_core
    juce::juce_recommended_config_flags
)

add_executable(TestPresetAudio
    tests/TestPresetAudio.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/params/ParameterLayout.cpp
    src/presets/PresetManager.cpp
    src/dsp/Oscillator.cpp
    src/dsp/LadderFilter.cpp
    src/dsp/Voice.cpp
)

target_include_directories(TestPresetAudio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(TestPresetAudio PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
)

add_executable(TestRelease
    tests/TestRelease.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/params/ParameterLayout.cpp
    src/presets/PresetManager.cpp
    src/dsp/Oscillator.cpp
    src/dsp/LadderFilter.cpp
    src/dsp/Voice.cpp
)

target_include_directories(TestRelease PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(TestRelease PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
)

add_executable(TestMarimba
    tests/TestMarimba.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/params/ParameterLayout.cpp
    src/presets/PresetManager.cpp
    src/dsp/Oscillator.cpp
    src/dsp/LadderFilter.cpp
    src/dsp/Voice.cpp
)

target_include_directories(TestMarimba PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(TestMarimba PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
)

add_executable(OfflineRenderer
    tools/OfflineRenderer.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/params/ParameterLayout.cpp
    src/presets/PresetManager.cpp
    src/dsp/Oscillator.cpp
    src/dsp/LadderFilter.cpp
    src/dsp/Voice.cpp
)

target_include_directories(OfflineRenderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(OfflineRenderer PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
)

enable_testing()
add_test(NAME AllTests COMMAND LadderMonoTests)
