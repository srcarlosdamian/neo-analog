#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

LadderMonoAudioProcessorEditor::LadderMonoAudioProcessorEditor(LadderMonoAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lnf);

    // Title & Preset header
    titleLabel.setText("N E O   A N A L O G", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe08b3c));
    addAndMakeVisible(titleLabel);

    presetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff18191d));
    presetButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff25272e));
    presetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe8eaee));
    presetButton.onClick = [this]() {
        juce::PopupMenu menu;

        menu.addItem(9999, "★ Open Preset Browser...", true, false);
        menu.addSeparator();

        // Group presets by category with Basics first
        std::map<juce::String, std::vector<std::pair<int, juce::String>>> categorized;
        const auto& presets = audioProcessor.getPresetManager().getPresets();
        for (size_t i = 0; i < presets.size(); ++i)
        {
            categorized[presets[i].category].emplace_back(static_cast<int>(i + 1), presets[i].name);
        }

        // 1. Basics first
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

        // 2. Thematic packs
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

        // 3. Init Patch
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

    browseBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22252c));
    browseBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff2f333c));
    browseBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe08b3c));
    browseBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
        if (presetBrowser != nullptr)
            presetBrowser->showBrowser();
    };
    addAndMakeVisible(browseBtn);

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

    initBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff18191d));
    initBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff9ea3b0));
    initBtn.onClick = [this]() {
        releaseAllHeldComputerKeys();
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
    oscModBtn.setButtonText("OSC MOD");
    setupToggle(filterModBtn, LadderMono::ParamIDs::filterModOn);
    filterModBtn.setButtonText("FILT MOD");

    // Arpeggiator
    setupToggle(arpOnBtn, LadderMono::ParamIDs::arpOn);
    arpOnBtn.setButtonText("ARP ON");
    setupToggle(arpLatchBtn, LadderMono::ParamIDs::arpLatch);
    arpLatchBtn.setButtonText("LATCH");
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
    osc3KbdBtn.setButtonText("OSC 3 KBD");

    // Mixer
    setupKnob(mix1Knob, LadderMono::ParamIDs::mixOsc1, "OSC 1");
    setupToggle(mix1Btn, LadderMono::ParamIDs::mixOsc1On);
    mix1Btn.setButtonText("1");
    setupKnob(mix2Knob, LadderMono::ParamIDs::mixOsc2, "OSC 2");
    setupToggle(mix2Btn, LadderMono::ParamIDs::mixOsc2On);
    mix2Btn.setButtonText("2");
    setupKnob(mix3Knob, LadderMono::ParamIDs::mixOsc3, "OSC 3");
    setupToggle(mix3Btn, LadderMono::ParamIDs::mixOsc3On);
    mix3Btn.setButtonText("3");
    setupKnob(mixNoiseKnob, LadderMono::ParamIDs::mixNoise, "NOISE");
    setupToggle(mixNoiseBtn, LadderMono::ParamIDs::mixNoiseOn);
    mixNoiseBtn.setButtonText("NOISE");
    setupKnob(mixExtKnob, LadderMono::ParamIDs::mixExt, "EXT IN");
    setupToggle(mixExtBtn, LadderMono::ParamIDs::mixExtOn);
    mixExtBtn.setButtonText("FEED");
    setupKnob(driveKnob, LadderMono::ParamIDs::mixerDrive, "DRIVE", true);
    setupComboBox(noiseColorBox, LadderMono::ParamIDs::noiseColor, {"White", "Pink"});

    // Filter
    setupKnob(cutoffKnob, LadderMono::ParamIDs::cutoff, "CUTOFF", true);
    setupKnob(emphasisKnob, LadderMono::ParamIDs::emphasis, "EMPHASIS");
    setupKnob(contourKnob, LadderMono::ParamIDs::contourAmount, "AMOUNT");
    setupKnob(bassCompKnob, LadderMono::ParamIDs::bassComp, "BASS COMP");
    setupToggle(kbd1Btn, LadderMono::ParamIDs::kbd1);
    kbd1Btn.setButtonText("KBD 1");
    setupToggle(kbd2Btn, LadderMono::ParamIDs::kbd2);
    kbd2Btn.setButtonText("KBD 2");

    // Envelopes
    setupKnob(fAttackKnob, LadderMono::ParamIDs::fAttack, "ATTACK");
    setupKnob(fDecayKnob, LadderMono::ParamIDs::fDecay, "DECAY");
    setupKnob(fSustainKnob, LadderMono::ParamIDs::fSustain, "SUSTAIN");

    setupKnob(aAttackKnob, LadderMono::ParamIDs::aAttack, "ATTACK");
    setupKnob(aDecayKnob, LadderMono::ParamIDs::aDecay, "DECAY");
    setupKnob(aSustainKnob, LadderMono::ParamIDs::aSustain, "SUSTAIN");
    setupToggle(decaySwitchBtn, LadderMono::ParamIDs::decaySwitchOn);
    decaySwitchBtn.setButtonText("DECAY REL");

    // Output & Master
    setupKnob(masterVolKnob, LadderMono::ParamIDs::masterVol, "VOLUME");
    setupKnob(analogKnob, LadderMono::ParamIDs::analogAmount, "DRIFT");
    setupComboBox(voicesBox, LadderMono::ParamIDs::voices, {"Mono", "4 Voices", "8 Voices"});

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
    keyboardHintLabel.setColour(juce::Label::textColourId, juce::Colour(0xffb5b9c4));
    keyboardHintLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(keyboardHintLabel);
    updateKeyboardHint();

    presetBrowser = std::make_unique<LadderMono::PresetBrowserOverlay>(
        audioProcessor.getPresetManager(),
        [this] { updatePresetDisplay(); }
    );
    addChildComponent(*presetBrowser);

    updatePresetDisplay();

    setWantsKeyboardFocus(true);
    setSize(1120, 680);
}

