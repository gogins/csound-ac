'''
This module implements facilities for supporting algorithmic composition in 
the Reaper digital audio workstation, especially using Csound and the 
CsoundAC library for algorithmic composition. 

Generated scores, which may be either CsoundAC.Score objects or plain Python 
lists of notes [time, duration, channel, key, velocity], are sent to Reaper 
MidiItems.

Author: Michael Gogins
'''

from reaper_python import *
try:
    import CsoundAC
except ImportError:
    RPR_ShowConsoleMsg("WARNING: Could not import CsoundAC!\n")
    import types
    CsoundAC = types.ModuleType("CsoundAC")
    RPR_ShowConsoleMsg("Created stubs for CsoundAC.\n")
    def dynamic_getattr(name):
        def stub_function(*args, **kwargs):
            RPR_ShowConsoleMsg(f"Stub for {name} called with args: {args}, kwargs: {kwargs}")
        return stub_function
    CsoundAC.__getattr__ = dynamic_getattr
    class Score:
        pass
    setattr(CsoundAC, "Score", Score)


RPR_ShowConsoleMsg("Hello, Reaper!\n")

'''
Returns False if the object evaluates to False or is an empty string, or if 
the object is a Python "string pointer" with a value of zero; otherwise, 
returns True.
'''
def is_valid_object(object):
    """Check if a Reaper object reference is valid."""
    if not object or object.strip() == "":
        return False
    # Check if the object string indicates a null pointer
    if object.startswith("(") and "0x0000000000000000" in object:
        return False
    return True

'''
Clears all data from the MIDI item.
'''
def clear_midi_item(midi_item):
    if not is_valid_object(midi_item):
        RPR_ShowConsoleMsg("No MIDI item selected!\n")
        return
    # Get the active take from the selected item
    midi_take = RPR_GetActiveTake(midi_item)
    if not is_valid_object(midi_take):
        RPR_ShowConsoleMsg("MIDI item has no active MIDI take!\n")
        return
    # Delete all events from a MidiItem.
    p1 = int(0)
    p2 = int(1)
    p3 = int(2)
    (result, midi_take, note_count, cc_count, text_sysex_count) = RPR_MIDI_CountEvts(midi_take, p1, p2, p3)
    for i in range(note_count - 1, -1, -1):  # Iterate in reverse to avoid index shifting
        RPR_MIDI_DeleteNote(midi_take, i)
    for i in range(cc_count - 1, -1, -1):  # Iterate in reverse
        RPR_MIDI_DeleteCC(midi_take, i)
    for i in range(text_sysex_count - 1, -1, -1):  # Iterate in reverse
        RPR_MIDI_DeleteTextSysexEvt(midi_take, i)
    # Commit changes
    RPR_MIDI_Sort(midi_take)
    RPR_ShowConsoleMsg("Cleared all contents from the MIDI item.\n")

''' 
Inserts a note into the MidiTake.
'''
def note_to_miditake(midi_take, start, duration, channel, key, velocity):
    start_ppq = RPR_MIDI_GetPPQPosFromProjTime(midi_take, start)
    end_ppq = RPR_MIDI_GetPPQPosFromProjTime(midi_take, start + duration)
    if start_ppq == -1 or end_ppq == -1:
        RPR_ShowConsoleMsg(
            f"Failed to calculate PPQ for note {note}: Start={start_ppq}, End={end_ppq}\n"
        )
        return
    RPR_ShowConsoleMsg(
        f"Inserting note: Start PPQ={start_ppq}, End PPQ={end_ppq}, "
        f"Key={key}, Velocity={velocity}\n"
    )
    success = RPR_MIDI_InsertNote(
        midi_take,    # Take
        False,        # Selected
        False,        # Muted
        start_ppq,    # Start time (PPQ)
        end_ppq,      # End time (PPQ)
        int(channel),      # Channel
        int(key),          # Note pitch
        int(velocity),     # Velocity
        True          # NoSort
    )
    if not success:
        RPR_ShowConsoleMsg(f"Failed to insert note: {note}\n")

'''
Sends all notes from the score to the currently selected MidiItem. 
Previously existing events in the item are first deleted. The score can be 
either a CsoundAC.Score object, or a plain Python list of notes 
(start, duration, channel, key, velocity).

NOTE: All times are project times in seconds!
'''
def score_to_midiitem(score, key_offset=0, start_offset=0):
    # Check for a selected MIDI item
    selected_item = RPR_GetSelectedMediaItem(0, 0)
    RPR_ShowConsoleMsg(f"Selected item: {type(selected_item)} {selected_item}\n")
    if is_valid_object(selected_item):
        # Get the active take from the selected item
        midi_take = RPR_GetActiveTake(selected_item)
        if not is_valid_object(midi_take):
            RPR_ShowConsoleMsg("Selected item has no active take!\n")
            return
        RPR_ShowConsoleMsg("Using active take of selected MIDI item.\n")
    else:
        # No item selected, create a new MIDI item
        track = RPR_GetTrack(0, 0)
        if not is_valid_object(track):
            RPR_ShowConsoleMsg("No track found to create a MIDI item!\n")
            return
        start_time = start_offset   # Start time in seconds
        end_time = start_time + 4 # End time in seconds
        midi_item, media_track, d1, d2, d3 = RPR_CreateNewMIDIItemInProj(track, start_time, end_time, False)
        RPR_ShowConsoleMsg(f"Created new MIDI Item: {midi_item}\n")
        if not is_valid_object(midi_item):
            RPR_ShowConsoleMsg("Failed to create a new MIDI item!\n")
            return
        # Select the newly created MIDI item
        RPR_SetMediaItemSelected(midi_item, True)
        midi_take = RPR_GetActiveTake(midi_item)
        # Get the active take for the new item
        midi_take = RPR_GetActiveTake(midi_item)
        if not is_valid_object(midi_take):
            RPR_ShowConsoleMsg("Failed to retrieve take for the new MIDI item!\n")
            return
        RPR_ShowConsoleMsg("Created, selected, and activated a new MIDI item.\n")
    clear_midi_item(selected_item)
    if isinstance(score, CsoundAC.Score):
        for eventI in range(len(score)):
            event = score[eventI]
            if event.isNote():
                event = score[eventI]
                start = event.getTime() + start_offset
                duration = event.getDuration()
                channel = int(event.getChannel())
                key = int(event.getKey() + key_offset)
                velocity = int(event.getVelocity())
                note_to_miditake(midi_take, start, duration, channel, key, velocity)
        # Fit MidiItem to the duration of the score.
        duration = score.getDurationFromZero()
        # Set the new length
        RPR_SetMediaItemInfo_Value(selected_item, "D_LENGTH", duration)
        # Update the project to reflect the changes
        RPR_UpdateArrange()

        RPR_ShowConsoleMsg(f"Duration of score: {duration}\n")
    else:
        for note in notes:
            start, duration, channel, key, velocity = note
            note_to_miditake(midi_take, start + start_offset, duration, channel, key, velocity)
    # Commit changes

    RPR_MIDI_Sort(midi_take)
    RPR_ShowConsoleMsg("MIDI notes added successfully! Check the editor.\n")

# Test case.
if __name__ == '__main__':  
    notes = [
        (0, 0.5, 1, 60, 100),  # (start, duration, channel, key, velocity)
        (0.5, 0.5, 1, 62, 100),
        (1, 0.5, 1, 64, 100),
        (1.5, 1.5, 1, 67, 100),
    ]
    score_to_midiitem(notes)


