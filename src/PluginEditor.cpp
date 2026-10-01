#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

LadderMonoAudioProcessorEditor::LadderMonoAudioProcessorEditor(LadderMonoAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lnf);

    // ====================================================================
    // Header & Top Bar Controls
    // ====================================================================
    titleLabel.setText("N E O   A N A L O G", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe08b3c));
    addAndMakeVisible(titleLabel);

    versionBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a261c));
    versionBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff233526));
    versionBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff48c764));
    versionBtn.setTooltip("Neo Analog v1.0 \xe2\x80\x94 Click for Credits & Roadmap");
    versionBtn.onClick = [this] { showAboutDialog(); };
    addAndMakeVisible(versionBtn);

    // Preset selector button
    presetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff18191d));
    presetButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff25272e));
    presetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe8eaee));
    presetButton.onClick = [this]() {
        juce::PopupMenu menu;
        menu.addItem(9999, "★ Open Preset Browser...", true, false);
        menu.addSeparator();

        std::map<juce::String, std::vector<std::pair<int, juce::String>>> categorized;
        const auto& presets = audioProcessor.getPresetManager().getPresets();
        for (size_t i = 0; i < presets.size(); ++i)
        {
            categorized[presets[i].category].emplace_back(static_cast<int>(i + 1), presets[i].name);
        }

        if (categorized.find("Basics") != categorized.end())
        {
            juce::PopupMenu basicsMenu;
            for (const auto& [id, name] : categorized["Basics"])
            {
                bool isCurrent = (id - 1 == audioProcessor.getPresetManager().getCurrentPresetIndex());
                basicsMenu.addItem(id, name, true, isCurrent);
            }
            menu.addSubMenu("★ Basics (Sonidos Básicos)", basicsMenu);
            menu.addSeparator();
        }

        for (const auto& [cat, list] : categorized)
        {
            if (cat == "Basics" || cat == "Init") continue;
            juce::PopupMenu subMenu;
            for (const auto& [id, name] : list)
            {
                bool isCurrent = (id - 1 == audioProcessor.getPresetManager().getCurrentPresetIndex());
                subMenu.addItem(id, name, true, isCurrent);
            }
            menu.addSubMenu(cat, subMenu);
        }

        if (categorized.find("Init") != categorized.end())
        {
            menu.addSeparator();
            for (const auto& [id, name] : categorized["Init"])
            {
                bool isCurrent = (id - 1 == audioProcessor.getPresetManager().getCurrentPresetIndex());
                menu.addItem(id, name, true, isCurrent);
            }
        }

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&presetButton),
            [this](int result) {
                if (result == 9999)
                {
                    releaseAllHeldComputerKeys();
                    if (presetBrowser != nullptr)
                        presetBrowser->showBrowser();
                }
                else if (result > 0)
                {
                    releaseAllHeldComputerKeys();
                    audioProcessor.getPresetManager().loadPreset(result - 1);
                    updatePresetDisplay();
                }
            });
    };
    addAndMakeVisible(presetButton);

    prevPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff18191d));
    prevPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe08b3c));
    prevPresetBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
        audioProcessor.getPresetManager().loadPrevPreset();
        updatePresetDisplay();
    };
    addAndMakeVisible(prevPresetBtn);

    nextPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff18191d));
    nextPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe08b3c));
    nextPresetBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
        audioProcessor.getPresetManager().loadNextPreset();
        updatePresetDisplay();
    };
    addAndMakeVisible(nextPresetBtn);

    browseBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22252c));
    browseBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff2f333c));
    browseBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe08b3c));
    browseBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
        if (presetBrowser != nullptr)
            presetBrowser->showBrowser();
    };
    addAndMakeVisible(browseBtn);

    saveBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff18191d));
    saveBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe0a350));
    saveBtn.setTooltip("Save / Export Preset (.nlog file)");
    saveBtn.onClick = [this] { saveCurrentPresetDialog(); };
    addAndMakeVisible(saveBtn);

    initBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff18191d));
    initBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff9ea3b0));
    initBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
        audioProcessor.getPresetManager().initPatch();
        updatePresetDisplay();
    };
    addAndMakeVisible(initBtn);

    // A/B Comparison Toggle
    abBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e2026));
    abBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe0e4eb));
    abBtn.setTooltip("A/B State Compare — Switch between Patch A and Patch B");
    abBtn.onClick = [this] { toggleABState(); };
    addAndMakeVisible(abBtn);

    // High Quality Oversampling Toggle
    hqBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e2026));
    hqBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffe08b3c));
    hqBtn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffffff));
    hqBtn.setClickingTogglesState(true);
    hqBtn.setTooltip("HQ Mode (2x Oversampling & Anti-Aliasing)");
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), LadderMono::ParamIDs::hqMode, hqBtn));
    addAndMakeVisible(hqBtn);

    // CPU Usage Meter
    cpuLabel.setText("CPU: 0.0%", juce::dontSendNotification);
    cpuLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    cpuLabel.setColour(juce::Label::textColourId, juce::Colour(0xff6fc070));
    cpuLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cpuLabel);

    aboutBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff18191d));
    aboutBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff25272e));
    aboutBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffa8adb8));
    aboutBtn.onClick = [this] { showAboutDialog(); };
    addAndMakeVisible(aboutBtn);

    // ====================================================================
    // Section 1: CONTROLLERS
    // ====================================================================
    setupKnob(tuneKnob, LadderMono::ParamIDs::tune, "TUNE", true);
    setupKnob(glideKnob, LadderMono::ParamIDs::glideTime, "GLIDE");
    setupToggle(glideOnBtn, LadderMono::ParamIDs::glideOn);
    setupComboBox(glideModeBox, LadderMono::ParamIDs::glideMode, {"Always", "Legato"});

    setupKnob(modMixKnob, LadderMono::ParamIDs::modMix, "MOD MIX");
    setupComboBox(modSourceBox, LadderMono::ParamIDs::modSource, {"Osc 3", "LFO"});
    setupToggle(oscModBtn, LadderMono::ParamIDs::oscModOn);
    setupToggle(filterModBtn, LadderMono::ParamIDs::filterModOn);

    setupKnob(lfoRateKnob, LadderMono::ParamIDs::lfoRate, "LFO RATE");
    setupComboBox(lfoShapeBox, LadderMono::ParamIDs::lfoShape, {"Triangle", "Square"});
    setupComboBox(bendRangeBox, LadderMono::ParamIDs::bendRange, {"2 Semi", "7 Semi", "12 Oct"});
    setupComboBox(notePriorityBox, LadderMono::ParamIDs::notePriority, {"Low", "Last", "High"});

    // ====================================================================
    // Section 2: OSCILLATOR BANK
    // ====================================================================
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

    // DRIFT (Analog Component Drift & Thermal Instability) - moved from Output
    setupKnob(analogKnob, LadderMono::ParamIDs::analogAmount, "DRIFT");

    // ====================================================================
    // Section 3: MIXER
    // ====================================================================
    setupKnob(mix1Knob, LadderMono::ParamIDs::mixOsc1, "OSC 1");
    setupToggle(mix1Btn, LadderMono::ParamIDs::mixOsc1On);

    setupKnob(mix2Knob, LadderMono::ParamIDs::mixOsc2, "OSC 2");
    setupToggle(mix2Btn, LadderMono::ParamIDs::mixOsc2On);

    setupKnob(mix3Knob, LadderMono::ParamIDs::mixOsc3, "OSC 3");
    setupToggle(mix3Btn, LadderMono::ParamIDs::mixOsc3On);

    setupKnob(mixNoiseKnob, LadderMono::ParamIDs::mixNoise, "NOISE");
    setupToggle(mixNoiseBtn, LadderMono::ParamIDs::mixNoiseOn);
    setupComboBox(noiseColorBox, LadderMono::ParamIDs::noiseColor, {"White", "Pink"});

    setupKnob(mixExtKnob, LadderMono::ParamIDs::mixExt, "EXT IN");
    setupToggle(mixExtBtn, LadderMono::ParamIDs::mixExtOn);
    setupKnob(driveKnob, LadderMono::ParamIDs::mixerDrive, "DRIVE", true);

    // ====================================================================
    // Section 4: MODIFIERS (Filter & Envelopes)
    // ====================================================================
    setupKnob(cutoffKnob, LadderMono::ParamIDs::cutoff, "CUTOFF", true);
    setupKnob(emphasisKnob, LadderMono::ParamIDs::emphasis, "EMPHASIS");
    setupKnob(contourKnob, LadderMono::ParamIDs::contourAmount, "CONTOUR AMT"); // Renamed from ambiguous AMOUNT!
    setupKnob(bassCompKnob, LadderMono::ParamIDs::bassComp, "BASS COMP");
    setupToggle(kbd1Btn, LadderMono::ParamIDs::kbd1);
    setupToggle(kbd2Btn, LadderMono::ParamIDs::kbd2);

    // Filter Contour
    setupKnob(fAttackKnob, LadderMono::ParamIDs::fAttack, "ATTACK");
    setupKnob(fDecayKnob, LadderMono::ParamIDs::fDecay, "DECAY");
    setupKnob(fSustainKnob, LadderMono::ParamIDs::fSustain, "SUSTAIN");

    // Loudness Contour
    setupKnob(aAttackKnob, LadderMono::ParamIDs::aAttack, "ATTACK");
    setupKnob(aDecayKnob, LadderMono::ParamIDs::aDecay, "DECAY");
    setupKnob(aSustainKnob, LadderMono::ParamIDs::aSustain, "SUSTAIN");
    setupToggle(decaySwitchBtn, LadderMono::ParamIDs::decaySwitchOn);

    // ====================================================================
    // Section 5: OUTPUT
    // ====================================================================
    setupKnob(masterVolKnob, LadderMono::ParamIDs::masterVol, "VOLUME");
    setupComboBox(voicesBox, LadderMono::ParamIDs::voices, {"Mono", "4 Voices", "8 Voices"});

    bypassBtn.setClickingTogglesState(true);
    bypassBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22242a));
    bypassBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff8b1c1c));
    bypassBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffa0a5b2));
    bypassBtn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffdddd));
    bypassBtn.setTooltip("Audio Engine Bypass");
    bypassBtn.onClick = [this] {
        bool isByp = bypassBtn.getToggleState();
        audioProcessor.setBypassed(isByp);
        bypassBtn.setButtonText(isByp ? "BYPASSED" : "BYPASS");
    };
    addAndMakeVisible(bypassBtn);

    // ====================================================================
    // Middle Strip: ARPEGGIATOR & PERFORMANCE BAR
    // ====================================================================
    arpSectionLabel.setText("ARPEGGIATOR", juce::dontSendNotification);
    arpSectionLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    arpSectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe08b3c));
    addAndMakeVisible(arpSectionLabel);

    setupToggle(arpOnBtn, LadderMono::ParamIDs::arpOn);
    setupToggle(arpLatchBtn, LadderMono::ParamIDs::arpLatch);

    arpModeLabel.setText("MODE", juce::dontSendNotification);
    arpModeLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    arpModeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ea3b0));
    addAndMakeVisible(arpModeLabel);
    setupComboBox(arpModeBox, LadderMono::ParamIDs::arpMode, {"Up", "Down", "Up/Down", "Random", "Order"});

    arpOctaveLabel.setText("OCT", juce::dontSendNotification);
    arpOctaveLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    arpOctaveLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ea3b0));
    addAndMakeVisible(arpOctaveLabel);
    setupComboBox(arpOctaveBox, LadderMono::ParamIDs::arpOctaves, {"1 Oct", "2 Oct", "3 Oct", "4 Oct"});

    arpRateLabel.setText("RATE", juce::dontSendNotification);
    arpRateLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    arpRateLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ea3b0));
    addAndMakeVisible(arpRateLabel);
    setupComboBox(arpRateBox, LadderMono::ParamIDs::arpRateSync, {"1/4", "1/8", "1/8 T", "1/16", "1/16 T", "1/32"});

    setupKnob(arpGateKnob, LadderMono::ParamIDs::arpGate, "GATE");

    // Performance Wheels
    pitchWheel.setSliderStyle(juce::Slider::LinearVertical);
    pitchWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pitchWheel.setRange(-1.0, 1.0, 0.01);
    pitchWheel.setValue(0.0);
    pitchWheel.onValueChange = [this]() {
        float normVal = static_cast<float>(pitchWheel.getValue());
        int bendRange = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue(LadderMono::ParamIDs::bendRange)->load());
        float semitones = normVal * bendRange;
        for (auto& v : audioProcessor.getVoices())
            v.setPitchBend(semitones);
    };
    addAndMakeVisible(pitchWheel);

    modWheel.setSliderStyle(juce::Slider::LinearVertical);
    modWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    modWheel.setRange(0.0, 1.0, 0.01);
    modWheel.setValue(0.0);
    modWheel.onValueChange = [this]() {
        float mw = static_cast<float>(modWheel.getValue());
        for (auto& v : audioProcessor.getVoices())
            v.setModWheel(mw);
    };
    addAndMakeVisible(modWheel);

    // Virtual Keyboard
    keyboard.setBaseOctaveNote(baseOctaveNote);
    keyboard.onNoteOn = [this](int note, float vel) {
        audioProcessor.handleNoteOn(note, vel);
    };
    keyboard.onNoteOff = [this](int note) {
        audioProcessor.handleNoteOff(note);
    };
    addAndMakeVisible(keyboard);

    // Keyboard Hint Label
    keyboardHintLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    keyboardHintLabel.setColour(juce::Label::textColourId, juce::Colour(0xffdcdfe5));
    keyboardHintLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(keyboardHintLabel);
    updateKeyboardHint();

    presetBrowser = std::make_unique<LadderMono::PresetBrowserOverlay>(
        audioProcessor.getPresetManager(),
        [this] { updatePresetDisplay(); }
    );
    addChildComponent(*presetBrowser);

    aboutDialog = std::make_unique<LadderMono::AboutDialogOverlay>([this] {
        grabKeyboardFocus();
    });
    addChildComponent(*aboutDialog);

    // Corner Resizer
    constrainer.setSizeLimits(960, 580, 1680, 1050);
    resizer = std::make_unique<juce::ResizableCornerComponent>(this, &constrainer);
    addAndMakeVisible(*resizer);

    // Capture initial State A
    stateA = audioProcessor.getAPVTS().copyState();

    updatePresetDisplay();

    setWantsKeyboardFocus(true);
    setSize(1120, 680);

    // Start 20 Hz UI refresh timer for CPU meter & live state
    startTimerHz(20);
}

