#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "presets/FactoryPresetsData.h"
#include "dsp/Saturation.h"

LadderMonoAudioProcessor::LadderMonoAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", LadderMono::createParameterLayout()),
      presetManager(apvts)
{
    // Wire up preset change to reset voices
    presetManager.setOnPresetChanged([this] {
        for (auto& v : voices)
            v.reset();
        voiceCounter = 0;
        voiceAge.fill(0);
    });

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

void LadderMonoAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    for (size_t i = 0; i < voices.size(); ++i)
    {
        voices[i].setVoiceIndex(i);
        voices[i].prepare(sampleRate);
    }

    voiceCounter = 0;
    voiceAge.fill(0);

    smoothedMasterVol.reset(sampleRate, 0.02);
    float initVolDb = apvts.getRawParameterValue(LadderMono::ParamIDs::masterVol)->load();
    smoothedMasterVol.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(initVolDb));
}

void LadderMonoAudioProcessor::releaseResources()
{
    for (auto& v : voices)
        v.reset();
}

void LadderMonoAudioProcessor::handleNoteOn(int noteNumber, float velocity) noexcept
{
    int voiceMode = static_cast<int>(apvts.getRawParameterValue(LadderMono::ParamIDs::voices)->load());
    bool arpEnabled = apvts.getRawParameterValue(LadderMono::ParamIDs::arpOn)->load() > 0.5f;

    if (arpEnabled || voiceMode == 0) // Mono or Arp
    {
        voices[0].noteOn(noteNumber, velocity);
        voiceAge[0] = ++voiceCounter;
        return;
    }

    size_t maxActive = (voiceMode == 1) ? 4 : 8;

    // 1. Re-use voice already playing this note
    for (size_t i = 0; i < maxActive; ++i)
    {
        if (voices[i].getActiveNote() == noteNumber)
        {
            voices[i].allNotesOff();
            voices[i].noteOn(noteNumber, velocity);
            voiceAge[i] = ++voiceCounter;
            return;
        }
    }

    // 2. Find an idle voice
    for (size_t i = 0; i < maxActive; ++i)
    {
        if (!voices[i].isAudible() && !voices[i].isKeyHeld())
        {
            voices[i].allNotesOff();
            voices[i].noteOn(noteNumber, velocity);
            voiceAge[i] = ++voiceCounter;
            return;
        }
    }

    // 3. Find a voice in release phase (oldest release)
    size_t oldestReleaseIdx = 0;
    bool foundRelease = false;
    uint32_t oldestReleaseAge = 0xFFFFFFFF;
    for (size_t i = 0; i < maxActive; ++i)
    {
        if (!voices[i].isKeyHeld())
        {
            if (voiceAge[i] < oldestReleaseAge)
            {
                oldestReleaseAge = voiceAge[i];
                oldestReleaseIdx = i;
                foundRelease = true;
            }
        }
    }
    if (foundRelease)
    {
        voices[oldestReleaseIdx].allNotesOff();
        voices[oldestReleaseIdx].noteOn(noteNumber, velocity);
        voiceAge[oldestReleaseIdx] = ++voiceCounter;
        return;
    }

    // 4. Steal oldest active voice
    size_t oldestIdx = 0;
    uint32_t oldestAge = voiceAge[0];
    for (size_t i = 1; i < maxActive; ++i)
    {
        if (voiceAge[i] < oldestAge)
        {
            oldestAge = voiceAge[i];
            oldestIdx = i;
        }
    }
    voices[oldestIdx].allNotesOff();
    voices[oldestIdx].noteOn(noteNumber, velocity);
    voiceAge[oldestIdx] = ++voiceCounter;
}

void LadderMonoAudioProcessor::handleNoteOff(int noteNumber) noexcept
{
    int voiceMode = static_cast<int>(apvts.getRawParameterValue(LadderMono::ParamIDs::voices)->load());
    bool arpEnabled = apvts.getRawParameterValue(LadderMono::ParamIDs::arpOn)->load() > 0.5f;

    if (arpEnabled || voiceMode == 0)
    {
        voices[0].noteOff(noteNumber);
        for (size_t i = 1; i < voices.size(); ++i)
        {
            if (voices[i].hasNote(noteNumber) || voices[i].getActiveNote() == noteNumber)
                voices[i].noteOff(noteNumber);
        }
        return;
    }

    for (size_t i = 0; i < voices.size(); ++i)
    {
        if (voices[i].getActiveNote() == noteNumber || voices[i].hasNote(noteNumber))
        {
            voices[i].noteOff(noteNumber);
        }
    }
}

