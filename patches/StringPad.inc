gi_StringPad_sine ftgen 0, 0, 65537, 10, 1
opcode StringPad_Chorus, aa, aaii
a1, a2, idlyml, ifrequency xin
k1 oscili idlyml/ifrequency, 1, gi_StringPad_sine
ar1l vdelay3 a1, idlyml/5+k1, 900 ;delayed sound 1
ar1r vdelay3 a2, idlyml/5+k1, 900 ;delayed sound 1
k2 oscili idlyml/ifrequency, .995, gi_StringPad_sine
ar2l vdelay3 a1, idlyml/5+k2, 700 ;delayed sound 2
ar2r vdelay3 a2, idlyml/5+k2, 700 ;delayed sound 2
k3 oscili idlyml/ifrequency, 1.05, gi_StringPad_sine
ar3l vdelay3 a1, idlyml/5+k3, 700 ;delayed sound 3
ar3r vdelay3 a2, idlyml/5+k3, 700 ;delayed sound 3
k4 oscili idlyml/ifrequency, 1, gi_StringPad_sine
ar4l vdelay3 a1, idlyml/5+k4, 900 ;delayed sound 4
ar4r vdelay3 a2, idlyml/5+k4, 900 ;delayed sound 4
aoutl = (a1+ar1l+ar2l+ar3l+ar4l)*.5
aoutr = (a2+ar1r+ar2r+ar3r+ar4r)*.5
xout aoutl, aoutr
endop

gk_StringPad_level chnexport "gk_StringPad_level", 3 ;  0
gi_StringPad_attack chnexport "gi_StringPad_attack", 3 ;  0.003
gi_StringPad_release chnexport "gi_StringPad_release", 3 ;  0.01
// TODO: Randomize this?
gi_StringPad_position chnexport "gi_StringPad_position", 3 ;  .76
gi_StringPad_chorus_delay_milliseconds chnexport "gi_StringPad_chorus_delay_milliseconds", 3 ;  10
gi_StringPad_chorus_frequency chnexport "gi_StringPad_chorus_frequency", 3 ;  30
gk_StringPad_midi_dynamic_range chnexport "gk_StringPad_midi_dynamic_range", 3 ;  20

gk_StringPad_level init 0
gi_StringPad_attack init 0.003
gi_StringPad_release init 0.01
// TODO: Randomize this?
gi_StringPad_position init .76
gi_StringPad_chorus_delay_milliseconds init 10
gi_StringPad_chorus_frequency init 30
gk_StringPad_midi_dynamic_range init 20

gk_StringPad_space_left_to_right chnexport "gk_StringPad_space_left_to_right", 3
gk_StringPad_space_left_to_right init .5


gi_StringPad_pad ftgen 0, 0, 65537, 10, 1, .5, .33, .25,  .0, .1,  .1, .1
gi_StringPad_sine ftgen 0, 0, 65537, 10, 1

instr StringPad
//////////////////////////////////////////////
// StringPad originally by Anthony Kozar.
// Adapted by Michael Gogins.
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
i_midi_key = p4
i_midi_dynamic_range = i(gk_StringPad_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
// Spatial location is specified in Ambisonic coordinates.
k_space_front_to_back = p6
// AKA stereo pan.
if p7 ==0 then
k_space_left_to_right = gk_StringPad_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
// Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 151
i_overall_amps = 65.2
i_normalization = ampdb(-i_overall_amps) / 2
i_midi_dynamic_range = i(gk_StringPad_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_StringPad_level)

; String-pad borrowed from the piece "Dorian Gray",
; http://akozar.spymac.net/music/ Modified to fit my needs
ihz = i_frequency
idb = i_midi_velocity
ipos = gi_StringPad_position
iamp = ampdb(idb)
; Slow attack and release
kctrl linseg 0, i_duration / 4, iamp, i_duration / 2, 0 
; Slight chorus effect
afund oscili 1, ihz, gi_StringPad_pad ; audio oscillator
acel1 oscili 1, ihz - .1, gi_StringPad_pad ; audio oscillator - flat
acel2 oscili 1, ihz + .1, gi_StringPad_pad ; audio oscillator - sharp
a_signal = afund + acel1 + acel2
; Cut-off high frequencies depending on midi-velocity
; (larger velocity implies more brighter sound)
a_signal butterlp a_signal, (p5-60)*40+900
; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .003
i_attack = .001 / i_frequency + i_declick_minimum
i_exponent = 7
a_physical_envelope transeg 0,   i_attack, i_exponent,  1,   i_duration, -i_exponent,  0
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
a_out_left, a_out_right StringPad_Chorus a_out_left, a_out_right, 10, 20
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
;printks "StringPad i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d l%9.4f r%9.4f\n", 1, p1, p2, p3, p4, p5, p7, active(p1), dbamp(rms(a_out_left)), dbamp(rms(a_out_right))
endin
