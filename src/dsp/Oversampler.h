#pragma once
#include <juce_dsp/juce_dsp.h>
#include <memory>

namespace LadderMono
{
    /**
     * Oversampling wrapper utilizing juce::dsp::Oversampling
     * Supports 2x and 4x internal oversampling for anti-aliasing in nonlinear processing.
     */
    template <typename SampleType = float>
    class Oversampler
    {
    public:
        enum class Factor
        {
            None = 0,
            TwoX = 1,
            FourX = 2
        };

        explicit Oversampler(Factor factor = Factor::TwoX, size_t numChannels = 1)
            : currentFactor(factor), channels(numChannels)
        {
            setFactor(currentFactor);
        }

        void prepare(double baseSampleRate, size_t maxBlockSize)
        {
            sampleRate = baseSampleRate;
            blockSize = maxBlockSize;
            if (oversamplingEngine != nullptr)
                oversamplingEngine->initProcessing(maxBlockSize);
        }

        void reset()
        {
            if (oversamplingEngine != nullptr)
                oversamplingEngine->reset();
        }

        void setFactor(Factor factor)
        {
            currentFactor = factor;
            if (currentFactor == Factor::None)
            {
                oversamplingEngine.reset();
            }
            else
            {
                size_t factorIndex = (currentFactor == Factor::TwoX) ? 1 : 2;
                oversamplingEngine = std::make_unique<juce::dsp::Oversampling<SampleType>>(
                    channels, factorIndex, juce::dsp::Oversampling<SampleType>::filterHalfBandPolyphaseIIR, true);
                if (blockSize > 0)
                    oversamplingEngine->initProcessing(blockSize);
            }
        }

        juce::dsp::AudioBlock<SampleType> processSamplesUp(const juce::dsp::AudioBlock<const SampleType>& inputBlock) noexcept
        {
            if (oversamplingEngine != nullptr)
                return oversamplingEngine->processSamplesUp(inputBlock);
            return juce::dsp::AudioBlock<SampleType>(const_cast<SampleType**>(inputBlock.getChannelPointers()),
                                                      inputBlock.getNumChannels(),
                                                      inputBlock.getNumSamples());
        }

        void processSamplesDown(juce::dsp::AudioBlock<SampleType>& outputBlock) noexcept
        {
            if (oversamplingEngine != nullptr)
                oversamplingEngine->processSamplesDown(outputBlock);
        }

        Factor getFactor() const noexcept { return currentFactor; }
        double getOversampledRate() const noexcept
        {
            if (currentFactor == Factor::TwoX) return sampleRate * 2.0;
            if (currentFactor == Factor::FourX) return sampleRate * 4.0;
            return sampleRate;
        }

    private:
        Factor currentFactor = Factor::TwoX;
        size_t channels = 1;
        double sampleRate = 44100.0;
        size_t blockSize = 512;
        std::unique_ptr<juce::dsp::Oversampling<SampleType>> oversamplingEngine;
    };
}
