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

        void setTheme(const ThemeColors& t)
        {
            theme = t;
            repaint();
        }

        void setSkinMode(MinimalistLookAndFeel::SkinMode mode)
        {
            skinMode = mode;
            if (mode == MinimalistLookAndFeel::SkinMode::CyberSchematic)
                setTheme(ThemeColors::getCyberCyan());
            else if (mode == MinimalistLookAndFeel::SkinMode::ClassicVintage)
                setTheme(ThemeColors::getAmberMatrix());
            else
                setTheme(ThemeColors::getDefaultNoir());
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
            g.fillAll(theme.bgChassis);

            // 1. Felt Strip at the top
            float feltHeight = 5.0f;
            auto feltRect = bounds.removeFromTop(feltHeight);
            g.setColour(theme.wellDark);
            g.fillRect(feltRect);
            g.setColour(theme.lineBorder);
            g.drawLine(feltRect.getX(), feltRect.getBottom(), feltRect.getRight(), feltRect.getBottom(), theme.borderWidth);

            // Count white keys
            int whiteKeyCount = 0;
            for (int i = 0; i < keyCount; ++i)
                if (!isBlackKey(startNote + i))
                    whiteKeyCount++;

            if (whiteKeyCount == 0) return;

            float whiteKeyWidth = bounds.getWidth() / static_cast<float>(whiteKeyCount);
            float blackKeyWidth = whiteKeyWidth * 0.62f;
            float blackKeyHeight = bounds.getHeight() * 0.62f;

            bool isNoirKeyboard = (theme.keyWhiteRest.getBrightness() < 0.35f);

            // 2. Draw White / Natural Keys
            float currentX = 0.0f;
            for (int i = 0; i < keyCount; ++i)
            {
                int note = startNote + i;
                if (!isBlackKey(note))
                {
                    bool isDown = (activeNotes.count(note) > 0 || activeKey == note);
                    juce::Rectangle<float> keyRect(currentX, bounds.getY(), whiteKeyWidth, bounds.getHeight());

                    if (isNoirKeyboard)
                    {
                        // Noir All-Black Natural Keys: Sleek matte velvet charcoal plate
                        if (isDown)
                        {
                            g.setColour(theme.keyWhiteDown);
                            g.fillRoundedRectangle(keyRect, theme.cornerRadius);
                            g.setColour(juce::Colour(0xaa000000));
                            g.fillRect(keyRect.getX(), keyRect.getY(), keyRect.getWidth(), 14.0f);
                        }
                        else
                        {
                            juce::ColourGradient matteGrad(theme.keyWhiteRest.brighter(0.06f), keyRect.getX(), keyRect.getY(),
                                                           theme.keyWhiteRest.darker(0.12f), keyRect.getX(), keyRect.getBottom(), false);
                            g.setGradientFill(matteGrad);
                            g.fillRoundedRectangle(keyRect, theme.cornerRadius);

                            // Subtle top fine rim
                            g.setColour(juce::Colour(0x18ffffff));
                            g.drawHorizontalLine(static_cast<int>(keyRect.getY()), keyRect.getX() + 1.0f, keyRect.getRight() - 1.0f);
                        }

                        // Front lip shadow
                        g.setColour(juce::Colour(0x44000000));
                        g.fillRoundedRectangle(keyRect.removeFromBottom(4.0f), theme.cornerRadius * 0.5f);

                        // Key separation groove (deep pitch black recess between keys)
                        g.setColour(theme.bgChassis);
                        g.drawRoundedRectangle(juce::Rectangle<float>(currentX, bounds.getY(), whiteKeyWidth, bounds.getHeight()), theme.cornerRadius, theme.borderWidth);
                    }
                    else
                    {
                        if (isDown)
                        {
                            g.setColour(theme.keyWhiteDown);
                            g.fillRect(keyRect);

                            // Top rail drop shadow
                            g.setColour(juce::Colour(0x66000000));
                            g.fillRect(keyRect.getX(), keyRect.getY(), keyRect.getWidth(), 12.0f);
                        }
                        else
                        {
                            g.setColour(theme.keyWhiteRest);
                            g.fillRect(keyRect);

                            // Top fine rim
                            g.setColour(juce::Colour(0x33ffffff));
                            g.drawHorizontalLine(static_cast<int>(keyRect.getY()), keyRect.getX() + 1.0f, keyRect.getRight() - 1.0f);
                        }

                        // Front lip shadow
                        g.setColour(juce::Colour(0x22000000));
                        g.fillRect(keyRect.removeFromBottom(4.0f));

                        // Key separation groove
                        g.setColour(theme.lineBorder);
                        g.drawRect(juce::Rectangle<float>(currentX, bounds.getY(), whiteKeyWidth, bounds.getHeight()), theme.borderWidth);
                    }

                    currentX += whiteKeyWidth;
                }
            }

            // 3. Draw Black / Accidental Keys
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

                        if (isNoirKeyboard)
                        {
                            // 1. Soft matte drop shadow onto natural keys
                            g.setColour(juce::Colour(0x77000000));
                            g.fillRoundedRectangle(blackRect.translated(2.0f, 2.5f), theme.cornerRadius * 0.5f);

                            // 2. Pure Flat Matte Black Key Body (no glossy sheen or specular reflections)
                            float r = juce::jlimit(1.0f, 4.0f, theme.cornerRadius * 0.6f);
                            g.setColour(isDown ? theme.keyBlackDown : theme.keyBlackRest);
                            g.fillRoundedRectangle(blackRect, r);

                            // 3. Subtle bottom lip crease (matte depth)
                            auto bottomLip = blackRect.removeFromBottom(4.0f);
                            g.setColour(juce::Colour(0x44000000));
                            g.fillRoundedRectangle(bottomLip, r * 0.5f);

                            // 4. Subtle crisp matte perimeter border
                            g.setColour(theme.lineBorder.brighter(0.05f));
                            g.drawRoundedRectangle(blackRect.withHeight(blackKeyHeight), r, theme.borderWidth);
                        }
                        else
                        {
                            // Drop shadow
                            g.setColour(juce::Colour(0x55000000));
                            g.fillRect(blackRect.translated(1.5f, 2.0f));

                            if (isDown)
                            {
                                g.setColour(theme.keyBlackDown);
                                g.fillRect(blackRect);
                            }
                            else
                            {
                                g.setColour(theme.keyBlackRest);
                                g.fillRect(blackRect);
                            }
                            g.setColour(theme.lineBorder);
                            g.drawRect(blackRect, theme.borderWidth);
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
        MinimalistLookAndFeel::SkinMode skinMode = MinimalistLookAndFeel::SkinMode::Noir3D;
        ThemeColors theme;

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
