#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>
#include <set>

#include "MinimalistLookAndFeel.h"

namespace LadderMono
{
    class VirtualKeyboard : public juce::Component
    {
    public:
        VirtualKeyboard(int startMidiNote = 48, int numKeys = 37) // 3 octaves C3 to C6
            : startNote(startMidiNote), keyCount(numKeys)
        {
        }

        std::function<void(int note, float velocity)> onNoteOn;
        std::function<void(int note)> onNoteOff;

        void setSkinMode(MinimalistLookAndFeel::SkinMode mode)
        {
            skinMode = mode;
            repaint();
        }

        void setNoteActive(int note, bool active)
        {
            if (active)
                activeNotes.insert(note);
            else
                activeNotes.erase(note);
            repaint();
        }

        void clearAllActiveNotes()
        {
            activeNotes.clear();
            repaint();
        }

        void setBaseOctaveNote(int note)
        {
            baseOctaveNote = note;
            repaint();
        }

        int getBaseOctaveNote() const noexcept { return baseOctaveNote; }

        juce::String getComputerKeyLabel(int note) const
        {
            int offset = note - baseOctaveNote;
            switch (offset)
            {
                case 0:  return "A";
                case 1:  return "W";
                case 2:  return "S";
                case 3:  return "E";
                case 4:  return "D";
                case 5:  return "F";
                case 6:  return "T";
                case 7:  return "G";
                case 8:  return "Y";
                case 9:  return "H";
                case 10: return "U";
                case 11: return "J";
                case 12: return "K";
                case 13: return "O";
                case 14: return "L";
                case 15: return "P";
                default: return {};
            }
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            g.fillAll(skinMode == MinimalistLookAndFeel::SkinMode::CyberSchematic ? CyberPalette::BG_DARK : juce::Colour(0xff0d0e10));

            // 1. Felt Strip at the top
            float feltHeight = 5.0f;
            auto feltRect = bounds.removeFromTop(feltHeight);
            if (skinMode == MinimalistLookAndFeel::SkinMode::CyberSchematic)
            {
                g.setColour(CyberPalette::P950);
                g.fillRect(feltRect);
                g.setColour(CyberPalette::R500);
                g.drawHorizontalLine(static_cast<int>(feltRect.getY() + 1.0f), feltRect.getX(), feltRect.getRight());
            }
            else if (skinMode == MinimalistLookAndFeel::SkinMode::Noir3D)
            {
                g.setColour(Noir3DPalette::PITCH_BLACK);
                g.fillRect(feltRect);
                g.setColour(Noir3DPalette::BEVEL_SPEC.withAlpha(0.4f));
                g.drawHorizontalLine(static_cast<int>(feltRect.getY()), feltRect.getX(), feltRect.getRight());
                g.setColour(Noir3DPalette::BEVEL_DARK);
                g.drawHorizontalLine(static_cast<int>(feltRect.getBottom()), feltRect.getX(), feltRect.getRight());
            }
            else
            {
                juce::ColourGradient feltGrad(juce::Colour(0xffb71c1c), 0.0f, feltRect.getY(),
                                              juce::Colour(0xff7f0000), 0.0f, feltRect.getBottom(), false);
                g.setGradientFill(feltGrad);
                g.fillRect(feltRect);
                g.setColour(juce::Colour(0x66000000));
                g.drawHorizontalLine(static_cast<int>(feltRect.getBottom()), feltRect.getX(), feltRect.getRight());
            }

            // Count white keys
            int whiteKeyCount = 0;
            for (int i = 0; i < keyCount; ++i)
                if (!isBlackKey(startNote + i))
                    whiteKeyCount++;

            if (whiteKeyCount == 0) return;

            float whiteKeyWidth = bounds.getWidth() / static_cast<float>(whiteKeyCount);
            float blackKeyWidth = whiteKeyWidth * 0.62f;
            float blackKeyHeight = bounds.getHeight() * 0.62f;

            bool isCyber = (skinMode == MinimalistLookAndFeel::SkinMode::CyberSchematic);
            bool isNoir  = (skinMode == MinimalistLookAndFeel::SkinMode::Noir3D);

            // 2. Draw White Keys
            float currentX = 0.0f;
            for (int i = 0; i < keyCount; ++i)
            {
                int note = startNote + i;
                if (!isBlackKey(note))
                {
                    bool isDown = (activeNotes.count(note) > 0 || activeKey == note);
                    juce::Rectangle<float> keyRect(currentX, bounds.getY(), whiteKeyWidth, bounds.getHeight());

                    if (isDown)
                    {
                        if (isCyber)
                        {
                            juce::ColourGradient cyberDown(CyberPalette::S500, keyRect.getX(), keyRect.getY(),
                                                          CyberPalette::S700, keyRect.getX(), keyRect.getBottom(), false);
                            g.setGradientFill(cyberDown);
                            g.fillRect(keyRect);
                        }
                        else if (isNoir)
                        {
                            // Pure physical lighting: key is depressed into the bed under directional light
                            // Dark cavity shadow at top rail, face tilted into shadow
                            juce::ColourGradient noirDown(juce::Colour(0xff686d78), keyRect.getX(), keyRect.getY(),
                                                          juce::Colour(0xff868b98), keyRect.getX(), keyRect.getBottom(), false);
                            g.setGradientFill(noirDown);
                            g.fillRect(keyRect);

                            // Top rail cavity drop shadow
                            juce::ColourGradient insetShadow(juce::Colour(0x99000000), keyRect.getX(), keyRect.getY(),
                                                             juce::Colour(0x00000000), keyRect.getX(), keyRect.getY() + 16.0f, false);
                            g.setGradientFill(insetShadow);
                            g.fillRect(keyRect.getX(), keyRect.getY(), keyRect.getWidth(), 16.0f);
                        }
                        else
                        {
                            juce::ColourGradient downGrad(juce::Colour(0xffe88d38), keyRect.getX(), keyRect.getY(),
                                                         juce::Colour(0xffcf7120), keyRect.getX(), keyRect.getBottom(), false);
                            g.setGradientFill(downGrad);
                            g.fillRect(keyRect);
                        }
                    }
                    else
                    {
                        if (isCyber)
                        {
                            juce::ColourGradient cyberWhite(juce::Colour(0xfff1f3f7), keyRect.getX(), keyRect.getY(),
                                                           juce::Colour(0xffdfe2eb), keyRect.getX(), keyRect.getBottom(), false);
                            g.setGradientFill(cyberWhite);
                            g.fillRect(keyRect);
                        }
                        else if (isNoir)
                        {
                            // Pure physical lighting: key is resting at full height
                            // Clean matte platinum with sharp top specular highlight
                            juce::ColourGradient noirWhite(juce::Colour(0xffe8ebf2), keyRect.getX(), keyRect.getY(),
                                                           juce::Colour(0xffc2c7d2), keyRect.getX(), keyRect.getBottom(), false);
                            g.setGradientFill(noirWhite);
                            g.fillRect(keyRect);

                            // Top specular edge catching light
                            g.setColour(juce::Colour(0xffffffff));
                            g.drawHorizontalLine(static_cast<int>(keyRect.getY()), keyRect.getX() + 1.0f, keyRect.getRight() - 1.0f);
                        }
                        else
                        {
                            juce::ColourGradient ivoryGrad(juce::Colour(0xfffaf8f2), keyRect.getX(), keyRect.getY(),
                                                          juce::Colour(0xffeae4d5), keyRect.getX(), keyRect.getBottom(), false);
                            g.setGradientFill(ivoryGrad);
                            g.fillRect(keyRect);
                        }
                    }

                    // Front lip shadow
                    g.setColour(juce::Colour(0x22000000));
                    g.fillRect(keyRect.removeFromBottom(4.0f));

                    // Key separation groove
                    g.setColour(isCyber ? CyberPalette::P800 : (isNoir ? Noir3DPalette::PITCH_BLACK : juce::Colour(0xff222428)));
                    g.drawRect(juce::Rectangle<float>(currentX, bounds.getY(), whiteKeyWidth, bounds.getHeight()), 1.0f);

                    // Computer keyboard key label
                    auto keyLabel = getComputerKeyLabel(note);
                    if (keyLabel.isNotEmpty())
                    {
                        g.setColour(isDown ? juce::Colour(0xffffffff) : (isCyber ? CyberPalette::P600 : (isNoir ? Noir3DPalette::TEXT_ACCENT : juce::Colour(0xff7a808c))));
                        g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
                        g.drawText(keyLabel, juce::Rectangle<float>(currentX, bounds.getBottom() - 20.0f, whiteKeyWidth, 16.0f).toNearestInt(), juce::Justification::centred);
                    }

                    currentX += whiteKeyWidth;
                }
            }

            // 3. Draw Black Keys
            currentX = 0.0f;
            for (int i = 0; i < keyCount; ++i)
            {
                int note = startNote + i;
                if (!isBlackKey(note))
                {
                    if (i + 1 < keyCount && isBlackKey(note + 1))
                    {
                        int bNote = note + 1;
                        float blackX = currentX + whiteKeyWidth - (blackKeyWidth * 0.5f);
                        bool isDown = (activeNotes.count(bNote) > 0 || activeKey == bNote);
                        juce::Rectangle<float> blackRect(blackX, bounds.getY(), blackKeyWidth, blackKeyHeight);

                        // Drop shadow
                        g.setColour(isNoir ? (isDown ? juce::Colour(0x33000000) : juce::Colour(0x77000000)) : juce::Colour(0x55000000));
                        g.fillRect(blackRect.translated(isNoir && isDown ? 1.0f : 2.0f, isNoir && isDown ? 1.0f : 2.5f));

                        if (isDown)
                        {
                            if (isCyber)
                            {
                                juce::ColourGradient bCyberDown(CyberPalette::A500, blackRect.getX(), blackRect.getY(),
                                                               CyberPalette::A800, blackRect.getX(), blackRect.getBottom(), false);
                                g.setGradientFill(bCyberDown);
                            }
                            else if (isNoir)
                            {
                                // Depressed black key: lower elevation, deeper cavity shadow
                                juce::ColourGradient bNoirDown(juce::Colour(0xff121317), blackRect.getX(), blackRect.getY(),
                                                              juce::Colour(0xff060709), blackRect.getX(), blackRect.getBottom(), false);
                                g.setGradientFill(bNoirDown);
                            }
                            else
                            {
                                juce::ColourGradient bDown(juce::Colour(0xffd47a2a), blackRect.getX(), blackRect.getY(),
                                                          juce::Colour(0xff9e5210), blackRect.getX(), blackRect.getBottom(), false);
                                g.setGradientFill(bDown);
                            }
                        }
                        else
                        {
                            if (isCyber)
                            {
                                juce::ColourGradient bCyberGrad(juce::Colour(0xff22252e), blackRect.getX(), blackRect.getY(),
                                                               CyberPalette::BG_DARK, blackRect.getX(), blackRect.getBottom(), false);
                                g.setGradientFill(bCyberGrad);
                            }
                            else if (isNoir)
                            {
                                // Resting black key: sharp directional top-left light
                                juce::ColourGradient bNoirGrad(juce::Colour(0xff282b35), blackRect.getX(), blackRect.getY(),
                                                              juce::Colour(0xff111216), blackRect.getX(), blackRect.getBottom(), false);
                                g.setGradientFill(bNoirGrad);
                            }
                            else
                            {
                                juce::ColourGradient ebonyGrad(juce::Colour(0xff2e3036), blackRect.getX(), blackRect.getY(),
                                                              juce::Colour(0xff141517), blackRect.getX(), blackRect.getBottom(), false);
                                g.setGradientFill(ebonyGrad);
                            }
                        }
                        g.fillRoundedRectangle(blackRect, 1.5f);

                        // Front bevel edge / directional rim
                        if (isNoir)
                        {
                            g.setColour(isDown ? juce::Colour(0xff1a1b20) : Noir3DPalette::BEVEL_SPEC.withAlpha(0.6f));
                            g.drawRoundedRectangle(blackRect, 1.5f, 1.0f);
                        }
                        else
                        {
                            g.setColour(isDown ? (isCyber ? CyberPalette::A300 : juce::Colour(0xffea9547)) : (isCyber ? CyberPalette::BORDER_HI : juce::Colour(0xff42454e)));
                            g.drawRoundedRectangle(blackRect, 1.5f, 1.0f);
                        }

                        auto keyLabel = getComputerKeyLabel(bNote);
                        if (keyLabel.isNotEmpty())
                        {
                            g.setColour(isDown ? juce::Colour(0xffffffff) : (isCyber ? CyberPalette::P300 : (isNoir ? Noir3DPalette::TEXT_ACCENT : juce::Colour(0xffa8adb8))));
                            g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
                            g.drawText(keyLabel, juce::Rectangle<float>(blackX, blackRect.getBottom() - 18.0f, blackKeyWidth, 14.0f).toNearestInt(), juce::Justification::centred);
                        }
                    }
                    currentX += whiteKeyWidth;
                }
            }
        }

