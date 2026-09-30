#!/usr/bin/env python3
"""
LadderMono Audio Spectrum Analyzer CLI
Analyzes a WAV file produced by OfflineRenderer and outputs frequency spectrum statistics.
"""

import sys
import wave
import struct
import math

def analyze_wav(filename):
    with wave.open(filename, 'r') as w:
        nchannels = w.getnchannels()
        sampwidth = w.getsampwidth()
        framerate = w.getframerate()
        nframes = w.getnframes()
        frames = w.readframes(min(nframes, framerate * 2))

    if sampwidth == 2:
        fmt = f"<{len(frames)//2}h"
        max_val = 32768.0
    elif sampwidth == 3:
        # 24-bit PCM
        samples = []
        for i in range(0, len(frames), 3 * nchannels):
            b = frames[i:i+3]
            val = int.from_bytes(b, byteorder='little', signed=True)
            samples.append(val / 8388608.0)
        return samples, framerate
    elif sampwidth == 4:
        fmt = f"<{len(frames)//4}i"
        max_val = 2147483648.0
    else:
        raise ValueError(f"Unsupported sample width: {sampwidth}")

    raw = struct.unpack(fmt, frames)
    # Take first channel
    samples = [raw[i] / max_val for i in range(0, len(raw), nchannels)]
    return samples, framerate

def compute_fft(samples, sample_rate, n_fft=2048):
    if len(samples) < n_fft:
        samples = samples + [0.0] * (n_fft - len(samples))
    else:
        # Take steady state region (e.g. from 20% to 20% + n_fft)
        start = len(samples) // 4
        samples = samples[start:start + n_fft]

    # Apply Hann window
    windowed = [samples[i] * 0.5 * (1.0 - math.cos(2.0 * math.pi * i / (n_fft - 1))) for i in range(n_fft)]

    # DFT for audio frequencies up to 10 kHz
    freq_bins = []
    magnitudes = []
    
    # Analyze 100 logarithmic bins from 20 Hz to 16 kHz
    num_bins = 60
    f_min = 20.0
    f_max = min(16000.0, sample_rate / 2.0)

    for b in range(num_bins):
        freq = f_min * ((f_max / f_min) ** (b / (num_bins - 1)))
        k = freq * n_fft / sample_rate
        # Goertzel / discrete sum
        real = sum(windowed[n] * math.cos(2.0 * math.pi * k * n / n_fft) for n in range(n_fft))
        imag = sum(windowed[n] * math.sin(2.0 * math.pi * k * n / n_fft) for n in range(n_fft))
        mag = math.sqrt(real * real + imag * imag) / (n_fft / 4.0)
        db = 20.0 * math.log10(max(1e-6, mag))
        freq_bins.append(freq)
        magnitudes.append(db)

    return freq_bins, magnitudes

def print_ascii_spectrum(freq_bins, magnitudes, preset_name=""):
    print(f"\n=======================================================")
    print(f" SPECTRUM ANALYSIS : {preset_name}")
    print(f"=======================================================")
    print(f" Frequency (Hz)  | Level (dBFS) | Visualization")
    print(f"-----------------+--------------+----------------------")

    # Pick representative frequency landmarks
    landmarks = [55, 110, 220, 261, 440, 523, 880, 1046, 1760, 2093, 3520, 5000, 8000, 12000]
    
    for target in landmarks:
        idx = min(range(len(freq_bins)), key=lambda i: abs(freq_bins[i] - target))
        f = freq_bins[idx]
        db = magnitudes[idx]
        # Bar chart from -60 dB to 0 dB (width 30 chars)
        norm = max(0.0, min(1.0, (db + 60.0) / 60.0))
        bars = "#" * int(norm * 30)
        print(f"  {f:8.1f} Hz   |   {db:6.1f} dB  | {bars}")

    max_db = max(magnitudes)
    peak_idx = magnitudes.index(max_db)
    print(f"-------------------------------------------------------")
    print(f" Dominant Fundamental Peak: {freq_bins[peak_idx]:.1f} Hz at {max_db:.1f} dBFS")
    print(f"=======================================================\n")

if __name__ == "__main__":
    wav_path = sys.argv[1] if len(sys.argv) > 1 else "test_french_lead.wav"
    try:
        samples, sr = analyze_wav(wav_path)
        bins, mags = compute_fft(samples, sr)
        print_ascii_spectrum(bins, mags, preset_name=wav_path)
    except Exception as e:
        print(f"Error analyzing WAV: {e}")
