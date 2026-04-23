#!/bin/zsh
set -euo pipefail

# Build and install Csound 7 from ~/csound
# Build directory: ~/csound/build-macos
# Use the fixed macOS custom CMake file.
# Install executables under /usr/local/bin and related default prefix paths.
# Sign the installed framework and the csound executable.

csound_src="$HOME/csound"
csound_build="$csound_src/build-macos"
csound_ref="${1:-develop}"

cmake_bin="/opt/homebrew/bin/cmake"
bison_bin="/opt/homebrew/opt/bison/bin/bison"
flex_bin="/opt/homebrew/opt/flex/bin/flex"
no_asan_cmake="$HOME/cmake/no_asan.cmake"
custom_cmake="$HOME/cmake/custom-osx-release.cmake"

codesign_identity="Developer ID Application: Michael Gogins (9UX792D3V9)"
framework_path="/Library/Frameworks/CsoundLib64.framework"
framework_binary="$framework_path/Versions/Current/CsoundLib64"
binary_path="/usr/local/bin/csound"

if [ ! -d "$csound_src/.git" ]; then
    echo "Error: $csound_src is not a git repository."
    echo "Clone it first: git clone https://github.com/csound/csound.git ~/csound"
    exit 1
fi

if [ ! -x "$cmake_bin" ]; then
    echo "Error: cmake not found at $cmake_bin"
    exit 1
fi

if [ ! -x "$bison_bin" ]; then
    echo "Error: bison not found at $bison_bin"
    exit 1
fi

if [ ! -x "$flex_bin" ]; then
    echo "Error: flex not found at $flex_bin"
    exit 1
fi

if [ ! -f "$no_asan_cmake" ]; then
    echo "Error: no_asan.cmake not found at $no_asan_cmake"
    exit 1
fi

if [ ! -f "$custom_cmake" ]; then
    echo "Error: fixed custom CMake file not found at $custom_cmake"
    exit 1
fi

if ! security find-identity -v -p codesigning | grep -Fq "$codesign_identity"; then
    echo "Error: code signing identity not found in keychain:"
    echo "  $codesign_identity"
    exit 1
fi

cd "$csound_src"
git fetch --tags --prune
git checkout "$csound_ref"
git pull --ff-only || true

rm -rf "$csound_build"
mkdir -p "$csound_build"

"$cmake_bin" -S "$csound_src" -B "$csound_build" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
    -DCS_FRAMEWORK_DEST:PATH=/Library/Frameworks \
    -DCUSTOM_CMAKE="$custom_cmake" \
    -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="$no_asan_cmake" \
    -DBUILD_TESTS=OFF \
    -DBISON_EXECUTABLE="$bison_bin" \
    -DFLEX_EXECUTABLE="$flex_bin"
    
echo "Checking generated install rules for home-directory targets..."
grep -n "$HOME\|/Users/" "$csound_build/cmake_install.cmake" || true
echo

"$cmake_bin" --build "$csound_build" -j"$(sysctl -n hw.ncpu)"
sudo "$cmake_bin" --install "$csound_build"

user_framework="$HOME/Library/Frameworks/CsoundLib64.framework"
system_framework="/Library/Frameworks/CsoundLib64.framework"

if [ -d "$user_framework" ] && [ ! -d "$system_framework" ]; then
    sudo mkdir -p /Library/Frameworks
    sudo mv "$user_framework" "$system_framework"
elif [ -d "$user_framework" ] && [ -d "$system_framework" ]; then
    echo "Removing existing system framework and replacing it with the newly installed one."
    sudo rm -rf "$system_framework"
    sudo mv "$user_framework" "$system_framework"
fi

# Remove stale per-user framework rpath from the installed executable, if present,
# and ensure the system framework location is in the runtime search path.
if [ -x "$binary_path" ]; then
    existing_rpaths="$(otool -l "$binary_path" | awk '
        $1 == "cmd" && $2 == "LC_RPATH" { in_rpath=1; next }
        in_rpath && $1 == "path" { print $2; in_rpath=0 }
    ')"

    if echo "$existing_rpaths" | grep -Fxq "$HOME/Library/Frameworks"; then
        sudo install_name_tool -delete_rpath "$HOME/Library/Frameworks" "$binary_path"
    fi

    if ! echo "$existing_rpaths" | grep -Fxq "/Library/Frameworks"; then
        sudo install_name_tool -add_rpath "/Library/Frameworks" "$binary_path"
    fi
fi

if [ ! -d "$framework_path" ]; then
    echo "Warning: installed framework not found at $framework_path"
else
    if [ -e "$framework_binary" ]; then
        sudo codesign --force --timestamp --sign "$codesign_identity" "$framework_binary"
    fi
    sudo codesign --force --deep --timestamp --sign "$codesign_identity" "$framework_path"
    codesign --verify --deep --strict --verbose=2 "$framework_path"
fi

if [ ! -x "$binary_path" ]; then
    echo "Warning: installed binary not found at $binary_path"
else
    sudo codesign --force --timestamp --sign "$codesign_identity" "$binary_path"
    codesign --verify --strict --verbose=2 "$binary_path"
    spctl -a -t exec -vv "$binary_path" || true
fi

echo
echo "Build and install complete."
echo "Expected binary:    $binary_path"
echo "Expected framework: $framework_path"

echo "See if it runs:"
csound --version