#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>

namespace LadderMono
{
    // ====================================================================
    // Palette 1: Cyber Schematic (Neon & Signal Flow)
    // ====================================================================
    namespace CyberPalette
    {
        // Primary (Greys)
        inline const juce::Colour P50  (0xfff1f1f1);
        inline const juce::Colour P100 (0xffeaeaea);
        inline const juce::Colour P200 (0xffdddddd);
        inline const juce::Colour P300 (0xffc4c4c4);
        inline const juce::Colour P400 (0xffa7a7a7);
        inline const juce::Colour P500 (0xff888888);
        inline const juce::Colour P600 (0xff727272);
        inline const juce::Colour P700 (0xff606060);
        inline const juce::Colour P800 (0xff4f4f4f);
        inline const juce::Colour P900 (0xff3b3b3b);
        inline const juce::Colour P950 (0xff303030);

        // Secondary (Mint Green)
        inline const juce::Colour S50  (0xffeaf7f2);
        inline const juce::Colour S100 (0xffe1f3ec);
        inline const juce::Colour S200 (0xffcdede0);
        inline const juce::Colour S300 (0xffa8e0ca);
        inline const juce::Colour S400 (0xff7cd3b0);
        inline const juce::Colour S500 (0xff4bc695);
        inline const juce::Colour S600 (0xff39ac7e);
        inline const juce::Colour S700 (0xff31906a);
        inline const juce::Colour S800 (0xff297456);
        inline const juce::Colour S900 (0xff1f5640);
        inline const juce::Colour S950 (0xff1a4634);

        // Accent (Electric / Cobalt Blue)
        inline const juce::Colour A50  (0xffe7ecfa);
        inline const juce::Colour A100 (0xffdce2f8);
        inline const juce::Colour A200 (0xffc5d0f4);
        inline const juce::Colour A300 (0xff9aaeee);
        inline const juce::Colour A400 (0xff6685e9);
        inline const juce::Colour A500 (0xff2c57e5);
        inline const juce::Colour A600 (0xff1c44c9);
        inline const juce::Colour A700 (0xff193ba8);
        inline const juce::Colour A800 (0xff163187);
        inline const juce::Colour A900 (0xff122564);
        inline const juce::Colour A950 (0xff0f1e50);

        // Accent-1 (Coral / Magenta Red)
        inline const juce::Colour R50  (0xfff9e8ec);
        inline const juce::Colour R100 (0xfff6dee4);
        inline const juce::Colour R200 (0xfff1c8d2);
        inline const juce::Colour R300 (0xffe8a0b1);
        inline const juce::Colour R400 (0xffe06f8a);
        inline const juce::Colour R500 (0xffd8395e);
        inline const juce::Colour R600 (0xffbc284b);
        inline const juce::Colour R700 (0xff9e2340);
        inline const juce::Colour R800 (0xff7f1e35);
        inline const juce::Colour R900 (0xff5e1828);
        inline const juce::Colour R950 (0xff4c1421);

        // Dark Backgrounds
        inline const juce::Colour BG_DARK   (0xff0d0e12);
        inline const juce::Colour BG_PANEL  (0xff14151b);
        inline const juce::Colour BG_CARD   (0xff1a1c24);
        inline const juce::Colour BORDER    (0xff333644);
        inline const juce::Colour BORDER_HI (0xff585d72);
    }

    // ====================================================================
    // Palette 2: Noir 3D (Pure Monochromatic Matte Titanium & Realistic Lighting)
    // ====================================================================
    namespace Noir3DPalette
    {
        inline const juce::Colour PITCH_BLACK  (0xff050608);
        inline const juce::Colour CHASSIS_BG   (0xff0d0e11);
        inline const juce::Colour PANEL_BG     (0xff141519);
        inline const juce::Colour PANEL_LIGHT  (0xff1b1d23);
        inline const juce::Colour PANEL_DARK   (0xff0e0f13);
        inline const juce::Colour WELL_DARK    (0xff08090b);

        inline const juce::Colour BEVEL_SPEC   (0xfff2f5fa);
        inline const juce::Colour BEVEL_LIGHT  (0xff828998);
        inline const juce::Colour BEVEL_MID    (0xff3e424c);
        inline const juce::Colour BEVEL_DARK   (0xff1f2127);
        inline const juce::Colour BEVEL_SHADOW (0xff060709);

        inline const juce::Colour TEXT_TITLE   (0xfff0f3f8);
        inline const juce::Colour TEXT_BODY    (0xffb0b5c0);
        inline const juce::Colour TEXT_ACCENT  (0xff828998);
        inline const juce::Colour TEXT_DIM     (0xff555964);
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
            setSkinMode(SkinMode::ClassicVintage);
        }

