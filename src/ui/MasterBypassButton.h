#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ThemePalette.h"

namespace LadderMono
{
    class MasterBypassButton : public juce::Button
    {
    public:
        MasterBypassButton() : juce::Button("MasterBypassButton")
        {
            setClickingTogglesState(true);
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            updateTooltip();
        }

        void setTheme(const ThemeColors& t)
        {
            theme = t;
            repaint();
        }

        void updateTooltip()
        {
            if (getToggleState())
                setTooltip("Audio Engine Bypassed (Muted) — Click to resume sound");
            else
                setTooltip("Audio Engine Active (Sound Running) — Click to bypass");
        }

        void clicked() override
        {
            updateTooltip();
            repaint();
        }

        void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(1.0f);
            bool isBypassed = getToggleState();

            // 1. Recessed outer chassis slot / well
            g.setColour(theme.wellDark);
            g.fillRoundedRectangle(bounds, 4.0f);
            g.setColour(theme.lineBorder);
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

            // 2. Interactive Button Plate
            auto plate = bounds.reduced(2.0f);
            if (shouldDrawButtonAsDown)
                plate = plate.translated(0.0f, 1.0f);

            if (isBypassed)
            {
                // Bypassed state: Rich cautionary dark crimson background
                juce::Colour bgRed = shouldDrawButtonAsHighlighted ? juce::Colour(0xff4a181c) : juce::Colour(0xff2d1215);
                g.setColour(bgRed);
                g.fillRoundedRectangle(plate, 3.0f);

                // Glowing warning red border
                g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(0xffff5544) : juce::Colour(0xffd93829));
                g.drawRoundedRectangle(plate, 3.0f, 1.4f);
            }
            else
            {
                // Normal Active state: Elevated, sleek, tactile plate (clearly available and clickable)
                juce::Colour plateBg = shouldDrawButtonAsHighlighted ? theme.panelLight.brighter(0.20f) : theme.panelLight;
                g.setColour(plateBg);
                g.fillRoundedRectangle(plate, 3.0f);

                // Border with interactive hover response
                if (shouldDrawButtonAsHighlighted)
                {
                    g.setColour(theme.lineAccent);
                    g.drawRoundedRectangle(plate, 3.0f, 1.4f);
                }
                else
                {
                    g.setColour(theme.lineBorder.brighter(0.18f));
                    g.drawRoundedRectangle(plate, 3.0f, 1.0f);
                }
            }

            // 3. Power Icon (Left side of the button)
            float iconX = plate.getX() + 15.0f;
            float iconY = plate.getCentreY();
            float iconR = 5.2f;

            if (isBypassed)
            {
                // Ambient warning glow
                g.setColour(juce::Colour(0x55ff3320));
                g.fillEllipse(iconX - 10.0f, iconY - 10.0f, 20.0f, 20.0f);

                // Warning Red Power Arc
                juce::Colour redColor = shouldDrawButtonAsHighlighted ? juce::Colour(0xffff5544) : juce::Colour(0xffff3b20);
                g.setColour(redColor);

                juce::Path arc;
                arc.addCentredArc(iconX, iconY, iconR, iconR, 0.0f,
                                  juce::MathConstants<float>::pi * 0.22f,
                                  juce::MathConstants<float>::pi * 1.78f, true);
                g.strokePath(arc, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                // Vertical top pin
                g.drawLine(iconX, iconY - iconR - 1.5f, iconX, iconY - 0.5f, 1.6f);

                // Text: BYPASSED
                g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
                g.setColour(juce::Colour(0xffffdddd));
                auto textBounds = plate.withTrimmedLeft(26.0f).withTrimmedRight(4.0f);
                g.drawText("BYPASSED", textBounds.toNearestInt(), juce::Justification::centred);
            }
            else
            {
                // Ambient green/cyan active glow
                g.setColour(juce::Colour(0x4400e6a8));
                g.fillEllipse(iconX - 9.0f, iconY - 9.0f, 18.0f, 18.0f);

                // Emerald Green Power Arc (Engine Alive & Active)
                juce::Colour greenColor = shouldDrawButtonAsHighlighted ? juce::Colour(0xff55ffcc) : juce::Colour(0xff00e6a8);
                g.setColour(greenColor);

                juce::Path arc;
                arc.addCentredArc(iconX, iconY, iconR, iconR, 0.0f,
                                  juce::MathConstants<float>::pi * 0.22f,
                                  juce::MathConstants<float>::pi * 1.78f, true);
                g.strokePath(arc, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                // Vertical top pin
                g.drawLine(iconX, iconY - iconR - 1.5f, iconX, iconY - 0.5f, 1.6f);

                // Text: BYPASS: OFF
                g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
                g.setColour(shouldDrawButtonAsHighlighted ? juce::Colour(0xffffffff) : theme.textPrimary);
                auto textBounds = plate.withTrimmedLeft(25.0f).withTrimmedRight(4.0f);
                g.drawText("BYPASS: OFF", textBounds.toNearestInt(), juce::Justification::centred);
            }
        }

    private:
        ThemeColors theme;
    };
}