void LadderMonoAudioProcessor::handleAllNotesOff() noexcept
{
    for (auto& v : voices)
        v.allNotesOff();
}

void LadderMonoAudioProcessor::updateVoiceParameters() noexcept
{
    using namespace LadderMono;

    // Controllers
    float tune = apvts.getRawParameterValue(ParamIDs::tune)->load();
    bool glideOn = apvts.getRawParameterValue(ParamIDs::glideOn)->load() > 0.5f;
    float glideTime = apvts.getRawParameterValue(ParamIDs::glideTime)->load();
    auto glideMode = static_cast<GlideMode>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::glideMode)->load()));
    float modMix = apvts.getRawParameterValue(ParamIDs::modMix)->load();
    bool oscModOn = apvts.getRawParameterValue(ParamIDs::oscModOn)->load() > 0.5f;
    bool filterModOn = apvts.getRawParameterValue(ParamIDs::filterModOn)->load() > 0.5f;
    auto modSource = static_cast<ModSource>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::modSource)->load()));
    float lfoRate = apvts.getRawParameterValue(ParamIDs::lfoRate)->load();
    auto lfoShape = static_cast<LFOShape>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::lfoShape)->load()));

    // Oscillators
    auto osc1Range = static_cast<Range>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc1Range)->load()));
    auto osc1Wave = static_cast<Waveform>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc1Wave)->load()));

    auto osc2Range = static_cast<Range>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc2Range)->load()));
    auto osc2Wave = static_cast<Waveform>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc2Wave)->load()));
    float osc2Fine = apvts.getRawParameterValue(ParamIDs::osc2Fine)->load();

    auto osc3Range = static_cast<Range>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc3Range)->load()));
    auto osc3Wave = static_cast<Waveform>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::osc3Wave)->load()));
    float osc3Fine = apvts.getRawParameterValue(ParamIDs::osc3Fine)->load();
    bool osc3Kbd = apvts.getRawParameterValue(ParamIDs::osc3KbdOn)->load() > 0.5f;
    bool phaseReset = apvts.getRawParameterValue(ParamIDs::oscPhaseReset)->load() > 0.5f;

    // Mixer
    float o1Lvl = apvts.getRawParameterValue(ParamIDs::mixOsc1)->load();
    float o2Lvl = apvts.getRawParameterValue(ParamIDs::mixOsc2)->load();
    float o3Lvl = apvts.getRawParameterValue(ParamIDs::mixOsc3)->load();
    float nLvl = apvts.getRawParameterValue(ParamIDs::mixNoise)->load();
    float extLvl = apvts.getRawParameterValue(ParamIDs::mixExt)->load();

    bool o1On = apvts.getRawParameterValue(ParamIDs::mixOsc1On)->load() > 0.5f;
    bool o2On = apvts.getRawParameterValue(ParamIDs::mixOsc2On)->load() > 0.5f;
    bool o3On = apvts.getRawParameterValue(ParamIDs::mixOsc3On)->load() > 0.5f;
    bool nOn = apvts.getRawParameterValue(ParamIDs::mixNoiseOn)->load() > 0.5f;
    bool extOn = apvts.getRawParameterValue(ParamIDs::mixExtOn)->load() > 0.5f;

    auto noiseColor = static_cast<NoiseColor>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::noiseColor)->load()));
    float drive = apvts.getRawParameterValue(ParamIDs::mixerDrive)->load();

    // Filter
    float cutoff = apvts.getRawParameterValue(ParamIDs::cutoff)->load();
    float emphasis = apvts.getRawParameterValue(ParamIDs::emphasis)->load();
    float contour = apvts.getRawParameterValue(ParamIDs::contourAmount)->load();
    bool kbd1 = apvts.getRawParameterValue(ParamIDs::kbd1)->load() > 0.5f;
    bool kbd2 = apvts.getRawParameterValue(ParamIDs::kbd2)->load() > 0.5f;
    auto fModel = static_cast<FilterModel>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::filterModel)->load()));
    float bassComp = apvts.getRawParameterValue(ParamIDs::bassComp)->load();

    // Envelopes
    float fA = apvts.getRawParameterValue(ParamIDs::fAttack)->load();
    float fD = apvts.getRawParameterValue(ParamIDs::fDecay)->load();
    float fS = apvts.getRawParameterValue(ParamIDs::fSustain)->load();

    float aA = apvts.getRawParameterValue(ParamIDs::aAttack)->load();
    float aD = apvts.getRawParameterValue(ParamIDs::aDecay)->load();
    float aS = apvts.getRawParameterValue(ParamIDs::aSustain)->load();
    bool decaySwitch = apvts.getRawParameterValue(ParamIDs::decaySwitchOn)->load() > 0.5f;

    // Global
    float analog = apvts.getRawParameterValue(ParamIDs::analogAmount)->load();
    auto notePrio = static_cast<NotePriority>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::notePriority)->load()));
    auto envRetrig = static_cast<EnvRetriggerMode>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::envMode)->load()));

    // Arpeggiator
    bool arpOn = apvts.getRawParameterValue(ParamIDs::arpOn)->load() > 0.5f;
    auto arpMode = static_cast<ArpMode>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::arpMode)->load()));
    int arpOct = static_cast<int>(apvts.getRawParameterValue(ParamIDs::arpOctaves)->load());
    auto arpSyncRate = static_cast<ArpRateSync>(static_cast<int>(apvts.getRawParameterValue(ParamIDs::arpRateSync)->load()));
    bool arpSync = apvts.getRawParameterValue(ParamIDs::arpSync)->load() > 0.5f;
    float arpFreeRate = apvts.getRawParameterValue(ParamIDs::arpFreeRate)->load();
    float arpGate = apvts.getRawParameterValue(ParamIDs::arpGate)->load();
    bool arpLatch = apvts.getRawParameterValue(ParamIDs::arpLatch)->load() > 0.5f;

    // Apply to all voices
    for (auto& v : voices)
    {
        v.setMasterTune(tune);
        v.getGlide().setEnabled(glideOn);
        v.getGlide().setGlideTime(glideTime);
        v.getGlide().setMode(glideMode);
        v.setModMix(modMix);
        v.setOscModEnabled(oscModOn);
        v.setFilterModEnabled(filterModOn);
        v.setModSource(modSource);
        v.setLFORate(lfoRate);
        v.setLFOShape(lfoShape);

        v.getOsc(0).setRange(osc1Range);
        v.getOsc(0).setWaveform(osc1Wave);
        v.getOsc(0).setPhaseReset(phaseReset);

        v.getOsc(1).setRange(osc2Range);
        v.getOsc(1).setWaveform(osc2Wave);
        v.getOsc(1).setFineTuneSemitones(osc2Fine);
        v.getOsc(1).setPhaseReset(phaseReset);

        v.getOsc(2).setRange(osc3Range);
        v.getOsc(2).setWaveform(osc3Wave);
        v.getOsc(2).setFineTuneSemitones(osc3Fine);
        v.getOsc(2).setKeyboardTracking(osc3Kbd);
        v.getOsc(2).setPhaseReset(phaseReset);

        v.setMixLevels(o1Lvl, o2Lvl, o3Lvl, nLvl, extLvl);
        v.setMixMutes(o1On, o2On, o3On, nOn, extOn);
        v.setNoiseColor(noiseColor);
        v.setMixerDrive(drive);

        v.setCutoffParam(cutoff);
        v.setEmphasis(emphasis);
        v.setContourAmount(contour);
        v.setKeyTracking(kbd1, kbd2);
        v.getFilter().setModel(fModel);
        v.setBassCompensation(bassComp);

        v.getFilterEnv().setParameters(fA, fD, fS);
        v.getAmpEnv().setParameters(aA, aD, aS);
        v.setDecaySwitch(decaySwitch);

        v.setAnalogAmount(analog);
        v.setNotePriority(notePrio);
        v.setEnvMode(envRetrig);

        auto& arp = v.getArp();
        arp.setEnabled(arpOn);
        arp.setMode(arpMode);
        arp.setOctaves(arpOct);
        arp.setRateSync(arpSyncRate);
        arp.setSyncMode(arpSync);
        arp.setFreeRateHz(arpFreeRate);
        arp.setGate(arpGate);
        arp.setLatch(arpLatch);
    }

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
    if (bypassed.load())
    {
        buffer.clear();
        cpuLoad.store(0.0f);
        return;
    }

    auto startTime = juce::Time::getHighResolutionTicks();

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
    int voiceMode = static_cast<int>(apvts.getRawParameterValue(LadderMono::ParamIDs::voices)->load());
    bool arpEnabled = apvts.getRawParameterValue(LadderMono::ParamIDs::arpOn)->load() > 0.5f;
    size_t maxActive = (voiceMode == 1) ? 4 : 8;

    while (sampleIndex < numSamples)
    {
        // Handle all midi events at current sample
        while (midiIterator != midiMessages.end() && (*midiIterator).samplePosition <= sampleIndex)
        {
            const auto metadata = *midiIterator;
            const auto msg = metadata.getMessage();

            if (msg.isNoteOn())
            {
                handleNoteOn(msg.getNoteNumber(), msg.getFloatVelocity());
            }
            else if (msg.isNoteOff())
            {
                handleNoteOff(msg.getNoteNumber());
            }
            else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            {
                handleAllNotesOff();
            }
            else if (msg.isPitchWheel())
            {
                // Pitch bend normalized -1.0 to +1.0
                float normBend = static_cast<float>(msg.getPitchWheelValue() - 8192) / 8192.0f;
                float bendSt = normBend * bendRange;
                for (auto& v : voices)
                    v.setPitchBend(bendSt);
            }
            else if (msg.isController())
            {
                int ccNum = msg.getControllerNumber();
                int ccVal = msg.getControllerValue();
                if (ccNum == 1) // Mod wheel
                {
                    float mw = ccVal / 127.0f;
                    for (auto& v : voices)
                        v.setModWheel(mw);
                }
            }

            ++midiIterator;
        }

        // Render audio sample from voices (Dynamic Voice Sleeping for zero idle CPU)
        float sample = 0.0f;

        if (arpEnabled || voiceMode == 0)
        {
            if (arpEnabled || voices[0].isAudible() || voices[0].isKeyHeld())
                sample = voices[0].processSample(bpm);
        }
        else
        {
            size_t activeCount = 0;
            for (size_t i = 0; i < maxActive; ++i)
            {
                if (voices[i].isAudible() || voices[i].isKeyHeld())
                {
                    sample += voices[i].processSample(bpm);
                    activeCount++;
                }
            }
            if (activeCount > 1)
            {
                sample *= (1.0f / std::sqrt(static_cast<float>(activeCount)));
            }
        }

        float gain = smoothedMasterVol.getNextValue();
        sample *= gain;

        // Master analog-modeled soft limiter (ceiling 0.985, completely eliminating speaker distortion)
        sample = LadderMono::Saturation::processMasterClip(sample);

        leftChannel[sampleIndex] = sample;
        if (rightChannel != nullptr)
            rightChannel[sampleIndex] = sample;

        ++sampleIndex;
    }

    // Update smoothed CPU load measurement
    auto elapsed = juce::Time::getHighResolutionTicks() - startTime;
    double elapsedSec = juce::Time::highResolutionTicksToSeconds(elapsed);
    double blockSec = static_cast<double>(numSamples) / std::max(44100.0, getSampleRate());
    float currentLoad = static_cast<float>((elapsedSec / std::max(0.00001, blockSec)) * 100.0);
    currentLoad = std::clamp(currentLoad, 0.0f, 100.0f);
    cpuLoad.store(cpuLoad.load() * 0.90f + currentLoad * 0.10f);
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
