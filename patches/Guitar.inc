gk_Guitar_midi_dynamic_range chnexport "gk_Guitar_midi_dynamic_range", 3 ; 127
gk_Guitar_midi_dynamic_range init 127
gk_Guitar_level chnexport "gk_Guitar_level", 3
gk_Guitar_level init 0

instr Guitar
; Michael Gogins
; Considerably cleaned up upon closer listening and testing.
i_instrument = p1
i_time = p2
if p3 == -1 then
i_duration = 20
else
i_duration = p3
endif
i_midi_key = p4
i_midi_dynamic_range = i(gk_Guitar_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 74
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Guitar_level)
i_frequency2 = i_frequency / 2.0
i_attack = 0.003
i_release = 20.
a_signal pluck 1.0, i_frequency, i_frequency, 0, 1
;;;a_signal dcblock2 a_signal
a_f1 reson a_signal, 110, 80
a_f2 reson a_signal, 220, 100
a_f3 reson a_signal, 440, 80
a_signal = (.6 * a_f1 + .8 * a_f2 + .6 * a_f3 + .4 * a_signal) * .001
; As with most instruments that are based upon an impulse delivered to a 
; resonator, there are two envelopes, one for the physical decay with a 
; fixed release ending at zero, and one with a release segment to remove 
; clicks from the attack and release.
a_envelope transeg 0, i_attack, 3, 1, i_duration, -6, 0
a_declicking cossegr 0, .005, 1, i_duration, 1, .005, 0
a_signal = a_signal * i_amplitude * a_envelope * a_declicking * k_gain

#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
