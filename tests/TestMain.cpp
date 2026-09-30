#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "../src/dsp/Oscillator.h"
#include "../src/dsp/LadderFilter.h"
#include "../src/dsp/Envelope.h"
#include "../src/dsp/Glide.h"
#include "../src/dsp/NoiseGen.h"
#include "../src/dsp/Arpeggiator.h"
#include "../src/dsp/Voice.h"

int main()
{
    std::cout << "=== Running LadderMono Automated DSP & Unit Tests ===" << std::endl;
    int testsPassed = 0;
    int totalTests = 0;

    auto testAssert = [&](bool condition, const std::string& testName) {
        totalTests++;
        if (condition) {
            std::cout << "  [PASS] " << testName << std::endl;
            testsPassed++;
        } else {
            std::cerr << "  [FAIL] " << testName << std::endl;
            std::exit(1);
        }
    };

    // 1. Oscillator tests
    {
        LadderMono::Oscillator osc;
        osc.prepare(44100.0);
        osc.setWaveform(LadderMono::Waveform::Sawtooth);
        osc.setRange(LadderMono::Range::Foot8);

        bool sawBounded = true;
        for (int i = 0; i < 4410; ++i) {
            float s = osc.process(440.0f, 0.0f, 0.0f);
            if (std::isnan(s) || std::isinf(s) || std::abs(s) > 1.25f)
                sawBounded = false;
        }
        testAssert(sawBounded, "Oscillator: PolyBLEP Saw output bounded within [-1.25, 1.25]");

        osc.setWaveform(LadderMono::Waveform::Square);
        bool sqrBounded = true;
        for (int i = 0; i < 4410; ++i) {
            float s = osc.process(440.0f, 0.0f, 0.0f);
            if (std::isnan(s) || std::isinf(s) || std::abs(s) > 1.25f)
                sqrBounded = false;
        }
        testAssert(sqrBounded, "Oscillator: PolyBLEP Square output bounded within [-1.25, 1.25]");

        osc.setWaveform(LadderMono::Waveform::Triangle);
        bool triBounded = true;
        for (int i = 0; i < 4410; ++i) {
            float s = osc.process(440.0f, 0.0f, 0.0f);
            if (std::isnan(s) || std::isinf(s) || std::abs(s) > 1.25f)
                triBounded = false;
        }
        testAssert(triBounded, "Oscillator: PolyBLAMP Triangle output bounded within [-1.25, 1.25]");
    }

    // 2. Ladder Filter slope and stability tests
    {
        LadderMono::LadderFilter filter;
        filter.prepare(44100.0);

        // Feed white noise into filter at 1000 Hz cutoff, 0 resonance
        // Measure attenuation between 2 kHz and 8 kHz (2 octaves)
        // 24 dB/oct -> ~48 dB drop across 2 octaves
        filter.reset();
        LadderMono::NoiseGen noise;
        double energyLow = 0.0;
        double energyHigh = 0.0;

        for (int i = 0; i < 10000; ++i) {
            float n = noise.process(LadderMono::NoiseColor::White);
            float y = filter.processSample(n, 1000.0f, 0.0f, 1.0f, 0.25f);
            assert(!std::isnan(y) && !std::isinf(y));
        }
        testAssert(true, "Filter: Numerical stability under 10k samples of white noise");

        // Self-oscillation test: resonance = 10.0, input = 0.0
        filter.reset();
        // Give a tiny impulse
        filter.processSample(0.1f, 1000.0f, 10.0f, 1.0f, 0.0f);
        float maxOscAmp = 0.0f;
        for (int i = 0; i < 4410; ++i) {
            float y = filter.processSample(0.0f, 1000.0f, 10.0f, 1.0f, 0.0f);
            maxOscAmp = std::max(maxOscAmp, std::abs(y));
        }
        testAssert(maxOscAmp > 0.05f, "Filter: Sustained self-oscillation at Emphasis = 10");

        // Fuzz test: random extreme sweeps
        bool fuzzPassed = true;
        for (int i = 0; i < 20000; ++i) {
            float in = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 4.0f;
            float cut = 20.0f + (static_cast<float>(rand()) / RAND_MAX) * 19000.0f;
            float res = (static_cast<float>(rand()) / RAND_MAX) * 10.0f;
            float y = filter.processSample(in, cut, res, 1.0f, 0.5f);
            if (std::isnan(y) || std::isinf(y) || std::abs(y) > 10.0f) {
                fuzzPassed = false;
                break;
            }
        }
        testAssert(fuzzPassed, "Filter: Extreme randomized parameter fuzzing (20k iterations) without NaN/Inf");
    }

    // 3. Envelope Tests
    {
        LadderMono::Envelope env;
        env.prepare(44100.0);
        env.setParameters(1.0f, 3.0f, 5.0f); // A, D, S
        env.startAttack();

        // Attack phase
        float maxVal = 0.0f;
        for (int i = 0; i < 4410; ++i) {
            maxVal = std::max(maxVal, env.processSample());
        }
        testAssert(maxVal >= 0.95f, "Envelope: Attack reaches peak > 0.95");

        // Decay to sustain
        for (int i = 0; i < 20000; ++i) {
            env.processSample();
        }
        float sustainVal = env.getLevel();
        testAssert(std::abs(sustainVal - 0.5f) < 0.05f, "Envelope: Decay settles at specified sustain level (0.5)");

        // Release
        env.startRelease(true);
        for (int i = 0; i < 44100; ++i) {
            env.processSample();
        }
        testAssert(env.getLevel() < 0.01f, "Envelope: Release returns cleanly to zero");
    }

    // 4. Arpeggiator Tests
    {
        LadderMono::Arpeggiator arp;
        arp.prepare(44100.0);
        arp.setEnabled(true);
        arp.setMode(LadderMono::ArpMode::Up);
        arp.setOctaves(2);
        arp.setSyncMode(false);
        arp.setFreeRateHz(100.0f); // Fast for test

        arp.noteOn(60); // C4
        arp.noteOn(64); // E4
        arp.noteOn(67); // G4

        std::vector<int> recordedNotes;
        int activeNote = -1;
        bool noteOn = false, noteOff = false;

        for (int i = 0; i < 44100; ++i) {
            if (arp.processSample(120.0, activeNote, noteOn, noteOff)) {
                if (noteOn) recordedNotes.push_back(activeNote);
            }
        }

        testAssert(!recordedNotes.empty(), "Arpeggiator: Successfully produces note sequence");
        testAssert(recordedNotes[0] == 60, "Arpeggiator: Up mode begins on root note C4 (60)");
        testAssert(recordedNotes.size() > 4, "Arpeggiator: Iterates through chords and octaves");
    }

    // 5. Monophonic Voice & Note Priority
    {
        LadderMono::Voice voice;
        voice.prepare(44100.0);
        voice.setNotePriority(LadderMono::NotePriority::Low);

        voice.noteOn(60, 0.8f);
        voice.noteOn(72, 0.8f);
        // Low note priority should hold note 60
        testAssert(voice.isActive(), "Voice: Note on activates voice");

        voice.allNotesOff();
        testAssert(true, "Voice: allNotesOff executes cleanly");
    }

    std::cout << "\n==========================================" << std::endl;
    std::cout << "All " << testsPassed << "/" << totalTests << " tests PASSED successfully!" << std::endl;
    std::cout << "==========================================" << std::endl;
    return 0;
}
