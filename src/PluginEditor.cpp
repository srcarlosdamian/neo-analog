#include "PluginProcessor.h"
#include "PluginEditor.h"

LadderMonoAudioProcessorEditor::LadderMonoAudioProcessorEditor(LadderMonoAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lnf);

    // Title & Preset header
    titleLabel.setText("L A D D E R   M O N O", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe08b3c));
    addAndMakeVisible(titleLabel);

    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setFont(juce::FontOptions(13.0f, juce::Font::plain));
    presetLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xff22242a));
    presetLabel.setColour(juce::Label::outlineColourId, juce::Colour(0xff353840));
    addAndMakeVisible(presetLabel);

    prevPresetBtn.onClick = [this]() {
        audioProcessor.getPresetManager().loadPrevPreset();
        updatePresetDisplay();
    };
    addAndMakeVisible(prevPresetBtn);

    nextPresetBtn.onClick = [this]() {
        audioProcessor.getPresetManager().loadNextPreset();
        updatePresetDisplay();
    };
    addAndMakeVisible(nextPresetBtn);

    initBtn.onClick = [this]() {
        audioProcessor.getPresetManager().initPatch();
        updatePresetDisplay();
    };
    addAndMakeVisible(initBtn);

    // Controllers
    setupKnob(tuneKnob, LadderMono::ParamIDs::tune, "TUNE", true);
    setupKnob(glideKnob, LadderMono::ParamIDs::glideTime, "GLIDE");
    setupKnob(modMixKnob, LadderMono::ParamIDs::modMix, "MOD MIX");
    setupKnob(lfoRateKnob, LadderMono::ParamIDs::lfoRate, "LFO RATE");
    setupToggle(oscModBtn, LadderMono::ParamIDs::oscModOn);
    setupToggle(filterModBtn, LadderMono::ParamIDs::filterModOn);

    // Arpeggiator (User requested)
    setupToggle(arpOnBtn, LadderMono::ParamIDs::arpOn);
    setupToggle(arpLatchBtn, LadderMono::ParamIDs::arpLatch);
    setupComboBox(arpModeBox, LadderMono::ParamIDs::arpMode, {"Up", "Down", "Up/Down", "Random", "Order"});
    setupComboBox(arpOctaveBox, LadderMono::ParamIDs::arpOctaves, {"1 Oct", "2 Oct", "3 Oct", "4 Oct"});
    setupComboBox(arpRateBox, LadderMono::ParamIDs::arpRateSync, {"1/4", "1/8", "1/8 T", "1/16", "1/16 T", "1/32"});

    // Oscillators
    const juce::StringArray ranges{"LO", "32'", "16'", "8'", "4'", "2'"};
    const juce::StringArray waves{"Tri", "Shark", "Saw", "Square", "Wide", "Narrow"};

    setupComboBox(osc1RangeBox, LadderMono::ParamIDs::osc1Range, ranges);
    setupComboBox(osc1WaveBox, LadderMono::ParamIDs::osc1Wave, waves);

    setupComboBox(osc2RangeBox, LadderMono::ParamIDs::osc2Range, ranges);
    setupComboBox(osc2WaveBox, LadderMono::ParamIDs::osc2Wave, waves);
    setupKnob(osc2FineKnob, LadderMono::ParamIDs::osc2Fine, "FINE", true);

    setupComboBox(osc3RangeBox, LadderMono::ParamIDs::osc3Range, ranges);
    setupComboBox(osc3WaveBox, LadderMono::ParamIDs::osc3Wave, waves);
    setupKnob(osc3FineKnob, LadderMono::ParamIDs::osc3Fine, "FINE", true);
    setupToggle(osc3KbdBtn, LadderMono::ParamIDs::osc3KbdOn);

    // Mixer
    setupKnob(mix1Knob, LadderMono::ParamIDs::mixOsc1, "OSC 1");
    setupToggle(mix1Btn, LadderMono::ParamIDs::mixOsc1On);
    setupKnob(mix2Knob, LadderMono::ParamIDs::mixOsc2, "OSC 2");
    setupToggle(mix2Btn, LadderMono::ParamIDs::mixOsc2On);
    setupKnob(mix3Knob, LadderMono::ParamIDs::mixOsc3, "OSC 3");
    setupToggle(mix3Btn, LadderMono::ParamIDs::mixOsc3On);
    setupKnob(mixNoiseKnob, LadderMono::ParamIDs::mixNoise, "NOISE");
    setupToggle(mixNoiseBtn, LadderMono::ParamIDs::mixNoiseOn);
    setupKnob(mixExtKnob, LadderMono::ParamIDs::mixExt, "FEEDBACK");
    setupToggle(mixExtBtn, LadderMono::ParamIDs::mixExtOn);
    setupKnob(driveKnob, LadderMono::ParamIDs::mixerDrive, "DRIVE", true);
    setupComboBox(noiseColorBox, LadderMono::ParamIDs::noiseColor, {"White", "Pink"});

    // Filter
    setupKnob(cutoffKnob, LadderMono::ParamIDs::cutoff, "CUTOFF", true);
    setupKnob(emphasisKnob, LadderMono::ParamIDs::emphasis, "EMPHASIS");
    setupKnob(contourKnob, LadderMono::ParamIDs::contourAmount, "AMOUNT");
    setupKnob(bassCompKnob, LadderMono::ParamIDs::bassComp, "BASS COMP");
    setupToggle(kbd1Btn, LadderMono::ParamIDs::kbd1);
    setupToggle(kbd2Btn, LadderMono::ParamIDs::kbd2);

    // Envelopes
    setupKnob(fAttackKnob, LadderMono::ParamIDs::fAttack, "ATTACK");
    setupKnob(fDecayKnob, LadderMono::ParamIDs::fDecay, "DECAY");
    setupKnob(fSustainKnob, LadderMono::ParamIDs::fSustain, "SUSTAIN");

    setupKnob(aAttackKnob, LadderMono::ParamIDs::aAttack, "ATTACK");
    setupKnob(aDecayKnob, LadderMono::ParamIDs::aDecay, "DECAY");
    setupKnob(aSustainKnob, LadderMono::ParamIDs::aSustain, "SUSTAIN");
    setupToggle(decaySwitchBtn, LadderMono::ParamIDs::decaySwitchOn);

    // Output & Master
    setupKnob(masterVolKnob, LadderMono::ParamIDs::masterVol, "VOLUME");
    setupKnob(analogKnob, LadderMono::ParamIDs::analogAmount, "DRIFT");

    // Performance Wheels
    pitchWheel.setSliderStyle(juce::Slider::LinearVertical);
    pitchWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pitchWheel.setRange(-1.0, 1.0, 0.01);
    pitchWheel.setValue(0.0);
    pitchWheel.onValueChange = [this]() {
        float bend = static_cast<float>(pitchWheel.getValue());
        int range = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue(LadderMono::ParamIDs::bendRange)->load());
        audioProcessor.getVoice().setPitchBend(bend * range);
    };
    addAndMakeVisible(pitchWheel);

    modWheel.setSliderStyle(juce::Slider::LinearVertical);
    modWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    modWheel.setRange(0.0, 1.0, 0.01);
    modWheel.setValue(0.0);
    modWheel.onValueChange = [this]() {
        audioProcessor.getVoice().setModWheel(static_cast<float>(modWheel.getValue()));
    };
    addAndMakeVisible(modWheel);

    // Virtual Keyboard
    keyboard.onNoteOn = [this](int note, float vel) {
        audioProcessor.triggerNoteOn(note, vel);
    };
    keyboard.onNoteOff = [this](int note) {
        audioProcessor.triggerNoteOff(note);
    };
    addAndMakeVisible(keyboard);

    updatePresetDisplay();
    setSize(1100, 600);
}