LadderMonoAudioProcessorEditor::~LadderMonoAudioProcessorEditor()
{
    stopTimer();
    sliderAttachments.clear();
    comboAttachments.clear();
    buttonAttachments.clear();
    setLookAndFeel(nullptr);
}

void LadderMonoAudioProcessorEditor::timerCallback()
{
    float cpu = audioProcessor.getCpuUsagePercent();
    cpuLabel.setText("CPU: " + juce::String(cpu, 1) + "%", juce::dontSendNotification);
    if (cpu > 40.0f)
        cpuLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff5555));
    else if (cpu > 20.0f)
        cpuLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffb84d));
    else
        cpuLabel.setColour(juce::Label::textColourId, juce::Colour(0xff6fc070));

    // Repaint overload LED if mixer drive changed
    repaint(472, 46, 186, 388);
}

void LadderMonoAudioProcessorEditor::toggleABState()
{
    if (isShowingA)
    {
        // Save current to A, load B
        stateA = audioProcessor.getAPVTS().copyState();
        if (!stateB.isValid())
            stateB = stateA.createCopy();
        audioProcessor.getAPVTS().replaceState(stateB);
        isShowingA = false;
        abBtn.setButtonText("A/B: B");
        abBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff884814));
    }
    else
    {
        // Save current to B, load A
        stateB = audioProcessor.getAPVTS().copyState();
        audioProcessor.getAPVTS().replaceState(stateA);
        isShowingA = true;
        abBtn.setButtonText("A/B: A");
        abBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e2026));
    }
    updatePresetDisplay();
}

