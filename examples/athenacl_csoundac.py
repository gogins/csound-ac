'''
Author: Michael Gogins

This module provides facilities for translating scores generated by athenaCL to 
CsoundAC Score objects.
'''
import os
import sys
import traceback
import CsoundAC
from athenaCL.libATH import command
from athenaCL.libATH import athenaObj
from athenaCL.libATH import eventList

'''
Translates an athenaCL event to a CsoundAC "note on" Event.
'''
def toCsoundAcEvent(athencacl_event):
    csoundac_event = CsoundAC.Event()
    # "Note on" event.
    csoundac_event.setStatus(144)
    # Not used: bpm = athencacl_event['bpm']
    # Csound instrument number
    instrument = athencacl_event['inst']
    csoundac_event.setInstrument(instrument)
    # athenaCL's time is apparently in seconds, the 'bpm' field does not seem 
    # to get into the Csound .sco file.
    time_ = athencacl_event['time']
    csoundac_event.setTime(time_)
    # Duration, ditto.
    duration = athencacl_event['dur']
    csoundac_event.setDuration(duration)
    # athenaCL "pitch space" is just semitones, with 0 = C4 = MIDI key 60.
    midi_key = athencacl_event['ps'] + 60.
    csoundac_event.setKey(midi_key)
    # athenaCL's amplitude is in [0, 1].
    midi_velocity = athencacl_event['amp'] * 127.
    csoundac_event.setVelocity(midi_velocity)
    # athenaCL's pan is in [0, 1].
    pan = athencacl_event['pan']
    csoundac_event.setPan(pan)
    return csoundac_event

'''
Appends the complete musical content of an AthenaObject, i.e. its PolySeq, 
to a CsoundAC Score object. The CsoundAC score is _not_ cleared first.

To generate materials using athenaCL for use by different Nodes of a CsoundAC 
music graph, each Node should use its own independent AthenaObject.
'''
def toCsoundAcScore(athena_object, csoundac_score):
    print("toCsoundAC...")
    performer = eventList.Performer()
    performer.flattenAll(athena_object)
    for key, value in performer.polySeq.items():
        sequence = value['esObj']
        events = sequence.list()
        for event in events:
            #print(event)
            csoundac_event = toCsoundAcEvent(event)
            #print("==>",csoundac_event.toCsoundIStatement())
            csoundac_score.append(csoundac_event)
    print("toCsoundAC.")
    
    
'''
Generates an athenaCL "score" by optionally creating an Interpreter, and then 
running the script. Python '#' comments are ignored. The generated score is 
the state of the AthenaObject instance in the interpreter, that is, 
interpreter.ao.
'''

def interpret(script, interpreter=None):
    if interpreter == None:
        interpreter = athenaObj.Interpreter()
    for line in script.split("\n"):
        line = line.strip()
        if len(line) > 1 and line[0] != '#':
            print("command:", line)
            print(interpreter.cmd(line))
    return interpreter
    
if __name__ == '__main__':
    
    # Unit test:

    script = '''
    emo cn
    pin a d3,e3,g3,a3,b3,d4,e4,g4,a4,b4,d5,e5,g5,a5,b5
    tmo ha
    # This is a comment.
    tin a 6 27
    tie r pt,(c,16),(ig,(bg,rc,(1,2,3,5,7)),(bg,rc,(3,6,9,12))),(c,1)
    tie a om,(ls,e,9,(ru,.2,1),(ru,.2,1)),(wp,e,23,0,0,1)
    tie d0 c,0
    tie d1 n,100,2,0,14
    tie d2 c,1
    tie d3 c,1
    tie d3 ru,1,4
    '''
    interpreter = interpret(script)
    csoundac_score = CsoundAC.Score()
    toCsoundAcScore(interpreter.ao, csoundac_score)    
    print()
    print("Generated CsoundAC Score (in Csound .sco format):")
    print(csoundac_score.getCsoundScore())
    print("Finished.")