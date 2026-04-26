#!/bin/zsh
set -euo pipefail

brew update

brew install \
    bison \
    boost \
    cmake \
    doxygen \
    eigen \
    fftw \
    flex \
    fluidsynth \
    git \
    googletest \
    graphviz \
    jack \
    liblo \
    libsndfile \
    ninja \
    node \
    pkg-config \
    portaudio \
    portmidi \
    python@3.12 \
    swig \
    wget 

cd "$HOME"

npm install -g node-addon-api
npm install -g cmake-js

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


