#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ThemePalette.h"
#include <iomanip>
#include <sstream>

namespace LadderMono
{
    // Custom LookAndFeel for visual rainbow and gradient slider tracks
    class SpectrumSliderLnf : public juce::LookAndFeel_V4
    {
    public:
        float currentHue = 0.0f;
        float currentSat = 1.0f;
        float currentBri = 0.5f;

        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                              juce::Slider::SliderStyle /*style*/, juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                                 static_cast<float>(width), static_cast<float>(height))
                               .reduced(2.0f, height * 0.22f);
            float r = 4.0f;

            if (slider.getName() == "HUE")
            {
                // Full rainbow spectrum from left to right
                juce::ColourGradient rainbow(juce::Colour::fromHSV(0.0f, 1.0f, 1.0f, 1.0f), bounds.getX(), bounds.getY(),
                                             juce::Colour::fromHSV(1.0f, 1.0f, 1.0f, 1.0f), bounds.getRight(), bounds.getY(), false);
                rainbow.addColour(0.166f, juce::Colour::fromHSV(0.166f, 1.0f, 1.0f, 1.0f));
                rainbow.addColour(0.333f, juce::Colour::fromHSV(0.333f, 1.0f, 1.0f, 1.0f));
                rainbow.addColour(0.500f, juce::Colour::fromHSV(0.500f, 1.0f, 1.0f, 1.0f));
                rainbow.addColour(0.666f, juce::Colour::fromHSV(0.666f, 1.0f, 1.0f, 1.0f));
                rainbow.addColour(0.833f, juce::Colour::fromHSV(0.833f, 1.0f, 1.0f, 1.0f));
                g.setGradientFill(rainbow);
                g.fillRoundedRectangle(bounds, r);
            }
            else if (slider.getName() == "LIGHT")
            {
                // Lightness gradient: Black -> Current Hue -> White
                juce::Colour midCol = juce::Colour::fromHSV(currentHue, 0.8f, 0.5f, 1.0f);
                juce::ColourGradient lightGrad(juce::Colours::black, bounds.getX(), bounds.getY(),
                                               juce::Colours::white, bounds.getRight(), bounds.getY(), false);
                lightGrad.addColour(0.5, midCol);
                g.setGradientFill(lightGrad);
                g.fillRoundedRectangle(bounds, r);
            }
            else if (slider.getName() == "SAT")
            {
                // Saturation gradient: Muted Gray -> Saturated Hue
                juce::Colour satCol = juce::Colour::fromHSV(currentHue, 1.0f, 0.8f, 1.0f);
                juce::ColourGradient satGrad(juce::Colour(0xff555a66), bounds.getX(), bounds.getY(),
                                             satCol, bounds.getRight(), bounds.getY(), false);
                g.setGradientFill(satGrad);
                g.fillRoundedRectangle(bounds, r);
            }
            else if (slider.getName() == "OPACITY")
            {
                // Checkerboard pattern for alpha visibility
                float boxSize = 4.0f;
                for (float cy = bounds.getY(); cy < bounds.getBottom(); cy += boxSize)
                {
                    for (float cx = bounds.getX(); cx < bounds.getRight(); cx += boxSize)
                    {
                        bool dark = (static_cast<int>((cx - bounds.getX()) / boxSize) + static_cast<int>((cy - bounds.getY()) / boxSize)) % 2 == 0;
                        g.setColour(dark ? juce::Colour(0xff181a20) : juce::Colour(0xff2c303c));
                        g.fillRect(juce::Rectangle<float>(cx, cy, boxSize, boxSize).getIntersection(bounds));
                    }
                }
                juce::Colour solidCol = juce::Colour::fromHSV(currentHue, currentSat, currentBri, 1.0f);
                juce::ColourGradient alphaGrad(solidCol.withAlpha(0.0f), bounds.getX(), bounds.getY(),
                                               solidCol, bounds.getRight(), bounds.getY(), false);
                g.setGradientFill(alphaGrad);
                g.fillRoundedRectangle(bounds, r);
            }
            else
            {
                // Generic style sliders (Font Size, Rounding, Border Width)
                g.setColour(juce::Colour(0xff181a22));
                g.fillRoundedRectangle(bounds, r);

                // Sleek left-to-right filled value bar
                float fillW = juce::jlimit(0.0f, bounds.getWidth(), sliderPos - bounds.getX());
                if (fillW > 0.0f)
                {
                    auto fillBounds = bounds.withWidth(fillW);
                    g.setColour(juce::Colour(0xff4a5266));
                    g.fillRoundedRectangle(fillBounds, r);
                }
            }

