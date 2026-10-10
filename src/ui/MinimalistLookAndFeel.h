#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include "ThemePalette.h"

namespace LadderMono
{
    namespace Noir3DPalette
    {
        inline const juce::Colour PITCH_BLACK  (0xff08090a);
        inline const juce::Colour CHASSIS_BG   (0xff101114);
        inline const juce::Colour PANEL_BG     (0xff141518);
        inline const juce::Colour PANEL_LIGHT  (0xff1a1b20);
        inline const juce::Colour PANEL_DARK   (0xff0e0f12);
        inline const juce::Colour WELL_DARK    (0xff090a0d);
        inline const juce::Colour WELL_BORDER  (0xff1b1d22);

        inline const juce::Colour KNOB_BODY    (0xff1f2127);
        inline const juce::Colour KNOB_CAP     (0xff141519);
        inline const juce::Colour KNOB_RING    (0xff2c2e36);
        inline const juce::Colour KNOB_DOT     (0xffd8dce6);

        inline const juce::Colour SWITCH_BG    (0xff0a0b0e);
        inline const juce::Colour SWITCH_BODY  (0xff1d1f25);
        inline const juce::Colour SWITCH_ON_DOT (0xffeaedf4);
        inline const juce::Colour SWITCH_OFF_DOT (0xff0e0f12);

        inline const juce::Colour BEVEL_SPEC   (0xffd4d8e2);
        inline const juce::Colour BEVEL_LIGHT  (0xff606470);
        inline const juce::Colour BEVEL_MID    (0xff2a2c34);
        inline const juce::Colour BEVEL_DARK   (0xff18191e);
        inline const juce::Colour BEVEL_SHADOW (0xff08090b);

        inline const juce::Colour TEXT_TITLE   (0xffd4d8e2);
        inline const juce::Colour TEXT_BODY    (0xff969ca8);
        inline const juce::Colour TEXT_ACCENT  (0xff727782);
        inline const juce::Colour TEXT_DIM     (0xff444750);
    }

    namespace CyberPalette
    {
        inline const juce::Colour BG_DARK (0xff090c10);
        inline const juce::Colour BG_CARD (0xff0f151e);
        inline const juce::Colour BORDER  (0xff00d0bc);
        inline const juce::Colour S500    (0xff00ffe1);
        inline const juce::Colour S600    (0xff00d0bc);
        inline const juce::Colour S700    (0xff00a89a);
        inline const juce::Colour P50     (0xfff1f1f1);
        inline const juce::Colour P100    (0xffeaeaea);
        inline const juce::Colour P200    (0xffdddddd);
        inline const juce::Colour P700    (0xff606060);
        inline const juce::Colour P950    (0xff303030);
    }

    enum class SkinMode
    {
        ClassicVintage = 0,
        CyberSchematic = 1,
        Noir3D         = 2
    };

    class MinimalistLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        using SkinMode = LadderMono::SkinMode;

        MinimalistLookAndFeel()
        {
            setTheme(ThemeColors::getDefaultNoir());
        }

