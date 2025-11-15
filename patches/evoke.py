'''
Tests and demonstrates `evoke_harmony_convolution.inc`.

The piece loops over changes of parameters to the `evoke_harmony_convolution` opcode, 
and at each setting, plays first noise then an ascending sawtooth through the opcode,
during which  ii-V-I chord progression is eoked from the source.
'''

import sys
import ctcsound
csound = ctcsound.Csound()
csound.compileCsdText('''
<CsoundSynthesizer>
<CsOptions>
-m163 -RWdfo evoke_harmony_convolution_test.wav 
</CsOptions>
<CsInstruments>
sr = 48000
ksmps = 128
nchnls = 2
0dbfs = 1000
                      
#include "evoke_harmony_convolution.inc"
                      
alwayson "MasterOutput"  
                      
ga_input_left      init 0
ga_input_right     init 0
ga_mix_left        init 0
ga_mix_right       init 0
                      
instr Noise
  k_env linsegr 0, 1, p3, 1, 0
  a_source rand 0.3
  a_source tone a_source, 10000
  a_source *= k_env
  ga_input_left  += a_source
  ga_input_right += a_source
  prints "Noise: p1 %f p2 %f p3 %f\\n", p1, p2, p3
endin
                      
instr Saw
  k_env linsegr 0, 1, p3, 1, 0
  k_amplitude = 0.3
  ; Frequency ramps from start of fade-in to end of fade-out
  k_hz line 32.70319783, p3, 523.2511307
  i_mode init 8
  a_signal vco2 k_amplitude, k_hz, i_mode
  a_signal *= k_env
  ga_input_left  += a_signal
  ga_input_right += a_signal
  prints "Saw: p1 %f p2 %f p3 %f\\n", p1, p2, p3
endin
                                   
instr Evoker
  i_fade_in_seconds   = p4
  i_fade_out_seconds  = p5
  k_kernel_duration = p6
  i_impulse_gain = p7
  i_dirac_level  = p8
  i_pitch_class_1 = p9
  i_pitch_class_2 = p10
  i_pitch_class_3 = p11
  i_pitch_class_4 = p12
  i_pitch_class_5 = p13
  a_source_left  = ga_input_left
  a_source_right = ga_input_right
  a_processed_left evoke_harmony_convolution a_source_left, k_kernel_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5   
  a_processed_right evoke_harmony_convolution a_source_right, k_kernel_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5   
  k_envelope linsegr 0, i_fade_in_seconds, p3, i_fade_out_seconds, 0
  ga_mix_left  += a_processed_left  * k_envelope
  ga_mix_right += a_processed_right * k_envelope
  prints "Evoker: p1 %f p2 %f p3 %f kernel dur %f impulse gain %f dirac level %f\\n", p1, p2, p3, k_kernel_duration, i_impulse_gain, i_dirac_level
endin

instr MasterOutput
  outs ga_mix_left, ga_mix_right
  ga_input_left  = 0
  ga_input_right = 0
  ga_mix_left  = 0
  ga_mix_right = 0
endin                   

instr ExitNow
    exitnow
endin 
</CsInstruments>
</CsoundSynthesizer>
''')
'''
a_output evoke_harmony_convolution k_kernel_duration, i_impulse_gain, 
  i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, 
  i_pitch_class_4, i_pitch_class_5
'''
k_kernel_durations = [.1, 1, 5]
i_impulse_gains = [.5, .25, .125]
i_dirac_levels = [.5, .25, .125]
source_instruments = [1, 2]
chords = []
chords.append([2, 5, 9, 12])   
chords.append([7, 11, 14, 17])  
chords.append([0, 4, 7, 11, 14])  
print("chords:", chords)

def generate_score():
    score_lines = []
    time = 0
    chord_duration = 5
    setting_number = 1
    for k_kernel_duration in k_kernel_durations:
        for i_impulse_gain in i_impulse_gains:
            for i_dirac_level in i_dirac_levels:
                print(
                    f"Setting {setting_number:3d}: "
                    f"kernel duration {k_kernel_duration:9.4f} "
                    f"impulse gain {i_impulse_gain:9.4f} "
                    f"dirac level {i_dirac_level:9.4f}"
            )
                setting_number += 1
                for source_instrument in source_instruments:
                    # Schedule the source instrument.
                    source_event = [source_instrument, time, chord_duration * 3]
                    score_line = "i " + " ".join(str(x) for x in source_event)
                    score_lines.append(score_line)
                    # Schedule the chords.
                    for i in range(len(chords)):
                        chord = chords[i]
                        # print("chord: ", chord)
                        # Schedule the chords.
                        p4 = k_kernel_duration
                        p5 = i_impulse_gain
                        p6 = i_dirac_level
                        chord_event = [3, time, chord_duration, 1, 1, p4, p5, p6, chord[0], chord[1], chord[2], chord[3]]
                        # print("chord_event:", chord_event) 
                        score_line = "i " + " ".join(str(x) for x in chord_event)
                        time = time + chord_duration
                        score_lines.append(score_line)
    score_lines.append(f'i 5 {time} 1')
    return '\n'.join(score_lines)
score = generate_score()
print(score)
csound.start()
csound.readScore(score)
csound.perform()

