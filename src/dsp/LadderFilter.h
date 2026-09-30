#pragma once
#include <juce_core/juce_core.h>
#include <cmath>

namespace LadderMono
{
    enum class FilterModel
    {
        SaturatedZDF = 0, // Default authentic DAFx-26 / Huovilainen nonlinear ladder
        CleanZDF,         // Linear zero-delay feedback ladder
        Huovilainen       // DAFx-04 style
    };

    class LadderFilter
    {
    public:
        LadderFilter() = default;

        void prepare(double sampleRate) noexcept;
        void reset() noexcept;

        // Process a single sample
        // input: input signal
        // cutoffHz: modulated cutoff in Hz
        // resonance: 0.0 to 10.0 (self-oscillation near 9.0 - 10.0)
        // drive: input overdrive factor (1.0 = standard, > 1.0 = fat drive)
        // bassCompensation: 0.0 (authentic passband dip) to 1.0 (full bass compensation)
        float processSample(float input, float cutoffHz, float resonance, float drive, float bassCompensation) noexcept;

        void setModel(FilterModel m) noexcept { model = m; }

    private:
        double sampleRate = 44100.0;
        FilterModel model = FilterModel::SaturatedZDF;

        // State variables for 4 stages (2x oversampled states)
        float s[4] = {0.0f, 0.0f, 0.0f, 0.0f};

        // Half-band filter states for 2x oversampling downsampling
        float downsampleState = 0.0f;

        static inline float fastTanh(float x) noexcept
        {
            // Fast rational approximation to tanh(x) with high precision
            if (x < -3.0f) return -1.0f;
            if (x > 3.0f) return 1.0f;
            float x2 = x * x;
            return x * (27.0f + x2) / (27.0f + 9.0f * x2);
        }

        // Inner oversampled step (at 2x sample rate)
        float processInner(float input, float g, float k, float drive, float bassComp) noexcept;
    };
}
