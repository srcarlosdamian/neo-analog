#pragma once
#include <cmath>
#include <algorithm>

namespace LadderMono
{
    /**
     * Parameter Smoothers (per-sample one-pole RC smoother and linear ramp)
     * Prevents zipper noise and zipper transients on real-time controls.
     */
    template <typename FloatType = float>
    class ExponentialSmoother
    {
    public:
        ExponentialSmoother() = default;

        void reset(double sampleRate, double timeSeconds) noexcept
        {
            currentValue = targetValue;
            setTime(sampleRate, timeSeconds);
        }

        void setTime(double sampleRate, double timeSeconds) noexcept
        {
            if (timeSeconds <= 0.0001)
            {
                alpha = 1.0f;
            }
            else
            {
                alpha = static_cast<FloatType>(1.0 - std::exp(-1.0 / (sampleRate * timeSeconds)));
            }
        }

        void setTarget(FloatType target) noexcept
        {
            targetValue = target;
        }

        void setCurrentAndTarget(FloatType val) noexcept
        {
            currentValue = val;
            targetValue = val;
        }

        FloatType getNextValue() noexcept
        {
            currentValue += alpha * (targetValue - currentValue);
            return currentValue;
        }

        FloatType getCurrentValue() const noexcept { return currentValue; }
        FloatType getTargetValue() const noexcept { return targetValue; }

    private:
        FloatType currentValue = static_cast<FloatType>(0);
        FloatType targetValue = static_cast<FloatType>(0);
        FloatType alpha = static_cast<FloatType>(0.05);
    };
}
