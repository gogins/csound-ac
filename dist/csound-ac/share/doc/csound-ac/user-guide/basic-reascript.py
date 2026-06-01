import copy
import CsoundAC
import math
import os
import string
import sys
import types
import time
from reaper_python import *
for p in sys.path:
    RPR_ShowConsoleMsg(p + '\n')
import ac_reaper

# Here an event is just a list of numbers,
# and a score is just a list of events.
# The event fields are ordered the same as in CsoundAC:
# [time, duration, MIDI status, MIDI key, MIDI veloity].
# But here, they are numbered like the pfields in the Csound
# instrument definitions.
score = []
for i in range(60):
    p1 = 1 + (i % 7)
    p2 = i / 3
    p3 = 6
    p4 = 36 + i
    p5 = 60
    score.append([p2, p3, 144, p1, p4, p5])
# Call the ac_reaper module's score_to_midiitem function, which 
# will translate each note in the generated score to a Reaper MIDI 
# note in the selected MIDI item.
ac_reaper.score_to_midiitem(score)
