#!/bin/bash
clear
echo "Began cleaning and rebuilding all macOS artifacts from `pwd`...."
sudo rm -rfd build-macos
sudo rm -rfd dependencies/libmusicxml/build/lib/*
sudo rm -rfd dependencies/libmusicxml/build/bin/*
sudo find . -wholename "*_pycache_*" -delete

# Ensure no root-owned doc artifacts remain.
sudo chown -R michaelgogins:staff doc/latex doc/html 2>/dev/null || true
cmake_args=("$@")
sudo -k
bash build-macos.sh "$@"
unzip -l build-macos/csound-ac-8.0.0-Darwin.zip 
echo "Finished cleaning and rebuilding all macOS artifacts from `pwd`."
