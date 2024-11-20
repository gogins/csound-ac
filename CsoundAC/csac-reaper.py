'''
This module implements facilities for supporting algorithmic composition in 
the Reaper digital audio workstation, especially using Csound and the 
CsoundAC library for algorithmic composition.

Author: Michael Gogins
'''
import CsoundAC

RPR_ShowConsoleMsg("Hello, Reaper!\n")

from reaper_python import *

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
    RPR_ShowConsoleMsg("Cleared all content from the MIDI item.\n")
    
'''
Sends all notes from the CsoundAC Score to the currently selected MidiItem. 
All previously existing events in the item are first deleted. 

NOTE: All times are _project_ times in seconds!
'''
def csac_score_to_midiitem():
    # Check for a selected MIDI item
    selected_item = RPR_GetSelectedMediaItem(0, 0)
    RPR_ShowConsoleMsg(f"selected item: {type(selected_item)} {selected_item}\n")
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
        start_time = 0  # Start time in seconds
        end_time = 4    # End time in seconds
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
    # Add MIDI notes
    notes = [
        (0, 0.5, 60, 100),  # (start, length, pitch, velocity)
        (0.5, 0.5, 62, 100),
        (1, 0.5, 64, 100),
        (1.5, 1.5, 67, 100),
    ]
    for note in notes:
        start, length, pitch, velocity = note
        start_ppq = RPR_MIDI_GetPPQPosFromProjTime(midi_take, start)
        end_ppq = RPR_MIDI_GetPPQPosFromProjTime(midi_take, start + length)
        
        if start_ppq == -1 or end_ppq == -1:
            RPR_ShowConsoleMsg(
                f"Failed to calculate PPQ for note {note}: Start={start_ppq}, End={end_ppq}\n"
            )
            continue
        RPR_ShowConsoleMsg(
            f"Inserting note: Start PPQ={start_ppq}, End PPQ={end_ppq}, "
            f"Pitch={pitch}, Velocity={velocity}\n"
        )
        success = RPR_MIDI_InsertNote(
            midi_take,    # Take
            False,        # Selected
            False,        # Muted
            start_ppq,    # Start time (PPQ)
            end_ppq,      # End time (PPQ)
            0,            # Channel
            pitch,        # Note pitch
            velocity,     # Velocity
            True          # NoSort
        )
        if not success:
            RPR_ShowConsoleMsg(f"Failed to insert note: {note}\n")
    # Commit changes
    RPR_MIDI_Sort(midi_take)
    RPR_ShowConsoleMsg("MIDI notes added successfully! Check the editor.\n")

# Run the function
csac_score_to_midiitem()

