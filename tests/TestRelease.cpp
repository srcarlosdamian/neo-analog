#include <iostream>
#include <vector>
#include <cmath>
#include "PluginProcessor.h"

int main()
{
    LadderMonoAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);

    auto& pm = processor.getPresetManager();
    const auto& presets = pm.getPresets();

    std::cout << "Testing note-off release on " << presets.size() << " presets..." << std::endl;

    juce::AudioBuffer<float> buffer(2, 512);

    for (size_t i = 0; i < presets.size(); ++i)
    {
        pm.loadPreset(static_cast<int>(i));

        // Empty block to apply params
        juce::MidiBuffer emptyMidi;
        buffer.clear();
        processor.processBlock(buffer, emptyMidi);

        // Note ON
        juce::MidiBuffer onMidi;
        onMidi.addEvent(juce::MidiMessage::noteOn(1, 60, 0.8f), 0);
        buffer.clear();
        processor.processBlock(buffer, onMidi);

        // Run for 0.5s with note held
        for (int b = 0; b < 40; ++b)
        {
            buffer.clear();
            processor.processBlock(buffer, emptyMidi);
        }

        // Note OFF
        juce::MidiBuffer offMidi;
        offMidi.addEvent(juce::MidiMessage::noteOff(1, 60), 0);
        buffer.clear();
        processor.processBlock(buffer, offMidi);

        // Run for 4.0 seconds in release (approx 344 blocks of 512 samples)
        for (int b = 0; b < 350; ++b)
        {
            buffer.clear();
            processor.processBlock(buffer, emptyMidi);
        }

        // Peak level after 4s release
        float peak = 0.0f;
        for (int s = 0; s < 512; ++s)
        {
            peak = std::max(peak, std::abs(buffer.getSample(0, s)));
        }

        if (peak > 0.0005f)
        {
            std::cout << "[STILL SOUNDING] Preset #" << i << ": " << presets[i].name
                      << " (Category: " << presets[i].category << ") -> Peak after 4s = " << peak << std::endl;
        }
    }

    std::cout << "Done release test." << std::endl;
    return 0;
}
