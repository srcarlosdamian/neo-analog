#include "ParameterIDs.h"

namespace LadderMono
{
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        // Controllers
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::tune, 1}, "Tune",
            juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel("cents")));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::glideOn, 1}, "Glide", false));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::glideTime, 1}, "Glide Time",
            juce::NormalisableRange<float>(0.0f, 10.0f, 0.05f), 0.5f));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::glideMode, 1}, "Glide Mode",
            juce::StringArray{"Always", "Legato"}, 0));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::modMix, 1}, "Mod Mix",
            juce::NormalisableRange<float>(0.0f, 10.0f, 0.05f), 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::oscModOn, 1}, "Osc Mod", false));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::filterModOn, 1}, "Filter Mod", false));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::modSource, 1}, "Mod Source",
            juce::StringArray{"Osc 3", "LFO"}, 1));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::lfoRate, 1}, "LFO Rate",
            juce::NormalisableRange<float>(0.1f, 30.0f, 0.05f, 0.4f), 5.0f,
            juce::AudioParameterFloatAttributes().withLabel("Hz")));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::lfoShape, 1}, "LFO Shape",
            juce::StringArray{"Triangle", "Square"}, 0));

        params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID{ParamIDs::bendRange, 1}, "Bend Range", 0, 12, 2));

        // Oscillators Range choices: LO, 32', 16', 8', 4', 2'
        const juce::StringArray rangeChoices{"LO", "32'", "16'", "8'", "4'", "2'"};
        const juce::StringArray waveChoices{"Triangle", "Sharkfin", "Sawtooth", "Square", "Wide Pulse", "Narrow Pulse"};

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::osc1Range, 1}, "Osc 1 Range", rangeChoices, 2)); // 16'
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::osc1Wave, 1}, "Osc 1 Wave", waveChoices, 2)); // Saw

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::osc2Range, 1}, "Osc 2 Range", rangeChoices, 3)); // 8'
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::osc2Wave, 1}, "Osc 2 Wave", waveChoices, 2));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::osc2Fine, 1}, "Osc 2 Fine",
            juce::NormalisableRange<float>(-7.0f, 7.0f, 0.01f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel("st")));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::osc3Range, 1}, "Osc 3 Range", rangeChoices, 3));
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::osc3Wave, 1}, "Osc 3 Wave", waveChoices, 2));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::osc3Fine, 1}, "Osc 3 Fine",
            juce::NormalisableRange<float>(-7.0f, 7.0f, 0.01f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel("st")));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::osc3KbdOn, 1}, "Osc 3 Kbd Ctrl", true));

        // Mixer
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::mixOsc1, 1}, "Mix Osc 1", 0.0f, 10.0f, 8.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::mixOsc1On, 1}, "Mix Osc 1 On", true));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::mixOsc2, 1}, "Mix Osc 2", 0.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::mixOsc2On, 1}, "Mix Osc 2 On", false));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::mixOsc3, 1}, "Mix Osc 3", 0.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::mixOsc3On, 1}, "Mix Osc 3 On", false));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::mixNoise, 1}, "Mix Noise", 0.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::mixNoiseOn, 1}, "Mix Noise On", false));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::mixExt, 1}, "Mix Feedback", 0.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::mixExtOn, 1}, "Mix Feedback On", false));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::noiseColor, 1}, "Noise Color",
            juce::StringArray{"White", "Pink"}, 1));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::mixerDrive, 1}, "Mixer Drive",
            juce::NormalisableRange<float>(-6.0f, 12.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")));

        // Filter
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::cutoff, 1}, "Cutoff", -5.0f, 5.0f, 3.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::emphasis, 1}, "Emphasis", 0.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::contourAmount, 1}, "Contour Amount", 0.0f, 10.0f, 3.0f));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::kbd1, 1}, "Keyboard 1 (1/3)", false));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::kbd2, 1}, "Keyboard 2 (2/3)", false));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::filterModel, 1}, "Filter Model",
            juce::StringArray{"Saturated ZDF", "Clean ZDF", "Huovilainen"}, 0));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::bassComp, 1}, "Bass Compensation", 0.0f, 1.0f, 0.25f));

        // Filter Contour
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::fAttack, 1}, "Filter Attack", 0.0f, 10.0f, 0.1f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::fDecay, 1}, "Filter Decay", 0.0f, 10.0f, 5.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::fSustain, 1}, "Filter Sustain", 0.0f, 10.0f, 3.0f));

        // Loudness Contour
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::aAttack, 1}, "Amp Attack", 0.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::aDecay, 1}, "Amp Decay", 0.0f, 10.0f, 5.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::aSustain, 1}, "Amp Sustain", 0.0f, 10.0f, 10.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::decaySwitchOn, 1}, "Decay Switch", true));

        // Global
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::masterVol, 1}, "Master Volume", -60.0f, 6.0f, -6.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::analogAmount, 1}, "Analog Drift", 0.0f, 1.0f, 0.30f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::oscPhaseReset, 1}, "Osc Phase Reset", false));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::hqMode, 1}, "HQ Mode", false));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::notePriority, 1}, "Note Priority",
            juce::StringArray{"Low", "Last", "High"}, 0));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::envMode, 1}, "Envelope Mode",
            juce::StringArray{"Multi-Trigger", "Legato"}, 0));

        // Arpeggiator
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::arpOn, 1}, "Arpeggiator", false));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::arpMode, 1}, "Arp Mode",
            juce::StringArray{"Up", "Down", "Up/Down", "Random", "As Played"}, 0));

        params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID{ParamIDs::arpOctaves, 1}, "Arp Octaves", 1, 4, 1));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{ParamIDs::arpRateSync, 1}, "Arp Sync Rate",
            juce::StringArray{"1/4", "1/8", "1/8 T", "1/16", "1/16 T", "1/32"}, 3)); // 1/16

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::arpSync, 1}, "Arp Tempo Sync", true));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::arpFreeRate, 1}, "Arp Free Rate",
            juce::NormalisableRange<float>(1.0f, 30.0f, 0.1f, 0.4f), 8.0f,
            juce::AudioParameterFloatAttributes().withLabel("Hz")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ParamIDs::arpGate, 1}, "Arp Gate", 0.1f, 1.0f, 0.85f));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ParamIDs::arpLatch, 1}, "Arp Latch", false));

        return {params.begin(), params.end()};
    }
}
