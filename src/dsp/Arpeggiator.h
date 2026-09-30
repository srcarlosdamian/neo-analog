#pragma once
#include <vector>
#include <algorithm>
#include <random>

namespace LadderMono
{
    enum class ArpMode
    {
        Up = 0,
        Down,
        UpDown,
        Random,
        AsPlayed,
        Count
    };

    enum class ArpRateSync
    {
        Quarter = 0,    // 1/4
        Eighth,         // 1/8
        EighthTriplet,  // 1/8T
        Sixteenth,      // 1/16
        SixteenthTriplet,// 1/16T
        ThirtySecond,   // 1/32
        Count
    };

    class Arpeggiator
    {
    public:
        Arpeggiator() : rng(42) {}

        void prepare(double sr) noexcept
        {
            sampleRate = sr;
            reset();
        }

        void reset() noexcept
        {
            heldNotes.clear();
            latchedNotes.clear();
            sequence.clear();
            currentIndex = 0;
            sampleCounter = 0;
            currentNotePlaying = -1;
            isNoteOn = false;
            isFirstStep = true;
        }

        void setEnabled(bool enabled) noexcept { isEnabled = enabled; }
        bool getEnabled() const noexcept { return isEnabled; }

        void setLatch(bool latch) noexcept
        {
            isLatch = latch;
            if (!isLatch && heldNotes.empty())
            {
                latchedNotes.clear();
                rebuildSequence();
            }
        }

        void setMode(ArpMode m) noexcept
        {
            if (mode != m)
            {
                mode = m;
                rebuildSequence();
            }
        }

        void setOctaves(int oct) noexcept
        {
            oct = std::clamp(oct, 1, 4);
            if (octaves != oct)
            {
                octaves = oct;
                rebuildSequence();
            }
        }

        void setRateSync(ArpRateSync r) noexcept { rateSync = r; }
        void setFreeRateHz(float hz) noexcept { freeRateHz = std::clamp(hz, 0.5f, 30.0f); }
        void setSyncMode(bool sync) noexcept { isTempoSynced = sync; }
        void setGate(float g) noexcept { gateRatio = std::clamp(g, 0.05f, 1.0f); }

        void noteOn(int note)
        {
            if (std::find(heldNotes.begin(), heldNotes.end(), note) == heldNotes.end())
            {
                heldNotes.push_back(note);
            }

            if (isLatch)
            {
                // If all keys were released and a new note starts, reset latch
                if (allKeysWereReleased)
                {
                    latchedNotes.clear();
                    allKeysWereReleased = false;
                }
                if (std::find(latchedNotes.begin(), latchedNotes.end(), note) == latchedNotes.end())
                    latchedNotes.push_back(note);
            }

            rebuildSequence();
        }

        void noteOff(int note)
        {
            auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
            if (it != heldNotes.end())
                heldNotes.erase(it);

            if (heldNotes.empty())
                allKeysWereReleased = true;

            if (!isLatch)
            {
                rebuildSequence();
            }
        }

