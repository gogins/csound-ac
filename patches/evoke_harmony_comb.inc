opcode evoke_harmony_comb, a, akkkjjjjj
  a_input, k_wet, k_cutoff, k_t60, i_note_1, i_note_2, i_note_3, i_note_4, i_note_5 xin

  ; -------- parameter guards --------
  k_wet    = max(0.0, min(k_wet, 1.0))
  k_cutoff = max(20.0,  min(k_cutoff, sr * 0.45))

  ; we repurpose k_t60 here as a coloration strength; map monotonically into [0..0.95]
  k_color  = (k_t60 > 0 ? k_t60 / (k_t60 + 0.5) : 0)
  k_color  = min(k_color, 0.95)

  ; -------- input-activity gate for the wet path --------
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

  ; -------- tiny delay modulation to de-stationarize comb teeth --------
  k_mod_rate  = 0.2                ; Hz (very slow)
  k_mod_depth = 0.10               ; ms (very shallow)

  a_sum   init 0
  i_count init 0
  i_max_delay_ms = 1000

  ; ----- note 1 -----
  if (i_note_1 >= 0) then
    i_freq_1      = cpsmidinn(i_note_1)
    k_base_ms_1   = 1000.0 / i_freq_1
    k_lfo_1       lfo k_mod_depth, k_mod_rate, 0
    a_del_1       vdelay a_input, (k_base_ms_1 + k_lfo_1), i_max_delay_ms
    a_tap_1       = k_color * a_del_1
    a_lp1_1       butlp a_tap_1, k_cutoff
    a_tap_1       butlp a_lp1_1, k_cutoff
    a_sum         = a_sum + a_tap_1
    i_count       = i_count + 1
  endif

  ; ----- note 2 -----
  if (i_note_2 >= 0) then
    i_freq_2      = cpsmidinn(i_note_2)
    k_base_ms_2   = 1000.0 / i_freq_2
    k_lfo_2       lfo k_mod_depth, k_mod_rate * 1.07, 0
    a_del_2       vdelay a_input, (k_base_ms_2 + k_lfo_2), i_max_delay_ms
    a_tap_2       = k_color * a_del_2
    a_lp1_2       butlp a_tap_2, k_cutoff
    a_tap_2       butlp a_lp1_2, k_cutoff
    a_sum         = a_sum + a_tap_2
    i_count       = i_count + 1
  endif

  ; ----- note 3 -----
  if (i_note_3 >= 0) then
    i_freq_3      = cpsmidinn(i_note_3)
    k_base_ms_3   = 1000.0 / i_freq_3
    k_lfo_3       lfo k_mod_depth, k_mod_rate * 0.93, 0
    a_del_3       vdelay a_input, (k_base_ms_3 + k_lfo_3), i_max_delay_ms
    a_tap_3       = k_color * a_del_3
    a_lp1_3       butlp a_tap_3, k_cutoff
    a_tap_3       butlp a_lp1_3, k_cutoff
    a_sum         = a_sum + a_tap_3
    i_count       = i_count + 1
  endif

  ; ----- note 4 -----
  if (i_note_4 >= 0) then
    i_freq_4      = cpsmidinn(i_note_4)
    k_base_ms_4   = 1000.0 / i_freq_4
    k_lfo_4       lfo k_mod_depth, k_mod_rate * 0.83, 0
    a_del_4       vdelay a_input, (k_base_ms_4 + k_lfo_4), i_max_delay_ms
    a_tap_4       = k_color * a_del_4
    a_lp1_4       butlp a_tap_4, k_cutoff
    a_tap_4       butlp a_lp1_4, k_cutoff
    a_sum         = a_sum + a_tap_4
    i_count       = i_count + 1
  endif

  ; ----- note 5 -----
  if (i_note_5 >= 0) then
    i_freq_5      = cpsmidinn(i_note_5)
    k_base_ms_5   = 1000.0 / i_freq_5
    k_lfo_5       lfo k_mod_depth, k_mod_rate * 1.19, 0
    a_del_5       vdelay a_input, (k_base_ms_5 + k_lfo_5), i_max_delay_ms
    a_tap_5       = k_color * a_del_5
    a_lp1_5       butlp a_tap_5, k_cutoff
    a_tap_5       butlp a_lp1_5, k_cutoff
    a_sum         = a_sum + a_tap_5
    i_count       = i_count + 1
  endif

  ; ----- average taps to keep wet level stable -----
  a_wet init 0
  if (i_count > 0) then
    a_wet = a_sum / i_count
  else
    a_wet = 0 * a_input
  endif

  ; ----- gate the wet when input is very quiet -----
  a_wet = a_wet * k_wet_gate

  ; ----- match wet level to input (prevents level creep) -----
  a_wet = balance(a_wet, a_input)

  ; ----- final wet/dry mix -----
  k_dry   = 1.0 - k_wet
  a_mix   = (a_wet * k_wet) + (a_input * k_dry)

  ; ----- clean up: DC block and tiny fade-in -----
  a_mix   dcblock2 a_mix
  a_env   linseg 0, 0.01, 1
  a_out   = a_mix * a_env

  xout a_out
endop
