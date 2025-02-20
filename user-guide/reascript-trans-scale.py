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
import random

# Uncomment the next line if you need to see more about what is going on.
CsoundAC.System.setMessageLevel(15)
music_model = CsoundAC.MusicModel()

# This sequence will hold all sections of this piece.
sequence = CsoundAC.Sequence()
music_model.addChild(sequence)
scale_node = CsoundAC.ScoreNode()

# The first section is just the ascending chromatic scale from the earlier 
# examples, only played more slowly.
sequence.addChild(scale_node)
for i in range(60):
    p1 = 1 + (i % 7)
    p2 = i / 2
    p3 = 6
    p4 = 36 + i
    p5 = 60
    scale_node.getScore().add(p2, p3, 144, p1, p4, p5)
    
# Add a section that randomizes the onsets of the notes in the scale.
random_times_node = CsoundAC.ScoreNode()
sequence.addChild(random_times_node)
# Copy the orginal scale, and at the same time, compile a list of its note 
# onsets.
note_onsets = []
for i in range(scale_node.getScore().size()):
    event = scale_node.getScore().get(i)
    random_times_node.getScore().append_event(event)
    note_onsets.append(event.getTime())
# Shuffle the note onsets and then update the events in this node, which 
# are value copies of the events in the original scale.
print(note_onsets)
random.shuffle(note_onsets)
for i in range(len(note_onsets)):
    random_times_node.getScore().get(i).setTime(note_onsets[i])
    random_times_node.getScore().get(i).setInstrument(1)
# The new score needs to be sorted to satisfy assumptions of further 
# processing.
random_times_node.getScore().sort()

# Add a section that creates a connected line from the shuffled events.
connected_line_node = CsoundAC.ScoreNode()
sequence.addChild(connected_line_node)
size = random_times_node.getScore().size()
for i in range(size):
    event = random_times_node.getScore().get(i)
    if i < (size - 1):
        next_event = random_times_node.getScore().get(i + 1)
        off_time = next_event.getTime() # - .1
    else:
        off_time = event.getTime() + 1
    connected_line_node.getScore().append_event(event)
    # The issue of references vs. values needs care.
    # So first we copy the note into the new score, then we update the off 
    # time of that note, leaving the original note unchanged.
    event = connected_line_node.getScore().get(i)
    event.setOffTime(off_time)
    event.setVelocity(event.getVelocity() + 9)
connected_line_node.getScore().rescale(CsoundAC.Event.INSTRUMENT, True, 7, True, 0)

# Add a section that removes most leaps from the line.
smoother_line_node = CsoundAC.ScoreNode()
sequence.addChild(smoother_line_node)
for i in range(connected_line_node.getScore().size()):
    if i == 0:
        next_event = connected_line_node.getScore().get(i)
        movement = 0
    else:
        event = connected_line_node.getScore().get(i - 1)
        next_event = connected_line_node.getScore().get(i)
        movement = (next_event.getKey() - event.getKey())
        movement = ((movement > 0) - (movement < 0)) * abs(movement % 5)
    # Again, the issue of references vs. values needs care. Append 
    # the event to create a copy, then update the copy.
    smoother_line_node.getScore().append_event(next_event)
    next_event = smoother_line_node.getScore().get(i)
    next_event.setKey(movement)
# Rescale the smoother line to start higher.
smoother_line_node.getScore().rescale(CsoundAC.Event.KEY, True, 48, False, 0)
smoother_line_node.getScore().sort()

# Add a section that is a canon of three voices.
canon_node = CsoundAC.ScoreNode()
sequence.addChild(canon_node)
prior_key = 0
key = 0
# Stagger or syncopate the voices.
delay = 2.75
# Here, we handle the issue of references vs. values by 
# working with fields, and appending the fields to the new score.
for i in range(smoother_line_node.getScore().size()):
    event = smoother_line_node.getScore().get(i)
    i1 = 7
    i2 = 7
    i3 = 7
    t1 = event.getTime()
    t2 = t1 + delay
    t3 = t2 + delay
    d = event.getDuration()
    s = event.getStatus()
    i = event.getInstrument()
    k1 = event.getKey() 
    k2 = k1 + 9
    k3 = k2 + 18
    v = event.getVelocity()
    canon_node.getScore().append(t1, d, s, i1, k1, v)
    canon_node.getScore().append(t2, d, s, i2, k2, v)
    canon_node.getScore().append(t3, d, s, i3, k3, v)
canon_node.getScore().sort()

# Add a section that applies chord transformations from the Generalized 
# Contextual Group to the canon. 
harmonized_node = CsoundAC.VoiceleadingNode()
sequence.addChild(harmonized_node)
cell_repeat = CsoundAC.Cell()
cell_repeat.setRepeatCount(2)
# Use a negative time to cut out the tail of the first cell, and create a 
# seamless segue to the second cell.
cell_repeat.setDurationSeconds(-4)
harmonized_node.addChild(cell_repeat)
# We make this one twice as long simply by repeating it.
cell_repeat.addChild(canon_node)
duration = canon_node.getScore().getDuration() * 2
chords = 16
# Create chord transformations using the chord space operations of CsoundAC.
scale = CsoundAC.scaleForName("E major")
scale_step = 1
for chord_ in range(chords):
    time = duration * (chord_ / chords)
    movement = random.choice([2, 2, 5, -1])
    scale_step = 1 + ((scale_step + movement) % 7)
    chord = scale.chord(scale_step, 5)
    if (chord_ % 5) == 0:
        scale = scale.modulations(chord)[0]
    # Use the VoiceleadingNode to schedule the transformed chords to be 
    # to the notes in the canon.
    harmonized_node.chord(chord, time)

# Generate and render the piece.
# Set basic metadata, which is used to creaate filenames.
music_model.setAuthor("CsoundAC Tutorial");
music_model.setTitle("python-trans-scale");
music_model.generateAllNames()
music_model.generate()
 # Tieing overlapping notes makes the line easier to hear.
music_model.getScore().tieOverlappingNotes(True)

# The ac_reaper module score_to_midiitem function can translate both scores as 
# raw Python arrays of notes, or scores as CsoundAC Scores.
ac_reaper.score_to_midiitem(music_model.getScore())

