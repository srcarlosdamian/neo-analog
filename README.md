# Neo Analog

**Neo Analog** is a vintage-modeled subtractive synthesizer plugin and standalone application inspired by classic analog ladder filter architecture. Crafted in modern C++20 with JUCE 8, it combines rich analog warmth, zero-delay feedback (ZDF) 24 dB/oct 4-pole ladder filtering, selectable 8-voice polyphony, an integrated multi-mode arpeggiator, and an authentic vintage walnut-and-brushed-metal aesthetic.

Shipped as **VST3**, **AU (Audio Unit)**, **CLAP**, and **Standalone application** for macOS and Windows.

---

## Highlights

- **Vintage Hardware Aesthetics:**
  - Procedural walnut wood chassis with beveled borders and chassis screws.
  - Anodized brushed aluminum faceplate with 6 structured control panels (`CONTROLLERS`, `ARPEGGIATOR`, `OSCILLATOR BANK`, `MIXER`, `MODIFIERS`, `OUTPUT`).
  - Vintage skirted knobs with 11 clear tick marks and spun-aluminum inserts.
  - 3D tactile rocker switches (Mixer Blue, Keyboard/Mod Orange, Noise/Decay Ivory).
  - Glowing ruby jewel pilot lamp and overload indicator.
  - Embossed brass nameplate badge (`N E O   A N A L O G`).
  - Dual fluted pitch & modulation wheels with center detents.
  - 44-key ivory & ebony virtual keyboard with authentic red felt damper strip.
  - Musical typing on computer keyboard (`A W S E D F T G Y H U J K`) with octave shift (`Z / X`) and alert-free typing.

- **3 Virtual-Analog Oscillators:**
  - Footage ranges: `LO, 32', 16', 8', 4', 2'`.
  - 6 Classic waveforms: Triangle, Shark-fin (saw-triangle blend), Sawtooth, Square, Wide Pulse, Narrow Pulse.
  - Band-limited anti-aliasing via PolyBLEP and PolyBLAMP with DC-offset correction.
  - Temperature/component analog drift simulation with subtle detuning and pitch wander.
  - Free-running phase with optional note-on hard sync.
  - Oscillator 3 sub-audio LFO mode with keyboard tracking toggle.

- **Mixer Section:**
  - 5-channel summing: Osc 1, Osc 2, Osc 3, Noise, and External Feedback loop.
  - White and authentic Paul Kellet Pink noise generator.
  - Mixer Overdrive with asymmetric warm saturation curve (`Saturation.h`).
  - Built-in output-to-input feedback loop reproducing the famous headphone-out feedback trick.

- **24 dB/oct 4-Pole Ladder Filter:**
  - Zero-Delay Feedback (ZDF) / Topology-Preserving Transform (TPT) core.
  - Hyperbolic tangent (tanh) stage saturation capturing authentic early-stage nonlinearities.
  - Genuine self-oscillation tracking keyboard pitch at emphasis 9.0–10.0.
  - Bass loss compensation parameter (`Bass Comp`).
  - Key tracking switches: 1/3, 2/3, and full (1.0).
  - 2x internal oversampling for low aliasing and high stability.

- **Dual Analog-Style Envelopes:**
  - Filter Contour & Loudness Contour (Attack, Decay, Sustain).
  - Decay-as-Release switch for authentic envelope behavior.
  - Exponential RC charge/discharge profiles calibrated to musical response with zero-leak silence on key release.

- **Voice Architecture & Polyphony:**
  - Selectable **Mono**, **4 Voices**, and **8 Voices** polyphony with intelligent LRU voice stealing.
  - Automatic headroom scaling for chord polyphony without clipping.
  - Authentic Last-Note, Low-Note, and High-Note priority modes.
  - Constant-time exponential portamento (Glide).

- **Integrated Arpeggiator:**
  - Directional modes: Up, Down, Up/Down, Random, As Played.
  - Tempo-synced (1/4, 1/8, 1/8T, 1/16, 1/16T, 1/32) and Free Hz modes.
  - 1 to 4 Octave range, adjustable Gate length, and Latch/Hold mode.

- **200 Factory Presets across 18 Curated Categories:**
  - **Bass (12):** Classic, Sub, Rubber, Pedal, Funk Pluck, Growl, Feedback, etc.
  - **Lead (12):** Portamento, Fusion, Screamer, Whistle, Pulse, Overdrive, etc.
  - **Keys (6):** Pluck Keys, Marimba, Clavinet, Well-Tempered Clavier, Bell FM.
  - **Brass/Pad (4):** Mono Brass, Brass Swell, Drone Pad, Hollow Pad.
  - **French Touch & Electro (20):** Squelchy overdriven leads, filtered house chords, Da Funk reso riffs, Homework acid, and Cross distortion stabs.
  - **Electroclash & Dark Wave (14):** Driving night cruiser basses, rapid-fire arps, modular techno sequences, and raw witch-house chipped saws.
  - **Pop & Funk 1982 (10):** Punchy studio funk basses, walking lines, quirky vocal leads, and horror brass stabs.
  - **Hip-Hop & Lo-Fi Beats (19):** Dusty vinyl Rhodes, melted tape keys, quirky flutes, gritty comic-book brass, and deep sub basses.
  - **Psych & Bedroom Pop (27):** Fuzz basses, woozy tape keys, bubbling shimmer arps, pastel bedroom keys, and ethereal dream pads.
  - **Fantasy & Chiptune (5):** Fairy fountain arps, lost forest leads, dungeon sub basses, and ancient temple swells.
  - **Neon Noir & Italo Disco (8):** Glassy poly keys, dark shadow basses, driving 16th disco arps, and emotive singing leads.
  - **80s Cinema & Disco (9):** Hypnotic arpeggios, driving synth basses, and soaring melodic leads.
  - **Cinematic Cyber Noir (5):** Expressive brass leads, lush ambient pads, and cosmic sequencers.
  - **1971 Baroque Electronic (20):** Baroque clarino trumpets, double reeds, pipe organs, cello continuos, and dystopian fanfares.
  - **Digital Grid & Cyberpunk (5):** Monolith sub-basses, arena leads, and ambient sci-fi swells.
  - **Space Pop & Downtempo (4):** Electric lounge piano, star gazing pulses, and nature soundscapes.
  - **FX (10):** Laser, Wind, Kick, Snare, Tom, Siren, Self-Oscillating Sine, Helicopter, etc.
  - **Init (1):** Clean starting template.

---

## Building from Source

### Prerequisites
- CMake 3.22 or higher
- C++20 compliant compiler (Apple Clang 15+, GCC 12+, or MSVC 2022)
- Git (with submodule support)

### Clone & Build
```bash
git clone --recursive https://github.com/srcarlosdamian/neo-analog.git
cd neo-analog

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

Built artifacts will be placed in `build/LadderMono_artefacts/`:
- **Standalone:** `Standalone/Neo Analog.app` (macOS) / `Neo Analog.exe` (Windows)
- **VST3:** `VST3/Neo Analog.vst3`
- **Audio Unit:** `AU/Neo Analog.component` (macOS)
- **CLAP:** `CLAP/Neo Analog.clap`

---

## License & Legal Notice
- **Neo Analog** is an independent project by Neo.
- All DSP code is cleanly written from first-principles analog circuit modeling or permissively licensed open sources.
- No protected trademarks or copyrighted song/artist names are used.
