gi_SeidelHarmonicOscillator_tabsz init 2^16

gi_SeidelHarmonicOscillator_Sin     ftgen 0, 0, gi_SeidelHarmonicOscillator_tabsz, 9, 1,1,0
gi_SeidelHarmonicOscillator_Tri     ftgen 0, 0, gi_SeidelHarmonicOscillator_tabsz, 9, 1,1,0,  3,0.333,180,  5,0.2,0,  7,0.143,180, 9,0.111,0, 11,0.091,180, 13,0.077,0, 15,0.067,180, 17,0.059,0, 19,0.053,180, 21,0.048,0, 23,0.043,180, 25,0.04,0, 27,0.037,180, 29,0.034,0, 31,0.032,180
gi_SeidelHarmonicOscillator_Saw     ftgen 0, 0, gi_SeidelHarmonicOscillator_tabsz, 7, 0, gi_SeidelHarmonicOscillator_tabsz/2, 1, 0, -1, gi_SeidelHarmonicOscillator_tabsz/2, 0
gi_SeidelHarmonicOscillator_Square  ftgen 0, 0, gi_SeidelHarmonicOscillator_tabsz, 7, 1, gi_SeidelHarmonicOscillator_tabsz/2, 1, 0, -1, gi_SeidelHarmonicOscillator_tabsz/2, -1
gi_SeidelHarmonicOscillator_Prime   ftgen 0, 0, gi_SeidelHarmonicOscillator_tabsz, 9, 1,1,0,  2,0.5,0,  3,0.3333,0,  5,0.2,0,   7,0.143,0,  11,0.0909,0,  13,0.077,0,   17,0.0588,0,  19,0.0526,0, 23,0.0435,0, 27,0.037,0, 31,0.032,180
gi_SeidelHarmonicOscillator_Fib     ftgen 0, 0, gi_SeidelHarmonicOscillator_tabsz, 9, 1,1,0,  2,0.5,0,  3,0.3333,0,  5,0.2,0,   8,0.125,0,  13,0.0769,0,  21,0.0476,0,  34,0.0294,0 ;,  55,0.0182,0,  89,0.0112,0, 144,0.0069,0

gi_SeidelHarmonicOscillator_NumTables = 5
gi_SeidelHarmonicOscillator_List ftgen 1000, 0, gi_SeidelHarmonicOscillator_NumTables, -2, gi_SeidelHarmonicOscillator_Tri, gi_SeidelHarmonicOscillator_Saw, gi_SeidelHarmonicOscillator_Square, gi_SeidelHarmonicOscillator_Prime, gi_SeidelHarmonicOscillator_Fib, gi_SeidelHarmonicOscillator_Sin
gi_SeidelHarmonicOscillator_Morf ftgen 1001, 0, gi_SeidelHarmonicOscillator_tabsz, 10, 1

gi_SeidelHarmonicOscillator_lforabsz init 2^13
gi_SeidelHarmonicOscillator_LfoTri ftgen 0, 0, gi_SeidelHarmonicOscillator_lforabsz, 7, 0, gi_SeidelHarmonicOscillator_lforabsz/4, 1, gi_SeidelHarmonicOscillator_lforabsz/2, -1, gi_SeidelHarmonicOscillator_lforabsz/4, 0

gk_SeidelHarmonicOscillator_level chnexport "gk_SeidelHarmonicOscillagi_SeidelHarmonicOscillator_tor_level", 3 ;  0
gi_SeidelHarmonicOscillator_attack chnexport "gi_SeidelHarmonicOscillator_attack", 3 ;  0.003
gi_SeidelHarmonicOscillator_petals chnexport "gi_SeidelHarmonicOscillator_petals", 3 ;  2.99
gi_SeidelHarmonicOscillator_release chnexport "gi_SeidelHarmonicOscillator_release", 3 ;  0.01
gk_SeidelHarmonicOscillator_midi_dynamic_range chnexport "gk_SeidelHarmonicOscillator_midi_dynamic_range", 3 ;  20

