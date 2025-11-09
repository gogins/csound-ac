<CsoundSynthesizer>
<CsOptions>
-odac    ; or -o out.wav
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 32
nchnls  = 2
0dbfs   = 1

; ============================================================
; PVS hybrid chord imposition:
; - PVS fractional-bin masking with Gaussian lobes at octaves
; - + low-level additive template (time-domain) following the
;   input's envelope
; ============================================================

instr pvs_hybrid_demo
  ; -------- settings you can tweak --------
  S_file              = "input.wav"      ; your 481 s walk file
  i_fft_size          = 4096             ; 8192 for even sharper bins
  i_hop               = 512              ; i_fft_size/8
  i_win               = 1                ; Hann
  k_sigma_bins        init 1.25          ; Gaussian width (bins)
  k_mask_boost_db     init 12            ; how much to boost chord bins
  k_baseline_db       init -6            ; floor for all other bins
  k_template_db       init -15           ; level of additive template
  k_octave_tilt_db    init -4            ; roll off each higher octave
  k_env_smooth_hz     init 4             ; smoothing for input envelope
  k_out_mix           init 0.75          ; final wet/dry mix (0..1)
  ; chord: MIDI notes (use -1 to disable)
  i_note_1 = 60      ; C4
  i_note_2 = 64      ; E4
  i_note_3 = 67      ; G4
  i_note_4 = -1
  i_note_5 = -1

  ; -------- stream the input --------
  a_l, a_r   soundin S_file
  a_in       = 0.5 * (a_l + a_r)

  ; -------- analyze --------
  f_sig      pvsanal a_in, i_fft_size, i_hop, i_fft_size, i_win

  ; -------- build a fractional-bin Gaussian mask (i-time) --------
  i_bins         = i_fft_size/2 + 1
  i_mask_table   ftgenonce 0, 0, i_bins, 7, 0   ; zeros

  ; collect chosen pitch classes
  i_pc_list[] init 5
  i_pc_list[0] = (i_note_1 >= 0 ? i_note_1 % 12 : -1)
  i_pc_list[1] = (i_note_2 >= 0 ? i_note_2 % 12 : -1)
  i_pc_list[2] = (i_note_3 >= 0 ? i_note_3 % 12 : -1)
  i_pc_list[3] = (i_note_4 >= 0 ? i_note_4 % 12 : -1)
  i_pc_list[4] = (i_note_5 >= 0 ? i_note_5 % 12 : -1)

  ; precompute linear gains
  i_boost_lin    = ampdb(k_mask_boost_db)
  i_baseline_lin = ampdb(k_baseline_db)

  ; fill baseline
  i_k = 0
  while (i_k < i_bins) do
    tableiw i_baseline_lin, i_k, i_mask_table
    i_k = i_k + 1
  od

  ; paint Gaussian lobes at all octaves of selected PCs
  i_two_pi = 6.283185307179586
  i_sigma  = k_sigma_bins
  i_nyq_hz = sr * 0.5
  i_midi   = 0
  while (i_midi <= 127) do
    i_pc   = i_midi % 12
    i_keep = 0
    i_pci  = 0
    while (i_pci < 5) do
      if (i_pc_list[i_pci] >= 0 && i_pc_list[i_pci] == i_pc) then
        i_keep = 1
      endif
      i_pci = i_pci + 1
    od

    if (i_keep == 1) then
      i_f    = cpsmidinn(i_midi)
      if (i_f > 0 && i_f < i_nyq_hz) then
        ; bin index center (fractional)
        i_bstar = (i_f * i_fft_size) / sr
        ; octave tilt (down each 1200 cents)
        i_oct   = int((i_midi - (i_pc)) / 12)
        i_oct_gain = ampdb(k_octave_tilt_db * i_oct)
        ; paint +/- 4 sigma (~negligible beyond)
        i_k0   = int(max(0, floor(i_bstar - 4*i_sigma)))
        i_k1   = int(min(i_bins-1, ceil(i_bstar + 4*i_sigma)))
        i_k    = i_k0
        while (i_k <= i_k1) do
          i_w = exp(-0.5 * ((i_k - i_bstar) / i_sigma)^2)
          i_val = table(i_k, i_mask_table)
          ; keep the stronger of existing vs new peak
          i_new = max(i_val, i_baseline_lin + (i_boost_lin - i_baseline_lin) * i_w * i_oct_gain)
          tableiw i_new, i_k, i_mask_table
          i_k = i_k + 1
        od
      endif
    endif
    i_midi = i_midi + 1
  od

  ; -------- apply mask --------
  f_filtered    pvsfilter f_sig, i_mask_table
  a_pvs         pvsynth f_filtered

  ; -------- additive template (time-domain, low level) --------
  ; input envelope to drive template loudness
  k_env_rms     rms a_in
  k_env_smooth  tonek k_env_rms, k_env_smooth_hz
  k_tmpl_gain   = ampdb(k_template_db) * k_env_smooth

  a_template    init 0
  ; synth all octaves of the PCs (same scan as mask but time-domain)
  i_midi2 = 0
  while (i_midi2 <= 127) do
    i_pc2   = i_midi2 % 12
    i_keep2 = 0
    i_pci2  = 0
    while (i_pci2 < 5) do
      if (i_pc_list[i_pci2] >= 0 && i_pc_list[i_pci2] == i_pc2) then
        i_keep2 = 1
      endif
      i_pci2 = i_pci2 + 1
    od
    if (i_keep2 == 1) then
      i_f2 = cpsmidinn(i_midi2)
      if (i_f2 > 0 && i_f2 < i_nyq_hz) then
        i_oct2      = int((i_midi2 - (i_pc2)) / 12)
        i_oct_gain2 = ampdb(k_octave_tilt_db * i_oct2)
        a_template += poscil(k_tmpl_gain * i_oct_gain2, i_f2)
      endif
    endif
    i_midi2 = i_midi2 + 1
  od

  ; gentle lowpass on template to tame hiss if many highs are present
  a_template butlp a_template, 0.45*sr

  ; -------- mix and present --------
  ; match pvs branch level to input so it does not run away
  a_pvs_bal    balance a_pvs, a_in

  a_wet        = a_pvs_bal + a_template
  a_mix        = (1 - k_out_mix) * a_in + k_out_mix * a_wet

  ; de-click attack and DC protection
  a_env        linseg 0, 0.01, 1
  a_out        dcblock2(a_mix * a_env)

  outs a_out, a_out
endin

</CsInstruments>
<CsScore>
i "pvs_hybrid_demo" 0 30
</CsScore>
</CsoundSynthesizer>
