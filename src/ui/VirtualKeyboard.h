#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>
#include <set>

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
            g.fillAll(juce::Colour(0xff121316));

            // Count white keys
            int whiteKeyCount = 0;
            for (int i = 0; i < keyCount; ++i)
                if (!isBlackKey(startNote + i))
                    whiteKeyCount++;

            if (whiteKeyCount == 0) return;

            float whiteKeyWidth = bounds.getWidth() / static_cast<float>(whiteKeyCount);
            float blackKeyWidth = whiteKeyWidth * 0.62f;
            float blackKeyHeight = bounds.getHeight() * 0.62f;

            // Draw white keys
            float currentX = 0.0f;
            for (int i = 0; i < keyCount; ++i)
            {
                int note = startNote + i;
                if (!isBlackKey(note))
                {
                    bool isDown = (activeNotes.count(note) > 0 || activeKey == note);
                    juce::Rectangle<float> keyRect(currentX, 0.0f, whiteKeyWidth, bounds.getHeight());

                    g.setColour(isDown ? juce::Colour(0xffe08b3c) : juce::Colour(0xffe8eaed));
                    g.fillRect(keyRect);

                    // Key border / separation
                    g.setColour(juce::Colour(0xff222428));
                    g.drawRect(keyRect, 1.0f);

                    // Minimalist computer keyboard key label
                    auto keyLabel = getComputerKeyLabel(note);
                    if (keyLabel.isNotEmpty())
                    {
                        g.setColour(isDown ? juce::Colour(0xffffffff) : juce::Colour(0xff808590));
                        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
                        g.drawText(keyLabel, keyRect.removeFromBottom(18.0f).toNearestInt(), juce::Justification::centred);
                    }

                    currentX += whiteKeyWidth;
                }
            }

            // Draw black keys
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
                        juce::Rectangle<float> blackRect(blackX, 0.0f, blackKeyWidth, blackKeyHeight);

                        g.setColour(isDown ? juce::Colour(0xffd47a2a) : juce::Colour(0xff1b1c20));
                        g.fillRect(blackRect);

                        g.setColour(juce::Colour(0xff0d0e10));
                        g.drawRect(blackRect, 1.0f);

                        auto keyLabel = getComputerKeyLabel(bNote);
                        if (keyLabel.isNotEmpty())
                        {
                            g.setColour(isDown ? juce::Colour(0xffffffff) : juce::Colour(0xffa0a5b2));
                            g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
                            g.drawText(keyLabel, blackRect.removeFromBottom(16.0f).toNearestInt(), juce::Justification::centred);
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
            if (note >= 0 && note != activeKey)
            {
                if (activeKey >= 0)
                {
                    setNoteActive(activeKey, false);
                    if (onNoteOff) onNoteOff(activeKey);
                }
                activeKey = note;
                setNoteActive(note, true);
                if (onNoteOn) onNoteOn(note, 0.85f);
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

    private:
        int startNote = 48;
        int keyCount = 37;
        int activeKey = -1;
        int baseOctaveNote = 60; // C4
        std::set<int> activeNotes;

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
