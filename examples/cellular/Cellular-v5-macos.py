'''
Cellular version 4

Copyright (C) 2020 by Michael Gogins

Mozart's musical dice game of 1787 is taken apart and put back together along 
the lines of Terry Riley's "In C" using Python, re-harmonized using the 
CsoundAC.Scale class, and rendered with a built-in Csound orchestra that 
uses the Pianoteq synthesized piano and other Csound instruments.

Comments in the code are provided in an attempt to clarify what is going on.

The source code for the chord and scale classes used here (inspired by the 
music theory of Dmitri Tymoczko), can be found at:

https://github.com/gogins/csound-extended/blob/develop/CsoundAC/ChordSpace.hpp

'''
print(__doc__)
import CsoundAC
import os
import random
# Using the same random seed for each performance makes the performance 
# deterministic, not random.
random.seed(330394)
import signal
import string
import sys
import traceback

print('Set "rendering" to:     "soundfile" or "audio".')
print
rendering = "soundfile"

model = CsoundAC.MusicModel()
model.thisown = 0
score = model.getScore()
score.thisown = 0

script_filename = sys.argv[0]
print('Full Python script:     %s' % script_filename)
title, exte = os.path.splitext(os.path.basename(script_filename))
model.setTitle(title)
model.setArtist("Michael Gogins")
model.setAuthor("Michael Gogins")
model.setYear("2020")
model.generateAllNames()
soundfile_name = model.getOutputSoundfileFilepath()
print('Soundfile name:         %s' % soundfile_name)
dac_name = 'dac'
print('Audio output name:      %s' % dac_name)
print

print('Rendering option:       %s' % rendering)
rendering_commands = {
    'soundfile':    'csound -d -r 48000 -k 128 -m195 -+msg_color=0 -RWZdfo %s' % (soundfile_name),
    'audio':        'csound -d -r 48000 -k 128 -m195 -+msg_color=0 -o%s'       % (dac_name),
}
csound_command = rendering_commands[rendering]
print('Csound command line:    %s' % csound_command)
print

# This is Mozart's original minuet table for his musical dice game; his trio 
# table is not used. Mozart's idiosyncratic (by our standards) indexing scheme 
# is preserved. Rather than pick measures by throwing dice according to 
# Mozart's original instructions, each voice in this piece plays each measure 
# in each column on each row in turn, but for a randomly chosen number of 
# repetitions in each voice, as in Terry Riley's "In C."

minuet_table = {}
minuet_table[ 2] = { 1: 96,  2: 22,  3:141,  4: 41,  5:105,  6:122,  7: 11,  8: 30,  9: 70, 10:121, 11: 26, 12:  9, 13:112, 14: 49, 15:109, 16: 14}
minuet_table[ 3] = { 1: 32,  2:  6,  3:128,  4: 63,  5:146,  6: 46,  7:134,  8: 81,  9:117, 10: 39, 11:126, 12: 56, 13:174, 14: 18, 15:116, 16: 83}
minuet_table[ 4] = { 1: 69,  2: 95,  3:158,  4: 13,  5:153,  6: 55,  7:110,  8: 24,  9: 66, 10:139, 11: 15, 12:132, 13: 73, 14: 58, 15:145, 16: 79}
minuet_table[ 5] = { 1: 40,  2: 17,  3:113,  4: 85,  5:161,  6:  2,  7:159,  8:100,  9: 90, 10:176, 11:  7, 12: 34, 13: 67, 14:160, 15: 52, 16:170}
minuet_table[ 6] = { 1:148,  2: 74,  3:163,  4: 45,  5: 80,  6: 97,  7: 36,  8:107,  9: 25, 10:143, 11: 64, 12:125, 13: 76, 14:136, 15:  1, 16: 93}
minuet_table[ 7] = { 1:104,  2:157,  3: 27,  4:167,  5:154,  6: 68,  7:118,  8: 91,  9:138, 10: 71, 11:150, 12: 29, 13:101, 14:162, 15: 23, 16:151}
minuet_table[ 8] = { 1:152,  2: 60,  3:171,  4: 53,  5: 99,  6:133,  7: 21,  8:127,  9: 16, 10:155, 11: 57, 12:175, 13: 43, 14:168, 15: 89, 16:172}
minuet_table[ 9] = { 1:119,  2: 84,  3:114,  4: 50,  5:140,  6: 86,  7:169,  8: 94,  9:120, 10: 88, 11: 48, 12:166, 13: 51, 14:115, 15: 72, 16:111}
minuet_table[10] = { 1: 98,  2:142,  3: 42,  4:156,  5: 75,  6:129,  7: 62,  8:123,  9: 65, 10: 77, 11: 19, 12: 82, 13:137, 14: 38, 15:149, 16:  8}
minuet_table[11] = { 1:  3,  2: 87,  3:165,  4: 61,  5:135,  6: 47,  7:147,  8: 33,  9:102, 10:  4, 11: 31, 12:164, 13:144, 14: 59, 15:173, 16: 78}
minuet_table[12] = { 1: 54,  2:130,  3: 10,  4:103,  5: 28,  6: 37,  7:106,  8:  5,  9: 35, 10: 20, 11:108, 12: 92, 13: 12, 14:124, 15: 44, 16:131}

def reverse_enumeration(L):
   for index in reversed(range(len(L))):
      yield index, L[index]
   
