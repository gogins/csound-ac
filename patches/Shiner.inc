gk_Shiner_midi_dynamic_range chnexport "gk_Shiner_midi_dynamic_range", 3 ;  127
gk_Shiner_attack chnexport "gk_Shiner_attack", 3 ;  .0125
gk_Shiner_release chnexport "gk_Shiner_release", 3 ;  .0125
gk_Shiner_level chnexport "gk_Shiner_level", 3 ;  0.5

gk_Shiner_midi_dynamic_range init 20
gk_Shiner_attack init .0125
gk_Shiner_release init .0125
gk_Shiner_level init 0.5

gk_Shiner_front_to_back chnexport "gk_Shiner_front_to_back", 3 ;  0
gk_Shiner_left_to_right chnexport "gk_Shiner_left_to_right", 3 ;  0.5
gk_Shiner_bottom_to_top chnexport "gk_Shiner_bottom_to_top", 3 ;  0

gk_Shiner_front_to_back init 0
gk_Shiner_left_to_right init 0.5
gk_Shiner_bottom_to_top init 0


instr Shiner
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
i_midi_dynamic_range = i(gk_Shiner_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_Shiner_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 42.5
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Shiner_level)
iattack = i(gk_Shiner_attack)
idecay = i(gk_Shiner_release)
isustain = i_duration
a_physical_envelope transeg 0.0, iattack / 2.0, 1.5, i_amplitude / 2.0, iattack / 2.0, -1.5, i_amplitude, isustain, 0.0, i_amplitude, idecay / 2.0, 1.5, i_amplitude / 2.0, idecay / 2.0, -1.5, 0
ihertz = cpsmidinn(i_midi_key)
gk_Harmonics = 1 * 20
a_signal vco2 4, ihertz, 12
kgain = ampdb(gk_Shiner_level) * .5
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
a_declicking_envelope cossegr 0, .005, 1,  i_duration, 1,  .01, 0
; The envelope of the instrument is the product of the physical envelope times 
; the declicking envelope. 
a_envelope = a_physical_envelope * a_declicking_envelope
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_envelope, 40, 4
a_signal = a_signal * i_amplitude * a_filtered_envelope * k_gain *.001
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, gk_Shiner_front_to_back, gk_Shiner_left_to_right, gk_Shiner_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
endin
