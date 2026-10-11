#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ThemePalette.h"

namespace LadderMono
{
    class ParameterHelpBar : public juce::Component
    {
    public:
        ParameterHelpBar()
        {
            setInterceptsMouseClicks(false, false);
        }

        void setHelpText(const juce::String& text, const juce::String& paramName = {})
        {
            if (currentText != text || currentParam != paramName)
            {
                currentText = text;
                currentParam = paramName;
                repaint();
            }
        }

        void setTheme(const ThemeColors& t)
        {
            theme = t;
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            float r = 3.5f;

            // Subtle dark pill container background
            g.setColour(theme.wellDark.withAlpha(0.95f));
            g.fillRoundedRectangle(bounds, r);
            g.setColour(theme.lineBorder);
            g.drawRoundedRectangle(bounds, r, 0.9f);

            // Help Icon Badge (Left side)
            float badgeSize = bounds.getHeight() - 6.0f;
            auto iconCircle = juce::Rectangle<float>(bounds.getX() + 4.0f, bounds.getY() + 3.0f, badgeSize, badgeSize);
            g.setColour(theme.buttonBg);
            g.fillEllipse(iconCircle);
            g.setColour(theme.lineBorder);
            g.drawEllipse(iconCircle, 0.8f);

            // '?' Icon Mark
            g.setFont(juce::FontOptions(badgeSize * 0.72f, juce::Font::bold));
            g.setColour(theme.textPrimary);
            g.drawText("?", iconCircle, juce::Justification::centred);

            // Help Text Area (1 single line, very small text)
            float textX = iconCircle.getRight() + 7.0f;
            float textW = bounds.getRight() - textX - 8.0f;
            auto textBounds = juce::Rectangle<float>(textX, 0.0f, textW, bounds.getHeight());

            if (currentParam.isNotEmpty())
            {
                juce::AttributedString str;
                str.append(currentParam.toUpperCase() + "  \xe2\x80\xa2  ",
                           juce::FontOptions(9.5f * theme.fontScale, juce::Font::bold),
                           theme.textPrimary);
                str.append(currentText,
                           juce::FontOptions(9.0f * theme.fontScale, juce::Font::plain),
                           theme.textSecondary);
                str.setWordWrap(juce::AttributedString::WordWrap::none);
                str.draw(g, textBounds);
            }
            else
            {
                g.setFont(juce::FontOptions(9.0f * theme.fontScale, juce::Font::plain));
                g.setColour(theme.textDim);
                g.drawText(currentText, textBounds.toNearestInt(), juce::Justification::centredLeft, true);
            }
        }

    private:
        ThemeColors theme;
        juce::String currentText = "Hover over any parameter for instant functional guide & DSP signal flow notes.";
        juce::String currentParam;
    };
}
