# csound-ac
Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## Unit Tests

This directory contains unit tests, mostly in Python, for CsoundAC. Many of 
these tests are old and no longer work, either because they were written for 
Python 2, or because they use resources not found here. They can be fixed up
if necessary.

However, there is one piece, "Cellular-v5-macos.py", that uses available 
resources (with the exception of Modartt VST3 plugins, which can be 
substituted), the MusicModel code in CsoundAC, and the Midifile code in 
CsoundAC, for both reading and writing MIDI files. This piece can be 
considered a useful sanity check to detect regressions after major commits.