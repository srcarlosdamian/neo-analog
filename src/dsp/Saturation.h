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
         * Asymmetrical mixer overload saturation with mild second harmonic generation.
         * Models the discrete transistor differential pair overload when driven past 0 dB.
         */
        inline float processMixerOverload(float input, float driveGain) noexcept
        {
            float driven = input * driveGain;
            // Asymmetric second harmonic term (+0.04 * driven^2) produces rich analog warmth
            return std::tanh(driven + 0.04f * driven * driven);
        }

        /**
         * Output stage soft-clipper / warm analog limiter.
         * Linear and completely transparent below 0.65 (-3.7 dBFS),
         * with smooth C1 tanh saturation approaching 1.0 (0 dBFS).
         * Prevents harsh digital rail clipping while preserving punch, volume and dynamics.
         */
        inline float processOutputClip(float input) noexcept
        {
            constexpr float threshold = 0.65f;
            constexpr float margin = 1.0f - threshold; // 0.35f
            if (input > threshold)
                return threshold + margin * std::tanh((input - threshold) / margin);
            if (input < -threshold)
                return -threshold + margin * std::tanh((input + threshold) / margin);
            return input;
        }
    }
}
