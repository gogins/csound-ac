#!/bin/bash
clear
echo "Building all for macOS..."
mkdir -p build-macos
cd build-macos
rm -f CMakeCache.txt
cmake -Wno-dev .. -DCMAKE_PREFIX_PATH=/usr/local:/usr
make -j6 VERBOSE=1
sudo cpack -V -G ZIP
sudo make install
echo "Finished building all for macOS."
