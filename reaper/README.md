# ac-reaper
Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## Introduction

This library contains facilities for supporting algorithmic composition within 
the Reaper digital audio workstation, using Python ReaScripts.

The primary objective is to enable using CsoundAC's Python API within 
ReaScript, but the library also supports scores that are generated in plain 
Python, as long as a score is just a list of notes, where each note is a list 
in the format `[start_time, duration, midi_channel, midi_key, midi_velocity]`.

For documentation of ac-reaper, look at the function comments in ac-reaper.py; 
this can be done in the ReaScript editor by clicking on a function name.

There are examples for using this library in this directory -- both for using 
CsoundAC, and for plain Python score generation.

## Installation

I recommend simply loading this library's Python code into Reaper using the 
_Actions_ menu, _Show action list..._ dialog, _New action_ dialog, _Load 
ReaScript..._ file picker.

Once loaded, the ReaScript location is remembered by Reaper, and the code can 
be edited and run within Reaper like any other ReaScript.

Alternatively, you can copy the Python code to Reaper's normal directory for 
ReaScript Actions (on macOS, that is the user's 
`~/Library/Application Support/REAPER/Scripts` directory).





