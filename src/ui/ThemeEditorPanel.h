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
            else
            {
                g.setColour(juce::Colour(0xff181a22));
                g.fillRoundedRectangle(bounds, r);
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

            // Preset Buttons
            auto setupPresetBtn = [this](juce::TextButton& btn, const juce::String& text, std::function<void()> onClick) {
                btn.setButtonText(text);
                btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181a20));
                btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff262a34));
                btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc2c7d4));
                btn.onClick = onClick;
                addAndMakeVisible(btn);
            };

            setupPresetBtn(noirPresetBtn, "OP-XY Noir", [this] {
                activeTheme = ThemeColors::getDefaultNoir();
                notifyThemeChanged();
            });

            setupPresetBtn(cyanPresetBtn, "Cyber Cyan", [this] {
                activeTheme = ThemeColors::getCyberCyan();
                notifyThemeChanged();
            });

            setupPresetBtn(amberPresetBtn, "Amber Matrix", [this] {
                activeTheme = ThemeColors::getAmberMatrix();
                notifyThemeChanged();
            });

            setupPresetBtn(bluePresetBtn, "Midnight Blue", [this] {
                activeTheme = ThemeColors::getMidnightBlue();
                notifyThemeChanged();
            });

            setupPresetBtn(crimsonPresetBtn, "Crimson Stealth", [this] {
                activeTheme = ThemeColors::getCrimsonStealth();
                notifyThemeChanged();
            });

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
            hexLabel.setColour(juce::Label::textColourId, juce::Colour(0xff989ea8));
            addAndMakeVisible(hexLabel);

            hexInput.setMultiLine(false);
            hexInput.setReturnKeyStartsNewLine(false);
            hexInput.setFont(juce::FontOptions(13.0f, juce::Font::bold));
            hexInput.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff14161c));
            hexInput.setColour(juce::TextEditor::textColourId, juce::Colour(0xffe2e6f0));
            hexInput.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff2f333e));
            hexInput.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff4a5264));
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
                lbl.setColour(juce::Label::textColourId, juce::Colour(0xff989ea8));
                addAndMakeVisible(lbl);
            };

            setupColorSlider(hueSlider, hueLabel, "HUE", 0.0, 1.0, 0.0);
            setupColorSlider(satSlider, satLabel, "SAT", 0.0, 1.0, 0.0);
            setupColorSlider(briSlider, briLabel, "LIGHT", 0.0, 1.0, 0.5);

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
            fontScaleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff989ea8));
            addAndMakeVisible(fontScaleLabel);

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
            cornerRadiusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff989ea8));
            addAndMakeVisible(cornerRadiusLabel);

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

            // Reset Button
            resetBtn.setButtonText("Reset to Default");
            resetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff221b1b));
            resetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe88a8a));
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
        }

        std::function<void(const ThemeColors&)> onThemeChanged;

        void setTheme(const ThemeColors& t)
        {
            activeTheme = t;
            updateSlidersFromCurrentTarget();
            repaint();
        }

        const ThemeColors& getTheme() const noexcept { return activeTheme; }

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

            // Header Banner
            juce::Rectangle<float> headerBar(1.0f, 1.0f, bounds.getWidth() - 2.0f, 28.0f);
            g.setColour(activeTheme.panelLight);
            g.fillRoundedRectangle(headerBar, r);
            g.setColour(activeTheme.lineBorder);
            g.drawHorizontalLine(29, 1.0f, bounds.getWidth() - 1.0f);

            g.setFont(juce::FontOptions(11.0f * activeTheme.fontScale, juce::Font::bold));
            g.setColour(activeTheme.textPrimary);
            g.drawText("COLOR THEME STUDIO | HORIZONTAL PALETTE EDITOR",
                       14, 0, static_cast<int>(bounds.getWidth() - 50.0f), 28, juce::Justification::centredLeft);

            // Vertical Dividers separating the 4 horizontal columns (de izquierda a derecha)
            g.setColour(activeTheme.lineBorder);
            g.drawVerticalLine(174, 34.0f, bounds.getHeight() - 10.0f);
            g.drawVerticalLine(388, 34.0f, bounds.getHeight() - 10.0f);
            g.drawVerticalLine(668, 34.0f, bounds.getHeight() - 10.0f);

            // Column Headers (Left to Right)
            g.setFont(juce::FontOptions(9.5f * activeTheme.fontScale, juce::Font::bold));
            g.setColour(activeTheme.textSecondary);
            g.drawText("1. PALETTES", 14, 34, 150, 16, juce::Justification::centredLeft);
            g.drawText("2. COMPONENT & HEX", 184, 34, 194, 16, juce::Justification::centredLeft);
            g.drawText("3. COLOR SPECTRUM", 398, 34, 260, 16, juce::Justification::centredLeft);
            g.drawText("4. STYLE & RESET", 678, 34, 170, 16, juce::Justification::centredLeft);

            // Large Live Swatch Preview in Column 2
            auto swatch = juce::Rectangle<float>(184.0f, 116.0f, 44.0f, 44.0f);
            g.setColour(getCurrentTargetColor());
            g.fillRoundedRectangle(swatch, juce::jmin(r, 4.0f));
            g.setColour(activeTheme.lineBorder);
            g.drawRoundedRectangle(swatch, juce::jmin(r, 4.0f), 1.2f);

            // Quick Chips Label in Column 3
            g.setFont(juce::FontOptions(8.5f * activeTheme.fontScale, juce::Font::bold));
            g.setColour(activeTheme.textDim);
            g.drawText("QUICK COLOR CHIPS", 398, 164, 250, 14, juce::Justification::centredLeft);
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            closeBtn.setBounds(bounds.getWidth() - 26, 3, 22, 22);

            // ================================================================
            // Column 1: FACTORY PALETTES (x: 14, w: 150)
            // ================================================================
            int py = 54;
            int btnH = 24;
            int gap = 6;
            noirPresetBtn.setBounds(14, py, 150, btnH);
            cyanPresetBtn.setBounds(14, py + (btnH + gap), 150, btnH);
            amberPresetBtn.setBounds(14, py + (btnH + gap) * 2, 150, btnH);
            bluePresetBtn.setBounds(14, py + (btnH + gap) * 3, 150, btnH);
            crimsonPresetBtn.setBounds(14, py + (btnH + gap) * 4, 150, btnH);

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
            int sy = 54;
            hueLabel.setBounds(398, sy, 46, 20);
            hueSlider.setBounds(448, sy, 210, 20);

            sy += 28;
            satLabel.setBounds(398, sy, 46, 20);
            satSlider.setBounds(448, sy, 210, 20);

            sy += 28;
            briLabel.setBounds(398, sy, 46, 20);
            briSlider.setBounds(448, sy, 210, 20);

            // 8 Quick Color Chips (Horizontal Row)
            int chipX = 398;
            int chipY = 180;
            int chipSize = 25;
            for (size_t i = 0; i < colorChips.size(); ++i)
            {
                colorChips[i]->setBounds(chipX + static_cast<int>(i) * 32, chipY, chipSize, chipSize);
            }

            // ================================================================
            // Column 4: STYLE & RESET (x: 678, w: 170)
            // ================================================================
            int gy = 54;
            fontScaleLabel.setBounds(678, gy, 170, 16);
            fontScaleSlider.setBounds(678, gy + 18, 170, 22);

            gy += 48;
            cornerRadiusLabel.setBounds(678, gy, 170, 16);
            cornerRadiusSlider.setBounds(678, gy + 18, 170, 22);

            resetBtn.setBounds(678, bounds.getHeight() - 36, 170, 26);
        }

    private:
        ThemeColors activeTheme;
        bool isUpdatingSliders = false;
        SpectrumSliderLnf spectrumLnf;

        juce::TextButton closeBtn;
        juce::TextButton noirPresetBtn;
        juce::TextButton cyanPresetBtn;
        juce::TextButton amberPresetBtn;
        juce::TextButton bluePresetBtn;
        juce::TextButton crimsonPresetBtn;

        juce::ComboBox targetBox;
        juce::Label hexLabel;
        juce::TextEditor hexInput;

        juce::Label hueLabel, satLabel, briLabel;
        juce::Slider hueSlider, satSlider, briSlider;
        std::vector<std::unique_ptr<juce::TextButton>> colorChips;

        juce::Label fontScaleLabel, cornerRadiusLabel;
        juce::Slider fontScaleSlider, cornerRadiusSlider;

        juce::TextButton resetBtn;

        static juce::String colorToHex(juce::Colour c)
        {
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
            fontScaleSlider.setValue(activeTheme.fontScale, juce::dontSendNotification);
            cornerRadiusSlider.setValue(activeTheme.cornerRadius, juce::dontSendNotification);

            spectrumLnf.currentHue = c.getHue();
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
                    juce::Colour newCol = juce::Colour::fromRGB((val >> 16) & 0xff, (val >> 8) & 0xff, val & 0xff);
                    setCurrentTargetColor(newCol);

                    isUpdatingSliders = true;
                    hueSlider.setValue(newCol.getHue(), juce::dontSendNotification);
                    satSlider.setValue(newCol.getSaturation(), juce::dontSendNotification);
                    briSlider.setValue(newCol.getBrightness(), juce::dontSendNotification);
                    spectrumLnf.currentHue = newCol.getHue();
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

            // If user moved hue while color was pitch black (sat=0 & b=0), auto-revive brightness
            if (b < 0.05f && s < 0.05f)
            {
                s = 0.85f;
                b = 0.60f;
                satSlider.setValue(s, juce::dontSendNotification);
                briSlider.setValue(b, juce::dontSendNotification);
            }

            juce::Colour newColor = juce::Colour::fromHSV(h, s, b, 1.0f);
            setCurrentTargetColor(newColor);
            spectrumLnf.currentHue = h;

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
