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
    static constexpr int kMaxVoices = 8;
    LadderMono::Voice& getVoice() noexcept { return voices[0]; }
    std::array<LadderMono::Voice, kMaxVoices>& getVoices() noexcept { return voices; }

    // Direct UI note audition and MIDI note handling
    void triggerNoteOn(int note, float vel) noexcept { handleNoteOn(note, vel); }
    void triggerNoteOff(int note) noexcept { handleNoteOff(note); }

    void handleNoteOn(int noteNumber, float velocity) noexcept;
    void handleNoteOff(int noteNumber) noexcept;
    void handleAllNotesOff() noexcept;

private:
    juce::AudioProcessorValueTreeState apvts;
    std::array<LadderMono::Voice, kMaxVoices> voices;
    std::array<uint32_t, kMaxVoices> voiceAge = {0};
    uint32_t voiceCounter = 0;
    LadderMono::PresetManager presetManager;

    // Smoothed parameters
    juce::SmoothedValue<float> smoothedMasterVol;

    void updateVoiceParameters() noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LadderMonoAudioProcessor)
};
