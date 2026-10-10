#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ThemePalette.h"
#include <iomanip>
#include <sstream>

namespace LadderMono
{
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
            hexLabel.setText("HEX", juce::dontSendNotification);
            hexLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
            hexLabel.setColour(juce::Label::textColourId, juce::Colour(0xff989ea8));
            addAndMakeVisible(hexLabel);

            hexInput.setMultiLine(false);
            hexInput.setReturnKeyStartsNewLine(false);
            hexInput.setFont(juce::FontOptions(12.0f, juce::Font::bold));
            hexInput.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff14161c));
            hexInput.setColour(juce::TextEditor::textColourId, juce::Colour(0xffe2e6f0));
            hexInput.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff2f333e));
            hexInput.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff4a5264));
            hexInput.setInputRestrictions(9, "#0123456789abcdefABCDEF");
            hexInput.onTextChange = [this] { onHexInputChanged(); };
            addAndMakeVisible(hexInput);

            // Sliders for H, S, B
            auto setupSlider = [this](juce::Slider& s, juce::Label& lbl, const juce::String& name, double min, double max, double init) {
                s.setSliderStyle(juce::Slider::LinearHorizontal);
                s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 16);
                s.setRange(min, max, 0.01);
                s.setValue(init, juce::dontSendNotification);
                s.onValueChange = [this] { onSliderChanged(); };
                addAndMakeVisible(s);

                lbl.setText(name, juce::dontSendNotification);
                lbl.setFont(juce::FontOptions(10.0f, juce::Font::bold));
                lbl.setColour(juce::Label::textColourId, juce::Colour(0xff989ea8));
                addAndMakeVisible(lbl);
            };

            setupSlider(hueSlider, hueLabel, "HUE", 0.0, 1.0, 0.0);
            setupSlider(satSlider, satLabel, "SAT", 0.0, 1.0, 0.0);
            setupSlider(briSlider, briLabel, "LIGHT", 0.0, 1.0, 0.5);

            // Geometry & Font controls
            fontScaleLabel.setText("FONT", juce::dontSendNotification);
            fontScaleLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
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

            cornerRadiusLabel.setText("ROUND", juce::dontSendNotification);
            cornerRadiusLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
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

            // Floating drawer card with deep shadow & fine border
            g.setColour(juce::Colour(0xbb000000));
            g.fillRoundedRectangle(bounds, activeTheme.cornerRadius + 2.0f);

            g.setColour(activeTheme.panelBg);
            g.fillRoundedRectangle(bounds.reduced(1.0f), activeTheme.cornerRadius);

            g.setColour(activeTheme.lineBorder);
            g.drawRoundedRectangle(bounds.reduced(1.0f), activeTheme.cornerRadius, 1.2f);

            // Title Banner
            g.setColour(activeTheme.panelLight);
            g.fillRect(1.0f, 1.0f, bounds.getWidth() - 2.0f, 32.0f);
            g.setColour(activeTheme.lineBorder);
            g.drawHorizontalLine(33, 1.0f, bounds.getWidth() - 1.0f);

            g.setFont(juce::FontOptions(11.5f * activeTheme.fontScale, juce::Font::bold));
            g.setColour(activeTheme.textPrimary);
            g.drawText("THEME & COLOR STUDIO", 14, 0, static_cast<int>(bounds.getWidth() - 48.0f), 32, juce::Justification::centredLeft);

            // Section 1: Presets
            g.setFont(juce::FontOptions(9.5f * activeTheme.fontScale, juce::Font::bold));
            g.setColour(activeTheme.textSecondary);
            g.drawText("FACTORY PALETTES", 14, 38, static_cast<int>(bounds.getWidth() - 28.0f), 16, juce::Justification::centredLeft);

            // Section 2: Custom Adjuster
            g.drawText("CUSTOMIZE COMPONENT", 14, 142, static_cast<int>(bounds.getWidth() - 28.0f), 16, juce::Justification::centredLeft);

            // Active Color Swatch Preview
            auto swatch = juce::Rectangle<float>(bounds.getWidth() - 44.0f, 162.0f, 30.0f, 22.0f);
            g.setColour(getCurrentTargetColor());
            g.fillRoundedRectangle(swatch, activeTheme.cornerRadius * 0.7f);
            g.setColour(activeTheme.lineBorder);
            g.drawRoundedRectangle(swatch, activeTheme.cornerRadius * 0.7f, 1.0f);

            // Section 3: Global Styling
            g.drawText("TYPOGRAPHY & ROUNDING", 14, 314, static_cast<int>(bounds.getWidth() - 28.0f), 16, juce::Justification::centredLeft);
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            closeBtn.setBounds(bounds.getWidth() - 28, 4, 24, 24);

            int btnW = (bounds.getWidth() - 34) / 2;
            noirPresetBtn.setBounds(14, 56, btnW, 22);
            cyanPresetBtn.setBounds(20 + btnW, 56, btnW, 22);

            amberPresetBtn.setBounds(14, 82, btnW, 22);
            bluePresetBtn.setBounds(20 + btnW, 82, btnW, 22);

            crimsonPresetBtn.setBounds(14, 108, bounds.getWidth() - 28, 22);

            // Target selector dropdown
            targetBox.setBounds(14, 162, bounds.getWidth() - 64, 22);

            // Hex input row
            hexLabel.setBounds(14, 192, 44, 20);
            hexInput.setBounds(62, 192, bounds.getWidth() - 76, 20);

            // HSL Sliders
            int sy = 218;
            hueLabel.setBounds(14, sy, 44, 18);
            hueSlider.setBounds(62, sy, bounds.getWidth() - 76, 18);

            sy += 24;
            satLabel.setBounds(14, sy, 44, 18);
            satSlider.setBounds(62, sy, bounds.getWidth() - 76, 18);

            sy += 24;
            briLabel.setBounds(14, sy, 44, 18);
            briSlider.setBounds(62, sy, bounds.getWidth() - 76, 18);

            // Font & Corner radius
            sy = 334;
            fontScaleLabel.setBounds(14, sy, 44, 18);
            fontScaleSlider.setBounds(62, sy, bounds.getWidth() - 76, 18);

            sy += 24;
            cornerRadiusLabel.setBounds(14, sy, 44, 18);
            cornerRadiusSlider.setBounds(62, sy, bounds.getWidth() - 76, 18);

            resetBtn.setBounds(14, bounds.getHeight() - 32, bounds.getWidth() - 28, 24);
        }

    private:
        ThemeColors activeTheme;
        bool isUpdatingSliders = false;

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
                    isUpdatingSliders = false;

                    if (onThemeChanged) onThemeChanged(activeTheme);
                    repaint();
                }
                catch (...) {}
            }
        }

        void onSliderChanged()
        {
            if (isUpdatingSliders) return;
            float h = static_cast<float>(hueSlider.getValue());
            float s = static_cast<float>(satSlider.getValue());
            float b = static_cast<float>(briSlider.getValue());
            juce::Colour newColor = juce::Colour::fromHSV(h, s, b, 1.0f);
            setCurrentTargetColor(newColor);

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
