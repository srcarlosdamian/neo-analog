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
    titleLabel.setColour(juce::Label::textColourId, currentTheme.textPrimary);
    addAndMakeVisible(titleLabel);

    versionBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    versionBtn.setColour(juce::TextButton::buttonOnColourId, currentTheme.lineAccent);
    versionBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textSecondary);
    versionBtn.setTooltip("Neo Analog v1.1 \xe2\x80\x94 Click for Credits & DSP Info");
    versionBtn.onClick = [this] { showAboutDialog(); };
    addAndMakeVisible(versionBtn);

    // Preset selector button
    presetButton.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    presetButton.setColour(juce::TextButton::buttonOnColourId, currentTheme.lineAccent);
    presetButton.setColour(juce::TextButton::textColourOffId, currentTheme.textPrimary);
    presetButton.onClick = [this]() {
        juce::PopupMenu menu;
        menu.addItem(9999, "Open Preset Browser...", true, false);
        menu.addSeparator();

        std::map<juce::String, std::vector<std::pair<int, juce::String>>> categorized;
        const auto& presets = audioProcessor.getPresetManager().getPresets();
        for (size_t i = 0; i < presets.size(); ++i)
        {
            categorized[presets[i].category].emplace_back(static_cast<int>(i + 1), presets[i].name);
        }

        for (const auto& [cat, list] : categorized)
        {
            if (cat == "Init") continue;
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

    prevPresetBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    prevPresetBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textSecondary);
    prevPresetBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
        audioProcessor.getPresetManager().loadPrevPreset();
        updatePresetDisplay();
    };
    addAndMakeVisible(prevPresetBtn);

    nextPresetBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    nextPresetBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textSecondary);
    nextPresetBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
        audioProcessor.getPresetManager().loadNextPreset();
        updatePresetDisplay();
    };
    addAndMakeVisible(nextPresetBtn);

    browseBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    browseBtn.setColour(juce::TextButton::buttonOnColourId, currentTheme.lineAccent);
    browseBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textPrimary);
    browseBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
        if (presetBrowser != nullptr)
            presetBrowser->showBrowser();
    };
    addAndMakeVisible(browseBtn);

    saveBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    saveBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textPrimary);
    saveBtn.setTooltip("Save / Export Preset (.nlog file)");
    saveBtn.onClick = [this] { saveCurrentPresetDialog(); };
    addAndMakeVisible(saveBtn);

    initBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    initBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textSecondary);
    initBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
        audioProcessor.getPresetManager().initPatch();
        updatePresetDisplay();
    };
    addAndMakeVisible(initBtn);

    // A/B Comparison Toggle
    abBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    abBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textPrimary);
    abBtn.setTooltip("A/B State Compare — Switch between Patch A and Patch B");
    abBtn.onClick = [this] { toggleABState(); };
    addAndMakeVisible(abBtn);

    // High Quality Oversampling Toggle
    hqBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    hqBtn.setColour(juce::TextButton::buttonOnColourId, currentTheme.lineAccent);
    hqBtn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffffff));
    hqBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textSecondary);
    hqBtn.setClickingTogglesState(true);
    hqBtn.setTooltip("HQ Mode (2x Oversampling & Anti-Aliasing)");
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), LadderMono::ParamIDs::hqMode, hqBtn));
    addAndMakeVisible(hqBtn);

    // CPU Usage Meter
    cpuLabel.setText("CPU: 0.0%", juce::dontSendNotification);
    cpuLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    cpuLabel.setColour(juce::Label::textColourId, currentTheme.textSecondary);
    cpuLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cpuLabel);

    editThemeBtn.setButtonText(juce::CharPointer_UTF8("\xe2\x9c\x8e Theme"));
    editThemeBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    editThemeBtn.setColour(juce::TextButton::buttonOnColourId, currentTheme.lineAccent);
    editThemeBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textPrimary);
    editThemeBtn.setTooltip("Theme Studio \xe2\x80\x94 Custom Live Palette & Color Editor");
    editThemeBtn.onClick = [this] { toggleThemeEditor(); };
    addAndMakeVisible(editThemeBtn);

    aboutBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    aboutBtn.setColour(juce::TextButton::buttonOnColourId, currentTheme.lineAccent);
    aboutBtn.setColour(juce::TextButton::textColourOffId, currentTheme.textSecondary);
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
    setupKnob(spatialKnob, LadderMono::ParamIDs::spatialFilter, "SPATIAL");
    setupToggle(kbd1Btn, LadderMono::ParamIDs::kbd1);
    setupToggle(kbd2Btn, LadderMono::ParamIDs::kbd2);
    setupComboBox(filterModelBox, LadderMono::ParamIDs::filterModel, {"Saturated ZDF", "Clean ZDF", "Huovilainen", "Model 72 Drive"});

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

    bypassBtn.setToggleState(audioProcessor.isBypassed(), juce::dontSendNotification);
    bypassBtn.setTheme(currentTheme);
    bypassBtn.onClick = [this] {
        bool isByp = bypassBtn.getToggleState();
        audioProcessor.setBypassed(isByp);
    };
    addAndMakeVisible(bypassBtn);

    // ====================================================================
    // Middle Strip: ARPEGGIATOR & PERFORMANCE BAR
    // ====================================================================
    arpSectionLabel.setText("ARPEGGIATOR", juce::dontSendNotification);
    arpSectionLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    arpSectionLabel.setColour(juce::Label::textColourId, currentTheme.textPrimary);
    addAndMakeVisible(arpSectionLabel);

    setupToggle(arpOnBtn, LadderMono::ParamIDs::arpOn);
    setupToggle(arpLatchBtn, LadderMono::ParamIDs::arpLatch);

    arpModeLabel.setText("MODE", juce::dontSendNotification);
    arpModeLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    arpModeLabel.setColour(juce::Label::textColourId, currentTheme.textSecondary);
    addAndMakeVisible(arpModeLabel);
    setupComboBox(arpModeBox, LadderMono::ParamIDs::arpMode, {"Up", "Down", "Up/Down", "Random", "Order"});

    arpOctaveLabel.setText("OCT", juce::dontSendNotification);
    arpOctaveLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    arpOctaveLabel.setColour(juce::Label::textColourId, currentTheme.textSecondary);
    addAndMakeVisible(arpOctaveLabel);
    setupComboBox(arpOctaveBox, LadderMono::ParamIDs::arpOctaves, {"1 Oct", "2 Oct", "3 Oct", "4 Oct"});

    arpOctModeLabel.setText("OCT DIR", juce::dontSendNotification);
    arpOctModeLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    arpOctModeLabel.setColour(juce::Label::textColourId, currentTheme.textSecondary);
    addAndMakeVisible(arpOctModeLabel);
    setupComboBox(arpOctModeBox, LadderMono::ParamIDs::arpOctMode, {"Ascend", "Alternate", "Spiral"});

    arpRateLabel.setText("RATE", juce::dontSendNotification);
    arpRateLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    arpRateLabel.setColour(juce::Label::textColourId, currentTheme.textSecondary);
    addAndMakeVisible(arpRateLabel);
    setupComboBox(arpRateBox, LadderMono::ParamIDs::arpRateSync, {"1/4", "1/8", "1/8 T", "1/16", "1/16 T", "1/32"});

    arpPatternLabel.setText("PATTERN", juce::dontSendNotification);
    arpPatternLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    arpPatternLabel.setColour(juce::Label::textColourId, currentTheme.textSecondary);
    addAndMakeVisible(arpPatternLabel);
    setupComboBox(arpPatternBox, LadderMono::ParamIDs::arpPattern, {"Straight", "Dotted", "Syncopated", "Euclid 3/8", "Euclid 5/8"});

    setupKnob(arpGateKnob, LadderMono::ParamIDs::arpGate, "GATE");
    arpGateKnob.slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 18);

    setupKnob(arpSwingKnob, LadderMono::ParamIDs::arpSwing, "SWING");
    arpSwingKnob.slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);

    setupKnob(arpMutateKnob, LadderMono::ParamIDs::arpMutate, "MUTATE");
    arpMutateKnob.slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 18);

    // Performance Wheels
    pitchWheel.setSliderStyle(juce::Slider::LinearVertical);
    pitchWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pitchWheel.setRange(-1.0, 1.0, 0.01);
    pitchWheel.setValue(0.0);
    pitchWheel.setDoubleClickReturnValue(true, 0.0);
    pitchWheel.onValueChange = [this]() {
        float normVal = static_cast<float>(pitchWheel.getValue());
        int bendRange = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue(LadderMono::ParamIDs::bendRange)->load());
        float semitones = normVal * bendRange;
        for (auto& v : audioProcessor.getVoices())
            v.setPitchBend(semitones);
    };
    // Physical spring-loaded return to center (0.0) upon mouse release
    pitchWheel.onDragEnd = [this]() {
        pitchWheel.setValue(0.0, juce::sendNotificationSync);
    };
    addAndMakeVisible(pitchWheel);

    pitchWheelLabel.setText("PITCH", juce::dontSendNotification);
    pitchWheelLabel.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    pitchWheelLabel.setColour(juce::Label::textColourId, currentTheme.textSecondary);
    pitchWheelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(pitchWheelLabel);

    modWheel.setSliderStyle(juce::Slider::LinearVertical);
    modWheel.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    modWheel.setRange(0.0, 1.0, 0.01);
    modWheel.setValue(0.0);
    modWheel.setDoubleClickReturnValue(true, 0.0);
    modWheel.onValueChange = [this]() {
        float mw = static_cast<float>(modWheel.getValue());
        audioProcessor.setModWheelAmount(mw);
        for (auto& v : audioProcessor.getVoices())
            v.setModWheel(mw);
        updateHelpBarForComponent(&modWheel);
    };
    addAndMakeVisible(modWheel);

    modWheelLabel.setText("MOD", juce::dontSendNotification);
    modWheelLabel.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    modWheelLabel.setColour(juce::Label::textColourId, currentTheme.textSecondary);
    modWheelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modWheelLabel);

    // Virtual Keyboard
    keyboard.setBaseOctaveNote(baseOctaveNote);
    keyboard.onNoteOn = [this](int note, float vel) {
        audioProcessor.handleNoteOn(note, vel);
    };
    keyboard.onNoteOff = [this](int note) {
        audioProcessor.handleNoteOff(note);
    };
    addAndMakeVisible(keyboard);

    presetBrowser = std::make_unique<LadderMono::PresetBrowserOverlay>(
        audioProcessor.getPresetManager(),
        [this] { updatePresetDisplay(); }
    );
    addChildComponent(*presetBrowser);

    aboutDialog = std::make_unique<LadderMono::AboutDialogOverlay>([this] {
        grabKeyboardFocus();
    });
    addChildComponent(*aboutDialog);

    themeEditorPanel = std::make_unique<LadderMono::ThemeEditorPanel>();
    themeEditorPanel->onThemeChanged = [this](const LadderMono::ThemeColors& newTheme) {
        applyTheme(newTheme);
    };
    addChildComponent(*themeEditorPanel);

    // Load saved custom theme if present
    auto themeFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                         .getChildFile("NeoAnalog").getChildFile("custom_theme.json");
    LadderMono::ThemeColors loadedTheme = LadderMono::ThemeColors::getDefaultNoir();
    if (themeFile.existsAsFile())
        loadedTheme.loadFromFile(themeFile);
    applyTheme(loadedTheme);

    // Corner Resizer & Host Aspect-Ratio Resizing Setup
    setResizable(true, true);
    setResizeLimits(952, 578, 1680, 1020);
    if (getConstrainer() != nullptr)
        getConstrainer()->setFixedAspectRatio(1120.0 / 680.0);

    constrainer.setSizeLimits(952, 578, 1680, 1020);
    constrainer.setFixedAspectRatio(1120.0 / 680.0);
    resizer = std::make_unique<LadderMono::CornerResizer>(this, getConstrainer() != nullptr ? getConstrainer() : &constrainer);
    resizer->setTheme(loadedTheme);
    addAndMakeVisible(*resizer);

    // Bottom Parameter Help Bar
    helpBar = std::make_unique<LadderMono::ParameterHelpBar>();
    helpBar->setTheme(loadedTheme);
    addAndMakeVisible(*helpBar);

    // Universal mouse listener for parameter inspector hover hints
    addMouseListener(this, true);

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
    removeMouseListener(this);
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
        cpuLabel.setColour(juce::Label::textColourId, currentTheme.textSecondary);

    // Repaint overload LED if mixer drive changed
    repaint(472, 46, 186, 388);
}

