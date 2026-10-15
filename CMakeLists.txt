cmake_minimum_required(VERSION 3.22)

# Set deployment target before project() for macOS
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum OS X deployment target")
endif()

project(NeoAnalog VERSION 1.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PRODUCT_NAME "Neo Analog")
set(COMPANY_NAME "Neo")
set(PLUGIN_CODE "NeAn")
set(MANUFACTURER_CODE "NeoA")

# Add JUCE
add_subdirectory(libs/JUCE EXCLUDE_FROM_ALL)

# Add clap-juce-extensions
add_subdirectory(libs/clap-juce-extensions EXCLUDE_FROM_ALL)

# Target plugin formats
set(PLUGIN_FORMATS Standalone VST3)
if (APPLE)
    list(APPEND PLUGIN_FORMATS AU)
endif()

juce_add_plugin(NeoAnalog
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

target_sources(NeoAnalog PRIVATE
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

target_include_directories(NeoAnalog PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(NeoAnalog PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(NeoAnalog PUBLIC
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
    clap_juce_extensions_plugin(TARGET NeoAnalog
        CLAP_ID "com.neoapps.neoanalog"
    )
endif()

# Automated Test Target
add_executable(NeoAnalogTests
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

target_include_directories(NeoAnalogTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(NeoAnalogTests PRIVATE
    juce::juce_core
    juce::juce_recommended_config_flags
)

add_executable(TestPresetAudio
    tests/TestPresetAudio.cpp
)
target_include_directories(TestPresetAudio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(TestPresetAudio PRIVATE
    NeoAnalog
)

add_executable(TestRelease
    tests/TestRelease.cpp
)
target_include_directories(TestRelease PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(TestRelease PRIVATE
    NeoAnalog
)

add_executable(TestMarimba
    tests/TestMarimba.cpp
)
target_include_directories(TestMarimba PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(TestMarimba PRIVATE
    NeoAnalog
)

add_executable(TestAudit
    tests/TestAudit.cpp
)
target_include_directories(TestAudit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(TestAudit PRIVATE
    NeoAnalog
)

add_executable(OfflineRenderer
    tools/OfflineRenderer.cpp
)
target_include_directories(OfflineRenderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(OfflineRenderer PRIVATE
    NeoAnalog
    juce::juce_audio_formats
)

add_executable(TestKeyHandling
    tests/TestKeyHandling.cpp
)
target_include_directories(TestKeyHandling PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(TestKeyHandling PRIVATE
    NeoAnalog
)

enable_testing()
add_test(NAME AllTests COMMAND NeoAnalogTests)
add_test(NAME KeyHandlingTest COMMAND TestKeyHandling)
add_test(NAME AuditTest COMMAND TestAudit)
