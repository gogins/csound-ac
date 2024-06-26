#!/usr/bin/python
# -*- coding: utf-8 -*-

'''
A D D 2 C S D

Usage: python add2csd.py existing.csd [[file 1], [file2], ...]

Encodes any number of files to base64 and includes each as a 
</CsFileB filename=name> element in a new .csd file named 
existing.embedded.csd.

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

def add2csd(csd_file, filepath):
    print(f"Adding: {filepath}.")
    filename = os.path.split(filepath)[1]
    file_data = open(filepath, 'rb').read()
    encoded_data = base64.encodebytes(file_data).decode('utf-8')
    if(len(encoded_data) < 1):
        print(f"WARNING: No data in {filepath}.")
        return
    start_tag = f"<CsFileB filename={filename}>"
    end_tag = "</CsFileB>"
    encoding = ''.join(['\n', start_tag, '\n', encoded_data, end_tag])
    print("Encoded as:")
    print(encoding)
    start_index = csd_file.find(start_tag)
    # If the file has already been encoded into this .csd,
    # remove the existing encoding.
    if start_index != -1:
        end_index = csd_file.find(end_tag, start_index)
        temp =  csd_file[0:start_index] + csd_file[end_index + 10:]
        print("After deletion:")
        csd_file = temp
    # Insert the new encoding immediately after </CsOptions>.
    csd_file = csd_file.replace('</CsOptions>', '</CsOptions>' + encoding)
    print('New file:')
    print(csd_file)
    return csd_file

        

try:
    # Read entire .csd into memory.
    csd_filename = sys.argv[1]
    target_csd_filename = csd_filename.replace(".csd", ".embedded.csd")
    csd_file = open(csd_filename).read()
    print(csd_file)
    # Iterate through all filenames in argv.
    for filename in sys.argv[2:]:
        csd_file = add2csd(csd_file, filename)
    target_csd_file = open(target_csd_filename, 'w').write(csd_file)
except:
    traceback.print_exc()



