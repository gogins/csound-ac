#!/bin/zsh
set -euo pipefail

brew update

brew install \
    git cmake ninja pkg-config \
    bison flex \
    python@3.12 swig \
    boost eigen \
    doxygen \
    googletest \
    graphviz \
    jack \
    libsndfile portaudio portmidi \
    liblo fftw fluidsynth

cd "$HOME"

clone_if_missing()
{
    local repo_url="$1"
    local dir_name="$2"

    if [ ! -d "$dir_name" ]; then
        git clone "$repo_url" "$dir_name"
    fi
}

clone_if_missing https://github.com/gogins/cloud-5.git cloud-5
clone_if_missing https://github.com/csound/csound.git csound
clone_if_missing https://github.com/gogins/csound-ac.git csound-ac
clone_if_missing https://github.com/gogins/csound-cxx-opcodes.git csound-cxx-opcodes
clone_if_missing https://github.com/gogins/csound-nwjs.git csound-nwjs
clone_if_missing https://github.com/gogins/csound-vst3.git csound-vst3
clone_if_missing https://github.com/gogins/csound-vst3-opcodes.git csound-vst3-opcodes
clone_if_missing https://github.com/gogins/csound-wasm.git csound-wasm
clone_if_missing https://github.com/gogins/gogins.github.io.git gogins.github.io

echo "Bootstrap complete."
echo "Next:"
echo "  1. Build Csound 6 into ~/opt/csound6"
echo "  2. Build Csound 7 into ~/opt/csound7"
echo "  3. source ~/.zshrc"
echo "  4. use_csound6" only in new shell for Csound 6 compatbility.


