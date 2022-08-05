#!/bin/bash
echo "Updating all packages required for csound-ac..."
echo
apt-get update
apt-get upgrade
echo "Updating build-essential..."
apt-get install build-essential
echo "Updating automake..."
apt-get install automake
echo "Updating autoconf..."
apt-get install autoconf
echo "Updating autotools..."
apt-get install autotools
echo "Updating boost..."
apt-get install libboost-dev
echo "Updating FFmpeg..."
apt-get install ffmpeg
apt-get install libavfilter-dev
apt-get install libmp3lame-dev
echo "Updating libtool..."
apt-get install libtool
echo "Updating autogen..."
apt-get install autogen
echo "Updating libeigen3-dev..."
apt-get install libeigen3-dev
echo "Updating PortSMF..."
apt-get install libportsmf-dev
echo "Updating libsox-dev..."
apt-get install libsox-dev
echo "Updating python3.9-dev..."
apt-get install python3.9-dev
echo "Updating OpenCV..."
apt-get libopencv-dev --fix-missig
echo "Finished updating all packages required for csound-ac."