LadderMonoAudioProcessorEditor::~LadderMonoAudioProcessorEditor()
{
    sliderAttachments.clear();
    comboAttachments.clear();
    buttonAttachments.clear();
    setLookAndFeel(nullptr);
}

void LadderMonoAudioProcessorEditor::updateKeyboardHint()
{
    int octNum = (baseOctaveNote / 12) - 1;
    keyboardHintLabel.setText("KEYBOARD PLAYING  [ A W S E D F T G Y H U J K ]  |  OCTAVE: C" + juce::String(octNum) + " (Z/X to shift)",
                             juce::dontSendNotification);
}

bool LadderMonoAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    if (presetBrowser != nullptr && presetBrowser->isVisible())
    {
        if (presetBrowser->isSearchFocused())
        {
            if (key == juce::KeyPress::escapeKey)
            {
                presetBrowser->clearSearchFocus();
                return true;
            }
            if (key == juce::KeyPress::returnKey)
            {
                presetBrowser->clearSearchFocus();
                return true;
            }
            return presetBrowser->keyPressed(key);
        }

        // Preset browser is open, but search bar is NOT focused:
        if (key == juce::KeyPress::escapeKey)
        {
            presetBrowser->setVisible(false);
            return true;
        }

        if (key == juce::KeyPress::upKey)
        {
            presetBrowser->selectPrevPreset();
            return true;
        }

        if (key == juce::KeyPress::downKey)
        {
            presetBrowser->selectNextPreset();
            return true;
        }
    }

    auto text = key.getTextDescription();
    if (text.equalsIgnoreCase("Z"))
    {
        if (baseOctaveNote > 12)
        {
            baseOctaveNote -= 12;
            keyboard.setBaseOctaveNote(baseOctaveNote);
            updateKeyboardHint();
        }
        return true;
    }
    else if (text.equalsIgnoreCase("X"))
    {
        if (baseOctaveNote < 96)
        {
            baseOctaveNote += 12;
            keyboard.setBaseOctaveNote(baseOctaveNote);
            updateKeyboardHint();
        }
        return true;
    }

    // Always consume key events in the editor to prevent macOS from playing
    // the system alert beep (NSBeep / error / camera sound) when playing musical keys or holding them down.
    return true;
}

