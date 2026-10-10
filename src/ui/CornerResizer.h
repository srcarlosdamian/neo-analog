#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ThemePalette.h"

namespace LadderMono
{
    class CornerResizer : public juce::ResizableCornerComponent
    {
    public:
        CornerResizer(juce::Component* componentToResize, juce::ComponentBoundsConstrainer* constrainer)
            : juce::ResizableCornerComponent(componentToResize, constrainer)
        {
            setMouseCursor(juce::MouseCursor::UpDownLeftRightResizeCursor);
            setAlwaysOnTop(true);
        }

        void setTheme(const ThemeColors& t)
        {
            theme = t;
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            float w = bounds.getWidth();
            float h = bounds.getHeight();

            // Highlight on hover
            bool isHovered = isMouseOver();
            juce::Colour gripCol = isHovered ? theme.lineAccent.brighter(0.5f) : theme.lineBorder.brighter(0.25f);

            // 3 sleek diagonal grip bars
            for (int i = 0; i < 3; ++i)
            {
                float offset = 4.0f + static_cast<float>(i) * 4.5f;

                // Subtle shadow
                g.setColour(juce::Colour(0x66000000));
                g.drawLine(w - offset, h - 2.0f + 1.0f, w - 2.0f + 1.0f, h - offset, 1.4f);

                // Glow on hover
                if (isHovered)
                {
                    g.setColour(theme.lineAccent.withAlpha(0.4f));
                    g.drawLine(w - offset, h - 2.0f, w - 2.0f, h - offset, 2.8f);
                }

                // Crisp bar
                g.setColour(gripCol);
                g.drawLine(w - offset, h - 2.0f, w - 2.0f, h - offset, 1.4f);
            }
        }

    private:
        ThemeColors theme;
    };
}
