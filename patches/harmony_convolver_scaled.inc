/**
# harmony_convolver_scaled

Like the original `harmony_convolver`, with two per-partial changes:

1. Window length (highs not over-long):
```
T(f) = min(T, n_max / f),   n_max = T * 440
```
Each partial uses a **causal** half-cosine fade-out on \([0,T(f)]\)
(starts at 1, ends at 0). A full Hann would peak mid-window and delay the
audible/visible onset of the grain.

2. Inverse IEC A-weighting on each partial independently (gain 1 at 1 kHz),
   with a rumble shelf and boost cap so wind does not dominate field recordings:
```
f'   = max(f, 150 Hz)              ; shelf: no extra rise below 150 Hz
w(f) = min( R_A(1000)/R_A(f'),  10^(6/20) )   ; cap boost at +6 dB
```

**Mix path matches the original:** grain × `i_impulse_gain`, Dirac at sample 0,
joint L2 of the whole IR, one `ftconv`.

## Syntax

```
a_output harmony_convolver_scaled a_input, i_impulse_duration, i_impulse_gain,
  i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3,
  i_pitch_class_4, i_pitch_class_5
```

*/
opcode harmony_convolver_scaled, a, aiiijjjjj
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5 xin

  i_T = max(0.01, min(abs(i_impulse_duration), 1.0))
  i_f_pivot = 440.0
  i_n_max = i_T * i_f_pivot

  i_pc1 = i_pitch_class_1
  i_pc2 = i_pitch_class_2
  i_pc3 = i_pitch_class_3
  i_pc4 = i_pitch_class_4
  i_pc5 = i_pitch_class_5

  i_nyquist_frequency = sr * 0.5 * 0.95
  i_frequency_list[] init 128
  i_loud_weight[] init 128
  i_frequency_count init 0

  ; IEC A-weighting R_A(f); weight = R_A(1000)/R_A(f'), capped.
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

  i_midi_note = 0
  while (i_midi_note <= 127) do
    i_pitch_class = i_midi_note % 12
    i_is_selected = 0
    if (i_pc1 >= 0 && i_pitch_class == i_pc1) then
      i_is_selected = 1
    endif
    if (i_pc2 >= 0 && i_pitch_class == i_pc2) then
      i_is_selected = 1
    endif
    if (i_pc3 >= 0 && i_pitch_class == i_pc3) then
      i_is_selected = 1
    endif
    if (i_pc4 >= 0 && i_pitch_class == i_pc4) then
      i_is_selected = 1
    endif
    if (i_pc5 >= 0 && i_pitch_class == i_pc5) then
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
        i_frequency_list[i_frequency_count] = i_frequency
        i_loud_weight[i_frequency_count] = i_w
        i_frequency_count = i_frequency_count + 1
      endif
    endif
    i_midi_note = i_midi_note + 1
  od

  i_sample_count = int(i_T * sr + 0.5)
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
        i_T_f = i_n_max / i_frequency
        if (i_T_f > i_T) then
          i_T_f = i_T
        endif
        if (i_time_seconds < i_T_f) then
          if (i_T_f > 0) then
            ; Causal fade-out (half-cosine): 1 at t=0, 0 at t=T_f.
            ; A full Hann peaks at mid-window and makes the effect look late.
            i_hann = 0.5 + 0.5 * cos(i_two_pi * 0.5 * i_time_seconds / i_T_f)
          else
            i_hann = 0
          endif
          ; Per-partial inverse A-weight; then average across partials.
          i_sum_of_sines = i_sum_of_sines + i_loud_weight[i_frequency_index] * i_hann * sin(i_two_pi * i_frequency * i_time_seconds)
        endif
        i_frequency_index = i_frequency_index + 1
      od
      i_sample_value = i_sum_of_sines / i_frequency_count
      tableiw i_sample_value * i_impulse_gain, i_sample_index, i_impulse_response_table
      i_sample_index = i_sample_index + 1
    od
  endif

  ; Dirac in the IR, then joint L2 — same as the audible original.
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

  a_convolved ftconv a_source, i_impulse_response_table, 128
  xout a_convolved
endop
