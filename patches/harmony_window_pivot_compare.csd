/*
  Compare the two independent axes of the harmony convolver's grain design:

    axis 1 -- grain length law
      cap = 1  "knee":       T(f) = min(T, n_max/f)   constant-Q above 440 Hz,
                                                      constant duration below
      cap = 0  constant Q:   T(f) = n_max/f           same cycle count at every f

    axis 2 -- grain window on [0, T(f)]
      window = 0  causal half-cosine  0.5 + 0.5 cos(pi t / T_f)   starts at 1
      window = 1  full cosine (Hann)  0.5 - 0.5 cos(2 pi t / T_f) peaks at T_f/2

  The four combinations, in score order:

    a  constant Q + full cosine
    b  constant Q + half cosine
    c  knee       + full cosine
    d  knee       + half cosine     <- what harmony_convolver_scaled does

  So that only grain shape and length vary, the grain block is normalized to
  unit energy *before* the Dirac is added. Every treatment therefore has the
  same wet/dry ratio and the same total impulse-response energy, and differs
  only in spectral tilt and time structure.

  Timeline (1 s silence before each segment; use a log-frequency spectrogram):

    t      source       treatment
    1-3    click        dry
    4-6    click        a, b, c, d      (one per segment, 3 s apart)
    ...
    19-23  pulse train  dry, a, b, c, d
    ...
    44-48  noise        dry, a, b, c, d

  Chord: pitch-classes 0, 4, 7, 11 (C E G B), as in the comparison figure.
*/

<CsoundSynthesizer>
<CsOptions>
-m32 -d -RWfo "harmony_window_pivot_compare.wav"
</CsOptions>
<CsInstruments>

sr     = 48000
ksmps  = 32
nchnls = 1
0dbfs  = 1

