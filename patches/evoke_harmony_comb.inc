opcode evoke_harmony_comb, a, akkkjjjjj
  a_input, k_wet, k_cutoff, k_t60, i_note_1, i_note_2, i_note_3, i_note_4, i_note_5 xin

  ; -------- guards --------
  k_wet    = max(0.0, min(k_wet, 1.0))
  k_cutoff = max(20.0,  min(k_cutoff, sr * 0.45))

  ; map k_t60 (your 4th k-param) to a coloration amount in [0..0.98]
  ; stronger than before so combing is clearly audible
  k_color = (k_t60 > 0 ? k_t60 / (k_t60 + 0.25) : 0)
  k_color = min(k_color, 0.98)

  ; -------- input-activity gate for wet path --------
  k_in_rms  rms a_input
  k_in_env  tonek k_in_rms, 5.0
  k_gate_lo = 0.0008
  k_gate_hi = 0.0030
  if (k_in_env <= k_gate_lo) then
    k_gate_raw = 0
  elseif (k_in_env >= k_gate_hi) then
    k_gate_raw = 1
  else
    k_gate_raw = (k_in_env - k_gate_lo) / (k_gate_hi - k_gate_lo)
  endif
  k_wet_gate tonek k_gate_raw, 5.0

  ; -------- tiny delay modulation (kills stationary teeth/whine) --------
  k_mod_rate  = 0.20          ; Hz
  k_mod_depth = 0.10          ; ms

  a_sum   init 0
  i_count init 0
  i_max_delay_ms = 1000

  ; ----- note 1 -----
  if (i_note_1 >= 0) then
    i_freq_1    = cpsmidinn(i_note_1)
    k_base_1    = 1000.0 / i_freq_1
    k_lfo_1     lfo k_mod_depth, k_mod_rate, 0
    a_del_1     vdelay a_input, (k_base_1 + k_lfo_1), i_max_delay_ms
    a_tap_1     = k_color * a_del_1
    a_lp1_1     butlp a_tap_1, k_cutoff
    a_tap_1     butlp a_lp1_1, k_cutoff
    a_sum       = a_sum + a_tap_1
    i_count     = i_count + 1
  endif

  ; ----- note 2 -----
  if (i_note_2 >= 0) then
    i_freq_2    = cpsmidinn(i_note_2)
    k_base_2    = 1000.0 / i_freq_2
    k_lfo_2     lfo k_mod_depth, k_mod_rate * 1.07, 0
    a_del_2     vdelay a_input, (k_base_2 + k_lfo_2), i_max_delay_ms
    a_tap_2     = k_color * a_del_2
    a_lp1_2     butlp a_tap_2, k_cutoff
    a_tap_2     butlp a_lp1_2, k_cutoff
    a_sum       = a_sum + a_tap_2
    i_count     = i_count + 1
  endif

  ; ----- note 3 -----
  if (i_note_3 >= 0) then
    i_freq_3    = cpsmidinn(i_note_3)
    k_base_3    = 1000.0 / i_freq_3
    k_lfo_3     lfo k_mod_depth, k_mod_rate * 0.93, 0
    a_del_3     vdelay a_input, (k_base_3 + k_lfo_3), i_max_delay_ms
    a_tap_3     = k_color * a_del_3
    a_lp1_3     butlp a_tap_3, k_cutoff
    a_tap_3     butlp a_lp1_3, k_cutoff
    a_sum       = a_sum + a_tap_3
    i_count     = i_count + 1
  endif

  ; ----- note 4 -----
  if (i_note_4 >= 0) then
    i_freq_4    = cpsmidinn(i_note_4)
    k_base_4    = 1000.0 / i_freq_4
    k_lfo_4     lfo k_mod_depth, k_mod_rate * 0.83, 0
    a_del_4     vdelay a_input, (k_base_4 + k_lfo_4), i_max_delay_ms
    a_tap_4     = k_color * a_del_4
    a_lp1_4     butlp a_tap_4, k_cutoff
    a_tap_4     butlp a_lp1_4, k_cutoff
    a_sum       = a_sum + a_tap_4
    i_count     = i_count + 1
  endif

  ; ----- note 5 -----
  if (i_note_5 >= 0) then
    i_freq_5    = cpsmidinn(i_note_5)
    k_base_5    = 1000.0 / i_freq_5
    k_lfo_5     lfo k_mod_depth, k_mod_rate * 1.19, 0
    a_del_5     vdelay a_input, (k_base_5 + k_lfo_5), i_max_delay_ms
    a_tap_5     = k_color * a_del_5
    a_lp1_5     butlp a_tap_5, k_cutoff
    a_tap_5     butlp a_lp1_5, k_cutoff
    a_sum       = a_sum + a_tap_5
    i_count     = i_count + 1
  endif

  ; ----- build wet (NO averaging; stronger combing) -----
  a_wet init 0
  if (i_count > 0) then
    a_wet = a_sum
  else
    a_wet = 0 * a_input
  endif

  ; gate the wet when input is very quiet
  a_wet = a_wet * k_wet_gate

  ; level-normalize wet to match input (prevents level creep even without averaging)
  a_wet = balance(a_wet, a_input)

  ; ----- final wet/dry mix -----
  k_dry   = 1.0 - k_wet
  a_mix   = (a_wet * k_wet) + (a_input * k_dry)

  ; cleanup: dc block + short attack
  a_mix   dcblock2 a_mix
  a_env   linseg 0, 0.01, 1
  a_out   = a_mix * a_env

  xout a_out
endop
