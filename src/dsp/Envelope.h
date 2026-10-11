#pragma once
#include <cmath>
#include <algorithm>

namespace LadderMono
{
    enum class EnvelopeStage
    {
        Idle = 0,
        Attack,
        Decay,
        Sustain,
        Release
    };

    class Envelope
    {
    public:
        Envelope() = default;

        void prepare(double sr) noexcept
        {
            sampleRate = std::max(sr, 1000.0);
            reset();
        }

        void reset() noexcept
        {
            stage = EnvelopeStage::Idle;
            currentLevel = 0.0f;
        }

        void startAttack() noexcept
        {
            stage = EnvelopeStage::Attack;
            // Retrigger starts smoothly from current level, preventing clicks!
        }

        void startRelease(bool decayAsRelease) noexcept
        {
            if (stage != EnvelopeStage::Idle)
            {
                useDecayRelease = decayAsRelease;
                stage = EnvelopeStage::Release;
            }
        }

        // Panel values 0 to 10
        // Attack: 1 ms to 10 s (exponential/log mapping)
        // Decay: 1 ms to 10 s
        // Sustain: 0 to 10 (0.0 to 1.0)
        void setParameters(float aParam, float dParam, float sParam) noexcept
        {
            attackTimeSec = attackParamToSeconds(aParam);
            decayTimeSec  = decayParamToSeconds(dParam);
            sustainLevel  = std::clamp(sParam / 10.0f, 0.0f, 1.0f);
        }

        float getLevel() const noexcept { return currentLevel; }
        bool isActive() const noexcept { return stage != EnvelopeStage::Idle; }

        float processSample() noexcept
        {
            switch (stage)
            {
                case EnvelopeStage::Idle:
                    currentLevel = 0.0f;
                    break;

                case EnvelopeStage::Attack:
                {
                    // Analog-style RC attack curve
                    float rate = 1.0f / std::max(1.0f, attackTimeSec * static_cast<float>(sampleRate) * 0.75f);
                    currentLevel += rate * (1.15f - currentLevel);
                    if (currentLevel >= 1.0f)
                    {
                        currentLevel = 1.0f;
                        stage = EnvelopeStage::Decay;
                    }
                    break;
                }

                case EnvelopeStage::Decay:
                {
                    // Exponential decay toward sustain level (reaches within 1.5% in decayTimeSec)
                    float rate = 1.0f / std::max(1.0f, decayTimeSec * static_cast<float>(sampleRate) * 0.25f);
                    currentLevel += (sustainLevel - currentLevel) * rate;
                    if (std::abs(currentLevel - sustainLevel) < 0.0005f)
                    {
                        currentLevel = sustainLevel;
                        stage = EnvelopeStage::Sustain;
                    }
                    break;
                }

                case EnvelopeStage::Sustain:
                {
                    currentLevel = sustainLevel;
                    break;
                }

                case EnvelopeStage::Release:
                {
                    // If decayAsRelease is on, use decayTimeSec; else fast 12 ms release
                    float relTime = useDecayRelease ? decayTimeSec : 0.012f;
                    float rate = 1.0f / std::max(1.0f, relTime * static_cast<float>(sampleRate) * 0.20f);
                    currentLevel -= currentLevel * rate;
                    // Dynamic Voice Sleep / Voice-Kill at -90 dB (0.0000316)
                    // Drops active voice computing instantly when inaudible for true 0% idle CPU
                    if (currentLevel <= 0.000032f)
                    {
                        currentLevel = 0.0f;
                        stage = EnvelopeStage::Idle;
                    }
                    break;
                }
            }

            return currentLevel;
        }

    private:
        double sampleRate = 44100.0;
        EnvelopeStage stage = EnvelopeStage::Idle;
        float currentLevel = 0.0f;
        float attackTimeSec = 0.01f;
        float decayTimeSec = 0.5f;
        float sustainLevel = 1.0f;
        bool useDecayRelease = false;

        static inline float attackParamToSeconds(float param) noexcept
        {
            // Model D specification: Attack time 1 ms to 10 s
            // 0 -> 1.5 ms (lightning fast analog bite and punch without digital click)
            // 2 -> 60 ms, 5 -> 700 ms, 8 -> 3.0 s, 10 -> 6.0 s
            float norm = std::clamp(param / 10.0f, 0.0f, 1.0f);
            return 0.0015f + 5.9985f * std::pow(norm, 2.4f);
        }

        static inline float decayParamToSeconds(float param) noexcept
        {
            // Model D specification: Decay / Release time 10 ms to 10 s
            // 0 -> 10 ms (punchy tight analog snap without abrupt cutoff)
            // 2 -> 70 ms, 5 -> 750 ms, 8 -> 3.2 s, 10 -> 6.0 s
            float norm = std::clamp(param / 10.0f, 0.0f, 1.0f);
            return 0.010f + 5.990f * std::pow(norm, 2.4f);
        }
    };
}
