/*
  Compare three treatments of the same sources (click, white noise):

    1) classic comb filter  — teeth periodic in *linear* Hz
    2) generic constant-Q bank — log-spaced every 3 semitones (diminished-like tiling)
    3) harmony_convolver_scaled — sparse PC×octave lattice

  Timeline (1 s silence before each segment; log-frequency spectrogram):

    t      source   treatment
    1–3    click    dry
    4–6    click    comb
    7–9    click    generic CQT
    10–12  click    harmony convolver
    13–17  noise    dry
    18–22  noise    comb
    23–27  noise    generic CQT
    28–32  noise    harmony convolver

  Chord for the convolver: pitch-classes 0, 4, 7, 11 (C E G B).
  Comb fundamental: MIDI 36 (C2) so teeth are n * 65.4 Hz.
*/

<CsoundSynthesizer>
<CsOptions>
-m32 -d -RWfo "harmony_compare_comb_cqt_convolver.wav"
</CsOptions>
<CsInstruments>

sr     = 48000
ksmps  = 32
nchnls = 1
0dbfs  = 1

#include "harmony_convolver_scaled.inc"

; Generic constant-Q filterbank: geometrically spaced centers, bw = f / Q.
; Log tiling every 3 semitones (4/octave) — diminished-like, not a named chord.
; Recursive a-rate chain (an i-time while that sums `reson` outputs stays silent).
opcode generic_cqt_recur, a, aiiii
  a_in, i_f, i_left, i_q, i_ratio xin
  if (i_left < 1) then
    a_out = 0
  else
    a_band reson a_in, i_f, max(i_f / i_q, 1.0), 1
    a_rest generic_cqt_recur a_in, i_f * i_ratio, i_left - 1, i_q, i_ratio
    a_out = a_band + a_rest
  endif
  xout a_out
endop

opcode generic_cqt_bank, a, aii
  a_in, i_q, i_bands_per_octave xin
  i_f_lo = 55.0
  i_f_hi = sr * 0.45
  i_ratio = 2.0 ^ (1.0 / i_bands_per_octave)
  i_n = int(log(i_f_hi / i_f_lo) / log(i_ratio) + 0.5)
  if (i_n > 96) then
    i_n = 96
  endif
  a_sum generic_cqt_recur a_in, i_f_lo, i_n, i_q, i_ratio
  a_out = a_sum * (2.5 / sqrt(max(i_n, 1)))
  xout a_out
endop

; p4: 0=click, 1=noise
; p5: 0=dry, 1=comb, 2=cqt, 3=convolver
instr compare
  i_src = p4
  i_fx  = p5

  if (i_src == 0) then
    ; Single click at note onset (Dirac-like).
    a_src mpulse 0.9, p3 + 10
    S_src = "click"
  else
    a_src rand 0.25
    ; Soft edges so segment boundaries are visible in a spectrogram.
    a_env linseg 0, 0.02, 1, p3 - 0.04, 1, 0.02, 0
    a_src = a_src * a_env
    S_src = "noise"
  endif

  if (i_fx == 0) then
    a_out = a_src
    S_fx = "dry"
  elseif (i_fx == 1) then
    ; Linear-frequency comb: delay = 1/f0.
    i_f0 = cpsmidinn(36)
    a_out comb a_src, 1.5, 1.0 / i_f0
    a_out = 0.35 * a_out
    S_fx = "comb"
  elseif (i_fx == 2) then
    ; 4 bands/octave (3 semitones), Q~6 ≈ bin spacing — diminished-like CQ tiling.
    a_out generic_cqt_bank a_src, 6, 4
    a_out = 1.0 * a_out
    S_fx = "cqt-bank"
  else
    ; Sparse chord lattice (C E G B), causal scaled kernels.
    a_out harmony_convolver_scaled a_src, 0.05, 0.35, 0.55, 0, 4, 7, 11, -1
    S_fx = "harmony-convolver"
  endif

  prints "compare: t=%.3f  %s  %s\\n", p2, S_src, S_fx
  out a_out
endin

</CsInstruments>
<CsScore>
;        start  dur  src fx   (1 s silence before each segment)
; --- click ---
i "compare"   1  2  0  0
i "compare"   4  2  0  1
i "compare"   7  2  0  2
i "compare"  10  2  0  3
; --- white noise ---
i "compare"  13  4  1  0
i "compare"  18  4  1  1
i "compare"  23  4  1  2
i "compare"  28  4  1  3
e
</CsScore>
</CsoundSynthesizer>
