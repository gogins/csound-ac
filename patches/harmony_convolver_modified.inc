/**
# harmony_convolver (modified drop-in)

Same audible mix path as the git original: harmonic grain * impulse_gain,
Dirac at sample 0, joint L2, single ftconv. Differences from git original:

- `i_impulse_duration` is i-rate (`aiiijjjjj`) for Csound 7 init safety
- unique `ftgen` per instance (avoids `ftgenonce` collisions)

*/
opcode harmony_convolver, a, aiiijjjjj
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5 xin

  i_kernel_duration = max(0.01, min(abs(i_impulse_duration), 1.0))

  i_pc1 = i_pitch_class_1
  i_pc2 = i_pitch_class_2
  i_pc3 = i_pitch_class_3
  i_pc4 = i_pitch_class_4
  i_pc5 = i_pitch_class_5

  i_nyquist_frequency = sr * 0.5 * 0.95
  i_frequency_list[] init 128
  i_frequency_count init 0

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
        i_frequency_list[i_frequency_count] = i_frequency
        i_frequency_count = i_frequency_count + 1
      endif
    endif
    i_midi_note = i_midi_note + 1
  od

  i_sample_count = int(i_kernel_duration * sr + 0.5)
  if (i_sample_count < 2) then
    i_sample_count = 2
  endif

  i_impulse_response_table ftgen 0, 0, -i_sample_count, 7, 0, i_sample_count, 0

  if (i_frequency_count > 0) then
    i_two_pi = 6.283185307179586
    i_sample_index = 0
    while (i_sample_index < i_sample_count) do
      i_time_seconds = i_sample_index / sr
      i_hanning_window = 0.5 - 0.5 * cos((i_two_pi * i_sample_index) / (i_sample_count - 1))
      i_sum_of_sines = 0.0
      i_frequency_index = 0
      while (i_frequency_index < i_frequency_count) do
        i_sum_of_sines = i_sum_of_sines + sin(i_two_pi * i_frequency_list[i_frequency_index] * i_time_seconds)
        i_frequency_index = i_frequency_index + 1
      od
      i_sample_value = (i_sum_of_sines / i_frequency_count) * i_hanning_window
      tableiw i_sample_value * i_impulse_gain, i_sample_index, i_impulse_response_table
      i_sample_index = i_sample_index + 1
    od
  endif

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
