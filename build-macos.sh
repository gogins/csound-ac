#!/bin/bash
clear
echo "Building all for macOS..."
mkdir -p build-macos
cd build-macos
rm -f CMakeCache.txt
cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/homebrew -Wno-dev .. "$@"
make -j6 VERBOSE=1
sudo cpack -V -G ZIP
# sudo make install
echo "Finished building all for macOS."
