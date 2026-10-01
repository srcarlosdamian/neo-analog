#pragma once
#include "Oscillator.h"
#include "LadderFilter.h"
#include "Envelope.h"
#include "Glide.h"
#include "NoiseGen.h"
#include "Arpeggiator.h"
#include <vector>
#include <array>

namespace LadderMono
{
    enum class NotePriority
    {
        Low = 0, // Minimoog default authentic
        Last,
        High
    };

    enum class EnvRetriggerMode
    {
        MultiTrigger = 0,
        Legato
    };

    enum class ModSource
    {
        Osc3 = 0,
        LFO
    };

    enum class LFOShape
    {
        Triangle = 0,
        Square
    };

    class Voice
    {
    public:
        Voice();

        void prepare(double sampleRate) noexcept;
        void reset() noexcept;

        // MIDI note events
        void noteOn(int noteNumber, float velocity) noexcept;
        void noteOff(int noteNumber) noexcept;
        void allNotesOff() noexcept;

        // Performance controllers
        void setPitchBend(float semitones) noexcept { pitchBendSemitones = semitones; }
        void setModWheel(float amount0to1) noexcept { modWheel = amount0to1; }
        void setMasterTune(float cents) noexcept { masterTuneCents = cents; }

        // Process a block or single sample
        float processSample(double bpm) noexcept;

        // Sub-component accessors for parameter bindings
        Oscillator& getOsc(size_t index) noexcept { return oscs[index]; }
        const Oscillator& getOsc(size_t index) const noexcept { return oscs[index]; }
        LadderFilter& getFilter() noexcept { return filter; }
        Envelope& getFilterEnv() noexcept { return filterEnv; }
        Envelope& getAmpEnv() noexcept { return ampEnv; }
        Glide& getGlide() noexcept { return glide; }
        Arpeggiator& getArp() noexcept { return arp; }

        // Configuration setters
        void setNotePriority(NotePriority p) noexcept { priority = p; }
        void setEnvMode(EnvRetriggerMode m) noexcept { envMode = m; }
        void setDecaySwitch(bool on) noexcept { decaySwitch = on; }

        // Mixer levels & switches (0 to 10)
        void setMixLevels(float o1, float o2, float o3, float noise, float ext) noexcept;
        void setMixMutes(bool o1, bool o2, bool o3, bool noise, bool ext) noexcept;
        void setNoiseColor(NoiseColor c) noexcept { noiseColor = c; }
        void setMixerDrive(float db) noexcept;

        // Modulation controls
        void setModMix(float val0to10) noexcept { modMix = val0to10 / 10.0f; }
        void setModSource(ModSource src) noexcept { modSource = src; }
        void setOscModEnabled(bool on) noexcept { oscModEnabled = on; }
        void setFilterModEnabled(bool on) noexcept { filterModEnabled = on; }
        void setLFORate(float hz) noexcept { lfoRate = hz; }
        void setLFOShape(LFOShape s) noexcept { lfoShape = s; }

        // Filter parameters
        void setCutoffParam(float paramMinus5To5) noexcept;
        void setEmphasis(float res0to10) noexcept { emphasis = res0to10; }
        void setContourAmount(float amt0to10) noexcept { contourAmount = amt0to10; }
        void setKeyTracking(bool kbd1, bool kbd2) noexcept;
        void setBassCompensation(float comp0to1) noexcept { bassComp = comp0to1; }

        // Analog Drift
        void setAnalogAmount(float amount0to1) noexcept { analogAmount = amount0to1; }

        // Feedback / external
        void setExternalFeedbackSample(float s) noexcept { externalInputSample = s; }

        bool isActive() const noexcept { return ampEnv.isActive() || arp.hasNotes(); }
        bool isAudible() const noexcept { return ampEnv.isActive(); }
        bool isKeyHeld() const noexcept { return activeNote >= 0; }
        int getActiveNote() const noexcept { return activeNote; }
        bool hasNote(int noteNumber) const noexcept
        {
            if (activeNote == noteNumber) return true;
            for (const auto& e : noteStack)
                if (e.note == noteNumber) return true;
            return false;
        }

    private:
        double sampleRate = 44100.0;

        std::array<Oscillator, 3> oscs;
        LadderFilter filter;
        Envelope filterEnv;
        Envelope ampEnv;
        Glide glide;
        NoiseGen noiseGen;
        Arpeggiator arp;

        // Note stack handling
        struct NoteEntry
        {
            int note;
            float velocity;
        };
        std::vector<NoteEntry> noteStack;
        int activeNote = -1;
        float activeVelocity = 1.0f;

        NotePriority priority = NotePriority::Last;
        EnvRetriggerMode envMode = EnvRetriggerMode::MultiTrigger;
        bool decaySwitch = true;

        // Controllers
        float pitchBendSemitones = 0.0f;
        float modWheel = 0.0f;
        float masterTuneCents = 0.0f;

        // Mixer
        float osc1Level = 0.8f, osc2Level = 0.0f, osc3Level = 0.0f;
        float noiseLevel = 0.0f, extLevel = 0.0f;
        bool osc1On = true, osc2On = false, osc3On = false;
        bool noiseOn = false, extOn = false;
        NoiseColor noiseColor = NoiseColor::Pink;
        float mixerDriveGain = 1.0f;
        float lastOutputSample = 0.0f;
        float externalInputSample = 0.0f;

        // Mod bus & LFO
        float modMix = 0.0f; // 0 = Osc3/LFO, 1 = Noise
        ModSource modSource = ModSource::LFO;
        bool oscModEnabled = false;
        bool filterModEnabled = false;
        float lfoRate = 5.0f;
        LFOShape lfoShape = LFOShape::Triangle;
        double lfoPhase = 0.0;

        // Filter settings
        float baseCutoffHz = 1000.0f;
        float emphasis = 0.0f;
        float contourAmount = 3.0f;
        float keyTrackingRatio = 0.0f;
        float bassComp = 0.25f;

        // Analog drift state
        float analogAmount = 0.30f;
        std::array<float, 3> oscDriftCents = {0.0f, 0.0f, 0.0f};
        std::array<float, 3> driftNoiseState = {0.0f, 0.0f, 0.0f};

        int selectNextActiveNote() const noexcept;
        void updateActiveNote(bool isLegato) noexcept;
    };
}