void LadderMonoAudioProcessorEditor::pushUndoSnapshot()
{
    if (isPerformingUndoRedo) return;
    auto snap = audioProcessor.getAPVTS().copyState();
    undoStack.push_back(snap);
    if (undoStack.size() > 50)
        undoStack.erase(undoStack.begin());
    redoStack.clear();
}

void LadderMonoAudioProcessorEditor::undo()
{
    if (undoStack.empty()) return;
    isPerformingUndoRedo = true;
    auto current = audioProcessor.getAPVTS().copyState();
    redoStack.push_back(current);

    auto prev = undoStack.back();
    undoStack.pop_back();
    audioProcessor.getAPVTS().replaceState(prev);
    updatePresetDisplay();
    isPerformingUndoRedo = false;
}

void LadderMonoAudioProcessorEditor::redo()
{
    if (redoStack.empty()) return;
    isPerformingUndoRedo = true;
    auto current = audioProcessor.getAPVTS().copyState();
    undoStack.push_back(current);

    auto next = redoStack.back();
    redoStack.pop_back();
    audioProcessor.getAPVTS().replaceState(next);
    updatePresetDisplay();
    isPerformingUndoRedo = false;
}

void LadderMonoAudioProcessorEditor::toggleABState()
{
    pushUndoSnapshot();
    if (isShowingA)
    {
        // Save current to A, load B
        stateA = audioProcessor.getAPVTS().copyState();
        if (!stateB.isValid())
            stateB = stateA.createCopy();
        audioProcessor.getAPVTS().replaceState(stateB);
        isShowingA = false;
        abBtn.setButtonText("A/B: B");
        abBtn.setColour(juce::TextButton::buttonColourId, currentTheme.buttonBg);
    }
    else
    {
        // Save current to B, load A
        stateB = audioProcessor.getAPVTS().copyState();
        audioProcessor.getAPVTS().replaceState(stateA);
        isShowingA = true;
        abBtn.setButtonText("A/B: A");
        abBtn.setColour(juce::TextButton::buttonColourId, currentTheme.comboBg);
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

void LadderMonoAudioProcessorEditor::applyTheme(const LadderMono::ThemeColors& t)
{
    currentTheme = t;
    lnf.setTheme(t);
    keyboard.setTheme(t);

    // Apply normalized colors across buttons & labels
    titleLabel.setColour(juce::Label::textColourId, t.textPrimary);
    versionBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    versionBtn.setColour(juce::TextButton::textColourOffId, t.textSecondary);
    cpuLabel.setColour(juce::Label::textColourId, t.textSecondary);

    presetButton.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    presetButton.setColour(juce::TextButton::textColourOffId, t.textPrimary);
    prevPresetBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    prevPresetBtn.setColour(juce::TextButton::textColourOffId, t.textSecondary);
    nextPresetBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    nextPresetBtn.setColour(juce::TextButton::textColourOffId, t.textSecondary);

    browseBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    browseBtn.setColour(juce::TextButton::textColourOffId, t.textPrimary);
    saveBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    saveBtn.setColour(juce::TextButton::textColourOffId, t.textPrimary);
    initBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    initBtn.setColour(juce::TextButton::textColourOffId, t.textSecondary);

    abBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    abBtn.setColour(juce::TextButton::textColourOffId, t.textPrimary);
    hqBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    hqBtn.setColour(juce::TextButton::buttonOnColourId, t.lineAccent);
    hqBtn.setColour(juce::TextButton::textColourOffId, t.textSecondary);
    hqBtn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffffff));

    editThemeBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    editThemeBtn.setColour(juce::TextButton::textColourOffId, t.textPrimary);

    aboutBtn.setColour(juce::TextButton::buttonColourId, t.buttonBg);
    aboutBtn.setColour(juce::TextButton::textColourOffId, t.textSecondary);

    bypassBtn.setTheme(t);
    if (resizer != nullptr)
        resizer->setTheme(t);
    if (helpBar != nullptr)
        helpBar->setTheme(t);

    arpSectionLabel.setColour(juce::Label::textColourId, t.textPrimary);
    arpModeLabel.setColour(juce::Label::textColourId, t.textSecondary);
    arpOctaveLabel.setColour(juce::Label::textColourId, t.textSecondary);
    arpOctModeLabel.setColour(juce::Label::textColourId, t.textSecondary);
    arpRateLabel.setColour(juce::Label::textColourId, t.textSecondary);
    arpPatternLabel.setColour(juce::Label::textColourId, t.textSecondary);

    pitchWheelLabel.setFont(juce::FontOptions(9.0f * t.fontScale, juce::Font::bold));
    pitchWheelLabel.setColour(juce::Label::textColourId, t.textSecondary);
    modWheelLabel.setFont(juce::FontOptions(9.0f * t.fontScale, juce::Font::bold));
    modWheelLabel.setColour(juce::Label::textColourId, t.textSecondary);

    auto settingsDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("NeoAnalog");
    settingsDir.createDirectory();
    t.saveToFile(settingsDir.getChildFile("custom_theme.json"));

    repaint();
}

