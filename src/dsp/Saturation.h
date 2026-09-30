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
         * Output stage soft-clipper to prevent harsh digital rail clipping.
         */
        inline float processOutputClip(float input) noexcept
        {
            return std::tanh(input * 0.9f);
        }
    }
}
