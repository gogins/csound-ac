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
import CsoundAC
import GeneralizedContextualGroup

import random

CsoundAC.System.setMessageLevel(15)
music_model = CsoundAC.MusicModel()
music_model.generateAllNames()
sequence = CsoundAC.Sequence()
rescale_node = CsoundAC.Rescale()
rescale_node.setRescale(CsoundAC.Event.TIME, True, False, 0., 0.);
rescale_node.setRescale(CsoundAC.Event.INSTRUMENT, True, True, 1., 6.999);
rescale_node.setRescale(CsoundAC.Event.KEY, True, True, 36., 60.);
rescale_node.setRescale(CsoundAC.Event.VELOCITY, True, True, 80., 10.);
image_node = CsoundAC.ImageToScore2()
image_node.setImageFilename("/Users/michaelgogins/csound-ac/user-guide/44025833484_70440d3a59_o.jpg")
image_node.threshhold(80)
image_node.setMaximumVoiceCount(20)
image_node.generateLocally()
for i in range(image_node.getScore().size()):
    event = image_node.getScore().get(i)
    duration = event.getDuration()
    duration *= 2.
    event.setDuration(duration)
image_node.getScore().tieOverlappingNotes(True)
duration = image_node.getScore().getDuration()

# Create chord progressions and modulations.
voiceleading_node = CsoundAC.VoiceleadingNode()
scale = CsoundAC.scaleForName("D minor")
tyme = 0
progressions = [2, 3, 5]
durations = [50, 150, 75, 250]
degree = 1
chord_count = 1
while tyme < duration:
    chord = scale.chord(degree, 5, 3)
    chord_count = chord_count + 1
    voiceleading_node.chord(chord, tyme)
    tyme = tyme + random.choice(durations)
    RPR_ShowConsoleMsg(f"chord {chord_count}: {chord.eOP().name()} in scale: {scale.name()}: time: {tyme}\n")
    progression = random.choice(progressions)
    degree = 1 + int(degree + progression) % 7
    if (chord_count % 7) == 0:
        modulations = scale.modulations(chord)
        if modulations.size() > 0:
            scale = random.choice(modulations)
        
voiceleading_node.rescaleTimes = True
# Must be a child of the voiceleading node.
voiceleading_node.addChild(rescale_node)
rescale_node.addChild(image_node)
sequence.addChild(voiceleading_node)
music_model.addChild(sequence)

music_model.setAuthor("CsoundAC Tutorial")
music_model.setTitle("python-imagery")
music_model.generateAllNames()
music_model.generate()
music_model.getScore().setDuration(720.)

# The ac_reaper module score_to_midiitem function can translate both scores as 
# raw Python arrays of notes, or scores as CsoundAC Scores.
ac_reaper.score_to_midiitem(music_model.getScore())
