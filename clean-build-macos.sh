#!/bin/bash
echo "Began cleaning and rebuilding all macOS artifacts from `pwd`...."
sudo make uninstall
sudo rm -rfd build-macos
sudo rm -rfd dependencies/libmusicxml/build/lib/*
sudo rm -rfd dependencies/libmusicxml/build/bin/*
sudo find . -wholename "*_pycache_*" -delete
sudo -k
bash build-macos.sh
find . -name "*.dylib" -ls
find . -name "*.so" -ls
find . -name "*.a" -ls
find . -name "*.py" -ls
echo "Finished cleaning and rebuilding all macOS artifacts from `pwd`."
