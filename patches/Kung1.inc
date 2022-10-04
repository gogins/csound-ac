gi_Kung1_detune_cents chnexport "gk_Kung1_detune_cents", 3
gi_Kung1_detune_cents init 8
gk_Kung1_level chnexport "gk_Kung1_level", 3
gk_Kung1_level init 0
gk_Kung1_midi_dynamic_range chnexport "gk_Kung1_midi_dynamic_range", 3
gk_Kung1_midi_dynamic_range init 30
gk_Kung1_space_left_to_right chnexport "gk_Kung1_space_left_to_right", 3
gk_Kung1_space_left_to_right init .5
gi_Kung1_sine ftgen 0, 0, 65537, 10, 1

/**
 * This is yet another attempt to adapt this beautiful sound for use in 
 * wider contexts.
 */
instr Kung1
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
i_midi_dynamic_range = i(gk_Kung1_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_Kung1_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 80
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Kung1_level)
ishift = gi_Kung1_detune_cents/12000
ipch = cpsmidinn(i_midi_key)
ioct = octmidinn(i_midi_key)
kvib poscil i_amplitude/120, ipch/50
ag pluck 1, cpsoct(ioct + kvib), 1000, gi_Kung1_sine, 1
agleft pluck 1, cpsoct(ioct+ishift), 1000, gi_Kung1_sine, 1
agright pluck 1, cpsoct(ioct-ishift), 1000, gi_Kung1_sine, 1
; Make indefinite notes last no longer than the physical decay.
i_physical_decay = 20
; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .003
i_attack = .001 / i_frequency + i_declick_minimum
i_exponent = 3
a_physical_envelope transeg 0,   i_attack, i_exponent,  1,   i_physical_decay, -i_exponent,  0
a_declicking_envelope cossegr 1, i_instrument_duration, 1,  .06, 0
a_envelope = a_physical_envelope * a_declicking_envelope
a_filtered_envelope tonex a_envelope, 120, 3
xtratim 1
af1 transeg 0.1,  5,  3, 1.0,  i_instrument_duration, 0, 1.0;
af2 transeg 1.0,  5, -3, 0.1,  i_instrument_duration, 0, 0.1;
adump delayr 4.0
atap1 deltap3 af1
atap2 deltap3 af2
ad1 deltap3 2.0
ad2 deltap3 1.1
delayw ag * a_filtered_envelope
a_out_left = agleft+atap1+ad1
a_out_right = agright+atap2+ad2
a_declicking_end tonex a_declicking_envelope, 60, 3
a_out_left = a_out_left * k_gain * i_amplitude * a_declicking_end
a_out_right = a_out_right * k_gain * i_amplitude * a_declicking_end
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
