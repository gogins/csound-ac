gk_FlossWgbowEnhanced_level chnexport "gk_FlossWgbowEnhanced_level", 3
gk_FlossWgbowEnhanced_level init 0
gk_FlossWgbowEnhanced_space_left_to_right chnexport "gk_FlossWgbowEnhanced_space_left_to_right", 3
gk_FlossWgbowEnhanced_space_left_to_right init .5

gk_FlossWgbowEnhanced_ipres chnexport "gk_FlossWgbowEnhanced_ipres", 3
gk_FlossWgbowEnhanced_ipres   init        .2
gk_FlossWgbowEnhanced_krat chnexport "gk_FlossWgbowEnhanced_krat", 3
gk_FlossWgbowEnhanced_krat     init        0.127236
gk_FlossWgbowEnhanced_kvibf chnexport "gk_FlossWgbowEnhanced_kvibf", 3
gk_FlossWgbowEnhanced_kvibf    init        4.5
gk_FlossWgbowEnhanced_kvibamp chnexport "gk_FlossWgbowEnhanced_kvibamp", 3
gk_FlossWgbowEnhanced_kvibamp  init        0
gi_FlossWgbowEnhanced_iminfreq chnexport "gi_FlossWgbowEnhanced_iminfreq", 3
gi_FlossWgbowEnhanced_iminfreq init        20

gi_FlossWgbowEnhanced_gisine ftgen 0,0,65537,10,1

instr FlossWgbowEnhanced
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
i_gain_correction = .0005
i_amplitude = ampdb(i_midi_velocity) * i_gain_correction
// Adjust the gain correction until the maximum amplitude is about -6 dBFS.
k_gain = ampdb(gk_FlossWgbowEnhanced_level)
if p7 == 0 then
k_space_left_to_right = gk_FlossWgbowEnhanced_space_left_to_right
endif
k_space_bottom_to_top = p8
i_phase = p9

//////////////////////////////////////////////////////////////////////////////
// Instrument code begins.
//////////////////////////////////////////////////////////////////////////////

;kamp     init i_amplitude
;kfreq    init i_frequency
; kpres (bow pressure) defined using a random spline
;kpres    rspline  gi_FlossWgbowEnhanced_ipres1,gi_FlossWgbowEnhanced_ipres2,0.5,2
; call the wgbow opcode
;aSigL    wgbow    1,kfreq,kpres,gk_FlossWgbowEnhanced_krat,gk_FlossWgbowEnhanced_kvibf,gk_FlossWgbowEnhanced_kvibamp,gi_FlossWgbowEnhanced_gisine,gi_FlossWgbowEnhanced_iminfreq
; modulating delay time
;kdel     rspline  0.01,0.1,0.1,0.5
; bow pressure parameter delayed by a varying time in the right channel
;kpres    vdel_k   kpres,kdel,0.2,2
;a_signal = aSigL + aSigR
;a_signal = a_signal * kamp
;outs a_signal, a_signal


kamp     init i_amplitude
kfreq    init i_frequency
kpres    =        gk_FlossWgbowEnhanced_ipres
krat     rspline  0.006,0.988,0.1,0.4
kvibf    =        gk_FlossWgbowEnhanced_kvibf
kvibamp  =        gk_FlossWgbowEnhanced_kvibamp
iminfreq =        20
aSig     wgbow    kamp,kfreq,kpres,krat,kvibf,kvibamp,gi_FlossWgbowEnhanced_gisine,iminfreq
aSig     butlp     aSig,2000
a_signal     pareq    aSig,80,6,0.707

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
