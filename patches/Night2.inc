gk_Night2_level chnexport "gk_Night2_level", 3 ;  0
gi_Night2_attack chnexport "gi_Night2_attack", 3 ;  0.003
gi_Night2_release chnexport "gi_Night2_release", 3 ;  0.01
gi_Night2_balance chnexport "gi_Night2_balance", 3 ;  .6
gk_Night2_midi_dynamic_range chnexport "gk_Night2_midi_dynamic_range", 3 ;  20

gk_Night2_level init 0
gi_Night2_attack init 0.003
gi_Night2_release init 0.01
gi_Night2_balance init .6
gk_Night2_midi_dynamic_range init 20

gk_Night2_space_left_to_right chnexport "gk_Night2_space_left_to_right", 3
gk_Night2_space_left_to_right init .5


gi_Night2_sine ftgen 0, 0, 65537, 10, 1
gi_Night2_sigmoid ftgen 0, 0, 65537, 19, .5, 1, 270, 1

instr Night2
//////////////////////////////////////////////
// Instrument by Volkov
// Adapted by Michael Gogins
//////////////////////////////////////////////
i_instrument = p1
i_time = p2
; Make indefinite notes last no longer than the physical decay.
i_physical_decay = 20
if p3 == -1 then
i_duration = i_physical_decay
else
i_duration = p3
endif
i_sustain = i_duration
i_midi_key = p4
i_midi_dynamic_range = i(gk_Night2_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
// Spatial location is specified in Ambisonic coordinates.
k_space_front_to_back = p6
// AKA stereo pan.
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_Night2_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
// Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 69.5
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Night2_level)
iamp = i_amplitude 
inote = i_frequency
ibal = gi_Night2_balance 
if1 = 0
if2 = 0 
ind1 = 35
ind2 = 30
k1 linseg 0,i_sustain/2,iamp,i_sustain/2, 0
//  ares pluck kamp, kcps, icps, ifn, imeth [, iparm1] [, iparm2]
a3 pluck k1,inote*.997,ind1,gi_Night2_sine,1 
a2 pluck k1,inote*1.003,ind1,gi_Night2_sine,1 
; a1 pluck k1,inote,ind1,if1,1 
aplk1 = a2+a3
k2 linseg 0,i_sustain*.66,iamp*1.2,i_sustain/3,0
a6 pluck k2,inote*.995,ind2,gi_Night2_sine,1 
a5 pluck k2,inote*1.005,ind2,gi_Night2_sine,1 
; a4 pluck k2,inote,ind2,if2,1 
aplk2 = a5+a6
a_signal = aplk1+aplk2
; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .003
i_attack = .001 / i_frequency + i_declick_minimum
i_exponent = 7
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
a_filtered_envelope tonex a_envelope, 40, 4
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
;printks "Night2 i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d l%9.4f r%9.4f\n", 1, p1, p2, p3, p4, p5, p7, active(p1), dbamp(rms(a_out_left)), dbamp(rms(a_out_right))
endin
