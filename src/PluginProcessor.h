#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/Voice.h"
#include "params/ParameterIDs.h"
#include "presets/PresetManager.h"

class LadderMonoAudioProcessor : public juce::AudioProcessor
{
public:
    LadderMonoAudioProcessor();
    ~LadderMonoAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "LadderMono"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    LadderMono::PresetManager& getPresetManager() noexcept { return presetManager; }
    LadderMono::Voice& getVoice() noexcept { return voice; }

    // Direct UI note audition
    void triggerNoteOn(int note, float vel) { voice.noteOn(note, vel); }
    void triggerNoteOff(int note) { voice.noteOff(note); }

private:
    juce::AudioProcessorValueTreeState apvts;
    LadderMono::Voice voice;
    LadderMono::PresetManager presetManager;

    // Smoothed parameters
    juce::SmoothedValue<float> smoothedMasterVol;

    void updateVoiceParameters() noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LadderMonoAudioProcessor)
};
