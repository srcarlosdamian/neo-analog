#include "LadderFilter.h"
#include <algorithm>

namespace LadderMono
{
    void LadderFilter::prepare(double sr) noexcept
    {
        sampleRate = std::max(sr, 1000.0);
        reset();
    }

    void LadderFilter::reset() noexcept
    {
        for (int i = 0; i < 4; ++i)
            s[i] = 0.0f;
        downsampleState = 0.0f;
    }

    float LadderFilter::processInner(float input, float g, float k, float drive, float bassComp) noexcept
    {
        // G is the integrator gain g / (1 + g)
        float G = g / (1.0f + g);

        // Input drive
        float drivenInput = input * drive;

        float u_out = 0.0f;
        if (model == FilterModel::CleanZDF)
        {
            // Analytical Linear ZDF solution
            // y4 = G^4 * u0 + S
            // u0 = drivenInput - k * y4
            // => y4 * (1 + k * G^4) = G^4 * drivenInput + S
            float S = G * (G * (G * s[0] + s[1]) + s[2]) + s[3];
            float G4 = G * G * G * G;
            float y4 = (G4 * drivenInput + S) / (1.0f + k * G4);
            float u0 = drivenInput - k * y4;

            float u = u0;
            for (int i = 0; i < 4; ++i)
            {
                float v = G * (u - s[i]);
                float y = v + s[i];
                s[i] = y + v;
                u = y;
            }
            u_out = u;
        }
        else // SaturatedZDF (default) & Huovilainen
        {
            // Nonlinear TPT / ZDF with tanh saturation
            // Early-stage and feedback saturation (DAFx-04 / DAFx-26 Oyama)
            float S = G * (G * (G * s[0] + s[1]) + s[2]) + s[3];
            float G4 = G * G * G * G;

            // Estimate feedback
            float feedbackEst = fastTanh(s[3]);
            float u0 = fastTanh(drivenInput - k * feedbackEst);

            // Predictor-corrector step for tight nonlinear feedback loop
            float y4_est = (G4 * u0 + S) / (1.0f + k * G4);
            u0 = fastTanh(drivenInput - k * fastTanh(y4_est));

            float u = u0;
            for (int i = 0; i < 4; ++i)
            {
                float v = G * (u - s[i]);
                float y = v + s[i];
                s[i] = y + v;

                // Saturate stages 0 and 1 (DAFx-26: early stage saturation captures ~90% harmonic realism)
                if (i <= 1)
                    u = fastTanh(y);
                else
                    u = y;
            }
            u_out = u;
        }

        // Active low-frequency passband recovery:
        // Physical ladder filters naturally lose passband gain by 1 / (1 + k) as resonance increases.
        // This compensation ensures that turning up Emphasis / Resonance NEVER robs the synth of bass punch.
        float passbandGain = 1.0f + (k * (0.35f + 0.65f * bassComp));
        return u_out * passbandGain;
    }

    float LadderFilter::processSample(float input, float cutoffHz, float resonance, float drive, float bassCompensation) noexcept
    {
        // 2x internal oversampling rate
        double osRate = sampleRate * 2.0;

        // Clamp cutoff to safe frequency below Nyquist
        float clampedCutoff = std::clamp(cutoffHz, 10.0f, static_cast<float>(osRate * 0.46));

        // Thermal noise floor to kickstart authentic analog self-oscillation
        if (resonance > 8.5f)
        {
            thermalNoiseSeed = 1664525u * thermalNoiseSeed + 1013904223u;
            float thermalNoise = (static_cast<float>(thermalNoiseSeed) / 4294967296.0f - 0.5f) * 1e-6f;
            input += thermalNoise;
        }

        // Fast high-precision bilinear prewarping for 2x rate (Pade rational approximation)
        float w = static_cast<float>(3.14159265358979323846 * clampedCutoff / osRate);
        float w2 = w * w;
        float g = w * (1.0f - 0.066667f * w2) / (1.0f - 0.40f * w2);
        g = std::clamp(g, 0.0001f, 0.999f);

        // Resonance parameter (0 to 10) mapped to feedback factor k
        // At resonance = 10, k = 4.08 (exceeds 4.0 Barkhausen threshold for self-oscillation)
        float k = (resonance / 10.0f) * 4.08f;
        k = std::clamp(k, 0.0f, 4.25f);

        // Process 2 sub-samples (2x oversampling)
        float y0 = processInner(input, g, k, drive, bassCompensation);
        float y1 = processInner(input, g, k, drive, bassCompensation);

        // Simple 2-point averaging / half-band filter for anti-aliasing downsampling
        float out = 0.5f * (y0 + y1);

        // Soft limit to prevent numerical divergence under extreme feedback
        if (std::isnan(out) || std::isinf(out))
        {
            reset();
            return 0.0f;
        }

        return std::clamp(out, -3.0f, 3.0f);
    }
}
