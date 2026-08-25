'''
Tests and demonstrates `chord_convolver.inc`.

The piece loops over changes of parameters to the `chord_convolver` opcode,
and at each setting, plays first noise then an ascending sawtooth through the opcode,
during which a ii-V-I chord progression is evoked from the source.
'''

import sys
from pathlib import Path

# Prefer the Csound 7 ctcsound shipped with the csound tree if not installed.
_csound7_python = Path.home() / "csound" / "Python"
if _csound7_python.is_dir() and str(_csound7_python) not in sys.path:
    sys.path.insert(0, str(_csound7_python))

import ctcsound

# Csound 7 API (snake_case): compile_csd / start / perform_ksmps
csound = ctcsound.Csound()
csd = '''<CsoundSynthesizer>
<CsOptions>
-m163 -RWdfo harmony_convolver_test.wav 
</CsOptions>
<CsInstruments>
sr = 48000
ksmps = 128
nchnls = 2
0dbfs = 1000
                      
#include "chord_convolver.inc"
                      
alwayson "MasterOutput"  
                      
ga_input_left      init 0
ga_input_right     init 0
ga_mix_left        init 0
ga_mix_right       init 0
gi_fade init 1
                      
instr Noise
  k_env linsegr 0, gi_fade, p3, gi_fade, 0
  a_source rand 0.3
  a_source tone a_source, 10000
  a_source *= k_env
  ga_input_left  += a_source
  ga_input_right += a_source
  prints "Noise: p1 %f p2 %f p3 %f\\n", p1, p2, p3
endin
                      
instr Saw
  k_env linsegr 0, gi_fade, p3, gi_fade, 0
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
  i_impulse_duration init p4
  i_impulse_gain init p5
  i_dirac_gain init p6
  i_compensation_type init p7
  i_partials[] passign 8
  a_source_left  = ga_input_left
  a_source_right = ga_input_right
  a_processed_left chord_convolver a_source_left, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  a_processed_right chord_convolver a_source_right, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  k_envelope linsegr 0, gi_fade, p3, gi_fade, 0
  ga_mix_left  += a_processed_left  * k_envelope
  ga_mix_right += a_processed_right * k_envelope
  prints "Evoker: p1 %f p2 %f p3 %f kernel dur %f impulse gain %f dirac gain %f compensation %d n_pcs %d\\n", p1, p2, p3, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, lenarray(i_partials)

endin

instr MasterOutput
  outs ga_mix_left, ga_mix_right
  ga_input_left  = 0
  ga_input_right = 0
  ga_mix_left  = 0
  ga_mix_right = 0
endin                   

</CsInstruments>
</CsoundSynthesizer>
'''   

'''
a_output chord_convolver a_input, i_impulse_duration,
  i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials...
'''
k_kernel_durations = [.01, .1, 1, 4]
i_impulse_gains = [.125, .25, .75]
i_dirac_gains = [.75, .5, .25, 1]
i_compensation_type = 1
source_instruments = [2, 1]
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
            for i_dirac_gain in i_dirac_gains:
                print(
                    f"Setting {setting_number:3d}: "
                    f"kernel duration {k_kernel_duration:9.4f} "
                    f"impulse gain {i_impulse_gain:9.4f} "
                    f"dirac gain {i_dirac_gain:9.4f}"
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
                        chord_event = [3, time, chord_duration, k_kernel_duration, i_impulse_gain, i_dirac_gain, i_compensation_type] + list(chord) + [-1]
                        score_line = "i " + " ".join(str(x) for x in chord_event)
                        time = time + chord_duration
                        score_lines.append(score_line)
    return '\n'.join(score_lines)
score = generate_score()
csd = csd.replace('''</CsInstruments>''', '''</CsInstruments>\n<CsScore>\n''' + score + '\n</CsScore>\n')
print(csd)
with open("from_python.csd", "w") as f:
    f.write(csd)
# mode 1: csd is a full CSD text string (not a filename)
csound.compile_csd(csd, 1)
csound.start()
while csound.perform_ksmps() == 0:
    pass
csound.reset()