void LadderMonoAudioProcessorEditor::toggleThemeEditor()
{
    if (themeEditorPanel == nullptr) return;
    bool willBeVisible = !themeEditorPanel->isVisible();
    if (willBeVisible)
    {
        themeEditorPanel->setTheme(currentTheme);
        themeEditorPanel->toFront(true);
    }
    themeEditorPanel->setVisible(willBeVisible);
}

void LadderMonoAudioProcessorEditor::simulateHoverTune()
{
    updateHelpBarForComponent(&tuneKnob.slider);
}

void LadderMonoAudioProcessorEditor::mouseMove(const juce::MouseEvent& e)
{
    updateHelpBarForComponent(e.eventComponent);
}

void LadderMonoAudioProcessorEditor::mouseEnter(const juce::MouseEvent& e)
{
    updateHelpBarForComponent(e.eventComponent);
}

void LadderMonoAudioProcessorEditor::mouseExit(const juce::MouseEvent& e)
{
    if (e.eventComponent == this && helpBar != nullptr)
        helpBar->setHelpText("Hover over any parameter for instant functional guide & DSP signal flow notes.", "");
}

void LadderMonoAudioProcessorEditor::updateHelpBarForComponent(juce::Component* comp)
{
    if (helpBar == nullptr) return;
    if (comp == nullptr)
    {
        helpBar->setHelpText("Hover over any parameter for instant functional guide & DSP signal flow notes.", "");
        return;
    }

    juce::Component* c = comp;
    while (c != nullptr && c != this)
    {
        // Controllers Section
        if (c == &tuneKnob.slider || c == &tuneKnob.label)
        {
            helpBar->setHelpText("Afinación general: Sube o baja el tono de todo el sintetizador para afinar con otros instrumentos.", "Tune");
            return;
        }
        if (c == &glideKnob.slider || c == &glideKnob.label)
        {
            helpBar->setHelpText("Velocidad de resbalón (Glide): Qué tan lento o rápido resbala el tono al cambiar de una nota a otra.", "Glide Time");
            return;
        }
        if (c == &glideOnBtn)
        {
            helpBar->setHelpText("Activa o desactiva el resbalón suave entre notas (estilo silbido o bajo continuo).", "Glide Enable");
            return;
        }
        if (c == &glideModeBox)
        {
            helpBar->setHelpText("Modo Glide: 'Always' resbala siempre; 'Legato' solo resbala si tocas una tecla sin soltar la anterior.", "Glide Mode");
            return;
        }
        if (c == &modMixKnob.slider || c == &modMixKnob.label)
        {
            helpBar->setHelpText("Mezcla de modulación: Hacia la izquierda modula con un pulso u onda suave; a la derecha agrega viento o textura ruidosa.", "Mod Mix");
            return;
        }
        if (c == &modSourceBox)
        {
            helpBar->setHelpText("Origen del efecto: Elige si la modulación la genera el LFO suave o el Oscilador 3 para efectos más agresivos.", "Mod Source");
            return;
        }
        if (c == &oscModBtn)
        {
            helpBar->setHelpText("Modula el tono: Hace vibrar el tono (vibrato estilo violín/guitarra) al mover la rueda de Modulación.", "Osc Mod Switch");
            return;
        }
        if (c == &filterModBtn)
        {
            helpBar->setHelpText("Modula el brillo: Hace que el filtro se abra y cierre rítmicamente (efecto 'wah-wah') al subir la rueda de Modulación.", "Filter Mod Switch");
            return;
        }
        if (c == &lfoRateKnob.slider || c == &lfoRateKnob.label)
        {
            helpBar->setHelpText("Velocidad del efecto (LFO): Qué tan rápido late o vibra el sonido (de ondas lentas a un zumbido rápido).", "LFO Rate");
            return;
        }
        if (c == &lfoShapeBox)
        {
            helpBar->setHelpText("Forma de vibración: 'Triangle' para un vaivén suave y ondulado; 'Square' para un salto seco de dos notas estilo sirena.", "LFO Shape");
            return;
        }
        if (c == &bendRangeBox)
        {
            helpBar->setHelpText("Límite de la palanca de pitch: Cuántas notas hacia arriba o abajo puede estirar el tono la rueda de Pitch.", "Bend Range");
            return;
        }
        if (c == &notePriorityBox)
        {
            helpBar->setHelpText("Prioridad de tecla: Decide qué nota suena si tocas varias a la vez (la más grave, la más aguda o la última tocada).", "Note Priority");
            return;
        }

        // Oscillator Bank
        if (c == &osc1RangeBox)
        {
            helpBar->setHelpText("Octava del Osc 1: 32' y 16' para bajos profundos y oscuros; 8' tono natural; 4' y 2' para notas agudas.", "Osc 1 Octave");
            return;
        }
        if (c == &osc1WaveBox)
        {
            helpBar->setHelpText("Timbre del Osc 1: Triángulo (flauta suave), Sierra (rasposo y brillante), Cuadrada (hueco estilo clarinete o chiptune).", "Osc 1 Waveform");
            return;
        }
        if (c == &osc2RangeBox)
        {
            helpBar->setHelpText("Octava del Osc 2: Combínalo en otra octava con el Osc 1 para un sonido el doble de robusto y lleno.", "Osc 2 Octave");
            return;
        }
        if (c == &osc2WaveBox)
        {
            helpBar->setHelpText("Timbre del Osc 2: Dale otra textura (sierra, cuadrada o pulso fino) para mezclar colores sonoros.", "Osc 2 Waveform");
            return;
        }
        if (c == &osc2FineKnob.slider || c == &osc2FineKnob.label)
        {
            helpBar->setHelpText("Desafinación fina Osc 2: Desafínalo un poquito para engordar el sonido y darle un efecto 'Chorus' vivo y ancho.", "Osc 2 Fine");
            return;
        }
        if (c == &osc3RangeBox)
        {
            helpBar->setHelpText("Octava del Osc 3: Úsalo como un tercer sonido musical, o ponlo en 'LO' para usarlo como generador de efectos lentos.", "Osc 3 Octave");
            return;
        }
        if (c == &osc3WaveBox)
        {
            helpBar->setHelpText("Timbre del Osc 3: Selecciona la forma de onda que aportará al sonido o a la modulación interna.", "Osc 3 Waveform");
            return;
        }
        if (c == &osc3FineKnob.slider || c == &osc3FineKnob.label)
        {
            helpBar->setHelpText("Desafinación fina Osc 3: Desafínalo respecto al Osc 1 y 2 para lograr el legendario sonido trifónico denso de Moog.", "Osc 3 Fine");
            return;
        }
        if (c == &osc3KbdBtn)
        {
            helpBar->setHelpText("Seguimiento de teclado Osc 3: Si lo apagas, el Osc 3 mantendrá siempre el mismo tono sin importar qué tecla toques.", "Osc 3 Kbd Track");
            return;
        }
        if (c == &analogKnob.slider || c == &analogKnob.label)
        {
            helpBar->setHelpText("Inestabilidad analógica (Drift): Imita los circuitos antiguos con microdesafinaciones que hacen que el sonido no suene rígido ni digital.", "Analog Drift");
            return;
        }

        // Mixer
        if (c == &mix1Knob.slider || c == &mix1Knob.label)
        {
            helpBar->setHelpText("Volumen Osc 1: Súbelo de 5.0 para saturar el circuito analógico con calor y armónicos ricos.", "Osc 1 Volume");
            return;
        }
        if (c == &mix1Btn)
        {
            helpBar->setHelpText("Interruptor del Oscilador 1: Enciende o apaga al instante la voz 1 en la mezcla.", "Osc 1 Switch");
            return;
        }
        if (c == &mix2Knob.slider || c == &mix2Knob.label)
        {
            helpBar->setHelpText("Volumen Osc 2: Ajusta cuánto cuerpo añade la segunda capa de sonido a la mezcla.", "Osc 2 Volume");
            return;
        }
        if (c == &mix2Btn)
        {
            helpBar->setHelpText("Interruptor del Oscilador 2: Enciende o apaga la voz 2 en la mezcla.", "Osc 2 Switch");
            return;
        }
        if (c == &mix3Knob.slider || c == &mix3Knob.label)
        {
            helpBar->setHelpText("Volumen Osc 3: Añade o quita la tercera voz. Apágalo si vas a usar el Osc 3 solo como modulador.", "Osc 3 Volume");
            return;
        }
        if (c == &mix3Btn)
        {
            helpBar->setHelpText("Interruptor del Oscilador 3: Enciende o apaga la voz 3 en la mezcla.", "Osc 3 Switch");
            return;
        }
        if (c == &mixNoiseKnob.slider || c == &mixNoiseKnob.label)
        {
            helpBar->setHelpText("Volumen de Ruido: Añade textura soplada, aire, viento o un golpe tipo caja/percusión.", "Noise Volume");
            return;
        }
        if (c == &mixNoiseBtn)
        {
            helpBar->setHelpText("Interruptor de Ruido: Enciende o silencia el generador de ruido ambiental.", "Noise Switch");
            return;
        }
        if (c == &noiseColorBox)
        {
            helpBar->setHelpText("Color del ruido: 'White' es brillante y siseante como TV sin señal; 'Pink' es grave y cálido como cascada o lluvia.", "Noise Spectrum");
            return;
        }
        if (c == &mixExtKnob.slider || c == &mixExtKnob.label)
        {
            helpBar->setHelpText("Volumen de Feedback/Entrada: Realimenta la salida hacia la entrada para engordar los graves y rugir.", "Ext / Feedback Level");
            return;
        }
        if (c == &mixExtBtn)
        {
            helpBar->setHelpText("Truco de Feedback: Emula conectar los audífonos a la entrada externa para un bajo superpesado y saturado.", "Feedback Loop");
            return;
        }
        if (c == &driveKnob.slider || c == &driveKnob.label)
        {
            helpBar->setHelpText("Saturación analógica (Drive): Pasa de un sonido limpio a una distorsión cálida, compacta y agresiva.", "Mixer Drive");
            return;
        }

        // Modifiers (Filter & Envelopes)
        if (c == &cutoffKnob.slider || c == &cutoffKnob.label)
        {
            helpBar->setHelpText("Brillo (Cutoff): Cerrado suena oscuro y apagado (bajo puro); abierto suena brillante, afilado y claro.", "Cutoff Frequency");
            return;
        }
        if (c == &emphasisKnob.slider || c == &emphasisKnob.label)
        {
            helpBar->setHelpText("Resonancia (Emphasis): Resalta el borde del brillo con un silbido metálico o efecto 'láser'. Al máximo silba solo.", "Emphasis Resonance");
            return;
        }
        if (c == &contourKnob.slider || c == &contourKnob.label)
        {
            helpBar->setHelpText("Intensidad del envolvente: Determina qué tanto 'se abre de golpe' el brillo al presionar cada tecla (el 'plop' o 'pluck').", "Contour Amount");
            return;
        }
        if (c == &bassCompKnob.slider || c == &bassCompKnob.label)
        {
            helpBar->setHelpText("Compensador de bajos: Evita que el sonido pierda fuerza y peso en los subgraves cuando subes la resonancia.", "Bass Compensation");
            return;
        }
        if (c == &spatialKnob.slider || c == &spatialKnob.label)
        {
            helpBar->setHelpText("Efecto Espacial: Abre el sonido en estéreo hacia los lados, creando una sensación de amplitud y sala 3D.", "Spatial Filter");
            return;
        }
        if (c == &kbd1Btn)
        {
            helpBar->setHelpText("Seguimiento 1/3: Hace que las notas agudas suenen un poco más brillantes de forma automática.", "Keyboard Track 1");
            return;
        }
        if (c == &kbd2Btn)
        {
            helpBar->setHelpText("Seguimiento 2/3: Abre el brillo con más fuerza en notas altas (con 1 y 2 activados sigue el tono exacto).", "Keyboard Track 2");
            return;
        }
        if (c == &filterModelBox)
        {
            helpBar->setHelpText("Tipo de filtro: Cambia el carácter analógico entre Model D original, respuesta limpia o la saturación agresiva Model 72.", "Filter Model");
            return;
        }
        if (c == &fAttackKnob.slider || c == &fAttackKnob.label)
        {
            helpBar->setHelpText("Ataque del filtro: Rapidez con la que se abre el brillo (en 0 da un golpe seco instantáneo; alto abre como un suspiro lento).", "Filter Attack");
            return;
        }
        if (c == &fDecayKnob.slider || c == &fDecayKnob.label)
        {
            helpBar->setHelpText("Decaimiento del filtro: Tiempo que tarda el brillo en apagarse después del golpe inicial mientras sigues pulsando la tecla.", "Filter Decay");
            return;
        }
        if (c == &fSustainKnob.slider || c == &fSustainKnob.label)
        {
            helpBar->setHelpText("Brillo sostenido: Qué tan brillante permanece el sonido mientras mantienes la tecla oprimida.", "Filter Sustain");
            return;
        }
        if (c == &aAttackKnob.slider || c == &aAttackKnob.label)
        {
            helpBar->setHelpText("Ataque de volumen: En 0 suena inmediato como un piano o bajo percusivo; alto entra suavemente como cuerdas o colchón.", "Amplifier Attack");
            return;
        }
        if (c == &aDecayKnob.slider || c == &aDecayKnob.label)
        {
            helpBar->setHelpText("Caída de volumen: Qué tan rápido baja del volumen máximo inicial al nivel de volumen sostenido.", "Amplifier Decay");
            return;
        }
        if (c == &aSustainKnob.slider || c == &aSustainKnob.label)
        {
            helpBar->setHelpText("Volumen sostenido: En 100% no baja de volumen al mantener la tecla; en 0% se apaga como una campana o guitarra.", "Amplifier Sustain");
            return;
        }
        if (c == &decaySwitchBtn)
        {
            helpBar->setHelpText("Cola al soltar (Release): Si está activo, el sonido se apaga desvaneciéndose lentamente al soltar la tecla.", "Decay / Release Mode");
            return;
        }

        // Output Controls
        if (c == &masterVolKnob.slider || c == &masterVolKnob.label)
        {
            helpBar->setHelpText("Volumen general: Controla el volumen final de salida sin distorsionar digitalmente los altavoces.", "Master Volume");
            return;
        }
        if (c == &voicesBox)
        {
            helpBar->setHelpText("Número de voces: 'Mono' para tocar solos y bajos clásicos de una nota; '4' u '8' para tocar acordes con ambas manos.", "Polyphony Voices");
            return;
        }
        if (c == &bypassBtn)
        {
            helpBar->setHelpText("Apagado maestro (Bypass): Silencia instantáneamente el sintetizador sin clicks ni ruidos molestos.", "Master Bypass");
            return;
        }

        // Arpeggiator
        if (c == &arpOnBtn)
        {
            helpBar->setHelpText("Arpegiador: Convierte los acordes que toques en secuencias de notas rítmicas automáticas.", "Arp On/Off");
            return;
        }
        if (c == &arpLatchBtn)
        {
            helpBar->setHelpText("Bloqueo de notas (Latch): Deja sonando el ritmo automáticamente aunque quites las manos del teclado.", "Arp Latch");
            return;
        }
        if (c == &arpModeBox)
        {
            helpBar->setHelpText("Orden de notas: Toca las notas hacia arriba, hacia abajo, rebotando o en orden aleatorio.", "Arp Direction");
            return;
        }
        if (c == &arpOctaveBox)
        {
            helpBar->setHelpText("Rango de octavas: Hace que el ritmo suba y baje por 1, 2, 3 o 4 octavas como una cascada.", "Arp Octave Range");
            return;
        }
        if (c == &arpOctModeBox)
        {
            helpBar->setHelpText("Movimiento de octavas: Si sube, baja o alterna en zigzag al recorrer las notas.", "Arp Octave Movement");
            return;
        }
        if (c == &arpRateBox)
        {
            helpBar->setHelpText("Velocidad del ritmo: Sincronizado a tu pista (1/4 lento, 1/8 estándar, 1/16 rápido o tresillos).", "Arp Clock Rate");
            return;
        }
        if (c == &arpPatternBox)
        {
            helpBar->setHelpText("Patrón rítmico: Notas rectas o patrones sincopados y grooves con swing para música bailable.", "Arp Rhythm Pattern");
            return;
        }
        if (c == &arpGateKnob.slider || c == &arpGateKnob.label)
        {
            helpBar->setHelpText("Duración de cada nota: Notas muy cortas y picadas (staccato) o largas y conectadas (legato).", "Arp Gate Length");
            return;
        }
        if (c == &arpSwingKnob.slider || c == &arpSwingKnob.label)
        {
            helpBar->setHelpText("Swing (Groove): Retrasa ligeramente una de cada dos notas para darle sabor bailable funk o jazz.", "Arp Swing Groove");
            return;
        }
        if (c == &arpMutateKnob.slider || c == &arpMutateKnob.label)
        {
            helpBar->setHelpText("Mutación/Caos: Introduce variaciones inesperadas en las notas para que la melodía nunca sea aburrida.", "Arp Mutate / Chaos");
            return;
        }

        // Performance
        if (c == &pitchWheel || c == &pitchWheelLabel)
        {
            helpBar->setHelpText("Pitch Bend Wheel: Temporarily bends master pitch up or down; springs back to center on release.", "Pitch Wheel");
            return;
        }
        if (c == &modWheel || c == &modWheelLabel)
        {
            bool oscMod = oscModBtn.getToggleState();
            bool filtMod = filterModBtn.getToggleState();
            if (!oscMod && !filtMod)
                helpBar->setHelpText("Modulation Wheel (inactive): Turn ON 'OSC MOD' or 'FILT MOD' in CONTROLLERS to hear vibrato or filter sweep.", "Modulation Wheel");
            else
                helpBar->setHelpText("Modulation Wheel: Scales modulation depth from LFO/Noise bus to pitch (vibrato) or filter cutoff.", "Modulation Wheel");
            return;
        }
        if (c == &keyboard)
        {
            helpBar->setHelpText("Interactive virtual keyboard; supports mouse clicks and computer keyboard typing (A-L keys).", "Virtual Keyboard");
            return;
        }

        // Header Tools
        if (c == &presetButton)
        {
            helpBar->setHelpText("Currently active factory or user preset; click to quick-jump or browse categories.", "Current Preset");
            return;
        }
        if (c == &prevPresetBtn || c == &nextPresetBtn)
        {
            helpBar->setHelpText("Step to previous or next preset in current bank.", "Preset Navigation");
            return;
        }
        if (c == &browseBtn)
        {
            helpBar->setHelpText("Opens the preset browser library with categorised patches, search filter, and instant auditioning.", "Browse Presets");
            return;
        }
        if (c == &saveBtn)
        {
            helpBar->setHelpText("Saves current sound parameters as a custom user preset to disk.", "Save Preset");
            return;
        }
        if (c == &initBtn)
        {
            helpBar->setHelpText("Resets all synth parameters to standard default initialization patch.", "Init Preset");
            return;
        }
        if (c == &abBtn)
        {
            helpBar->setHelpText("A/B state comparator; toggles between two independent sound configurations for direct comparison.", "A/B Compare");
            return;
        }
        if (c == &hqBtn)
        {
            helpBar->setHelpText("High-Quality oversampling mode (4x oversampled ladder filter DSP core for ultra-clean anti-aliased highs).", "HQ Oversampling");
            return;
        }
        if (c == &editThemeBtn)
        {
            helpBar->setHelpText("Opens Color Theme Studio to customize interface colors, rainbow palette, fonts, and corner rounding.", "Theme Studio");
            return;
        }
        if (c == &aboutBtn)
        {
            helpBar->setHelpText("Displays plugin version, technical specs, DSP architecture credits, and license notes.", "About & Credits");
            return;
        }

        c = c->getParentComponent();
    }

    helpBar->setHelpText("Hover over any parameter for instant functional guide & DSP signal flow notes.", "");
}