gk_SeidelHarmonicOscillator_level init 0
gi_SeidelHarmonicOscillator_attack init 0.003
gi_SeidelHarmonicOscillator_petals init 2.99
gi_SeidelHarmonicOscillator_release init 0.01
gk_SeidelHarmonicOscillator_midi_dynamic_range init 20

gk_SeidelHarmonicOscillator_P1 chnexport "gk_SeidelHarmonicOscillator_P1", 3
gk_SeidelHarmonicOscillator_IN1CON chnexport "gk_SeidelHarmonicOscillator_IN1CON", 3
gk_SeidelHarmonicOscillator_IN1C1 chnexport "gk_SeidelHarmonicOscillator_IN1C1", 3

gk_SeidelHarmonicOscillator_P1 init 0.1
gk_SeidelHarmonicOscillator_IN1CON init 0.1
gk_SeidelHarmonicOscillator_IN1C1 init 1

gk_SeidelHarmonicOscillator_P2 chnexport "gk_SeidelHarmonicOscillator_P2", 3
gk_SeidelHarmonicOscillator_IN2C1 chnexport "gk_SeidelHarmonicOscillator_IN2C1", 3
gk_SeidelHarmonicOscillator_IN2CON chnexport "gk_SeidelHarmonicOscillator_IN2CON", 3

gk_SeidelHarmonicOscillator_P2 init 0.1
gk_SeidelHarmonicOscillator_IN2C1 init 1
gk_SeidelHarmonicOscillator_IN2CON init 0.1

gk_SeidelHarmonicOscillator_P3 chnexport "gk_SeidelHarmonicOscillator_P3", 3
gk_SeidelHarmonicOscillator_IN3C1 chnexport "gk_SeidelHarmonicOscillator_IN3C1", 3
gk_SeidelHarmonicOscillator_IN3CON chnexport "gk_SeidelHarmonicOscillator_IN3CON", 3

gk_SeidelHarmonicOscillator_P3 init 0.1
gk_SeidelHarmonicOscillator_IN3C1 init 1
gk_SeidelHarmonicOscillator_IN3CON init 0.1

gk_SeidelHarmonicOscillator_P4 chnexport "gk_SeidelHarmonicOscillator_P4", 3
gk_SeidelHarmonicOscillator_IN4C1 chnexport "gk_SeidelHarmonicOscillator_IN4C1", 3
gk_SeidelHarmonicOscillator_IN4CON chnexport "gk_SeidelHarmonicOscillator_IN4CON", 3

gk_SeidelHarmonicOscillator_P4 init 0.1
gk_SeidelHarmonicOscillator_IN4C1 init 1
gk_SeidelHarmonicOscillator_IN4CON init 0.1

gi_SeidelHarmonicOscillator_pitch_bend_table ftgen 0, 0, 1024, -7, 1, 1024, 1 
gi_SeidelHarmonicOscillator_sine ftgen 0, 0, 65537, 10, 1

instr SeidelHarmonicOscillator
i_instrument = p1
i_time = p2
i_sustain = p3
xtratim gi_SeidelHarmonicOscillator_attack + gi_SeidelHarmonicOscillator_release
i_midi_key = p4
i_midi_dynamic_range = i(gk_SeidelHarmonicOscillator_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
// Spatial location is specified in Ambisonic coordinates.
k_space_front_to_back = p6
// AKA stereo pan.
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
// Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 87
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_SeidelHarmonicOscillator_level)
; prints("p1=%f, p2=%f, p3=%f, p4=%f\n", p1, p2, p3, p4)
;;;kfreq chnget sprintf("FREQ%d", p4)
kfreq init i_frequency

kin1con init 0
kin2con init 0
kin3con init 0
kin4con init 0