void LadderMonoAudioProcessorEditor::saveCurrentPresetDialog()
{
    releaseAllHeldComputerKeys();
    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Export Neo Analog Preset (.nlog)",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(audioProcessor.getPresetManager().getCurrentPresetName() + ".nlog"),
        "*.nlog");

    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, fileChooser](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File())
            {
                juce::DynamicObject::Ptr rootObj = new juce::DynamicObject();
                rootObj->setProperty("format", "neo-analog-preset");
                rootObj->setProperty("version", "1.0");
                rootObj->setProperty("name", file.getFileNameWithoutExtension());
                rootObj->setProperty("category", "User");
                rootObj->setProperty("author", "Carlos Damian");

                juce::DynamicObject::Ptr paramsObj = new juce::DynamicObject();
                for (auto* param : audioProcessor.getAPVTS().processor.getParameters())
                {
                    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
                    {
                        float actualVal = ranged->convertFrom0to1(ranged->getValue());
                        paramsObj->setProperty(ranged->paramID, actualVal);
                    }
                }
                rootObj->setProperty("params", juce::var(paramsObj.get()));

                juce::var jsonVar(rootObj.get());
                juce::String jsonString = juce::JSON::toString(jsonVar, false);
                file.replaceWithText(jsonString);
            }
        });
}