        void setSkinMode(SkinMode mode)
        {
            currentSkin = mode;

            if (currentSkin == SkinMode::Noir3D)
            {
                // Pure Monochrome 3D Matte Titanium
                setColour(juce::ResizableWindow::backgroundColourId, Noir3DPalette::PITCH_BLACK);
                setColour(juce::Slider::thumbColourId, Noir3DPalette::BEVEL_SPEC);
                setColour(juce::Slider::rotarySliderFillColourId, Noir3DPalette::BEVEL_SPEC);
                setColour(juce::Slider::rotarySliderOutlineColourId, Noir3DPalette::WELL_DARK);
                setColour(juce::Label::textColourId, Noir3DPalette::TEXT_BODY);
                setColour(juce::ComboBox::backgroundColourId, Noir3DPalette::PANEL_BG);
                setColour(juce::ComboBox::textColourId, Noir3DPalette::TEXT_TITLE);
                setColour(juce::ComboBox::outlineColourId, Noir3DPalette::BEVEL_MID);
                setColour(juce::TextButton::buttonColourId, Noir3DPalette::PANEL_LIGHT);
                setColour(juce::TextButton::buttonOnColourId, Noir3DPalette::BEVEL_MID);
                setColour(juce::TextButton::textColourOnId, Noir3DPalette::BEVEL_SPEC);
                setColour(juce::TextButton::textColourOffId, Noir3DPalette::TEXT_BODY);
                setColour(juce::PopupMenu::backgroundColourId, Noir3DPalette::PANEL_BG);
                setColour(juce::PopupMenu::textColourId, Noir3DPalette::TEXT_TITLE);
                setColour(juce::PopupMenu::highlightedBackgroundColourId, Noir3DPalette::BEVEL_MID);
                setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xffffffff));
            }
            else if (currentSkin == SkinMode::CyberSchematic)
            {
                // Cyber Schematic Palette
                setColour(juce::ResizableWindow::backgroundColourId, CyberPalette::BG_DARK);
                setColour(juce::Slider::thumbColourId, CyberPalette::S500);
                setColour(juce::Slider::rotarySliderFillColourId, CyberPalette::S500);
                setColour(juce::Slider::rotarySliderOutlineColourId, CyberPalette::P950);
                setColour(juce::Label::textColourId, CyberPalette::P100);
                setColour(juce::ComboBox::backgroundColourId, CyberPalette::BG_CARD);
                setColour(juce::ComboBox::textColourId, CyberPalette::P50);
                setColour(juce::ComboBox::outlineColourId, CyberPalette::BORDER);
                setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1c1e26));
                setColour(juce::TextButton::buttonOnColourId, CyberPalette::S600);
                setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffffff));
                setColour(juce::TextButton::textColourOffId, CyberPalette::P200);
                setColour(juce::PopupMenu::backgroundColourId, CyberPalette::BG_CARD);
                setColour(juce::PopupMenu::textColourId, CyberPalette::P100);
                setColour(juce::PopupMenu::highlightedBackgroundColourId, CyberPalette::S700);
                setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xffffffff));
            }
            else
            {
                // Dark vintage analog palette (Original Classic)
                setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff18191c));
                setColour(juce::Slider::thumbColourId, juce::Colour(0xffd4d7dc));
                setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffe08b3c));
                setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff222428));
                setColour(juce::Label::textColourId, juce::Colour(0xffdcdfe4));
                setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1c1d21));
                setColour(juce::ComboBox::textColourId, juce::Colour(0xffe0e3e8));
                setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff454850));
                setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22242a));
                setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffd47a2a));
                setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffffff));
                setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd0d4dc));
                setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1e2025));
                setColour(juce::PopupMenu::textColourId, juce::Colour(0xffe2e5eb));
                setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xffc87024));
                setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xffffffff));
            }
        }

        SkinMode getSkinMode() const noexcept { return currentSkin; }

        // ====================================================================
        // Rotary Slider Drawing (Classic Vintage vs Cyber Schematic vs Noir 3D)
        // ====================================================================
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
            auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto centre = bounds.getCentre();
            auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            // ================================================================
            // NOIR 3D: Chiseled Monolithic Titanium Knob with Directional Light
            // ================================================================
            if (currentSkin == SkinMode::Noir3D)
            {
                // 1. Soft directional cast shadow (light from top-left)
                g.setColour(juce::Colour(0x66000000));
                g.fillEllipse(centre.x - radius + 2.0f, centre.y - radius + 3.0f, radius * 2.0f, radius * 2.0f);

                // 2. Milled outer chassis well
                g.setColour(Noir3DPalette::WELL_DARK);
                g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
                // Top-left shadow inside well, bottom-right specular rim
                g.setColour(juce::Colour(0x40000000));
                g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.2f);

                // 3. Machined Titanium Outer Cylinder Bevel (Light angle ~ 135 deg)
                float knobRadius = radius * 0.90f;
                juce::ColourGradient bevelGrad(Noir3DPalette::BEVEL_MID, centre.x - knobRadius, centre.y - knobRadius,
                                               Noir3DPalette::PITCH_BLACK, centre.x + knobRadius, centre.y + knobRadius, false);
                bevelGrad.addColour(0.2, Noir3DPalette::PANEL_LIGHT);
                bevelGrad.addColour(0.8, Noir3DPalette::BEVEL_DARK);
                g.setGradientFill(bevelGrad);
                g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

                // Directional top-left rim highlight
                g.setColour(Noir3DPalette::BEVEL_LIGHT);
                g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);
                g.setColour(Noir3DPalette::BEVEL_SPEC.withAlpha(0.7f));
                juce::Path rimArc;
                rimArc.addArc(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f,
                              juce::MathConstants<float>::pi * 1.0f, juce::MathConstants<float>::pi * 1.5f, true);
                g.strokePath(rimArc, juce::PathStrokeType(1.4f));

                // 4. Subtle Radial Engraved Gauge Ticks (Inspired by Image 3)
                const int numTicks = 11;
                for (int i = 0; i < numTicks; ++i)
                {
                    float t = static_cast<float>(i) / (numTicks - 1);
                    float a = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
                    float cosA = std::cos(a - juce::MathConstants<float>::halfPi);
                    float sinA = std::sin(a - juce::MathConstants<float>::halfPi);

                    float r1 = knobRadius * 0.78f;
                    float r2 = knobRadius * 0.90f;
                    g.setColour((i == 0 || i == 5 || i == 10) ? Noir3DPalette::TEXT_TITLE : Noir3DPalette::TEXT_DIM);
                    g.drawLine(centre.x + r1 * cosA, centre.y + r1 * sinA,
                               centre.x + r2 * cosA, centre.y + r2 * sinA, 1.0f);
                }

                // 5. Center Rotor Disk (Machined matte face with directional lighting)
                float rotorRadius = knobRadius * 0.65f;
                juce::ColourGradient rotorGrad(juce::Colour(0xff2d3038), centre.x - rotorRadius * 0.6f, centre.y - rotorRadius * 0.6f,
                                               juce::Colour(0xff121316), centre.x + rotorRadius * 0.6f, centre.y + rotorRadius * 0.6f, false);
                g.setGradientFill(rotorGrad);
                g.fillEllipse(centre.x - rotorRadius, centre.y - rotorRadius, rotorRadius * 2.0f, rotorRadius * 2.0f);
                g.setColour(Noir3DPalette::BEVEL_MID);
                g.drawEllipse(centre.x - rotorRadius, centre.y - rotorRadius, rotorRadius * 2.0f, rotorRadius * 2.0f, 1.0f);

                // 6. Central Chrome Specular Dome (Inspired by Image 4)
                float domeRadius = rotorRadius * 0.32f;
                juce::ColourGradient domeGrad(juce::Colour(0xffffffff), centre.x - domeRadius * 0.4f, centre.y - domeRadius * 0.4f,
                                              juce::Colour(0xff3a3e48), centre.x + domeRadius * 0.6f, centre.y + domeRadius * 0.6f, false);
                g.setGradientFill(domeGrad);
                g.fillEllipse(centre.x - domeRadius, centre.y - domeRadius, domeRadius * 2.0f, domeRadius * 2.0f);

                // 7. Milled 3D Indicator Slot (Catches pure light)
                float cosPtr = std::cos(toAngle - juce::MathConstants<float>::halfPi);
                float sinPtr = std::sin(toAngle - juce::MathConstants<float>::halfPi);

                // Shadow edge of the slot
                g.setColour(Noir3DPalette::BEVEL_SHADOW);
                g.drawLine(centre.x + (rotorRadius * 0.35f) * cosPtr + 0.6f,
                           centre.y + (rotorRadius * 0.35f) * sinPtr + 0.6f,
                           centre.x + (knobRadius * 0.88f) * cosPtr + 0.6f,
                           centre.y + (knobRadius * 0.88f) * sinPtr + 0.6f, 2.2f);

                // Polished silver/white highlight ridge of the slot catching the light
                g.setColour(Noir3DPalette::BEVEL_SPEC);
                g.drawLine(centre.x + (rotorRadius * 0.35f) * cosPtr,
                           centre.y + (rotorRadius * 0.35f) * sinPtr,
                           centre.x + (knobRadius * 0.88f) * cosPtr,
                           centre.y + (knobRadius * 0.88f) * sinPtr, 2.0f);
                return;
            }

            // ================================================================
            // CYBER SCHEMATIC
            // ================================================================
            if (currentSkin == SkinMode::CyberSchematic)
            {
                auto sName = slider.getName();
                juce::Colour activeColour = CyberPalette::S500;
                if (sName.containsIgnoreCase("DRIVE") || sName.containsIgnoreCase("FEED") || sName.containsIgnoreCase("EXT"))
                    activeColour = CyberPalette::R500;
                else if (sName.containsIgnoreCase("TUNE") || sName.containsIgnoreCase("OSC") || sName.containsIgnoreCase("FINE") || sName.containsIgnoreCase("MOD"))
                    activeColour = CyberPalette::A500;
                else if (sName.containsIgnoreCase("CUTOFF") || sName.containsIgnoreCase("EMPHASIS") || sName.containsIgnoreCase("CONTOUR"))
                    activeColour = CyberPalette::S500;

                g.setColour(juce::Colour(0x55000000));
                g.fillEllipse(centre.x - radius, centre.y - radius + 1.5f, radius * 2.0f, radius * 2.0f);

                g.setColour(CyberPalette::BG_CARD);
                g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
                g.setColour(CyberPalette::P800);
                g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

                float arcRadius = radius * 0.82f;
                juce::Path bgArc;
                bgArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
                g.setColour(CyberPalette::P950);
                g.strokePath(bgArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                if (sliderPosProportional > 0.001f)
                {
                    juce::Path activeArc;
                    activeArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);

                    g.setColour(activeColour.withAlpha(0.25f));
                    g.strokePath(activeArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                    g.setColour(activeColour);
                    g.strokePath(activeArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }

                const int numTicks = 9;
                for (int i = 0; i < numTicks; ++i)
                {
                    float t = static_cast<float>(i) / (numTicks - 1);
                    float a = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
                    float cosA = std::cos(a - juce::MathConstants<float>::halfPi);
                    float sinA = std::sin(a - juce::MathConstants<float>::halfPi);

                    float r1 = radius * 0.88f;
                    float r2 = radius * 0.96f;
                    g.setColour(i == 0 || i == (numTicks - 1) || i == (numTicks / 2) ? CyberPalette::P300 : CyberPalette::P700);
                    g.drawLine(centre.x + r1 * cosA, centre.y + r1 * sinA,
                               centre.x + r2 * cosA, centre.y + r2 * sinA, (i % 2 == 0) ? 1.2f : 0.8f);
                }

                float capRadius = radius * 0.52f;
                juce::ColourGradient capGrad(CyberPalette::P900, centre.x - capRadius * 0.3f, centre.y - capRadius * 0.3f,
                                             CyberPalette::BG_DARK, centre.x + capRadius * 0.5f, centre.y + capRadius * 0.5f, false);
                g.setGradientFill(capGrad);
                g.fillEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);
                g.setColour(CyberPalette::BORDER_HI);
                g.drawEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);

                float cosPtr = std::cos(toAngle - juce::MathConstants<float>::halfPi);
                float sinPtr = std::sin(toAngle - juce::MathConstants<float>::halfPi);

                g.setColour(activeColour);
                g.drawLine(centre.x + capRadius * 0.2f * cosPtr, centre.y + capRadius * 0.2f * sinPtr,
                           centre.x + radius * 0.84f * cosPtr, centre.y + radius * 0.84f * sinPtr, 2.2f);

                g.setColour(CyberPalette::P100);
                g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
                return;
            }

            // ================================================================
            // CLASSIC VINTAGE (Original)
            // ================================================================
            g.setColour(juce::Colour(0x66000000));
            g.fillEllipse(centre.x - radius + 1.0f, centre.y - radius + 2.5f, radius * 2.0f, radius * 2.0f);

            juce::ColourGradient skirtGrad(juce::Colour(0xff26282e), centre.x, centre.y - radius,
                                           juce::Colour(0xff141518), centre.x, centre.y + radius, false);
            g.setGradientFill(skirtGrad);
            g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

            g.setColour(juce::Colour(0xff3c3f46));
            g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.2f);

            const int numTicks = 11;
            g.setColour(juce::Colour(0xffb0b5be));
            for (int i = 0; i < numTicks; ++i)
            {
                float t = static_cast<float>(i) / (numTicks - 1);
                float angle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
                float cosA = std::cos(angle - juce::MathConstants<float>::halfPi);
                float sinA = std::sin(angle - juce::MathConstants<float>::halfPi);

                float tickInner = radius * 0.76f;
                float tickOuter = radius * 0.90f;

                float x1 = centre.x + tickInner * cosA;
                float y1 = centre.y + tickInner * sinA;
                float x2 = centre.x + tickOuter * cosA;
                float y2 = centre.y + tickOuter * sinA;

                float thickness = (i == 0 || i == 5 || i == 10) ? 1.4f : 0.9f;
                g.drawLine(x1, y1, x2, y2, thickness);
            }

            float colletRadius = radius * 0.65f;
            juce::ColourGradient colletGrad(juce::Colour(0xff32353c), centre.x, centre.y - colletRadius,
                                            juce::Colour(0xff16171a), centre.x, centre.y + colletRadius, false);
            g.setGradientFill(colletGrad);
            g.fillEllipse(centre.x - colletRadius, centre.y - colletRadius, colletRadius * 2.0f, colletRadius * 2.0f);

            g.setColour(juce::Colour(0xff454952));
            g.drawEllipse(centre.x - colletRadius, centre.y - colletRadius, colletRadius * 2.0f, colletRadius * 2.0f, 1.0f);

            float capRadius = colletRadius * 0.72f;
            juce::ColourGradient capGrad(juce::Colour(0xffe8eaed), centre.x - capRadius * 0.4f, centre.y - capRadius * 0.4f,
                                         juce::Colour(0xff787c86), centre.x + capRadius * 0.6f, centre.y + capRadius * 0.6f, false);
            g.setGradientFill(capGrad);
            g.fillEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

            g.setColour(juce::Colour(0xff4e525a));
            g.drawEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);

            float cosPtr = std::cos(toAngle - juce::MathConstants<float>::halfPi);
            float sinPtr = std::sin(toAngle - juce::MathConstants<float>::halfPi);

            juce::Path ptr;
            ptr.addLineSegment(juce::Line<float>(centre.x + (capRadius * 0.15f) * cosPtr,
                                                 centre.y + (capRadius * 0.15f) * sinPtr,
                                                 centre.x + (radius * 0.88f) * cosPtr,
                                                 centre.y + (radius * 0.88f) * sinPtr), 2.2f);
            g.setColour(slider.isEnabled() ? juce::Colour(0xffffffff) : juce::Colour(0xff7a7d86));
            g.strokePath(ptr, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // ====================================================================
        // Toggle Button (Classic Vintage vs Cyber Schematic vs Noir 3D)
        // ====================================================================
        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
        {
            auto bounds = button.getLocalBounds().toFloat();
            auto name = button.getButtonText();
            bool isOn = button.getToggleState();

            // ================================================================
            // NOIR 3D: Tactile Rocker Switch (Pure Directional Light Reflection)
            // ================================================================
            if (currentSkin == SkinMode::Noir3D)
            {
                float switchWidth = 26.0f;
                float switchHeight = 36.0f;
                auto switchBox = bounds.removeFromLeft(switchWidth + 4.0f).withSizeKeepingCentre(switchWidth, switchHeight);

                // Recessed well
                g.setColour(Noir3DPalette::WELL_DARK);
                g.fillRoundedRectangle(switchBox, 3.0f);
                g.setColour(Noir3DPalette::BEVEL_MID);
                g.drawRoundedRectangle(switchBox, 3.0f, 1.0f);

                auto paddle = switchBox.reduced(2.5f, 2.5f);
                float midY = paddle.getCentreY();
                auto topHalf = paddle.withBottom(midY);
                auto btmHalf = paddle.withTop(midY);

                if (isOn)
                {
                    // PRESSED / ON: Physically tilted forward catching directional light!
                    // Upper facet receded into cavity shadow
                    g.setColour(Noir3DPalette::PITCH_BLACK);
                    g.fillRect(topHalf);

                    // Lower facet tilted forward: catches bright specular reflection
                    juce::ColourGradient onGrad(juce::Colour(0xff9298a8), paddle.getX(), midY,
                                                juce::Colour(0xff555966), paddle.getX(), paddle.getBottom(), false);
                    g.setGradientFill(onGrad);
                    g.fillRect(btmHalf);

                    // Bottom specular rim highlight
                    g.setColour(Noir3DPalette::BEVEL_SPEC);
                    g.drawHorizontalLine(static_cast<int>(paddle.getBottom() - 1.0f), paddle.getX(), paddle.getRight());

                    // Central polished silver jewel pip that catches the light
                    g.setColour(juce::Colour(0xffffffff));
                    g.fillEllipse(paddle.getCentreX() - 2.5f, paddle.getBottom() - 7.0f, 5.0f, 5.0f);
                }
                else
                {
                    // UNPRESSED / OFF: Upper facet tilted toward the light catching soft reflection
                    juce::ColourGradient offGrad(juce::Colour(0xff454a55), paddle.getX(), paddle.getY(),
                                                 juce::Colour(0xff22252c), paddle.getX(), midY, false);
                    g.setGradientFill(offGrad);
                    g.fillRect(topHalf);

                    // Lower facet in shadow
                    g.setColour(Noir3DPalette::WELL_DARK);
                    g.fillRect(btmHalf);

                    // Top specular rim highlight
                    g.setColour(Noir3DPalette::BEVEL_LIGHT);
                    g.drawHorizontalLine(static_cast<int>(paddle.getY()), paddle.getX(), paddle.getRight());
                }

                // Crease divider
                g.setColour(juce::Colour(0x88000000));
                g.drawHorizontalLine(static_cast<int>(midY), paddle.getX(), paddle.getRight());

                // Label Beside Switch
                bounds.removeFromLeft(4.0f);
                g.setColour(isOn ? Noir3DPalette::TEXT_TITLE : Noir3DPalette::TEXT_DIM);
                g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
                g.drawFittedText(name, bounds.toNearestInt(), juce::Justification::centredLeft, 1);
                return;
            }

            // ================================================================
            // CYBER SCHEMATIC
            // ================================================================
            if (currentSkin == SkinMode::CyberSchematic)
            {
                float switchWidth = 26.0f;
                float switchHeight = 36.0f;
                auto switchBox = bounds.removeFromLeft(switchWidth + 4.0f).withSizeKeepingCentre(switchWidth, switchHeight);

                juce::Colour glowColour = CyberPalette::S500;
                if (name.containsIgnoreCase("DECAY") || name.containsIgnoreCase("FEED"))
                    glowColour = CyberPalette::R500;
                else if (name.containsIgnoreCase("OSC") || name.containsIgnoreCase("FILT"))
                    glowColour = CyberPalette::A500;

                g.setColour(CyberPalette::BG_DARK);
                g.fillRoundedRectangle(switchBox, 3.0f);
                g.setColour(isOn ? glowColour : CyberPalette::BORDER);
                g.drawRoundedRectangle(switchBox, 3.0f, isOn ? 1.5f : 1.0f);

                auto paddle = switchBox.reduced(3.0f, 3.0f);
                if (isOn)
                {
                    juce::ColourGradient onGrad(glowColour.withAlpha(0.9f), paddle.getX(), paddle.getY(),
                                                glowColour.darker(0.5f), paddle.getX(), paddle.getBottom(), false);
                    g.setGradientFill(onGrad);
                    g.fillRoundedRectangle(paddle, 2.0f);

                    g.setColour(juce::Colour(0xffffffff));
                    g.fillEllipse(paddle.getCentreX() - 2.5f, paddle.getBottom() - 7.0f, 5.0f, 5.0f);
                }
                else
                {
                    g.setColour(CyberPalette::P950);
                    g.fillRoundedRectangle(paddle, 2.0f);
                    g.setColour(CyberPalette::P800);
                    g.drawRoundedRectangle(paddle, 2.0f, 0.8f);
                }

                bounds.removeFromLeft(4.0f);
                g.setColour(isOn ? CyberPalette::P50 : CyberPalette::P400);
                g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
                g.drawFittedText(name, bounds.toNearestInt(), juce::Justification::centredLeft, 1);
                return;
            }

            // ================================================================
            // CLASSIC VINTAGE (Original)
            // ================================================================
            const juce::Colour onBaseColour(0xffe66a10);
            const juce::Colour onBrightColour(0xffff9a36);
            const juce::Colour offBaseColour(0xff22242a);
            const juce::Colour offHighlightColour(0xff363942);

            float switchWidth = 26.0f;
            float switchHeight = 36.0f;
            auto switchBox = bounds.removeFromLeft(switchWidth + 4.0f).withSizeKeepingCentre(switchWidth, switchHeight);

            g.setColour(juce::Colour(0xff0c0d10));
            g.fillRoundedRectangle(switchBox, 2.5f);
            g.setColour(isOn ? onBaseColour.withAlpha(0.6f) : juce::Colour(0xff32353d));
            g.drawRoundedRectangle(switchBox, 2.5f, 1.0f);

            auto paddle = switchBox.reduced(2.5f, 2.5f);
            float midY = paddle.getCentreY();
            auto topHalf = paddle.withBottom(midY);
            auto btmHalf = paddle.withTop(midY);

            if (isOn)
            {
                juce::Colour darkFace = onBaseColour.darker(0.6f);
                g.setColour(darkFace);
                g.fillRect(topHalf);

                juce::ColourGradient btmGrad(onBrightColour, paddle.getX(), midY,
                                             onBaseColour, paddle.getX(), paddle.getBottom(), false);
                g.setGradientFill(btmGrad);
                g.fillRect(btmHalf);

                g.setColour(onBrightColour.brighter(0.4f));
                g.drawHorizontalLine(static_cast<int>(paddle.getBottom() - 1.0f), paddle.getX(), paddle.getRight());
            }
            else
            {
                juce::ColourGradient topGrad(offHighlightColour, paddle.getX(), paddle.getY(),
                                             offBaseColour, paddle.getX(), midY, false);
                g.setGradientFill(topGrad);
                g.fillRect(topHalf);

                g.setColour(offBaseColour.darker(0.4f));
                g.fillRect(btmHalf);

                g.setColour(offHighlightColour.brighter(0.25f));
                g.drawHorizontalLine(static_cast<int>(paddle.getY()), paddle.getX(), paddle.getRight());
            }

            g.setColour(juce::Colour(0x55000000));
            g.drawHorizontalLine(static_cast<int>(midY), paddle.getX(), paddle.getRight());

            bounds.removeFromLeft(4.0f);
            g.setColour(isOn ? juce::Colour(0xffffffff) : juce::Colour(0xff9aa0af));
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

            // ================================================================
            // NOIR 3D
            // ================================================================
            if (currentSkin == SkinMode::Noir3D)
            {
                g.setColour(Noir3DPalette::WELL_DARK);
                g.fillRoundedRectangle(bounds, 3.0f);
                g.setColour(Noir3DPalette::BEVEL_MID);
                g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

                auto wheelBounds = bounds.reduced(4.0f, 4.0f);
                juce::ColourGradient cylGrad(juce::Colour(0xff444955), wheelBounds.getX(), wheelBounds.getY(),
                                             juce::Colour(0xff121418), wheelBounds.getX(), wheelBounds.getBottom(), false);
                cylGrad.addColour(0.5, juce::Colour(0xff757d8e));
                g.setGradientFill(cylGrad);
                g.fillRoundedRectangle(wheelBounds, 2.0f);

                const int numRibs = 14;
                float stepY = wheelBounds.getHeight() / static_cast<float>(numRibs);
                for (int i = 0; i <= numRibs; ++i)
                {
                    float ry = wheelBounds.getY() + i * stepY;
                    g.setColour(Noir3DPalette::PITCH_BLACK);
                    g.drawHorizontalLine(static_cast<int>(ry), wheelBounds.getX(), wheelBounds.getRight());
                    g.setColour(juce::Colour(0x35ffffff));
                    g.drawHorizontalLine(static_cast<int>(ry + 1.0f), wheelBounds.getX() + 1.0f, wheelBounds.getRight() - 1.0f);
                }

                double rangeLen = slider.getMaximum() - slider.getMinimum();
                float posRatio = (rangeLen > 0.00001) ? static_cast<float>((slider.getValue() - slider.getMinimum()) / rangeLen) : 0.5f;
                float indY = wheelBounds.getBottom() - posRatio * wheelBounds.getHeight();

                g.setColour(Noir3DPalette::BEVEL_SPEC);
                g.drawHorizontalLine(static_cast<int>(indY), wheelBounds.getX() + 2.0f, wheelBounds.getRight() - 2.0f);
                g.setColour(juce::Colour(0xffffffff));
                g.fillEllipse(wheelBounds.getCentreX() - 2.5f, indY - 2.5f, 5.0f, 5.0f);
                return;
            }

            // ================================================================
            // CYBER SCHEMATIC
            // ================================================================
            if (currentSkin == SkinMode::CyberSchematic)
            {
                g.setColour(CyberPalette::BG_DARK);
                g.fillRoundedRectangle(bounds, 3.0f);
                g.setColour(CyberPalette::BORDER);
                g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

                auto wheelBounds = bounds.reduced(4.0f, 4.0f);
                juce::ColourGradient cylGrad(CyberPalette::P900, wheelBounds.getX(), wheelBounds.getY(),
                                             CyberPalette::BG_DARK, wheelBounds.getX(), wheelBounds.getBottom(), false);
                cylGrad.addColour(0.5, CyberPalette::P700);
                g.setGradientFill(cylGrad);
                g.fillRoundedRectangle(wheelBounds, 2.0f);

                const int numRibs = 14;
                float stepY = wheelBounds.getHeight() / static_cast<float>(numRibs);
                for (int i = 0; i <= numRibs; ++i)
                {
                    float ry = wheelBounds.getY() + i * stepY;
                    g.setColour(CyberPalette::P950);
                    g.drawHorizontalLine(static_cast<int>(ry), wheelBounds.getX(), wheelBounds.getRight());
                    g.setColour(juce::Colour(0x22ffffff));
                    g.drawHorizontalLine(static_cast<int>(ry + 1.0f), wheelBounds.getX() + 1.0f, wheelBounds.getRight() - 1.0f);
                }

                double rangeLen = slider.getMaximum() - slider.getMinimum();
                float posRatio = (rangeLen > 0.00001) ? static_cast<float>((slider.getValue() - slider.getMinimum()) / rangeLen) : 0.5f;
                float indY = wheelBounds.getBottom() - posRatio * wheelBounds.getHeight();

                juce::Colour indColour = (slider.getName().containsIgnoreCase("PITCH")) ? CyberPalette::A500 : CyberPalette::S500;
                g.setColour(indColour);
                g.drawHorizontalLine(static_cast<int>(indY), wheelBounds.getX() + 2.0f, wheelBounds.getRight() - 2.0f);
                g.setColour(juce::Colour(0xffffffff));
                g.fillEllipse(wheelBounds.getCentreX() - 2.5f, indY - 2.5f, 5.0f, 5.0f);
                return;
            }

            // ================================================================
            // CLASSIC VINTAGE
            // ================================================================
            g.setColour(juce::Colour(0xff0e0f11));
            g.fillRoundedRectangle(bounds, 3.0f);
            g.setColour(juce::Colour(0xff2d3036));
            g.drawRoundedRectangle(bounds, 3.0f, 1.2f);

            g.setColour(juce::Colour(0xaa000000));
            g.drawRect(bounds.reduced(1.0f), 1.0f);

            auto wheelBounds = bounds.reduced(4.0f, 4.0f);
            juce::ColourGradient cylGrad(juce::Colour(0xff3e424a), wheelBounds.getX(), wheelBounds.getY(),
                                         juce::Colour(0xff18191c), wheelBounds.getX(), wheelBounds.getBottom(), false);
            cylGrad.addColour(0.5, juce::Colour(0xff626773));
            g.setGradientFill(cylGrad);
            g.fillRoundedRectangle(wheelBounds, 2.0f);

            const int numRibs = 14;
            float stepY = wheelBounds.getHeight() / static_cast<float>(numRibs);
            for (int i = 0; i <= numRibs; ++i)
            {
                float ry = wheelBounds.getY() + i * stepY;
                g.setColour(juce::Colour(0xff121316));
                g.drawHorizontalLine(static_cast<int>(ry), wheelBounds.getX(), wheelBounds.getRight());
                g.setColour(juce::Colour(0x44ffffff));
                g.drawHorizontalLine(static_cast<int>(ry + 1.0f), wheelBounds.getX() + 1.0f, wheelBounds.getRight() - 1.0f);
            }

            double rangeLen = slider.getMaximum() - slider.getMinimum();
            float posRatio = (rangeLen > 0.00001) ? static_cast<float>((slider.getValue() - slider.getMinimum()) / rangeLen) : 0.5f;
            float indY = wheelBounds.getBottom() - posRatio * wheelBounds.getHeight();

            g.setColour(juce::Colour(0xffe08b3c));
            g.drawHorizontalLine(static_cast<int>(indY), wheelBounds.getX() + 2.0f, wheelBounds.getRight() - 2.0f);
            g.setColour(juce::Colour(0xffffffff));
            g.fillEllipse(wheelBounds.getCentreX() - 2.0f, indY - 2.0f, 4.0f, 4.0f);
        }

        // ====================================================================
        // ComboBox Drawing (Classic Vintage vs Cyber Schematic vs Noir 3D)
        // ====================================================================
        void drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                          int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                          juce::ComboBox& box) override
        {
            auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);

            if (currentSkin == SkinMode::Noir3D)
            {
                // Chiseled 3D well with directional light
                juce::ColourGradient bgGrad(juce::Colour(0xff1f2128), 0.0f, 0.0f,
                                            juce::Colour(0xff0d0e11), 0.0f, static_cast<float>(height), false);
                g.setGradientFill(bgGrad);
                g.fillRoundedRectangle(bounds, 3.0f);

                g.setColour(Noir3DPalette::BEVEL_MID);
                g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

                // Top highlight line
                g.setColour(Noir3DPalette::BEVEL_LIGHT);
                g.drawHorizontalLine(1, bounds.getX() + 2.0f, bounds.getRight() - 2.0f);

                float arrowX = width - 16.0f;
                float arrowY = height * 0.5f - 2.0f;
                juce::Path arrow;
                arrow.startNewSubPath(arrowX, arrowY);
                arrow.lineTo(arrowX + 4.0f, arrowY + 4.0f);
                arrow.lineTo(arrowX + 8.0f, arrowY);
                g.setColour(box.isEnabled() ? Noir3DPalette::BEVEL_SPEC : Noir3DPalette::TEXT_DIM);
                g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
                return;
            }

            if (currentSkin == SkinMode::CyberSchematic)
            {
                g.setColour(CyberPalette::BG_CARD);
                g.fillRoundedRectangle(bounds, 3.0f);
                g.setColour(CyberPalette::BORDER);
                g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

                float arrowX = width - 16.0f;
                float arrowY = height * 0.5f - 2.0f;
                juce::Path arrow;
                arrow.startNewSubPath(arrowX, arrowY);
                arrow.lineTo(arrowX + 4.0f, arrowY + 4.0f);
                arrow.lineTo(arrowX + 8.0f, arrowY);
                g.setColour(box.isEnabled() ? CyberPalette::S500 : CyberPalette::P700);
                g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
                return;
            }

            // Classic Vintage ComboBox
            juce::ColourGradient bgGrad(juce::Colour(0xff24262b), 0.0f, 0.0f,
                                        juce::Colour(0xff16171a), 0.0f, static_cast<float>(height), false);
            g.setGradientFill(bgGrad);
            g.fillRoundedRectangle(bounds, 3.0f);

            g.setColour(juce::Colour(0xff454952));
            g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

            float arrowX = width - 16.0f;
            float arrowY = height * 0.5f - 2.0f;
            juce::Path arrow;
            arrow.startNewSubPath(arrowX, arrowY);
            arrow.lineTo(arrowX + 4.0f, arrowY + 4.0f);
            arrow.lineTo(arrowX + 8.0f, arrowY);
            g.setColour(box.isEnabled() ? juce::Colour(0xffd47a2a) : juce::Colour(0xff606470));
            g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        }

    private:
        SkinMode currentSkin = SkinMode::ClassicVintage;
    };
}