        void mouseDown(const juce::MouseEvent& e) override
        {
            int note = getNoteAtPos(e.position);
            if (note >= 0)
            {
                activeKey = note;
                setNoteActive(note, true);
                if (onNoteOn) onNoteOn(note, 0.85f);
            }
        }

        void mouseDrag(const juce::MouseEvent& e) override
        {
            int note = getNoteAtPos(e.position);
            if (note != activeKey)
            {
                if (activeKey >= 0)
                {
                    setNoteActive(activeKey, false);
                    if (onNoteOff) onNoteOff(activeKey);
                    activeKey = -1;
                }
                if (note >= 0)
                {
                    activeKey = note;
                    setNoteActive(note, true);
                    if (onNoteOn) onNoteOn(note, 0.85f);
                }
            }
        }

        void mouseUp(const juce::MouseEvent&) override
        {
            if (activeKey >= 0)
            {
                setNoteActive(activeKey, false);
                if (onNoteOff) onNoteOff(activeKey);
                activeKey = -1;
            }
        }

        void mouseExit(const juce::MouseEvent&) override
        {
            if (activeKey >= 0)
            {
                setNoteActive(activeKey, false);
                if (onNoteOff) onNoteOff(activeKey);
                activeKey = -1;
            }
        }

    private:
        int startNote = 48;
        int keyCount = 37;
        int activeKey = -1;
        int baseOctaveNote = 60; // C4
        std::set<int> activeNotes;
        MinimalistLookAndFeel::SkinMode skinMode = MinimalistLookAndFeel::SkinMode::ClassicVintage;