            g.setColour(juce::Colour(0x66ffffff));
            g.drawRoundedRectangle(bounds, r, 0.9f);

            // Thumb cursor (sleek white vector indicator)
            float thumbX = sliderPos;
            float thumbH = bounds.getHeight() + 6.0f;
            float thumbW = 6.0f;
            juce::Rectangle<float> thumb(thumbX - thumbW * 0.5f, bounds.getCentreY() - thumbH * 0.5f, thumbW, thumbH);

            g.setColour(juce::Colour(0x99000000));
            g.fillRoundedRectangle(thumb.expanded(1.0f), 2.5f);
            g.setColour(juce::Colours::white);
            g.fillRoundedRectangle(thumb, 2.0f);
            g.setColour(juce::Colours::black);
            g.drawRoundedRectangle(thumb, 2.0f, 0.8f);
        }
    };

    class ThemeEditorPanel : public juce::Component
    {
    public:
        ThemeEditorPanel()
        {
            // Close button
            closeBtn.setButtonText(juce::CharPointer_UTF8("\xe2\x9c\x95"));
            closeBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22242c));
            closeBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd4d8e2));
            closeBtn.onClick = [this] { setVisible(false); };
            addAndMakeVisible(closeBtn);

            // Column 1: Theme & Clipboard Action Buttons
            auto setupActionBtn = [this](juce::TextButton& btn, const juce::String& text, std::function<void()> onClick) {
                btn.setButtonText(text);
                btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e1e1e));
                btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff2a2a2a));
                btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd0d0d0));
                btn.onClick = onClick;
                addAndMakeVisible(btn);
            };

            setupActionBtn(noirPresetBtn, "Noir", [this] {
                activeTheme = ThemeColors::getDefaultNoir();
                notifyThemeChanged();
                clipStatusLabel.setText("Noir theme loaded", juce::dontSendNotification);
            });

            setupActionBtn(copyThemeBtn, "Copy Preset", [this] {
                juce::SystemClipboard::copyTextToClipboard(activeTheme.toJSONString());
                copyThemeBtn.setButtonText("Copied!");
                clipStatusLabel.setText("Copied to clipboard!", juce::dontSendNotification);
                juce::Timer::callAfterDelay(1600, [safe = juce::Component::SafePointer<juce::TextButton>(&copyThemeBtn),
                                                   safeLbl = juce::Component::SafePointer<juce::Label>(&clipStatusLabel)] {
                    if (safe != nullptr) safe->setButtonText("Copy Preset");
                    if (safeLbl != nullptr) safeLbl->setText("", juce::dontSendNotification);
                });
            });

            setupActionBtn(pasteThemeBtn, "Paste Preset", [this] {
                auto clip = juce::SystemClipboard::getTextFromClipboard().trim();
                if (clip.isNotEmpty() && activeTheme.fromJSONString(clip))
                {
                    notifyThemeChanged();
                    pasteThemeBtn.setButtonText("Applied!");
                    clipStatusLabel.setText("Theme applied!", juce::dontSendNotification);
                    juce::Timer::callAfterDelay(1600, [safe = juce::Component::SafePointer<juce::TextButton>(&pasteThemeBtn),
                                                       safeLbl = juce::Component::SafePointer<juce::Label>(&clipStatusLabel)] {
                        if (safe != nullptr) safe->setButtonText("Paste Preset");
                        if (safeLbl != nullptr) safeLbl->setText("", juce::dontSendNotification);
                    });
                }
                else
                {
                    pasteThemeBtn.setButtonText("Invalid JSON");
                    clipStatusLabel.setText("Clipboard is not a theme", juce::dontSendNotification);
                    juce::Timer::callAfterDelay(2000, [safe = juce::Component::SafePointer<juce::TextButton>(&pasteThemeBtn),
                                                       safeLbl = juce::Component::SafePointer<juce::Label>(&clipStatusLabel)] {
                        if (safe != nullptr) safe->setButtonText("Paste Preset");
                        if (safeLbl != nullptr) safeLbl->setText("", juce::dontSendNotification);
                    });
                }
            });

            clipStatusLabel.setFont(juce::FontOptions(9.0f, juce::Font::plain));
            clipStatusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d0bc));
            clipStatusLabel.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(clipStatusLabel);

            // Target Property Selector
            targetBox.addItem("Lines & Borders", 1);
            targetBox.addItem("Section Accents", 2);
            targetBox.addItem("Knob Cylinders", 3);
            targetBox.addItem("Knob Indicators", 4);
            targetBox.addItem("Button Faceplates", 5);
            targetBox.addItem("Dropdown Backgrounds", 6);
            targetBox.addItem("White / Natural Keys", 7);
            targetBox.addItem("Dark / Accidental Keys", 8);
            targetBox.addItem("Active Switches (LED)", 9);
            targetBox.addItem("Primary Text", 10);
            targetBox.addItem("Secondary Text", 11);
            targetBox.addItem("Panel Faceplate", 12);
            targetBox.addItem("Cutout Wells", 13);
            targetBox.addItem("Chassis Base", 14);
            targetBox.addItem("OLED Screen Glass", 15);
            targetBox.setSelectedId(1, juce::dontSendNotification);
            targetBox.onChange = [this] { updateSlidersFromCurrentTarget(); };
            addAndMakeVisible(targetBox);

            // Hex Code Input
            hexLabel.setText("HEX CODE", juce::dontSendNotification);
            hexLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            hexLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9c9c9c));
            addAndMakeVisible(hexLabel);

            hexInput.setMultiLine(false);
            hexInput.setReturnKeyStartsNewLine(false);
            hexInput.setFont(juce::FontOptions(13.0f, juce::Font::bold));
            hexInput.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff141414));
            hexInput.setColour(juce::TextEditor::textColourId, juce::Colour(0xffe0e0e0));
            hexInput.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff282828));
            hexInput.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff444444));
            hexInput.setInputRestrictions(9, "#0123456789abcdefABCDEF");
            hexInput.onTextChange = [this] { onHexInputChanged(); };
            addAndMakeVisible(hexInput);

            // Visual Spectrum Sliders with custom rainbow look
            auto setupColorSlider = [this](juce::Slider& s, juce::Label& lbl, const juce::String& name, double min, double max, double init) {
                s.setName(name);
                s.setLookAndFeel(&spectrumLnf);
                s.setSliderStyle(juce::Slider::LinearHorizontal);
                s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 38, 16);
                s.setRange(min, max, 0.01);
                s.setValue(init, juce::dontSendNotification);
                s.onValueChange = [this] { onColorSliderChanged(); };
                addAndMakeVisible(s);

                lbl.setText(name, juce::dontSendNotification);
                lbl.setFont(juce::FontOptions(9.5f, juce::Font::bold));
                lbl.setColour(juce::Label::textColourId, juce::Colour(0xff9c9c9c));
                addAndMakeVisible(lbl);
            };

            setupColorSlider(hueSlider, hueLabel, "HUE", 0.0, 1.0, 0.0);
            setupColorSlider(satSlider, satLabel, "SAT", 0.0, 1.0, 0.0);
            setupColorSlider(briSlider, briLabel, "LIGHT", 0.0, 1.0, 0.5);
            setupColorSlider(alphaSlider, alphaLabel, "OPACITY", 0.0, 1.0, 1.0);
            alphaSlider.textFromValueFunction = [](double v) {
                return juce::String(juce::roundToInt(v * 100.0)) + "%";
            };

            // 8 Instant Color Chips (One-click colors!)
            struct ChipDef { const char* name; juce::Colour col; };
            std::vector<ChipDef> chips = {
                { "Black",   juce::Colour(0xff08090a) },
                { "Dark",    juce::Colour(0xff181a20) },
                { "Slate",   juce::Colour(0xff444750) },
                { "Amber",   juce::Colour(0xffe08b3c) },
                { "Cyan",    juce::Colour(0xff00ffe1) },
                { "Blue",    juce::Colour(0xff3b82f6) },
                { "Crimson", juce::Colour(0xffe11d48) },
                { "White",   juce::Colour(0xffffffff) }
            };

            for (const auto& chip : chips)
            {
                auto btn = std::make_unique<juce::TextButton>();
                btn->setButtonText("");
                btn->setTooltip(chip.name);
                btn->setColour(juce::TextButton::buttonColourId, chip.col);
                btn->setColour(juce::TextButton::buttonOnColourId, chip.col.brighter(0.2f));
                juce::Colour chipCol = chip.col;
                btn->onClick = [this, chipCol] {
                    setCurrentTargetColor(chipCol);
                    notifyThemeChanged();
                };
                addAndMakeVisible(*btn);
                colorChips.push_back(std::move(btn));
            }

            // Geometry & Font controls
            fontScaleLabel.setText("FONT SIZE", juce::dontSendNotification);
            fontScaleLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
            fontScaleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9c9c9c));
            addAndMakeVisible(fontScaleLabel);

            fontScaleSlider.setLookAndFeel(&spectrumLnf);
            fontScaleSlider.setSliderStyle(juce::Slider::LinearHorizontal);
            fontScaleSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 16);
            fontScaleSlider.setRange(0.85, 1.30, 0.05);
            fontScaleSlider.setValue(activeTheme.fontScale, juce::dontSendNotification);
            fontScaleSlider.textFromValueFunction = [](double v) {
                return juce::String(juce::roundToInt(v * 100.0)) + "%";
            };
            fontScaleSlider.onValueChange = [this] {
                if (isUpdatingSliders) return;
                activeTheme.fontScale = static_cast<float>(fontScaleSlider.getValue());
                if (onThemeChanged) onThemeChanged(activeTheme);
            };
            addAndMakeVisible(fontScaleSlider);

            cornerRadiusLabel.setText("ROUNDING", juce::dontSendNotification);
            cornerRadiusLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
            cornerRadiusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9c9c9c));
            addAndMakeVisible(cornerRadiusLabel);

            cornerRadiusSlider.setLookAndFeel(&spectrumLnf);
            cornerRadiusSlider.setSliderStyle(juce::Slider::LinearHorizontal);
            cornerRadiusSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 16);
            cornerRadiusSlider.setRange(0.0, 12.0, 0.5);
            cornerRadiusSlider.setValue(activeTheme.cornerRadius, juce::dontSendNotification);
            cornerRadiusSlider.textFromValueFunction = [](double v) {
                return juce::String(v, 1) + "px";
            };
            cornerRadiusSlider.onValueChange = [this] {
                if (isUpdatingSliders) return;
                activeTheme.cornerRadius = static_cast<float>(cornerRadiusSlider.getValue());
                if (onThemeChanged) onThemeChanged(activeTheme);
            };
            addAndMakeVisible(cornerRadiusSlider);

            borderWidthLabel.setText("LINE THICKNESS", juce::dontSendNotification);
            borderWidthLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
            borderWidthLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9c9c9c));
            addAndMakeVisible(borderWidthLabel);

            borderWidthSlider.setLookAndFeel(&spectrumLnf);
            borderWidthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
            borderWidthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 16);
            borderWidthSlider.setRange(0.5, 4.0, 0.25);
            borderWidthSlider.setValue(activeTheme.borderWidth, juce::dontSendNotification);
            borderWidthSlider.textFromValueFunction = [](double v) {
                return juce::String(v, 2) + "px";
            };
            borderWidthSlider.onValueChange = [this] {
                if (isUpdatingSliders) return;
                activeTheme.borderWidth = static_cast<float>(borderWidthSlider.getValue());
                if (onThemeChanged) onThemeChanged(activeTheme);
            };
            addAndMakeVisible(borderWidthSlider);

            // Reset Button
            resetBtn.setButtonText("Reset to Default");
            resetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
            resetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe0e0e0));
            resetBtn.onClick = [this] {
                activeTheme = ThemeColors::getDefaultNoir();
                notifyThemeChanged();
            };
            addAndMakeVisible(resetBtn);

            updateSlidersFromCurrentTarget();
        }

        ~ThemeEditorPanel() override
        {
            hueSlider.setLookAndFeel(nullptr);
            satSlider.setLookAndFeel(nullptr);
            briSlider.setLookAndFeel(nullptr);
            alphaSlider.setLookAndFeel(nullptr);
            fontScaleSlider.setLookAndFeel(nullptr);
            cornerRadiusSlider.setLookAndFeel(nullptr);
            borderWidthSlider.setLookAndFeel(nullptr);
        }

        std::function<void(const ThemeColors&)> onThemeChanged;

        void setTheme(const ThemeColors& t)
        {
            activeTheme = t;
            updateSlidersFromCurrentTarget();
            repaint();
        }

        const ThemeColors& getTheme() const noexcept { return activeTheme; }

        bool hasBeenMoved = false;

        void mouseDown(const juce::MouseEvent& e) override
        {
            dragger.startDraggingComponent(this, e);
            toFront(true);
        }

        void mouseDrag(const juce::MouseEvent& e) override
        {
            dragger.dragComponent(this, e, nullptr);
            hasBeenMoved = true;
            if (auto* parent = getParentComponent())
            {
                int minVisible = 48;
                int clampedX = juce::jlimit(-getWidth() + minVisible, parent->getWidth() - minVisible, getX());
                int clampedY = juce::jlimit(0, parent->getHeight() - 32, getY());
                if (clampedX != getX() || clampedY != getY())
                    setTopLeftPosition(clampedX, clampedY);
            }
        }

        void mouseMove(const juce::MouseEvent& e) override
        {
            if (e.y <= 30)
                setMouseCursor(juce::MouseCursor::DraggingHandCursor);
            else
                setMouseCursor(juce::MouseCursor::NormalCursor);
        }

        void mouseDoubleClick(const juce::MouseEvent& e) override
        {
            if (e.y <= 30)
            {
                hasBeenMoved = false;
                if (auto* parent = getParentComponent())
                {
                    int defaultX = (parent->getWidth() - getWidth()) / 2;
                    int defaultY = 46;
                    setTopLeftPosition(defaultX, defaultY);
                }
            }
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            float r = activeTheme.cornerRadius;

            // Deep floating studio chassis
            g.setColour(juce::Colour(0xd0000000));
            g.fillRoundedRectangle(bounds, r + 2.0f);

            g.setColour(activeTheme.panelBg);
            g.fillRoundedRectangle(bounds.reduced(1.0f), r);

            g.setColour(activeTheme.lineBorder);
            g.drawRoundedRectangle(bounds.reduced(1.0f), r, 1.2f);

            // Header Banner (Draggable handle)
            juce::Rectangle<float> headerBar(1.0f, 1.0f, bounds.getWidth() - 2.0f, 28.0f);
            g.setColour(activeTheme.panelLight);
            g.fillRoundedRectangle(headerBar, r);
            g.setColour(activeTheme.lineBorder);
            g.drawHorizontalLine(29, 1.0f, bounds.getWidth() - 1.0f);

            g.setFont(juce::FontOptions(11.0f * activeTheme.fontScale, juce::Font::bold));
            g.setColour(activeTheme.textPrimary);
            g.drawText("COLOR THEME STUDIO | DRAG HEADER TO MOVE",
                       14, 0, static_cast<int>(bounds.getWidth() - 80.0f), 28, juce::Justification::centredLeft);

            // Subtle drag handle grip dots icon
            g.setColour(activeTheme.textDim.withAlpha(0.6f));
            float gripX = bounds.getWidth() - 60.0f;
            for (int row = 0; row < 2; ++row)
            {
                for (int col = 0; col < 4; ++col)
                {
                    g.fillEllipse(gripX + col * 5.0f, 10.0f + row * 5.0f, 2.5f, 2.5f);
                }
            }

            // Vertical Dividers separating the 4 horizontal columns (de izquierda a derecha)
            g.setColour(activeTheme.lineBorder);
            g.drawVerticalLine(174, 34.0f, bounds.getHeight() - 10.0f);
            g.drawVerticalLine(388, 34.0f, bounds.getHeight() - 10.0f);
            g.drawVerticalLine(668, 34.0f, bounds.getHeight() - 10.0f);

            // Column Headers (Left to Right)
            g.setFont(juce::FontOptions(9.5f * activeTheme.fontScale, juce::Font::bold));
            g.setColour(activeTheme.textSecondary);
            g.drawText("1. THEME & CLIPBOARD", 14, 34, 150, 16, juce::Justification::centredLeft);
            g.drawText("2. COMPONENT & HEX", 184, 34, 194, 16, juce::Justification::centredLeft);
            g.drawText("3. COLOR SPECTRUM", 398, 34, 260, 16, juce::Justification::centredLeft);
            g.drawText("4. STYLE & RESET", 678, 34, 170, 16, juce::Justification::centredLeft);

            // Large Live Swatch Preview in Column 2 (with checkerboard underlay for alpha)
            auto swatch = juce::Rectangle<float>(184.0f, 116.0f, 44.0f, 44.0f);
            float boxSize = 5.0f;
            for (float cy = swatch.getY(); cy < swatch.getBottom(); cy += boxSize)
            {
                for (float cx = swatch.getX(); cx < swatch.getRight(); cx += boxSize)
                {
                    bool dark = (static_cast<int>((cx - swatch.getX()) / boxSize) + static_cast<int>((cy - swatch.getY()) / boxSize)) % 2 == 0;
                    g.setColour(dark ? juce::Colour(0xff181818) : juce::Colour(0xff303030));
                    g.fillRect(juce::Rectangle<float>(cx, cy, boxSize, boxSize).getIntersection(swatch));
                }
            }
            g.setColour(getCurrentTargetColor());
            g.fillRoundedRectangle(swatch, juce::jmin(r, 4.0f));
            g.setColour(activeTheme.lineBorder);
            g.drawRoundedRectangle(swatch, juce::jmin(r, 4.0f), 1.2f);

            // Quick Chips Label in Column 3
            g.setFont(juce::FontOptions(8.5f * activeTheme.fontScale, juce::Font::bold));
            g.setColour(activeTheme.textDim);
            g.drawText("QUICK COLOR CHIPS", 398, 154, 250, 14, juce::Justification::centredLeft);
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            closeBtn.setBounds(bounds.getWidth() - 26, 3, 22, 22);

            // ================================================================
            // Column 1: THEME & CLIPBOARD (x: 14, w: 150)
            // ================================================================
            int py = 54;
            int btnH = 26;
            int gap = 8;
            noirPresetBtn.setBounds(14, py, 150, btnH);
            copyThemeBtn.setBounds(14, py + (btnH + gap), 150, btnH);
            pasteThemeBtn.setBounds(14, py + (btnH + gap) * 2, 150, btnH);
            clipStatusLabel.setBounds(14, py + (btnH + gap) * 3 + 4, 150, 18);

            // ================================================================
            // Column 2: TARGET & HEX (x: 184, w: 194)
            // ================================================================
            targetBox.setBounds(184, 54, 194, 24);

            hexLabel.setBounds(236, 112, 140, 16);
            hexInput.setBounds(236, 128, 142, 28);

            // ================================================================
            // Column 3: VISUAL COLOR PICKER (x: 398, w: 260)
            // Full left-to-right visual rainbow spectrum
            // ================================================================
            int sy = 52;
            hueLabel.setBounds(398, sy, 46, 18);
            hueSlider.setBounds(448, sy, 210, 18);

            sy += 23;
            satLabel.setBounds(398, sy, 46, 18);
            satSlider.setBounds(448, sy, 210, 18);

            sy += 23;
            briLabel.setBounds(398, sy, 46, 18);
            briSlider.setBounds(448, sy, 210, 18);

            sy += 23;
            alphaLabel.setBounds(398, sy, 46, 18);
            alphaSlider.setBounds(448, sy, 210, 18);

            // 8 Quick Color Chips (Horizontal Row)
            int chipX = 398;
            int chipY = 172;
            int chipSize = 25;
            for (size_t i = 0; i < colorChips.size(); ++i)
            {
                colorChips[i]->setBounds(chipX + static_cast<int>(i) * 32, chipY, chipSize, chipSize);
            }

            // ================================================================
            // Column 4: STYLE & RESET (x: 678, w: 170)
            // ================================================================
            int gy = 46;
            fontScaleLabel.setBounds(678, gy, 170, 15);
            fontScaleSlider.setBounds(678, gy + 15, 170, 20);

            gy += 38;
            cornerRadiusLabel.setBounds(678, gy, 170, 15);
            cornerRadiusSlider.setBounds(678, gy + 15, 170, 20);

            gy += 38;
            borderWidthLabel.setBounds(678, gy, 170, 15);
            borderWidthSlider.setBounds(678, gy + 15, 170, 20);

            resetBtn.setBounds(678, bounds.getHeight() - 36, 170, 26);
        }

    private:
        ThemeColors activeTheme;
        bool isUpdatingSliders = false;
        juce::ComponentDragger dragger;
        SpectrumSliderLnf spectrumLnf;

        juce::TextButton closeBtn;
        juce::TextButton noirPresetBtn;
        juce::TextButton copyThemeBtn;
        juce::TextButton pasteThemeBtn;
        juce::Label clipStatusLabel;

        juce::ComboBox targetBox;
        juce::Label hexLabel;
        juce::TextEditor hexInput;

        juce::Label hueLabel, satLabel, briLabel, alphaLabel;
        juce::Slider hueSlider, satSlider, briSlider, alphaSlider;
        std::vector<std::unique_ptr<juce::TextButton>> colorChips;

        juce::Label fontScaleLabel, cornerRadiusLabel, borderWidthLabel;
        juce::Slider fontScaleSlider, cornerRadiusSlider, borderWidthSlider;

        juce::TextButton resetBtn;

        static juce::String colorToHex(juce::Colour c)
        {
            if (c.getAlpha() < 255)
            {
                return "#" + juce::String::toHexString(c.getRed()).paddedLeft('0', 2).toUpperCase()
                           + juce::String::toHexString(c.getGreen()).paddedLeft('0', 2).toUpperCase()
                           + juce::String::toHexString(c.getBlue()).paddedLeft('0', 2).toUpperCase()
                           + juce::String::toHexString(c.getAlpha()).paddedLeft('0', 2).toUpperCase();
            }
            return "#" + juce::String::toHexString((c.getRed() << 16) | (c.getGreen() << 8) | c.getBlue()).paddedLeft('0', 6).toUpperCase();
        }

        juce::Colour getCurrentTargetColor() const
        {
            int id = targetBox.getSelectedId();
            switch (id)
            {
                case 1:  return activeTheme.lineBorder;
                case 2:  return activeTheme.lineAccent;
                case 3:  return activeTheme.knobBody;
                case 4:  return activeTheme.knobIndicator;
                case 5:  return activeTheme.buttonBg;
                case 6:  return activeTheme.comboBg;
                case 7:  return activeTheme.keyWhiteRest;
                case 8:  return activeTheme.keyBlackRest;
                case 9:  return activeTheme.switchActive;
                case 10: return activeTheme.textPrimary;
                case 11: return activeTheme.textSecondary;
                case 12: return activeTheme.panelBg;
                case 13: return activeTheme.wellDark;
                case 14: return activeTheme.bgChassis;
                case 15: return activeTheme.screenBg;
                default: return activeTheme.lineBorder;
            }
        }

        void setCurrentTargetColor(juce::Colour c)
        {
            int id = targetBox.getSelectedId();
            switch (id)
            {
                case 1:  activeTheme.lineBorder = c; break;
                case 2:  activeTheme.lineAccent = c; break;
                case 3:  activeTheme.knobBody = c; break;
                case 4:  activeTheme.knobIndicator = c; break;
                case 5:  activeTheme.buttonBg = c; break;
                case 6:  activeTheme.comboBg = c; break;
                case 7:  activeTheme.keyWhiteRest = c; break;
                case 8:  activeTheme.keyBlackRest = c; break;
                case 9:  activeTheme.switchActive = c; break;
                case 10: activeTheme.textPrimary = c; break;
                case 11: activeTheme.textSecondary = c; break;
                case 12: activeTheme.panelBg = c; break;
                case 13: activeTheme.wellDark = c; break;
                case 14: activeTheme.bgChassis = c; break;
                case 15: activeTheme.screenBg = c; break;
            }
        }

        void updateSlidersFromCurrentTarget()
        {
            isUpdatingSliders = true;
            juce::Colour c = getCurrentTargetColor();
            hexInput.setText(colorToHex(c), juce::dontSendNotification);
            hueSlider.setValue(c.getHue(), juce::dontSendNotification);
            satSlider.setValue(c.getSaturation(), juce::dontSendNotification);
            briSlider.setValue(c.getBrightness(), juce::dontSendNotification);
            alphaSlider.setValue(c.getFloatAlpha(), juce::dontSendNotification);
            fontScaleSlider.setValue(activeTheme.fontScale, juce::dontSendNotification);
            cornerRadiusSlider.setValue(activeTheme.cornerRadius, juce::dontSendNotification);
            borderWidthSlider.setValue(activeTheme.borderWidth, juce::dontSendNotification);

            spectrumLnf.currentHue = c.getHue();
            spectrumLnf.currentSat = c.getSaturation();
            spectrumLnf.currentBri = c.getBrightness();
            isUpdatingSliders = false;
            repaint();
        }

        void onHexInputChanged()
        {
            if (isUpdatingSliders) return;
            auto text = hexInput.getText().trim();
            if (text.startsWith("#")) text = text.substring(1);
            if (text.length() == 6)
            {
                try
                {
                    unsigned long val = std::stoul(text.toStdString(), nullptr, 16);
                    float currentAlpha = static_cast<float>(alphaSlider.getValue());
                    juce::Colour newCol = juce::Colour::fromRGBA(static_cast<juce::uint8>((val >> 16) & 0xff),
                                                                 static_cast<juce::uint8>((val >> 8) & 0xff),
                                                                 static_cast<juce::uint8>(val & 0xff),
                                                                 static_cast<juce::uint8>(juce::roundToInt(currentAlpha * 255.0f)));
                    setCurrentTargetColor(newCol);

                    isUpdatingSliders = true;
                    hueSlider.setValue(newCol.getHue(), juce::dontSendNotification);
                    satSlider.setValue(newCol.getSaturation(), juce::dontSendNotification);
                    briSlider.setValue(newCol.getBrightness(), juce::dontSendNotification);
                    spectrumLnf.currentHue = newCol.getHue();
                    spectrumLnf.currentSat = newCol.getSaturation();
                    spectrumLnf.currentBri = newCol.getBrightness();
                    isUpdatingSliders = false;

                    if (onThemeChanged) onThemeChanged(activeTheme);
                    repaint();
                }
                catch (...) {}
            }
            else if (text.length() == 8)
            {
                try
                {
                    unsigned long long val = std::stoull(text.toStdString(), nullptr, 16);
                    juce::uint8 r = static_cast<juce::uint8>((val >> 24) & 0xff);
                    juce::uint8 g = static_cast<juce::uint8>((val >> 16) & 0xff);
                    juce::uint8 b = static_cast<juce::uint8>((val >> 8) & 0xff);
                    juce::uint8 a = static_cast<juce::uint8>(val & 0xff);
                    juce::Colour newCol = juce::Colour::fromRGBA(r, g, b, a);
                    setCurrentTargetColor(newCol);

                    isUpdatingSliders = true;
                    hueSlider.setValue(newCol.getHue(), juce::dontSendNotification);
                    satSlider.setValue(newCol.getSaturation(), juce::dontSendNotification);
                    briSlider.setValue(newCol.getBrightness(), juce::dontSendNotification);
                    alphaSlider.setValue(newCol.getFloatAlpha(), juce::dontSendNotification);
                    spectrumLnf.currentHue = newCol.getHue();
                    spectrumLnf.currentSat = newCol.getSaturation();
                    spectrumLnf.currentBri = newCol.getBrightness();
                    isUpdatingSliders = false;

                    if (onThemeChanged) onThemeChanged(activeTheme);
                    repaint();
                }
                catch (...) {}
            }
        }

        void onColorSliderChanged()
        {
            if (isUpdatingSliders) return;
            float h = static_cast<float>(hueSlider.getValue());
            float s = static_cast<float>(satSlider.getValue());
            float b = static_cast<float>(briSlider.getValue());
            float a = static_cast<float>(alphaSlider.getValue());

            // If user moved hue while color was pitch black (sat=0 & b=0), auto-revive brightness
            if (b < 0.05f && s < 0.05f)
            {
                s = 0.85f;
                b = 0.60f;
                satSlider.setValue(s, juce::dontSendNotification);
                briSlider.setValue(b, juce::dontSendNotification);
            }

            juce::Colour newColor = juce::Colour::fromHSV(h, s, b, a);
            setCurrentTargetColor(newColor);
            spectrumLnf.currentHue = h;
            spectrumLnf.currentSat = s;
            spectrumLnf.currentBri = b;

            isUpdatingSliders = true;
            hexInput.setText(colorToHex(newColor), juce::dontSendNotification);
            isUpdatingSliders = false;

            notifyThemeChanged();
            repaint();
        }

        void notifyThemeChanged()
        {
            updateSlidersFromCurrentTarget();
            if (onThemeChanged)
                onThemeChanged(activeTheme);
        }
    };
}
