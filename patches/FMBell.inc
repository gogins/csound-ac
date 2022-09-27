gk_FMBell_level chnexport "gk_FMBell_level", 3

gk_FMBell_level init 0
gk_FMBell_space_left_to_right chnexport "gk_Guitar_space_left_to_right", 3
gk_FMBell_space_left_to_right init .5

gi_FMBell_cosine ftgen 0, 0, 65537, 11, 1

instr FMBell
; Authors: John ffitch, Michael Gogins
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
if p7 ==0 then
k_space_left_to_right = gk_FMBell_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
i_overall_amps = 90
i_normalization = ampdb(-i_overall_amps) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_FMBell_level)
kc1 = 5
kc2 = 5
kvdepth = 0.025
kvrate = 5.1
ifn1 = gi_FMBell_cosine
ifn2 = gi_FMBell_cosine
ifn3 = gi_FMBell_cosine
ifn4 = gi_FMBell_cosine
ivfn = gi_FMBell_cosine
i_exponent = 9
i_attack = .0005  
i_release = 0.01
;;;aenv transeg 0,  .003, i_exponent, 1.0, i_physical_decay, -i_exponent, 0
a_physical_envelope transeg 0,   i_attack, i_exponent,  1,   i_physical_decay, -i_exponent,  0

;;;a_signal fmbell 1, i_frequency, kc1, kc2, kvdepth, kvrate, ifn1, ifn2, ifn3, ifn4, ivfn
kc1 = 3 + k(a_physical_envelope) * 3
a_signal fmbell 1, i_frequency, kc1, kc2, kvdepth, kvrate, ifn1, ifn2, ifn3, ifn4, ivfn
xtratim 1
a_declicking cossegr 0, i_attack, 1, i_physical_decay, 1, i_release, 0
a_envelope = a_physical_envelope * a_declicking
a_envelope tonex a_envelope, 60, 4
a_signal = a_signal * i_amplitude * a_envelope * k_gain * 1.2

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
