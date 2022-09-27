gk_FMDroner_partial1 chnexport "gk_FMDroner_partial1", 3 ; .5
gk_FMDroner_partial2 chnexport "gk_FMDroner_partial2", 3 ; .05
gk_FMDroner_partial3 chnexport "gk_FMDroner_partial3", 3 ; .1
gk_FMDroner_partial4 chnexport "gk_FMDroner_partial4", 3 ; .2
gk_FMDroner_partial5 chnexport "gk_FMDroner_partial5", 3 ; .1
gk_FMDroner_partial6 chnexport "gk_FMDroner_partial6", 3 ; 0
gk_FMDroner_partial7 chnexport "gk_FMDroner_partial7", 3 ; 0
gk_FMDroner_partial8 chnexport "gk_FMDroner_partial8", 3 ; 0
gk_FMDroner_partial9 chnexport "gk_FMDroner_partial9", 3 ; 0
gk_FMDroner_partial10 chnexport "gk_FMDroner_partial10", 3 ; 0
gk_FMDroner_index chnexport "gk_FMDroner_index", 3 ; 3
gk_FMDroner_carrier chnexport "gk_FMDroner_carrier", 3 ; 1
gk_FMDroner_modulator chnexport "gk_FMDroner_modulator", 3 ; 3
gk_FMDroner_transeg_exponent chnexport "gk_FMDroner_transeg_exponent", 3 ; -20
gk_FMDroner_x chnexport "gk_FMDroner_x", 3 ; 6
gk_FMDroner_y chnexport "gk_FMDroner_y", 3 ; 0
gk_FMDroner_z chnexport "gk_FMDroner_z", 3 ; 0
gk_FMDroner_level chnexport "gk_FMDroner_level", 3 ; 0

gk_FMDroner_partial1 init .5
gk_FMDroner_partial2 init .05
gk_FMDroner_partial3 init .1
gk_FMDroner_partial4 init .2
gk_FMDroner_partial5 init .1
gk_FMDroner_partial6 init 0
gk_FMDroner_partial7 init 0
gk_FMDroner_partial8 init 0
gk_FMDroner_partial9 init 0
gk_FMDroner_partial10 init 0
gk_FMDroner_index init 3
gk_FMDroner_carrier init 1
gk_FMDroner_modulator init 3
gk_FMDroner_transeg_exponent init -20
gk_FMDroner_x init 6
gk_FMDroner_y init 0
gk_FMDroner_z init 0
gk_FMDroner_level init 0

gk_FMDroner_space_left_to_right chnexport "gk_Guitar_space_left_to_right", 3
gk_FMDroner_space_left_to_right init .5


gi_FMDroner_sine ftgen 0, 0, 65537, 10, 1, 0, .02

instr FMDroner
i_instrument = p1
i_time = p2
; Make indefinite notes last no longer than the physical decay.
i_physical_decay = 20
if p3 == -1 then
i_duration = i_physical_decay
else
i_duration = p3
endif
i_midi_key = p4
i_midi_velocity = p5
k_space_front_to_back = p6
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_FMDroner_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_overall_amps = -20 + 98 
i_normalization = ampdb(-i_overall_amps) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_FMDroner_level)
k1 = gk_FMDroner_partial1
k2 = gk_FMDroner_partial2
k3 = gk_FMDroner_partial3
k4 = gk_FMDroner_partial4
k5 = gk_FMDroner_partial5
k6 = gk_FMDroner_partial6
k7 = gk_FMDroner_partial7
k8 = gk_FMDroner_partial8
k9 = gk_FMDroner_partial9
k10 = gk_FMDroner_partial10
iattack = .5
idecay = .5
isustain = p3
xtratim iattack + idecay
aenvelope transeg 0.0, iattack / 2.0, 1.5, 1 / 2.0, iattack / 2.0, -1.5, 1, isustain, 0.0, 1, idecay / 2.0, 1.5, 1 / 2.0, idecay / 2.0, -1.5, 0
ihertz = cpsmidinn(i_midi_key)
;asignal poscil3 1, ihertz, gi_FMDroner_sine
kndx transeg i(gk_FMDroner_index), i(p3), i(gk_FMDroner_transeg_exponent), 0
asignal foscili .5, ihertz, gk_FMDroner_carrier, gk_FMDroner_modulator, kndx, gi_FMDroner_sine
asignal chebyshevpoly asignal, 0, k1, k2, k3, k4, k5, k6, k7, k8, k9, k10
a_signal = asignal
i_attack = .002
i_sustain = p3
i_release = 0.01
xtratim 1

; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .003
i_attack = .0004 / i_frequency + i_declick_minimum
i_exponent = 9
a_physical_envelope transeg 0,   i_attack, i_exponent,  1,   i_physical_decay, -i_exponent,  0
; The de-clicking envelope must have attack and release segments that damp 
; artifacts in the signal. The duration of these segments depends on 
; the behavior of the instrument, and may vary as a function of frequency.
i_declick_attack = i_attack
i_declick_release = i_declick_minimum * 2
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_duration, 1,  i_declick_release, 0
; The envelope of the instrument is the product of the physical envelope times 
; the declicking envelope. 
a_envelope = a_physical_envelope * a_declicking_envelope
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_envelope, 60, 4
a_signal = a_signal * i_amplitude * a_filtered_envelope * k_gain * .001

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
