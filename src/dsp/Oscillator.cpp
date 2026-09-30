#include "Oscillator.h"
#include <cmath>
#include <algorithm>

namespace LadderMono
{
    float Oscillator::polyBLEP(float t, float dt) noexcept
    {
        if (dt <= 0.0f) return 0.0f;
        if (t < dt)
        {
            float tn = t / dt;
            return tn + tn - tn * tn - 1.0f; // 2*tn - tn^2 - 1
        }
        else if (t > 1.0f - dt)
        {
            float tn = (t - 1.0f) / dt;
            return tn * tn + tn + tn + 1.0f; // tn^2 + 2*tn + 1
        }
        return 0.0f;
    }

    float Oscillator::polyBLAMP(float t, float dt) noexcept
    {
        if (dt <= 0.0f) return 0.0f;
        if (t < dt)
        {
            float tn = t / dt - 1.0f;
            return -1.0f / 3.0f * tn * tn * tn;
        }
        else if (t > 1.0f - dt)
        {
            float tn = (t - 1.0f) / dt + 1.0f;
            return 1.0f / 3.0f * tn * tn * tn;
        }
        return 0.0f;
    }

    void Oscillator::prepare(double sr) noexcept
    {
        sampleRate = std::max(sr, 1000.0);
        phase = 0.0;
    }

    void Oscillator::reset() noexcept
    {
        phase = 0.0;
    }

    void Oscillator::triggerNote() noexcept
    {
        if (phaseResetOnNote)
            phase = 0.0;
    }

    float Oscillator::computeOctaveMultiplier() const noexcept
    {
        switch (currentRange)
        {
            case Range::LO:     return 0.03125f; // sub-audio (~1/32)
            case Range::Foot32: return 0.25f;    // 2 octaves down
            case Range::Foot16: return 0.5f;     // 1 octave down
            case Range::Foot8:  return 1.0f;     // unison
            case Range::Foot4:  return 2.0f;     // 1 octave up
            case Range::Foot2:  return 4.0f;     // 2 octaves up
            default:            return 1.0f;
        }
    }

    float Oscillator::process(float baseFreqHz, float modPitchSemitones, float driftCents) noexcept
    {
        float freqHz = kbdTrack ? baseFreqHz : 440.0f;
        float octMult = computeOctaveMultiplier();
        freqHz *= octMult;

        // Apply fine tune, pitch modulation, and analog drift (in semitones)
        float totalSemitones = fineTuneSt + modPitchSemitones + (driftCents * 0.01f);
        freqHz *= std::pow(2.0f, totalSemitones / 12.0f);

        // Clamp frequency to safe range
        freqHz = std::clamp(freqHz, 0.1f, static_cast<float>(sampleRate * 0.49));
        float dt = static_cast<float>(freqHz / sampleRate);

        float out = 0.0f;
        float t = static_cast<float>(phase);

        switch (currentWaveform)
        {
            case Waveform::Sawtooth:
            {
                // Naive saw from -1 to +1
                out = (2.0f * t) - 1.0f;
                out -= polyBLEP(t, dt);
                break;
            }
            case Waveform::Square:
            {
                // Duty = 0.5
                out = (t < 0.5f) ? 1.0f : -1.0f;
                out += polyBLEP(t, dt);
                float tHalf = t - 0.5f;
                if (tHalf < 0.0f) tHalf += 1.0f;
                out -= polyBLEP(tHalf, dt);
                break;
            }
            case Waveform::WidePulse:
            {
                // Duty ~ 0.75
                constexpr float duty = 0.75f;
                out = (t < duty) ? 1.0f : -1.0f;
                out += polyBLEP(t, dt);
                float tDuty = t - duty;
                if (tDuty < 0.0f) tDuty += 1.0f;
                out -= polyBLEP(tDuty, dt);
                // AC-couple: subtract DC offset of asymmetric pulse and normalize
                out = (out - (2.0f * duty - 1.0f)) / (2.0f * duty);
                break;
            }
            case Waveform::NarrowPulse:
            {
                // Duty ~ 0.85
                constexpr float duty = 0.85f;
                out = (t < duty) ? 1.0f : -1.0f;
                out += polyBLEP(t, dt);
                float tDuty = t - duty;
                if (tDuty < 0.0f) tDuty += 1.0f;
                out -= polyBLEP(tDuty, dt);
                // AC-couple: subtract DC offset of asymmetric pulse and normalize
                out = (out - (2.0f * duty - 1.0f)) / (2.0f * duty);
                break;
            }
            case Waveform::Triangle:
            {
                // Integrated PolyBLEP square or differentiated naive triangle
                // Naive triangle
                out = 2.0f * std::abs(2.0f * (t - std::floor(t + 0.5f))) - 1.0f;
                // PolyBLAMP at 0.0 and 0.5 for smoothed corners
                out += 4.0f * dt * polyBLAMP(t, dt);
                float tHalf = t - 0.5f;
                if (tHalf < 0.0f) tHalf += 1.0f;
                out -= 4.0f * dt * polyBLAMP(tHalf, dt);
                break;
            }
            case Waveform::Sharkfin:
            {
                // Characteristic Minimoog Shark-fin: asymmetric wave, 
                // blend between Triangle and Sawtooth
                float saw = (2.0f * t) - 1.0f - polyBLEP(t, dt);
                float tri = 2.0f * std::abs(2.0f * (t - std::floor(t + 0.5f))) - 1.0f;
                tri += 4.0f * dt * polyBLAMP(t, dt);
                float tHalf = t - 0.5f;
                if (tHalf < 0.0f) tHalf += 1.0f;
                tri -= 4.0f * dt * polyBLAMP(tHalf, dt);

                out = 0.60f * tri + 0.40f * saw;
                break;
            }
            default:
                out = 0.0f;
                break;
        }

        // Advance phase
        phase += dt;
        if (phase >= 1.0)
            phase -= 1.0;

        return out;
    }
}
