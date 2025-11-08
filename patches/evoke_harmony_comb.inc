opcode evoke_harmony_comb, a, akkkjjjjj
  a_input, k_wet, k_cutoff, k_resonance, i_note_1, i_note_2, i_note_3, i_note_4, i_note_5 xin

  a_sum init 0
  i_count init 0

  ; --- note 1 ---
  if (i_note_1 >= 0) then
    i_freq_1  = cpsmidinn(i_note_1)
    i_delay_1 = 1.0 / i_freq_1
    a_comb_1  comb a_input, k_resonance, i_delay_1
    a_damped_1 tone a_comb_1, k_cutoff
    a_sum = a_sum + a_damped_1
    i_count = i_count + 1
  endif

  ; --- note 2 ---
  if (i_note_2 >= 0) then
    i_freq_2  = cpsmidinn(i_note_2)
    i_delay_2 = 1.0 / i_freq_2
    a_comb_2  comb a_input, k_resonance, i_delay_2
    a_damped_2 tone a_comb_2, k_cutoff
    a_sum = a_sum + a_damped_2
    i_count = i_count + 1
  endif

  ; --- note 3 ---
  if (i_note_3 >= 0) then
    i_freq_3  = cpsmidinn(i_note_3)
    i_delay_3 = 1.0 / i_freq_3
    a_comb_3  comb a_input, k_resonance, i_delay_3
    a_damped_3 tone a_comb_3, k_cutoff
    a_sum = a_sum + a_damped_3
    i_count = i_count + 1
  endif

  ; --- note 4 ---
  if (i_note_4 >= 0) then
    i_freq_4  = cpsmidinn(i_note_4)
    i_delay_4 = 1.0 / i_freq_4
    a_comb_4  comb a_input, k_resonance, i_delay_4
    a_damped_4 tone a_comb_4, k_cutoff
    a_sum = a_sum + a_damped_4
    i_count = i_count + 1
  endif

  ; --- note 5 ---
  if (i_note_5 >= 0) then
    i_freq_5  = cpsmidinn(i_note_5)
    i_delay_5 = 1.0 / i_freq_5
    a_comb_5  comb a_input, k_resonance, i_delay_5
    a_damped_5 tone a_comb_5, k_cutoff
    a_sum = a_sum + a_damped_5
    i_count = i_count + 1
  endif

  ; --- wet mix (safe) ---
  a_wet init 0
  if (i_count > 0) then
    a_wet = a_sum / i_count
  else
    a_wet = 0 * a_input
  endif

  k_dry    = 1.0 - k_wet
  a_output = (a_wet * k_wet) + (a_input * k_dry)

  ; ==========================================================
  ; k-rate RMS normalization (smooth gain to avoid pumping)
  ; ==========================================================
  k_rms          rms a_output                ; k-rate RMS of output
  k_rms_smooth   tonek k_rms, 5.0            ; smooth amplitude estimate (~5 Hz)
  k_target_rms   = 0.25                      ; desired RMS reference (0..1 of 0dbfs)
  k_eps          = 1.0e-9
  k_gain_raw     = (k_rms_smooth > k_eps ? k_target_rms / k_rms_smooth : 1.0)
  k_gain_smooth  tonek k_gain_raw, 3.0       ; smooth the gain itself

  a_output = a_output * k_gain_smooth

  xout a_output
endop
