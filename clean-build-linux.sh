#!/bin/bash
echo "Began cleaning and rebuilding all Linux artifacts from `pwd`...."
sudo apt remove csound-ac
sudo rm -rfd build-linux
sudo rm -rfd dependencies/libmusicxml/build/lib/*
sudo rm -rfd dependencies/libmusicxml/build/bin/*
sudo find . -wholename "*_pycache_*" -delete
sudo -k
bash build-linux.sh "$@"
echo "Finished cleaning and rebuilding all Linux artifacts from `pwd`."
