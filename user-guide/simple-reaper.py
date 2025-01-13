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

music_model = CsoundAC.MusicModel()
score_node = CsoundAC.ScoreNode()
music_model.addChild(score_node)
for i in range(100):
    p1 = 1 + (i % 8)
    p2 = i / 4
    p3 = 6
    p4 = 36 + (i % 60)
    p5 = 60
    score_node.getScore().add(p2, p3, 144, p1, p4, p5)
print("Generated score:")
print(score_node.getScore().getCsoundScore())
ac_reaper.score_to_midiitem(score_node.getScore())