# These individually varied repetitions implement the process used by Terry 
# Riley's "In C:" In each voice, randomly choose between 1 and N repetitions 
# for each measure of one complete circuit through a selected subset of the 
# minuet table. The number of repetitions of each measure will often differ 
# between the voices, and this creates many differential canons between the 
# voices.
# NOTE: A "measure" is Mozart's measure to be played for N repetitions, a 
# "bar" is one measure played one time.

rows_to_play = 5
columns_to_play = 12
measures_to_play = rows_to_play * columns_to_play
minimum_repetitions_per_measure = 1
maximum_repetitions_per_measure = 8 
repetitions_for_measures = []
for i in range(measures_to_play):
    repetitions_for_measures.append(random.randint(minimum_repetitions_per_measure, maximum_repetitions_per_measure))     
    
def read_measure(number):
    score_node = CsoundAC.ScoreNode()
    score_node.thisown = 0
    filename = 'M' + str(number) + '.mid'
    score_for_measure = score_node.getScore()
    score_for_measure.thisown = 0
    score_for_measure.load(filename)
    # Remove false notes.
    for i, event in reverse_enumeration(score_for_measure):
        if event.getChannel() < 0:
            score_for_measure.remove(i)
    print("Loaded '%s':\n%s" % (filename, score_for_measure.toString()))
    return score_node

tempo = 5./3.

scale = CsoundAC.Scale("D major")
scale.thisown = 0
chord = scale.chord(1, 4)
chord.thisown = 0
initial_bass = 32
forte_measures = random.choices([1,0], [2/6, 4/6], k=measures_to_play)

def build_voice(voiceleading_node, sequence, instrument, bass, time_offset, pan):
    global repetitions_for_measures
    global tempo
    global off_time
    global chord
    global scale
    # Ensure that each voice plays a different sequence of repetitions, as in 
    # "In C". But note that shuffling, rather than resampling, ensures 
    # that each voice actually plays the same number of bars.
    random.shuffle(repetitions_for_measures)
    random.shuffle(forte_measures)
    bars_total = sum(repetitions_for_measures)
    print("Instrument: {:3} measures: {} bars: {} repetitions_for_measures: {}".format(instrument, len(repetitions_for_measures), bars_total, repetitions_for_measures))    
    print("Instrument: {:3} measures: {} bars: {} forte_mearues:            {}".format(instrument, len(repetitions_for_measures), bars_total, forte_measures))    
    print()
    bars_played = 0
    real_time = 1.0
    cumulative_time = real_time + time_offset
    # Make both pitch range and dynamic range get bigger through time.
    bass = bass
    bass_at_end = bass - 5
    bass_increment_per_bar = (bass_at_end - bass) / bars_total
    range_ = 48.
    range_at_end = 52.
    range_increment_per_bar = (range_at_end - range_) / bars_total
    piano = 60.
    piano_at_end = 56.
    piano_increment_per_bar = (piano_at_end - piano) / bars_total
    dynamic_range = 20.
    dynamic_range_at_end = 30.
    dynamic_range_increment_per_bar = (dynamic_range_at_end - dynamic_range) / bars_total
    # Mozart's minuet table has columns indexed [1,16] and rows indexed [2,12]. 
    repetitions_for_measure_index = 0
    # Preserve Mozart's indexing.
    for minuet_column in range(1, columns_to_play + 1):
        for minuet_row in range(2, rows_to_play + 2):
            repetitions_for_measure = repetitions_for_measures[repetitions_for_measure_index]
            forte = forte_measures[repetitions_for_measure]
            repetitions_for_measure_index = repetitions_for_measure_index + 1
            scales = scale.modulations(chord)
            scale_count = len(scales)
            count = 0
            # After picking a number of repetitions for a measure, find if the 
            # current chord can be a pivot chord, and if so, choose one of the 
            # possible modulations to perform. Do this in the first voice 
            # only, but it will be applied to all voices.
            if (scale_count > 1 and time_offset == 0):
                random_index = random.randint(0, scale_count -1)
                for s in scales:
                    print("Possible modulation at: {:9.4f} {} {}".format(cumulative_time, s.toString(), s.name()))
                    if count == random_index:
                        scale = s  
                        print("             Chose modulation to: {} {}".format(scale.toString(), scale.name()))
                    count = count + 1
                print()
            for k in range(repetitions_for_measure):
                if time_offset == 0:
                    # Once the scale is chosen, perform root progressions 
                    # within the scale; away from the tonic in multiples of -2
                    # scale degrees, back to the tonic in multiples of 1 scale 
                    # degree with a preference for 3 steps (as used by V to I). 
                    # These root progressions are random but weighted.
                    progression = random.choices([-2, -4, -6, -8, -10, -12, 3, 6], [10, 3, 2, 1, 1, 1, 8, 3], k=1)
                    steps = progression[0]
                    chord = scale.transpose_degrees(chord, steps)
                    voiceleading_node.chord(chord, cumulative_time)
                measure = read_measure(minuet_table[minuet_row][minuet_column])
                score_for_measure = measure.getScore()
                duration = score_for_measure.getDuration() * tempo
                score_for_measure.setDuration(duration)
                rescale = CsoundAC.Rescale() 
                rescale.thisown = 0
                rescale.setRescale(CsoundAC.Event.TIME, bool(1), bool(0), cumulative_time, 0)
                rescale.setRescale(CsoundAC.Event.INSTRUMENT, bool(1), bool(1), instrument, 0)
                bass = bass + bass_increment_per_bar
                range_ = range_ + range_increment_per_bar
                rescale.setRescale(CsoundAC.Event.KEY, bool(1), bool(1), bass, range_)
                piano = piano + piano_increment_per_bar
                dynamic_range = dynamic_range + dynamic_range_increment_per_bar
                rescale.setRescale(CsoundAC.Event.VELOCITY, bool(1), bool(1), piano + (forte * 4), dynamic_range)
                rescale.setRescale(CsoundAC.Event.PAN, bool(1), bool(1), pan, float(0))
                rescale.thisown = 0
                rescale.addChild(measure)
                bars_played = bars_played + 1
                sequence.addChild(rescale)
                cumulative_time = cumulative_time + duration
                real_time = real_time + duration
    print("Bars played for instrument {}: {}".format(instrument, bars_played))
    print()