LadderMonoAudioProcessorEditor::~LadderMonoAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void LadderMonoAudioProcessorEditor::setupKnob(Knobby& k, const juce::String& paramId, const juce::String& labelText, bool isBipolar)
{
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 14);
    addAndMakeVisible(k.slider);

    k.label.setText(labelText, juce::dontSendNotification);
    k.label.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    k.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(k.label);

    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), paramId, k.slider));
}

void LadderMonoAudioProcessorEditor::setupComboBox(juce::ComboBox& box, const juce::String& paramId, const juce::StringArray& items)
{
    box.addItemList(items, 1);
    addAndMakeVisible(box);
    comboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), paramId, box));
}

void LadderMonoAudioProcessorEditor::setupToggle(juce::ToggleButton& btn, const juce::String& paramId)
{
    addAndMakeVisible(btn);
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), paramId, btn));
}

void LadderMonoAudioProcessorEditor::updatePresetDisplay()
{
    presetLabel.setText(audioProcessor.getPresetManager().getCurrentPresetName(), juce::dontSendNotification);
}

void LadderMonoAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Minimalist dark background
    g.fillAll(juce::Colour(0xff16171a));

    // Section dividers and headers
    g.setColour(juce::Colour(0xff22242a));
    g.fillRect(0, 0, getWidth(), 40); // Top bar

    g.setColour(juce::Colour(0xff2d2f36));
    g.drawHorizontalLine(40, 0.0f, static_cast<float>(getWidth()));
    g.drawHorizontalLine(480, 0.0f, static_cast<float>(getWidth())); // Above keyboard

    // Section dividing vertical lines
    g.drawVerticalLine(160, 40.0f, 480.0f); // Controllers / Arp divider
    g.drawVerticalLine(340, 40.0f, 480.0f); // Arp / Osc bank divider
    g.drawVerticalLine(560, 40.0f, 480.0f); // Osc / Mixer divider
    g.drawVerticalLine(740, 40.0f, 480.0f); // Mixer / Filter divider
    g.drawVerticalLine(980, 40.0f, 480.0f); // Filter / Output divider

    // Section titles
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff8a8f9d));

    g.drawText("CONTROLLERS", 10, 42, 140, 18, juce::Justification::centred);
    g.drawText("ARPEGGIATOR", 170, 42, 160, 18, juce::Justification::centred);
    g.drawText("OSCILLATOR BANK", 350, 42, 200, 18, juce::Justification::centred);
    g.drawText("MIXER", 570, 42, 160, 18, juce::Justification::centred);
    g.drawText("FILTER & ENVELOPES", 750, 42, 220, 18, juce::Justification::centred);
    g.drawText("OUTPUT", 990, 42, 100, 18, juce::Justification::centred);
}

void LadderMonoAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Top bar
    auto topBar = area.removeFromTop(40);
    titleLabel.setBounds(topBar.removeFromLeft(220).reduced(10, 5));
    initBtn.setBounds(topBar.removeFromRight(60).reduced(5, 7));
    nextPresetBtn.setBounds(topBar.removeFromRight(36).reduced(4, 7));
    prevPresetBtn.setBounds(topBar.removeFromRight(36).reduced(4, 7));
    presetLabel.setBounds(topBar.removeFromRight(200).reduced(5, 7));

    // Bottom Performance Section (Keyboard + Wheels)
    auto bottomArea = area.removeFromBottom(120);
    pitchWheel.setBounds(bottomArea.removeFromLeft(36).reduced(6, 4));
    modWheel.setBounds(bottomArea.removeFromLeft(36).reduced(6, 4));
    keyboard.setBounds(bottomArea);

    // Section 1: Controllers (0..160)
    tuneKnob.slider.setBounds(15, 70, 60, 60);
    tuneKnob.label.setBounds(15, 130, 60, 14);

    glideKnob.slider.setBounds(85, 70, 60, 60);
    glideKnob.label.setBounds(85, 130, 60, 14);

    modMixKnob.slider.setBounds(15, 160, 60, 60);
    modMixKnob.label.setBounds(15, 220, 60, 14);

    lfoRateKnob.slider.setBounds(85, 160, 60, 60);
    lfoRateKnob.label.setBounds(85, 220, 60, 14);

    oscModBtn.setBounds(20, 250, 120, 26);
    filterModBtn.setBounds(20, 285, 120, 26);

    // Section 2: Arpeggiator (160..340)
    arpOnBtn.setBounds(180, 75, 130, 26);
    arpLatchBtn.setBounds(180, 110, 130, 26);
    arpModeBox.setBounds(180, 155, 140, 24);
    arpOctaveBox.setBounds(180, 195, 140, 24);
    arpRateBox.setBounds(180, 235, 140, 24);

    // Section 3: Oscillator Bank (340..560)
    // Osc 1
    osc1RangeBox.setBounds(355, 75, 85, 24);
    osc1WaveBox.setBounds(450, 75, 95, 24);

    // Osc 2
    osc2RangeBox.setBounds(355, 135, 85, 24);
    osc2WaveBox.setBounds(450, 135, 95, 24);
    osc2FineKnob.slider.setBounds(400, 175, 55, 55);
    osc2FineKnob.label.setBounds(400, 230, 55, 14);

    // Osc 3
    osc3RangeBox.setBounds(355, 265, 85, 24);
    osc3WaveBox.setBounds(450, 265, 95, 24);
    osc3FineKnob.slider.setBounds(370, 305, 55, 55);
    osc3FineKnob.label.setBounds(370, 360, 55, 14);
    osc3KbdBtn.setBounds(450, 320, 90, 26);

    // Section 4: Mixer (560..740)
    mix1Knob.slider.setBounds(575, 70, 50, 50);
    mix1Knob.label.setBounds(575, 120, 50, 14);
    mix1Btn.setBounds(635, 80, 80, 24);

    mix2Knob.slider.setBounds(575, 145, 50, 50);
    mix2Knob.label.setBounds(575, 195, 50, 14);
    mix2Btn.setBounds(635, 155, 80, 24);

    mix3Knob.slider.setBounds(575, 220, 50, 50);
    mix3Knob.label.setBounds(575, 270, 50, 14);
    mix3Btn.setBounds(635, 230, 80, 24);

    mixNoiseKnob.slider.setBounds(575, 295, 50, 50);
    mixNoiseKnob.label.setBounds(575, 345, 50, 14);
    mixNoiseBtn.setBounds(635, 300, 80, 24);
    noiseColorBox.setBounds(635, 330, 80, 20);

    mixExtKnob.slider.setBounds(575, 370, 50, 50);
    mixExtKnob.label.setBounds(575, 420, 50, 14);
    mixExtBtn.setBounds(635, 375, 80, 24);
    driveKnob.slider.setBounds(645, 410, 48, 48);
    driveKnob.label.setBounds(645, 458, 48, 14);

    // Section 5: Filter & Envelopes (740..980)
    cutoffKnob.slider.setBounds(755, 70, 60, 60);
    cutoffKnob.label.setBounds(755, 130, 60, 14);

    emphasisKnob.slider.setBounds(830, 70, 60, 60);
    emphasisKnob.label.setBounds(830, 130, 60, 14);

    contourKnob.slider.setBounds(905, 70, 60, 60);
    contourKnob.label.setBounds(905, 130, 60, 14);

    kbd1Btn.setBounds(760, 155, 70, 24);
    kbd2Btn.setBounds(840, 155, 70, 24);
    bassCompKnob.slider.setBounds(915, 145, 50, 50);
    bassCompKnob.label.setBounds(915, 195, 50, 14);

    // Filter Env (Row 2)
    fAttackKnob.slider.setBounds(755, 220, 55, 55);
    fAttackKnob.label.setBounds(755, 275, 55, 14);
    fDecayKnob.slider.setBounds(830, 220, 55, 55);
    fDecayKnob.label.setBounds(830, 275, 55, 14);
    fSustainKnob.slider.setBounds(905, 220, 55, 55);
    fSustainKnob.label.setBounds(905, 275, 55, 14);

    // Amp Env (Row 3)
    aAttackKnob.slider.setBounds(755, 305, 55, 55);
    aAttackKnob.label.setBounds(755, 360, 55, 14);
    aDecayKnob.slider.setBounds(830, 305, 55, 55);
    aDecayKnob.label.setBounds(830, 360, 55, 14);
    aSustainKnob.slider.setBounds(905, 305, 55, 55);
    aSustainKnob.label.setBounds(905, 360, 55, 14);
    decaySwitchBtn.setBounds(800, 395, 120, 24);

    // Section 6: Output & Master (980..1100)
    masterVolKnob.slider.setBounds(1005, 70, 70, 70);
    masterVolKnob.label.setBounds(1005, 140, 70, 14);

    analogKnob.slider.setBounds(1005, 180, 60, 60);
    analogKnob.label.setBounds(1005, 240, 60, 14);
}
