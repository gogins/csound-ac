#!/bin/bash
clear
echo "Began cleaning and rebuilding all macOS artifacts from `pwd`...."
sudo rm -rfd build-macos
sudo rm -rfd dependencies/libmusicxml/build/lib/*
sudo rm -rfd dependencies/libmusicxml/build/bin/*
sudo find . -wholename "*_pycache_*" -delete
sudo -k
bash build-macos.sh "$@"
unzip -l build-macos/csound-ac-0.6.1-Darwin.zip 
echo "Finished cleaning and rebuilding all macOS artifacts from `pwd`."
