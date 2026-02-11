#!/bin/bash
echo SETUP FOR DEBUGGING CORE DUMPS ON MACOS VENTURA

#!/usr/bin/env bash
set -euo pipefail

sudo mkdir -p /cores
sudo chmod 1777 /cores

sudo sysctl -w kern.coredump=1
sudo sysctl -w kern.corefile=/cores/core.%P

ulimit -c unlimited
rm -f /cores/core.*

# 3) (Optional but harmless) show the effective settings.
echo "kern.coredump  = $(sysctl -n kern.coredump)"
echo "kern.corefile  = $(sysctl -n kern.corefile)"
echo "ulimit -c      = $(ulimit -c)"

# 4) Run the program (pass args through).
#    IMPORTANT: the program must actually crash (SIGSEGV, SIGABRT, etc.)
#    for a core to be written.
exec ./build-macos/CsoundAC/ChordSpaceTests "$@"
/ls -lt /cores/core.* 2>/dev/null || echo "no core written"


