#!/usr/bin/python
# -*- coding: utf-8 -*-

'''
A D D 2 C S D

Usage: python add2csd.py existing.csd [[file 1], [file2], ...]

Encodes any number of files to base64 and includes each as a 
</CsFileB filename=name> element in the (already existing) .csd file.

If a file has already been included in the .csd, it is replaced.

When Csound runs, it will decode all of the base64 encoded elements in the 
.csd and extract them to the current directory with their original filenames.

This is useful for several purposes included preparing a .csd file with 
SoundFonts, samples, analysis files, MIDI files, etc. that will run from 
Csound for WebAssembly, as the decoded files will be written to Emscripten's 
local filesystem within the browser's hidden sandbox before Csound runs the 
.csd.
'''
print(__doc__)

import base64
import os
import os.path
import string
import sys
import traceback

def add2csd(csd_file, filename):
    print(f"Adding: {filename}.")
    tail = os.path.split(filename)[1]
    file_data = open(filename, 'rb').read()
    encoded_data = base64.encodebytes(file_data).decode('utf-8')
    if(len(encoded_data) < 1):
        print(f"WARNING: No data in {filename}.")
        return
    start_tag = f"<CsFileB filename={tail}>"
    end_tag = "</CsFileB>"
    encoding = ''.join([start_tag, '\n', encoded_data, end_tag])
    print("Encoded as:")
    print(encoding)
    start_index = csd_file.find(start_tag)
    # If the file has already been encoded into this .csd,
    # remove the existing encoding.
    if start_index != -1:
        end_index = csd_file.find(end_tag, start_index)
    # Insert the encoding immediately after </CsOptions>.
        

try:
    # Read entire .csd into memory.
    csd_filename = sys.argv[1]
    csd_file = open(csd_filename).read()
    print(csd_file)
    # Iterate through all filenames in argv.
    for filename in sys.argv[2:]:
        csd_file = add2csd(csd_file, filename)

except:
    traceback.print_exc()