sequence = CsoundAC.Rescale()
sequence.thisown = 0
sequence.setRescale(CsoundAC.Event.VELOCITY, bool(1), bool(1), 60., 12.)
# The actual harmony is applied after the notes for all voices have been '
# generated.
voiceleading_node = CsoundAC.VoiceleadingNode()
voiceleading_node.thisown = 0
voiceleading_node.addChild(sequence);
model.addChild(voiceleading_node)

instruments_used = 0
total_instruments = 6
# Stagger starting times for each voice to create a canon at the very 
# beginning.
time_offset = (tempo * total_instruments) / 4.0
# Make it possible to experimentally shrink or enlarge the arrangement.
# 1 PianoNotePianoteq.
instruments_used = instruments_used + 1
build_voice(voiceleading_node, sequence, 1, initial_bass + 4, time_offset * (instruments_used - 1), (instruments_used / (total_instruments + 1)))
# 2 Zakian flute.
instruments_used = instruments_used + 1
build_voice(voiceleading_node, sequence, 2, initial_bass + 0, time_offset * (instruments_used - 1), (instruments_used / (total_instruments + 1)))
# 3 FMWaterBell.
instruments_used = instruments_used + 1
build_voice(voiceleading_node, sequence, 3, initial_bass + 1, time_offset * (instruments_used - 1), (instruments_used / (total_instruments + 1)))
# 4 Harpsichord.
instruments_used = instruments_used + 1
build_voice(voiceleading_node, sequence, 4, initial_bass + 0, time_offset * (instruments_used - 1), (instruments_used / (total_instruments + 1)))
# 5 ChebyshevMelody.
instruments_used = instruments_used + 1
build_voice(voiceleading_node, sequence, 5, initial_bass + 2, time_offset * (instruments_used - 1), (instruments_used / (total_instruments + 1)))
# 6 Rhodes.
instruments_used = instruments_used + 1
build_voice(voiceleading_node, sequence, 6, initial_bass + 4, time_offset * (instruments_used - 1), (instruments_used / (total_instruments + 1)))

# No #includes are used here, all Csound instruments are defined in this very file.
# The only non-standard external dependencies are the vst4cs opcodes, and the 
# Pianoteq physically modeled piano VST instrument plugin. These could easily be 
# replaced with a Fluidsynth sampled piano soundfont.