void LadderMonoAudioProcessorEditor::showAboutDialog()
{
    releaseAllHeldComputerKeys();
    if (presetBrowser != nullptr)
        presetBrowser->setVisible(false);
    if (aboutDialog != nullptr)
        aboutDialog->showDialog();
}

void LadderMonoAudioProcessorEditor::updateKeyboardHint()
{
    int octNum = (baseOctaveNote / 12) - 1;
    keyboardHintLabel.setText("KEYBOARD PLAYING  [ A W S E D F T G Y H U J K ]  |  OCTAVE: C" + juce::String(octNum) + " (Z/X)",
                             juce::dontSendNotification);
}

bool LadderMonoAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    if (aboutDialog != nullptr && aboutDialog->isVisible())
    {
        if (key == juce::KeyPress::escapeKey)
        {
            aboutDialog->setVisible(false);
            return true;
        }
        return aboutDialog->keyPressed(key);
    }

    if (presetBrowser != nullptr && presetBrowser->isVisible())
    {
        if (presetBrowser->isSearchFocused())
        {
            if (key == juce::KeyPress::escapeKey || key == juce::KeyPress::returnKey)
            {
                presetBrowser->clearSearchFocus();
                return true;
            }
            return presetBrowser->keyPressed(key);
        }

        if (key == juce::KeyPress::escapeKey)
        {
            presetBrowser->setVisible(false);
            return true;
        }

        if (key == juce::KeyPress::upKey)
        {
            presetBrowser->selectPrevPreset();
            updatePresetDisplay();
            return true;
        }
        if (key == juce::KeyPress::downKey)
        {
            presetBrowser->selectNextPreset();
            updatePresetDisplay();
            return true;
        }
    }

    // Octave shifting with Z and X
    int code = key.getKeyCode();
    if (code == 'Z' || code == 'z')
    {
        if (baseOctaveNote > 12)
        {
            releaseAllHeldComputerKeys();
            baseOctaveNote -= 12;
            keyboard.setBaseOctaveNote(baseOctaveNote);
            updateKeyboardHint();
        }
        return true;
    }
    if (code == 'X' || code == 'x')
    {
        if (baseOctaveNote < 96)
        {
            releaseAllHeldComputerKeys();
            baseOctaveNote += 12;
            keyboard.setBaseOctaveNote(baseOctaveNote);
            updateKeyboardHint();
        }
        return true;
    }

    // Musical typing keys mapping
    static const std::map<char, int> keyOffsets = {
        {'a', 0}, {'A', 0},
        {'w', 1}, {'W', 1},
        {'s', 2}, {'S', 2},
        {'e', 3}, {'E', 3},
        {'d', 4}, {'D', 4},
        {'f', 5}, {'F', 5},
        {'t', 6}, {'T', 6},
        {'g', 7}, {'G', 7},
        {'y', 8}, {'Y', 8},
        {'h', 9}, {'H', 9},
        {'u', 10}, {'U', 10},
        {'j', 11}, {'J', 11},
        {'k', 12}, {'K', 12}
    };

    auto it = keyOffsets.find(static_cast<char>(code));
    if (it != keyOffsets.end())
    {
        int noteNumber = baseOctaveNote + it->second;
        if (charCodeToPlayingNote.find(code) == charCodeToPlayingNote.end())
        {
            charCodeToPlayingNote[code] = noteNumber;
            audioProcessor.handleNoteOn(noteNumber, 0.90f);
            keyboard.setNoteActive(noteNumber, true);
        }
        return true;
    }

    return false;
}

bool LadderMonoAudioProcessorEditor::keyStateChanged(bool isKeyDown)
{
    if (presetBrowser != nullptr && presetBrowser->isVisible() && presetBrowser->isSearchFocused())
        return false;

    if (!isKeyDown)
    {
        std::vector<int> keysToRelease;
        for (const auto& [charCode, noteNumber] : charCodeToPlayingNote)
        {
            if (!juce::KeyPress::isKeyCurrentlyDown(charCode))
                keysToRelease.push_back(charCode);
        }

        for (int c : keysToRelease)
        {
            auto it = charCodeToPlayingNote.find(c);
            if (it != charCodeToPlayingNote.end())
            {
                int playedNote = it->second;
                charCodeToPlayingNote.erase(it);
                audioProcessor.handleNoteOff(playedNote);
                keyboard.setNoteActive(playedNote, false);
            }
        }
    }
    return true;
}

void LadderMonoAudioProcessorEditor::focusLost(FocusChangeType)
{
    releaseAllHeldComputerKeys();
}

void LadderMonoAudioProcessorEditor::visibilityChanged()
{
    if (!isVisible())
        releaseAllHeldComputerKeys();
}

void LadderMonoAudioProcessorEditor::releaseAllHeldComputerKeys()
{
    for (const auto& [charCode, noteNumber] : charCodeToPlayingNote)
    {
        audioProcessor.handleNoteOff(noteNumber);
        keyboard.setNoteActive(noteNumber, false);
    }
    charCodeToPlayingNote.clear();
}

void LadderMonoAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    grabKeyboardFocus();
    juce::AudioProcessorEditor::mouseDown(e);
}

