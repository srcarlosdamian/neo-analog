#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>

namespace LadderMono
{
    class MinimalistLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        MinimalistLookAndFeel()
        {
            // Dark vintage analog palette
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

        // ====================================================================
        // Authentic Skirted Vintage Knob
        // ====================================================================
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
            auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto centre = bounds.getCentre();
            auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            // 1. Drop shadow behind the skirt
            g.setColour(juce::Colour(0x66000000));
            g.fillEllipse(centre.x - radius + 1.0f, centre.y - radius + 2.5f, radius * 2.0f, radius * 2.0f);

            // 2. Flared Outer Skirt (Dark phenolic base)
            juce::ColourGradient skirtGrad(juce::Colour(0xff26282e), centre.x, centre.y - radius,
                                           juce::Colour(0xff141518), centre.x, centre.y + radius, false);
            g.setGradientFill(skirtGrad);
            g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

            // Skirt metallic outer rim
            g.setColour(juce::Colour(0xff3c3f46));
            g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.2f);

            // 3. Skirt Graduations / Tick Marks (11 ticks around rotation arc)
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

            // 4. Raised Inner Fluted Grip Collar
            float colletRadius = radius * 0.65f;
            juce::ColourGradient colletGrad(juce::Colour(0xff32353c), centre.x, centre.y - colletRadius,
                                            juce::Colour(0xff16171a), centre.x, centre.y + colletRadius, false);
            g.setGradientFill(colletGrad);
            g.fillEllipse(centre.x - colletRadius, centre.y - colletRadius, colletRadius * 2.0f, colletRadius * 2.0f);

            g.setColour(juce::Colour(0xff454952));
            g.drawEllipse(centre.x - colletRadius, centre.y - colletRadius, colletRadius * 2.0f, colletRadius * 2.0f, 1.0f);

            // 5. Spun Aluminum Brushed Metal Center Cap
            float capRadius = colletRadius * 0.72f;
            juce::ColourGradient capGrad(juce::Colour(0xffe8eaed), centre.x - capRadius * 0.4f, centre.y - capRadius * 0.4f,
                                         juce::Colour(0xff787c86), centre.x + capRadius * 0.6f, centre.y + capRadius * 0.6f, false);
            g.setGradientFill(capGrad);
            g.fillEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

            // Fine concentric spun rim
            g.setColour(juce::Colour(0xff4e525a));
            g.drawEllipse(centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);

            // 6. High-Contrast Indicator Line
            float cosPtr = std::cos(toAngle - juce::MathConstants<float>::halfPi);
            float sinPtr = std::sin(toAngle - juce::MathConstants<float>::halfPi);

            // White/silver indicator line running through the center cap and onto the skirt
            juce::Path ptr;
            ptr.addLineSegment(juce::Line<float>(centre.x + (capRadius * 0.15f) * cosPtr,
                                                 centre.y + (capRadius * 0.15f) * sinPtr,
                                                 centre.x + (radius * 0.88f) * cosPtr,
                                                 centre.y + (radius * 0.88f) * sinPtr), 2.2f);
            g.setColour(slider.isEnabled() ? juce::Colour(0xffffffff) : juce::Colour(0xff7a7d86));
            g.strokePath(ptr, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // ====================================================================
        // Authentic Vintage 3D Rocker Switch
        // ====================================================================
        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/) override
        {
            auto bounds = button.getLocalBounds().toFloat();
            auto name = button.getButtonText();
            bool isOn = button.getToggleState();

            // Determine Rocker Palette based on function
            juce::Colour activeColour(0xffe65100); // Default Vintage Orange
            if (name == "1" || name == "2" || name == "3" || name.contains("Feed") || name.contains("Ext"))
            {
                activeColour = juce::Colour(0xff29b6f6); // Classic Blue/Cyan Mixer Rocker
            }
            else if (name.contains("Noise") || name.contains("Decay") || name.contains("Latch"))
            {
                activeColour = juce::Colour(0xffe0e4eb); // White/Ivory Rocker
            }

            // Outer recessed bezel area
            float switchWidth = 26.0f;
            float switchHeight = 36.0f;
            auto switchBox = bounds.removeFromLeft(switchWidth + 4.0f).withSizeKeepingCentre(switchWidth, switchHeight);

            // 1. Recessed Bezel (Outer Frame)
            g.setColour(juce::Colour(0xff0d0e10));
            g.fillRoundedRectangle(switchBox, 2.5f);
            g.setColour(juce::Colour(0xff34373e));
            g.drawRoundedRectangle(switchBox, 2.5f, 1.0f);

            // 2. Rocker Paddle (Divided into Top and Bottom Facets)
            auto paddle = switchBox.reduced(2.5f, 2.5f);
            float midY = paddle.getCentreY();
            auto topHalf = paddle.withBottom(midY);
            auto btmHalf = paddle.withTop(midY);

            if (isOn)
            {
                // Switched DOWN / ON:
                // Upper half is angled forward (in shadow), Lower half is depressed (illuminated)
                juce::Colour darkFace = activeColour.darker(0.6f);
                juce::Colour brightFace = activeColour.brighter(0.15f);

                // Top facet (receded/shadow)
                g.setColour(darkFace);
                g.fillRect(topHalf);

                // Bottom facet (pressed/prominent with highlight)
                juce::ColourGradient btmGrad(brightFace, paddle.getX(), midY,
                                             activeColour, paddle.getX(), paddle.getBottom(), false);
                g.setGradientFill(btmGrad);
                g.fillRect(btmHalf);

                // Bottom edge highlight
                g.setColour(brightFace.brighter(0.4f));
                g.drawHorizontalLine(static_cast<int>(paddle.getBottom() - 1.0f), paddle.getX(), paddle.getRight());
            }
            else
            {
                // Switched UP / OFF:
                // Upper half is pressed out (illuminated), Lower half is receded in shadow
                juce::Colour mutedColour = activeColour.withSaturation(activeColour.getSaturation() * 0.4f).darker(0.35f);
                juce::Colour highlight = mutedColour.brighter(0.3f);

                // Top facet (raised/highlighted)
                juce::ColourGradient topGrad(highlight, paddle.getX(), paddle.getY(),
                                             mutedColour, paddle.getX(), midY, false);
                g.setGradientFill(topGrad);
                g.fillRect(topHalf);

                // Bottom facet (shadow)
                g.setColour(mutedColour.darker(0.5f));
                g.fillRect(btmHalf);

                // Top edge highlight
                g.setColour(highlight.brighter(0.3f));
                g.drawHorizontalLine(static_cast<int>(paddle.getY()), paddle.getX(), paddle.getRight());
            }

            // Subtle center divider crease
            g.setColour(juce::Colour(0x55000000));
            g.drawHorizontalLine(static_cast<int>(midY), paddle.getX(), paddle.getRight());

            // 3. Label Text Beside Switch
            bounds.removeFromLeft(4.0f);
            g.setColour(isOn ? juce::Colour(0xffffffff) : juce::Colour(0xff9ea3b0));
            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.drawFittedText(name, bounds.toNearestInt(), juce::Justification::centredLeft, 1);
        }

