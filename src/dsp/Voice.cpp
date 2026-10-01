#include "Voice.h"
#include "Saturation.h"
#include <cmath>
#include <algorithm>

namespace LadderMono
{
    Voice::Voice()
    {
        noteStack.reserve(16);
    }

    void Voice::prepare(double sr) noexcept
    {
        sampleRate = std::max(sr, 1000.0);
        for (auto& osc : oscs)
            osc.prepare(sampleRate);
        filter.prepare(sampleRate);
        filterEnv.prepare(sampleRate);
        ampEnv.prepare(sampleRate);
        glide.prepare(sampleRate);
        arp.prepare(sampleRate);
        noiseGen.reset();
        reset();
    }

    void Voice::reset() noexcept
    {
        for (auto& osc : oscs)
            osc.reset();
        filter.reset();
        filterEnv.reset();
        ampEnv.reset();
        glide.reset();
        arp.reset();
        noteStack.clear();
        activeNote = -1;
        lfoPhase = 0.0;
        lastOutputSample = 0.0f;
        externalInputSample = 0.0f;
    }

    int Voice::selectNextActiveNote() const noexcept
    {
        if (noteStack.empty())
            return -1;

        if (priority == NotePriority::Low)
        {
            int lowest = noteStack[0].note;
            for (const auto& entry : noteStack)
                if (entry.note < lowest)
                    lowest = entry.note;
            return lowest;
        }
        else if (priority == NotePriority::High)
        {
            int highest = noteStack[0].note;
            for (const auto& entry : noteStack)
                if (entry.note > highest)
                    highest = entry.note;
            return highest;
        }
        else // Last note priority
        {
            return noteStack.back().note;
        }
    }

    void Voice::updateActiveNote(bool isLegato) noexcept
    {
        int nextNote = selectNextActiveNote();
        if (nextNote != activeNote)
        {
            activeNote = nextNote;
            if (activeNote >= 0)
            {
                glide.setTargetNote(activeNote, isLegato);
                if (!isLegato || envMode == EnvRetriggerMode::MultiTrigger)
                {
                    filterEnv.startAttack();
                    ampEnv.startAttack();
                    for (auto& osc : oscs)
                        osc.triggerNote();
                }
            }
            else
            {
                filterEnv.startRelease(decaySwitch);
                ampEnv.startRelease(decaySwitch);
            }
        }
    }

    void Voice::noteOn(int noteNumber, float velocity) noexcept
    {
        if (arp.getEnabled())
        {
            arp.noteOn(noteNumber);
            return;
        }

        bool wasPlaying = (activeNote >= 0);
        // Remove if already present, then append
        auto it = std::find_if(noteStack.begin(), noteStack.end(), [noteNumber](const NoteEntry& e) { return e.note == noteNumber; });
        if (it != noteStack.end())
            noteStack.erase(it);

        noteStack.push_back({noteNumber, velocity});
        activeVelocity = velocity;

        updateActiveNote(wasPlaying);
    }

    void Voice::noteOff(int noteNumber) noexcept
    {
        if (arp.getEnabled())
        {
            arp.noteOff(noteNumber);
            return;
        }

        auto origCount = noteStack.size();
        noteStack.erase(std::remove_if(noteStack.begin(), noteStack.end(),
            [noteNumber](const NoteEntry& e) { return e.note == noteNumber; }), noteStack.end());

        if (noteStack.size() != origCount)
        {
            updateActiveNote(true);
        }
        else if (activeNote == noteNumber)
        {
            activeNote = -1;
            filterEnv.startRelease(decaySwitch);
            ampEnv.startRelease(decaySwitch);
        }
    }

    void Voice::allNotesOff() noexcept
    {
        noteStack.clear();
        activeNote = -1;
        arp.reset();
        filterEnv.startRelease(decaySwitch);
        ampEnv.startRelease(decaySwitch);
    }

    void Voice::setMixLevels(float o1, float o2, float o3, float noise, float ext) noexcept
    {
        osc1Level  = std::clamp(o1 / 10.0f, 0.0f, 1.0f);
        osc2Level  = std::clamp(o2 / 10.0f, 0.0f, 1.0f);
        osc3Level  = std::clamp(o3 / 10.0f, 0.0f, 1.0f);
        noiseLevel = std::clamp(noise / 10.0f, 0.0f, 1.0f);
        extLevel   = std::clamp(ext / 10.0f, 0.0f, 1.0f);
    }

    void Voice::setMixMutes(bool o1, bool o2, bool o3, bool noise, bool ext) noexcept
    {
        osc1On = o1;
        osc2On = o2;
        osc3On = o3;
        noiseOn = noise;
        extOn = ext;
    }

    void Voice::setMixerDrive(float db) noexcept
    {
        mixerDriveGain = std::pow(10.0f, db / 20.0f);
    }

    void Voice::setCutoffParam(float paramMinus5To5) noexcept
    {
        // -5 to +5 maps exponentially from ~20 Hz to 20 kHz
        // 0.0 maps to roughly 1000 Hz
        float normalized = (paramMinus5To5 + 5.0f) / 10.0f; // 0.0 to 1.0
        baseCutoffHz = 20.0f * std::pow(1000.0f, normalized);
    }

    void Voice::setKeyTracking(bool kbd1, bool kbd2) noexcept
    {
        // Classic Minimoog: Kbd 1 adds 1/3, Kbd 2 adds 2/3, both = full (1.0)
        float ratio = 0.0f;
        if (kbd1) ratio += (1.0f / 3.0f);
        if (kbd2) ratio += (2.0f / 3.0f);
        keyTrackingRatio = std::clamp(ratio, 0.0f, 1.0f);
    }

