#!/usr/bin/env bash
set -euo pipefail

echo "Building csound-ac for Linux..."

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${repo_root}/build-linux"

mkdir -p "${build_dir}"
rm -f "${build_dir}/CMakeCache.txt"

cmake -S "${repo_root}" -B "${build_dir}" -Wno-dev \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DCMAKE_PREFIX_PATH=/usr/local:/usr \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    "$@"

cmake --build "${build_dir}" --parallel 6 --verbose

echo "Installing csound-ac into standard system locations under /usr/local..."
sudo cmake --install "${build_dir}"

echo "Building packages..."
cmake --build "${build_dir}" --target package --verbose

echo "Debian packages and contents..."
find "${build_dir}" -name '*.deb' -ls -exec dpkg -f '{}' ';'

if command -v lintian >/dev/null 2>&1
then
    echo "Running lintian..."
    lintian --no-tag-display-limit --suppress-tags=spelling-error-in-changelog,non-dev-pkg-with-shlib-symlink -i "${build_dir}"/csound-extended-*.deb
else
    echo "lintian not found; skipping lintian checks."
fi

bash "${repo_root}/executable-targets-linux.sh"
echo "Finished building csound-ac for Linux."