koff  init 0.001
koff1 init 0.001
koff2 init 2 * 0.001
koff3 init 3 * 0.001
koff4 init 5 * 0.001
koffa = scale2(gk_SeidelHarmonicOscillator_P1, 0.001, 0.1, -10, 10)
if (gk_SeidelHarmonicOscillator_IN1CON == 1) then
koffb = scale2(gk_SeidelHarmonicOscillator_IN1C1, 0.001, 0.1, 0, 10)
koff = koffa + koffb
else
koff = koffa
endif
koff1 = koff
koff2 = 2 * koff
koff3 = 3 * koff
koff4 = 5 * koff

itbl = gi_SeidelHarmonicOscillator_Morf
kndx init 0
kndxa = scale2(gk_SeidelHarmonicOscillator_P2, 0, gi_SeidelHarmonicOscillator_NumTables-1.01, -10, 10)
if (gk_SeidelHarmonicOscillator_IN2CON == 1) then
kndxb = scale2(gk_SeidelHarmonicOscillator_IN2C1, 0, gi_SeidelHarmonicOscillator_NumTables-1.01, 0, 10)
kndx = kndxa + kndxb
else
kndx = kndxa
endif
ftmorf kndx, gi_SeidelHarmonicOscillator_List, gi_SeidelHarmonicOscillator_Morf

kamp init 0.8/9

; a1 oscil3 kamp, kfreq, itbl
a2 oscil3 kamp, kfreq+koff1, itbl
a3 oscil3 kamp, kfreq+koff2, itbl
a4 oscil3 kamp, kfreq+koff3, itbl
a5 oscil3 kamp, kfreq+koff4, itbl
a6 oscil3 kamp, kfreq-koff1, itbl
a7 oscil3 kamp, kfreq-koff2, itbl
a8 oscil3 kamp, kfreq-koff3, itbl
a9 oscil3 kamp, kfreq-koff4, itbl

kdst init 0
kdsta = scale2(gk_SeidelHarmonicOscillator_P3, 0, 10, -10, 10)
if (gk_SeidelHarmonicOscillator_IN3CON == 1) then
kdstb = scale2(gk_SeidelHarmonicOscillator_IN3C1, 0, 10, 0, 10)
kdst = kdsta + kdstb
else
kdst = kdsta
endif

aL = a2+a4+a6+a8
aR = a3+a5+a7+a9
aoutL = aL + distort1(aL, kdst, 0.1, 0, 0)
aoutR = aR + distort1(aR, kdst, 0.1, 0, 0)

kpana = scale2(gk_SeidelHarmonicOscillator_P4, 0, 7, -10, 10)
if (gk_SeidelHarmonicOscillator_IN4CON == 1) then
kpanb = scale2(gk_SeidelHarmonicOscillator_IN4C1, 0, 5, 0, 10)
kpan = kpana + kpanb
else
kpan = kpana
endif
if (kpan == 0) then
kext = 0
else
kext = 0.1
endif
klfoL oscili 0.49, kpan-kext, gi_SeidelHarmonicOscillator_LfoTri, 90
klfoR oscili 0.49, kpan+kext, gi_SeidelHarmonicOscillator_LfoTri, 270
klfoL += 0.5
klfoR += 0.5
aoutL1, aoutR1 pan2 aoutL, klfoL
aoutL2, aoutR2 pan2 aoutR, klfoR
aoutL = aoutL1+aoutL2
aoutR = aoutR1+aoutR2

aenv madsr 0.07,0,1,0.7
a_out_left = aoutL * aenv * k_gain * i_amplitude
a_out_right = aoutR * aenv * k_gain * i_amplitude
;;; outs aoutL*aenv, aoutR*aenv
#ifdef USE_SPATIALIZATION
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, k_space_front_to_back, k_space_left_to_right, k_space_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
;;; a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
;printks "WGPluck      %9.4f   %9.4f\n", 0.5, a_out_left, a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
