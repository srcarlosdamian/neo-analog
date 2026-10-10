#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>
#include <algorithm>

namespace LadderMono
{
    /**
     * @brief Stereo Spatial Ambient Filter (Cross-Diffusion Resonator & Spatializer)
     * Provides an analog acoustic room / spatial ambient diffusion filter
     * around the monophonic synthesizer core.
     * 100% Real-Time Safe: Fixed circular buffers, no allocations on audio thread.
     */
    class SpatialFilter
    {
    public:
        SpatialFilter() noexcept { reset(); }

        void prepare(double sampleRate) noexcept
        {
            currentSampleRate = sampleRate > 8000.0 ? sampleRate : 44100.0;
            reset();
        }

        void reset() noexcept
        {
            bufL1.fill(0.0f); bufL2.fill(0.0f); bufL3.fill(0.0f);
            bufR1.fill(0.0f); bufR2.fill(0.0f); bufR3.fill(0.0f);
            idxL1 = idxL2 = idxL3 = 0;
            idxR1 = idxR2 = idxR3 = 0;
            dampL = dampR = 0.0f;
            smoothedAmount = 0.0f;
        }

        void setAmount(float newAmount) noexcept
        {
            targetAmount = std::clamp(newAmount, 0.0f, 1.0f);
        }

        /**
         * @brief Process a stereo block of audio through the Spatial Ambient Filter
         */
        void process(float* leftChannel, float* rightChannel, int numSamples) noexcept
        {
            if (targetAmount < 0.0001f && smoothedAmount < 0.0001f)
                return; // 100% dry bypass

            const float smoothAlpha = 0.005f;

            // Prime delay lengths scaled by sample rate
            const float srScale = static_cast<float>(currentSampleRate / 44100.0);
            const size_t lenL1 = static_cast<size_t>(std::clamp(static_cast<int>(149.0f * srScale), 10, static_cast<int>(BUF_SIZE - 1)));
            const size_t lenL2 = static_cast<size_t>(std::clamp(static_cast<int>(373.0f * srScale), 10, static_cast<int>(BUF_SIZE - 1)));
            const size_t lenL3 = static_cast<size_t>(std::clamp(static_cast<int>(719.0f * srScale), 10, static_cast<int>(BUF_SIZE - 1)));

            const size_t lenR1 = static_cast<size_t>(std::clamp(static_cast<int>(167.0f * srScale), 10, static_cast<int>(BUF_SIZE - 1)));
            const size_t lenR2 = static_cast<size_t>(std::clamp(static_cast<int>(397.0f * srScale), 10, static_cast<int>(BUF_SIZE - 1)));
            const size_t lenR3 = static_cast<size_t>(std::clamp(static_cast<int>(773.0f * srScale), 10, static_cast<int>(BUF_SIZE - 1)));

            for (int i = 0; i < numSamples; ++i)
            {
                smoothedAmount += smoothAlpha * (targetAmount - smoothedAmount);
                if (smoothedAmount < 0.0001f)
                    continue;

                float inL = leftChannel[i];
                float inR = rightChannel ? rightChannel[i] : inL;

                // Stage 1: All-pass diffusion left
                float readL1 = bufL1[idxL1];
                float diffusedL1 = -0.6f * inL + readL1;
                bufL1[idxL1] = inL + 0.6f * diffusedL1;
                if (++idxL1 >= lenL1) idxL1 = 0;

                // Stage 1: All-pass diffusion right
                float readR1 = bufR1[idxR1];
                float diffusedR1 = -0.6f * inR + readR1;
                bufR1[idxR1] = inR + 0.6f * diffusedR1;
                if (++idxR1 >= lenR1) idxR1 = 0;

                // Stage 2: Cross-diffusion stage
                float readL2 = bufL2[idxL2];
                float diffusedL2 = -0.5f * (diffusedL1 + 0.25f * readR1) + readL2;
                bufL2[idxL2] = (diffusedL1 + 0.25f * readR1) + 0.5f * diffusedL2;
                if (++idxL2 >= lenL2) idxL2 = 0;

                float readR2 = bufR2[idxR2];
                float diffusedR2 = -0.5f * (diffusedR1 + 0.25f * readL1) + readR2;
                bufR2[idxR2] = (diffusedR1 + 0.25f * readL1) + 0.5f * diffusedR2;
                if (++idxR2 >= lenR2) idxR2 = 0;

                // Stage 3: Resonant spatial comb & high-frequency damping
                float readL3 = bufL3[idxL3];
                dampL = 0.72f * dampL + 0.28f * readL3; // One-pole analog HF shelf damping
                bufL3[idxL3] = diffusedL2 + 0.45f * dampL;
                if (++idxL3 >= lenL3) idxL3 = 0;

                float readR3 = bufR3[idxR3];
                dampR = 0.72f * dampR + 0.28f * readR3;
                bufR3[idxR3] = diffusedR2 + 0.45f * dampR;
                if (++idxR3 >= lenR3) idxR3 = 0;

                // Spatial wet mix (0.0 = 100% dry mono, 1.0 = full immersive ambient stereo)
                float wetL = (readL3 - 0.4f * readR3) * 0.75f;
                float wetR = (readR3 - 0.4f * readL3) * 0.75f;

                leftChannel[i] = inL * (1.0f - smoothedAmount * 0.25f) + wetL * (smoothedAmount * 0.85f);
                if (rightChannel)
                    rightChannel[i] = inR * (1.0f - smoothedAmount * 0.25f) + wetR * (smoothedAmount * 0.85f);
            }
        }

    private:
        static constexpr size_t BUF_SIZE = 4096;
        double currentSampleRate = 44100.0;
        float targetAmount = 0.0f;
        float smoothedAmount = 0.0f;

        std::array<float, BUF_SIZE> bufL1{}, bufL2{}, bufL3{};
        std::array<float, BUF_SIZE> bufR1{}, bufR2{}, bufR3{};
        size_t idxL1 = 0, idxL2 = 0, idxL3 = 0;
        size_t idxR1 = 0, idxR2 = 0, idxR3 = 0;
        float dampL = 0.0f, dampR = 0.0f;
    };
}
