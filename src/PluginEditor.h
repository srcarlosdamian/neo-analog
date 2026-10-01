#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/MinimalistLookAndFeel.h"
#include "ui/VirtualKeyboard.h"
#include "ui/PresetBrowser.h"

class LadderMonoAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit LadderMonoAudioProcessorEditor(LadderMonoAudioProcessor&);
    ~LadderMonoAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;
    void focusLost(FocusChangeType) override;
    void visibilityChanged() override;
    void releaseAllHeldComputerKeys();
    void mouseDown(const juce::MouseEvent& e) override;

private:
    LadderMonoAudioProcessor& audioProcessor;
    LadderMono::MinimalistLookAndFeel lnf;

    // Preset Controls
    juce::Label titleLabel;
    juce::TextButton presetButton;
    juce::TextButton browseBtn{"Browse"};
    juce::TextButton prevPresetBtn{"<"};
    juce::TextButton nextPresetBtn{">"};
    juce::TextButton initBtn{"Init"};

    std::unique_ptr<LadderMono::PresetBrowserOverlay> presetBrowser;

    // Helpers to create sliders and combos with APVTS attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;

    struct Knobby
    {
        juce::Slider slider;
        juce::Label label;
    };

    // Controllers
    Knobby tuneKnob, glideKnob, modMixKnob, lfoRateKnob;
    juce::ToggleButton oscModBtn{"Osc Mod"}, filterModBtn{"Filt Mod"};

    // Arpeggiator (User requested)
    juce::ToggleButton arpOnBtn{"Arp On"}, arpLatchBtn{"Latch"};
    juce::ComboBox arpModeBox, arpOctaveBox, arpRateBox;

    // Oscillators
    juce::ComboBox osc1RangeBox, osc1WaveBox;
    juce::ComboBox osc2RangeBox, osc2WaveBox;
    Knobby osc2FineKnob;
    juce::ComboBox osc3RangeBox, osc3WaveBox;
    Knobby osc3FineKnob;
    juce::ToggleButton osc3KbdBtn{"Kbd"};

    // Mixer
    Knobby mix1Knob, mix2Knob, mix3Knob, mixNoiseKnob, mixExtKnob, driveKnob;
    juce::ToggleButton mix1Btn{"1"}, mix2Btn{"2"}, mix3Btn{"3"}, mixNoiseBtn{"Noise"}, mixExtBtn{"Feed"};
    juce::ComboBox noiseColorBox;

    // Filter
    Knobby cutoffKnob, emphasisKnob, contourKnob, bassCompKnob;
    juce::ToggleButton kbd1Btn{"K1"}, kbd2Btn{"K2"};

    // Filter Env
    Knobby fAttackKnob, fDecayKnob, fSustainKnob;

    // Loudness Env
    Knobby aAttackKnob, aDecayKnob, aSustainKnob;
    juce::ToggleButton decaySwitchBtn{"Decay Rel"};

    // Output & Master
    Knobby masterVolKnob, analogKnob;
    juce::ComboBox voicesBox;

    // Performance Wheels & Keyboard
    juce::Slider pitchWheel;
    juce::Slider modWheel;
    LadderMono::VirtualKeyboard keyboard;
    juce::Label keyboardHintLabel;

    // Computer Keyboard musical typing state
    int baseOctaveNote = 60; // C4 default
    std::map<int, int> charCodeToPlayingNote;

    void setupKnob(Knobby& k, const juce::String& paramId, const juce::String& labelText, bool isBipolar = false);
    void setupComboBox(juce::ComboBox& box, const juce::String& paramId, const juce::StringArray& items);
    void setupToggle(juce::ToggleButton& btn, const juce::String& paramId);
    void updatePresetDisplay();
    void updateKeyboardHint();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LadderMonoAudioProcessorEditor)
};
