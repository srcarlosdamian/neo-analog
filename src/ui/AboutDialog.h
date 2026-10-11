#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "MinimalistLookAndFeel.h"

namespace LadderMono
{
    class AboutDialogOverlay : public juce::Component
    {
    public:
        explicit AboutDialogOverlay(std::function<void()> onCloseCallback)
            : onClose(std::move(onCloseCallback))
        {
            setAlwaysOnTop(true);

            // Close button
            closeBtn.setButtonText(juce::CharPointer_UTF8("\xc3\x97"));
            closeBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22252c));
            closeBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd8dcde));
            closeBtn.onClick = [this] {
                setVisible(false);
                if (onClose) onClose();
            };
            addAndMakeVisible(closeBtn);

            // Website link button
            websiteBtn.setButtonText(juce::CharPointer_UTF8("carlosdamian.com  \xe2\x86\x97"));
            websiteBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2318));
            websiteBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff3e311f));
            websiteBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff5a642));
            websiteBtn.onClick = [] {
                juce::URL("https://carlosdamian.com").launchInDefaultBrowser();
            };
            addAndMakeVisible(websiteBtn);

            // Theme Configuration Buttons (Noir Matte with Theme Studio)
            skinNoirBtn.setButtonText("Noir Matte");
            skinNoirBtn.onClick = [this] {
                updateSkinSelection(MinimalistLookAndFeel::SkinMode::Noir3D);
                if (onSkinChanged) onSkinChanged(MinimalistLookAndFeel::SkinMode::Noir3D);
            };
            addAndMakeVisible(skinNoirBtn);

            updateSkinSelection(MinimalistLookAndFeel::SkinMode::Noir3D);

            // Hidden v1.1 toggle button (Clean neutral text, no emojis)
            revealV11Btn.setButtonText("Upcoming v1.1 Updates");
            revealV11Btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181a20));
            revealV11Btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff2d2315));
            revealV11Btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8c92a2));
            revealV11Btn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffd4d8e2));
            revealV11Btn.setClickingTogglesState(true);
            revealV11Btn.onClick = [this] {
                showV11 = revealV11Btn.getToggleState();
                if (showV11)
                    revealV11Btn.setButtonText("v1.1 Roadmap (Click to hide)");
                else
                    revealV11Btn.setButtonText("Upcoming v1.1 Updates");
                repaint();
            };
            addAndMakeVisible(revealV11Btn);
        }

        std::function<void(MinimalistLookAndFeel::SkinMode)> onSkinChanged;

        void updateSkinSelection(MinimalistLookAndFeel::SkinMode mode)
        {
            activeSkin = mode;
            skinNoirBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff25272e));
            skinNoirBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd4d8e2));
            repaint();
        }

        void showDialog()
        {
            setVisible(true);
            toFront(true);
            grabKeyboardFocus();
            repaint();
        }

        bool keyPressed(const juce::KeyPress& key) override
        {
            if (key == juce::KeyPress::escapeKey)
            {
                setVisible(false);
                if (onClose) onClose();
                return true;
            }
            return false;
        }

        void mouseDown(const juce::MouseEvent& e) override
        {
            auto card = getCardBounds();
            if (!card.contains(e.getPosition()))
            {
                setVisible(false);
                if (onClose) onClose();
            }
        }

        void paint(juce::Graphics& g) override
        {
            // Dimmed semi-transparent backdrop
            g.fillAll(juce::Colour(0xd00a0b0e));

            auto card = getCardBounds().toFloat();

            // Card border (Deep Matte Noir chassis)
            g.setColour(Noir3DPalette::PITCH_BLACK);
            g.fillRoundedRectangle(card.expanded(6.0f), 8.0f);
            g.setColour(Noir3DPalette::BEVEL_SPEC);
            g.drawRoundedRectangle(card.expanded(6.0f), 8.0f, 1.5f);

            juce::ColourGradient cardGrad(Noir3DPalette::PANEL_BG, 0.0f, card.getY(),
                                          Noir3DPalette::PITCH_BLACK, 0.0f, card.getBottom(), false);
            g.setGradientFill(cardGrad);
            g.fillRoundedRectangle(card, 5.0f);
            g.setColour(Noir3DPalette::BEVEL_LIGHT);
            g.drawRoundedRectangle(card, 5.0f, 1.0f);

            // Title Badge
            juce::Rectangle<float> titleBadge(card.getX() + 24.0f, card.getY() + 16.0f, 220.0f, 28.0f);
            juce::ColourGradient badgeGrad(Noir3DPalette::PITCH_BLACK, 0.0f, titleBadge.getY(),
                                           Noir3DPalette::PANEL_BG, 0.0f, titleBadge.getBottom(), false);
            g.setGradientFill(badgeGrad);
            g.fillRoundedRectangle(titleBadge, 3.0f);
            g.setColour(Noir3DPalette::TEXT_TITLE);
            g.drawRoundedRectangle(titleBadge, 3.0f, 1.2f);

            g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
            g.setColour(Noir3DPalette::TEXT_TITLE);
            g.drawText("N E O   A N A L O G", titleBadge.toNearestInt(), juce::Justification::centred);

            // Version Pill Badge
            juce::Rectangle<float> vPill(titleBadge.getRight() + 12.0f, card.getY() + 18.0f, 82.0f, 24.0f);
            g.setColour(juce::Colour(0xff1b2b1d));
            g.fillRoundedRectangle(vPill, 12.0f);
            g.setColour(juce::Colour(0xff48c764));
            g.drawRoundedRectangle(vPill, 12.0f, 1.0f);
            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.drawText("v1.0 (PROD)", vPill.toNearestInt(), juce::Justification::centred);

            // Company: Neo
            juce::Rectangle<float> compPill(vPill.getRight() + 8.0f, card.getY() + 18.0f, 68.0f, 24.0f);
            g.setColour(juce::Colour(0xff1a1e28));
            g.fillRoundedRectangle(compPill, 12.0f);
            g.setColour(juce::Colour(0xff4a84e8));
            g.drawRoundedRectangle(compPill, 12.0f, 1.0f);
            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.drawText("NEO", compPill.toNearestInt(), juce::Justification::centred);

            // Subtitle
            g.setFont(juce::FontOptions(11.5f));
            g.setColour(juce::Colour(0xff8c92a0));
            g.drawText("Analog-Modeled Subtractive Synthesizer with Integrated Arpeggiator",
                       juce::Rectangle<float>(card.getX() + 26.0f, card.getY() + 50.0f, card.getWidth() - 52.0f, 18.0f).toNearestInt(),
                       juce::Justification::centredLeft);

            // Horizontal hairline separator
            g.setColour(juce::Colour(0xff333742));
            g.drawHorizontalLine(static_cast<int>(card.getY() + 74.0f), card.getX() + 24.0f, card.getRight() - 24.0f);

            // Designer Credits & Bio Section
            float bioY = card.getY() + 82.0f;
            g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
            g.setColour(juce::Colour(0xfff0f2f5));
            g.drawText(juce::CharPointer_UTF8("Designed by Carlos Dami\xc3\xa1n"),
                       juce::Rectangle<float>(card.getX() + 26.0f, bioY, 300.0f, 20.0f).toNearestInt(),
                       juce::Justification::centredLeft);

            g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
            g.setColour(activeSkin == MinimalistLookAndFeel::SkinMode::CyberSchematic ? CyberPalette::S500 : juce::Colour(0xffe08b3c));
            g.drawText(juce::CharPointer_UTF8("Product Designer & UX/UI Specialist  \xe2\x80\xa2  Founder of Neo"),
                       juce::Rectangle<float>(card.getX() + 26.0f, bioY + 20.0f, 400.0f, 16.0f).toNearestInt(),
                       juce::Justification::centredLeft);

            // Mini Biography text in English
            g.setFont(juce::FontOptions(11.0f));
            g.setColour(juce::Colour(0xffb5b9c4));
            juce::String bioText(juce::CharPointer_UTF8(
                "Carlos Dami\xc3\xa1n is a Senior Product Designer & UX/UI specialist with over 12 years of experience "
                "crafting digital products, design systems, SaaS platforms, and cutting-edge software for startups and technology "
                "teams globally. Built with Neo to unite precision visual ergonomics with vintage analog sound synthesis."));
            g.drawFittedText(bioText, static_cast<int>(card.getX() + 26.0f), static_cast<int>(bioY + 38.0f),
                             static_cast<int>(card.getWidth() - 52.0f), 42, juce::Justification::topLeft, 3);

            // Hairline separator
            g.setColour(juce::Colour(0xff2d313c));
            g.drawHorizontalLine(static_cast<int>(bioY + 86.0f), card.getX() + 24.0f, card.getRight() - 24.0f);

            // Version details area
            float contentY = bioY + 94.0f;
            if (!showV11)
            {
                // Current Release: v1.0 specifications
                g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
                g.setColour(juce::Colour(0xffe8eaee));
                g.drawText(juce::CharPointer_UTF8("Neo Analog v1.0 \xe2\x80\xa2 Current Production Release"),
                           juce::Rectangle<float>(card.getX() + 26.0f, contentY, 380.0f, 18.0f).toNearestInt(),
                           juce::Justification::centredLeft);

                static const char* v1Features[] = {
                    "\xe2\x9c\x93  202 factory presets in 13 curated sound collections (Basics, Hip-Hop, Pop/Funk, Cinema)",
                    "\xe2\x9c\x93  Discrete 4-pole 24 dB/oct Ladder filter with saturation, resonance, and self-oscillation",
                    "\xe2\x9c\x93  3 vintage multi-wave oscillators with fine detune, noise generator, and overdrive feedback",
                    "\xe2\x9c\x93  Monophonic, 4-voice, and 8-voice polyphony modes with vintage analog voice drift",
                    "\xe2\x9c\x93  Integrated multi-mode arpeggiator with host tempo sync and latch"
                };

                g.setFont(juce::FontOptions(11.0f));
                g.setColour(juce::Colour(0xff9ea4b4));
                for (size_t i = 0; i < 5; ++i)
                {
                    g.drawText(juce::CharPointer_UTF8(v1Features[i]),
                               static_cast<int>(card.getX() + 32.0f), static_cast<int>(contentY + 22.0f + (i * 18.0f)),
                               static_cast<int>(card.getWidth() - 64.0f), 17, juce::Justification::centredLeft);
                }
            }
            else
            {
                // Secret / Hidden v1.1 Roadmap Unlocked
                juce::Rectangle<float> v11Badge(card.getX() + 26.0f, contentY, 140.0f, 20.0f);
                g.setColour(juce::Colour(0xff33240e));
                g.fillRoundedRectangle(v11Badge, 3.0f);
                g.setColour(juce::Colour(0xfff5a642));
                g.drawRoundedRectangle(v11Badge, 3.0f, 1.0f);
                g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
                g.drawText("v1.1 PREVIEW (DEV)", v11Badge.toNearestInt(), juce::Justification::centred);

                g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
                g.setColour(juce::Colour(0xfff5a642));
                g.drawText("Upcoming Updates in Development",
                           juce::Rectangle<float>(card.getX() + 175.0f, contentY, 340.0f, 20.0f).toNearestInt(),
                           juce::Justification::centredLeft);

                static const char* v11Features[] = {
                    "\xe2\x97\x86  Dual Bucket-Brigade (BBD) stereo delay & vintage analog chorus/ensemble unit",
                    "\xe2\x97\x86  Real-time CRT phosphor oscilloscope & live harmonic spectrum analyzer",
                    "\xe2\x97\x86  Micro-tuning engine with Scala (.scl / .kbm) microtonal keyboard mapping",
                    "\xe2\x97\x86  Expanded Modulation Matrix (LFO routing to Pulse Width, Filter FM, Pitch EG)",
                    "\xe2\x97\x86  Full MPE (MIDI Polyphonic Expression) per-note pitch bend and timber pressure"
                };

                g.setFont(juce::FontOptions(11.0f));
                g.setColour(juce::Colour(0xffe0caa0));
                for (size_t i = 0; i < 5; ++i)
                {
                    g.drawText(juce::CharPointer_UTF8(v11Features[i]),
                               static_cast<int>(card.getX() + 32.0f), static_cast<int>(contentY + 22.0f + (i * 18.0f)),
                               static_cast<int>(card.getWidth() - 64.0f), 17, juce::Justification::centredLeft);
                }
            }

            // Skin Selector Label in Settings
            float skinY = card.getBottom() - 84.0f;
            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.setColour(juce::Colour(0xff989ea8));
            g.drawText("THEME ARCHITECTURE: NOIR & LIVE THEME STUDIO (CLICK THEME IN TOP BAR)",
                       juce::Rectangle<float>(card.getX() + 26.0f, skinY, card.getWidth() - 52.0f, 24.0f).toNearestInt(),
                       juce::Justification::centredLeft);

            // Bottom bar info
            g.setColour(juce::Colour(0xff2d313c));
            g.drawHorizontalLine(static_cast<int>(card.getBottom() - 44.0f), card.getX() + 24.0f, card.getRight() - 24.0f);

            g.setFont(juce::FontOptions(10.5f));
            g.setColour(juce::Colour(0xff686d7a));
            g.drawText(juce::CharPointer_UTF8("\xc2\xa9 2026 Neo \xe2\x80\xa2 Designed by Carlos Dami\xc3\xa1n \xe2\x80\xa2 All Rights Reserved"),
                       juce::Rectangle<float>(card.getX() + 26.0f, card.getBottom() - 38.0f, 360.0f, 28.0f).toNearestInt(),
                       juce::Justification::centredLeft);
        }

        void resized() override
        {
            auto card = getCardBounds();
            closeBtn.setBounds(card.getRight() - 42, card.getY() + 14, 28, 28);
            websiteBtn.setBounds(card.getRight() - 170, card.getY() + 82, 145, 24);

            float skinY = card.getBottom() - 86.0f;
            skinNoirBtn.setBounds(card.getRight() - 170, static_cast<int>(skinY), 145, 26);

            revealV11Btn.setBounds(card.getRight() - 250, card.getBottom() - 38, 230, 24);
        }

    private:
        std::function<void()> onClose;
        juce::TextButton closeBtn;
        juce::TextButton websiteBtn;
        juce::TextButton revealV11Btn;
        juce::TextButton skinNoirBtn;
        MinimalistLookAndFeel::SkinMode activeSkin = MinimalistLookAndFeel::SkinMode::Noir3D;
        bool showV11 = false;

        juce::Rectangle<int> getCardBounds() const
        {
            int cardW = 680;
            int cardH = 460;
            return juce::Rectangle<int>((getWidth() - cardW) / 2,
                                        (getHeight() - cardH) / 2,
                                        cardW, cardH);
        }
    };
}
