<CsoundSynthesizer>
<CsOptions>
-RWdfo "evoke_harmony_pvs.wav"
</CsOptions>
<CsInstruments>

sr     = 48000
ksmps  = 32
nchnls = 2
0dbfs  = 1

instr evoke_harmony_pvs
  ; ===== user controls =====
  S_file              = "/Users/michaelgogins/Dropbox/imparting_harmonies/source_soundfiles/TASCAM_0101.normalized.wav"
  i_n                 = 4096
  i_hop               = 512
  i_win               = 1                 ; Hann
  k_wet               init 0.75           ; 0..1 wet/dry for PVS-shaped branch
  k_mask_strength_db  init 12             ; dB boost for MASK template before analysis
  k_add_db            init -12            ; dB level of additive (noise-shaped) chord energy
  i_oct_tilt_db       init -4             ; dB per octave up (negative to roll off highs)
  k_depth             init 1.0            ; 0..1 depth for pvsfilter
  i_gain              init 1.0            ; post-gain in pvsfilter
  i_q                 init 30             ; Q of bandpass filters for noise additive (20..50 good)

  ; chord notes (MIDI), pass -1 to disable
  i_note_1 = 60
  i_note_2 = 64
  i_note_3 = 67
  i_note_4 = -1
  i_note_5 = -1

  ; ===== read source =====
  a_left, a_right  soundin S_file
  a_in             = 0.5 * (a_left + a_right)

  ; input envelope & gate (avoid residual tones in silences)
  k_in_rms     rms a_in
  k_in_env     tonek k_in_rms, 3
  k_gate_lo    init 0.0008
  k_gate_hi    init 0.0030
  if (k_in_env <= k_gate_lo) then
    k_gate = 0
  elseif (k_in_env >= k_gate_hi) then
    k_gate = 1
  else
    k_gate = (k_in_env - k_gate_lo) / (k_gate_hi - k_gate_lo)
  endif
  k_gate_sm  tonek k_gate, 3

  ; ===== collect chord octave freqs (i-time) =====
  i_freqs[]  init 256
  i_gains[]  init 256
  i_count    init 0
  i_nyq      = sr * 0.5

  i_pc1 = (i_note_1 >= 0 ? i_note_1 % 12 : -1)
  i_pc2 = (i_note_2 >= 0 ? i_note_2 % 12 : -1)
  i_pc3 = (i_note_3 >= 0 ? i_note_3 % 12 : -1)
  i_pc4 = (i_note_4 >= 0 ? i_note_4 % 12 : -1)
  i_pc5 = (i_note_5 >= 0 ? i_note_5 % 12 : -1)

  i_m = 0
  while (i_m <= 127) do
    i_pc = i_m % 12
    if (i_pc == i_pc1 || i_pc == i_pc2 || i_pc == i_pc3 || i_pc == i_pc4 || i_pc == i_pc5) then
      i_f = cpsmidinn(i_m)
      if (i_f > 0 && i_f < i_nyq) then
        i_oct             = int((i_m - i_pc)/12)
        i_freqs[i_count]  = i_f
        i_gains[i_count]  = ampdb(i_oct_tilt_db * i_oct)
        i_count           = i_count + 1
      endif
    endif
    i_m = i_m + 1
  od

  ; ===== MASK TEMPLATE (for PVS shaping) =====
  ; Build a broad-band but chord-shaped template so the mask fsig is strong and non-whiny.
  ; We use band-limited noise passed through gentle resonators at the chord octaves.
  a_mask_tmpl init 0
  k_idxm init 0
mask_loop:
  if (k_idxm < i_count) then
    a_n       rand  0.5                              ; white-ish noise per tap
    k_bw_hz   = i_freqs[k_idxm] / i_q                ; bandwidth from Q
    a_bpf     butbp a_n, i_freqs[k_idxm], k_bw_hz
    a_mask_tmpl = a_mask_tmpl + (a_bpf * i_gains[k_idxm])
    k_idxm    = k_idxm + 1
    kgoto mask_loop
  endif
  ; normalize mask template RMS, then scale by mask strength and gate by input activity
  k_mrms   rms a_mask_tmpl
  k_mrms_s tonek k_mrms, 5
  k_mref   init 0.35
  k_meps   init 1e-9
  k_mnorm  = (k_mrms_s > k_meps ? k_mref / k_mrms_s : 1)
  a_mask_n = a_mask_tmpl * k_mnorm
  k_mask_lin   = ampdb(k_mask_strength_db)
  a_mask_final = a_mask_n * k_mask_lin * k_gate_sm

  ; analyze source and mask (IDENTICAL params)
  f_src   pvsanal a_in,         i_n, i_hop, i_n, i_win
  f_mask  pvsanal a_mask_final, i_n, i_hop, i_n, i_win

  ; PVS shaping (depth scaled by gate)
  f_flt   pvsfilter f_src, f_mask, (k_depth * k_gate_sm), i_gain
  a_wet_pvs  pvsynth f_flt
  a_wet_bal  balance a_wet_pvs, a_in

  ; ===== ADDITIVE (NOISE-SHAPED) CHORD ENERGY — NO SINES =====
  a_add init 0
  k_idxa init 0
add_loop:
  if (k_idxa < i_count) then
    a_n2      rand  0.5
    k_bw2_hz  = i_freqs[k_idxa] / i_q
    a_bpf2    butbp a_n2, i_freqs[k_idxa], k_bw2_hz
    a_add     = a_add + (a_bpf2 * i_gains[k_idxa])
    k_idxa    = k_idxa + 1
    kgoto add_loop
  endif
  ; normalize additive RMS and scale by k_add_db, also gate by input activity
  k_arms   rms a_add
  k_arms_s tonek k_arms, 5
  k_aref   init 0.25
  k_anorm  = (k_arms_s > 1e-9 ? k_aref / k_arms_s : 1)
  a_add_n  = a_add * k_anorm
  k_add_lin = ampdb(k_add_db)
  a_add_g   = a_add_n * k_add_lin * k_gate_sm

  ; ===== final mix =====
  a_mix = (1 - k_wet) * a_in + k_wet * a_wet_bal + a_add_g
  a_mix dcblock2 a_mix
  k_att linseg 0, 0.01, 1
  outs a_mix * k_att, a_mix * k_att
endin

</CsInstruments>
<CsScore>
i "evoke_harmony_pvs" 0 481
</CsScore>
</CsoundSynthesizer>
