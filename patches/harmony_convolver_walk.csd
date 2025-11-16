<CsoundSynthesizer>
<CsOptions>
-m163 -RWdo "harmony_convolver_walk.wav"  
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 32
nchnls  = 2
0dbfs   = 10000

#include "harmony_convolver.inc"

instr source_sound
  a_input_left, a_input_right soundin "/Users/michaelgogins/Dropbox/imparting_harmonies/source_soundfiles/TASCAM_0101.normalized.wav"
  outleta "leftout", a_input_left
  outleta "rightout", a_input_right
endin

/*
  a_output harmony_convolver k_kernel_duration, i_impulse_gain, 
    i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, 
    i_pitch_class_4, i_pitch_class_5
*/

instr evoke
  a_input_left inleta "leftin"
  a_input_right inleta "rightin"
  i_gain init .3
  i_fadein init p4
  i_fadeout init p5
  k_kernel_duration init p6
  i_impulse_gain init p7
  i_dirac_level init p8
  i_pitch_class_1 init p9
  i_pitch_class_2 init p10    
  i_pitch_class_3 init p11    
  i_pitch_class_4 init p12    
  i_pitch_class_5 init p13
  a_envelope linsegr 0, i_fadein, 1, p3, 1, i_fadeout, 0
  a_convolved_left harmony_convolver a_input_left, k_kernel_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5
  a_convolved_right harmony_convolver a_input_right, k_kernel_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5
  a_convolved_left *= i_gain
  a_convolved_right *= i_gain
  a_convolved_left *= a_envelope
  a_convolved_right *= a_envelope
  outleta "leftout", a_convolved_left
  outleta "rightout", a_convolved_right
  prints "Evoke instrument: time: %f duration: %f fadein: %f fadeout: %f kernel duration: %f impulse gain: %f dirac level: %f pitch classes: %d %d %d %d %d\n", p2, p3, i_fadein, i_fadeout, k_kernel_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5
endin

instr master_output
  a_left inleta "leftin"
  a_right inleta "rightin"
  outs a_left, a_right
endin

connect "source_sound",   "leftout",     "evoke",     	"leftin"
connect "source_sound",   "rightout",    "evoke",     	"rightin"

connect "evoke",   "leftout",     "master_output",     	"leftin"
connect "evoke",   "rightout",    "master_output",     	"rightin"

alwayson "source_sound"
alwayson "master_output"

</CsInstruments>
<CsScore>
;           onset             duration  fadein  fadeout kernel_dur kernel_gain dirac pitch_classes
; Voices and cars
i "evoke"   0.000   [ 28.729 -   0.000]       1        1       0.01          .1    .5   0 4 7 11 14 
i "evoke"  28.729   [ 37.200 -  28.739]       1        1       0.05          .1    .5   2 5 9 12 14
i "evoke"  37.200   [ 45.000 -  37.200]       1        1       0.25          .2    .2   5 7 9 14
i "evoke"  45.000   [ 90.000 -  45.000]       1        1       0.10          .1    .2   4 7 11 14 
i "evoke"  90.000   [123.270 -  90.000]       1        1       0.05          .1    .5   2 5 9 12 12
; Three bell strikes
i "evoke" 123.270   [124.000 - 123.000]      .25     .25       0.05          .1    .5  3 6 10 13 15
i "evoke" 124.000   [125.140 - 124.000]      .25     .24       0.05          .1    .1  10 13 16 20
i "evoke" 125.140   [431.000 - 125.140]      .25      1        2.00          .1    .05  0 4 7 11
</CsScore>
</CsoundSynthesizer>