        void setTheme(const ThemeColors& t)
        {
            theme = t;
            setColour(juce::ResizableWindow::backgroundColourId, theme.bgChassis);
            setColour(juce::Slider::thumbColourId, theme.knobIndicator);
            setColour(juce::Slider::rotarySliderFillColourId, theme.knobIndicator);
            setColour(juce::Slider::rotarySliderOutlineColourId, theme.wellDark);
            setColour(juce::Label::textColourId, theme.textSecondary);
            setColour(juce::ComboBox::backgroundColourId, theme.panelLight);
            setColour(juce::ComboBox::textColourId, theme.textPrimary);
            setColour(juce::ComboBox::outlineColourId, theme.lineBorder);
            setColour(juce::TextButton::buttonColourId, theme.panelLight);
            setColour(juce::TextButton::buttonOnColourId, theme.lineAccent);
            setColour(juce::TextButton::textColourOnId, theme.textPrimary);
            setColour(juce::TextButton::textColourOffId, theme.textSecondary);
            setColour(juce::PopupMenu::backgroundColourId, theme.panelLight);
            setColour(juce::PopupMenu::textColourId, theme.textPrimary);
            setColour(juce::PopupMenu::highlightedBackgroundColourId, theme.lineAccent);
            setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xffffffff));
        }

        const ThemeColors& getTheme() const noexcept { return theme; }

        void setSkinMode(SkinMode mode)
        {
            currentSkin = mode;
            if (mode == SkinMode::CyberSchematic)
                setTheme(ThemeColors::getCyberCyan());
            else if (mode == SkinMode::ClassicVintage)
                setTheme(ThemeColors::getAmberMatrix());
            else
                setTheme(ThemeColors::getDefaultNoir());
        }

        SkinMode getSkinMode() const noexcept { return currentSkin; }

        // ====================================================================
        // Rotary Slider Drawing (Pure Normalized Flat Matte OP-XY Dial)
        // ====================================================================
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& /*slider*/) override
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
            auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto centre = bounds.getCentre();
            auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            // 1. Recessed outer chassis well
            g.setColour(theme.wellDark);
            g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
            g.setColour(theme.lineBorder);
            g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

            // 2. Solid flat matte cylinder rotor
            float knobRadius = radius * 0.84f;
            g.setColour(theme.knobBody);
            g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
            g.setColour(theme.knobRing);
            g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

            // 3. Subtle center cap
            float capRadius = knobRadius * 0.44f;
            g.setColour(theme.knobCap);
            g.fillEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);
            g.setColour(theme.lineBorder);
            g.drawEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 0.8f);

            // 4. Indicator dot
            float dotDistance = knobRadius * 0.72f;
            float dotAngle = toAngle - juce::MathConstants<float>::halfPi;
            float dotX = centre.x + dotDistance * std::cos(dotAngle);
            float dotY = centre.y + dotDistance * std::sin(dotAngle);
            float dotR = juce::jmax(2.2f, knobRadius * 0.12f);

            // Recessed socket
            g.setColour(theme.bgChassis);
            g.fillEllipse(dotX - dotR - 0.5f, dotY - dotR - 0.5f, (dotR + 0.5f) * 2.0f, (dotR + 0.5f) * 2.0f);

            // Crisp indicator dot
            g.setColour(theme.knobIndicator);
            g.fillEllipse(dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
        }

        // ====================================================================
        // Toggle Button (Pure Normalized Flat Matte OP-XY Switch)
        // ====================================================================
        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
        {
            auto bounds = button.getLocalBounds().toFloat();
            auto name = button.getButtonText();
            bool isOn = button.getToggleState();

            float switchWidth = 24.0f;
            float switchHeight = 32.0f;
            auto switchBox = bounds.removeFromLeft(switchWidth + 4.0f).withSizeKeepingCentre(switchWidth, switchHeight);

            // 1. Recessed switch slot
            g.setColour(theme.wellDark);
            g.fillRoundedRectangle(switchBox, 3.0f);
            g.setColour(theme.lineBorder);
            g.drawRoundedRectangle(switchBox, 3.0f, 1.0f);

            // 2. Flat matte switch body
            auto paddle = switchBox.reduced(2.0f);
            g.setColour(theme.switchBody);
            g.fillRoundedRectangle(paddle, 2.0f);

            // 3. Indicator dot
            float dotR = 2.8f;
            float dotX = paddle.getCentreX();
            float dotY = paddle.getCentreY();

            if (isOn)
            {
                g.setColour(theme.switchActive);
                g.fillEllipse(dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
            }
            else
            {
                g.setColour(theme.switchInactive);
                g.fillEllipse(dotX - 2.0f, dotY - 2.0f, 4.0f, 4.0f);
                g.setColour(theme.lineBorder);
                g.drawEllipse(dotX - 2.0f, dotY - 2.0f, 4.0f, 4.0f, 0.8f);
            }

            // Label Beside Switch
            bounds.removeFromLeft(4.0f);
            g.setColour(isOn ? theme.textPrimary : theme.textDim);
            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.drawFittedText(name, bounds.toNearestInt(), juce::Justification::centredLeft, 1);
        }

        // ====================================================================
        // Linear Wheels (Pitch & Mod)
        // ====================================================================
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/,
                              juce::Slider::SliderStyle /*style*/, juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);

            // Recessed well
            g.setColour(theme.wellDark);
            g.fillRoundedRectangle(bounds, 3.0f);
            g.setColour(theme.lineBorder);
            g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

            auto wheelBounds = bounds.reduced(3.0f, 3.0f);
            g.setColour(theme.knobBody);
            g.fillRoundedRectangle(wheelBounds, 2.0f);

            const int numRibs = 14;
            float stepY = wheelBounds.getHeight() / static_cast<float>(numRibs);
            for (int i = 0; i <= numRibs; ++i)
            {
                float ry = wheelBounds.getY() + i * stepY;
                g.setColour(theme.bgChassis);
                g.drawHorizontalLine(static_cast<int>(ry), wheelBounds.getX(), wheelBounds.getRight());
                g.setColour(theme.lineBorder);
                g.drawHorizontalLine(static_cast<int>(ry + 1.0f), wheelBounds.getX() + 1.0f, wheelBounds.getRight() - 1.0f);
            }

            double rangeLen = slider.getMaximum() - slider.getMinimum();
            float posRatio = (rangeLen > 0.00001) ? static_cast<float>((slider.getValue() - slider.getMinimum()) / rangeLen) : 0.5f;
            float indY = wheelBounds.getBottom() - posRatio * wheelBounds.getHeight();

            g.setColour(theme.knobIndicator);
            g.drawHorizontalLine(static_cast<int>(indY), wheelBounds.getX() + 2.0f, wheelBounds.getRight() - 2.0f);
            g.fillEllipse(wheelBounds.getCentreX() - 2.5f, indY - 2.5f, 5.0f, 5.0f);
        }

        // ====================================================================
        // ComboBox Drawing
        // ====================================================================
        void drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                          int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                          juce::ComboBox& box) override
        {
            auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);

            g.setColour(theme.panelLight);
            g.fillRoundedRectangle(bounds, 3.0f);

            g.setColour(theme.lineBorder);
            g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

            float arrowX = width - 16.0f;
            float arrowY = height * 0.5f - 2.0f;
            juce::Path arrow;
            arrow.startNewSubPath(arrowX, arrowY);
            arrow.lineTo(arrowX + 4.0f, arrowY + 4.0f);
            arrow.lineTo(arrowX + 8.0f, arrowY);
            g.setColour(box.isEnabled() ? theme.textPrimary : theme.textDim);
            g.strokePath(arrow, juce::PathStrokeType(1.4f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        }

    private:
        ThemeColors theme;
        SkinMode currentSkin = SkinMode::Noir3D;
    };
}