        static bool isBlackKey(int midiNote) noexcept
        {
            int noteInOctave = midiNote % 12;
            return (noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 || noteInOctave == 8 || noteInOctave == 10);
        }

        int getNoteAtPos(juce::Point<float> pos)
        {
            auto bounds = getLocalBounds().toFloat();
            int whiteKeyCount = 0;
            for (int i = 0; i < keyCount; ++i)
                if (!isBlackKey(startNote + i))
                    whiteKeyCount++;

            float whiteKeyWidth = bounds.getWidth() / static_cast<float>(whiteKeyCount);
            float blackKeyWidth = whiteKeyWidth * 0.62f;
            float blackKeyHeight = bounds.getHeight() * 0.62f;

            // Check black keys first (they sit on top)
            if (pos.y <= blackKeyHeight)
            {
                float currentX = 0.0f;
                for (int i = 0; i < keyCount; ++i)
                {
                    int note = startNote + i;
                    if (!isBlackKey(note))
                    {
                        if (i + 1 < keyCount && isBlackKey(note + 1))
                        {
                            float blackX = currentX + whiteKeyWidth - (blackKeyWidth * 0.5f);
                            if (pos.x >= blackX && pos.x <= blackX + blackKeyWidth)
                                return note + 1;
                        }
                        currentX += whiteKeyWidth;
                    }
                }
            }

            // Check white keys
            float currentX = 0.0f;
            for (int i = 0; i < keyCount; ++i)
            {
                int note = startNote + i;
                if (!isBlackKey(note))
                {
                    if (pos.x >= currentX && pos.x < currentX + whiteKeyWidth)
                        return note;
                    currentX += whiteKeyWidth;
                }
            }

            return -1;
        }
    };
}
