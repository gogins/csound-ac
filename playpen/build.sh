#!/bin/bash
echo "Clean build of the Csound Playpen..."
sudo -k xargs rm < install_manifest.txt
cmake .
make VERBOSE=1
sudo -k make install
echo "Installed:"
cat install_manifest.txt
echo
