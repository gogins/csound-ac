# csound-ac
Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## Examples

This directory contains examples, mostly in Python, for CsoundAC. Many of 
these pieces are old and no longer work, either because they were written for 
Python 2, or because they use resources not found here. They can be fixed up
if necessary.

However, there are some pieces that can be used not only as examples but also 
as sanity checks to detect regressions after major commits:

1. `cellular/Cellular-v5-macos.py`, that uses available resources (with the 
   exception of Modartt VST3 plugins, which can be substituted), the 
   MusicModel code in CsoundAC, and the Midifile code in CsoundAC, for both 
   reading and writing MIDI files. 
   
2. `Parachronic-Piano.cpp`.