gk_Droner_partial1 chnexport "gk_Droner_partial1", 3
gk_Droner_partial2 chnexport "gk_Droner_partial2", 3
gk_Droner_partial3 chnexport "gk_Droner_partial3", 3
gk_Droner_partial4 chnexport "gk_Droner_partial4", 3
gk_Droner_partial5 chnexport "gk_Droner_partial5", 3
gk_Droner_partial6 chnexport "gk_Droner_partial6", 3
gk_Droner_partial7 chnexport "gk_Droner_partial7", 3
gk_Droner_partial8 chnexport "gk_Droner_partial8", 3
gk_Droner_partial9 chnexport "gk_Droner_partial9", 3
gk_Droner_partial10 chnexport "gk_Droner_partial10", 3
gk_Droner_level chnexport "gk_Droner_level", 3
gi_Droner_waveform chnexport "gi_Droner_waveform", 3

gk_Droner_partial1 init .5
gk_Droner_partial2 init .05
gk_Droner_partial3 init .1
gk_Droner_partial4 init .2
gk_Droner_partial5 init .1
gk_Droner_partial6 init 0
gk_Droner_partial7 init 0
gk_Droner_partial8 init 0
gk_Droner_partial9 init 0
gk_Droner_partial10 init 0
gk_Droner_level init 0
gi_Droner_waveform init 0

gk_Droner_space_left_to_right chnexport "gk_Guitar_space_left_to_right", 3
gk_Droner_space_left_to_right init .5


gi_Droner_sine ftgen 0, 0, 65537, 10, 1, 0, .02

instr Droner
i_instrument = p1
i_time = p2
; Make indefinite notes last no longer than the physical decay.
i_physical_decay = 200000
if p3 == -1 then
i_duration = i_physical_decay
else
i_duration = p3
endif
i_midi_key = p4
i_midi_velocity = p5
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_Droner_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_overall_amps = -20 + 98 + 4
i_normalization = ampdb(-i_overall_amps) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Droner_level)
k1 = gk_Droner_partial1
k2 = gk_Droner_partial2
k3 = gk_Droner_partial3
k4 = gk_Droner_partial4
k5 = gk_Droner_partial5
k6 = gk_Droner_partial6
k7 = gk_Droner_partial7
k8 = gk_Droner_partial8
k9 = gk_Droner_partial9
k10 = gk_Droner_partial10
iwaveform = gi_Droner_waveform
iattack = .5
idecay = .5
isustain = p3
aenvelope transegr 0.0, iattack / 2.0, 1.5, 1 / 2.0, iattack / 2.0, -1.5, 1, isustain, 0.0, 1, idecay / 2.0, 1.5, 1 / 2.0, idecay / 2.0, -1.5, 0
ihertz = cpsmidinn(i_midi_key)
if iwaveform == 0 goto i_waveform_0
if iwaveform == 1 goto i_waveform_1
if iwaveform == 2 goto i_waveform_2
i_waveform_0:
asignal poscil3 1, ihertz, gi_Droner_sine
goto i_waveform_endif
i_waveform_1:
asignal vco2 1, ihertz, 8 ; integrated saw
goto i_waveform_endif
i_waveform_2:
asignal vco2 1, ihertz, 12 ; triangle
i_waveform_endif:
a_signal chebyshevpoly asignal, 0, k1, k2, k3, k4, k5, k6, k7, k8, k9, k10
;adeclicking linsegr 0, .004, 1, p3 - .014, 1, .1, 0
;a_signal = asignal * adeclicking
;
; The de-clicking envelope must have attack and release segments that damp 
; artifacts in the signal. The duration of these segments depends on 
; the behavior of the instrument, and may vary as a function of frequency.
i_declick_attack = .005
i_declick_release = .01
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_duration, 1,  i_declick_release, 0
; The envelope of the instrument is the product of the physical envelope times 
; the declicking envelope. 
a_envelope = aenvelope * a_declicking_envelope
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_envelope, 40, 4
a_signal = a_signal * i_amplitude * a_filtered_envelope * k_gain *.001

prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
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
endin
