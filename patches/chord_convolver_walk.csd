<CsoundSynthesizer>

A WALK IN LAGARDA

Author:     Michael Gogins
License:    Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
Copyright:  2025 Michael Gogins

This is a short walk through the small village of Lagarde, Ariege, France, in 
autumn, with sounds of voices, cars, jackdaws, and the church bell. The source 
recording is convolved with various grains of sound built up from sinusoids on 
each octave of a set of pitch-classes, thus evoking or imparting different 
harmonic content on different segments of the recording. This convolution is 
done using the `chord_convolver` opcode defined by me.

"Lagarda" is the Occitan name for this ancient village of Lagarde.

<CsLicense>
</CsLicense>
<CsOptions>
-m32 -RWdo "chord_convolver_walk.wav"  
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 32
nchnls  = 2
0dbfs   = 20

#include "chord_convolver.inc"

opcode splice_crossfade_stereo, aa, Siii
  S_filename, i_start_skip, i_end_skip, i_fade_dur xin
  ; basic sanity checks
  if (i_end_skip <= i_start_skip) then
    prints "splice_crossfade_stereo: i_end_skip must be > i_start_skip.\n"
  endif
  if (i_fade_dur >= i_start_skip) then
    prints "splice_crossfade_stereo: i_fade_dur must be < i_start_skip.\n"
  endif
  ; two playheads on the same file (stereo)
  a_pre_left,  a_pre_right  diskin2 S_filename, 1, 0
  a_post_left, a_post_right diskin2 S_filename, 1, i_end_skip - i_start_skip
  k_time timeinsts
  i_crossfade_start = i_start_skip - i_fade_dur
  i_pi init 3.14159265358979
  a_out_left  init 0
  a_out_right init 0
  if (k_time < i_crossfade_start) then
    ; before crossfade: original section
    a_out_left  = a_pre_left
    a_out_right = a_pre_right
  elseif (k_time < i_start_skip) then
    ; crossfade region: raised cosine
    k_phase    = (k_time - i_crossfade_start) / i_fade_dur
    k_fade_in  = 0.5 - 0.5 * cos(i_pi * k_phase)
    k_fade_out = 1.0 - k_fade_in
    a_out_left  = a_pre_left  * k_fade_out + a_post_left  * k_fade_in
    a_out_right = a_pre_right * k_fade_out + a_post_right * k_fade_in
  else
    ; after crossfade: only later section
    a_out_left  = a_post_left
    a_out_right = a_post_right
  endif
  xout a_out_left, a_out_right
endop

instr source_sound
  a_input_left, a_input_right splice_crossfade_stereo \
    "/Users/michaelgogins/Dropbox/imparting_harmonies/source_soundfiles/TASCAM_0101.normalized.wav", 420, 480, .2
  ; Bells are too loud, compress.
  ; k-rate parameters, initialized at i-time
  k_thresh     init 30      ; noise floor / gate threshold
  k_lo_knee    init 55      ; start knee a bit lower (was 60)
  k_hi_knee    init 72      ; end knee a bit lower (was 75)
  k_ratio      init 8       ; stronger compression (was 6)
  k_attack     init 0.01    ; 10 ms attack – keep bell transients
  k_release    init 0.30    ; longer release (was 0.20) for smoother leveling
  i_lookahead = 0.05        ; 50 ms lookahead
  a_compressed_left  compress a_input_left,  a_input_left,  k_thresh, k_lo_knee, k_hi_knee, \
                                       k_ratio, k_attack, k_release, i_lookahead
  a_compressed_right compress a_input_right, a_input_right, k_thresh, k_lo_knee, k_hi_knee, \
                                       k_ratio, k_attack, k_release, i_lookahead
  outleta "leftout", a_compressed_left
  outleta "rightout", a_compressed_right
endin

/*
  a_output chord_convolver a_input, i_impulse_duration,
    i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials...
*/

