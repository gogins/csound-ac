gk_TubularBell_level chnexport "gk_TubularBell_level", 3
gi_TubularBell_crossfade chnexport "gi_TubularBell_crossfade", 3
gi_TubularBell_vibrato_depth chnexport "gi_TubularBell_vibrato_depth", 3
gi_TubularBell_vibrato_rate chnexport "gi_TubularBell_vibrato_rate", 3

gk_TubularBell_level init 0
gi_TubularBell_crossfade init 2
gi_TubularBell_vibrato_depth init .1
gi_TubularBell_vibrato_rate init 5
gk_TubularBell_midi_dynamic_range chnexport "gk_TubularBell_midi_dynamic_range", 3 ; 127
gk_TubularBell_midi_dynamic_range init 30

gk_TubularBell_space_left_to_right chnexport "gk_TubularBell_space_left_to_right", 3
gk_TubularBell_space_left_to_right init .5

gi_TubularBell_isine ftgen 0, 0, 65537, 10, 1
gi_TubularBell_icosine ftgen 0, 0, 65537, 11, 1
gi_TubularBell_icook3 ftgen 0, 0, 65537, 10, 1, .4, 0.2, 0.1, 0.1, .05

instr TubularBell
; Authors: Perry Cook, John ffitch, Michael Gogins
i_instrument = p1
i_time = p2
i_physical_decay = 15
if p3 == -1 then
i_duration = i_physical_decay
else
i_duration = p3
endif
i_midi_key = p4
i_midi_velocity = p5
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_TubularBell_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_overall_amps = 12.5
i_normalization = ampdb(-i_overall_amps) / 2
i_midi_dynamic_range = i(gk_TubularBell_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_TubularBell_level)
iattack = 0.002
isustain = p3
irelease = 0.05
iindex = 1
ifn1 = gi_TubularBell_isine
ifn2 = gi_TubularBell_icook3
ifn3 = gi_TubularBell_isine
ifn4 = gi_TubularBell_isine
ivibefn = gi_TubularBell_icosine
a_signal fmbell 1, i_frequency, iindex, gi_TubularBell_crossfade, gi_TubularBell_vibrato_depth, gi_TubularBell_vibrato_rate, ifn1, ifn2, ifn3, ifn4, ivibefn
i_attack = .002
i_sustain = i_duration
i_release = 0.01
; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .001
i_attack = .001 / i_frequency + i_declick_minimum
i_exponent = 7
a_physical_envelope transeg 0,   i_attack, i_exponent,  1,   i_physical_decay, -i_exponent,  0
; The de-clicking envelope must have attack and release segments that damp 
; artifacts in the signal. The duration of these segments depends on 
; the behavior of the instrument, and may vary as a function of frequency.
i_declick_attack = .004
i_declick_release = i_declick_minimum * 2
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_duration, 1,  i_declick_release, 0
; The envelope of the instrument is the product of the physical envelope times 
; the declicking envelope. 
a_envelope = a_physical_envelope * a_declicking_envelope
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_envelope, 120, 4
a_signal = a_signal * i_amplitude * a_filtered_envelope * k_gain *.001


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
