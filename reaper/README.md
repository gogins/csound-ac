# ac-reaper
Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## Introduction

This library contains facilities for supporting algorithmic composition within 
the [Reaper](https://www.reaper.fm/) digital audio workstation, using Python 
[ReaScripts](https://www.reaper.fm/sdk/reascript/reascript.php).

The primary objective is to enable using CsoundAC's Python API within 
ReaScript, but the library also supports scores that are generated in plain 
Python, as long as a score is just a list of notes, where each note is a list 
in the format `[start_time, duration, midi_channel, midi_key, midi_velocity]`.

For documentation, read the function comments in `ac-reaper.py` in the 
ReaScript editor; this can be done by clicking on a function name.

## Installation

I recommend simply loading `ac_python.py` into Reaper using the _Actions_ 
menu, _Show action list..._ dialog, _New action_ dialog, _Load ReaScript..._ 
file picker.

Once loaded, the ReaScript location is remembered by Reaper, and the code can 
be edited and run within Reaper like any other ReaScript.

Alternatively, you can copy the Python code to Reaper's normal directory for 
ReaScript Actions (on macOS, that is the user's 
`~/Library/Application Support/REAPER/Scripts` directory).

## Usage

The main concept is that the composer will write a ReaScript script in Python 
to algorithmically generate a score as a list of MIDI notes, either using 
CsoundAC, or using plain Python. The script must add the directory containing 
`ac_python.py` to the Python `sys.path` list, and import `ac_python`. After 
generating the score, the script must call the `score_to_midiitem` function to
send the generated score to Reaper.

To actually use the script, the composer selects a MidiItem on a Reaper Track, 
and runs the script to generate the score and send its notes to the MidiItem, 
which is resized to fit the notes.

If a MidiItem is not selected, a new MidiItem is created on the first track.

MIDI from the generated score will then be routed in the normal way to 
MIDI inputs in Reaper.

There are examples for using this library in this directory -- both for using 
CsoundAC, and for plain Python score generation.







