#!/bin/bash
clear
echo "Building all for macOS..."
mkdir -p build-macos
cd build-macos
rm -f CMakeCache.txt
# Change this for yourself or comment it out.
sudo chown -R michaelgogins:staff /Users/michaelgogins/csound-ac/build-macos
cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/homebrew -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev .. "$@"
make -j6 VERBOSE=1
sudo cpack -V -G ZIP
sudo make install
echo "Finished building all for macOS."