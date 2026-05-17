#!/usr/bin/env bash

# check-git-status.sh
#
# Recursively find Git repositories under $HOME and print one line per repo:
#   STATUS(8)  path  branch/upstream  summary-or-blank
#
# STATUS is a fixed-width column (blank when clean, "dirty" otherwise). Summary
# is empty when clean, otherwise a short dirty count. Untracked files are ignored (--untracked-files=no).
# If GitHub CLI is available, latest release assets are listed below each
# GitHub repository with the platform and release asset on one aligned line.

set -euo pipefail

home_dir="${HOME}"

printf '\n'

readonly STATUS_COL_WIDTH=8
readonly PLATFORM_COL_WIDTH=7

github_repo_slug() {
    remote_url="$(git config --get remote.origin.url 2>/dev/null || true)"
    case "${remote_url}" in
        git@github.com:*)
            slug="${remote_url#git@github.com:}"
            ;;
        https://github.com/*)
            slug="${remote_url#https://github.com/}"
            ;;
        ssh://git@github.com/*)
            slug="${remote_url#ssh://git@github.com/}"
            ;;
        *)
            return 1
            ;;
    esac

    slug="${slug%.git}"
    case "${slug}" in
        */*) printf '%s\n' "${slug}" ;;
        *) return 1 ;;
    esac
}

print_release_assets_for_platform() {
    local platform_name="$1"
    local asset_pattern="$2"
    local release_tag="$3"
    local release_json="$4"
    local matches

    matches="$(
        RELEASE_JSON="${release_json}" ASSET_PATTERN="${asset_pattern}" python3 - <<'PY' || true
import json
import os
import re

release = json.loads(os.environ["RELEASE_JSON"])
pattern = re.compile(os.environ["ASSET_PATTERN"], re.IGNORECASE)
release_time = release.get("publishedAt") or ""
for asset in release.get("assets") or []:
    name = asset.get("name") or ""
    if pattern.search(name):
        asset_time = asset.get("updatedAt") or asset.get("createdAt") or release_time
        print(f"{name}\t{asset_time}")
PY
    )"

    [ -n "${matches}" ] || return 0

    while IFS=$'\t' read -r asset_name asset_time
    do
        printf "%-${STATUS_COL_WIDTH}s      %-${PLATFORM_COL_WIDTH}s: %s %s %s\n" "" "${platform_name}" "${asset_time}" "${release_tag}" "${asset_name}"
    done <<< "${matches}"
}

print_latest_github_release() {
    if ! command -v gh >/dev/null 2>&1
    then
        return 0
    fi

    local github_repo
    github_repo="$(github_repo_slug || true)"
    if [ -z "${github_repo:-}" ]
    then
        return 0
    fi

    local release_json
    release_json="$(gh release view --repo "${github_repo}" --json tagName,publishedAt,assets 2>/dev/null || true)"
    if [ -z "${release_json}" ]
    then
        return 0
    fi

    local release_tag
    release_tag="$(
        RELEASE_JSON="${release_json}" python3 - <<'PY' || true
import json
import os

release = json.loads(os.environ["RELEASE_JSON"])
print(release.get("tagName", "(unknown)"))
PY
    )"
    if [ -z "${release_tag}" ]
    then
        return 0
    fi

    print_release_assets_for_platform "macOS" "macos|darwin|osx" "${release_tag}" "${release_json}"
    print_release_assets_for_platform "Linux" "linux" "${release_tag}" "${release_json}"
    print_release_assets_for_platform "Windows" "windows|win64|win32|mingw" "${release_tag}" "${release_json}"
}

# Skip noisy or very large directories (basename match). Do NOT list ".git"
# in the prune group before -print, or find will never print repo roots.

while IFS= read -r git_dir
do
    repo_dir="$(dirname "$git_dir")"

    (
        cd "$repo_dir"

        if [ -n "$(git rev-parse --show-superproject-working-tree 2>/dev/null || true)" ]
        then
            exit 0
        fi

        branch_info="$(git status -sb --untracked-files=no 2>/dev/null | head -n 1 | sed 's/^##[[:space:]]*//')"
        if [ -z "${branch_info}" ]
        then
            branch_info="(unknown)"
        fi

        porcelain="$(git status --porcelain --untracked-files=no 2>/dev/null || true)"
        if [ -z "${porcelain}" ]
        then
            printf "%-${STATUS_COL_WIDTH}s  %s  %s  %s\n" "" "${repo_dir}" "${branch_info}" ""
        else
            n="$(printf '%s\n' "${porcelain}" | wc -l | tr -d '[:space:]')"
            printf "%-${STATUS_COL_WIDTH}s  %s  %s  %s\n" "dirty" "${repo_dir}" "${branch_info}" "${n} path(s)"
        fi

        print_latest_github_release
    )
done < <(
    find "${home_dir}" \
        \( \
            -name ".Trash" -o \
            -name ".cache" -o \
            -name "Library" -o \
            -name "node_modules" \
        \) -prune -o \
        -type d -name ".git" -print -prune 2>/dev/null || true
)

printf '\n'