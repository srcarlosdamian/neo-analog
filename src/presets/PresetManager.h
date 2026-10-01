#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

namespace LadderMono
{
    struct Preset
    {
        juce::String name;
        juce::String category;  // Collection / Bank / Pack (e.g. "Basics", "French Touch", etc.)
        juce::String soundType; // Primary Instrument Type (e.g. "Bass", "Lead", "Keys", etc.)
        juce::String author;
        std::vector<juce::String> tags;
        std::map<juce::String, float> params;
    };

    class PresetManager
    {
    public:
        explicit PresetManager(juce::AudioProcessorValueTreeState& apvts);

        void loadFactoryPresets(const juce::String& jsonContent);
        const std::vector<Preset>& getPresets() const noexcept { return presets; }

        int getCurrentPresetIndex() const noexcept { return currentPresetIndex; }
        juce::String getCurrentPresetName() const noexcept;

        void loadPreset(int index);
        void loadNextPreset();
        void loadPrevPreset();
        void initPatch();
        void saveUserPreset(const juce::String& name, const juce::String& category);

        void setOnPresetChanged(std::function<void()> cb) { onPresetChanged = std::move(cb); }

    private:
        juce::AudioProcessorValueTreeState& apvts;
        std::vector<Preset> presets;
        int currentPresetIndex = 0;
        std::function<void()> onPresetChanged;

        void applyPreset(const Preset& p);
    };
}