void LadderMonoAudioProcessorEditor::showAboutDialog()
{
    releaseAllHeldComputerKeys();
    if (presetBrowser != nullptr)
        presetBrowser->setVisible(false);
    if (aboutDialog != nullptr)
        aboutDialog->showDialog();
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

    // Undo (Cmd+Z or Ctrl+Z) / Redo (Cmd+Shift+Z or Ctrl+Y)
    auto mods = key.getModifiers();
    if (mods.isCommandDown() || mods.isCtrlDown())
    {
        if (key.getKeyCode() == 'Z' || key.getKeyCode() == 'z')
        {
            if (mods.isShiftDown())
                redo();
            else
                undo();
            return true;
        }
        else if (key.getKeyCode() == 'Y' || key.getKeyCode() == 'y')
        {
            redo();
            return true;
        }
    }

    // Octave shifting with Z and X (when no Command/Ctrl modifier is held)
    int code = key.getKeyCode();
    if (code == 'Z' || code == 'z')
    {
        if (baseOctaveNote > 12)
        {
            releaseAllHeldComputerKeys();
            baseOctaveNote -= 12;
            keyboard.setBaseOctaveNote(baseOctaveNote);
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

void LadderMonoAudioProcessorEditor::parentHierarchyChanged()
{
    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
    {
        window->setName("Neo Analog");
    }
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
    k.slider.setName(labelText);
    k.slider.setComponentID(paramId);
    k.slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe0e0e0));
    k.slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff121212));
    k.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2a2a2a));
    addAndMakeVisible(k.slider);

    k.label.setText(labelText, juce::dontSendNotification);
    k.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    k.label.setColour(juce::Label::textColourId, juce::Colour(0xff9c9c9c)); // High-contrast label
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
    else if (paramId == LadderMono::ParamIDs::analogAmount || paramId == LadderMono::ParamIDs::bassComp ||
             paramId == LadderMono::ParamIDs::arpGate || paramId == LadderMono::ParamIDs::spatialFilter)
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
    box.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff181818));
    box.setColour(juce::ComboBox::textColourId, juce::Colour(0xffe0e0e0));
    box.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2a2a2a));
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
// Paint: Unified Normalized Noir Architecture driven by ThemeColors
// ====================================================================
void LadderMonoAudioProcessorEditor::paint(juce::Graphics& g)
{
    float scale = static_cast<float>(getWidth()) / 1120.0f;
    g.addTransform(juce::AffineTransform::scale(scale));
    auto totalArea = juce::Rectangle<float>(0.0f, 0.0f, 1120.0f, 680.0f);

    // 1. Monolithic Dark Chassis Base
    g.fillAll(currentTheme.bgChassis);

    // 2. Chassis Top Bar (y: 0..42)
    juce::Rectangle<float> topBarRect(0.0f, 0.0f, totalArea.getWidth(), 42.0f);
    g.setColour(currentTheme.panelBg);
    g.fillRect(topBarRect);
    g.setColour(currentTheme.lineBorder);
    g.drawLine(0.0f, 42.0f, totalArea.getWidth(), 42.0f, currentTheme.borderWidth);

    // Side Chassis Bezels
    juce::Rectangle<float> leftBezel(0.0f, 42.0f, 20.0f, 398.0f);
    juce::Rectangle<float> rightBezel(totalArea.getWidth() - 20.0f, 42.0f, 20.0f, 398.0f);
    g.setColour(currentTheme.bgChassis);
    g.fillRect(leftBezel);
    g.fillRect(rightBezel);

    // Subtle technical measurement tick marks on bezels
    g.setColour(currentTheme.lineBorder);
    for (float ty = 46.0f; ty < 436.0f; ty += 18.0f)
    {
        g.drawLine(5.0f, ty, 15.0f, ty, currentTheme.borderWidth * 0.8f);
        g.drawLine(totalArea.getWidth() - 15.0f, ty, totalArea.getWidth() - 5.0f, ty, currentTheme.borderWidth * 0.8f);
    }

    // Mid Arpeggiator Strip Perimeter (Recessed tray)
    juce::Rectangle<float> arpBarRect(0.0f, 440.0f, totalArea.getWidth(), 42.0f);
    g.setColour(currentTheme.panelBg);
    g.fillRect(arpBarRect);
    g.setColour(currentTheme.lineBorder);
    g.drawLine(0.0f, 440.0f, totalArea.getWidth(), 440.0f, currentTheme.borderWidth);
    g.drawLine(0.0f, 482.0f, totalArea.getWidth(), 482.0f, currentTheme.borderWidth);
    g.setColour(currentTheme.lineBorder.withAlpha(0.5f));
    g.drawLine(268.0f, 444.0f, 268.0f, 478.0f, currentTheme.borderWidth * 0.8f);
    g.drawLine(782.0f, 444.0f, 782.0f, 478.0f, currentTheme.borderWidth * 0.8f);

    // Bottom border
    g.setColour(currentTheme.bgChassis);
    g.fillRect(juce::Rectangle<float>(0.0f, totalArea.getHeight() - 8.0f, totalArea.getWidth(), 8.0f));

    // 3. Central Dark Anodized Metal Faceplate
    juce::Rectangle<float> panelRect(20.0f, 42.0f, totalArea.getWidth() - 40.0f, 398.0f);
    g.setColour(currentTheme.panelBg);
    g.fillRect(panelRect);

    // 4. Normalized Section Frames (Milled into faceplate)
    auto drawSectionFrame = [this, &g](float x, float y, float w, float h, const juce::String& title) {
        juce::Rectangle<float> frame(x, y, w, h);
        float r = currentTheme.cornerRadius;

        // Recessed cavity well
        g.setColour(currentTheme.wellDark);
        g.fillRoundedRectangle(frame, r);

        // Normalized Bevel & Border line
        g.setColour(currentTheme.lineBorder);
        g.drawRoundedRectangle(frame, r, 1.2f * currentTheme.borderWidth);

        // Section Header Plate
        juce::Rectangle<float> headerPlate(x + 10.0f, y - 9.0f, w - 20.0f, 18.0f);
        g.setColour(currentTheme.panelBg);
        g.fillRoundedRectangle(headerPlate, juce::jmin(r, 3.0f));
        g.setColour(currentTheme.lineAccent);
        g.drawRoundedRectangle(headerPlate, juce::jmin(r, 3.0f), 1.0f * currentTheme.borderWidth);

        g.setFont(juce::FontOptions(11.0f * currentTheme.fontScale, juce::Font::bold));
        g.setColour(currentTheme.textPrimary);
        g.drawText(title, headerPlate.toNearestInt(), juce::Justification::centred);
    };

    drawSectionFrame(24.0f, 46.0f, 172.0f, 388.0f, "CONTROLLERS");
    drawSectionFrame(202.0f, 46.0f, 236.0f, 388.0f, "OSCILLATOR BANK");
    drawSectionFrame(444.0f, 46.0f, 176.0f, 388.0f, "MIXER");
    drawSectionFrame(626.0f, 46.0f, 256.0f, 388.0f, "MODIFIERS");
    drawSectionFrame(888.0f, 46.0f, 208.0f, 388.0f, "OUTPUT");

    // Oscillator Bank row labels (OSC 1, OSC 2, OSC 3)
    g.setFont(juce::FontOptions(10.5f * currentTheme.fontScale, juce::Font::bold));
    g.setColour(currentTheme.textSecondary);
    g.drawText("OSC 1", 208, 78, 40, 24, juce::Justification::centredLeft);
    g.drawText("OSC 2", 208, 138, 40, 24, juce::Justification::centredLeft);
    g.drawText("OSC 3", 208, 215, 40, 24, juce::Justification::centredLeft);

    // Modifiers Sub-Headers (FILTER CONTOUR, LOUDNESS CONTOUR)
    auto drawSubHeader = [this, &g](float x, float y, float w, const juce::String& text) {
        juce::Rectangle<float> bar(x, y, w, 15.0f);
        float r = juce::jmin(currentTheme.cornerRadius, 3.0f);
        g.setColour(currentTheme.panelLight);
        g.fillRoundedRectangle(bar, r);
        g.setColour(currentTheme.lineBorder);
        g.drawRoundedRectangle(bar, r, 0.8f * currentTheme.borderWidth);

        g.setFont(juce::FontOptions(9.5f * currentTheme.fontScale, juce::Font::bold));
        g.setColour(currentTheme.textSecondary);
        g.drawText(text, bar.toNearestInt(), juce::Justification::centred);
    };

    drawSubHeader(632, 204, 244, "FILTER CONTOUR");
    drawSubHeader(632, 290, 244, "LOUDNESS CONTOUR");

    // Drive Overload Indicator
    float ovlX = 604.0f;
    float ovlY = 395.0f;
    float driveVal = audioProcessor.getAPVTS().getRawParameterValue(LadderMono::ParamIDs::mixerDrive)->load();
    bool isOverload = driveVal > 0.05f;

    g.setFont(juce::FontOptions(9.0f * currentTheme.fontScale, juce::Font::bold));
    if (isOverload)
    {
        // Bright glowing red badge text
        g.setColour(juce::Colour(0xffff4d4d));
        g.drawText("OVERLOAD", static_cast<int>(ovlX - 35.0f), static_cast<int>(ovlY - 20.0f), 70, 14, juce::Justification::centred);

        // 1. Wide ambient glowing halo
        g.setColour(juce::Colour(0x55ff2200));
        g.fillEllipse(ovlX - 14.0f, ovlY - 14.0f, 28.0f, 28.0f);

        // 2. Secondary soft glow ring
        g.setColour(juce::Colour(0x99ff3b20));
        g.fillEllipse(ovlX - 9.0f, ovlY - 9.0f, 18.0f, 18.0f);

        // 3. Incandescent lamp bulb with brilliant hot core
        float ledR = 6.5f;
        juce::ColourGradient bulbGrad(juce::Colour(0xffffffff), ovlX - 2.0f, ovlY - 2.0f,
                                      juce::Colour(0xffff2200), ovlX + ledR, ovlY + ledR, true);
        bulbGrad.addColour(0.3, juce::Colour(0xffffddaa));
        bulbGrad.addColour(0.7, juce::Colour(0xffff3b20));
        g.setGradientFill(bulbGrad);
        g.fillEllipse(ovlX - ledR, ovlY - ledR, ledR * 2.0f, ledR * 2.0f);

        // Chrome/metal bezel rim
        g.setColour(juce::Colour(0xffffe0d0));
        g.drawEllipse(ovlX - ledR, ovlY - ledR, ledR * 2.0f, ledR * 2.0f, 1.2f);
    }
    else
    {
        // Off state: dark recessed socket
        g.setColour(currentTheme.textDim);
        g.drawText("OVERLOAD", static_cast<int>(ovlX - 35.0f), static_cast<int>(ovlY - 20.0f), 70, 14, juce::Justification::centred);

        float ledR = 6.0f;
        g.setColour(currentTheme.wellDark);
        g.fillEllipse(ovlX - ledR, ovlY - ledR, ledR * 2.0f, ledR * 2.0f);
        g.setColour(currentTheme.lineBorder);
        g.drawEllipse(ovlX - ledR, ovlY - ledR, ledR * 2.0f, ledR * 2.0f, 0.8f * currentTheme.borderWidth);
    }

    // ================================================================
    // Section 5: Top Horizontal Screen (Pure Black OLED as requested)
    // ================================================================
    juce::Rectangle<float> screenBezel(896.0f, 56.0f, 192.0f, 136.0f);
    g.setColour(currentTheme.wellDark);
    g.fillRoundedRectangle(screenBezel, currentTheme.cornerRadius);
    g.setColour(currentTheme.lineBorder);
    g.drawRoundedRectangle(screenBezel, currentTheme.cornerRadius, 1.0f * currentTheme.borderWidth);

    // Inner Pitch Black Glass - "por ahora para identificarla pon la negra"
    juce::Rectangle<float> screenGlass(900.0f, 60.0f, 184.0f, 128.0f);
    g.setColour(juce::Colour(0xff000000));
    g.fillRoundedRectangle(screenGlass, juce::jmax(1.0f, currentTheme.cornerRadius - 1.0f));
    g.setColour(juce::Colour(0x22ffffff));
    g.drawRoundedRectangle(screenGlass, juce::jmax(1.0f, currentTheme.cornerRadius - 1.0f), 0.8f * currentTheme.borderWidth);

    // Subtle glass horizon reflection line
    g.setColour(juce::Colour(0x0cffffff));
    g.fillRoundedRectangle(screenGlass.removeFromTop(screenGlass.getHeight() * 0.45f), juce::jmax(1.0f, currentTheme.cornerRadius - 1.0f));

    // OLED Standby Text
    g.setFont(juce::FontOptions(9.0f * currentTheme.fontScale, juce::Font::bold));
    g.setColour(currentTheme.screenText);
    g.drawText(juce::CharPointer_UTF8("OLED DISPLAY \xe2\x80\xa2 STANDBY"), juce::Rectangle<float>(900.0f, 60.0f, 184.0f, 128.0f).toNearestInt(), juce::Justification::centred);

    // Master Output sub-header bar
    juce::Rectangle<float> outBar(896.0f, 204.0f, 192.0f, 16.0f);
    g.setColour(currentTheme.panelLight);
    g.fillRoundedRectangle(outBar, currentTheme.cornerRadius * 0.5f);
    g.setColour(currentTheme.lineBorder);
    g.drawRoundedRectangle(outBar, currentTheme.cornerRadius * 0.5f, 0.8f * currentTheme.borderWidth);
    g.setFont(juce::FontOptions(9.5f * currentTheme.fontScale, juce::Font::bold));
    g.setColour(currentTheme.textSecondary);
    g.drawText("OUTPUT CONTROLS", outBar.toNearestInt(), juce::Justification::centred);

    // Labels in Output Controls section
    g.setFont(juce::FontOptions(9.5f * currentTheme.fontScale, juce::Font::bold));
    g.setColour(currentTheme.textSecondary);
    g.drawText("VOICES", 912, 320, 160, 14, juce::Justification::centred);
    g.drawText("BYPASS", 912, 372, 160, 14, juce::Justification::centred);

}