/*
Equal-energy harmony convolver with the grain length law and the grain window
both selectable. i_cap = 1 caps T(f) at T (the knee), 0 lets it run as pure
constant Q. i_window = 0 is the causal half-cosine, 1 is the full Hann.
*/
opcode harmony_convolver_variant, a, aiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_cap, i_window, i_chord_unused xin

  i_T = max(0.01, min(abs(i_impulse_duration), 1.5))
  i_f_pivot = 440.0
  i_n_max = i_T * i_f_pivot
  i_duration_exponent = 0.5

  i_pc1 = 0
  i_pc2 = 4
  i_pc3 = 7
  i_pc4 = 11

  i_nyquist_frequency = sr * 0.5 * 0.95
  i_frequency_list[] init 128
  i_loud_weight[] init 128
  i_window_length[] init 128
  i_partial_gain[] init 128
  i_frequency_count init 0

  ; IEC A-weighting R_A(f); weight = R_A(1000)/R_A(f'), capped at +6 dB.
  i_fref = 1000.0
  i_f_shelf = 150.0
  i_w_boost_max = 1.9952623149688796
  i_f2r = i_fref * i_fref
  i_f4r = i_f2r * i_f2r
  i_c1 = 12194.0 * 12194.0
  i_c2 = 20.6 * 20.6
  i_c3 = 107.7 * 107.7
  i_c4 = 737.9 * 737.9
  i_ra_1k = (i_c1 * i_f4r) / ((i_f2r + i_c2) * sqrt((i_f2r + i_c3) * (i_f2r + i_c4)) * (i_f2r + i_c1))

  i_max_window_length = i_T

  i_midi_note = 0
  while (i_midi_note <= 127) do
    i_pitch_class = i_midi_note % 12
    i_is_selected = 0
    if (i_pitch_class == i_pc1 || i_pitch_class == i_pc2) then
      i_is_selected = 1
    endif
    if (i_pitch_class == i_pc3 || i_pitch_class == i_pc4) then
      i_is_selected = 1
    endif
    if (i_is_selected == 1) then
      i_frequency = cpsmidinn(i_midi_note)
      if (i_frequency >= 20 && i_frequency <= i_nyquist_frequency) then
        i_f_w = i_frequency
        if (i_f_w < i_f_shelf) then
          i_f_w = i_f_shelf
        endif
        i_f2 = i_f_w * i_f_w
        i_f4 = i_f2 * i_f2
        i_ra = (i_c1 * i_f4) / ((i_f2 + i_c2) * sqrt((i_f2 + i_c3) * (i_f2 + i_c4)) * (i_f2 + i_c1))
        if (i_ra < 1.0e-30) then
          i_ra = 1.0e-30
        endif
        i_w = i_ra_1k / i_ra
        if (i_w > i_w_boost_max) then
          i_w = i_w_boost_max
        endif
        ; Grain length law: capped (knee) or pure constant Q.
        i_T_f = i_n_max / i_frequency
        if (i_cap == 1 && i_T_f > i_T) then
          i_T_f = i_T
        endif
        if (i_T_f > i_max_window_length) then
          i_max_window_length = i_T_f
        endif
        i_frequency_list[i_frequency_count] = i_frequency
        i_loud_weight[i_frequency_count] = i_w
        i_window_length[i_frequency_count] = i_T_f
        i_partial_gain[i_frequency_count] = (i_T / i_T_f) ^ i_duration_exponent
        i_frequency_count = i_frequency_count + 1
      endif
    endif
    i_midi_note = i_midi_note + 1
  od

  i_sample_count = int(i_max_window_length * sr + 0.5)
  if (i_sample_count < 2) then
    i_sample_count = 2
  endif

  i_impulse_response_table ftgen 0, 0, -i_sample_count, 7, 0, i_sample_count, 0

  if (i_frequency_count > 0) then
    i_two_pi = 6.283185307179586
    i_sample_index = 0
    while (i_sample_index < i_sample_count) do
      i_time_seconds = i_sample_index / sr
      i_sum_of_sines = 0.0
      i_frequency_index = 0
      while (i_frequency_index < i_frequency_count) do
        i_frequency = i_frequency_list[i_frequency_index]
        i_T_f = i_window_length[i_frequency_index]
        if (i_time_seconds < i_T_f) then
          if (i_window == 1) then
            ; Full cosine (Hann): 0 at both ends, peaks at T_f/2.
            i_env = 0.5 - 0.5 * cos(i_two_pi * i_time_seconds / i_T_f)
          else
            ; Causal half-cosine: 1 at t=0, 0 at t=T_f.
            i_env = 0.5 + 0.5 * cos(i_two_pi * 0.5 * i_time_seconds / i_T_f)
          endif
          i_amplitude = i_loud_weight[i_frequency_index] * i_partial_gain[i_frequency_index]
          i_sum_of_sines = i_sum_of_sines + i_amplitude * i_env * sin(i_two_pi * i_frequency * i_time_seconds)
        endif
        i_frequency_index = i_frequency_index + 1
      od
      tableiw i_sum_of_sines / i_frequency_count, i_sample_index, i_impulse_response_table
      i_sample_index = i_sample_index + 1
    od
  endif

  ; Normalize the grain block to unit energy *before* the Dirac, so that all
  ; four variants share one wet/dry ratio and only the tilt and time structure
  ; of the grains can differ.
  i_grain_energy = 0.0
  i_idx = 0
  while (i_idx < i_sample_count) do
    i_val = table(i_idx, i_impulse_response_table)
    i_grain_energy = i_grain_energy + (i_val * i_val)
    i_idx = i_idx + 1
  od
  i_grain_scale = i_impulse_gain / sqrt(max(i_grain_energy, 1.0e-12))
  i_idx = 0
  while (i_idx < i_sample_count) do
    tableiw table(i_idx, i_impulse_response_table) * i_grain_scale, i_idx, i_impulse_response_table
    i_idx = i_idx + 1
  od

  tableiw (table(0, i_impulse_response_table) + i_dirac_level), 0, i_impulse_response_table

  i_energy = 0.0
  i_idx = 0
  while (i_idx < i_sample_count) do
    i_val = table(i_idx, i_impulse_response_table)
    i_energy = i_energy + (i_val * i_val)
    i_idx = i_idx + 1
  od
  i_scale = 1.0 / sqrt(max(i_energy, 1.0e-12))
  i_idx = 0
  while (i_idx < i_sample_count) do
    tableiw table(i_idx, i_impulse_response_table) * i_scale, i_idx, i_impulse_response_table
    i_idx = i_idx + 1
  od

  prints "  variant: cap=%d window=%d  partials=%d  IR=%d samples (%.3f s)\\n", i_cap, i_window, i_frequency_count, i_sample_count, i_sample_count / sr

  a_convolved ftconv a_source, i_impulse_response_table, 128
  xout a_convolved
endop

; p4: 0=click, 1=pulse train, 2=noise
; p5: 0=dry, 1=a (constQ+full), 2=b (constQ+half), 3=c (knee+full), 4=d (knee+half)
instr compare
  i_src = p4
  i_fx  = p5

  if (i_src == 0) then
    a_src mpulse 0.9, p3 + 10
    S_src = "click"
  elseif (i_src == 1) then
    ; Repeated clicks: time smear and grain delay read as rhythm.
    a_src mpulse 0.9, 0.3333333
    S_src = "pulses"
  else
    a_src rand 0.25
    a_env linseg 0, 0.02, 1, p3 - 0.04, 1, 0.02, 0
    a_src = a_src * a_env
    S_src = "noise"
  endif

  i_T = 0.05

  if (i_fx == 0) then
    a_out = a_src
    S_fx = "dry"
  else
    if (i_fx == 1) then
      i_cap = 0
      i_window = 1
      S_fx = "a constQ+full"
    elseif (i_fx == 2) then
      i_cap = 0
      i_window = 0
      S_fx = "b constQ+half"
    elseif (i_fx == 3) then
      i_cap = 1
      i_window = 1
      S_fx = "c knee+full"
    else
      i_cap = 1
      i_window = 0
      S_fx = "d knee+half"
    endif
    a_out harmony_convolver_variant a_src, i_T, 0.22, 0.65, i_cap, i_window, 0
  endif

  prints "compare: t=%.3f  %s  %s\\n", p2, S_src, S_fx

  ; One makeup gain per source, shared by all four variants, since the IRs are
  ; energy-matched by construction.
  i_loud = 1.0
  if (i_fx > 0) then
    if (i_src == 2) then
      i_loud = 0.16
    else
      i_loud = 7.0
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
;        start  dur  src fx
; --- single click ---
i "compare"   1  2  0  0
i "compare"   4  2  0  1
i "compare"   7  2  0  2
i "compare"  10  2  0  3
i "compare"  13  2  0  4
; --- pulse train ---
i "compare"  16  3  1  0
i "compare"  20  3  1  1
i "compare"  24  3  1  2
i "compare"  28  3  1  3
i "compare"  32  3  1  4
; --- white noise ---
i "compare"  36  4  2  0
i "compare"  41  4  2  1
i "compare"  46  4  2  2
i "compare"  51  4  2  3
i "compare"  56  4  2  4
e
</CsScore>
</CsoundSynthesizer>