orc = '''
sr = 48000
ksmps = 128
nchnls = 2
0dbfs = 15

; Ensure the same random stream for each rendering.
; rand, randh, randi, rnd(x) and birnd(x) are not affected by seed.

seed 38493

gi_Pianoteq vst3init "/Library/Audio/Plug-Ins/VST3/Pianoteq 7.vst3", "Pianoteq 7", 1
vst3info gi_Pianoteq

alwayson "PianoOutPianoteq"
alwayson "ReverbSC"
alwayson "MasterOutput"

connect "ChebyshevMelody", "outleft", "ReverbSC", "inleft"
connect "ChebyshevMelody", "outright", "ReverbSC", "inright"
connect "FMWaterBell", "outleft", "ReverbSC", "inleft"
connect "FMWaterBell", "outright", "ReverbSC", "inright"
connect "Harpsichord", "outleft", "ReverbSC", "inleft"
connect "Harpsichord", "outright", "ReverbSC", "inright"
connect "Rhodes", "outleft", "ReverbSC", "inleft"
connect "Rhodes", "outright", "ReverbSC", "inright"
connect "ZakianFlute", "outleft", "ReverbSC", "inleft"
connect "ZakianFlute", "outright", "ReverbSC", "inright"

connect "PianoOutPianoteq", "outleft", "ReverbSC", "inleft"
connect "PianoOutPianoteq", "outright", "ReverbSC", "inright"
connect "ReverbSC", "outleft", "MasterOutput", "inleft"
connect "ReverbSC", "outright", "MasterOutput", "inright"

gk_PianoNotePianoteq_midi_dynamic_range chnexport "gk_PianoNotePianoteq_midi_dynamic_range", 3 ;  20
gk_PianoNotePianoteq_midi_dynamic_range init 20

instr PianoNotePianoteq
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_dynamic_range = i(gk_PianoNotePianoteq_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_velocity = p5
i_homogeneity = p11
instances active p1
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
i_result vst3note gi_Pianoteq, 0, i_midi_key, i_midi_velocity, i_duration
endin

gk_ZakianFlute_midi_dynamic_range init 80
gk_ZakianFlute_level init 0
gk_ZakianFlute_pan init .5
gi_ZakianFLute_seed init .5
gi_ZakianFLute_f2  ftgen 0, 0, 16, -2, 40, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 10240
gi_ZakianFlute_f26 ftgen 0, 0, 65537, -10, 2000, 489, 74, 219, 125, 9, 33, 5, 5
gi_ZakianFlute_f27 ftgen 0, 0, 65537, -10, 2729, 1926, 346, 662, 537, 110, 61, 29, 7
gi_ZakianFlute_f28 ftgen 0, 0, 65537, -10, 2558, 2012, 390, 361, 534, 139, 53, 22, 10, 13, 10
gi_ZakianFlute_f29 ftgen 0, 0, 65537, -10, 12318, 8844, 1841, 1636, 256, 150, 60, 46, 11
gi_ZakianFlute_f30 ftgen 0, 0, 65537, -10, 1229, 16, 34, 57, 32
gi_ZakianFlute_f31 ftgen 0, 0, 65537, -10, 163, 31, 1, 50, 31
gi_ZakianFlute_f32 ftgen 0, 0, 65537, -10, 4128, 883, 354, 79, 59, 23
gi_ZakianFlute_f33 ftgen 0, 0, 65537, -10, 1924, 930, 251, 50, 25, 14
gi_ZakianFlute_f34 ftgen 0, 0, 65537, -10, 94, 6, 22, 8
gi_ZakianFlute_f35 ftgen 0, 0, 65537, -10, 2661, 87, 33, 18
gi_ZakianFlute_f36 ftgen 0, 0, 65537, -10, 174, 12
gi_ZakianFlute_f37 ftgen 0, 0, 65537, -10, 314, 13
gi_ZakianFlute_wtsin ftgen 0, 0, 65537, 10, 1

instr ZakianFlute
; Author: Lee Zakian
; Adapted by: Michael Gogins
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_velocity = p5
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_overall_amps = 65.2
i_normalization = ampdb(-i_overall_amps) / 2
i_midi_dynamic_range = i(gk_ZakianFlute_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_ZakianFlute_level)
iattack = .002
isustain = p3
irelease = .3
xtratim iattack + irelease
iHz = cpsmidinn(i_midi_key)
kHz = k(iHz)
aenvelope transeg 1.0, 20.0, -10.0, 0.05
ip3 = (p3 < 3.0 ? p3 : 3.0)
; parameters
; p4 overall amplitude scaling factor
ip4 init i_amplitude
; p5 pitch in Hertz (normal pitch range: C4-C7)
ip5 init iHz
; p6 percent vibrato depth, recommended values in range [-1., +1.]
ip6 init 0.5
; 0.0 -> no vibrato
; +1. -> 1% vibrato depth, where vibrato rate increases slightly
; -1. -> 1% vibrato depth, where vibrato rate decreases slightly
; p7 attack time in seconds
; recommended value: .12 for slurred notes, .06 for tongued notes
; (.03 for short notes)
ip7 init .08
; p8 decay time in seconds
; recommended value: .1 (.05 for short notes)
ip8 init .08
; p9 overall brightness / filter cutoff factor
; 1 -> least bright / minimum filter cutoff frequency (40 Hz)
; 9 -> brightest / maximum filter cutoff frequency (10,240Hz)
ip9 init 5
; initial variables
iampscale = ip4 ; overall amplitude scaling factor
ifreq = ip5 ; pitch in Hertz
ivibdepth = abs(ip6*ifreq/100.0) ; vibrato depth relative to fundamental frequency
iattack = ip7 * (1.1 - .2*gi_ZakianFLute_seed) ; attack time with up to +-10% random deviation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947) ; reset gi_ZakianFLute_seed
idecay = ip8 * (1.1 - .2*gi_ZakianFLute_seed) ; decay time with up to +-10% random deviation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifiltcut tablei ip9, gi_ZakianFLute_f2 ; lowpass filter cutoff frequency
iattack = (iattack < 6/kr ? 6/kr : iattack) ; minimal attack length
idecay = (idecay < 6/kr ? 6/kr : idecay) ; minimal decay length
isustain = p3 - iattack - idecay
p3 = (isustain < 5/kr ? iattack+idecay+5/kr : p3) ; minimal sustain length
isustain = (isustain < 5/kr ? 5/kr : isustain)
iatt = iattack/6
isus = isustain/4
idec = idecay/6
iphase = gi_ZakianFLute_seed ; use same phase for all wavetables
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
; vibrato block
; kvibdepth linseg .1, .8*p3, 1, .2*p3, .7
kvibdepth linseg .1, .8*ip3, 1, isustain, 1, .2*ip3, .7
kvibdepth = kvibdepth* ivibdepth ; vibrato depth
kvibdepthr randi .1*kvibdepth, 5, gi_ZakianFLute_seed ; up to 10% vibrato depth variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kvibdepth = kvibdepth + kvibdepthr
ivibr1 = gi_ZakianFLute_seed ; vibrato rate
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ivibr2 = gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
if ip6 < 0 goto vibrato1
kvibrate linseg 2.5+ivibr1, p3, 4.5+ivibr2 ; if p6 positive vibrato gets faster
 goto vibrato2
vibrato1:
ivibr3 = gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kvibrate linseg 3.5+ivibr1, .1, 4.5+ivibr2, p3-.1, 2.5+ivibr3 ; if p6 negative vibrato gets slower
vibrato2:
kvibrater randi .1*kvibrate, 5, gi_ZakianFLute_seed ; up to 10% vibrato rate variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kvibrate = kvibrate + kvibrater
kvib oscili kvibdepth, kvibrate, gi_ZakianFlute_wtsin
ifdev1 = -.03 * gi_ZakianFLute_seed ; frequency deviation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifdev2 = .003 * gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifdev3 = -.0015 * gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifdev4 = .012 * gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kfreqr linseg ifdev1, iattack, ifdev2, isustain, ifdev3, idecay, ifdev4
kfreq = kHz * (1 + kfreqr) + kvib
if ifreq < 427.28 goto range1 ; (cpspch(8.08) + cpspch(8.09))/2
if ifreq < 608.22 goto range2 ; (cpspch(9.02) + cpspch(9.03))/2
if ifreq < 1013.7 goto range3 ; (cpspch(9.11) + cpspch(10.00))/2
goto range4
; wavetable amplitude envelopes
range1: ; for low range tones
kamp1 linseg 0, iatt, 0.002, iatt, 0.045, iatt, 0.146, iatt, \
0.272, iatt, 0.072, iatt, 0.043, isus, 0.230, isus, 0.000, isus, \
0.118, isus, 0.923, idec, 1.191, idec, 0.794, idec, 0.418, idec, \
0.172, idec, 0.053, idec, 0
kamp2 linseg 0, iatt, 0.009, iatt, 0.022, iatt, -0.049, iatt, \
-0.120, iatt, 0.297, iatt, 1.890, isus, 1.543, isus, 0.000, isus, \
0.546, isus, 0.690, idec, -0.318, idec, -0.326, idec, -0.116, idec, \
-0.035, idec, -0.020, idec, 0
kamp3 linseg 0, iatt, 0.005, iatt, -0.026, iatt, 0.023, iatt, \
0.133, iatt, 0.060, iatt, -1.245, isus, -0.760, isus, 1.000, isus, \
0.360, isus, -0.526, idec, 0.165, idec, 0.184, idec, 0.060, idec, \
0.010, idec, 0.013, idec, 0
iwt1 = gi_ZakianFlute_f26 ; wavetable numbers
iwt2 = gi_ZakianFlute_f27
iwt3 = gi_ZakianFlute_f28
inorm = 3949
goto end
range2: ; for low mid-range tones
kamp1 linseg 0, iatt, 0.000, iatt, -0.005, iatt, 0.000, iatt, \
0.030, iatt, 0.198, iatt, 0.664, isus, 1.451, isus, 1.782, isus, \
1.316, isus, 0.817, idec, 0.284, idec, 0.171, idec, 0.082, idec, \
0.037, idec, 0.012, idec, 0
kamp2 linseg 0, iatt, 0.000, iatt, 0.320, iatt, 0.882, iatt, \
1.863, iatt, 4.175, iatt, 4.355, isus, -5.329, isus, -8.303, isus, \
-1.480, isus, -0.472, idec, 1.819, idec, -0.135, idec, -0.082, idec, \
-0.170, idec, -0.065, idec, 0
kamp3 linseg 0, iatt, 1.000, iatt, 0.520, iatt, -0.303, iatt, \
0.059, iatt, -4.103, iatt, -6.784, isus, 7.006, isus, 11, isus, \
12.495, isus, -0.562, idec, -4.946, idec, -0.587, idec, 0.440, idec, \
0.174, idec, -0.027, idec, 0
iwt1 = gi_ZakianFlute_f29
iwt2 = gi_ZakianFlute_f30
iwt3 = gi_ZakianFlute_f31
inorm = 27668.2
goto end
range3: ; for high mid-range tones
kamp1 linseg 0, iatt, 0.005, iatt, 0.000, iatt, -0.082, iatt, \
0.36, iatt, 0.581, iatt, 0.416, isus, 1.073, isus, 0.000, isus, \
0.356, isus, .86, idec, 0.532, idec, 0.162, idec, 0.076, idec, 0.064, \
idec, 0.031, idec, 0
kamp2 linseg 0, iatt, -0.005, iatt, 0.000, iatt, 0.205, iatt, \
-0.284, iatt, -0.208, iatt, 0.326, isus, -0.401, isus, 1.540, isus, \
0.589, isus, -0.486, idec, -0.016, idec, 0.141, idec, 0.105, idec, \
-0.003, idec, -0.023, idec, 0
kamp3 linseg 0, iatt, 0.722, iatt, 1.500, iatt, 3.697, iatt, \
0.080, iatt, -2.327, iatt, -0.684, isus, -2.638, isus, 0.000, isus, \
1.347, isus, 0.485, idec, -0.419, idec, -.700, idec, -0.278, idec, \
0.167, idec, -0.059, idec, 0
iwt1 = gi_ZakianFlute_f32
iwt2 = gi_ZakianFlute_f33
iwt3 = gi_ZakianFlute_f34
inorm = 3775
goto end
range4: ; for high range tones
kamp1 linseg 0, iatt, 0.000, iatt, 0.000, iatt, 0.211, iatt, \
0.526, iatt, 0.989, iatt, 1.216, isus, 1.727, isus, 1.881, isus, \
1.462, isus, 1.28, idec, 0.75, idec, 0.34, idec, 0.154, idec, 0.122, \
idec, 0.028, idec, 0
kamp2 linseg 0, iatt, 0.500, iatt, 0.000, iatt, 0.181, iatt, \
0.859, iatt, -0.205, iatt, -0.430, isus, -0.725, isus, -0.544, isus, \
-0.436, isus, -0.109, idec, -0.03, idec, -0.022, idec, -0.046, idec, \
-0.071, idec, -0.019, idec, 0
kamp3 linseg 0, iatt, 0.000, iatt, 1.000, iatt, 0.426, iatt, \
0.222, iatt, 0.175, iatt, -0.153, isus, 0.355, isus, 0.175, isus, \
0.16, isus, -0.246, idec, -0.045, idec, -0.072, idec, 0.057, idec, \
-0.024, idec, 0.002, idec, 0
iwt1 = gi_ZakianFlute_f35
iwt2 = gi_ZakianFlute_f36
iwt3 = gi_ZakianFlute_f37
inorm = 4909.05
goto end
end:
kampr1 randi .02*kamp1, 10, gi_ZakianFLute_seed ; up to 2% wavetable amplitude variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kamp1 = kamp1 + kampr1
kampr2 randi .02*kamp2, 10, gi_ZakianFLute_seed ; up to 2% wavetable amplitude variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kamp2 = kamp2 + kampr2
kampr3 randi .02*kamp3, 10, gi_ZakianFLute_seed ; up to 2% wavetable amplitude variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kamp3 = kamp3 + kampr3
awt1 poscil kamp1, kfreq, iwt1, iphase ; wavetable lookup
awt2 poscil kamp2, kfreq, iwt2, iphase
awt3 poscil kamp3, kfreq, iwt3, iphase
asig = awt1 + awt2 + awt3
asig = asig*(iampscale/inorm)
kcut linseg 0, iattack, ifiltcut, isustain, ifiltcut, idecay, 0 ; lowpass filter for brightness control
afilt tone asig, kcut
a_signal balance afilt, asig
i_attack = .002
i_sustain = p3
i_release = 0.01
xtratim i_attack + i_sustain + i_release
a_declicking linsegr 0, i_attack, 1, i_sustain, 1, i_release, 0
a_signal = a_signal * i_amplitude * a_declicking * k_gain
#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
a_signal *= .7
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

//////////////////////////////////////////////
// Original by Steven Yi.
// Adapted by Michael Gogins.
//////////////////////////////////////////////
gk_FMWaterBell_level init 0
gi_FMWaterBell_attack init 0.002
gi_FMWaterBell_release init 0.01
gi_FMWaterBell_sustain init 20
gi_FMWaterBell_sustain_level init .1
gk_FMWaterBell_index init .5
gk_FMWaterBell_crossfade init .5
gk_FMWaterBell_vibrato_depth init 0.05
gk_FMWaterBell_vibrato_rate init 6
gk_FMWaterBell_midi_dynamic_range init 127
gi_FMWaterBell_cosine ftgen 0, 0, 65537, 11, 1
instr FMWaterBell
i_instrument = p1
i_time = p2
i_duration = p3
; One of the envelopes in this instrument should be releasing, and use this:
i_sustain = 1000
;xtratim gi_FMWaterBell_attack + gi_FMWaterBell_release
xtratim gi_FMWaterBell_attack + gi_FMWaterBell_release
i_midi_key = p4
i_midi_dynamic_range = i(gk_FMWaterBell_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 81
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization * 1.6
k_gain = ampdb(gk_FMWaterBell_level)
a_signal fmbell	1, i_frequency, gk_FMWaterBell_index, gk_FMWaterBell_crossfade, gk_FMWaterBell_vibrato_depth, gk_FMWaterBell_vibrato_rate, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine ;, gi_FMWaterBell_sustain
;a_envelope linsegr 0, gi_FMWaterBell_attack, 1, i_sustain, gi_FMWaterBell_sustain_level, gi_FMWaterBell_release, 0
a_envelope linsegr 0, gi_FMWaterBell_attack, 1, i_sustain, 1, gi_FMWaterBell_release, 0
; ares transegr ia, idur, itype, ib [, idur2] [, itype] [, ic] ...
; a_envelope transegr 0, gi_FMWaterBell_attack, 12, 1, i_sustain, 12, gi_FMWaterBell_sustain_level, gi_FMWaterBell_release, 12, 0
a_signal = a_signal * i_amplitude * a_envelope * k_gain
;_signal = a_signal * i_amplitude * k_gain
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
; printks "FMWaterBell    i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d l%9.4f r%9.4f\\n", 1, p1, p2, p3, p4, p5, p7, active(p1), dbamp(rms(a_out_left)), dbamp(rms(a_out_right))
endin

gk_Harpsichord_midi_dynamic_range init 127
gk_Harpsichord_level init 0
gk_Harpsichord_pick init .075
gk_Harpsichord_reflection init .5
gk_Harpsichord_pluck init .75
gi_Harpsichord_harptable ftgen 0, 0, 65537, 7, -1, 1024, 1, 1024, -1
instr Harpsichord
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_dynamic_range = i(gk_Harpsichord_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 110
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Harpsichord_level)
iHz = cpsmidinn(i_midi_key)
kHz = k(iHz)
aenvelope transeg 1.0, 40.0, -25.0, 0.0
apluck pluck i_amplitude, kHz, iHz, 0, 1
aharp poscil aenvelope, kHz, gi_Harpsichord_harptable
aharp2 balance apluck, aharp
a_signal	= (apluck + aharp2)
i_attack = .0005
i_sustain = p3
i_release = 0.01
xtratim i_attack + i_release
a_declicking linsegr 0, i_attack, 1, i_sustain, 1, i_release, 0
a_signal = a_signal * a_declicking * k_gain
#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
;printks "Harpsichord      %9.4f   %9.4f\\n", 0.5, a_out_left, a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

gk_ChebyshevMelody_level init 0
gi_ChebyshevMelody_attack init 0.003
gi_ChebyshevMelody_release init 0.01
gk_ChebyshevMelody_midi_dynamic_range init 127
gk_ChebyshevMelody_level init 0
gi_ChebyshevMelody_chebyshev ftgen 0, 0, 65537, -7, -1, 150, 0.1, 110, 0, 252, 0
gi_ChebyshevMelody_sine ftgen 0, 0, 65537, 10, 1
gi_ChebyshevMelody_cook3 ftgen 0, 0, 65537, 10, 1, .4, 0.2, 0.1, 0.1, .05
instr ChebyshevMelody
///////////////////////////////////////////////////////
// Original by Jon Nelson.
// Adapted by Michael Gogins.
///////////////////////////////////////////////////////
i_instrument = p1
i_time = p2
i_sustain = p3
xtratim gi_ChebyshevMelody_attack + gi_ChebyshevMelody_release
i_midi_key = p4
i_midi_dynamic_range = i(gk_ChebyshevMelody_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
// Spatial location is specified in Ambisonic coordinates.
k_space_front_to_back = p6
// AKA stereo pan.
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
// Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 72
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_ChebyshevMelody_level)

iattack = .01
isustain = p3
irelease = .01
p3 = iattack + isustain + irelease
iHz = cpsmidinn(i_midi_key)
iamplitude = ampdb(i_midi_velocity) * 7.
ip6 = gi_ChebyshevMelody_chebyshev
kHz = k(iHz)
idB = i_midi_velocity
i1 = iHz
k100 randi 1,0.05
ak101 poscil 1, 5 + k100, gi_ChebyshevMelody_sine
ak102 linseg 0, .5, 1, p3, 1
k100 = i1 + (ak101 * ak102)
; Envelope for driving oscillator.
ip3 init 3.0
; k1 linenr 0.5, ip3 * .3, ip3 * 2, 0.01
ak1 linseg 0, ip3 * .3, .5, ip3 * 2, 0.01, isustain, 0.01, irelease, 0
; k2 line 1, p3, .5
ak2 linseg 1.0, ip3, .5, isustain, .5, irelease, 0
ak1 = ak2 * ak1
; Amplitude envelope.
ak10 expseg 0.0001, iattack, 1.0, isustain, 0.8, irelease, .0001
ak10 = (ak10 - .0001)
; Power to partials.
k20 linseg 1.485, iattack, 1.5, (i_sustain + irelease), 1.485
; a1-3 are for cheby with p6=1-4
a1 poscil ak1, k100 - .25, gi_ChebyshevMelody_cook3
; Tables a1 to fn13, others normalize,
a2 tablei a1, ip6, 1, .5
a3 balance a2, a1
; Try other waveforms as well.
a4 foscili 1, k100 + .04, 1, 2.000, k20, gi_ChebyshevMelody_sine
a5 poscil 1, k100, gi_ChebyshevMelody_sine
a6 = ((a3 * .1) + (a4 * .1) + (a5 * .8)) * ak10
a7 comb a6, .5, 1 / i1
a8 = (a6 * .9) + (a7 * .1)
asignal balance a8, a1
a_declicking linsegr 0, gi_ChebyshevMelody_attack, 1, i_sustain, 1, gi_ChebyshevMelody_release, 0
a_signal = asignal * i_amplitude * a_declicking * k_gain
#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
printks "ChebyshevMelody i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d l%9.4f r%9.4f\\n", .1, p1, p2, p3, p4, p5, p7, active(p1), dbamp(rms(a_out_left)), dbamp(rms(a_out_right))
endin

gk_Rhodes_level init 0
gk_Rhodes_midi_dynamic_range init 127
gi_Rhodes_sine ftgen 0, 0, 65537, 10, 1
gi_Rhodes_cosine ftgen 0, 0, 65537, 11, 1
gi_Rhodes_blank ftgen 0, 0, 65537, 10, 0 ; Blank wavetable for some Cook FM opcodes.
instr Rhodes
; Authors: Perry Cook, John ffitch, Michael Gogins
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_dynamic_range = i(gk_Rhodes_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
i_midi_velocity = p5
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_overall_amps = 82
i_normalization = ampdb(-i_overall_amps) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Rhodes_level)
iindex = 4
icrossfade = 3
ivibedepth = 0.2
iviberate = 6
ifn1 = gi_Rhodes_sine
ifn2 = gi_Rhodes_cosine
ifn3 = gi_Rhodes_sine
ifn4 = gi_Rhodes_blank
ivibefn = gi_Rhodes_sine
a_signal fmrhode i_amplitude, i_frequency, iindex, icrossfade, ivibedepth, iviberate, ifn1, ifn2, ifn3, ifn4, ivibefn
i_attack = .002
i_sustain = p3
i_release = 0.01
xtratim i_attack + i_release
a_declicking linsegr 0, i_attack, 1, i_sustain, 1, i_release, 0
a_signal = a_signal * a_declicking * k_gain
#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

gk_PianoOutPianoteq_level chnexport "gk_PianoOutPianoteq_level", 3 ;  0
gk_PianoOutPianoteq_front_to_back chnexport "gk_PianoOutPianoteq_front_to_back", 3 ;  0
gk_PianoOutPianoteq_left_to_right chnexport "gk_PianoOutPianoteq_left_to_right", 3 ;  0.5
gk_PianoOutPianoteq_bottom_to_top chnexport "gk_PianoOutPianoteq_bottom_to_top", 3 ;  0

gk_PianoOutPianoteq_level init 0
gk_PianoOutPianoteq_front_to_back init 0
gk_PianoOutPianoteq_left_to_right init 0.5
gk_PianoOutPianoteq_bottom_to_top init 0

instr PianoOutPianoteq
k_gain = ampdb(gk_PianoOutPianoteq_level)
i_overall_amps = 87
i_normalization = ampdb(-i_overall_amps) * 2
i_amplitude = ampdb(80) * i_normalization
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_velocity = p5
ainleft init 0
ainright init 0
aoutleft, aoutright vst3audio gi_Pianoteq
a_signal = aoutleft + aoutright
a_signal *= k_gain
a_signal *= i_amplitude
a_out_left, a_out_right pan2 a_signal, gk_PianoOutPianoteq_left_to_right

kx = gk_PianoOutPianoteq_front_to_back
ky = gk_PianoOutPianoteq_left_to_right
kz = gk_PianoOutPianoteq_bottom_to_top

a_out_left, a_out_right pan2 a_signal, ky
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

gk_Reverb_feedback init 0.875
gk_Reverb_wet init 0.5
gi_Reverb_delay_modulation init 0.0075
gk_Reverb_frequency_cutoff init 15000
instr ReverbSC
gk_Reverb_dry = 1.0 - gk_Reverb_wet
aleftin init 0
arightin init 0
aleftout init 0
arightout init 0
aleftin inleta "inleft"
arightin inleta "inright"
aleftout, arightout reverbsc aleftin, arightin, gk_Reverb_feedback, gk_Reverb_frequency_cutoff, sr, gi_Reverb_delay_modulation
aleftoutmix = aleftin * gk_Reverb_dry + aleftout * gk_Reverb_wet
arightoutmix = arightin * gk_Reverb_dry + arightout * gk_Reverb_wet
outleta "outleft", aleftoutmix
outleta "outright", arightoutmix
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

gk_MasterOutput_level init 0
gS_MasterOutput_filename init "check.wav"
instr MasterOutput
aleft inleta "inleft"
aright inleta "inright"
k_gain = ampdb(gk_MasterOutput_level)
printks2 "Master gain: %f\\n", k_gain
iamp init 1
aleft butterlp aleft, 18000
aright butterlp aright, 18000
outs aleft * k_gain, aright * k_gain
; We want something that will play on my phone.
i_amplitude_adjustment = 1; ampdbfs(-3) / 32767
i_filename_length strlen gS_MasterOutput_filename
if i_filename_length > 0 goto filename_exists
goto filename_endif
filename_exists:
prints sprintf("Output filename: %s\\n", gS_MasterOutput_filename)
fout gS_MasterOutput_filename, 18, aleft * i_amplitude_adjustment, aright * i_amplitude_adjustment
filename_endif:
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

; It is important for levels to be evenly balanced _on average_! This 
; enables instruments to come forward and recede according to their 
; response to MIDI velocity.

gk_PianoOutPianoteq_level init  26 
gk_ZakianFlute_level init       19
gk_FMWaterBell_level init       24;16
gk_ChebyshevMelody_level init   28;19
gk_Harpsichord_level init       11
gk_Rhodes_level init            31

gk_Reverb_wet init 0.25
gk_Reverb_feedback init 0.85
gi_Reverb_delay_modulation init 0.0875
gk_Reverb_frequency_cutoff init 14000
'''

model.setCsoundOrchestra(orc)
model.setCsoundCommand(csound_command)
model.generate()
# Fix ending. Instruments would drop out, sustain till the end. Some notes 
# course will decay first.
print("score:\n", score.getCsoundScore())
score_duration = score.getDuration() + 4.
print("score_duration: ", score_duration)
sounding = set()
for i, event in reverse_enumeration(score):
    event.setOffTime(score_duration)
    CsoundAC.conformToChord(event, chord)
    instrument = str(event.getInstrument())
    sounding.add(instrument)
    if len(sounding) == total_instruments:
        break
score.save(model.getMidifileFilepath())
## print("score again:\n", score.toString())
model.performMaster()
if rendering == 'soundfile':
    model.translateMaster()
