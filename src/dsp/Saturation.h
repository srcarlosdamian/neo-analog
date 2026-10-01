#pragma once
#include <cmath>
#include <algorithm>

namespace LadderMono
{
    /**
     * Analog Saturation & Overdrive functions
     * Models the Minimoog Model D mixer overload characteristics and output soft clipping.
     */
    namespace Saturation
    {
        /**
         * Fast rational approximation to tanh(x) with high precision (Pade approximant).
         * Zero allocations, no transcendental CPU overhead.
         */
        inline float fastTanh(float x) noexcept
        {
            if (x < -3.0f) return -1.0f;
            if (x > 3.0f) return 1.0f;
            float x2 = x * x;
            return x * (27.0f + x2) / (27.0f + 9.0f * x2);
        }

        /**
         * Symmetrical differential pair mixer overload saturation.
         * Models the discrete transistor differential pair overload when driven past 0 dB.
         * Zero DC offset, rich third and odd harmonic warmth.
         */
        inline float processMixerOverload(float input, float driveGain) noexcept
        {
            float driven = input * driveGain;
            return fastTanh(driven);
        }

        /**
         * Voice stage headroom clipper.
         * Linear and completely uncompressed below 1.0f.
         * Gently cushions extreme voice peaks above 1.0f without harsh distortion.
         */
        inline float processVoiceClip(float input) noexcept
        {
            if (input > 1.0f)
                return 1.0f + 0.25f * fastTanh((input - 1.0f) * 4.0f);
            if (input < -1.0f)
                return -1.0f + 0.25f * fastTanh((input + 1.0f) * 4.0f);
            return input;
        }

        inline float processOutputClip(float input) noexcept
        {
            return processVoiceClip(input);
        }

        /**
         * Transparent analog-modeled master soft limiter.
         * 100% bit-transparent and linear below 0.90 (-0.9 dBFS).
         * Smooth, warm hyperbolic rounding up to 0.985 (-0.13 dBFS).
         * Guarantees MAXIMUM POSSIBLE VOLUME without harsh speaker rattle, DC bias or digital clipping.
         */
        inline float processMasterClip(float input) noexcept
        {
            constexpr float threshold = 0.90f;
            constexpr float margin = 0.085f; // ceiling = 0.985f
            if (input > threshold)
                return threshold + margin * fastTanh((input - threshold) / margin);
            if (input < -threshold)
                return -threshold + margin * fastTanh((input + threshold) / margin);
            return input;
        }
    }
}
