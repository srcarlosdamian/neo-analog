#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>

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
                    bool isDown = (activeKey == note);
                    juce::Rectangle<float> keyRect(currentX, 0.0f, whiteKeyWidth, bounds.getHeight());

                    g.setColour(isDown ? juce::Colour(0xffe08b3c) : juce::Colour(0xffe8eaed));
                    g.fillRect(keyRect);

                    // Key border / separation
                    g.setColour(juce::Colour(0xff222428));
                    g.drawRect(keyRect, 1.0f);

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
                    // Check if next note is black key
                    if (i + 1 < keyCount && isBlackKey(note + 1))
                    {
                        float blackX = currentX + whiteKeyWidth - (blackKeyWidth * 0.5f);
                        bool isDown = (activeKey == note + 1);
                        juce::Rectangle<float> blackRect(blackX, 0.0f, blackKeyWidth, blackKeyHeight);

                        g.setColour(isDown ? juce::Colour(0xffd47a2a) : juce::Colour(0xff1b1c20));
                        g.fillRect(blackRect);

                        g.setColour(juce::Colour(0xff0d0e10));
                        g.drawRect(blackRect, 1.0f);
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
                if (onNoteOn) onNoteOn(note, 0.85f);
                repaint();
            }
        }

        void mouseDrag(const juce::MouseEvent& e) override
        {
            int note = getNoteAtPos(e.position);
            if (note >= 0 && note != activeKey)
            {
                if (activeKey >= 0 && onNoteOff) onNoteOff(activeKey);
                activeKey = note;
                if (onNoteOn) onNoteOn(note, 0.85f);
                repaint();
            }
        }

        void mouseUp(const juce::MouseEvent&) override
        {
            if (activeKey >= 0)
            {
                if (onNoteOff) onNoteOff(activeKey);
                activeKey = -1;
                repaint();
            }
        }

    private:
        int startNote = 48;
        int keyCount = 37;
        int activeKey = -1;

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