// ====================================================================
// Setup Helpers with Strict Value Formatting & Real Units
// ====================================================================
void LadderMonoAudioProcessorEditor::setupKnob(Knobby& k, const juce::String& paramId, const juce::String& labelText, bool /*isBipolar*/)
{
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 15);
    k.slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffdcdfe8));
    k.slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff18191d));
    k.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff34363d));
    addAndMakeVisible(k.slider);

    k.label.setText(labelText, juce::dontSendNotification);
    k.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    k.label.setColour(juce::Label::textColourId, juce::Colour(0xffd4d8e2)); // High-contrast label
    k.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(k.label);

    // 1. Attach to APVTS first (so default attachment handler does not overwrite our custom lambdas)
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), paramId, k.slider));

    // 2. Strict value formatting eliminating "-0.00" bug and displaying real engineering units
    if (paramId == LadderMono::ParamIDs::masterVol)
    {
        k.slider.textFromValueFunction = [](double v) {
            if (v <= -59.5) return juce::String("-inf dB");
            double cv = (std::abs(v) < 0.05) ? 0.0 : v;
            return juce::String(cv, 1) + " dB";
        };
        k.slider.valueFromTextFunction = [](const juce::String& t) {
            return t.upToFirstOccurrenceOf("dB", false, false).trim().getDoubleValue();
        };
    }
    else if (paramId == LadderMono::ParamIDs::tune)
    {
        k.slider.textFromValueFunction = [](double v) {
            double cv = (std::abs(v) < 0.05) ? 0.0 : v;
            return (cv > 0.05 ? "+" : "") + juce::String(cv, 1) + " ct";
        };
    }
    else if (paramId == LadderMono::ParamIDs::osc2Fine || paramId == LadderMono::ParamIDs::osc3Fine)
    {
        k.slider.textFromValueFunction = [](double v) {
            double cv = (std::abs(v) < 0.005) ? 0.0 : v;
            return (cv > 0.005 ? "+" : "") + juce::String(cv, 2) + " st";
        };
    }
    else if (paramId == LadderMono::ParamIDs::glideTime)
    {
        k.slider.textFromValueFunction = [](double v) {
            double cv = (std::abs(v) < 0.005) ? 0.0 : v;
            if (cv < 1.0)
                return juce::String(juce::roundToInt(cv * 1000.0)) + " ms";
            return juce::String(cv, 2) + " s";
        };
    }
    else if (paramId == LadderMono::ParamIDs::lfoRate)
    {
        k.slider.textFromValueFunction = [](double v) {
            return juce::String(v, 2) + " Hz";
        };
    }
    else if (paramId == LadderMono::ParamIDs::mixerDrive)
    {
        k.slider.textFromValueFunction = [](double v) {
            double cv = (std::abs(v) < 0.05) ? 0.0 : v;
            return (cv > 0.05 ? "+" : "") + juce::String(cv, 1) + " dB";
        };
    }
    else if (paramId == LadderMono::ParamIDs::cutoff)
    {
        k.slider.textFromValueFunction = [](double v) {
            double cv = (std::abs(v) < 0.005) ? 0.0 : v;
            return (cv > 0.005 ? "+" : "") + juce::String(cv, 2) + " oct";
        };
    }
    else if (paramId == LadderMono::ParamIDs::analogAmount || paramId == LadderMono::ParamIDs::bassComp || paramId == LadderMono::ParamIDs::arpGate)
    {
        k.slider.textFromValueFunction = [](double v) {
            return juce::String(juce::roundToInt(v * 100.0)) + "%";
        };
    }
    else if (paramId == LadderMono::ParamIDs::fAttack || paramId == LadderMono::ParamIDs::aAttack ||
             paramId == LadderMono::ParamIDs::fDecay || paramId == LadderMono::ParamIDs::aDecay)
    {
        k.slider.textFromValueFunction = [](double v) {
            float norm = std::clamp(static_cast<float>(v) / 10.0f, 0.0f, 1.0f);
            float sec = 0.005f + 5.995f * std::pow(norm, 2.4f);
            if (sec < 1.0f)
                return juce::String(juce::roundToInt(sec * 1000.0f)) + " ms";
            return juce::String(sec, 2) + " s";
        };
    }
    else if (paramId == LadderMono::ParamIDs::fSustain || paramId == LadderMono::ParamIDs::aSustain)
    {
        k.slider.textFromValueFunction = [](double v) {
            return juce::String(juce::roundToInt(v * 10.0)) + "%";
        };
    }
    else
    {
        // Mixer volumes & 0-10 analog pots
        k.slider.textFromValueFunction = [](double v) {
            double cv = (std::abs(v) < 0.005) ? 0.0 : v;
            return juce::String(cv, 2);
        };
    }

    if (paramId != LadderMono::ParamIDs::masterVol)
    {
        k.slider.valueFromTextFunction = [](const juce::String& t) {
            auto cleaned = t.retainCharacters("0123456789.-");
            return cleaned.getDoubleValue();
        };
    }

    // 3. Immediately update text display
    k.slider.updateText();
}

void LadderMonoAudioProcessorEditor::setupComboBox(juce::ComboBox& box, const juce::String& paramId, const juce::StringArray& items)
{
    box.addItemList(items, 1);
    box.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1b1f));
    box.setColour(juce::ComboBox::textColourId, juce::Colour(0xffe0e4eb));
    box.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff444750));
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
    presetButton.setButtonText(audioProcessor.getPresetManager().getCurrentPresetName() + "  ▾");
    if (presetBrowser != nullptr)
        presetBrowser->updateFilteredList();
}

// ====================================================================
// Procedural Walnut Wood & Hardware Graphics
// ====================================================================
static void drawWoodGrain(juce::Graphics& g, juce::Rectangle<float> rect, bool isVertical = false)
{
    if (isVertical)
    {
        juce::ColourGradient grad(juce::Colour(0xff55331c), rect.getX(), 0.0f,
                                   juce::Colour(0xff3b2010), rect.getRight(), 0.0f, false);
        grad.addColour(0.3, juce::Colour(0xff6a3e22));
        grad.addColour(0.7, juce::Colour(0xff482814));
        g.setGradientFill(grad);
    }
    else
    {
        juce::ColourGradient grad(juce::Colour(0xff55331c), 0.0f, rect.getY(),
                                   juce::Colour(0xff341c0d), 0.0f, rect.getBottom(), false);
        grad.addColour(0.25, juce::Colour(0xff6e4124));
        grad.addColour(0.65, juce::Colour(0xff452412));
        g.setGradientFill(grad);
    }
    g.fillRect(rect);

    g.setColour(juce::Colour(0x18ffffff));
    if (isVertical)
    {
        for (float x = rect.getX() + 3.0f; x < rect.getRight(); x += 4.5f)
            g.drawVerticalLine(static_cast<int>(x), rect.getY(), rect.getBottom());
    }
    else
    {
        for (float y = rect.getY() + 3.0f; y < rect.getBottom(); y += 5.0f)
            g.drawHorizontalLine(static_cast<int>(y), rect.getX(), rect.getRight());
    }

    g.setColour(juce::Colour(0x35ffffff));
    g.drawHorizontalLine(static_cast<int>(rect.getY()), rect.getX(), rect.getRight());
    g.setColour(juce::Colour(0x66000000));
    g.drawHorizontalLine(static_cast<int>(rect.getBottom() - 1.0f), rect.getX(), rect.getRight());
}

