gk_Dripwater_level chnexport "gk_Dripwater_level", 3
gk_Dripwater_level init 0
gk_Dripwater_space_left_to_right chnexport "gk_Dripwater_space_left_to_right", 3
gk_Dripwater_space_left_to_right init .5

instr Dripwater
; Credits:
; Michael Gogins

; Standard pfields:
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_velocity = p5
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9

; Process pfields:
i_physical_decay = 20
if p3 == -1 then
i_duration = i_physical_decay
else
endif
i_frequency = cpsmidinn(i_midi_key)
i_gain_correction = .09
i_amplitude = ampdb(i_midi_velocity) * i_gain_correction
// Adjust the gain correction until the maximum amplitude is about -6 dBFS.
k_gain = ampdb(gk_Dripwater_level)
if p7 == 0 then
k_space_left_to_right = gk_Dripwater_space_left_to_right
endif
k_space_bottom_to_top = p8
i_phase = p9

//////////////////////////////////////////////////////////////////////////////
// Instrument code begins.
//////////////////////////////////////////////////////////////////////////////

imaxshake = .5 ;ip4
ifreq     = i_frequency ;p5
ifreq1    = 1000 ;p6
ifreq2    = 800 ;p7
idettack = .9;0.09

;low amplitude
adrp dripwater .1, idettack, 10, .9, imaxshake, ifreq, ifreq1, ifreq2 
a_signal clip adrp, 2, 0.9	; avoid drips that drip too loud

//////////////////////////////////////////////////////////////////////////////
// Instrument code ends.
//////////////////////////////////////////////////////////////////////////////

; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .003
i_attack = .001 / i_frequency + i_declick_minimum
i_exponent = 7
a_physical_envelope =1;transeg 0,   i_attack, i_exponent,  1,   i_physical_decay, -i_exponent,  0
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
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
