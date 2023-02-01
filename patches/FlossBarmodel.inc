gk_FlossBarmodel_level chnexport "gk_FlossBarmodel_level", 3
gk_FlossBarmodel_level init 0
gk_FlossBarmodel_space_left_to_right chnexport "gk_FlossBarmodel_space_left_to_right", 3
gk_FlossBarmodel_space_left_to_right init .5

gi_FlossBarmodel_ipres1 chnexport "gi_FlossBarmodel_ipres1", 3
gi_FlossBarmodel_ipres1 init .03
gi_FlossBarmodel_ipres2 chnexport "gi_FlossBarmodel_ipres2", 3
gi_FlossBarmodel_ipres2 init .1
gk_FlossBarmodel_krat chnexport "gk_FlossBarmodel_krat", 3
gk_FlossBarmodel_krat init 0.127236
gk_FlossBarmodel_kvibf chnexport "gk_FlossBarmodel_kvibf", 3
gk_FlossBarmodel_kvibf init 4.5
gk_FlossBarmodel_kvibamp chnexport "gk_FlossBarmodel_kvibamp", 3
gk_FlossBarmodel_kvibamp init 0
gi_FlossBarmodel_iminfreq chnexport "gi_FlossBarmodel_iminfreq", 3
gi_FlossBarmodel_iminfreq init 20

gi_FlossBarmodel_gisine ftgen 0,0,65537,10,1

instr FlossBarmodel
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
i_gain_correction = .2
i_amplitude = ampdb(i_midi_velocity) * i_gain_correction
// Adjust the gain correction until the maximum amplitude is about -6 dBFS.
k_gain = ampdb(gk_FlossBarmodel_level)
if p7 == 0 then
k_space_left_to_right = gk_FlossBarmodel_space_left_to_right
endif
k_space_bottom_to_top = p8
i_phase = p9

//////////////////////////////////////////////////////////////////////////////
// Instrument code begins.
//////////////////////////////////////////////////////////////////////////////

; boundary conditions 1=fixed 2=pivot 3=free
kbcL =  1
kbcR =  1
ip5 = .23
ip6 = .1
ip7 = 2
; stiffness
iK =  i_frequency ;p4
; high freq. loss (damping)
ib =  ip5
; scanning frequency
kscan rspline  ip6,ip7,0.2,0.8
; time to reach 30db decay
iT30 =  i_duration * 12 ;p3
; strike position
ipos random  0,1
; strike velocity
ivel =  i_amplitude ; 1000
; width of strike
iwid =  0.1156
a_signal barmodel kbcL,kbcR,iK,ib,kscan,iT30,ipos,ivel,iwid
kPan rspline  0.1,0.9,0.5,2
;aL,aR pan2  aSig,kPan
; outs  aL,aR
;t 0 90 1 30 2 60 5 90 7 30
; p4 = stiffness (pitch)

; #define gliss(dur'Kstrt'Kend'b'scan1'scan2)
; #
; i 1 0 20 $Kstrt $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur > $b $scan1 $scan2
; i 1 ^+0.05 $dur $Kend $b $scan1 $scan2
; #
; $gliss(15'40'400'0.0755'0.1'2)
; b 5
; $gliss(2'80'800'0.755'0'0.1)
; b 10
; $gliss(3'10'100'0.1'0'0)
; b 15
; $gliss(40'40'433'0'0.2'5)

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
a_declicking_envelope cossegr 0, i_declick_attack, 1, i_duration, 1, i_declick_release, 0
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
