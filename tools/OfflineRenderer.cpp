#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "../src/PluginProcessor.h"
#include <juce_audio_formats/juce_audio_formats.h>

int main(int argc, char* argv[])
{
    std::string presetName = "Classic Bass";
    std::string outputFile = "rendered_output.wav";
    int midiNote = 60; // C4
    float durationSec = 3.0f;
    double bpm = 120.0;
    double sampleRate = 44100.0;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--preset" && i + 1 < argc)
            presetName = argv[++i];
        else if (arg == "--out" && i + 1 < argc)
            outputFile = argv[++i];
        else if (arg == "--note" && i + 1 < argc)
            midiNote = std::stoi(argv[++i]);
        else if (arg == "--duration" && i + 1 < argc)
            durationSec = std::stof(argv[++i]);
        else if (arg == "--bpm" && i + 1 < argc)
            bpm = std::stod(argv[++i]);
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "LadderMono Offline Renderer CLI\n"
                      << "Usage: OfflineRenderer [options]\n"
                      << "Options:\n"
                      << "  --preset <name>       Preset name (default: \"Classic Bass\")\n"
                      << "  --out <filename.wav>  Output WAV file path (default: \"rendered_output.wav\")\n"
                      << "  --note <midi>         MIDI note to play (default: 60)\n"
                      << "  --duration <sec>      Duration in seconds (default: 3.0)\n"
                      << "  --bpm <bpm>           Tempo in BPM (default: 120.0)\n";
            return 0;
        }
    }

    std::cout << "=== LadderMono Offline Audio Renderer ===\n";
    std::cout << "Target Preset : " << presetName << "\n";
    std::cout << "Output File   : " << outputFile << "\n";
    std::cout << "MIDI Note     : " << midiNote << "\n";
    std::cout << "Duration      : " << durationSec << " s\n";
    std::cout << "BPM           : " << bpm << "\n";

    LadderMonoAudioProcessor processor;
    const int blockSize = 512;
    processor.prepareToPlay(sampleRate, blockSize);

    auto& pm = processor.getPresetManager();
    const auto& presets = pm.getPresets();
    bool presetFound = false;

    for (size_t i = 0; i < presets.size(); ++i)
    {
        if (presets[i].name.equalsIgnoreCase(juce::String(presetName)))
        {
            pm.loadPreset(static_cast<int>(i));
            std::cout << "Found preset in category [" << presets[i].category << "]\n";
            presetFound = true;
            break;
        }
    }

    if (!presetFound)
    {
        std::cerr << "Warning: Preset '" << presetName << "' not found. Using default patch.\n";
    }

    juce::File outFile(outputFile);
    outFile.deleteFile();

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatWriter> writer;
    if (auto* wavFormat = formatManager.findFormatForFileExtension("wav"))
    {
        std::unique_ptr<juce::OutputStream> outStream = outFile.createOutputStream();
        if (outStream != nullptr)
        {
            writer = wavFormat->createWriterFor(outStream,
                                                juce::AudioFormatWriterOptions()
                                                    .withSampleRate(sampleRate)
                                                    .withNumChannels(2)
                                                    .withBitsPerSample(24));
        }
    }

    if (writer == nullptr)
    {
        std::cerr << "Error: Could not open output file for writing: " << outputFile << "\n";
        return 1;
    }

    int totalSamples = static_cast<int>(durationSec * sampleRate);
    int noteOffSample = static_cast<int>((durationSec * 0.75f) * sampleRate); // Release after 75% of duration

    juce::AudioBuffer<float> blockBuffer(2, blockSize);
    int samplesRendered = 0;

    float peakAmp = 0.0f;
    double sumSquared = 0.0;

    while (samplesRendered < totalSamples)
    {
        int currentBlockSize = std::min(blockSize, totalSamples - samplesRendered);
        blockBuffer.clear();

        juce::MidiBuffer midi;
        if (samplesRendered == 0)
        {
            midi.addEvent(juce::MidiMessage::noteOn(1, midiNote, (juce::uint8)100), 0);
        }

        if (samplesRendered <= noteOffSample && (samplesRendered + currentBlockSize) > noteOffSample)
        {
            int offset = noteOffSample - samplesRendered;
            midi.addEvent(juce::MidiMessage::noteOff(1, midiNote), offset);
        }

        processor.processBlock(blockBuffer, midi);

        for (int ch = 0; ch < 2; ++ch)
        {
            const float* channelData = blockBuffer.getReadPointer(ch);
            for (int s = 0; s < currentBlockSize; ++s)
            {
                float v = std::abs(channelData[s]);
                peakAmp = std::max(peakAmp, v);
                sumSquared += channelData[s] * channelData[s];
            }
        }

        writer->writeFromAudioSampleBuffer(blockBuffer, 0, currentBlockSize);
        samplesRendered += currentBlockSize;
    }

    writer.reset(); // Flush & close

    double rms = std::sqrt(sumSquared / (totalSamples * 2));
    float peakDb = juce::Decibels::gainToDecibels(peakAmp);
    float rmsDb = juce::Decibels::gainToDecibels(static_cast<float>(rms));

    std::cout << "\nRendering Complete!\n";
    std::cout << "Peak Level : " << std::fixed << std::setprecision(2) << peakDb << " dBFS (" << peakAmp << ")\n";
    std::cout << "RMS Level  : " << std::fixed << std::setprecision(2) << rmsDb << " dBFS\n";
    std::cout << "WAV File   : " << outFile.getFullPathName() << " (" << (outFile.getSize() / 1024) << " KB)\n";

    return 0;
}
