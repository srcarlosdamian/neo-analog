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

    enum class ArpPattern
    {
        Straight = 0,
        Dotted,
        Syncopated,
        Euclid3_8,
        Euclid5_8,
        Count
    };

    enum class ArpOctMode
    {
        Ascend = 0,
        Alternate,
        Spiral,
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
            stepCount = 0;
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

        void setOctaveMode(ArpOctMode om) noexcept
        {
            if (octMode != om)
            {
                octMode = om;
                rebuildSequence();
            }
        }

        void setRateSync(ArpRateSync r) noexcept { rateSync = r; }
        void setFreeRateHz(float hz) noexcept { freeRateHz = std::clamp(hz, 0.5f, 30.0f); }
        void setSyncMode(bool sync) noexcept { isTempoSynced = sync; }
        void setGate(float g) noexcept { gateRatio = std::clamp(g, 0.05f, 1.0f); }
        void setSwing(float swingPct) noexcept { swingAmount = std::clamp(swingPct, 50.0f, 75.0f) / 100.0f; }
        void setPattern(ArpPattern p) noexcept { pattern = p; }
        void setMutate(float mutPct) noexcept { mutateChance = std::clamp(mutPct, 0.0f, 100.0f) / 100.0f; }

        void noteOn(int note)
        {
            if (std::find(heldNotes.begin(), heldNotes.end(), note) == heldNotes.end())
            {
                heldNotes.push_back(note);
            }

            if (isLatch)
            {
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

            // Calculate base step duration in samples
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

            // Apply Swing / Shuffle timing
            double currentStepDuration = stepDurationSamples;
            if (swingAmount > 0.501f)
            {
                double swingDelta = (swingAmount - 0.5) * 0.60 * stepDurationSamples;
                currentStepDuration = (stepCount % 2 == 1) ? (stepDurationSamples - swingDelta) : (stepDurationSamples + swingDelta);
            }

            double gateDurationSamples = currentStepDuration * gateRatio;

            // Trigger step boundary
            if (sampleCounter >= currentStepDuration || isFirstStep)
            {
                sampleCounter = 0.0;

                if (!isFirstStep)
                {
                    stepCount++;
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
                    stepCount = 0;
                    isFirstStep = false;
                }

                // Check rhythm pattern mask (Rests / Ties)
                bool stepAllowed = true;
                if (pattern == ArpPattern::Dotted)
                {
                    const int mask[8] = {1, 0, 1, 1, 0, 1, 1, 0};
                    stepAllowed = (mask[stepCount % 8] != 0);
                }
                else if (pattern == ArpPattern::Syncopated)
                {
                    const int mask[8] = {1, 0, 0, 1, 0, 1, 0, 0};
                    stepAllowed = (mask[stepCount % 8] != 0);
                }
                else if (pattern == ArpPattern::Euclid3_8)
                {
                    const int mask[8] = {1, 0, 0, 1, 0, 0, 1, 0};
                    stepAllowed = (mask[stepCount % 8] != 0);
                }
                else if (pattern == ArpPattern::Euclid5_8)
                {
                    const int mask[8] = {1, 0, 1, 1, 0, 1, 1, 0};
                    stepAllowed = (mask[stepCount % 8] != 0);
                }

                if (stepAllowed)
                {
                    int noteToPlay = sequence[currentIndex];

                    // Mutate chance: probabilistic octave displacement
                    if (mutateChance > 0.01f)
                    {
                        std::uniform_real_distribution<float> prob(0.0f, 1.0f);
                        if (prob(rng) < mutateChance)
                        {
                            int octOffset = (prob(rng) > 0.5f) ? 12 : -12;
                            int mutated = noteToPlay + octOffset;
                            if (mutated >= 12 && mutated <= 127)
                                noteToPlay = mutated;
                        }
                    }

                    currentNotePlaying = noteToPlay;
                    triggeredNoteOn = true;
                    isNoteOn = true;
                }
                else
                {
                    if (isNoteOn)
                    {
                        triggeredNoteOff = true;
                        isNoteOn = false;
                    }
                }
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
        float swingAmount = 0.50f; // 50% = straight
        float mutateChance = 0.0f; // 0% to 100%
        int octaves = 1;
        ArpMode mode = ArpMode::Up;
        ArpRateSync rateSync = ArpRateSync::Sixteenth;
        ArpPattern pattern = ArpPattern::Straight;
        ArpOctMode octMode = ArpOctMode::Ascend;

        std::vector<int> heldNotes;
        std::vector<int> latchedNotes;
        std::vector<int> sequence;
        size_t currentIndex = 0;
        uint32_t stepCount = 0;
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
                stepCount = 0;
                isFirstStep = true;
                return;
            }

            std::vector<int> baseNotes = sourceNotes;
            if (mode != ArpMode::AsPlayed)
            {
                std::sort(baseNotes.begin(), baseNotes.end());
            }

            sequence.clear();

            if (octMode == ArpOctMode::Alternate)
            {
                // Alternate note by note across octaves (Note 1 Oct 0, Note 1 Oct 1..., Note 2...)
                for (int n : baseNotes)
                {
                    for (int oct = 0; oct < octaves; ++oct)
                    {
                        int noteWithOct = n + oct * 12;
                        if (noteWithOct <= 127)
                            sequence.push_back(noteWithOct);
                    }
                }
            }
            else if (octMode == ArpOctMode::Spiral)
            {
                // Ascends octaves then descends octaves
                for (int oct = 0; oct < octaves; ++oct)
                {
                    for (int n : baseNotes)
                    {
                        int noteWithOct = n + oct * 12;
                        if (noteWithOct <= 127)
                            sequence.push_back(noteWithOct);
                    }
                }
                if (octaves > 2)
                {
                    for (int oct = octaves - 2; oct > 0; --oct)
                    {
                        for (int n : baseNotes)
                        {
                            int noteWithOct = n + oct * 12;
                            if (noteWithOct <= 127)
                                sequence.push_back(noteWithOct);
                        }
                    }
                }
            }
            else
            {
                // Normal Ascend
                for (int oct = 0; oct < octaves; ++oct)
                {
                    for (int n : baseNotes)
                    {
                        int noteWithOct = n + oct * 12;
                        if (noteWithOct <= 127)
                            sequence.push_back(noteWithOct);
                    }
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