bool LadderMonoAudioProcessorEditor::keyStateChanged(bool isKeyDown)
{
    if (presetBrowser != nullptr && presetBrowser->isVisible())
    {
        if (presetBrowser->isSearchFocused())
        {
            releaseAllHeldComputerKeys();
            return false;
        }
    }

    static const std::vector<std::pair<juce::KeyPress, int>> keyMap = {
        { juce::KeyPress('a'), 0 },
        { juce::KeyPress('w'), 1 },
        { juce::KeyPress('s'), 2 },
        { juce::KeyPress('e'), 3 },
        { juce::KeyPress('d'), 4 },
        { juce::KeyPress('f'), 5 },
        { juce::KeyPress('t'), 6 },
        { juce::KeyPress('g'), 7 },
        { juce::KeyPress('y'), 8 },
        { juce::KeyPress('h'), 9 },
        { juce::KeyPress('u'), 10 },
        { juce::KeyPress('j'), 11 },
        { juce::KeyPress('k'), 12 },
        { juce::KeyPress('o'), 13 },
        { juce::KeyPress('l'), 14 },
        { juce::KeyPress('p'), 15 }
    };

    for (const auto& [kp, semitoneOffset] : keyMap)
    {
        int charCode = kp.getKeyCode();
        bool isPressed = juce::KeyPress::isKeyCurrentlyDown(charCode);

        auto it = charCodeToPlayingNote.find(charCode);
        if (isPressed && it == charCodeToPlayingNote.end())
        {
            int noteNumber = baseOctaveNote + semitoneOffset;
            charCodeToPlayingNote[charCode] = noteNumber;
            audioProcessor.handleNoteOn(noteNumber, 0.85f);
            keyboard.setNoteActive(noteNumber, true);
        }
        else if (!isPressed && it != charCodeToPlayingNote.end())
        {
            int playedNote = it->second;
            charCodeToPlayingNote.erase(it);
            audioProcessor.handleNoteOff(playedNote);
            keyboard.setNoteActive(playedNote, false);
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

void LadderMonoAudioProcessorEditor::setupKnob(Knobby& k, const juce::String& paramId, const juce::String& labelText, bool isBipolar)
{
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 15);
    k.slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffd0d4dc));
    k.slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff18191d));
    k.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff34363d));
    addAndMakeVisible(k.slider);

    k.label.setText(labelText, juce::dontSendNotification);
    k.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    k.label.setColour(juce::Label::textColourId, juce::Colour(0xffb5b9c4));
    k.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(k.label);

    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), paramId, k.slider));
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
    // Deep warm walnut gradient
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

    // Fine organic wood grain lines
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

    // Top/edge highlight and bottom drop shadow
    g.setColour(juce::Colour(0x35ffffff));
    g.drawHorizontalLine(static_cast<int>(rect.getY()), rect.getX(), rect.getRight());
    g.setColour(juce::Colour(0x66000000));
    g.drawHorizontalLine(static_cast<int>(rect.getBottom() - 1.0f), rect.getX(), rect.getRight());
}

