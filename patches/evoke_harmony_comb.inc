opcode evoke_harmony_comb, a, akkkjjjjj
  a_input, k_wet, k_cutoff, k_t60, i_note_1, i_note_2, i_note_3, i_note_4, i_note_5 xin

  ; guards
  k_wet    = max(0.0, min(k_wet, 1.0))
  k_cutoff = max(20.0,  min(k_cutoff, sr * 0.45))
  k_t60    = max(0.0,  k_t60)

  ; map k_t60 (seconds) to a coloration amount in [0..~0.95] (monotonic, safe)
  k_color = (k_t60 > 0 ? k_t60 / (k_t60 + 0.5) : 0)
  k_color = min(k_color, 0.95)

  a_sum   init 0
  i_count init 0
  i_max_delay_ms = 1000                          ; enough for low notes

  ; note 1
  if (i_note_1 >= 0) then
    i_freq_1      = cpsmidinn(i_note_1)
    k_delay_ms_1  = 1000.0 / i_freq_1
    a_del_1       vdelay a_input, k_delay_ms_1, i_max_delay_ms
    a_comb_1      = a_input + (k_color * a_del_1)
    a_damped_1    butlp a_comb_1, k_cutoff
    a_sum         = a_sum + a_damped_1
    i_count       = i_count + 1
  endif

  ; note 2
  if (i_note_2 >= 0) then
    i_freq_2      = cpsmidinn(i_note_2)
    k_delay_ms_2  = 1000.0 / i_freq_2
    a_del_2       vdelay a_input, k_delay_ms_2, i_max_delay_ms
    a_comb_2      = a_input + (k_color * a_del_2)
    a_damped_2    butlp a_comb_2, k_cutoff
    a_sum         = a_sum + a_damped_2
    i_count       = i_count + 1
  endif

  ; note 3
  if (i_note_3 >= 0) then
    i_freq_3      = cpsmidinn(i_note_3)
    k_delay_ms_3  = 1000.0 / i_freq_3
    a_del_3       vdelay a_input, k_delay_ms_3, i_max_delay_ms
    a_comb_3      = a_input + (k_color * a_del_3)
    a_damped_3    butlp a_comb_3, k_cutoff
    a_sum         = a_sum + a_damped_3
    i_count       = i_count + 1
  endif

  ; note 4
  if (i_note_4 >= 0) then
    i_freq_4      = cpsmidinn(i_note_4)
    k_delay_ms_4  = 1000.0 / i_freq_4
    a_del_4       vdelay a_input, k_delay_ms_4, i_max_delay_ms
    a_comb_4      = a_input + (k_color * a_del_4)
    a_damped_4    butlp a_comb_4, k_cutoff
    a_sum         = a_sum + a_damped_4
    i_count       = i_count + 1
  endif

  ; note 5
  if (i_note_5 >= 0) then
    i_freq_5      = cpsmidinn(i_note_5)
    k_delay_ms_5  = 1000.0 / i_freq_5
    a_del_5       vdelay a_input, k_delay_ms_5, i_max_delay_ms
    a_comb_5      = a_input + (k_color * a_del_5)
    a_damped_5    butlp a_comb_5, k_cutoff
    a_sum         = a_sum + a_damped_5
    i_count       = i_count + 1
  endif

  ; wet average
  a_wet init 0
  if (i_count > 0) then
    a_wet = a_sum / i_count
  else
    a_wet = 0 * a_input
  endif

  ; mix
  k_dry    = 1.0 - k_wet
  a_mixed  = (a_wet * k_wet) + (a_input * k_dry)

  ; dc removal, level match, fade-in
  a_mixed     dcblock2 a_mixed
  a_balanced  balance a_mixed, a_input
  a_attack    linseg 0, 0.01, 1
  a_output    = a_balanced * a_attack

  xout a_output
endop
