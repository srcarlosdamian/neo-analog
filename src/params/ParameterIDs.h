#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace LadderMono
{
    namespace ParamIDs
    {
        // Controllers
        inline constexpr auto tune         = "tune";
        inline constexpr auto glideOn      = "glideOn";
        inline constexpr auto glideTime    = "glideTime";
        inline constexpr auto glideMode    = "glideMode";
        inline constexpr auto modMix       = "modMix";
        inline constexpr auto oscModOn     = "oscModOn";
        inline constexpr auto filterModOn  = "filterModOn";
        inline constexpr auto modSource    = "modSource";
        inline constexpr auto lfoRate      = "lfoRate";
        inline constexpr auto lfoShape     = "lfoShape";
        inline constexpr auto bendRange    = "bendRange";

        // Oscillators
        inline constexpr auto osc1Range    = "osc1Range";
        inline constexpr auto osc1Wave     = "osc1Wave";

        inline constexpr auto osc2Range    = "osc2Range";
        inline constexpr auto osc2Wave     = "osc2Wave";
        inline constexpr auto osc2Fine     = "osc2Fine";

        inline constexpr auto osc3Range    = "osc3Range";
        inline constexpr auto osc3Wave     = "osc3Wave";
        inline constexpr auto osc3Fine     = "osc3Fine";
        inline constexpr auto osc3KbdOn    = "osc3KbdOn";

        // Mixer
        inline constexpr auto mixOsc1      = "mixOsc1";
        inline constexpr auto mixOsc1On    = "mixOsc1On";
        inline constexpr auto mixOsc2      = "mixOsc2";
        inline constexpr auto mixOsc2On    = "mixOsc2On";
        inline constexpr auto mixOsc3      = "mixOsc3";
        inline constexpr auto mixOsc3On    = "mixOsc3On";
        inline constexpr auto mixNoise     = "mixNoise";
        inline constexpr auto mixNoiseOn   = "mixNoiseOn";
        inline constexpr auto mixExt       = "mixExt";
        inline constexpr auto mixExtOn     = "mixExtOn";
        inline constexpr auto noiseColor   = "noiseColor";
        inline constexpr auto mixerDrive   = "mixerDrive";

        // Filter
        inline constexpr auto cutoff        = "cutoff";
        inline constexpr auto emphasis      = "emphasis";
        inline constexpr auto contourAmount = "contourAmount";
        inline constexpr auto kbd1          = "kbd1";
        inline constexpr auto kbd2          = "kbd2";
        inline constexpr auto filterModel   = "filterModel";
        inline constexpr auto bassComp      = "bassComp";

        // Envelopes
        inline constexpr auto fAttack       = "fAttack";
        inline constexpr auto fDecay        = "fDecay";
        inline constexpr auto fSustain      = "fSustain";

        inline constexpr auto aAttack       = "aAttack";
        inline constexpr auto aDecay        = "aDecay";
        inline constexpr auto aSustain      = "aSustain";
        inline constexpr auto decaySwitchOn = "decaySwitchOn";

        // Global
        inline constexpr auto masterVol     = "masterVol";
        inline constexpr auto analogAmount  = "analogAmount";
        inline constexpr auto oscPhaseReset = "oscPhaseReset";
        inline constexpr auto hqMode        = "hqMode";
        inline constexpr auto notePriority  = "notePriority";
        inline constexpr auto envMode       = "envMode";
        inline constexpr auto voices        = "voices";

        // Arpeggiator
        inline constexpr auto arpOn         = "arpOn";
        inline constexpr auto arpMode       = "arpMode";
        inline constexpr auto arpOctaves    = "arpOctaves";
        inline constexpr auto arpRateSync   = "arpRateSync";
        inline constexpr auto arpSync       = "arpSync";
        inline constexpr auto arpFreeRate   = "arpFreeRate";
        inline constexpr auto arpGate       = "arpGate";
        inline constexpr auto arpLatch      = "arpLatch";
    }

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
}