static void drawScrewHead(juce::Graphics& g, float cx, float cy, float radius = 4.2f)
{
    // Outer recessed shadow
    g.setColour(juce::Colour(0x88000000));
    g.fillEllipse(cx - radius - 1.0f, cy - radius - 1.0f, (radius + 1.0f) * 2.0f, (radius + 1.0f) * 2.0f);

    // Steel screw head gradient
    juce::ColourGradient screwGrad(juce::Colour(0xffdcdfe4), cx - radius * 0.4f, cy - radius * 0.4f,
                                   juce::Colour(0xff585c66), cx + radius * 0.6f, cy + radius * 0.6f, false);
    g.setGradientFill(screwGrad);
    g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    g.setColour(juce::Colour(0xff3c3f46));
    g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 0.8f);

    // 45-degree screw slot
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
    // Top Wood Rail
    drawWoodGrain(g, juce::Rectangle<float>(0.0f, 0.0f, totalArea.getWidth(), 42.0f), false);

    // Left and Right Wood Cheeks
    drawWoodGrain(g, juce::Rectangle<float>(0.0f, 0.0f, 20.0f, totalArea.getHeight()), true);
    drawWoodGrain(g, juce::Rectangle<float>(totalArea.getWidth() - 20.0f, 0.0f, 20.0f, totalArea.getHeight()), true);

    // Middle Separation Wood Crossbar
    drawWoodGrain(g, juce::Rectangle<float>(0.0f, 440.0f, totalArea.getWidth(), 42.0f), false);

    // Bottom Wood Rim
    drawWoodGrain(g, juce::Rectangle<float>(0.0f, totalArea.getHeight() - 8.0f, totalArea.getWidth(), 8.0f), false);

    // Screws on Top Wood Rail
    drawScrewHead(g, 40.0f, 21.0f);
    drawScrewHead(g, totalArea.getWidth() * 0.5f - 40.0f, 21.0f);
    drawScrewHead(g, totalArea.getWidth() - 40.0f, 21.0f);

    // Screws on Middle Crossbar
    drawScrewHead(g, 35.0f, 461.0f);
    drawScrewHead(g, 125.0f, 461.0f);

    // 3. Anodized Dark Aluminum Faceplate
    juce::Rectangle<float> panelRect(20.0f, 42.0f, totalArea.getWidth() - 40.0f, 398.0f);
    juce::ColourGradient panelGrad(juce::Colour(0xff222429), 0.0f, panelRect.getY(),
                                   juce::Colour(0xff161719), 0.0f, panelRect.getBottom(), false);
    panelGrad.addColour(0.5, juce::Colour(0xff1d1e22));
    g.setGradientFill(panelGrad);
    g.fillRect(panelRect);

    // Subtle brushed aluminum texture
    g.setColour(juce::Colour(0x0affffff));
    for (float y = panelRect.getY(); y < panelRect.getBottom(); y += 3.0f)
        g.drawHorizontalLine(static_cast<int>(y), panelRect.getX(), panelRect.getRight());

    // 4. White / Silver Borderline Frames around Sections
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
        g.setColour(juce::Colour(0xffdcdfe5));
        g.drawText(title, juce::Rectangle<float>(x, y + 2.0f, w, 22.0f).toNearestInt(), juce::Justification::centred);
    };

    // 6 Distinct Panel Sections
    drawSectionFrame(24.0f, 46.0f, 140.0f, 388.0f, "CONTROLLERS");
    drawSectionFrame(168.0f, 46.0f, 140.0f, 388.0f, "ARPEGGIATOR");
    drawSectionFrame(312.0f, 46.0f, 220.0f, 388.0f, "OSCILLATOR BANK");
    drawSectionFrame(536.0f, 46.0f, 190.0f, 388.0f, "MIXER");
    drawSectionFrame(730.0f, 46.0f, 250.0f, 388.0f, "MODIFIERS");
    drawSectionFrame(984.0f, 46.0f, 112.0f, 388.0f, "OUTPUT");

    // Sub-headers in Modifiers
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff8c92a0));
    g.drawText("FILTER CONTOUR", 735, 204, 240, 14, juce::Justification::centred);
    g.drawText("LOUDNESS CONTOUR", 735, 290, 240, 14, juce::Justification::centred);
    g.drawText("VOICES", 985, 256, 110, 14, juce::Justification::centred);

    // 5. Overload Indicator Lamp (Mixer)
    float ovlX = 688.0f;
    float ovlY = 385.0f;
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff9ea3b0));
    g.drawText("OVERLOAD", static_cast<int>(ovlX - 25.0f), static_cast<int>(ovlY - 18.0f), 50, 14, juce::Justification::centred);

    float driveVal = audioProcessor.getAPVTS().getRawParameterValue(LadderMono::ParamIDs::mixerDrive)->load();
    bool isOverload = driveVal > 0.05f;

    // Outer lamp bezel
    g.setColour(juce::Colour(0xff2d3038));
    g.fillEllipse(ovlX - 7.0f, ovlY - 7.0f, 14.0f, 14.0f);
    g.setColour(juce::Colour(0xff555a66));
    g.drawEllipse(ovlX - 7.0f, ovlY - 7.0f, 14.0f, 14.0f, 1.0f);

    // Crimson glowing lamp
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

    // 6. Red Jewel Pilot Lamp (Power) in Output
    float pwrX = 1040.0f;
    float pwrY = 345.0f;
    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff9ea3b0));
    g.drawText("POWER", static_cast<int>(pwrX - 25.0f), static_cast<int>(pwrY + 12.0f), 50, 14, juce::Justification::centred);

    // Bezel
    g.setColour(juce::Colour(0xff32353d));
    g.fillEllipse(pwrX - 10.0f, pwrY - 10.0f, 20.0f, 20.0f);
    g.setColour(juce::Colour(0xff6e7380));
    g.drawEllipse(pwrX - 10.0f, pwrY - 10.0f, 20.0f, 20.0f, 1.2f);

    // Ruby jewel ambient glow
    g.setColour(juce::Colour(0x55ff1a00));
    g.fillEllipse(pwrX - 15.0f, pwrY - 15.0f, 30.0f, 30.0f);

    // Faceted jewel body
    juce::ColourGradient jewelGrad(juce::Colour(0xffff3b20), pwrX - 4.0f, pwrY - 4.0f,
                                   juce::Colour(0xff8a0000), pwrX + 6.0f, pwrY + 6.0f, false);
    g.setGradientFill(jewelGrad);
    g.fillEllipse(pwrX - 7.0f, pwrY - 7.0f, 14.0f, 14.0f);

    // Crystal facet highlight
    g.setColour(juce::Colour(0xffffffff));
    g.fillEllipse(pwrX - 4.0f, pwrY - 4.0f, 3.5f, 3.5f);

    // 7. Metal Nameplate Badge on the Middle Wood Bar
    juce::Rectangle<float> badgeRect(totalArea.getWidth() - 240.0f, 447.0f, 210.0f, 28.0f);
    juce::ColourGradient badgeGrad(juce::Colour(0xff1f2126), 0.0f, badgeRect.getY(),
                                   juce::Colour(0xff0e0f12), 0.0f, badgeRect.getBottom(), false);
    g.setGradientFill(badgeGrad);
    g.fillRoundedRectangle(badgeRect, 3.0f);

    // Gold / brass border
    g.setColour(juce::Colour(0xffc89240));
    g.drawRoundedRectangle(badgeRect, 3.0f, 1.2f);

    // Badge corner screws
    drawScrewHead(g, badgeRect.getX() + 6.0f, badgeRect.getY() + 6.0f, 2.5f);
    drawScrewHead(g, badgeRect.getRight() - 6.0f, badgeRect.getY() + 6.0f, 2.5f);
    drawScrewHead(g, badgeRect.getX() + 6.0f, badgeRect.getBottom() - 6.0f, 2.5f);
    drawScrewHead(g, badgeRect.getRight() - 6.0f, badgeRect.getBottom() - 6.0f, 2.5f);

    // Embossed metal name text
    g.setFont(juce::FontOptions(13.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xffe89a38));
    g.drawText("N E O   A N A L O G", badgeRect.toNearestInt(), juce::Justification::centred);

    // 8. Performance Wheels Well (Dark recessed area to the left of the keyboard)
    juce::Rectangle<float> wheelsWell(20.0f, 482.0f, 116.0f, 190.0f);
    juce::ColourGradient wellGrad(juce::Colour(0xff151619), 0.0f, wheelsWell.getY(),
                                  juce::Colour(0xff0d0e10), 0.0f, wheelsWell.getBottom(), false);
    g.setGradientFill(wellGrad);
    g.fillRect(wheelsWell);
    g.setColour(juce::Colour(0xff2d3038));
    g.drawRect(wheelsWell, 1.0f);

    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff8c92a0));
    g.drawText("PITCH", 28, static_cast<int>(wheelsWell.getBottom() - 16.0f), 44, 14, juce::Justification::centred);
    g.drawText("MOD", 78, static_cast<int>(wheelsWell.getBottom() - 16.0f), 44, 14, juce::Justification::centred);
}

