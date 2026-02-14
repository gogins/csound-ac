#!/bin/bash
clear
echo "Building all for macOS..."
mkdir -p build-macos
cd build-macos
rm -f CMakeCache.txt

# Ensure generated documentation and in-tree dependency build outputs are writable.
# (A previous sudo build can leave root-owned files behind.)
mkdir -p ../doc/latex ../doc/html
sudo chown -R michaelgogins:staff /Users/michaelgogins/csound-ac/build-macos
sudo chown -R michaelgogins:staff /Users/michaelgogins/csound-ac/doc/latex /Users/michaelgogins/csound-ac/doc/html
sudo chown -R michaelgogins:staff /Users/michaelgogins/csound-ac/dependencies/libmusicxml/build

cmake -DCSOUND_AC_PREFER_LOCAL_DEPS:BOOL=ON -DCMAKE_INSTALL_PREFIX:PATH=/opt/homebrew -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev .. "$@"
# cmake -DCMAKE_CXX_FLAGS="-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" -DCMAKE_INSTALL_PREFIX:PATH=/opt/homebrew -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev .. "$@"
make -j6 VERBOSE=1
cpack -V -G ZIP
sudo make install
echo "Finished building all for macOS."