static void drawScrewHead(juce::Graphics& g, float cx, float cy, float radius = 4.2f)
{
    g.setColour(juce::Colour(0x88000000));
    g.fillEllipse(cx - radius - 1.0f, cy - radius - 1.0f, (radius + 1.0f) * 2.0f, (radius + 1.0f) * 2.0f);

    juce::ColourGradient screwGrad(juce::Colour(0xffdcdfe4), cx - radius * 0.4f, cy - radius * 0.4f,
                                   juce::Colour(0xff585c66), cx + radius * 0.6f, cy + radius * 0.6f, false);
    g.setGradientFill(screwGrad);
    g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    g.setColour(juce::Colour(0xff3c3f46));
    g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 0.8f);

    float slotLen = radius * 0.72f;
    g.setColour(juce::Colour(0xff181a1d));
    g.drawLine(cx - slotLen * 0.707f, cy - slotLen * 0.707f,
               cx + slotLen * 0.707f, cy + slotLen * 0.707f, 1.4f);
}

void LadderMonoAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto totalArea = getLocalBounds().toFloat();

    // 1. Base dark background
    g.fillAll(juce::Colour(0xff141517));

    // 2. Walnut Wooden Frame Components
    drawWoodGrain(g, juce::Rectangle<float>(0.0f, 0.0f, totalArea.getWidth(), 42.0f), false);
    drawWoodGrain(g, juce::Rectangle<float>(0.0f, 0.0f, 20.0f, totalArea.getHeight()), true);
    drawWoodGrain(g, juce::Rectangle<float>(totalArea.getWidth() - 20.0f, 0.0f, 20.0f, totalArea.getHeight()), true);
    drawWoodGrain(g, juce::Rectangle<float>(0.0f, 440.0f, totalArea.getWidth(), 42.0f), false);
    drawWoodGrain(g, juce::Rectangle<float>(0.0f, totalArea.getHeight() - 8.0f, totalArea.getWidth(), 8.0f), false);

    // Screws
    drawScrewHead(g, 40.0f, 21.0f);
    drawScrewHead(g, totalArea.getWidth() * 0.5f - 40.0f, 21.0f);
    drawScrewHead(g, totalArea.getWidth() - 40.0f, 21.0f);
    drawScrewHead(g, 35.0f, 461.0f);
    drawScrewHead(g, 125.0f, 461.0f);

    // 3. Anodized Dark Aluminum Faceplate
    juce::Rectangle<float> panelRect(20.0f, 42.0f, totalArea.getWidth() - 40.0f, 398.0f);
    juce::ColourGradient panelGrad(juce::Colour(0xff222429), 0.0f, panelRect.getY(),
                                   juce::Colour(0xff161719), 0.0f, panelRect.getBottom(), false);
    panelGrad.addColour(0.5, juce::Colour(0xff1d1e22));
    g.setGradientFill(panelGrad);
    g.fillRect(panelRect);

    // Brushed aluminum grain lines
    g.setColour(juce::Colour(0x0affffff));
    for (float y = panelRect.getY(); y < panelRect.getBottom(); y += 3.0f)
        g.drawHorizontalLine(static_cast<int>(y), panelRect.getX(), panelRect.getRight());

    g.setColour(juce::Colour(0xff3c404a));
    g.drawRect(panelRect, 1.0f);

    auto drawSectionFrame = [&g](float x, float y, float w, float h, const juce::String& title) {
        juce::Rectangle<float> frame(x, y, w, h);
        g.setColour(juce::Colour(0xff3e424c));
        g.drawRoundedRectangle(frame, 3.0f, 1.2f);

        // Header banner
        g.setColour(juce::Colour(0xff191a1e));
        g.fillRect(x + 2.0f, y + 2.0f, w - 4.0f, 22.0f);
        g.setColour(juce::Colour(0xff4a4f5c));
        g.drawHorizontalLine(static_cast<int>(y + 24.0f), x, x + w);

        // Section Title
        g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
        g.setColour(juce::Colour(0xffe2e5ed));
        g.drawText(title, juce::Rectangle<float>(x, y + 2.0f, w, 22.0f).toNearestInt(), juce::Justification::centred);
    };

    // 5 Clean Classic Hardware Signal-Flow Sections:
    // CONTROLLERS -> OSCILLATOR BANK -> MIXER -> MODIFIERS -> OUTPUT
    drawSectionFrame(24.0f, 46.0f, 176.0f, 388.0f, "CONTROLLERS");
    drawSectionFrame(206.0f, 46.0f, 260.0f, 388.0f, "OSCILLATOR BANK");
    drawSectionFrame(472.0f, 46.0f, 186.0f, 388.0f, "MIXER");
    drawSectionFrame(664.0f, 46.0f, 296.0f, 388.0f, "MODIFIERS");
    drawSectionFrame(966.0f, 46.0f, 130.0f, 388.0f, "OUTPUT");

    // Oscillator Bank row labels (OSC 1, OSC 2, OSC 3)
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xffe0a350));
    g.drawText("OSC 1", 212, 78, 40, 24, juce::Justification::centredLeft);
    g.drawText("OSC 2", 212, 138, 40, 24, juce::Justification::centredLeft);
    g.drawText("OSC 3", 212, 215, 40, 24, juce::Justification::centredLeft);

    // Modifiers High-Contrast Sub-Header Banners
    auto drawSubHeader = [&g](float x, float y, float w, const juce::String& text) {
        g.setColour(juce::Colour(0xff181a1f));
        g.fillRoundedRectangle(x, y, w, 16.0f, 2.0f);
        g.setColour(juce::Colour(0xff3a3e49));
        g.drawRoundedRectangle(x, y, w, 16.0f, 2.0f, 1.0f);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xffe89e4a)); // Warm distinct copper banner text
        g.drawText(text, juce::Rectangle<float>(x, y, w, 16.0f).toNearestInt(), juce::Justification::centred);
    };

    drawSubHeader(672, 204, 280, "FILTER CONTOUR");
    drawSubHeader(672, 290, 280, "LOUDNESS CONTOUR");

    // Output Voices Label
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xffdcdfe8));
    g.drawText("VOICES", 970, 202, 122, 14, juce::Justification::centred);

    // Overload Indicator Lamp (Mixer)
    float ovlX = 632.0f;
    float ovlY = 405.0f;
    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xffd4d8e2));
    g.drawText("OVERLOAD", static_cast<int>(ovlX - 35.0f), static_cast<int>(ovlY - 18.0f), 70, 14, juce::Justification::centred);

    float driveVal = audioProcessor.getAPVTS().getRawParameterValue(LadderMono::ParamIDs::mixerDrive)->load();
    bool isOverload = driveVal > 0.05f;

    g.setColour(juce::Colour(0xff2d3038));
    g.fillEllipse(ovlX - 7.0f, ovlY - 7.0f, 14.0f, 14.0f);
    g.setColour(juce::Colour(0xff555a66));
    g.drawEllipse(ovlX - 7.0f, ovlY - 7.0f, 14.0f, 14.0f, 1.0f);

    if (isOverload)
    {
        g.setColour(juce::Colour(0x66ff1100));
        g.fillEllipse(ovlX - 12.0f, ovlY - 12.0f, 24.0f, 24.0f);
        g.setColour(juce::Colour(0xffff2200));
        g.fillEllipse(ovlX - 5.0f, ovlY - 5.0f, 10.0f, 10.0f);
        g.setColour(juce::Colour(0xffffa090));
        g.fillEllipse(ovlX - 2.0f, ovlY - 2.0f, 4.0f, 4.0f);
    }
    else
    {
        g.setColour(juce::Colour(0xff4a0808));
        g.fillEllipse(ovlX - 5.0f, ovlY - 5.0f, 10.0f, 10.0f);
    }
}

