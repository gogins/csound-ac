import copy
import CsoundAC
import math
import os
import string
import sys
sys.path.append('/Users/michaelgogins/csound-ac/reaper')
import types
import time
from reaper_python import *
for p in sys.path:
    RPR_ShowConsoleMsg(p + '\n')
import ac_reaper
import CsoundAC

# The only difference from the basic-reascript.py example is that the 
# generated score is stored in a CsoundAC ScoreNode.

music_model = CsoundAC.MusicModel()
music_model.setCsd(csd)
music_model.setAuthor("CsoundAC Tutorial");
music_model.setTitle("csoundac_basic_reascript");
music_model.generateAllNames()
score_node = CsoundAC.ScoreNode()
music_model.addChild(score_node)
for i in range(60):
    p1 = 1 + (i % 7)
    p2 = i / 3
    p3 = 6
    p4 = 36 + i
    p5 = 60
    score_node.getScore().append(p2, p3, 144, p1, p4, p5)
# Call music_model generate to actually create a performable score.
music_model.generate()
# The ac_reaper module score_to_midiitem function can translate both scores as 
# raw Python arrays of notes, or scores as CsoundAC Scores.
ac_reaper.score_to_midiitem(music_model.getScore())
