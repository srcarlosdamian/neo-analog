#include <iostream>
#include "../src/PluginProcessor.h"

int main()
{
    LadderMonoAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);
    auto& pm = processor.getPresetManager();

    // Find Marimba-ish
    for (size_t i = 0; i < pm.getPresets().size(); ++i) {
        if (pm.getPresets()[i].name == "Marimba-ish") {
            pm.loadPreset(static_cast<int>(i));
            break;
        }
    }

    std::cout << "Marimba-ish parameters:\n";
    auto& apvts = processor.getAPVTS();
    for (const char* pid : {
        LadderMono::ParamIDs::mixOsc1,
        LadderMono::ParamIDs::mixOsc1On,
        LadderMono::ParamIDs::mixOsc2,
        LadderMono::ParamIDs::mixOsc2On,
        LadderMono::ParamIDs::cutoff,
        LadderMono::ParamIDs::emphasis,
        LadderMono::ParamIDs::contourAmount,
        LadderMono::ParamIDs::kbd1,
        LadderMono::ParamIDs::kbd2,
        LadderMono::ParamIDs::fAttack,
        LadderMono::ParamIDs::fDecay,
        LadderMono::ParamIDs::fSustain,
        LadderMono::ParamIDs::aAttack,
        LadderMono::ParamIDs::aDecay,
        LadderMono::ParamIDs::aSustain
    }) {
        auto* p = apvts.getRawParameterValue(pid);
        std::cout << "  " << pid << " = " << (p ? p->load() : -1) << "\n";
    }

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);

    for (int b = 0; b < 4; ++b) {
        buffer.clear();
        processor.processBlock(buffer, midi);
        midi.clear();
        float bPeak = 0.0f;
        for (int s = 0; s < 512; ++s) {
            bPeak = std::max(bPeak, std::abs(buffer.getSample(0, s)));
        }
        std::cout << "Block " << b << " peak: " << bPeak 
                  << " | AmpEnv level: " << processor.getVoice().getAmpEnv().getLevel()
                  << " | FilterEnv level: " << processor.getVoice().getFilterEnv().getLevel() << "\n";
    }

    return 0;
}
