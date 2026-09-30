#pragma once
#include <cmath>
#include <algorithm>

namespace LadderMono
{
    enum class GlideMode
    {
        Always = 0,
        LegatoOnly
    };

    class Glide
    {
    public:
        Glide() = default;

        void prepare(double sr) noexcept
        {
            sampleRate = std::max(sr, 1000.0);
            reset();
        }

        void reset() noexcept
        {
            currentMidiNote = targetMidiNote = 60.0f;
        }

        void setGlideTime(float param0to10) noexcept
        {
            // Panel 0-10 -> 0.001s to 5.0s exponential
            float norm = std::clamp(param0to10 / 10.0f, 0.0f, 1.0f);
            glideTimeSec = 0.001f + 4.999f * std::pow(norm, 2.5f);
        }

        void setMode(GlideMode m) noexcept { mode = m; }
        void setEnabled(bool enabled) noexcept { isEnabled = enabled; }

        void setTargetNote(int midiNote, bool isLegatoTransition) noexcept
        {
            targetMidiNote = static_cast<float>(midiNote);

            bool shouldGlide = isEnabled && (mode == GlideMode::Always || (mode == GlideMode::LegatoOnly && isLegatoTransition));
            if (!shouldGlide)
            {
                currentMidiNote = targetMidiNote;
            }
        }

        float processSample() noexcept
        {
            if (std::abs(currentMidiNote - targetMidiNote) < 0.001f)
            {
                currentMidiNote = targetMidiNote;
                return currentMidiNote;
            }

            // Exponential convergence: constant-time slide
            float rate = 1.0f / (glideTimeSec * static_cast<float>(sampleRate) * 0.25f);
            currentMidiNote += (targetMidiNote - currentMidiNote) * rate;
            return currentMidiNote;
        }

        float getCurrentNote() const noexcept { return currentMidiNote; }

    private:
        double sampleRate = 44100.0;
        float currentMidiNote = 60.0f;
        float targetMidiNote = 60.0f;
        float glideTimeSec = 0.05f;
        bool isEnabled = false;
        GlideMode mode = GlideMode::Always;
    };
}
