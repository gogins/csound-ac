gi_Kung4_detune_cents chnexport "gk_Kung4_detune_cents", 3
gi_Kung4_detune_cents init 8
gk_Kung4_level chnexport "gk_Kung4_level", 3
gk_Kung4_level init 0
gk_Kung4_midi_dynamic_range chnexport "gk_Kung4_midi_dynamic_range", 3
gk_Kung4_midi_dynamic_range init 30
gk_Kung4_space_left_to_right chnexport "gk_Kung4_space_left_to_right", 3
gk_Kung4_space_left_to_right init .5
gi_Kung4_modulation_ratio_start chnexport "gi_Kung4_modulation_ratio_start", 3
gi_Kung4_modulation_ratio_start init 1.7
gi_Kung4_modulation_ratio_end chnexport "gi_Kung4_modulation_ratio_end", 3
gi_Kung4_modulation_ratio_end init .5

gi_Kung4_sine ftgen 0, 0, 65537, 10, 1
gi_Kung4_cosine ftgen 0, 0, 65537, 11, 1
gi_Kung4_ln ftgen 0, 9, 65537, -12, 20.0  ;unscaled ln(I(x)) from 0 to 20.0
/**
 * This is yet another attempt to adapt this beautiful sound for use in 
 * wider contexts.
 */
instr Kung4
i_instrument = p1
i_time = p2
; Make indefinite notes last no longer than the physical decay.
i_instrument_duration = 999999
if p3 == -1 then
i_duration = i_instrument_duration
else
i_duration = p3
endif
print i_duration
i_midi_key = p4
i_midi_dynamic_range = i(gk_Kung4_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_Kung4_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 130 + 8 + 6
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Kung4_level)
ishift = gi_Kung4_detune_cents/12000
ipch = cpsmidinn(i_midi_key)
ioct = octmidinn(i_midi_key)
kvib poscil 1/120, ipch/50, gi_Kung4_sine
; The instrument envelope should observe limits.
if (i_duration > 10) then
i_instrument_attack = 10 / 3
elseif (i_duration < 1) then
i_instrument_attack = 1 / 3
else
i_instrument_attack = i_duration / 3
endif
print i_instrument_attack
;i_instrument_duration -= (2 * i_instrument_attack)
aadsr linsegr 0, i_instrument_attack, 1.0, i_duration, 1.0, i_instrument_attack, 0 ;ADSR envelope
amodi linseg 0, i_instrument_attack, 5, i_duration, 3, i_instrument_attack, 0 ;ADSR envelope for I
amodr linseg gi_Kung4_modulation_ratio_start, i_duration, gi_Kung4_modulation_ratio_end ;r moves from p6->p7 in p3 sec.
a1 = amodi*(amodr-1/amodr)/2
a1ndx = abs(a1*2/20) ;a1*2 is normalized from 0-1.
a2 = amodi*(amodr+1/amodr)/2
a3 tablei a1ndx, gi_Kung4_ln, 1 ;lookup tbl in f3, normal index
ao1 poscil a1, ipch, gi_Kung4_cosine ;cosine
a4 = exp(-0.5*a3+ao1)
ao2 poscil a2*ipch, ipch, gi_Kung4_cosine ;cosine
a_out_left poscil 1000*aadsr*a4, ao2+cpsoct(ioct+ishift), gi_Kung4_sine ;fnl outleft
a_out_right poscil 1000*aadsr*a4, ao2+cpsoct(ioct-ishift), gi_Kung4_sine ;fnl outright
i_declick_minimum = .003
i_attack = .001 / i_frequency + i_declick_minimum
i_exponent = 7
a_physical_envelope = aadsr ;  transeg 0,   i_attack, i_exponent,  1,   i_duration, -i_exponent,  0
; The de-clicking envelope must have attack and release segments that damp 
; artifacts in the signal. The duration of these segments depends on 
; the behavior of the instrument, and may vary as a function of frequency.
i_declick_attack = i_attack
i_declick_release = i_declick_minimum * 2
; The end of the note must be extended _past_ the end of the release segment.
xtratim 5
print i_declick_attack
print i_declick_release
print i_duration
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_duration, 1,  i_declick_release, 0
; The envelope of the instrument is the product of the physical envelope times 
; the declicking envelope. 
a_envelope = a_physical_envelope * a_declicking_envelope
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_envelope, 40, 4
a_out_left = a_out_left * i_amplitude * a_filtered_envelope * k_gain 
a_out_right = a_out_right * i_amplitude * a_filtered_envelope * k_gain 

outleta "outleft", a_out_left 
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