        // ====================================================================
        // Authentic Fluted Performance Wheels (Pitch & Mod)
        // ====================================================================
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/,
                              juce::Slider::SliderStyle /*style*/, juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);

            // 1. Recessed Wheel Well (Deep Cavity)
            g.setColour(juce::Colour(0xff0e0f11));
            g.fillRoundedRectangle(bounds, 3.0f);
            g.setColour(juce::Colour(0xff2d3036));
            g.drawRoundedRectangle(bounds, 3.0f, 1.2f);

            // Inner well drop shadow
            g.setColour(juce::Colour(0xaa000000));
            g.drawRect(bounds.reduced(1.0f), 1.0f);

            // 2. Visible Wheel Cylinder
            auto wheelBounds = bounds.reduced(4.0f, 4.0f);

            // Cylinder curvature gradient
            juce::ColourGradient cylGrad(juce::Colour(0xff3e424a), wheelBounds.getX(), wheelBounds.getY(),
                                         juce::Colour(0xff18191c), wheelBounds.getX(), wheelBounds.getBottom(), false);
            cylGrad.addColour(0.5, juce::Colour(0xff626773)); // Center specular reflection
            g.setGradientFill(cylGrad);
            g.fillRoundedRectangle(wheelBounds, 2.0f);

            // 3. Tactile Ribs / Flutes on the wheel
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

            // 4. Center Notch / Indicator line corresponding to slider position
            double rangeLen = slider.getMaximum() - slider.getMinimum();
            float posRatio = (rangeLen > 0.00001) ? static_cast<float>((slider.getValue() - slider.getMinimum()) / rangeLen) : 0.5f;
            float indY = wheelBounds.getBottom() - posRatio * wheelBounds.getHeight();

            // Draw center marker or position dot
            g.setColour(juce::Colour(0xffe08b3c));
            g.drawHorizontalLine(static_cast<int>(indY), wheelBounds.getX() + 2.0f, wheelBounds.getRight() - 2.0f);
            g.setColour(juce::Colour(0xffffffff));
            g.fillEllipse(wheelBounds.getCentreX() - 2.0f, indY - 2.0f, 4.0f, 4.0f);
        }

        // ====================================================================
        // Vintage Selector ComboBox
        // ====================================================================
        void drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                          int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                          juce::ComboBox& box) override
        {
            auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);

            // Dark phenolic casing
            juce::ColourGradient bgGrad(juce::Colour(0xff24262b), 0.0f, 0.0f,
                                        juce::Colour(0xff16171a), 0.0f, static_cast<float>(height), false);
            g.setGradientFill(bgGrad);
            g.fillRoundedRectangle(bounds, 3.0f);

            // Bevel border
            g.setColour(juce::Colour(0xff454952));
            g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

            // Down chevron arrow
            float arrowX = width - 16.0f;
            float arrowY = height * 0.5f - 2.0f;
            juce::Path arrow;
            arrow.startNewSubPath(arrowX, arrowY);
            arrow.lineTo(arrowX + 4.0f, arrowY + 4.0f);
            arrow.lineTo(arrowX + 8.0f, arrowY);
            g.setColour(box.isEnabled() ? juce::Colour(0xffd47a2a) : juce::Colour(0xff606470));
            g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        }
    };
}
