#include <iostream>
#include <cassert>
#include <cmath>
#include "PluginProcessor.h"
#include "PluginEditor.h"

int main()
{
    juce::ScopedJuceInitialiser_GUI guiInit;
    std::cout << "Running TestKeyHandling..." << std::endl;

    LadderMonoAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer emptyMidi;

    auto processBlocks = [&](int count) {
        for (int b = 0; b < count; ++b)
        {
            buffer.clear();
            processor.processBlock(buffer, emptyMidi);
        }
    };

    auto getBufferPeak = [&]() {
        float peak = 0.0f;
        for (int s = 0; s < 512; ++s)
            peak = std::max(peak, std::abs(buffer.getSample(0, s)));
        return peak;
    };

    // Test 1: Rapid retrigger note-off purge
    {
        std::cout << "  Test 1: Rapid retrigger note-off..." << std::endl;
        processor.handleNoteOn(60, 0.8f);
        processBlocks(5);
        processor.handleNoteOn(60, 0.9f);
        processBlocks(5);
        processor.handleNoteOff(60);

        // Process release phase (1.5 seconds)
        processBlocks(130);
        float peak = getBufferPeak();
        if (peak > 0.0001f)
        {
            std::cerr << "FAIL Test 1: Lingering audio after note-off on duplicate note, peak=" << peak << std::endl;
            return 1;
        }
        std::cout << "  PASS Test 1" << std::endl;
    }

    // Test 2: Polyphonic chord release
    {
        std::cout << "  Test 2: Polyphonic chord release..." << std::endl;
        if (auto* param = processor.getAPVTS().getParameter(LadderMono::ParamIDs::voices))
            param->setValueNotifyingHost(param->convertTo0to1(2.0f)); // 8-voice mode

        processBlocks(2);

        processor.handleNoteOn(60, 0.8f);
        processor.handleNoteOn(64, 0.8f);
        processor.handleNoteOn(67, 0.8f);
        processor.handleNoteOn(72, 0.8f);
        processBlocks(10);

        // Release one note
        processor.handleNoteOff(64);
        processBlocks(10);

        // Other notes must still be audible
        float peak = getBufferPeak();
        if (peak < 0.05f)
        {
            std::cerr << "FAIL Test 2: Poly chord stopped prematurely after releasing one note" << std::endl;
            return 1;
        }

        // Release remaining
        processor.handleNoteOff(60);
        processor.handleNoteOff(67);
        processor.handleNoteOff(72);

        processBlocks(150);
        peak = getBufferPeak();
        if (peak > 0.0001f)
        {
            std::cerr << "FAIL Test 2: Lingering audio in poly mode after all note offs, peak=" << peak << std::endl;
            return 1;
        }
        std::cout << "  PASS Test 2" << std::endl;
    }

    // Test 3: Voice mode switch while notes sounding
    {
        std::cout << "  Test 3: Voice mode switch during note hold..." << std::endl;
        if (auto* param = processor.getAPVTS().getParameter(LadderMono::ParamIDs::voices))
            param->setValueNotifyingHost(param->convertTo0to1(2.0f)); // 8-voice

        processBlocks(2);
        processor.handleNoteOn(60, 0.8f);
        processor.handleNoteOn(65, 0.8f);
        processor.handleNoteOn(70, 0.8f);
        processBlocks(5);

        // Switch to Mono (voices = 0)
        if (auto* param = processor.getAPVTS().getParameter(LadderMono::ParamIDs::voices))
            param->setValueNotifyingHost(param->convertTo0to1(0.0f));

        processBlocks(2);

        // Release notes
        processor.handleNoteOff(60);
        processor.handleNoteOff(65);
        processor.handleNoteOff(70);

        processBlocks(150);
        float peak = getBufferPeak();
        if (peak > 0.0001f)
        {
            std::cerr << "FAIL Test 3: Lingering audio after mode switch, peak=" << peak << std::endl;
            return 1;
        }
        std::cout << "  PASS Test 3" << std::endl;
    }

    // Test 4: Computer keyboard octave shift tracking and focus loss
    {
        std::cout << "  Test 4: Computer keyboard octave shift and focus loss..." << std::endl;
        std::unique_ptr<LadderMonoAudioProcessorEditor> editor(
            dynamic_cast<LadderMonoAudioProcessorEditor*>(processor.createEditor()));

        if (editor != nullptr)
        {
            // Simulate pressing 'A' (which triggers C4 = note 60 by default)
            juce::KeyPress keyA('a');
            editor->keyPressed(keyA);

            // Shift octave up with 'X'
            juce::KeyPress keyX('x');
            editor->keyPressed(keyX);

            // Loss of focus should safely release any held computer keys
            editor->focusLost(juce::Component::focusChangedDirectly);

            processBlocks(150);
            float peak = getBufferPeak();
            if (peak > 0.0001f)
            {
                std::cerr << "FAIL Test 4: Lingering audio after focus lost, peak=" << peak << std::endl;
                return 1;
            }
            std::cout << "  PASS Test 4" << std::endl;
            editor.reset();
        }
    }

    std::cout << "All Key Handling tests passed successfully!" << std::endl;
    return 0;
}
