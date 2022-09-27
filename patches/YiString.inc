gk_YiString_midi_dynamic_range chnexport "gk_YiString_midi_dynamic_range", 3 ;  127
gk_YiString_level chnexport "gk_YiString_level", 3 ;  0
gk_YiString_reverb_send chnexport "gk_YiString_reverb_send", 3 ;  .5
gk_YiString_chorus_send chnexport "gk_YiString_chorus_send", 3 ;  .5
gi_YiString_overlap chnexport "gi_YiString_overlap", 3 ;  .1

gk_YiString_midi_dynamic_range init 20
gk_YiString_level init 0
gk_YiString_reverb_send init .5
gk_YiString_chorus_send init .5
gi_YiString_overlap init .1

gk_YiString_space_left_to_right chnexport "gk_YiString_space_left_to_right", 3
gk_YiString_space_left_to_right init .5

instr YiString
//////////////////////////////////////////////
// Original by Steven Yi.
// Adapted by Michael Gogins.
//////////////////////////////////////////////
i_instrument = p1
i_time = p2
; Make indefinite notes last no longer than the physical decay.
i_physical_duration = 20000
if p3 == -1 then
i_duration = i_physical_duration
else
i_duration = p3
endif
i_midi_key = p4
i_midi_dynamic_range = i(gk_YiString_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 62.25
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_YiString_level)
iattack = gi_YiString_overlap
isustain = i_duration
idecay = gi_YiString_overlap
xtratim 1
a_physical_envelope transeg 0.0, iattack / 2.0, 1.5, i_amplitude / 2.0, iattack / 2.0, -1.5, i_amplitude, isustain, 0.0, i_amplitude, idecay / 2.0, 1.5, i_amplitude / 2.0, idecay / 2.0, -1.5, 0
;ampenv = madsr:a(1, 0.1, 0.95, 0.5)
a_signal = vco2(1, i_frequency)
a_signal = moogladder(a_signal, 6000, 0.1)

; The de-clicking envelope must have attack and release segments that damp 
; artifacts in the signal. The duration of these segments depends on 
; the behavior of the instrument, and may vary as a function of frequency.
i_declick_attack = iattack
i_declick_release = i_declick_attack * 2
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_duration, 1,  i_declick_release, 0
; The envelope of the instrument is the product of the physical envelope times 
; the declicking envelope. 
a_envelope = a_physical_envelope * a_declicking_envelope
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_envelope, 40, 4
a_signal = a_signal * i_amplitude * a_filtered_envelope * k_gain *.001

#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
a_signal_reverb = a_signal * gk_YiString_reverb_send
a_signal_chorus = a_signal * gk_YiString_chorus_send
a_out_left, a_out_right pan2 a_signal_reverb, p7
outleta "outleft", a_out_left
outleta "outright",  a_out_right
a_out_left, a_out_right pan2 a_signal_chorus, p7
outleta "chorusleft", a_out_left 
outleta "chorusright", a_out_right 
;printks "YiString         %9.4f  %9.4f\n", 0.5, a_out_left, a_out_right
#endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
