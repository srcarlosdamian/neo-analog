# LadderMono

A monophonic virtual-analog synthesizer inspired by the legendary Minimoog Model D architecture, crafted with modern C++20, JUCE 8, and a sleek minimalist dark interface with an integrated poly-rhythmic arpeggiator.

Shipped as **VST3**, **AU (Audio Unit)**, **CLAP**, and **Standalone application** for macOS and Windows.

---

## Features

- **3 Virtual-Analog Oscillators:**
  - Classic footage ranges: `LO, 32', 16', 8', 4', 2'`.
  - 6 Classic waveforms: Triangle, Shark-fin (saw-triangle blend), Sawtooth, Square, Wide Pulse, Narrow Pulse.
  - Band-limited anti-aliasing via PolyBLEP and PolyBLAMP.
  - Analog drift simulation with subtle detuning and pitch wander.
  - Free-running phase with optional note-on hard sync.
  - Oscillator 3 sub-audio LFO mode with keyboard tracking toggle.
- **Mixer Section:**
  - 5-channel summing: Osc 1, Osc 2, Osc 3, Noise, and External Feedback loop.
  - White and authentic Paul Kellet Pink noise generator.
  - Mixer Overdrive with asymmetric warm saturation curve.
  - Built-in output-to-input feedback loop reproducing the famous Minimoog headphone-out trick.
- **24 dB/oct 4-Pole Ladder Filter:**
  - Zero-Delay Feedback (ZDF) / Topology-Preserving Transform (TPT) core.
  - Hyperbolic tangent (tanh) stage saturation capturing authentic early-stage nonlinearities.
  - Genuine self-oscillation tracking keyboard pitch at emphasis 9.0–10.0.
  - Bass loss compensation parameter (`Bass Comp`).
  - Key tracking switches: 1/3, 2/3, and full (1.0).
  - 2x internal oversampling for low aliasing and high stability.
- **Dual Analog-Style Envelopes:**
  - Filter Contour & Loudness Contour (Attack, Decay, Sustain).
  - Decay-as-Release switch for authentic Minimoog envelope behavior.
  - Exponential RC charge/discharge profiles.
- **Integrated Arpeggiator:**
  - Directional modes: Up, Down, Up/Down, Random, As Played.
  - Tempo-synced (1/4, 1/8, 1/8T, 1/16, 1/16T, 1/32) and Free Hz modes.
  - 1 to 4 Octave range, adjustable Gate length, and Latch/Hold mode.
- **Performance & Navigation:**
  - Authentic low-note priority note stack with legato handling.
  - Exponential constant-time portamento (Glide).
  - Pitch & Mod Wheels and an interactive 3-octave virtual keyboard.
  - 43 embedded Factory Presets with instant recall and category browsing.

---

## Installation & Formats

On macOS, binaries are automatically installed to:
- **VST3:** `~/Library/Audio/Plug-Ins/VST3/LadderMono.vst3`
- **AU:** `~/Library/Audio/Plug-Ins/Components/LadderMono.component`
- **CLAP:** `~/Library/Audio/Plug-Ins/CLAP/LadderMono.clap`
- **Standalone:** `build/LadderMono_artefacts/Standalone/LadderMono.app`

## Building from Source

```bash
# Configure with Ninja and CMake
cmake -B build -G Ninja

# Build all formats
cmake --build build --target LadderMono_Standalone LadderMono_VST3 LadderMono_AU LadderMono_CLAP

# Run automated DSP test suite
cmake --build build --target LadderMonoTests && ./build/LadderMonoTests
```

## License & Third-Party Notice
See `LICENSE_NOTES.md` and `THIRD_PARTY.md` for complete licensing information.
