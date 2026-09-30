#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "presets/FactoryPresetsData.h"

LadderMonoAudioProcessor::LadderMonoAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", LadderMono::createParameterLayout()),
      presetManager(apvts)
{
    // Load embedded factory bank
    presetManager.loadFactoryPresets(juce::String::fromUTF8(LadderMono::kFactoryPresetsJSON));
}

LadderMonoAudioProcessor::~LadderMonoAudioProcessor()
{
}

bool LadderMonoAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
     || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo())
    {
        return true;
    }
    return false;
}

void LadderMonoAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    voice.prepare(sampleRate);

    smoothedMasterVol.reset(sampleRate, 0.02);
    float initVolDb = apvts.getRawParameterValue(LadderMono::ParamIDs::masterVol)->load();
    smoothedMasterVol.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(initVolDb));
}

void LadderMonoAudioProcessor::releaseResources()
{
    voice.reset();
}

void LadderMonoAudioProcessor::updateVoiceParameters() noexcept
{
    using namespace LadderMono;

    // Controllers
    voice.setMasterTune(apvts.getRawParameterValue(ParamIDs::tune)->load());
    voice.getGlide().setEnabled(apvts.getRawParameterValue(ParamIDs::glideOn)->load() > 0.5f);
    voice.getGlide().setGlideTime(apvts.getRawParameterValue(ParamIDs::glideTime)->load());
    voice.getGlide().setMode(static_cast<GlideMode>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::glideMode)->load())));
    voice.setModMix(apvts.getRawParameterValue(ParamIDs::modMix)->load());
    voice.setOscModEnabled(apvts.getRawParameterValue(ParamIDs::oscModOn)->load() > 0.5f);
    voice.setFilterModEnabled(apvts.getRawParameterValue(ParamIDs::filterModOn)->load() > 0.5f);
    voice.setModSource(static_cast<ModSource>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::modSource)->load())));
    voice.setLFORate(apvts.getRawParameterValue(ParamIDs::lfoRate)->load());
    voice.setLFOShape(static_cast<LFOShape>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::lfoShape)->load())));

    // Oscillators
    voice.getOsc(0).setRange(static_cast<Range>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc1Range)->load())));
    voice.getOsc(0).setWaveform(static_cast<Waveform>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc1Wave)->load())));

    voice.getOsc(1).setRange(static_cast<Range>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc2Range)->load())));
    voice.getOsc(1).setWaveform(static_cast<Waveform>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc2Wave)->load())));
    voice.getOsc(1).setFineTuneSemitones(apvts.getRawParameterValue(ParamIDs::osc2Fine)->load());

    voice.getOsc(2).setRange(static_cast<Range>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc3Range)->load())));
    voice.getOsc(2).setWaveform(static_cast<Waveform>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc3Wave)->load())));
    voice.getOsc(2).setFineTuneSemitones(apvts.getRawParameterValue(ParamIDs::osc3Fine)->load());
    voice.getOsc(2).setKeyboardTracking(apvts.getRawParameterValue(ParamIDs::osc3KbdOn)->load() > 0.5f);

    bool phaseReset = apvts.getRawParameterValue(ParamIDs::oscPhaseReset)->load() > 0.5f;
    for (int i = 0; i < 3; ++i)
        voice.getOsc(i).setPhaseReset(phaseReset);

    // Mixer
    voice.setMixLevels(
        apvts.getRawParameterValue(ParamIDs::mixOsc1)->load(),
        apvts.getRawParameterValue(ParamIDs::mixOsc2)->load(),
        apvts.getRawParameterValue(ParamIDs::mixOsc3)->load(),
        apvts.getRawParameterValue(ParamIDs::mixNoise)->load(),
        apvts.getRawParameterValue(ParamIDs::mixExt)->load()
    );
    voice.setMixMutes(
        apvts.getRawParameterValue(ParamIDs::mixOsc1On)->load() > 0.5f,
        apvts.getRawParameterValue(ParamIDs::mixOsc2On)->load() > 0.5f,
        apvts.getRawParameterValue(ParamIDs::mixOsc3On)->load() > 0.5f,
        apvts.getRawParameterValue(ParamIDs::mixNoiseOn)->load() > 0.5f,
        apvts.getRawParameterValue(ParamIDs::mixExtOn)->load() > 0.5f
    );
    voice.setNoiseColor(static_cast<NoiseColor>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::noiseColor)->load())));
    voice.setMixerDrive(apvts.getRawParameterValue(ParamIDs::mixerDrive)->load());

    // Filter
    voice.setCutoffParam(apvts.getRawParameterValue(ParamIDs::cutoff)->load());
    voice.setEmphasis(apvts.getRawParameterValue(ParamIDs::emphasis)->load());
    voice.setContourAmount(apvts.getRawParameterValue(ParamIDs::contourAmount)->load());
    voice.setKeyTracking(
        apvts.getRawParameterValue(ParamIDs::kbd1)->load() > 0.5f,
        apvts.getRawParameterValue(ParamIDs::kbd2)->load() > 0.5f
    );
    voice.getFilter().setModel(static_cast<FilterModel>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::filterModel)->load())));
    voice.setBassCompensation(apvts.getRawParameterValue(ParamIDs::bassComp)->load());

    // Envelopes
    voice.getFilterEnv().setParameters(
        apvts.getRawParameterValue(ParamIDs::fAttack)->load(),
        apvts.getRawParameterValue(ParamIDs::fDecay)->load(),
        apvts.getRawParameterValue(ParamIDs::fSustain)->load()
    );
    voice.getAmpEnv().setParameters(
        apvts.getRawParameterValue(ParamIDs::aAttack)->load(),
        apvts.getRawParameterValue(ParamIDs::aDecay)->load(),
        apvts.getRawParameterValue(ParamIDs::aSustain)->load()
    );
    voice.setDecaySwitch(apvts.getRawParameterValue(ParamIDs::decaySwitchOn)->load() > 0.5f);

    // Global
    voice.setAnalogAmount(apvts.getRawParameterValue(ParamIDs::analogAmount)->load());
    voice.setNotePriority(static_cast<NotePriority>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::notePriority)->load())));
    voice.setEnvMode(static_cast<EnvRetriggerMode>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::envMode)->load())));

    // Arpeggiator
    auto& arp = voice.getArp();
    arp.setEnabled(apvts.getRawParameterValue(ParamIDs::arpOn)->load() > 0.5f);
    arp.setMode(static_cast<ArpMode>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::arpMode)->load())));
    arp.setOctaves(static_cast<int>(apvts.getRawParameterValue(ParamIDs::arpOctaves)->load()));
    arp.setRateSync(static_cast<ArpRateSync>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::arpRateSync)->load())));
    arp.setSyncMode(apvts.getRawParameterValue(ParamIDs::arpSync)->load() > 0.5f);
    arp.setFreeRateHz(apvts.getRawParameterValue(ParamIDs::arpFreeRate)->load());
    arp.setGate(apvts.getRawParameterValue(ParamIDs::arpGate)->load());
    arp.setLatch(apvts.getRawParameterValue(ParamIDs::arpLatch)->load() > 0.5f);

    // Master Volume
    float volDb = apvts.getRawParameterValue(ParamIDs::masterVol)->load();
    smoothedMasterVol.setTargetValue(juce::Decibels::decibelsToGain(volDb));
}

void LadderMonoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update voice parameters from APVTS atomics
    updateVoiceParameters();

    // Query host BPM
    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
    {
        if (auto posOpt = playHead->getPosition())
        {
            if (posOpt->getBpm().hasValue())
                bpm = *posOpt->getBpm();
        }
    }

    // Process MIDI messages sample-accurately
    int sampleIndex = 0;
    int numSamples = buffer.getNumSamples();

    auto midiIterator = midiMessages.findNextSamplePosition(0);

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = totalNumOutputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    int bendRange = static_cast<int>(apvts.getRawParameterValue(LadderMono::ParamIDs::bendRange)->load());

    while (sampleIndex < numSamples)
    {
        // Handle all midi events at current sample
        while (midiIterator != midiMessages.end() && (*midiIterator).samplePosition <= sampleIndex)
        {
            const auto metadata = *midiIterator;
            const auto msg = metadata.getMessage();

            if (msg.isNoteOn())
            {
                voice.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
            }
            else if (msg.isNoteOff())
            {
                voice.noteOff(msg.getNoteNumber());
            }
            else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            {
                voice.allNotesOff();
            }
            else if (msg.isPitchWheel())
            {
                // Pitch bend normalized -1.0 to +1.0
                float normBend = static_cast<float>(msg.getPitchWheelValue() - 8192) / 8192.0f;
                voice.setPitchBend(normBend * bendRange);
            }
            else if (msg.isController())
            {
                int ccNum = msg.getControllerNumber();
                int ccVal = msg.getControllerValue();
                if (ccNum == 1) // Mod wheel
                {
                    voice.setModWheel(ccVal / 127.0f);
                }
            }

            ++midiIterator;
        }

        // Render audio sample
        float sample = voice.processSample(bpm);
        float gain = smoothedMasterVol.getNextValue();
        sample *= gain;

        leftChannel[sampleIndex] = sample;
        if (rightChannel != nullptr)
            rightChannel[sampleIndex] = sample;

        ++sampleIndex;
    }
}

juce::AudioProcessorEditor* LadderMonoAudioProcessor::createEditor()
{
    return new LadderMonoAudioProcessorEditor(*this);
}

void LadderMonoAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("presetName", presetManager.getCurrentPresetName(), nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LadderMonoAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LadderMonoAudioProcessor();
}
