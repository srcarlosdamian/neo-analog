#pragma once
#include <juce_core/juce_core.h>

namespace LadderMono
{
    // Waveform choices
    enum class Waveform
    {
        Triangle = 0,
        Sharkfin,
        Sawtooth,
        Square,
        WidePulse,
        NarrowPulse,
        Count
    };

    // Range choices in feet
    enum class Range
    {
        LO = 0,
        Foot32,
        Foot16,
        Foot8,
        Foot4,
        Foot2,
        Count
    };

    class Oscillator
    {
    public:
        Oscillator() = default;

        void prepare(double sampleRate) noexcept;
        void reset() noexcept;

        // Note: keyboardFreq is in Hz (typically calculated from MIDI note + bend + tune)
        // returns single sample
        float process(float baseFreqHz, float modPitchSemitones, float driftCents) noexcept;

        void setWaveform(Waveform wave) noexcept { currentWaveform = wave; }
        void setRange(Range range) noexcept { currentRange = range; }
        void setFineTuneSemitones(float st) noexcept { fineTuneSt = st; }
        void setPhaseReset(bool reset) noexcept { phaseResetOnNote = reset; }
        void triggerNote() noexcept;

        void setKeyboardTracking(bool track) noexcept { kbdTrack = track; }

        static float polyBLEP(float t, float dt) noexcept;
        static float polyBLAMP(float t, float dt) noexcept;

    private:
        double sampleRate = 44100.0;
        double phase = 0.0;
        Waveform currentWaveform = Waveform::Sawtooth;
        Range currentRange = Range::Foot8;
        float fineTuneSt = 0.0f;
        bool phaseResetOnNote = false;
        bool kbdTrack = true;

        float computeOctaveMultiplier() const noexcept;
    };
}