void LadderMonoAudioProcessorEditor::resized()
{
    float scale = static_cast<float>(getWidth()) / 1120.0f;
    auto S = [scale](int v) -> int { return juce::roundToInt(static_cast<float>(v) * scale); };
    auto Srect = [scale](int x, int y, int w, int h) -> juce::Rectangle<int> {
        return { juce::roundToInt(static_cast<float>(x) * scale),
                 juce::roundToInt(static_cast<float>(y) * scale),
                 juce::roundToInt(static_cast<float>(w) * scale),
                 juce::roundToInt(static_cast<float>(h) * scale) };
    };

    // ====================================================================
    // Top Bar (y: 0..42)
    // ====================================================================
    titleLabel.setBounds(Srect(62, 8, 135, 26));
    versionBtn.setBounds(Srect(200, 9, 44, 24));

    presetButton.setBounds(Srect(255, 8, 175, 26));
    prevPresetBtn.setBounds(Srect(433, 8, 28, 26));
    nextPresetBtn.setBounds(Srect(463, 8, 28, 26));
    browseBtn.setBounds(Srect(495, 8, 58, 26));
    saveBtn.setBounds(Srect(556, 8, 48, 26));
    initBtn.setBounds(Srect(607, 8, 42, 26));

    abBtn.setBounds(Srect(658, 8, 54, 26));
    hqBtn.setBounds(Srect(716, 8, 40, 26));
    cpuLabel.setBounds(getWidth() - S(222), S(8), S(68), S(26));
    editThemeBtn.setBounds(getWidth() - S(146), S(8), S(64), S(26));
    aboutBtn.setBounds(getWidth() - S(76), S(8), S(62), S(26));

    // ====================================================================
    // Section 1: CONTROLLERS (x: 24..196, w: 172)
    // ====================================================================
    // Row 1: Tune & Glide Time
    tuneKnob.slider.setBounds(Srect(34, 72, 52, 52));
    tuneKnob.label.setBounds(Srect(34, 124, 52, 14));

    glideKnob.slider.setBounds(Srect(118, 72, 52, 52));
    glideKnob.label.setBounds(Srect(118, 124, 52, 14));

    // Row 2: Glide Switch & Glide Mode
    glideOnBtn.setBounds(Srect(30, 146, 76, 28));
    glideModeBox.setBounds(Srect(114, 148, 72, 24));

    // Row 3: Mod Mix & LFO Rate
    modMixKnob.slider.setBounds(Srect(34, 182, 52, 52));
    modMixKnob.label.setBounds(Srect(34, 234, 52, 14));

    lfoRateKnob.slider.setBounds(Srect(118, 182, 52, 52));
    lfoRateKnob.label.setBounds(Srect(118, 234, 52, 14));

    // Row 4: Mod Source & LFO Shape
    modSourceBox.setBounds(Srect(30, 258, 76, 24));
    lfoShapeBox.setBounds(Srect(114, 258, 72, 24));

    // Row 5: Osc Mod & Filter Mod switches
    oscModBtn.setBounds(Srect(30, 294, 76, 30));
    filterModBtn.setBounds(Srect(114, 294, 76, 30));

    // Row 6: Bend Range & Note Priority
    bendRangeBox.setBounds(Srect(30, 338, 76, 24));
    notePriorityBox.setBounds(Srect(114, 338, 72, 24));

    // ====================================================================
    // Section 2: OSCILLATOR BANK (x: 202..438, w: 236)
    // ====================================================================
    // Osc 1
    osc1RangeBox.setBounds(Srect(248, 78, 86, 24));
    osc1WaveBox.setBounds(Srect(342, 78, 88, 24));

    // Osc 2 (with its own FINE knob on the same row)
    osc2RangeBox.setBounds(Srect(248, 138, 68, 24));
    osc2WaveBox.setBounds(Srect(322, 138, 62, 24));
    osc2FineKnob.slider.setBounds(Srect(390, 122, 44, 44));
    osc2FineKnob.label.setBounds(Srect(390, 168, 44, 12));

    // Osc 3 (with its own FINE knob and OSC 3 KBD on the same section)
    osc3RangeBox.setBounds(Srect(248, 215, 68, 24));
    osc3WaveBox.setBounds(Srect(322, 215, 62, 24));
    osc3FineKnob.slider.setBounds(Srect(390, 199, 44, 44));
    osc3FineKnob.label.setBounds(Srect(390, 245, 44, 12));

    osc3KbdBtn.setBounds(Srect(248, 252, 94, 28));

    // DRIFT Knob (Analog Component Tolerance & Thermal Drift)
    analogKnob.slider.setBounds(Srect(350, 266, 58, 58));
    analogKnob.label.setBounds(Srect(350, 326, 58, 14));

    // ====================================================================
    // Section 3: MIXER (x: 444..620, w: 176)
    // ====================================================================
    mix1Knob.slider.setBounds(Srect(452, 70, 46, 46));
    mix1Knob.label.setBounds(Srect(452, 118, 46, 13));
    mix1Btn.setBounds(Srect(504, 78, 70, 30));

    mix2Knob.slider.setBounds(Srect(452, 134, 46, 46));
    mix2Knob.label.setBounds(Srect(452, 182, 46, 13));
    mix2Btn.setBounds(Srect(504, 142, 70, 30));

    mix3Knob.slider.setBounds(Srect(452, 198, 46, 46));
    mix3Knob.label.setBounds(Srect(452, 246, 46, 13));
    mix3Btn.setBounds(Srect(504, 206, 70, 30));

    mixNoiseKnob.slider.setBounds(Srect(452, 262, 46, 46));
    mixNoiseKnob.label.setBounds(Srect(452, 310, 46, 13));
    mixNoiseBtn.setBounds(Srect(504, 264, 52, 26));
    noiseColorBox.setBounds(Srect(560, 264, 52, 24));

    // Clean Feed & Drive
    mixExtKnob.slider.setBounds(Srect(452, 326, 46, 46));
    mixExtKnob.label.setBounds(Srect(452, 374, 46, 13));
    mixExtBtn.setBounds(Srect(504, 328, 70, 28));

    driveKnob.slider.setBounds(Srect(546, 368, 46, 46));
    driveKnob.label.setBounds(Srect(546, 416, 46, 12));

    // ====================================================================
    // Section 4: MODIFIERS (x: 626..882, w: 256)
    // ====================================================================
    // Filter Core
    cutoffKnob.slider.setBounds(Srect(636, 68, 54, 54));
    cutoffKnob.label.setBounds(Srect(636, 124, 54, 14));

    emphasisKnob.slider.setBounds(Srect(700, 68, 54, 54));
    emphasisKnob.label.setBounds(Srect(700, 124, 54, 14));

    contourKnob.slider.setBounds(Srect(764, 68, 54, 54));
    contourKnob.label.setBounds(Srect(756, 124, 70, 14));

    kbd1Btn.setBounds(Srect(632, 146, 56, 28));
    kbd2Btn.setBounds(Srect(690, 146, 56, 28));
    filterModelBox.setBounds(Srect(632, 180, 114, 24));

    bassCompKnob.slider.setBounds(Srect(750, 138, 46, 46));
    bassCompKnob.label.setBounds(Srect(746, 186, 54, 12));

    spatialKnob.slider.setBounds(Srect(804, 138, 46, 46));
    spatialKnob.label.setBounds(Srect(800, 186, 54, 12));

    // Filter Env (Row 3)
    fAttackKnob.slider.setBounds(Srect(642, 224, 48, 48));
    fAttackKnob.label.setBounds(Srect(642, 272, 48, 13));
    fDecayKnob.slider.setBounds(Srect(704, 224, 48, 48));
    fDecayKnob.label.setBounds(Srect(704, 272, 48, 13));
    fSustainKnob.slider.setBounds(Srect(766, 224, 48, 48));
    fSustainKnob.label.setBounds(Srect(766, 272, 48, 13));

    // Amp Env (Row 4)
    aAttackKnob.slider.setBounds(Srect(642, 308, 48, 48));
    aAttackKnob.label.setBounds(Srect(642, 356, 48, 13));
    aDecayKnob.slider.setBounds(Srect(704, 308, 48, 48));
    aDecayKnob.label.setBounds(Srect(704, 356, 48, 13));
    aSustainKnob.slider.setBounds(Srect(766, 308, 48, 48));
    aSustainKnob.label.setBounds(Srect(766, 356, 48, 13));

    decaySwitchBtn.setBounds(Srect(700, 388, 106, 32));

    // ====================================================================
    // Section 5: OUTPUT (x: 888..1096, w: 208)
    // Vertically distributed layout utilizing full available space:
    // Volume (centered prominent knob) -> Voices selector -> Bypass button
    // ====================================================================
    masterVolKnob.slider.setBounds(Srect(959, 226, 66, 66));
    masterVolKnob.label.setBounds(Srect(950, 294, 84, 14));

    voicesBox.setBounds(Srect(912, 336, 160, 26));
    bypassBtn.setBounds(Srect(912, 388, 160, 32));

    // ====================================================================
    // Middle Crossbar: ARPEGGIATOR & PERFORMANCE BAR (y: 440..482, x: 28..1092)
    // Distributed evenly across the full available panel span
    // ====================================================================
    arpSectionLabel.setBounds(Srect(28, 444, 96, 34));
    arpOnBtn.setBounds(Srect(128, 447, 64, 28));
    arpLatchBtn.setBounds(Srect(196, 447, 64, 28));

    arpModeLabel.setBounds(Srect(276, 442, 94, 13));
    arpModeBox.setBounds(Srect(276, 456, 94, 22));

    arpOctaveLabel.setBounds(Srect(380, 442, 84, 13));
    arpOctaveBox.setBounds(Srect(380, 456, 84, 22));

    arpOctModeLabel.setBounds(Srect(474, 442, 90, 13));
    arpOctModeBox.setBounds(Srect(474, 456, 90, 22));

    arpRateLabel.setBounds(Srect(574, 442, 86, 13));
    arpRateBox.setBounds(Srect(574, 456, 86, 22));

    arpPatternLabel.setBounds(Srect(670, 442, 104, 13));
    arpPatternBox.setBounds(Srect(670, 456, 104, 22));

    arpGateKnob.label.setBounds(Srect(790, 442, 94, 13));
    arpGateKnob.slider.setBounds(Srect(790, 454, 94, 24));

    arpSwingKnob.label.setBounds(Srect(894, 442, 94, 13));
    arpSwingKnob.slider.setBounds(Srect(894, 454, 94, 24));

    arpMutateKnob.label.setBounds(Srect(998, 442, 94, 13));
    arpMutateKnob.slider.setBounds(Srect(998, 454, 94, 24));

    // ====================================================================
    // Lower Performance Section (y: 482..672)
    // ====================================================================
    pitchWheel.setBounds(Srect(28, 484, 44, 156));
    pitchWheelLabel.setBounds(Srect(28, 642, 44, 14));
    modWheel.setBounds(Srect(78, 484, 44, 156));
    modWheelLabel.setBounds(Srect(78, 642, 44, 14));
    keyboard.setBounds(S(136), S(482), getWidth() - S(156), getHeight() - S(506));

    if (helpBar != nullptr)
    {
        int barW = juce::jmin(S(800), getWidth() - S(60));
        helpBar->setBounds(S(28), getHeight() - S(23), barW, S(20));
    }

    // Resizer handle in bottom-right corner (always on top)
    if (resizer != nullptr)
    {
        resizer->setBounds(getWidth() - 22, getHeight() - 22, 22, 22);
        resizer->toFront(true);
    }

    if (presetBrowser != nullptr)
        presetBrowser->setBounds(getLocalBounds());
    if (aboutDialog != nullptr)
        aboutDialog->setBounds(getLocalBounds());
    if (themeEditorPanel != nullptr)
    {
        if (!themeEditorPanel->hasBeenMoved)
        {
            themeEditorPanel->setBounds(S(130), S(46), S(860), S(244));
        }
        else
        {
            themeEditorPanel->setSize(S(860), S(244));
            int maxX = juce::jmax(0, getWidth() - themeEditorPanel->getWidth());
            int maxY = juce::jmax(0, getHeight() - themeEditorPanel->getHeight());
            themeEditorPanel->setTopLeftPosition(juce::jlimit(0, maxX, themeEditorPanel->getX()),
                                                 juce::jlimit(0, maxY, themeEditorPanel->getY()));
        }
    }
}
