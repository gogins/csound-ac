#!/usr/bin/env bash
set -euo pipefail

clear
echo "Building csound-ac release for macOS..."

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$repo_root/build-macos"
dist_dir="$repo_root/dist/csound-ac"
archive="$build_dir/csound-ac-8.0.0-Darwin.zip"

rm -rf "$build_dir" "$repo_root/dist"
mkdir -p "$build_dir" "$dist_dir"

mkdir -p "$repo_root/doc/latex" "$repo_root/doc/html"
sudo chown -R "$USER":staff \
    "$build_dir" \
    "$repo_root/doc/latex" \
    "$repo_root/doc/html" \
    "$repo_root/dependencies/libmusicxml/build" 2>/dev/null || true

: "${APPLE_CODESIGN_IDENTITY:?APPLE_CODESIGN_IDENTITY is not exported}"
: "${APPLE_NOTARY_KEY:?APPLE_NOTARY_KEY is not exported}"
: "${APPLE_NOTARY_KEY_ID:?APPLE_NOTARY_KEY_ID is not exported}"
: "${APPLE_NOTARY_ISSUER_ID:?APPLE_NOTARY_ISSUER_ID is not exported}"

cmake -S "$repo_root" -B "$build_dir" -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCSOUND_AC_PREFER_LOCAL_DEPS=ON \
    -DCMAKE_INSTALL_PREFIX=/opt/homebrew \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCSOUND_AC_ENABLE_CODESIGN=ON \
    -DCSOUND_AC_ENABLE_NOTARIZATION=ON \
    -DAPPLE_CODESIGN_IDENTITY="$APPLE_CODESIGN_IDENTITY" \
    -DAPPLE_NOTARY_KEY="$APPLE_NOTARY_KEY" \
    -DAPPLE_NOTARY_KEY_ID="$APPLE_NOTARY_KEY_ID" \
    -DAPPLE_NOTARY_ISSUER_ID="$APPLE_NOTARY_ISSUER_ID" \
    -Wno-dev \
    "$@"

cmake --build "$build_dir" --parallel 6 --verbose

echo "Staging csound-ac artifacts without recursively installing libmusicxml..."

rm -rf "$dist_dir"
mkdir -p \
    "$dist_dir/bin" \
    "$dist_dir/lib" \
    "$dist_dir/include/csound" \
    "$dist_dir/python" \
    "$dist_dir/examples" \
    "$dist_dir/doc"

find "$build_dir" -type f \( \
    -name "libCsoundAC*.dylib" -o \
    -name "_CsoundAC*.so" -o \
    -name "CsoundAC.py" -o \
    -name "ChordSpaceTests" \
\) -print

find "$build_dir" -type f -name "libCsoundAC*.dylib" -exec cp -p {} "$dist_dir/lib/" \;
find "$build_dir" -type f -name "_CsoundAC*.so" -exec cp -p {} "$dist_dir/python/" \;
find "$build_dir" -type f -name "CsoundAC.py" -exec cp -p {} "$dist_dir/python/" \;
find "$build_dir" -type f -name "ChordSpaceTests" -exec cp -p {} "$dist_dir/bin/" \;

find "$repo_root/CsoundAC" -type f -name "*.hpp" -exec cp -p {} "$dist_dir/include/csound/" \;

cp -p "$repo_root/README.md" "$dist_dir/" 2>/dev/null || true
cp -p "$repo_root/LICENSE" "$dist_dir/" 2>/dev/null || true
cp -R "$repo_root/examples/." "$dist_dir/examples/" 2>/dev/null || true
cp -R "$repo_root/doc/html/." "$dist_dir/doc/html/" 2>/dev/null || true
cp -R "$repo_root/doc/latex/." "$dist_dir/doc/latex/" 2>/dev/null || true

echo "Signing staged Mach-O files..."

find "$dist_dir" -type f | while IFS= read -r file
do
    if file "$file" | grep -q "Mach-O"
    then
        codesign --force --timestamp --options runtime \
            --sign "$APPLE_CODESIGN_IDENTITY" \
            "$file"
    fi
done

echo "Creating release archive..."

rm -f "$archive"
(
    cd "$repo_root/dist"
    /usr/bin/zip -qry "$archive" "csound-ac"
)

echo "Notarizing release archive..."

xcrun notarytool submit "$archive" \
    --key "$APPLE_NOTARY_KEY" \
    --key-id "$APPLE_NOTARY_KEY_ID" \
    --issuer "$APPLE_NOTARY_ISSUER_ID" \
    --wait

echo "Installing to /opt/homebrew..."
sudo cmake --install "$build_dir"

echo "Finished building csound-ac release for macOS."
echo "Staged files: $dist_dir"
echo "Archive:      $archive"