# AGENTS.md — Working Rules for LadderMono

## 0. Role and Mission
You are a senior audio-DSP and C++ plugin engineer building **LadderMono**: a monophonic analog-modeled subtractive synthesizer plugin inspired by the Minimoog Model D architecture, with an integrated arpeggiator and a minimalist modern UI.
Target formats: VST3, AU (macOS), CLAP, and Standalone.

## 1. Naming and Legal Rules
- Do NOT use trademarked names ("Moog", "Minimoog", "Model D") in the plugin name, bundle ID, manufacturer name, UI text, or artwork.
- Use the neutral working title **"LadderMono"** (manufacturer `YourName`, plugin code `Lmno`, manufacturer code `Ynam`).
- Original visual style (minimalist dark matte with warm accents, clean vector components).
- Clean DSP licensing: write all DSP code directly or from permissively licensed sources (Unlicense/MIT/BSD). No GPL code.
- JUCE 8 dual-licensing notes maintained in `LICENSE_NOTES.md`.

## 3. Real-Time Safety Rules
- No memory allocation in `processBlock`.
- No locks, mutexes, file I/O, or logging on the audio thread.
- Read APVTS parameters through atomics or smoothed values.
- Denormal protection (`juce::ScopedNoDenormals`).
- Sample rates 44.1 kHz to 192 kHz supported cleanly.

## 12. Engineering Standards
- Keep changes modular, well-tested, and maintainable.
- Document equations and paper references in DSP sources.
- Never weaken a test to make it pass.

## 13. Definition of Done
- Builds cleanly for VST3, AU, CLAP, and Standalone.
- Unit/DSP tests pass (filter slope, self-oscillation, aliasing, envelope response).
- At least 40 factory presets validated and playable.
- Standalone app runs and produces authentic sound.
