#!/usr/bin/env bash
set -euo pipefail

clear
echo "Building csound-ac for macOS..."

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$repo_root/build-macos"
archive="$build_dir/csound-ac-8.0.0-Darwin.zip"

rm -rf "$build_dir" "$repo_root/dist"
mkdir -p "$build_dir"

mkdir -p "$repo_root/doc/latex" "$repo_root/doc/html"
sudo chown -R "$USER":staff     "$build_dir"     "$repo_root/doc/latex"     "$repo_root/doc/html"     "$repo_root/dependencies/libmusicxml/build" 2>/dev/null || true

cmake -S "$repo_root" -B "$build_dir" -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCSOUND_AC_PREFER_LOCAL_DEPS=ON \
    -DCMAKE_INSTALL_PREFIX=/opt/homebrew \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCSOUND_AC_INSTALL_PYTHON_TO_SITEARCH=OFF \
    -Wno-dev \
    "$@"
    
cmake --build "$build_dir" --parallel 6 --verbose
cmake --build "$build_dir" --target release_dist --verbose

echo "Archive: $archive"
./external/codesign-check.bash ./build-macos/csound-ac-9.0.0-Darwin.zip

echo "Installing csound-ac into standard system locations under /opt/homebrew..."
echo "sudo is required for installation; enter your password if prompted."
sudo -v
sudo cmake --install "$build_dir"

echo "Finished building csound-ac for macOS."

