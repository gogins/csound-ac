/*
  Compare three treatments of the same sources (click, white noise):

    1) classic comb filter  — teeth periodic in *linear* Hz
    2) generic constant-Q bank — every 3 semitones (noise: high-Q; click: RT60=comb)

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
  Comb f0 = 200 Hz (linear teeth). CQT is every 3 semitones (~50 Hz at C4).
  A 50 Hz comb matches CQT at C4 in Hz but is below full-band spectrogram
  resolution (~3 px/tooth) and looks solid — 200 Hz keeps the linear grid readable.
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

; Generic constant-Q / constant-RT60 filterbank: geometrically spaced centers.
; i_q > 0: bw = f/Q (constant-Q). i_q < 0: bw from RT60=|i_q| (comb-like decay).
; Recursive a-rate chain (an i-time while that sums `reson` outputs stays silent).
opcode generic_cqt_recur, a, aiiii
  a_in, i_f, i_left, i_q, i_ratio xin
  if (i_left < 1) then
    a_out = 0
  else
    if (i_q > 0) then
      i_bw = max(i_f / i_q, 1.0)
    else
      ; Amplitude ~ e^(-π bw t); T60 = ln(1000)/(π bw).
      i_bw = max(6.907755 / (3.14159265 * (-i_q)), 0.5)
    endif
    a_band reson a_in, i_f, i_bw, 1
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
  a_out = a_sum * (5.5 / sqrt(max(i_n, 1)))
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

  ; Shared decay target for Dirac-like responses (matches comb krvt).
  i_click_tail = 1.5

  if (i_fx == 0) then
    a_out = a_src
    S_fx = "dry"
  elseif (i_fx == 1) then
    ; Linear-Hz comb; f0=200 Hz so teeth resolve on full-band spectrograms.
    i_f0 = 200.0
    a_out comb a_src, i_click_tail, 1.0 / i_f0
    a_out = 0.35 * a_out
    S_fx = "comb"
  elseif (i_fx == 2) then
    ; 4/octave (3 st). Noise: high-Q CQ. Click: constant RT60 = comb tail.
    if (i_src == 0) then
      a_out generic_cqt_bank a_src, -i_click_tail, 4
      a_out = a_out * 18.0
    else
      a_out generic_cqt_bank a_src, 32, 4
    endif
    S_fx = "cqt-bank"
  else
    ; Sparse chord lattice (C E G B). Long IR on click to match comb tail; short on noise.
    if (i_src == 0) then
      a_out harmony_convolver_scaled a_src, i_click_tail, 0.22, 0.65, 0, 4, 7, 11, -1
    else
      a_out harmony_convolver_scaled a_src, 0.05, 0.22, 0.65, 0, 4, 7, 11, -1
    endif
    S_fx = "harmony-convolver"
  endif

  prints "compare: t=%.3f  %s  %s\\n", p2, S_src, S_fx

  ; Match subjective (A-weighted) loudness of the 6 treated examples.
  ; Extra makeup on click comb/conv; peaks hard-limited (high crest factor).
  i_loud = 1.0
  if (i_fx > 0) then
    if (i_src == 0) then
      if (i_fx == 1) then
        i_loud = 3.34
      elseif (i_fx == 2) then
        i_loud = 4.677
      else
        i_loud = 7.437
      endif
    else
      if (i_fx == 1) then
        i_loud = 0.0595
      elseif (i_fx == 2) then
        i_loud = 0.196
      else
        i_loud = 0.1595
      endif
    endif
  endif
  a_out = a_out * i_loud
  if (i_fx > 0) then
    a_out limit a_out, -0.95, 0.95
  endif
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