        // Returns true if a note is currently active from the arpeggiator,
        // and sets activeMidiNote and whether this sample triggered a new noteOn.
        bool processSample(double bpm, int& activeMidiNote, bool& triggeredNoteOn, bool& triggeredNoteOff) noexcept
        {
            triggeredNoteOn = false;
            triggeredNoteOff = false;

            if (!isEnabled || sequence.empty())
            {
                if (isNoteOn)
                {
                    triggeredNoteOff = true;
                    isNoteOn = false;
                }
                currentNotePlaying = -1;
                return false;
            }

            // Calculate step duration in samples
            double stepDurationSamples = 0.0;
            if (isTempoSynced)
            {
                double safeBpm = std::clamp(bpm, 20.0, 300.0);
                double beatDurationSec = 60.0 / safeBpm;
                double multiplier = 0.25; // default 1/16

                switch (rateSync)
                {
                    case ArpRateSync::Quarter:          multiplier = 1.0; break;
                    case ArpRateSync::Eighth:           multiplier = 0.5; break;
                    case ArpRateSync::EighthTriplet:    multiplier = 0.5 * (2.0 / 3.0); break;
                    case ArpRateSync::Sixteenth:        multiplier = 0.25; break;
                    case ArpRateSync::SixteenthTriplet: multiplier = 0.25 * (2.0 / 3.0); break;
                    case ArpRateSync::ThirtySecond:     multiplier = 0.125; break;
                    case ArpRateSync::Count:            break;
                }
                stepDurationSamples = beatDurationSec * multiplier * sampleRate;
            }
            else
            {
                stepDurationSamples = sampleRate / std::max(0.5f, freeRateHz);
            }

            double gateDurationSamples = stepDurationSamples * gateRatio;

            // Trigger step boundary
            if (sampleCounter >= stepDurationSamples || isFirstStep)
            {
                sampleCounter = 0.0;

                if (!isFirstStep)
                {
                    if (mode == ArpMode::Random)
                    {
                        std::uniform_int_distribution<size_t> dist(0, sequence.size() - 1);
                        currentIndex = dist(rng);
                    }
                    else
                    {
                        currentIndex = (currentIndex + 1) % sequence.size();
                    }
                }
                else
                {
                    currentIndex = 0;
                    isFirstStep = false;
                }

                currentNotePlaying = sequence[currentIndex];
                triggeredNoteOn = true;
                isNoteOn = true;
            }
            else if (isNoteOn && sampleCounter >= gateDurationSamples)
            {
                triggeredNoteOff = true;
                isNoteOn = false;
            }

            sampleCounter += 1.0;
            activeMidiNote = currentNotePlaying;
            return isNoteOn;
        }

        bool hasNotes() const noexcept
        {
            return isLatch ? !latchedNotes.empty() : !heldNotes.empty();
        }

    private:
        double sampleRate = 44100.0;
        bool isEnabled = false;
        bool isLatch = false;
        bool allKeysWereReleased = false;
        bool isTempoSynced = true;
        float freeRateHz = 8.0f;
        float gateRatio = 0.85f;
        int octaves = 1;
        ArpMode mode = ArpMode::Up;
        ArpRateSync rateSync = ArpRateSync::Sixteenth;

        std::vector<int> heldNotes;
        std::vector<int> latchedNotes;
        std::vector<int> sequence;
        size_t currentIndex = 0;
        double sampleCounter = 0.0;
        int currentNotePlaying = -1;
        bool isNoteOn = false;
        bool isFirstStep = true;
        std::minstd_rand rng;

        void rebuildSequence()
        {
            const auto& sourceNotes = (isLatch && !latchedNotes.empty()) ? latchedNotes : heldNotes;
            if (sourceNotes.empty())
            {
                sequence.clear();
                currentIndex = 0;
                isFirstStep = true;
                return;
            }

            std::vector<int> baseNotes = sourceNotes;
            if (mode != ArpMode::AsPlayed)
            {
                std::sort(baseNotes.begin(), baseNotes.end());
            }

            sequence.clear();
            for (int oct = 0; oct < octaves; ++oct)
            {
                for (int n : baseNotes)
                {
                    int noteWithOct = n + oct * 12;
                    if (noteWithOct <= 127)
                        sequence.push_back(noteWithOct);
                }
            }

            if (mode == ArpMode::Down)
            {
                std::reverse(sequence.begin(), sequence.end());
            }
            else if (mode == ArpMode::UpDown && sequence.size() > 2)
            {
                size_t originalSize = sequence.size();
                for (int i = static_cast<int>(originalSize) - 2; i > 0; --i)
                {
                    sequence.push_back(sequence[static_cast<size_t>(i)]);
                }
            }

            if (currentIndex >= sequence.size())
                currentIndex = 0;
        }
    };
}