void LadderMonoAudioProcessorEditor::resized()
{
    // Top Bar (y: 0..42)
    titleLabel.setBounds(62, 8, 200, 26);
    presetButton.setBounds(getWidth() - 505, 8, 215, 26);
    browseBtn.setBounds(getWidth() - 285, 8, 65, 26);
    prevPresetBtn.setBounds(getWidth() - 215, 8, 30, 26);
    nextPresetBtn.setBounds(getWidth() - 180, 8, 30, 26);
    initBtn.setBounds(getWidth() - 145, 8, 48, 26);

    // Section 1: Controllers (x: 24..164, y: 46..434)
    tuneKnob.slider.setBounds(30, 78, 60, 60);
    tuneKnob.label.setBounds(30, 138, 60, 14);

    glideKnob.slider.setBounds(96, 78, 60, 60);
    glideKnob.label.setBounds(96, 138, 60, 14);

    modMixKnob.slider.setBounds(30, 168, 60, 60);
    modMixKnob.label.setBounds(30, 228, 60, 14);

    lfoRateKnob.slider.setBounds(96, 168, 60, 60);
    lfoRateKnob.label.setBounds(96, 228, 60, 14);

    oscModBtn.setBounds(35, 265, 115, 34);
    filterModBtn.setBounds(35, 315, 115, 34);

    // Section 2: Arpeggiator (x: 168..308, y: 46..434)
    arpOnBtn.setBounds(180, 85, 115, 34);
    arpLatchBtn.setBounds(180, 130, 115, 34);
    arpModeBox.setBounds(180, 185, 115, 24);
    arpOctaveBox.setBounds(180, 230, 115, 24);
    arpRateBox.setBounds(180, 275, 115, 24);

    // Section 3: Oscillator Bank (x: 312..532, y: 46..434)
    // Osc 1
    osc1RangeBox.setBounds(325, 80, 85, 24);
    osc1WaveBox.setBounds(425, 80, 95, 24);

    // Osc 2
    osc2RangeBox.setBounds(325, 140, 85, 24);
    osc2WaveBox.setBounds(425, 140, 95, 24);
    osc2FineKnob.slider.setBounds(375, 175, 55, 55);
    osc2FineKnob.label.setBounds(375, 230, 55, 14);

    // Osc 3
    osc3RangeBox.setBounds(325, 265, 85, 24);
    osc3WaveBox.setBounds(425, 265, 95, 24);
    osc3FineKnob.slider.setBounds(325, 310, 55, 55);
    osc3FineKnob.label.setBounds(325, 365, 55, 14);
    osc3KbdBtn.setBounds(415, 320, 105, 34);

    // Section 4: Mixer (x: 536..726, y: 46..434)
    mix1Knob.slider.setBounds(548, 70, 50, 50);
    mix1Knob.label.setBounds(548, 120, 50, 14);
    mix1Btn.setBounds(610, 78, 85, 34);

    mix2Knob.slider.setBounds(548, 135, 50, 50);
    mix2Knob.label.setBounds(548, 185, 50, 14);
    mix2Btn.setBounds(610, 143, 85, 34);

    mix3Knob.slider.setBounds(548, 200, 50, 50);
    mix3Knob.label.setBounds(548, 250, 50, 14);
    mix3Btn.setBounds(610, 208, 85, 34);

    mixNoiseKnob.slider.setBounds(548, 265, 50, 50);
    mixNoiseKnob.label.setBounds(548, 315, 50, 14);
    mixNoiseBtn.setBounds(610, 268, 85, 34);
    noiseColorBox.setBounds(610, 306, 85, 22);

    mixExtKnob.slider.setBounds(548, 335, 48, 48);
    mixExtKnob.label.setBounds(548, 383, 48, 14);
    mixExtBtn.setBounds(610, 342, 85, 34);
    driveKnob.slider.setBounds(608, 384, 42, 42);
    driveKnob.label.setBounds(608, 368, 42, 12);

    // Section 5: Modifiers (x: 730..980, y: 46..434)
    // Filter Core
    cutoffKnob.slider.setBounds(740, 70, 62, 62);
    cutoffKnob.label.setBounds(740, 132, 62, 14);

    emphasisKnob.slider.setBounds(822, 70, 62, 62);
    emphasisKnob.label.setBounds(822, 132, 62, 14);

    contourKnob.slider.setBounds(904, 70, 62, 62);
    contourKnob.label.setBounds(904, 132, 62, 14);

    kbd1Btn.setBounds(745, 155, 75, 34);
    kbd2Btn.setBounds(830, 155, 75, 34);
    bassCompKnob.slider.setBounds(915, 148, 50, 50);
    bassCompKnob.label.setBounds(915, 198, 50, 14);

    // Filter Env (Row 3)
    fAttackKnob.slider.setBounds(745, 222, 52, 52);
    fAttackKnob.label.setBounds(745, 274, 52, 14);
    fDecayKnob.slider.setBounds(825, 222, 52, 52);
    fDecayKnob.label.setBounds(825, 274, 52, 14);
    fSustainKnob.slider.setBounds(905, 222, 52, 52);
    fSustainKnob.label.setBounds(905, 274, 52, 14);

    // Amp Env (Row 4)
    aAttackKnob.slider.setBounds(745, 308, 52, 52);
    aAttackKnob.label.setBounds(745, 360, 52, 14);
    aDecayKnob.slider.setBounds(825, 308, 52, 52);
    aDecayKnob.label.setBounds(825, 360, 52, 14);
    aSustainKnob.slider.setBounds(905, 308, 52, 52);
    aSustainKnob.label.setBounds(905, 360, 52, 14);

    decaySwitchBtn.setBounds(795, 395, 120, 34);

    // Section 6: Output & Master (x: 984..1096, y: 46..434)
    masterVolKnob.slider.setBounds(1005, 70, 72, 72);
    masterVolKnob.label.setBounds(1005, 142, 72, 14);

    analogKnob.slider.setBounds(1010, 175, 60, 60);
    analogKnob.label.setBounds(1010, 235, 60, 14);

    voicesBox.setBounds(1000, 275, 85, 24);

    // Middle Separation Wood Bar (y: 440..482)
    keyboardHintLabel.setBounds(140, 448, 680, 26);

    // Lower Performance Section (y: 482..672)
    pitchWheel.setBounds(28, 492, 44, 160);
    modWheel.setBounds(78, 492, 44, 160);
    keyboard.setBounds(136, 482, getWidth() - 156, 190);

    if (presetBrowser != nullptr)
        presetBrowser->setBounds(getLocalBounds());
}