    float Voice::processSample(double bpm) noexcept
    {
        // 1. Process Arpeggiator if active
        if (arp.getEnabled())
        {
            int arpNote = -1;
            bool arpNoteOn = false;
            bool arpNoteOff = false;
            bool arpActive = arp.processSample(bpm, arpNote, arpNoteOn, arpNoteOff);

            if (arpNoteOn)
            {
                glide.setTargetNote(arpNote, false);
                filterEnv.startAttack();
                ampEnv.startAttack();
                for (auto& osc : oscs)
                    osc.triggerNote();
                activeNote = arpNote;
            }
            else if (arpNoteOff)
            {
                filterEnv.startRelease(decaySwitch);
                ampEnv.startRelease(decaySwitch);
            }
            else if (!arpActive && !arp.hasNotes() && activeNote >= 0)
            {
                activeNote = -1;
                filterEnv.startRelease(decaySwitch);
                ampEnv.startRelease(decaySwitch);
            }
        }

        // 2. Analog Drift Simulation (slow filtered random walk per oscillator)
        if (analogAmount > 0.001f)
        {
            for (size_t i = 0; i < 3; ++i)
            {
                float white = noiseGen.process(NoiseColor::White);
                // Very slow one-pole lowpass filter (~0.1 Hz)
                driftNoiseState[i] = 0.9997f * driftNoiseState[i] + 0.0003f * white;
                oscDriftCents[i] = driftNoiseState[i] * (analogAmount * 4.0f); // +/- 1 to 4 cents
            }
        }
        else
        {
            oscDriftCents = {0.0f, 0.0f, 0.0f};
        }

        // 3. Process Glide & Pitch
        float currentPitchNote = glide.processSample();
        float baseFreqHz = 440.0f * std::exp2f((currentPitchNote - 69.0f) * (1.0f / 12.0f));

        // Master tune
        if (std::abs(masterTuneCents) > 0.001f)
            baseFreqHz *= std::exp2f(masterTuneCents * (1.0f / 1200.0f));

        // 4. LFO & Modulation Bus
        float lfoOut = 0.0f;
        if (lfoShape == LFOShape::Triangle)
        {
            lfoOut = 2.0f * std::abs(2.0f * static_cast<float>(lfoPhase) - 1.0f) - 1.0f;
        }
        else // Square
        {
            lfoOut = (lfoPhase < 0.5) ? 1.0f : -1.0f;
        }
        lfoPhase += lfoRate / sampleRate;
        if (lfoPhase >= 1.0)
            lfoPhase -= 1.0;

        float noiseSample = noiseGen.process(noiseColor);

        // Osc 3 mod source (if osc3 is used as modulation source)
        float osc3Sample = 0.0f;

        // Mod mix: 0 = Osc3/LFO, 1 = Noise
        float modSourceSignal = (modSource == ModSource::LFO) ? lfoOut : osc3Sample;
        float modBusSignal = (1.0f - modMix) * modSourceSignal + modMix * noiseSample;
        float effectiveMod = modBusSignal * modWheel; // scaled by Mod Wheel

        float pitchModSemitones = pitchBendSemitones;
        if (oscModEnabled)
        {
            // Up to +/- 7 semitones modulation
            pitchModSemitones += effectiveMod * 7.0f;
        }

        // 5. Generate Oscillators
        float o1 = osc1On ? oscs[0].process(baseFreqHz, pitchModSemitones, oscDriftCents[0]) : 0.0f;
        float o2 = osc2On ? oscs[1].process(baseFreqHz, pitchModSemitones, oscDriftCents[1]) : 0.0f;
        float o3 = osc3On ? oscs[2].process(baseFreqHz, pitchModSemitones, oscDriftCents[2]) : 0.0f;

        // 6. Mixer Summation & Overdrive
        float extInput = extOn ? (lastOutputSample + externalInputSample) : 0.0f;
        float mixerSum = (o1 * osc1Level) + (o2 * osc2Level) + (o3 * osc3Level)
                       + (noiseOn ? (noiseSample * noiseLevel) : 0.0f)
                       + (extInput * extLevel);

        // Mixer Overload soft saturation with mild asymmetric second harmonic
        float saturatedMixer = Saturation::processMixerOverload(mixerSum, mixerDriveGain);

        // 7. Envelopes
        float fEnvLevel = filterEnv.processSample();
        float aEnvLevel = ampEnv.processSample();

        // 8. Filter Cutoff Calculation
        // Modulation in octaves (1V/octave tracking)
        float cutoffOctaves = 0.0f;

        // Keyboard tracking: note 60 (C4) is reference
        if (keyTrackingRatio > 0.0f)
        {
            cutoffOctaves += (currentPitchNote - 60.0f) / 12.0f * keyTrackingRatio;
        }

        // Filter contour envelope modulation (0 to ~4.5 octaves)
        cutoffOctaves += fEnvLevel * (contourAmount / 10.0f) * 4.5f;

        // Filter mod from modulation bus (up to ~3.5 octaves)
        if (filterModEnabled)
        {
            cutoffOctaves += effectiveMod * 3.5f;
        }

        float modulatedCutoffHz = baseCutoffHz * std::exp2f(cutoffOctaves);

        // Process Ladder Filter
        float filterOut = filter.processSample(saturatedMixer, modulatedCutoffHz, emphasis, 1.0f, bassComp);

        // 9. VCA & Amp Envelope (calibrated analog stage with clean headroom)
        float output = (filterOut * aEnvLevel) * 1.25f;

        // Gentle voice headroom limiter (cushions extreme peaks above 1.0)
        output = Saturation::processVoiceClip(output);

        lastOutputSample = output;
        return output;
    }
}
