#!/usr/bin/env bash
set -e

echo "=== Building LadderMono on macOS ==="
export PATH="/Users/damian/.local/bin:/opt/homebrew/bin:/usr/local/bin:$PATH"

if ! command -v cmake &> /dev/null; then
    echo "Error: cmake not found in PATH"
    exit 1
fi

cmake -B build -G Ninja
cmake --build build --target LadderMono_Standalone LadderMono_VST3 LadderMono_AU LadderMono_CLAP LadderMonoTests

echo "Running tests..."
./build/LadderMonoTests

echo "Installing plugins to ~/Library/Audio/Plug-Ins/..."
mkdir -p ~/Library/Audio/Plug-Ins/VST3 ~/Library/Audio/Plug-Ins/Components ~/Library/Audio/Plug-Ins/CLAP
cp -R build/LadderMono_artefacts/VST3/LadderMono.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R build/LadderMono_artefacts/AU/LadderMono.component ~/Library/Audio/Plug-Ins/Components/
cp -R build/LadderMono_artefacts/CLAP/LadderMono.clap ~/Library/Audio/Plug-Ins/CLAP/

echo "Build and installation complete!"
