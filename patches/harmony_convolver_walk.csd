<CsoundSynthesizer>
<CsOptions>
-m0 -RWdo "harmony_convolver_walk.wav"  
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 32
nchnls  = 2
0dbfs   = 1000

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
  prints "Evoke:  seconds: %12.3f duration: %9.3f fadein: %5.2f fadeout: %5.2f impulse duration: %5.3f impulse gain: %5.3ff dirac level: %5.3f pitch-classes: %3d %3d %3d %3d %3d\n", p2, p3, i_fadein, i_fadeout, k_kernel_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5
endin

instr master_output
  k_time times
  a_left  inleta "leftin"
  a_right inleta "rightin"
  outs a_left, a_right
  ; compute RMS at k-rate
  k_rms_left   rms a_left
  k_rms_right  rms a_right
  ; convert to dBFS using the orchestra's 0dbfs value
  k_norm_left  = k_rms_left  / 0dbfs
  k_norm_right = k_rms_right / 0dbfs
  k_db_left    = 20 * log10(k_norm_left  + 1e-20)
  k_db_right   = 20 * log10(k_norm_right + 1e-20)
  ; print every second
  printks "Output: seconds: %12.3f L: %9.4f dBFS   R: %9.4f dBFS\n", 1, k_time, k_db_left, k_db_right
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
i "evoke"   0.000   [ 29.266 -   0.000]   1.00     1.00       0.03         0.3   0.6   0 4 7 11 14 
i "evoke"  29.266   [ 44.929 -  29.266]   1.00     1.00       0.04         0.1   0.6   2 5 9 12 14
i "evoke"  44.929   [ 97.100 -  44.929]   1.00     1.00       0.06         0.15  0.7   5 7 9 14
i "evoke"  97.100   [123.308 -  97.100]   1.00     1.00       0.05         0.1   0.5   2 5 9 12 4
; Three bell strikes.
i "evoke" 123.308   [125.140 - 123.308]   0.05     0.05       0.05         0.4   0.5   7 10 13 15
i "evoke" 125.140   [126.922 - 125.140]   0.05     0.05       0.07         0.5   0.5   3 6 10 13 15
i "evoke" 126.922   [149.000 - 126.922]   0.05    10.00       0.12         0.2   0.5   0 4 7 11 14
; Voices and cars return.
i "evoke" 149.000   [243.000 - 149.000]  10.00     1.00       0.02         0.1   0.6   10 12 14 17
; Three more bell strikes.
i "evoke" 243.000   [245.000 - 243.000]   1.00     1.00       0.12         0.1   0.5    7 11 14 17 21
i "evoke" 245.000   [247.000 - 245.000]   1.00     1.00       0.16         0.11  0.5   4 11 7  10 14
i "evoke" 247.000   [260.000 - 247.000]   1.00     1.00       0.20         0.12  0.5   5  9 0 4 6
; Other sounds again.
i "evoke" 260.000   [306.000 - 260.000]   1.00     1.00       0.03         0.2   0.7   0 4 7 11 14
; A woman.
i "evoke" 306.00    [320.781 - 306.000]   0.50    10.00       0.12         0.5   0.4   7 10 11 14
; Other sounds.
i "evoke" 320.781   [431.000 - 320.781]  10.00     1.00       0.01         0.4   0.7   0 4 7 11
i "evoke" 431.000   [536.740 - 431.000]   1.00     1.00       0.70         0.8   0.2   2 5 9 0
i "evoke" 536.740   [575.000 - 536.740]   1.00     1.00       2.00         0.2   0.2   0 4 7 11 
 </CsScore>
</CsoundSynthesizer>
