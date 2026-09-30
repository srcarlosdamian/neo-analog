#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace LadderMono
{
    class MinimalistLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        MinimalistLookAndFeel()
        {
            // Dark minimalist palette
            setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff18191c));
            setColour(juce::Slider::thumbColourId, juce::Colour(0xffe08b3c)); // Warm amber
            setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffe08b3c));
            setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2b2d32));
            setColour(juce::Label::textColourId, juce::Colour(0xffdcdfe4));
            setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff22242a));
            setColour(juce::ComboBox::textColourId, juce::Colour(0xffe8eaed));
            setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff383b42));
            setColour(juce::TextButton::buttonColourId, juce::Colour(0xff26282e));
            setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffe08b3c));
            setColour(juce::TextButton::textColourOnId, juce::Colour(0xff121316));
            setColour(juce::TextButton::textColourOffId, juce::Colour(0xffdcdfe4));
        }

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
            auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
            auto centre = bounds.getCentre();

            // Background circle / track
            juce::Path backgroundArc;
            backgroundArc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f,
                                        rotaryStartAngle, rotaryEndAngle, true);
            g.setColour(juce::Colour(0xff25272d));
            g.strokePath(backgroundArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // Value arc
            if (sliderPosProportional > 0.0f)
            {
                juce::Path valueArc;
                valueArc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f,
                                       rotaryStartAngle, toAngle, true);
                g.setColour(slider.isEnabled() ? juce::Colour(0xffe08b3c) : juce::Colour(0xff555860));
                g.strokePath(valueArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }

            // Minimalist inner knob disc
            float innerRadius = radius - 8.0f;
            if (innerRadius > 4.0f)
            {
                // Subtle gradient
                juce::ColourGradient discGrad(juce::Colour(0xff2d2f36), centre.x, centre.y - innerRadius,
                                             juce::Colour(0xff1e2025), centre.x, centre.y + innerRadius, false);
                g.setGradientFill(discGrad);
                g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

                g.setColour(juce::Colour(0xff3c3f47));
                g.drawEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f, 1.2f);

                // Indicator needle/pip
                juce::Path p;
                auto pointerLength = innerRadius * 0.75f;
                p.addLineSegment(juce::Line<float>(centre.x, centre.y,
                                                  centre.x + pointerLength * std::cos(toAngle - juce::MathConstants<float>::halfPi),
                                                  centre.y + pointerLength * std::sin(toAngle - juce::MathConstants<float>::halfPi)), 2.2f);
                g.setColour(slider.isEnabled() ? juce::Colour(0xfff0a85d) : juce::Colour(0xff606470));
                g.strokePath(p, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat();
            auto toggleBounds = bounds.removeFromLeft(36.0f).reduced(4.0f, 6.0f);

            bool isOn = button.getToggleState();

            // Minimalist rocker / capsule switch
            g.setColour(isOn ? juce::Colour(0xffe08b3c).withAlpha(0.25f) : juce::Colour(0xff22242a));
            g.fillRoundedRectangle(toggleBounds, toggleBounds.getHeight() * 0.5f);

            g.setColour(isOn ? juce::Colour(0xffe08b3c) : juce::Colour(0xff444750));
            g.drawRoundedRectangle(toggleBounds, toggleBounds.getHeight() * 0.5f, 1.2f);

            // Capsule handle
            float knobDiameter = toggleBounds.getHeight() - 4.0f;
            float knobX = isOn ? (toggleBounds.getRight() - knobDiameter - 2.0f) : (toggleBounds.getX() + 2.0f);
            float knobY = toggleBounds.getY() + 2.0f;

            g.setColour(isOn ? juce::Colour(0xffe08b3c) : juce::Colour(0xff888c96));
            g.fillEllipse(knobX, knobY, knobDiameter, knobDiameter);

            // Text
            g.setColour(isOn ? juce::Colour(0xffffffff) : juce::Colour(0xff9ea3b0));
            g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
            g.drawFittedText(button.getButtonText(), bounds.toNearestInt(), juce::Justification::centredLeft, 1);
        }
    };
}
