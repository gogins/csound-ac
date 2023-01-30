gk_FlossWgbow_level chnexport "gk_FlossWgbow_level", 3
gk_FlossWgbow_level init 0
gk_FlossWgbow_space_left_to_right chnexport "gk_FlossWgbow_space_left_to_right", 3
gk_FlossWgbow_space_left_to_right init .5

gi_FlossWgbow_ipres1 chnexport "gi_FlossWgbow_ipres1", 3
gi_FlossWgbow_ipres1   init        .03
gi_FlossWgbow_ipres2 chnexport "gi_FlossWgbow_ipres2", 3
gi_FlossWgbow_ipres2   init        .1
gk_FlossWgbow_krat chnexport "gk_FlossWgbow_krat", 3
gk_FlossWgbow_krat     init        0.127236
gk_FlossWgbow_kvibf chnexport "gk_FlossWgbow_kvibf", 3
gk_FlossWgbow_kvibf    init        4.5
gk_FlossWgbow_kvibamp chnexport "gk_FlossWgbow_kvibamp", 3
gk_FlossWgbow_kvibamp  init        0
gi_FlossWgbow_iminfreq chnexport "gi_FlossWgbow_iminfreq", 3
gi_FlossWgbow_iminfreq init        20

gi_FlossWgbow_gisine ftgen 0,0,65537,10,1

instr FlossWgbow
; Credits:
; Michael Gogins
; Ian McCurdy

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
i_gain_correction = .001
i_amplitude = ampdb(i_midi_velocity) * i_gain_correction
// Adjust the gain correction until the maximum amplitude is about -6 dBFS.
k_gain = ampdb(gk_FlossWgbow_level)
if p7 == 0 then
k_space_left_to_right = gk_FlossWgbow_space_left_to_right
endif
k_space_bottom_to_top = p8
i_phase = p9

//////////////////////////////////////////////////////////////////////////////
// Instrument code begins.
//////////////////////////////////////////////////////////////////////////////

kamp     init i_amplitude
kfreq    init i_frequency
; kpres (bow pressure) defined using a random spline
kpres    rspline  gi_FlossWgbow_ipres1,gi_FlossWgbow_ipres2,0.5,2
; call the wgbow opcode
aSigL    wgbow    1,kfreq,kpres,gk_FlossWgbow_krat,gk_FlossWgbow_kvibf,gk_FlossWgbow_kvibamp,gi_FlossWgbow_gisine,gi_FlossWgbow_iminfreq
; modulating delay time
kdel     rspline  0.01,0.1,0.1,0.5
; bow pressure parameter delayed by a varying time in the right channel
kpres    vdel_k   kpres,kdel,0.2,2
aSigR    wgbow    1,kfreq,kpres,gk_FlossWgbow_krat,gk_FlossWgbow_kvibf,gk_FlossWgbow_kvibamp,gi_FlossWgbow_gisine,gi_FlossWgbow_iminfreq
a_signal = aSigL + aSigR
a_signal = a_signal * kamp
;outs a_signal, a_signal

//////////////////////////////////////////////////////////////////////////////
// Instrument code ends.
//////////////////////////////////////////////////////////////////////////////

; The de-clicking envelope must have attack and release segments that damp 
; artifacts in the signal. The duration of these segments depends on 
; the behavior of the instrument, and may vary as a function of frequency.
i_declick_attack = .005
i_declick_release = .015
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_duration, 1,  i_declick_release, 0
; The envelope of the instrument is the product of the physical envelope times 
; the declicking envelope. 
a_envelope = a_declicking_envelope
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_envelope, 40, 4
a_signal = a_signal * i_amplitude * a_filtered_envelope * k_gain *.001
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
