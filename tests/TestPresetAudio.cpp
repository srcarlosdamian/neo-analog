#include <iostream>
#include <iomanip>
#include "../src/PluginProcessor.h"

int main()
{
    LadderMonoAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);
    auto& pm = processor.getPresetManager();
    auto& apvts = processor.getAPVTS();

    auto testPresetSound = [&](int index) {
        processor.releaseResources();
        processor.prepareToPlay(44100.0, 512);
        pm.loadPreset(index);
        auto name = pm.getCurrentPresetName();

        juce::AudioBuffer<float> buffer(2, 512);
        juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);

        float maxSample = 0.0f;
        // Process 10 blocks (approx 120 ms)
        for (int b = 0; b < 10; ++b) {
            buffer.clear();
            processor.processBlock(buffer, midi);
            midi.clear(); // only noteOn on first block

            for (int s = 0; s < 512; ++s) {
                maxSample = std::max(maxSample, std::abs(buffer.getSample(0, s)));
            }
        }

        std::cout << "[" << std::setw(2) << index << "] " << std::setw(30) << std::left << name
                  << " Peak: " << std::fixed << std::setprecision(4) << maxSample;

        if (maxSample < 0.001f) {
            std::cout << "  <--- SILENT! *** ERROR ***";
        }
        std::cout << "\n";
    };

    std::cout << "Testing all presets with noteOn(60):\n";
    for (size_t i = 0; i < pm.getPresets().size(); ++i) {
        testPresetSound(static_cast<int>(i));
    }

    return 0;
}
