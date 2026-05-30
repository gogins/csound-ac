#!/usr/bin/env bash
set -euo pipefail

clear
echo "Building csound-ac for macOS..."

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$repo_root/build-macos"

# Use the csound venv for SWIG/Python configure when it exists and is not already active.
if [[ -z "${VIRTUAL_ENV:-}" && -f "${HOME}/venv/csound/bin/activate" ]]; then
    # shellcheck disable=SC1091
    source "${HOME}/venv/csound/bin/activate"
fi

cmake_args=(
    -S "$repo_root"
    -B "$build_dir"
    -G "Unix Makefiles"
    -DCMAKE_BUILD_TYPE=RelWithDebInfo
    -DCSOUND_AC_PREFER_LOCAL_DEPS=ON
    -DCMAKE_INSTALL_PREFIX=/opt/homebrew
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    -DCSOUND_AC_INSTALL_PYTHON_TO_SITEARCH=ON
    -Wno-dev
)

venv_site="${HOME}/venv/csound/lib/python3.12/site-packages"
if [[ -d "$(dirname "$venv_site")" ]]; then
    cmake_args+=(-DCSOUND_AC_EXTRA_PYTHON_SITEARCH="$venv_site")
fi

rm -rf "$build_dir" "$repo_root/dist"
mkdir -p "$build_dir"

mkdir -p "$repo_root/doc/latex" "$repo_root/doc/html"
sudo chown -R "$USER":staff \
    "$build_dir" \
    "$repo_root/doc/latex" \
    "$repo_root/doc/html" \
    "$repo_root/dependencies/libmusicxml/build" 2>/dev/null || true

cmake "${cmake_args[@]}" "$@"

cmake --build "$build_dir" --parallel 6 --verbose
cmake --build "$build_dir" --target release_dist --verbose

archive="$(find "$build_dir" -maxdepth 1 -name 'csound-ac-*-Darwin.zip' -print -quit)"
echo "Archive: ${archive:-$build_dir/csound-ac-*-Darwin.zip}"
if [[ -n "${archive:-}" ]]; then
    ./external/codesign-check.bash "$archive"
fi

echo "Installing csound-ac into standard system locations under /opt/homebrew..."
echo "sudo is required for installation; enter your password if prompted."
sudo -v
sudo cmake --install "$build_dir"

# Venv / active-interpreter site-packages (absolute paths; excluded from stage_dist).
if [[ -d "$(dirname "$venv_site")" ]] || [[ -n "${VIRTUAL_ENV:-}" ]]; then
    echo "Installing Python bindings into active interpreter site-packages..."
    cmake --install "$build_dir" --component PythonActive
fi

prefix_site="/opt/homebrew/lib/python3.12/site-packages"
if [[ -d "$prefix_site" ]]; then
    echo "System python3.12 site-packages: $prefix_site"
fi
if [[ -d "$venv_site" ]]; then
    echo "Csound venv site-packages: $venv_site"
fi

echo "Finished building csound-ac for macOS."
