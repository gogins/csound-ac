#!/usr/bin/env bash

# check-git-status.sh
#
# Recursively find Git repositories under $HOME and print one line per repo:
#   STATUS(8)  path  branch/upstream  summary-or-blank
#
# STATUS is a fixed-width column (blank when clean, "dirty" otherwise). Summary
# is empty when clean, otherwise a short dirty count. Untracked files are ignored (--untracked-files=no).
# If GitHub CLI is available, uploaded assets from the latest GitHub release are
# listed below each repository (source code archives are omitted).

set -euo pipefail

home_dir="${HOME}"

printf '\n'

readonly STATUS_COL_WIDTH=8

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

is_nested_git_repo() {
    local repo_dir="$1"
    local parent

    parent="$(dirname "${repo_dir}")"
    while [ "${parent}" != "${home_dir}" ] && [ "${parent}" != "/" ]
    do
        if [ -e "${parent}/.git" ]
        then
            return 0
        fi
        parent="$(dirname "${parent}")"
    done
    return 1
}

print_latest_github_release() {
    if ! command -v gh >/dev/null 2>&1
    then
        return 0
    fi

    local github_repo release_json lines
    github_repo="$(github_repo_slug || true)"
    if [ -z "${github_repo:-}" ]
    then
        return 0
    fi

    release_json="$(gh release view --repo "${github_repo}" \
        --json tagName,publishedAt,assets 2>/dev/null || true)"
    if [ -z "${release_json}" ]
    then
        return 0
    fi

    lines="$(
        RELEASE_JSON="${release_json}" python3 - <<'PY' || true
import json
import os
import re

try:
    release = json.loads(os.environ.get("RELEASE_JSON", "") or "{}")
except json.JSONDecodeError:
    release = {}

tag = release.get("tagName") or "(unknown)"
release_time = release.get("publishedAt") or ""
source_code = re.compile(r"^source code\b", re.IGNORECASE)

for asset in release.get("assets") or []:
    name = (asset.get("name") or "").strip()
    if not name:
        continue
    label = (asset.get("label") or "").strip()
    if source_code.search(name) or source_code.search(label):
        continue
    asset_time = asset.get("updatedAt") or asset.get("createdAt") or release_time
    asset_time = asset_time.replace("T", " ", 1)
    print(f"{asset_time}\t{tag}\t{name}")
PY
    )"

    [ -n "${lines}" ] || return 0

    while IFS=$'\t' read -r asset_time release_tag asset_name
    do
        [ -n "${asset_name}" ] || continue
        printf "%-${STATUS_COL_WIDTH}s      %s %s %s\n" "" "${asset_time}" "${release_tag}" "${asset_name}"
    done <<< "${lines}"
}

# Skip noisy or very large directories (basename match). Do NOT list ".git"
# in the prune group before -print, or find will never print repo roots.

while IFS= read -r git_dir
do
    repo_dir="$(dirname "$git_dir")"

    if is_nested_git_repo "${repo_dir}"
    then
        continue
    fi

    (
        cd "$repo_dir"

        branch_info="$(git status -sb --untracked-files=no --ignore-submodules=all 2>/dev/null | head -n 1 | sed 's/^##[[:space:]]*//')"
        if [ -z "${branch_info}" ]
        then
            branch_info="(unknown)"
        fi

        porcelain="$(git status --porcelain --untracked-files=no --ignore-submodules=all 2>/dev/null || true)"
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
