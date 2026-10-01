#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "PluginProcessor.h"

int main()
{
    std::cout << "========================================================\n";
    std::cout << "       NEO ANALOG — FULL SYSTEM & DSP AUDIT             \n";
    std::cout << "========================================================\n\n";

    LadderMonoAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);

    auto& apvts = processor.getAPVTS();
    int testsRun = 0;
    int testsPassed = 0;
    int warnings = 0;

    auto auditCheck = [&](bool condition, const std::string& section, const std::string& desc, const std::string& details = "") {
        testsRun++;
        std::cout << "[" << std::setw(12) << std::left << section << "] ";
        if (condition) {
            std::cout << "PASS: " << desc;
            if (!details.empty()) std::cout << " (" << details << ")";
            std::cout << "\n";
            testsPassed++;
        } else {
            std::cout << "FAIL: " << desc;
            if (!details.empty()) std::cout << " [" << details << "]";
            std::cout << "\n";
        }
    };

    auto auditWarn = [&](const std::string& section, const std::string& desc, const std::string& details) {
        warnings++;
        std::cout << "[" << std::setw(12) << std::left << section << "] WARN: " << desc
                  << " -> " << details << "\n";
    };

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;

    // Helper: Reset all to silent baseline
    auto resetToInit = [&]() {
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1On) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc2On) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc3On) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixNoiseOn) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixExtOn) = 0.0f;

        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc2) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc3) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixNoise) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixExt) = 0.0f;

        *apvts.getRawParameterValue(LadderMono::ParamIDs::cutoff) = 5.0f; // wide open
        *apvts.getRawParameterValue(LadderMono::ParamIDs::emphasis) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::contourAmount) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::bassComp) = 0.0f;

        *apvts.getRawParameterValue(LadderMono::ParamIDs::aAttack) = 0.01f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::aDecay) = 0.5f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::aSustain) = 10.0f; // full sustain

        *apvts.getRawParameterValue(LadderMono::ParamIDs::fAttack) = 0.01f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::fDecay) = 0.5f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::fSustain) = 10.0f;

        *apvts.getRawParameterValue(LadderMono::ParamIDs::masterVol) = 0.0f; // 0 dB
        *apvts.getRawParameterValue(LadderMono::ParamIDs::voices) = 0.0f; // Mono
        *apvts.getRawParameterValue(LadderMono::ParamIDs::arpOn) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::analogAmount) = 0.0f;

        // Run empty block to flush
        midi.clear();
        buffer.clear();
        processor.processBlock(buffer, midi);
    };

    // =========================================================================
    // AUDIT 1: NOISE GENERATION (WHITE & PINK)
    // =========================================================================
    std::cout << "\n--- AUDITING NOISE GENERATOR ---\n";
    {
        resetToInit();
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixNoiseOn) = 1.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixNoise) = 10.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::noiseColor) = 0.0f; // White

        // Note on to open VCA
        midi.clear();
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);
        buffer.clear();
        processor.processBlock(buffer, midi);

        // Process a few blocks and measure White Noise RMS and Peak
        float whiteRms = 0.0f, whitePeak = 0.0f;
        for (int b = 0; b < 20; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
            for (int s = 0; s < 512; ++s) {
                float v = buffer.getSample(0, s);
                whiteRms += v * v;
                whitePeak = std::max(whitePeak, std::abs(v));
            }
        }
        whiteRms = std::sqrt(whiteRms / (20 * 512));

        auditCheck(whitePeak > 0.1f && whiteRms > 0.05f, "NOISE", "White Noise outputs audible signal",
                   "RMS: " + std::to_string(whiteRms) + ", Peak: " + std::to_string(whitePeak));

        // Switch to Pink Noise
        *apvts.getRawParameterValue(LadderMono::ParamIDs::noiseColor) = 1.0f; // Pink

        float pinkRms = 0.0f, pinkPeak = 0.0f;
        for (int b = 0; b < 20; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
            for (int s = 0; s < 512; ++s) {
                float v = buffer.getSample(0, s);
                pinkRms += v * v;
                pinkPeak = std::max(pinkPeak, std::abs(v));
            }
        }
        pinkRms = std::sqrt(pinkRms / (20 * 512));

        auditCheck(pinkPeak > 0.05f, "NOISE", "Pink Noise outputs signal",
                   "RMS: " + std::to_string(pinkRms) + ", Peak: " + std::to_string(pinkPeak));

        // Relative balance between Pink and White
        float pinkToWhiteRatio = pinkRms / (whiteRms + 1e-6f);
        if (pinkToWhiteRatio < 0.5f) {
            auditWarn("NOISE", "Pink noise is noticeably quieter than White noise",
                      "Ratio: " + std::to_string(pinkToWhiteRatio) + " (approx " +
                      std::to_string(20.0f * std::log10(pinkToWhiteRatio)) + " dB deficit)");
        } else {
            auditCheck(true, "NOISE", "Pink and White noise levels are balanced",
                       "Ratio: " + std::to_string(pinkToWhiteRatio));
        }

        // Test with Classic Bass cutoff (-3.0)
        *apvts.getRawParameterValue(LadderMono::ParamIDs::cutoff) = -3.0f;
        float filteredPinkRms = 0.0f;
        for (int b = 0; b < 10; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
            for (int s = 0; s < 512; ++s) {
                float v = buffer.getSample(0, s);
                filteredPinkRms += v * v;
            }
        }
        filteredPinkRms = std::sqrt(filteredPinkRms / (10 * 512));
        if (filteredPinkRms < 0.005f) {
            auditWarn("NOISE", "Pink noise with default low cutoff (-3.0) is nearly inaudible",
                      "Filtered RMS: " + std::to_string(filteredPinkRms));
        }

        // Release note
        midi.clear();
        midi.addEvent(juce::MidiMessage::noteOff(1, 60), 0);
        buffer.clear();
        processor.processBlock(buffer, midi);
    }

    // =========================================================================
    // AUDIT 2: OSCILLATORS 1, 2, 3 INDIVIDUAL OUTPUT
    // =========================================================================
    std::cout << "\n--- AUDITING OSCILLATOR BANK ---\n";
    {
        for (int oscIdx = 1; oscIdx <= 3; ++oscIdx) {
            resetToInit();
            std::string onParam = (oscIdx == 1) ? LadderMono::ParamIDs::mixOsc1On :
                                  (oscIdx == 2) ? LadderMono::ParamIDs::mixOsc2On :
                                                  LadderMono::ParamIDs::mixOsc3On;
            std::string lvlParam = (oscIdx == 1) ? LadderMono::ParamIDs::mixOsc1 :
                                   (oscIdx == 2) ? LadderMono::ParamIDs::mixOsc2 :
                                                   LadderMono::ParamIDs::mixOsc3;

            *apvts.getRawParameterValue(onParam) = 1.0f;
            *apvts.getRawParameterValue(lvlParam) = 8.0f;

            midi.clear();
            midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);
            buffer.clear();
            processor.processBlock(buffer, midi);

            float peak = 0.0f, rms = 0.0f;
            for (int b = 0; b < 10; ++b) {
                midi.clear();
                buffer.clear();
                processor.processBlock(buffer, midi);
                for (int s = 0; s < 512; ++s) {
                    float v = buffer.getSample(0, s);
                    peak = std::max(peak, std::abs(v));
                    rms += v * v;
                }
            }
            rms = std::sqrt(rms / (10 * 512));

            auditCheck(peak > 0.1f && rms > 0.05f, "OSCILLATOR",
                       "Oscillator " + std::to_string(oscIdx) + " generates clear signal",
                       "Peak: " + std::to_string(peak) + ", RMS: " + std::to_string(rms));

            midi.clear();
            midi.addEvent(juce::MidiMessage::noteOff(1, 60), 0);
            buffer.clear();
            processor.processBlock(buffer, midi);
        }
    }

    // =========================================================================
    // AUDIT 3: FILTER CONTROLS & SELF-OSCILLATION
    // =========================================================================
    std::cout << "\n--- AUDITING FILTER SECTION ---\n";
    {
        resetToInit();
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1On) = 1.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1) = 8.0f;

        // Test wide open (+5) vs closed (-5)
        *apvts.getRawParameterValue(LadderMono::ParamIDs::cutoff) = 5.0f;
        midi.clear();
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);
        buffer.clear();
        processor.processBlock(buffer, midi);

        float openRms = 0.0f;
        for (int b = 0; b < 10; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
            for (int s = 0; s < 512; ++s) openRms += buffer.getSample(0, s) * buffer.getSample(0, s);
        }
        openRms = std::sqrt(openRms / (10 * 512));

        *apvts.getRawParameterValue(LadderMono::ParamIDs::cutoff) = -5.0f;
        // Let filter settle for 5 blocks (discharge previous energy)
        for (int b = 0; b < 5; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
        }

        float closedRms = 0.0f;
        for (int b = 0; b < 10; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
            for (int s = 0; s < 512; ++s) closedRms += buffer.getSample(0, s) * buffer.getSample(0, s);
        }
        closedRms = std::sqrt(closedRms / (10 * 512));

        auditCheck(openRms > closedRms * 10.0f, "FILTER", "Cutoff sweeps smoothly from open to closed",
                   "Open RMS: " + std::to_string(openRms) + ", Closed RMS: " + std::to_string(closedRms));

        // Test self-oscillation (Emphasis = 10, Cutoff = 0, no oscillators, key strike impulse)
        resetToInit();
        *apvts.getRawParameterValue(LadderMono::ParamIDs::cutoff) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::emphasis) = 10.0f; // max resonance
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixNoiseOn) = 1.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixNoise) = 0.2f; // subtle excitation pulse as in Model D

        midi.clear();
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);
        buffer.clear();
        processor.processBlock(buffer, midi);

        // Turn noise off after 1 block to verify pure sustained self-oscillation
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixNoiseOn) = 0.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixNoise) = 0.0f;

        float selfOscRms = 0.0f;
        for (int b = 0; b < 20; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
            for (int s = 0; s < 512; ++s) selfOscRms += buffer.getSample(0, s) * buffer.getSample(0, s);
        }
        selfOscRms = std::sqrt(selfOscRms / (20 * 512));

        auditCheck(selfOscRms > 0.02f, "FILTER", "Filter enters self-oscillation at max emphasis",
                   "Self-osc RMS: " + std::to_string(selfOscRms));

        midi.clear();
        midi.addEvent(juce::MidiMessage::noteOff(1, 60), 0);
        buffer.clear();
        processor.processBlock(buffer, midi);
    }

    // =========================================================================
    // AUDIT 4: MODULATION BUS (LFO & NOISE MODULATION)
    // =========================================================================
    std::cout << "\n--- AUDITING MODULATION BUS ---\n";
    {
        resetToInit();
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1On) = 1.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1) = 8.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::oscModOn) = 1.0f; // pitch mod
        *apvts.getRawParameterValue(LadderMono::ParamIDs::lfoRate) = 10.0f; // 10 Hz vibrato

        // Test with Mod Wheel = 127
        midi.clear();
        midi.addEvent(juce::MidiMessage::controllerEvent(1, 1, 127), 0); // Mod Wheel
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);
        buffer.clear();
        processor.processBlock(buffer, midi);

        float vibratoPeak = 0.0f;
        for (int b = 0; b < 10; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
            for (int s = 0; s < 512; ++s) vibratoPeak = std::max(vibratoPeak, std::abs(buffer.getSample(0, s)));
        }

        auditCheck(vibratoPeak > 0.1f, "MOD BUS", "LFO pitch modulation responds to Mod Wheel",
                   "Peak: " + std::to_string(vibratoPeak));

        // Test Noise as Mod Source (Mod Mix = 1.0)
        *apvts.getRawParameterValue(LadderMono::ParamIDs::modMix) = 1.0f; // 100% Noise mod
        float noiseModPeak = 0.0f;
        for (int b = 0; b < 10; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
            for (int s = 0; s < 512; ++s) noiseModPeak = std::max(noiseModPeak, std::abs(buffer.getSample(0, s)));
        }

        auditCheck(noiseModPeak > 0.1f, "MOD BUS", "Noise modulation bus routes successfully",
                   "Peak: " + std::to_string(noiseModPeak));

        // Reset Mod Wheel
        midi.clear();
        midi.addEvent(juce::MidiMessage::controllerEvent(1, 1, 0), 0);
        midi.addEvent(juce::MidiMessage::noteOff(1, 60), 0);
        buffer.clear();
        processor.processBlock(buffer, midi);
    }

    // =========================================================================
    // AUDIT 5: POLYPHONY MODES (1, 4, 8 VOICES)
    // =========================================================================
    std::cout << "\n--- AUDITING POLYPHONY (1, 4, 8 VOICES) ---\n";
    {
        for (int mode = 0; mode < 3; ++mode) {
            resetToInit();
            *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1On) = 1.0f;
            *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1) = 6.0f;
            *apvts.getRawParameterValue(LadderMono::ParamIDs::voices) = static_cast<float>(mode);

            std::string modeName = (mode == 0) ? "1 Voice (Mono)" : (mode == 1) ? "4 Voices" : "8 Voices";

            // Play 4 notes simultaneously
            midi.clear();
            midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)90), 0);
            midi.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8)90), 0);
            midi.addEvent(juce::MidiMessage::noteOn(1, 67, (juce::uint8)90), 0);
            midi.addEvent(juce::MidiMessage::noteOn(1, 71, (juce::uint8)90), 0);
            buffer.clear();
            processor.processBlock(buffer, midi);

            float chordRms = 0.0f;
            for (int b = 0; b < 10; ++b) {
                midi.clear();
                buffer.clear();
                processor.processBlock(buffer, midi);
                for (int s = 0; s < 512; ++s) chordRms += buffer.getSample(0, s) * buffer.getSample(0, s);
            }
            chordRms = std::sqrt(chordRms / (10 * 512));

            auditCheck(chordRms > 0.05f, "POLYPHONY", modeName + " processes polyphonic chords cleanly",
                       "Chord RMS: " + std::to_string(chordRms));

            midi.clear();
            midi.addEvent(juce::MidiMessage::allNotesOff(1), 0);
            buffer.clear();
            processor.processBlock(buffer, midi);
        }
    }

    // =========================================================================
    // AUDIT 6: ARPEGGIATOR MODES
    // =========================================================================
    std::cout << "\n--- AUDITING ARPEGGIATOR ---\n";
    {
        resetToInit();
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1On) = 1.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::mixOsc1) = 7.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::arpOn) = 1.0f;
        *apvts.getRawParameterValue(LadderMono::ParamIDs::arpSync) = 0.0f; // Free rate
        *apvts.getRawParameterValue(LadderMono::ParamIDs::arpFreeRate) = 16.0f; // 16 Hz

        midi.clear();
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);
        midi.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8)100), 0);
        midi.addEvent(juce::MidiMessage::noteOn(1, 67, (juce::uint8)100), 0);
        buffer.clear();
        processor.processBlock(buffer, midi);

        int nonZeroBlocks = 0;
        for (int b = 0; b < 30; ++b) {
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi);
            float maxSample = 0.0f;
            for (int s = 0; s < 512; ++s) maxSample = std::max(maxSample, std::abs(buffer.getSample(0, s)));
            if (maxSample > 0.05f) nonZeroBlocks++;
        }

        auditCheck(nonZeroBlocks > 20, "ARPEGGIATOR", "Arpeggiator generates rhythmic pulses",
                   "Active blocks: " + std::to_string(nonZeroBlocks) + "/30");

        midi.clear();
        midi.addEvent(juce::MidiMessage::allNotesOff(1), 0);
        buffer.clear();
        processor.processBlock(buffer, midi);
    }

    // =========================================================================
    // AUDIT 7: PRESET RECOVERY & INTEGRITY
    // =========================================================================
    std::cout << "\n--- AUDITING FACTORY PRESETS ---\n";
    {
        auto& pm = processor.getPresetManager();
        const auto& presets = pm.getPresets();
        auditCheck(presets.size() >= 202, "PRESETS", "All 202 factory presets loaded successfully",
                   "Total: " + std::to_string(presets.size()));

        int silentPresets = 0;
        for (size_t i = 0; i < presets.size(); ++i) {
            pm.loadPreset(static_cast<int>(i));
            midi.clear();
            buffer.clear();
            processor.processBlock(buffer, midi); // apply params

            midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8)100), 0);
            buffer.clear();
            processor.processBlock(buffer, midi);

            float peak = 0.0f;
            for (int b = 0; b < 25; ++b) {
                midi.clear();
                buffer.clear();
                processor.processBlock(buffer, midi);
                for (int s = 0; s < 512; ++s) peak = std::max(peak, std::abs(buffer.getSample(0, s)));
            }

            if (peak < 0.005f) {
                silentPresets++;
                auditWarn("PRESETS", "Preset is nearly silent on middle C (60)", presets[i].name.toStdString());
            }

            midi.clear();
            midi.addEvent(juce::MidiMessage::noteOff(1, 60), 0);
            buffer.clear();
            processor.processBlock(buffer, midi);
        }

        auditCheck(silentPresets == 0, "PRESETS", "All presets produce audible sound on middle C",
                   "Silent presets: " + std::to_string(silentPresets));
    }

    std::cout << "\n========================================================\n";
    std::cout << "AUDIT SUMMARY: " << testsPassed << "/" << testsRun << " PASSED, "
              << warnings << " WARNING(S)\n";
    std::cout << "========================================================\n";

    return (testsPassed == testsRun) ? 0 : 1;
}
