gk_Melody_midi_dynamic_range chnexport "gk_Melody_midi_dynamic_range", 3 ; 127
gk_Melody_level chnexport "gk_Melody_level", 3 ; 0

gk_Melody_midi_dynamic_range init 20
gk_Melody_level init 0

gi_Melody_chebyshev ftgen 0, 0, 65537, -7, -1, 150, 0.1, 110, 0, 252, 0
gi_Melody_sine ftgen 0, 0, 65537, 10, 1
gi_Melody_cook3 ftgen 0, 0, 65537, 10, 1, .4, 0.2, 0.1, 0.1, .05
instr Melody
; Author: Jon Nelson
; Adapted by: Michael Gogins
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
i_midi_dynamic_range = i(gk_Melody_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 67
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Melody_level)
ip3 init 3.0
iattack = 0.05
isustain = i_duration
irelease = 0.1
ip6 = gi_Melody_chebyshev
i1 = i_frequency
k100 randi 1,10
k101 poscil 1, 5 + k100, gi_Melody_sine
ak102 linseg 0, .5, 1, p3, 1
k100 = i1 + (k101 * ak102)
; Envelope for driving oscillator.
; k1 linenr 0.5, ip3 * .3, ip3 * 2, 0.01
k1 linseg 0, ip3 * .3, .5, ip3 * 2, 0.01, isustain, 0.01, irelease, 0
; k2 line 1, p3, .5
k2 linseg 1.0, ip3, .5, isustain, .5, irelease, 0
k1 = k2 * k1
; Amplitude envelope.
i_exponent = 4
a_physical_envelope transeg 0,   iattack, i_exponent,  1,   i_duration, 0, 1,  irelease, -i_exponent, 0

; Power to partials.
k20 linseg 1.485, iattack, 1.5, (isustain + irelease), 1.485
; a1-3 are for cheby with p6=1-4
a1 poscil k1, k100 - .025, gi_Melody_cook3
; Tables a1 to fn13, others normalize,
a2 tablei a1, ip6, 1, .5
a3 balance a2, a1
; Try other waveforms as well.
a4 foscili 1, k100 + .04, 1, 2.005, k20, gi_Melody_sine
a5 poscil 1, k100, gi_Melody_sine
a6 = ((a3 * .1) + (a4 * .1) + (a5 * .8)) * a_physical_envelope
a7 comb a6, .5, 1 / i1
a8 = (a6 * .9) + (a7 * .1)
a_signal balance a8, a1
; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .003
i_attack = .001 / i_frequency + i_declick_minimum
i_exponent = 7
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

aleft, aright pan2 a_signal * i_amplitude * k_gain, k_space_left_to_right


#ifdef USE_SPATIALIZATION
a_signal = aleft + aright
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
outleta "outleft", aleft
outleta "outright", aright
#endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
