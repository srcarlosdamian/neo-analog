#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/MinimalistLookAndFeel.h"
#include "ui/VirtualKeyboard.h"
#include "ui/PresetBrowser.h"
#include "ui/AboutDialog.h"
#include "ui/ThemeEditorPanel.h"
#include "ui/MasterBypassButton.h"
#include "ui/CornerResizer.h"
#include "ui/ParameterHelpBar.h"

class LadderMonoAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
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
    void parentHierarchyChanged() override;
    void releaseAllHeldComputerKeys();
    void mouseDown(const juce::MouseEvent& e) override;

    void showAboutDialog();
    void saveCurrentPresetDialog();
    void toggleABState();
    void undo();
    void redo();
    void pushUndoSnapshot();
    void applyTheme(const LadderMono::ThemeColors& theme);
    void toggleThemeEditor();
    void updateHelpBarForComponent(juce::Component* comp);
    void simulateHoverTune();

    void mouseMove(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

private:
    void timerCallback() override;

    // Undo / Redo history
    std::vector<juce::ValueTree> undoStack;
    std::vector<juce::ValueTree> redoStack;
    bool isPerformingUndoRedo = false;

    LadderMonoAudioProcessor& audioProcessor;
    LadderMono::MinimalistLookAndFeel lnf;
    LadderMono::ThemeColors currentTheme;

    // Preset & Header Controls
    juce::Label titleLabel;
    juce::TextButton versionBtn{"v1.0"};
    juce::TextButton presetButton;
    juce::TextButton prevPresetBtn{"<"};
    juce::TextButton nextPresetBtn{">"};
    juce::TextButton browseBtn{"Browse"};
    juce::TextButton saveBtn{"Save"};
    juce::TextButton initBtn{"Init"};
    juce::TextButton abBtn{"A/B: A"};
    juce::TextButton hqBtn{"HQ"};
    juce::Label cpuLabel;
    juce::TextButton editThemeBtn;
    juce::TextButton aboutBtn{"Credits"};

    // A/B Comparison States
    juce::ValueTree stateA;
    juce::ValueTree stateB;
    bool isShowingA = true;

    std::unique_ptr<LadderMono::PresetBrowserOverlay> presetBrowser;
    std::unique_ptr<LadderMono::AboutDialogOverlay> aboutDialog;
    std::unique_ptr<LadderMono::ThemeEditorPanel> themeEditorPanel;

    // Window Resizer
    juce::ComponentBoundsConstrainer constrainer;
    std::unique_ptr<LadderMono::CornerResizer> resizer;
    std::unique_ptr<LadderMono::ParameterHelpBar> helpBar;

    // Helpers to create sliders and combos with APVTS attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;

    struct Knobby
    {
        juce::Slider slider;
        juce::Label label;
    };

    // Section 1: Controllers
    Knobby tuneKnob;
    Knobby glideKnob;
    juce::ToggleButton glideOnBtn{"GLIDE"};
    juce::ComboBox glideModeBox;
    Knobby modMixKnob;
    juce::ComboBox modSourceBox;
    juce::ToggleButton oscModBtn{"OSC MOD"}, filterModBtn{"FILT MOD"};
    Knobby lfoRateKnob;
    juce::ComboBox lfoShapeBox;
    juce::ComboBox bendRangeBox;
    juce::ComboBox notePriorityBox;

    // Section 2: Oscillators
    juce::ComboBox osc1RangeBox, osc1WaveBox;
    juce::ComboBox osc2RangeBox, osc2WaveBox;
    Knobby osc2FineKnob;
    juce::ComboBox osc3RangeBox, osc3WaveBox;
    Knobby osc3FineKnob;
    juce::ToggleButton osc3KbdBtn{"OSC 3 KBD"};
    Knobby analogKnob; // DRIFT knob (moved to Oscillator Bank)

    // Section 3: Mixer
    Knobby mix1Knob, mix2Knob, mix3Knob, mixNoiseKnob, mixExtKnob, driveKnob;
    juce::ToggleButton mix1Btn{"1"}, mix2Btn{"2"}, mix3Btn{"3"}, mixNoiseBtn{"NOISE"}, mixExtBtn{"FEED"};
    juce::ComboBox noiseColorBox;

    // Section 4: Modifiers (Filter & Envelopes)
    Knobby cutoffKnob, emphasisKnob, contourKnob, bassCompKnob, spatialKnob;
    juce::ToggleButton kbd1Btn{"KBD 1"}, kbd2Btn{"KBD 2"};
    juce::ComboBox filterModelBox;

    Knobby fAttackKnob, fDecayKnob, fSustainKnob;
    Knobby aAttackKnob, aDecayKnob, aSustainKnob;
    juce::ToggleButton decaySwitchBtn{"DECAY REL"};

    // Section 5: Output & Master
    Knobby masterVolKnob;
    juce::ComboBox voicesBox;
    LadderMono::MasterBypassButton bypassBtn;

    // Arpeggiator (Moved to dedicated Performance Bar above keyboard)
    juce::Label arpSectionLabel;
    juce::ToggleButton arpOnBtn{"ARP ON"}, arpLatchBtn{"LATCH"};
    juce::ComboBox arpModeBox, arpOctaveBox, arpRateBox;
    juce::ComboBox arpOctModeBox, arpPatternBox;
    Knobby arpGateKnob;
    Knobby arpSwingKnob;
    Knobby arpMutateKnob;
    juce::Label arpModeLabel, arpOctaveLabel, arpRateLabel;
    juce::Label arpOctModeLabel, arpPatternLabel;

    // Performance Wheels & Keyboard
    juce::Slider pitchWheel;
    juce::Slider modWheel;
    LadderMono::VirtualKeyboard keyboard;

    // Computer Keyboard musical typing state
    int baseOctaveNote = 60; // C4 default
    std::map<int, int> charCodeToPlayingNote;

    void setupKnob(Knobby& k, const juce::String& paramId, const juce::String& labelText, bool isBipolar = false);
    void setupComboBox(juce::ComboBox& box, const juce::String& paramId, const juce::StringArray& items);
    void setupToggle(juce::ToggleButton& btn, const juce::String& paramId);
    void updatePresetDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LadderMonoAudioProcessorEditor)
};
