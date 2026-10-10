#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ThemePalette.h"

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
            targetBox.addItem("Knob Indicators", 3);
            targetBox.addItem("Active Switches (LED)", 4);
            targetBox.addItem("Primary Text", 5);
            targetBox.addItem("Secondary Text", 6);
            targetBox.addItem("Panel Faceplate", 7);
            targetBox.addItem("Cutout Wells", 8);
            targetBox.addItem("Knob Cylinders", 9);
            targetBox.addItem("Chassis Background", 10);
            targetBox.addItem("OLED Screen Glass", 11);
            targetBox.setSelectedId(1, juce::dontSendNotification);
            targetBox.onChange = [this] { updateSlidersFromCurrentTarget(); };
            addAndMakeVisible(targetBox);

            // Sliders for H, S, B
            auto setupSlider = [this](juce::Slider& s, juce::Label& lbl, const juce::String& name, double min, double max, double init) {
                s.setSliderStyle(juce::Slider::LinearHorizontal);
                s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 46, 18);
                s.setRange(min, max, 0.01);
                s.setValue(init, juce::dontSendNotification);
                s.onValueChange = [this] { onSliderChanged(); };
                addAndMakeVisible(s);

                lbl.setText(name, juce::dontSendNotification);
                lbl.setFont(juce::FontOptions(11.0f, juce::Font::bold));
                lbl.setColour(juce::Label::textColourId, juce::Colour(0xff989ea8));
                addAndMakeVisible(lbl);
            };

            setupSlider(hueSlider, hueLabel, "HUE", 0.0, 1.0, 0.0);
            setupSlider(satSlider, satLabel, "SAT", 0.0, 1.0, 0.0);
            setupSlider(briSlider, briLabel, "LIGHT", 0.0, 1.0, 0.5);

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
            g.setColour(juce::Colour(0x99000000));
            g.fillRoundedRectangle(bounds, 6.0f);

            g.setColour(activeTheme.panelBg);
            g.fillRoundedRectangle(bounds.reduced(1.0f), 5.0f);

            g.setColour(activeTheme.lineBorder);
            g.drawRoundedRectangle(bounds.reduced(1.0f), 5.0f, 1.2f);

            // Title Banner
            g.setColour(activeTheme.panelLight);
            g.fillRect(1.0f, 1.0f, bounds.getWidth() - 2.0f, 34.0f);
            g.setColour(activeTheme.lineBorder);
            g.drawHorizontalLine(35, 1.0f, bounds.getWidth() - 1.0f);

            g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
            g.setColour(activeTheme.textPrimary);
            g.drawText("COLOR THEME STUDIO", 14, 0, static_cast<int>(bounds.getWidth() - 50.0f), 35, juce::Justification::centredLeft);

            // Section 1: Presets
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.setColour(activeTheme.textSecondary);
            g.drawText("FACTORY PALETTES", 14, 42, static_cast<int>(bounds.getWidth() - 28.0f), 18, juce::Justification::centredLeft);

            // Section 2: Custom Adjuster
            g.drawText("CUSTOMIZE COMPONENT", 14, 150, static_cast<int>(bounds.getWidth() - 28.0f), 18, juce::Justification::centredLeft);

            // Active Color Swatch Preview
            auto swatch = juce::Rectangle<float>(bounds.getWidth() - 48.0f, 178.0f, 34.0f, 24.0f);
            g.setColour(getCurrentTargetColor());
            g.fillRoundedRectangle(swatch, 3.0f);
            g.setColour(activeTheme.lineBorder);
            g.drawRoundedRectangle(swatch, 3.0f, 1.0f);
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            closeBtn.setBounds(bounds.getWidth() - 32, 5, 24, 24);

            int btnW = (bounds.getWidth() - 36) / 2;
            noirPresetBtn.setBounds(14, 64, btnW, 24);
            cyanPresetBtn.setBounds(22 + btnW, 64, btnW, 24);

            amberPresetBtn.setBounds(14, 92, btnW, 24);
            bluePresetBtn.setBounds(22 + btnW, 92, btnW, 24);

            crimsonPresetBtn.setBounds(14, 120, bounds.getWidth() - 28, 24);

            // Target selector dropdown
            targetBox.setBounds(14, 178, bounds.getWidth() - 70, 24);

            // HSL Sliders
            int sy = 216;
            hueLabel.setBounds(14, sy, 50, 20);
            hueSlider.setBounds(68, sy, bounds.getWidth() - 82, 20);

            sy += 28;
            satLabel.setBounds(14, sy, 50, 20);
            satSlider.setBounds(68, sy, bounds.getWidth() - 82, 20);

            sy += 28;
            briLabel.setBounds(14, sy, 50, 20);
            briSlider.setBounds(68, sy, bounds.getWidth() - 82, 20);

            resetBtn.setBounds(14, bounds.getHeight() - 38, bounds.getWidth() - 28, 26);
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
        juce::Label hueLabel, satLabel, briLabel;
        juce::Slider hueSlider, satSlider, briSlider;
        juce::TextButton resetBtn;

        juce::Colour getCurrentTargetColor() const
        {
            int id = targetBox.getSelectedId();
            switch (id)
            {
                case 1: return activeTheme.lineBorder;
                case 2: return activeTheme.lineAccent;
                case 3: return activeTheme.knobIndicator;
                case 4: return activeTheme.switchActive;
                case 5: return activeTheme.textPrimary;
                case 6: return activeTheme.textSecondary;
                case 7: return activeTheme.panelBg;
                case 8: return activeTheme.wellDark;
                case 9: return activeTheme.knobBody;
                case 10: return activeTheme.bgChassis;
                case 11: return activeTheme.screenBg;
                default: return activeTheme.lineBorder;
            }
        }

        void setCurrentTargetColor(juce::Colour c)
        {
            int id = targetBox.getSelectedId();
            switch (id)
            {
                case 1: activeTheme.lineBorder = c; break;
                case 2: activeTheme.lineAccent = c; break;
                case 3: activeTheme.knobIndicator = c; break;
                case 4: activeTheme.switchActive = c; break;
                case 5: activeTheme.textPrimary = c; break;
                case 6: activeTheme.textSecondary = c; break;
                case 7: activeTheme.panelBg = c; break;
                case 8: activeTheme.wellDark = c; break;
                case 9: activeTheme.knobBody = c; break;
                case 10: activeTheme.bgChassis = c; break;
                case 11: activeTheme.screenBg = c; break;
            }
        }

        void updateSlidersFromCurrentTarget()
        {
            isUpdatingSliders = true;
            juce::Colour c = getCurrentTargetColor();
            hueSlider.setValue(c.getHue(), juce::dontSendNotification);
            satSlider.setValue(c.getSaturation(), juce::dontSendNotification);
            briSlider.setValue(c.getBrightness(), juce::dontSendNotification);
            isUpdatingSliders = false;
            repaint();
        }

        void onSliderChanged()
        {
            if (isUpdatingSliders) return;
            float h = static_cast<float>(hueSlider.getValue());
            float s = static_cast<float>(satSlider.getValue());
            float b = static_cast<float>(briSlider.getValue());
            juce::Colour newColor = juce::Colour::fromHSV(h, s, b, 1.0f);
            setCurrentTargetColor(newColor);
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