void LadderMonoAudioProcessorEditor::resized()
{
    // ====================================================================
    // Top Bar (y: 0..42)
    // ====================================================================
    titleLabel.setBounds(62, 8, 135, 26);
    versionBtn.setBounds(200, 9, 44, 24);

    presetButton.setBounds(255, 8, 175, 26);
    prevPresetBtn.setBounds(433, 8, 28, 26);
    nextPresetBtn.setBounds(463, 8, 28, 26);
    browseBtn.setBounds(495, 8, 58, 26);
    saveBtn.setBounds(556, 8, 48, 26);
    initBtn.setBounds(607, 8, 42, 26);

    abBtn.setBounds(658, 8, 54, 26);
    hqBtn.setBounds(716, 8, 40, 26);
    cpuLabel.setBounds(762, 8, 70, 26);
    aboutBtn.setBounds(getWidth() - 110, 8, 62, 26);

    // ====================================================================
    // Section 1: CONTROLLERS (x: 24..200, w: 176)
    // ====================================================================
    // Row 1: Tune & Glide Time
    tuneKnob.slider.setBounds(34, 72, 54, 54);
    tuneKnob.label.setBounds(34, 126, 54, 14);

    glideKnob.slider.setBounds(124, 72, 54, 54);
    glideKnob.label.setBounds(124, 126, 54, 14);

    // Row 2: Glide Switch & Glide Mode
    glideOnBtn.setBounds(32, 148, 78, 28);
    glideModeBox.setBounds(118, 150, 74, 24);

    // Row 3: Mod Mix & LFO Rate
    modMixKnob.slider.setBounds(34, 184, 54, 54);
    modMixKnob.label.setBounds(34, 238, 54, 14);

    lfoRateKnob.slider.setBounds(124, 184, 54, 54);
    lfoRateKnob.label.setBounds(124, 238, 54, 14);

    // Row 4: Mod Source & LFO Shape
    modSourceBox.setBounds(32, 260, 78, 24);
    lfoShapeBox.setBounds(118, 260, 74, 24);

    // Row 5: Osc Mod & Filter Mod switches
    oscModBtn.setBounds(32, 296, 78, 30);
    filterModBtn.setBounds(118, 296, 78, 30);

    // Row 6: Bend Range & Note Priority
    bendRangeBox.setBounds(32, 340, 78, 24);
    notePriorityBox.setBounds(118, 340, 74, 24);

    // ====================================================================
    // Section 2: OSCILLATOR BANK (x: 206..466, w: 260)
    // ====================================================================
    // Osc 1
    osc1RangeBox.setBounds(256, 78, 88, 24);
    osc1WaveBox.setBounds(354, 78, 98, 24);

    // Osc 2 (with its own FINE knob on the same row)
    osc2RangeBox.setBounds(256, 138, 72, 24);
    osc2WaveBox.setBounds(334, 138, 72, 24);
    osc2FineKnob.slider.setBounds(412, 122, 48, 48);
    osc2FineKnob.label.setBounds(412, 170, 48, 12);

    // Osc 3 (with its own FINE knob and OSC 3 KBD on the same section)
    osc3RangeBox.setBounds(256, 215, 72, 24);
    osc3WaveBox.setBounds(334, 215, 72, 24);
    osc3FineKnob.slider.setBounds(412, 199, 48, 48);
    osc3FineKnob.label.setBounds(412, 247, 48, 12);

    osc3KbdBtn.setBounds(256, 250, 110, 30);

    // DRIFT Knob (Analog Component Tolerance & Thermal Drift - moved from Output)
    analogKnob.slider.setBounds(325, 308, 62, 62);
    analogKnob.label.setBounds(325, 370, 62, 14);

    // ====================================================================
    // Section 3: MIXER (x: 472..658, w: 186)
    // ====================================================================
    mix1Knob.slider.setBounds(480, 70, 50, 50);
    mix1Knob.label.setBounds(480, 120, 50, 14);
    mix1Btn.setBounds(548, 78, 75, 34);

    mix2Knob.slider.setBounds(480, 134, 50, 50);
    mix2Knob.label.setBounds(480, 184, 50, 14);
    mix2Btn.setBounds(548, 142, 75, 34);

    mix3Knob.slider.setBounds(480, 198, 50, 50);
    mix3Knob.label.setBounds(480, 248, 50, 14);
    mix3Btn.setBounds(548, 206, 75, 34);

    mixNoiseKnob.slider.setBounds(480, 262, 50, 50);
    mixNoiseKnob.label.setBounds(480, 312, 50, 14);
    mixNoiseBtn.setBounds(548, 264, 75, 34);
    noiseColorBox.setBounds(548, 302, 75, 22);

    // Clean Feed & Drive (ZERO overlap between FEED switch and DRIVE knob)
    mixExtKnob.slider.setBounds(480, 332, 48, 48);
    mixExtKnob.label.setBounds(480, 380, 48, 14);
    mixExtBtn.setBounds(548, 338, 75, 34);

    driveKnob.slider.setBounds(538, 380, 46, 46);
    driveKnob.label.setBounds(538, 426, 46, 12);

    // ====================================================================
    // Section 4: MODIFIERS (x: 664..960, w: 296)
    // ====================================================================
    // Section 4: MODIFIERS (x: 664..960, w: 296)
    // ====================================================================
    // Filter Core
    cutoffKnob.slider.setBounds(676, 68, 58, 58);
    cutoffKnob.label.setBounds(676, 126, 58, 14);

    emphasisKnob.slider.setBounds(754, 68, 58, 58);
    emphasisKnob.label.setBounds(754, 126, 58, 14);

    contourKnob.slider.setBounds(832, 68, 58, 58);
    contourKnob.label.setBounds(822, 126, 78, 14); // CONTOUR AMT

    kbd1Btn.setBounds(678, 148, 68, 30);
    kbd2Btn.setBounds(754, 148, 68, 30);

    // Bass Comp (cleanly separated from keyboard switches)
    bassCompKnob.slider.setBounds(838, 140, 48, 48);
    bassCompKnob.label.setBounds(832, 186, 66, 13);

    // Filter Env (Row 3)
    fAttackKnob.slider.setBounds(678, 224, 52, 52);
    fAttackKnob.label.setBounds(678, 274, 52, 13);
    fDecayKnob.slider.setBounds(754, 224, 52, 52);
    fDecayKnob.label.setBounds(754, 274, 52, 13);
    fSustainKnob.slider.setBounds(830, 224, 52, 52);
    fSustainKnob.label.setBounds(830, 274, 52, 13);

    // Amp Env (Row 4)
    aAttackKnob.slider.setBounds(678, 308, 52, 52);
    aAttackKnob.label.setBounds(678, 358, 52, 13);
    aDecayKnob.slider.setBounds(754, 308, 52, 52);
    aDecayKnob.label.setBounds(754, 358, 52, 13);
    aSustainKnob.slider.setBounds(830, 308, 52, 52);
    aSustainKnob.label.setBounds(830, 358, 52, 13);

    decaySwitchBtn.setBounds(745, 388, 106, 32);

    // ====================================================================
    // Section 5: OUTPUT & MASTER (x: 966..1096, w: 130)
    // ====================================================================
    masterVolKnob.slider.setBounds(992, 72, 78, 78);
    masterVolKnob.label.setBounds(992, 150, 78, 14);

    voicesBox.setBounds(984, 224, 94, 26);
    bypassBtn.setBounds(984, 318, 94, 30);

    // ====================================================================
    // Middle Crossbar: ARPEGGIATOR & PERFORMANCE BAR (y: 440..482)
    // ====================================================================
    arpSectionLabel.setBounds(48, 448, 76, 24);
    arpOnBtn.setBounds(126, 446, 68, 28);
    arpLatchBtn.setBounds(196, 446, 64, 28);

    arpModeLabel.setBounds(264, 452, 34, 16);
    arpModeBox.setBounds(300, 448, 76, 24);

    arpOctaveLabel.setBounds(380, 452, 26, 16);
    arpOctaveBox.setBounds(408, 448, 64, 24);

    arpRateLabel.setBounds(476, 452, 30, 16);
    arpRateBox.setBounds(508, 448, 64, 24);

    arpGateKnob.slider.setBounds(576, 441, 38, 38);
    arpGateKnob.label.setBounds(616, 452, 38, 16);

    keyboardHintLabel.setBounds(660, 448, getWidth() - 670, 24);

    // ====================================================================
    // Lower Performance Section (y: 482..672)
    // ====================================================================
    pitchWheel.setBounds(28, 492, 44, 160);
    modWheel.setBounds(78, 492, 44, 160);
    keyboard.setBounds(136, 482, getWidth() - 156, 190);

    // Resizer handle in bottom-right corner
    if (resizer != nullptr)
        resizer->setBounds(getWidth() - 16, getHeight() - 16, 16, 16);

    if (presetBrowser != nullptr)
        presetBrowser->setBounds(getLocalBounds());
    if (aboutDialog != nullptr)
        aboutDialog->setBounds(getLocalBounds());
}