instr evoke
  a_input_left inleta "leftin"
  a_input_right inleta "rightin"
  i_gain init .3
  i_fadein init p4
  i_fadeout init p5
  i_kernel_duration init p6
  i_impulse_gain init p7
  i_dirac_gain init p8
  i_compensation_type init p9
  i_partials[] passign 10
  a_envelope linsegr 0, i_fadein, 1, p3, 1, i_fadeout, 0
  a_convolved_left chord_convolver a_input_left, i_kernel_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  a_convolved_right chord_convolver a_input_right, i_kernel_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  a_convolved_left *= i_gain
  a_convolved_right *= i_gain
  a_convolved_left *= a_envelope
  a_convolved_right *= a_envelope
  outleta "leftout", a_convolved_left
  outleta "rightout", a_convolved_right
  prints "Evoke:  seconds: %12.3f duration: %9.3f fadein: %5.2f fadeout: %5.2f impulse duration: %5.3f impulse gain: %5.3f dirac gain: %5.3f compensation: %d n_pcs: %d\n", p2, p3, i_fadein, i_fadeout, i_kernel_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, lenarray(i_partials)
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
;            onset                     duration              fadein  fadeout kernel_dur kernel_gain dirac comp pitch_classes
; Voices and cars. Compensation 1 = equal energy. Trailing -1 clears carried p-fields.
i "evoke"    0.000          [ 29.266 -   0.000]                8.00     1.00       0.03         0.3   0.6  1  0 4 7 11 14 -1
i "evoke"   29.266          [ 44.929 -  29.266]                1.00     1.00       0.04         0.1   0.6  1  2 5 9 12 14 -1
i "evoke"   44.929          [ 97.100 -  44.929]                1.00     1.00       0.06         0.15  0.7  1  5 7 9 14 -1
i "evoke"   97.100          [123.308 -  97.100]                1.00     1.00       0.05         0.1   0.5  1  2 5 9 12 4 -1
; Three bell strikes. Stronger chord vs dry (was impulse 0.1–0.2, Dirac 0.8–0.9).
i "evoke"  123.308          [125.140 - 123.308]                0.05     0.05       0.08         0.45  0.35  1  7 10 13 15 -1
i "evoke"  125.140          [126.922 - 125.140]                0.05     0.05       0.08         0.50  0.30  1  3 6 10 13 15 -1
i "evoke"  126.922          [149.000 - 126.922]                0.05    10.00       0.06         0.40  0.35  1  0 4 7 11 14 -1
; Voices and cars return.
i "evoke"  149.000          [243.000 - 149.000]               10.00     1.00       0.02         0.1   0.6  1  10 12 14 17 -1
; Three more bell strikes. Stronger chord vs dry (was impulse ~0.1, Dirac 0.5).
i "evoke"  243.000          [245.000 - 243.000]                1.00     1.00       0.14         0.40  0.28  1  7 11 14 17 21 -1
i "evoke"  245.000          [247.000 - 245.000]                1.00     1.00       0.18         0.42  0.26  1  4 11 7 10 14 -1
i "evoke"  247.000          [260.000 - 247.000]                1.00     1.00       0.22         0.45  0.25  1  5 9 0 4 6 -1
; Other sounds again.
i "evoke"  260.000          [306.750 - 260.000]                1.00     0.25       0.03         0.1   0.7  1  0 4 7 11 14 -1
; A woman.
i "evoke"  306.750          [320.781 - 306.750]                0.25    10.00       0.12         0.5   0.4  1  7 10 11 14 -1
; Other sounds.
i "evoke"  320.781          [431.000 - 320.781]               10.00     1.00       0.01         0.4   0.7  1  0 4 7 11 -1

; 60 seconds cut from source recording starting at 420 seconds.

i "evoke" [431.000 - 60]   [(536.740 - 60) - (431.000 - 60)]   1.00     1.00       0.06         0.6   0.7  1  2 6 9 1 -1
; Walking through fallen leaves.
i "evoke" [536.740 - 60]   [(544.000 - 60) - (536.740 - 60)]   1.00     0.50       0.01         0.2   0.9  1  4 7 11 2 -1
i "evoke" [544.500 - 60]   [(558.000 - 60) - (544.500 - 60)]   0.50     1.00       0.06         0.2   0.7  1  7 11 2 5 -1
i "evoke" [558.000 - 60]   [(576.000 - 60) - (558.000 - 60)]   0.50     1.00       0.02         0.2   0.9  1  0 4 7 11 -1 

 </CsScore>
</CsoundSynthesizer>
