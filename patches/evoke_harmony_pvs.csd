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
  S_file             = "/Users/michaelgogins/Dropbox/imparting_harmonies/source_soundfiles/TASCAM_0101.normalized.wav"
  i_n                = 4096
  i_hop              = 512
  i_win              = 1                 ; Hann
  k_wet              init 0.75           ; 0..1 final wet/dry
  k_mask_depth       init 1.0            ; 0..1 depth inside pvsfilter
  i_mask_gain        init 1.0            ; post-gain inside pvsfilter (usually 1)
  k_add_db           init -12            ; dB of chord spectrum to ADD (make less negative / positive to push harder)
  i_oct_tilt_db      init -4             ; dB per octave up for template partials

  ; chord notes (MIDI), pass -1 to disable
  i_note_1 = 60
  i_note_2 = 64
  i_note_3 = 67
  i_note_4 = -1
  i_note_5 = -1

  ; ===== read source =====
  a_left, a_right  soundin S_file
  a_in             = 0.5 * (a_left + a_right)

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

  ; ===== build template audio at a-rate (steady chord spectrum) =====
  a_tmpl    init 0
  i_baseamp = 0.25                                ; pre-normalization partial level
  k_idx init 0
loop_partials:
  if (k_idx < i_count) then
    a_tmpl += poscil(i_baseamp * i_gains[k_idx], i_freqs[k_idx], 1)
    k_idx = k_idx + 1
    kgoto loop_partials
  endif
  a_tmpl butlp a_tmpl, 0.45*sr

  ; normalize template RMS, then scale for additive strength (k_add_db)
  k_trms     rms a_tmpl
  k_trms_s   tonek k_trms, 5
  k_target   init 0.35
  k_eps      init 1e-9
  k_norm     = (k_trms_s > k_eps ? k_target / k_trms_s : 1)
  a_tmpl_n   = a_tmpl * k_norm
  k_add_lin  = ampdb(k_add_db)
  a_tmpl_add = a_tmpl_n * k_add_lin

  ; ===== analysis (identical params) =====
  f_src    pvsanal a_in,       i_n, i_hop, i_n, i_win
  f_mask   pvsanal a_tmpl_n,   i_n, i_hop, i_n, i_win    ; for shaping
  f_add    pvsanal a_tmpl_add, i_n, i_hop, i_n, i_win    ; for injection

  ; ===== spectral shaping (mask) + spectral addition (inject energy) =====
  f_shaped pvsfilter f_src, f_mask, k_mask_depth, i_mask_gain   ; shape/tilt toward chord
  f_sum    pvsadd   f_shaped, f_add                              ; inject chord energy

  ; ===== resynth & mix =====
  a_wet_pvs  pvsynth f_sum
  a_wet_bal  balance a_wet_pvs, a_in
  a_mix      = (1 - k_wet) * a_in + k_wet * a_wet_bal
  a_mix      dcblock2 a_mix
  k_att      linseg 0, 0.01, 1
  outs a_mix * k_att, a_mix * k_att
endin

</CsInstruments>
<CsScore>
; sine table for poscil (required by poscil)
f 1 0 16384 10 1

i "evoke_harmony_pvs" 0 481
</CsScore>
</CsoundSynthesizer>
