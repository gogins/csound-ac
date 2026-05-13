#!/usr/bin/env bash

# check-git-status.sh
#
# Recursively find Git repositories under $HOME and print one line per repo:
#   STATUS(8)  path  branch/upstream  summary-or-blank
#
# STATUS is a fixed-width column (blank when clean, "dirty" otherwise). Summary
# is empty when clean, otherwise a short dirty count. Untracked files are ignored (--untracked-files=no).

set -euo pipefail

home_dir="${HOME}"

readonly STATUS_COL_WIDTH=8

# Skip noisy or very large directories (basename match). Do NOT list ".git"
# in the prune group before -print, or find will never print repo roots.

while IFS= read -r git_dir
do
    repo_dir="$(dirname "$git_dir")"

    (
        cd "$repo_dir"

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
