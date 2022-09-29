gk_PulseWidthModulation_level chnexport "gk_PulseWidthModulation_level", 3 ;  0
gi_PulseWidthModulation_attack chnexport "gi_PulseWidthModulation_attack", 3 ;  0.003
gi_PulseWidthModulation_release chnexport "gi_PulseWidthModulation_release", 3 ;  0.01
gk_PulseWidthModulation_midi_dynamic_range chnexport "gk_PulseWidthModulation_midi_dynamic_range", 3 ;  20

gk_PulseWidthModulation_level init 0
gi_PulseWidthModulation_attack init 0.003
gi_PulseWidthModulation_release init 0.01
gk_PulseWidthModulation_midi_dynamic_range init 20

gk_PulseWidthModulation_left_to_right chnexport "gk_PulseWidthModulation_left_to_right", 3
gk_PulseWidthModulation_left_to_right init .5


gi_PulseWidthModulation_sine ftgen 0, 0, 65537, 10, 1

instr PulseWidthModulation
//////////////////////////////////////////////
//  Instrument definition patch template.
//  Author: Michael Gogins
//////////////////////////////////////////////
i_instrument = p1
i_time = p2
; Make indefinite notes last no longer than the physical decay.
i_physical_decay = 20000
if p3 == -1 then
i_duration = i_physical_decay
else
i_duration = p3
endif
i_sustain = i_duration
xtratim 1
i_midi_key = p4
i_midi_dynamic_range = i(gk_PulseWidthModulation_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
//  Spatial location is specified in Ambisonic coordinates.
k_space_front_to_back = p6
//  AKA stereo pan.
if p7 ==0 then
k_space_left_to_right = gk_PulseWidthModulation_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
//  Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 221.2 - 45
i_normalization = ampdb(-i_level_correction)
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_PulseWidthModulation_level)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; coded by Josep M Comajuncosas / jan98
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
iamp = 10000; intended max. amplitude
;;kenv linen 1, .2,i_duration,.2; volume envelope

kfreq = i_frequency
ibound = sr/4; set it to sr/2 for true BL square wave
kratio line .01, i_duration, .99; fractional width of the pulse part of a cycle
apulse1 buzz 1, kfreq, ibound/kfreq, gi_PulseWidthModulation_sine
; comb filter to give pulse width modulation
atemp delayr 1/20
apulse2 deltapi kratio/kfreq
delayw apulse1
avpw = apulse1 - apulse2
apwmdc integ avpw
apwm = apwmdc + kratio - .5
a_signal = apwm*iamp

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
a_envelope = a_physical_envelope * a_declicking_envelope
a_filtered_envelope tonex a_declicking_envelope, 40, 4
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
;printks "PulseWidthModulation      i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d l%9.4f r%9.4f\n", 1, p1, p2, p3, p4, p5, p7, active(p1), dbamp(rms(a_out_left)), dbamp(rms(a_out_right))
endin
