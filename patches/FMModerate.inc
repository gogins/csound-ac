gk_FMModerate_level chnexport "gk_FMModerate_level", 3
gk_FMModerate_level init 0
gk_FMModerate_space_left_to_right chnexport "gk_Guitar_space_left_to_right", 3
gk_FMModerate_space_left_to_right init .5

gi_FMModerate_cosine ftgen 0, 0, 65537, 11, 1

instr FMModerate
; Authors: Michael Gogins
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
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
i_overall_amps = 86 ; To start - change to observed value.
i_normalization = ampdb(-i_overall_amps) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_FMModerate_level)
icarrier = 1
imodulator = 1
ifmamplitude = 8
index = 5.4
ifrequencyb = i_frequency * 1.003
icarrierb = icarrier * 1.004
aindenv transeg 1,  i_physical_decay, -10.0, 0
aindex = aindenv * index * ifmamplitude
aouta foscili 1.0, i_frequency, icarrier, imodulator, index, gi_FMModerate_cosine
aoutb foscili 1.0, ifrequencyb, icarrierb, imodulator, index, gi_FMModerate_cosine
a_signal = aouta + aoutb
i_attack = .002
i_release = 0.01
; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .008
i_attack = i_declick_minimum
i_exponent = 9
a_physical_envelope transeg 0,   i_attack, i_exponent,  1,   i_physical_decay, -i_exponent,  0
; The de-clicking envelope must have attack and release segments that damp 
; artifacts in the signal. The duration of these segments depends on 
; the behavior of the instrument, and may vary as a function of frequency.
i_declick_attack = .008
i_declick_release = .016
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_physical_decay, 1,  i_declick_release, 0
; The envelope of the instrument is the product of the physical envelope times 
; the declicking envelope. 
a_envelope = a_physical_envelope * a_declicking_envelope
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_envelope, 40, 4
a_signal = a_signal * i_amplitude * a_envelope * k_gain

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
