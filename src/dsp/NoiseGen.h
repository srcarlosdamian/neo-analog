#pragma once
#include <random>
#include <cmath>

namespace LadderMono
{
    enum class NoiseColor
    {
        White = 0,
        Pink
    };

    class NoiseGen
    {
    public:
        NoiseGen() : rng(1337), dist(-1.0f, 1.0f) {}

        void reset(unsigned int seed = 1337)
        {
            rng.seed(seed);
            b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0.0f;
        }

        float process(NoiseColor color) noexcept
        {
            float white = dist(rng);
            if (color == NoiseColor::White)
            {
                return white;
            }

            // Paul Kellet's filtered pink noise algorithm
            // Produces an accurate -3 dB/octave slope
            b0 = 0.99886f * b0 + white * 0.0555179f;
            b1 = 0.99332f * b1 + white * 0.0750759f;
            b2 = 0.96900f * b2 + white * 0.1538520f;
            b3 = 0.86650f * b3 + white * 0.3104856f;
            b4 = 0.55000f * b4 + white * 0.5329522f;
            b5 = -0.7616f * b5 - white * 0.0168980f;
            float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
            b6 = white * 0.115926f;

            // Scale to match White Noise perceived loudness and RMS (~0.55)
            return std::clamp(pink * 0.30f, -1.0f, 1.0f);
        }

    private:
        std::minstd_rand rng;
        std::uniform_real_distribution<float> dist;
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, b3 = 0.0f, b4 = 0.0f, b5 = 0.0f, b6 = 0.0f;
    };
}
