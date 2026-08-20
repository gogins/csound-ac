#!/bin/bash

set -euo pipefail

CSOUND_SOURCE="$HOME/csound"
CSOUND_BUILD="$CSOUND_SOURCE/build"
CSOUND_FRAMEWORK="$HOME/Library/Frameworks/CsoundLib64.framework/Versions/7.0/CsoundLib64"

echo
echo "============================================================"
echo "Building and installing Csound 7"
echo "Source: $CSOUND_SOURCE"
echo "Build:  $CSOUND_BUILD"
echo "============================================================"
echo

if [[ ! -f "$CSOUND_SOURCE/CMakeLists.txt" ]]; then
    echo "ERROR: Csound source tree not found at:"
    echo "  $CSOUND_SOURCE"
    exit 1
fi

#
# Start with a completely clean CMake configuration. This is important
# because an old Debug configuration may have cached AddressSanitizer.
#
echo "Removing previous build directory..."
rm -rf "$CSOUND_BUILD"

#
# Configure.
#
# USE_ASA is explicitly disabled because an ASan-linked Csound framework
# cannot safely be dlopen()ed into an ordinary Python process.
#
echo
echo "Configuring Csound..."
cmake \
    -S "$CSOUND_SOURCE" \
    -B "$CSOUND_BUILD" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DUSE_ASA=OFF

#
# Verify the CMake cache before doing the build.
#
echo
echo "Checking CMake configuration..."

BUILD_TYPE="$(
    sed -n 's/^CMAKE_BUILD_TYPE:STRING=//p' \
        "$CSOUND_BUILD/CMakeCache.txt"
)"

USE_ASA="$(
    sed -n 's/^USE_ASA:BOOL=//p' \
        "$CSOUND_BUILD/CMakeCache.txt"
)"

echo "CMAKE_BUILD_TYPE = $BUILD_TYPE"
echo "USE_ASA          = $USE_ASA"

if [[ "$BUILD_TYPE" != "RelWithDebInfo" ]]; then
    echo "ERROR: CMAKE_BUILD_TYPE is not RelWithDebInfo."
    exit 1
fi

if [[ "$USE_ASA" != "OFF" ]]; then
    echo "ERROR: USE_ASA is not OFF."
    exit 1
fi

#
# Build using all available CPU cores.
#
JOBS="$(sysctl -n hw.ncpu)"

echo
echo "Building Csound with $JOBS parallel jobs..."
cmake --build "$CSOUND_BUILD" --parallel "$JOBS"

#
# Install. The Csound macOS configuration installs the framework into
# ~/Library/Frameworks.
#
echo
echo "Installing Csound..."
sudo cmake --install "$CSOUND_BUILD"

#
# Verify the installed framework.
#
echo
echo "Checking installed Csound framework..."

if [[ ! -f "$CSOUND_FRAMEWORK" ]]; then
    echo "ERROR: Installed Csound framework not found at:"
    echo "  $CSOUND_FRAMEWORK"
    exit 1
fi

if otool -L "$CSOUND_FRAMEWORK" | grep -qi asan; then
    echo
    echo "ERROR: Installed CsoundLib64 is still linked against AddressSanitizer:"
    otool -L "$CSOUND_FRAMEWORK" | grep -i asan
    exit 1
fi

echo "No AddressSanitizer dependency found."

echo
echo "Installed framework:"
echo "  $CSOUND_FRAMEWORK"

echo
echo "Csound 7 build and installation completed successfully."
echo