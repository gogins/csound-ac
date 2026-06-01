/**
 * This is yet another attempt to adapt this beautiful sound for use in 
 * wider contexts. 
 *
 * In its original form this could not work without artifacts because of the 
 * k-rate frequency modulation of the pluck opcode.
 */
gi_Kung2_detune_cents chnexport "gk_Kung2_detune_cents", 3
gi_Kung2_detune_cents init 8
gk_Kung2_level chnexport "gk_Kung2_level", 3
gk_Kung2_level init 0
gk_Kung2_midi_dynamic_range chnexport "gk_Kung2_midi_dynamic_range", 3
gk_Kung2_midi_dynamic_range init 30
gk_Kung2_space_left_to_right chnexport "gk_Kung2_space_left_to_right", 3
gk_Kung2_space_left_to_right init .5
gi_Kung2_sine ftgen 0, 0, 65537, 10, 1
opcode Kung2_, aa, 0
setksmps 1
i_instrument = p1
i_time = p2
; Make indefinite notes last no longer than the physical decay.
i_instrument_duration = 999999
if p3 == -1 then
i_duration = i_instrument_duration
else
i_duration = p3
endif
i_midi_key = p4
i_midi_dynamic_range = i(gk_Kung2_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_Kung2_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 21 + 51 + 7
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Kung2_level)
ishift = gi_Kung2_detune_cents/12000
ipch = cpsmidinn(i_midi_key)
ioct = octmidinn(i_midi_key)
i_vibrato_frequency = (i_midi_key / 127) * 2
kvib poscil3 1/120, i_vibrato_frequency, gi_Kung2_sine
ag pluck 1, cpsoct(ioct+kvib), 1000, gi_Kung2_sine, 1
agleft pluck 1, cpsoct(ioct+ishift), 1000, gi_Kung2_sine, 1
agright pluck 1, cpsoct(ioct-ishift), 1000, gi_Kung2_sine, 1
i_physical_decay = 20
; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .003
i_attack = .001 / i_frequency + i_declick_minimum
i_exponent = 3
a_physical_envelope transeg 0,   i_attack, i_exponent,  1,   i_physical_decay, -i_exponent,  0
a_declicking_envelope cossegr 0, .006, 1, i_instrument_duration, 1,  .06, 0
a_envelope = a_physical_envelope * a_declicking_envelope
a_filtered_envelope tonex a_envelope, 30, 3
xtratim 3
ag = a_declicking_envelope * ag
agleft = a_declicking_envelope * agleft
agright = a_declicking_envelope * agright
adump delayr 0.3
ad1 deltap3 0.1
ad2 deltap3 0.2
delayw ag
a_out_left = agleft + ad1
a_out_right = agright + ad2
a_filtered_declicking tonex a_declicking_envelope, 30, 4
a_out_left = a_out_left * k_gain * i_amplitude * a_filtered_declicking
a_out_right = a_out_right * k_gain * i_amplitude * a_filtered_declicking 
xout a_out_left, a_out_right
endop

instr Kung2
a_out_left, a_out_right Kung2_
outleta "outleft", a_out_left 
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
