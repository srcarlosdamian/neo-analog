# Third-Party Dependencies and References

## Dependencies
1. **JUCE 8** (https://github.com/juce-framework/JUCE)
   - License: AGPLv3 / Commercial dual license.
   - Used for plugin wrapper (VST3, AU, Standalone), GUI framework, and audio device management.
2. **clap-juce-extensions** (https://github.com/free-audio/clap-juce-extensions)
   - License: MIT
   - Used for CLAP plugin format export from JUCE targets.

## DSP References and Mathematical Models
1. **TPT / Zero-Delay Feedback Filter Structure**
   - Reference: Vadim Zavalishin, *The Art of VA Filter Design* (Native Instruments, 2012–2020).
2. **Nonlinear Analog Modeling of Ladder Filters**
   - Reference: Antti Huovilainen, *Non-Linear Digital Implementation of the Moog Ladder Filter*, DAFx-04.
   - Reference: H. Oyama, *Quantifying Nonlinear Behavior in Digital Moog Ladder Filters*, DAFx-26.
3. **Band-Limited Oscillators (PolyBLEP & PolyBLAMP)**
   - Reference: Välimäki & Huovilainen, *Oscillator and Filter Algorithms for Virtual Analog Synthesis*, Computer Music Journal, 2006.
   - Reference: Esqueda, Välimäki, & Bilbao, *Rounding Corners with BLAMP*, IEEE SPM, 2016.
4. **Pink Noise Generation**
   - Reference: Paul Kellet's filtered pink noise algorithm (public domain).
