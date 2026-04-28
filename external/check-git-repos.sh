#!/bin/bash
clear

set -euo pipefail

REPOS=(
    cloud-5
    csound
    csound-ac
    csound-cxx-opcodes
    csound-examples
    csound-nwjs
    csound-vst3
    csound-vst3-opcodes
    csound-wasm
    gogins.github.io
)

RED=$'\033[31m'
YELLOW=$'\033[33m'
BOLD=$'\033[1m'
RESET=$'\033[0m'

ROOT="${HOME}"

echo "G I T   S T A T U S"

for repo in "${REPOS[@]}"; do
    DIR="${ROOT}/${repo}"
    line="==== ${repo} "
    printf "%-79s\n" "$line" | tr ' ' '='
    if [ ! -d "${DIR}" ]; then
        echo "Not found: ${DIR}"
        continue
    fi

    cd "${DIR}"

    if [ ! -d ".git" ]; then
        echo "Not a git repo"
        continue
    fi

    BRANCH=$(git branch --show-current 2>/dev/null || echo "(detached)")
    COMMIT=$(git rev-parse --short HEAD)

    echo "Branch: ${BRANCH} (${COMMIT})"

    STATUS=$(git status --porcelain -uno)

    if [ -z "${STATUS}" ]; then
        echo "Status: clean"
    else
        echo -e "${RED}${BOLD}Status : dirty${RESET}"
        echo "${STATUS}"
    fi

    # Optional: show upstream divergence
    if git rev-parse --abbrev-ref @{u} >/dev/null 2>&1; then
        UPSTREAM=$(git rev-parse --abbrev-ref @{u})
        AHEAD_BEHIND=$(git rev-list --left-right --count HEAD...@{u})
        echo "Upstream: ${UPSTREAM} (${AHEAD_BEHIND})"
    else
        echo "Upstream: none"
    fi
done

