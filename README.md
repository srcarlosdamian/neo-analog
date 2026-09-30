# LadderMono

A virtual-analog synthesizer plugin and standalone application inspired by the Minimoog Model D architecture, crafted with modern C++20, JUCE 8, and a sleek minimalist dark interface with an integrated poly-rhythmic arpeggiator and selectable 8-voice polyphony.

Shipped as **VST3**, **AU (Audio Unit)**, **CLAP**, and **Standalone application** for macOS and Windows.

---

## Features

- **3 Virtual-Analog Oscillators:**
  - Classic footage ranges: `LO, 32', 16', 8', 4', 2'`.
  - 6 Classic waveforms: Triangle, Shark-fin (saw-triangle blend), Sawtooth, Square, Wide Pulse, Narrow Pulse.
  - Band-limited anti-aliasing via PolyBLEP and PolyBLAMP with DC-offset correction.
  - Analog drift simulation with subtle detuning and pitch wander.
  - Free-running phase with optional note-on hard sync.
  - Oscillator 3 sub-audio LFO mode with keyboard tracking toggle.
- **Mixer Section:**
  - 5-channel summing: Osc 1, Osc 2, Osc 3, Noise, and External Feedback loop.
  - White and authentic Paul Kellet Pink noise generator.
  - Mixer Overdrive with asymmetric warm saturation curve (`Saturation.h`).
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
  - Exponential RC charge/discharge profiles calibrated to hardware panel timing.
- **Voice Architecture & Polyphony:**
  - Selectable **Mono**, **4 Voices**, and **8 Voices** polyphony with intelligent LRU voice stealing.
  - Automatic headroom scaling for chord polyphony without clipping.
  - Authentic Last-Note and Low-Note priority modes.
  - Constant-time exponential portamento (Glide).
- **Integrated Arpeggiator:**
  - Directional modes: Up, Down, Up/Down, Random, As Played.
  - Tempo-synced (1/4, 1/8, 1/8T, 1/16, 1/16T, 1/32) and Free Hz modes.
  - 1 to 4 Octave range, adjustable Gate length, and Latch/Hold mode.
- **73 Factory Presets across 9 Categories:**
  - **French Touch & Electro (10):** Squelchy distorted leads, punchy disco basses, dramatic brass stabs, and poly house keys.
  - **Indie & Vintage Pop (10):** Silky velvet basses, sparkling arps, combo organs, and dreamy nostalgic pads.
  - **1971 Electronic (8):** Wendy Carlos / Clockwork Orange inspired classical synthesis.
  - **Bass (12):** Classic, Sub, Rubber, Pedal, Funk Pluck, Growl, Feedback, etc.
  - **Lead (12):** Prog Portamento, Fusion, Screamer, Whistle, Pulse, Overdrive, etc.
  - **Brass/Pad (4):** Mono Brass, Soft Swell, Drone Pad, Hollow Pad.
  - **Keys (6):** Pluck Keys, Marimba, Clavinet, Well-Tempered Clavier, Bell FM.
  - **FX (10):** Laser, Wind, Kick, Snare, Tom, Siren, Self-Oscillating Sine, Helicopter, etc.
  - **Init (1):** Clean initialization template.
- **Minimalist Modern UI:**
  - Dark matte aesthetic with warm amber accents and custom vector knobs.
  - Categorized preset popup browser on preset click.
  - Pitch & Mod Wheels and an interactive 3-octave virtual keyboard.
  - Musical typing on computer keyboard (`A W S E D F T G H U J K`) with octave switching (`Z / X`).

---

## Installation & Formats

On macOS, binaries are automatically installed to:
- **VST3:** `~/Library/Audio/Plug-Ins/VST3/LadderMono.vst3`
- **AU:** `~/Library/Audio/Plug-Ins/Components/LadderMono.component` (validated with Apple `auval`)
- **CLAP:** `~/Library/Audio/Plug-Ins/CLAP/LadderMono.clap`
- **Standalone:** `build/LadderMono_artefacts/Standalone/LadderMono.app`

## Building from Source

```bash
# Configure with Ninja and CMake
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build all plugin formats and standalone app
cmake --build build --target LadderMono_Standalone LadderMono_VST3 LadderMono_AU LadderMono_CLAP

# Run automated DSP test suite (14/14 tests)
cmake --build build --target LadderMonoTests && ./build/LadderMonoTests

# Run Offline Audio Renderer CLI
cmake --build build --target OfflineRenderer
./build/OfflineRenderer --preset "French Touch Overdrive Lead" --out "lead.wav" --duration 3.0

# Analyze spectrum
python3 tools/plot_spectrum.py lead.wav
```

## Validation

- **AU Validation (`auval`):** `auval -v aumu Lmno Ynam` -> **AU VALIDATION SUCCEEDED.**
- **DSP Test Suite (`ctest`):** All 14/14 unit tests pass.
- **Audio Output Verification (`TestPresetAudio`):** 100% of the 73 presets verified to produce healthy signal.

---

## License & Third-Party Notice
See `LICENSE_NOTES.md` and `THIRD_PARTY.md` for complete licensing information.
