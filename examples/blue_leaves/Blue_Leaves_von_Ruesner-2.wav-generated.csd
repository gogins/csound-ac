<CsoundSynthesizer>
<CsOptions>
--midi-key=4 --midi-velocity=5 -m168 -j1 -RWdfo "/Users/michaelgogins/csound-ac/examples/blue_leaves/Blue_Leaves_von_Ruesner-2.wav"
</CsOptions>
<CsInstruments>


sr                              =                       96000
ksmps                           =                       100
nchnls                          =                       2
0dbfs                           =                       2000
iampdbfs                        init                    32768
                                prints                  "Default amplitude at 0 dBFS:  %9.4f\n", iampdbfs
idbafs                          init                    dbamp(iampdbfs)
                                prints                  "dbA at 0 dBFS:                 %9.4f\n", idbafs
iheadroom                       init                    6
                                prints                  "Headroom (dB):                 %9.4f\n", iheadroom
idbaheadroom                    init                    idbafs - iheadroom
                                prints                  "dbA at headroom:               %9.4f\n", idbaheadroom
iampheadroom                    init                    ampdb(idbaheadroom)
                                prints                  "Amplitude at headroom:        %9.4f\n", iampheadroom
                                prints                  "Balance so the overall amps at the end of performance -6 dbfs.\n"

giFlatQ                         init                    sqrt(0.5)
giseed				            init                    0.5

gkHarpsichordGain               chnexport               "gkHarpsichordGain",            1
gkHarpsichordGain               init                    1
gkHarpsichordPan                chnexport               "gkHarpsichordPan",             1
gkHarpsichordPan                init                    0.5

gkChebyshevDroneCoefficient1    chnexport               "gkChebyshevDroneCoefficient1", 1
gkChebyshevDroneCoefficient1    init                    0.5
gkChebyshevDroneCoefficient2    chnexport               "gkChebyshevDroneCoefficient2", 1
gkChebyshevDroneCoefficient3    chnexport               "gkChebyshevDroneCoefficient3", 1
gkChebyshevDroneCoefficient4    chnexport               "gkChebyshevDroneCoefficient4", 1
gkChebyshevDroneCoefficient5    chnexport               "gkChebyshevDroneCoefficient5", 1
gkChebyshevDroneCoefficient6    chnexport               "gkChebyshevDroneCoefficient6", 1
gkChebyshevDroneCoefficient7    chnexport               "gkChebyshevDroneCoefficient7", 1
gkChebyshevDroneCoefficient8    chnexport               "gkChebyshevDroneCoefficient8", 1
gkChebyshevDroneCoefficient9    chnexport               "gkChebyshevDroneCoefficient9", 1
gkChebyshevDroneCoefficient10   chnexport               "gkChebyshevDroneCoefficient10", 1
gkChebyshevDroneCoefficient10   init                    0.05

gkReverberationEnabled          chnexport               "gkReverberationEnabled", 1
gkReverberationEnabled          init                    1
gkReverberationDelay            chnexport               "gkReverberationDelay", 1
gkReverberationDelay            init                    0.65
gkReverberationWet          	chnexport               "gkReverberationWet", 1
gkReverberationWet          	init                    0.125

gkMasterLevel                   chnexport               "gkMasterLevel", 1
gkMasterLevel                   init                    1.5

                                connect                 "BanchoffKleinBottle",  "outleft", 	"Reverberation",        "inleft"
                                connect                 "BanchoffKleinBottle",  "outright", "Reverberation",        "inright"
                                connect                 "BandedWG",             "outleft", 	"Reverberation",        "inleft"
                                connect                 "BandedWG",             "outright", "Reverberation",        "inright"
                                connect                 "BassModel",            "outleft", 	"Reverberation",        "inleft"
                                connect                 "BassModel",            "outright", "Reverberation",        "inright"
                                connect                 "ChebyshevDrone",       "outleft", 	"Reverberation",        "inleft"
                                connect                 "ChebyshevDrone",       "outright", "Reverberation",        "inright"
                                connect                 "ChebyshevMelody",      "outleft", 	"Reverberation",        "inleft"
                                connect                 "ChebyshevMelody",      "outright", "Reverberation",        "inright"
                                connect                 "DelayedPluckedString", "outleft", 	"Reverberation",        "inleft"
                                connect                 "DelayedPluckedString", "outright", "Reverberation",        "inright"
                                connect                 "EnhancedFMBell",       "outleft", 	"Reverberation",        "inleft"
                                connect                 "EnhancedFMBell",       "outright", "Reverberation",        "inright"
                                connect                 "FenderRhodesModel",    "outleft", 	"Reverberation",        "inleft"
                                connect                 "FenderRhodesModel",    "outright", "Reverberation",        "inright"
                                connect                 "FilteredSines",        "outleft", 	"Reverberation",        "inleft"
                                connect                 "FilteredSines",        "outright", "Reverberation",        "inright"
                                connect                 "Flute",                "outleft", 	"Reverberation",        "inleft"
                                connect                 "Flute",                "outright", "Reverberation",        "inright"
                                connect                 "FMModulatedChorusing", "outleft", 	"Reverberation",        "inleft"
                                connect                 "FMModulatedChorusing", "outright", "Reverberation",        "inright"
                                connect                 "FMModerateIndex",      "outleft", 	"Reverberation",        "inleft"
                                connect                 "FMModerateIndex",      "outright", "Reverberation",        "inright"
                                connect                 "FMModerateIndex2",     "outleft", 	"Reverberation",        "inleft"
                                connect                 "FMModerateIndex2",     "outright", "Reverberation",        "inright"
                                connect                 "FMWaterBell",          "outleft", 	"Reverberation",        "inleft"
                                connect                 "FMWaterBell",          "outright", "Reverberation",        "inright"
                                connect                 "Granular",             "outleft", 	"Reverberation",        "inleft"
                                connect                 "Granular",             "outright", "Reverberation",        "inright"
                                connect                 "Guitar",               "outleft", 	"Reverberation",        "inleft"
                                connect                 "Guitar",               "outright", "Reverberation",        "inright"
                                connect                 "Guitar2",              "outleft", 	"Reverberation",        "inleft"
                                connect                 "Guitar2",              "outright", "Reverberation",        "inright"
                                connect                 "Harpsichord",          "outleft", 	"Reverberation",        "inleft"
                                connect                 "Harpsichord",          "outright", "Reverberation",        "inright"
                                connect                 "HeavyMetalModel",      "outleft", 	"Reverberation",        "inleft"
                                connect                 "HeavyMetalModel",      "outright", "Reverberation",        "inright"
                                connect                 "Hypocycloid",          "outleft", 	"Reverberation",        "inleft"
                                connect                 "Hypocycloid",          "outright", "Reverberation",        "inright"
                                connect                 "KungModulatedFM",      "outleft", 	"Reverberation",        "inleft"
                                connect                 "KungModulatedFM",      "outright", "Reverberation",        "inright"
                                connect                 "ModerateFM",          	"outleft", 	"Reverberation",        "inleft"
                                connect                 "ModerateFM",          	"outright", "Reverberation",        "inright"
                                connect                 "ModulatedFM",          "outleft", 	"Reverberation",        "inleft"
                                connect                 "ModulatedFM",          "outright", "Reverberation",        "inright"
                                connect                 "Melody",               "outleft", 	"Reverberation",        "inleft"
                                connect                 "Melody",               "outright", "Reverberation",        "inright"
                                connect                 "PlainPluckedString",   "outleft", 	"Reverberation",        "inleft"
                                connect                 "PlainPluckedString",   "outright", "Reverberation",        "inright"
                                connect                 "PRCBeeThree",          "outleft", 	"Reverberation",        "inleft"
                                connect                 "PRCBeeThree",          "outright", "Reverberation",        "inright"
                                connect                 "PRCBeeThreeDelayed",   "outleft", 	"Reverberation",        "inleft"
                                connect                 "PRCBeeThreeDelayed",   "outright", "Reverberation",        "inright"
                                connect                 "PRCBowed",             "outleft", 	"Reverberation",        "inleft"
                                connect                 "PRCBowed",             "outright", "Reverberation",        "inright"
                                connect                 "Reverberation",        "outleft", 	"MasterOutput",           "inleft"
                                connect                 "Reverberation",        "outright", "MasterOutput",           "inright"
                                connect                 "StringPad",            "outleft", 	"Reverberation",        "inleft"
                                connect                 "StringPad",            "outright", "Reverberation",        "inright"
                                connect                 "ToneWheelOrgan",       "outleft", 	"Reverberation",        "inleft"
                                connect                 "ToneWheelOrgan",       "outright", "Reverberation",        "inright"
                                connect                 "TubularBellModel",     "outleft", 	"Reverberation",        "inleft"
                                connect                 "TubularBellModel",     "outright", "Reverberation",        "inright"
                                connect                 "WaveguideGuitar",      "outleft", 	"Reverberation",        "inleft"
                                connect                 "WaveguideGuitar",      "outright", "Reverberation",        "inright"
                                connect                 "Xing",                 "outleft", 	"Reverberation",        "inleft"
                                connect                 "Xing",                 "outright", "Reverberation",        "inright"
                                connect                 "ZakianFlute",          "outleft", 	"Reverberation",        "inleft"
                                connect                 "ZakianFlute",          "outright", "Reverberation",        "inright"
                                
                                alwayson                "Reverberation"
                                alwayson                "MasterOutput"

;; Original Instruments

giBanchoffKleinBottle_sine_1    ftgen                   0, 0, 65536, 10, 1
giBanchoffKleinBottle_cosine_2  ftgen                   0, 0, 65536, 11, 1

                                instr                   BanchoffKleinBottle
                                //////////////////////////////////////////////
                                // Original by Hans Mikelson.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) 
                                ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                ;   p1  p2     p3   p4    p5     p6   p7
                                ;       Start  Dur  Amp   Frqc   U    V  
                                ; i 4   32     6    6000  6.00   3    2
                                ; i 4   36     4    .     5.11   5.6  0.4
                                ; i 4   +      4    .     6.05   2    8.5
                                ; i 4   .      2    .     6.02   4    5
                                ; i 4   .      2    .     6.02   5    0.5
iHz                             =                       ifrequency
ifqc                            init                    iHz
ip4                             init                    iamplitude
iu                              init                    5 ; p6
iv                              init                    0.5 ; p7
irt2                            init                    sqrt(2)
aampenv                         linseg                  0, 0.02, ip4,  p3 - 0.04, ip4, 0.02, 0
isine                           =                       giBanchoffKleinBottle_sine_1
icosine                         =                       giBanchoffKleinBottle_cosine_2
                                ; Cosines
acosu                           oscili                  1, iu * ifqc, icosine
acosu2                          oscili                  1, iu * ifqc / 2, icosine
acosv                           oscili                  1, iv * ifqc, icosine
                                ; Sines
asinu                           oscili                  1, iu * ifqc, isine
asinu2                          oscili                  1, iu * ifqc / 2, isine
asinv                           oscili                  1, iv * ifqc, isine
                                ; Compute X and Y
ax                              =                       acosu * (acosu2 * (irt2 + acosv) + asinu2 * asinv * acosv)
ay                              =                       asinu * (acosu2 * (irt2 + acosv) + asinu2 * asinv * acosv)
                                ; Low frequency rotation in spherical coordinates z, phi, theta.
klfsinth                        oscili                  1, 4, isine
klfsinph                        oscili                  1, 1, isine
klfcosth                        oscili                  1, 4, icosine
klfcosph                        oscili                  1, 1, icosine
aox                             =                       -ax * klfsinth + ay * klfcosth
aoy                             =                       -ax * klfsinth * klfcosph - ay * klfsinth * klfcosph + klfsinph
aoutleft                        =                       aampenv * aox
aoutright                       =                       aampenv * aoy

                                outleta                 "outleft", aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin                                

                                instr                   BassModel
                                //////////////////////////////////////////////
                                // Original by Hans Mikelson.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) / 35
                                ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                ; p1  p2     p3   p4    p5     p6
                                ;     Start  Dur  Amp   Pitch  PluckDur
                                ; i2  128    4    1400  6.00   0.25
                                ; i2  +      2    1200  6.01   0.25
                                ; i2  .      4    1000  6.05   0.5
                                ; i2  .      2     500  6.04   1
                                ; i2  .      4    1000  6.03   0.5
                                ; i2  .      16   1000  6.00   0.5
iHz                             =                       ifrequency
ifqc                            =                       iHz
ip4                             =                       iamplitude
ip6                             =                       0.5
ipluck                          =                       1 / ifqc * ip6
kcount                          init                    0
adline                          init                    0
ablock2                         init                    0
ablock3                         init                    0
afiltr                          init                    0
afeedbk                         init                    0
koutenv                         linseg                  0, .01, 1, p3 - .11 , 1, .1 , 0 ; Output envelope
kfltenv                         linseg                  0, 1.5, 1, 1.5, 0 
                                ; This envelope loads the string with a triangle wave.
kenvstr                         linseg                  0, ipluck / 4, -ip4 / 2, ipluck / 2, ip4 / 2, ipluck / 4, 0, p3 - ipluck, 0
aenvstr                         =                       kenvstr
ainput                          tone                    aenvstr, 200
                                ; DC Blocker
ablock2                         =                       afeedbk - ablock3 + .99 * ablock2
ablock3                         =                       afeedbk
ablock                          =                       ablock2
                                ; Delay line with filtered feedback
adline                          delay                   ablock + ainput, 1 / ifqc - 15 / sr
afiltr                          tone                    adline, 400
                                ; Resonance of the body 
abody1                          reson                   afiltr, 110, 40
abody1                          =                       abody1 / 5000
abody2                          reson                   afiltr, 70, 20
abody2                          =                       abody2 / 50000
afeedbk                         =                       afiltr
aout                            =                       afeedbk
asignal                         =                       50 * koutenv * (aout + kfltenv * (abody1 + abody2))
iattack                         =                       0.005
isustain                        =                       p3
irelease                        =                       0.06
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
aoutleft, aoutright             pan2                    asignal * iamplitude * adeclick, i_pan

                                outleta                 "outleft", aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin                                

giChebyshevDrone_sinetable_1    ftgen                   0, 0, 65536, 10, 1, 0, .02

                                instr                   ChebyshevDrone
                                ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                ; By Michael Gogins.
                                ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ihertz                          =                       cpsmidinn(i_midikey)
iamp                            =                       ampdb(i_midivelocity) * 6
; Level correction
iamp                            =                       iamp * .5
idampingattack                  =                       .01
idampingrelease                 =                       .02
idampingsustain                 =                       p3
iduration                       =                       idampingattack + idampingsustain + idampingrelease
p3                              =                       iduration
iattack                         init                    p3 / 4.0
idecay                          init                    p3 / 4.0
isustain                        init                    p3 / 2.0
aenvelope                       transeg                 0.0, iattack / 2.0, 2.5, iamp / 2.0, iattack / 2.0, -2.5, iamp, isustain, 0.0, iamp, idecay / 2.0, 2.5, iamp / 2.0, idecay / 2.0, -2.5, 0.
isinetable                      =                       giChebyshevDrone_sinetable_1
asignal                         poscil3                 1, ihertz, isinetable
asignal                         chebyshevpoly           asignal, 0, gkChebyshevDroneCoefficient1, gkChebyshevDroneCoefficient2, gkChebyshevDroneCoefficient3, gkChebyshevDroneCoefficient4, gkChebyshevDroneCoefficient5, gkChebyshevDroneCoefficient6, gkChebyshevDroneCoefficient7, gkChebyshevDroneCoefficient8, gkChebyshevDroneCoefficient9, gkChebyshevDroneCoefficient10
adeclick                        linsegr                 0, idampingattack, 1, idampingsustain, 1, idampingrelease, 0
asignal                         =                       asignal * aenvelope
aoutleft, aoutright             pan2                    asignal * adeclick, i_pan

                                outleta                 "outleft", aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giChebyshevMelody_sine_1        ftgen                   0, 0, 65536, 10, 1
giChebyshevMelody_cook3_2       ftgen                   0, 0, 65536,    10,     1, .4, 0.2, 0.1, 0.1, .05
giChebyshevMelody_p6_3          ftgen                   0, 0, 65536,    -7,    -1, 150, 0.1, 110, 0, 252, 0

                                instr                   ChebyshevMelody
                                ///////////////////////////////////////////////////////
                                // Original by Jon Nelson.
                                // Adapted by Michael Gogins.
                                ///////////////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iHz                             =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 7.
iattack                         =                       .01
isustain                        =                       p3
irelease                        =                       .01
p3                              =                       iattack + isustain + irelease
adeclick                        linsegr                 0, iattack, 1, isustain, 1, irelease, 0
kHz                             =                       k(iHz)
idB                             =                       i_midivelocity
i1                              =                       iHz
k100                            randi                   1,0.05
isine                           =                       giChebyshevMelody_sine_1
k101                            poscil                  1, 5 + k100, isine
k102                            linseg                  0, .5, 1, p3, 1
k100                            =                       i1 + (k101 * k102)
; Envelope for driving oscillator.
ip3                             init                    3.0
; k1                            linenr                  0.5, ip3 * .3, ip3 * 2, 0.01
k1                              linseg                  0, ip3 * .3, .5, ip3 * 2, 0.01, isustain, 0.01, irelease, 0
; k2                            line                    1, p3, .5
k2                              linseg                  1.0, ip3, .5, isustain, .5, irelease, 0
k1                              =                       k2 * k1
; Amplitude envelope.
k10                             expseg                  0.0001, iattack, 1.0, isustain, 0.8, irelease, .0001
k10                             =                       (k10 - .0001)
; Power to partials.
k20                             linseg                  1.485, iattack, 1.5, (isustain + irelease), 1.485
; a1-3 are for cheby with p6=1-4
icook3                          =                       giChebyshevMelody_cook3_2
a1                              poscil                  k1, k100 - .25, icook3
; Tables a1 to fn13, others normalize,
ip6                             =                       giChebyshevMelody_p6_3
a2                              tablei                  a1, ip6, 1, .5
a3                              balance                 a2, a1
; Try other waveforms as well.
a4                              foscili                 1, k100 + .04, 1, 2.000, k20, isine
a5                              poscil                  1, k100, isine
a6                              =                       ((a3 * .1) + (a4 * .1) + (a5 * .8)) * k10
a7                              comb                    a6, .5, 1 / i1
a8                              =                       (a6 * .9) + (a7 * .1)
asignal        		            balance         	    a8, a1
asignal                         =                       asignal * iamplitude
aoutleft, aoutright		        pan2			        asignal * adeclick, i_pan

                                outleta                 "outleft", aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giDelayedPluckedString_sine_1   ftgen                   0, 0, 65536,    10,     1
giDelayedPluckedString_cosine_2 ftgen                   0, 0, 65536,    11,     1 

                                instr                   DelayedPluckedString
                                //////////////////////////////////////////////////////
                                // By Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 1000
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iattack                         =                       0.02
isustain                        =                       p3
irelease                        =                       0.15
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                  0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
ikeyin                          =                       i_midikey
ihertz                          =                       cpsmidinn(ikeyin)
; Detuning of strings by 4 cents each way.
idetune                         =                       4.0 / 1200.0
ihertzleft                      =                       cpsmidinn(ikeyin + idetune)
ihertzright                     =                       cpsmidinn(ikeyin - idetune)
iamplitude                      =                       ampdb(i_midivelocity)
isine                           =                       giDelayedPluckedString_sine_1
icosine                         =                       giDelayedPluckedString_cosine_2
igenleft                        =                       isine
igenright                       =                       icosine
kvibrato                        oscili                  1.0 / 120.0, 7.0, icosine
kexponential                    expseg                  1.0, p3 + iattack, 0.0001, irelease, 0.0001
aenvelope                       =                       (kexponential - 0.0001) * adeclick
ag                              pluck                   iamplitude, cpsmidinn(ikeyin + kvibrato), 200, igenleft, 1
agleft                          pluck                   iamplitude, ihertzleft, 200, igenleft, 1
agright                         pluck                   iamplitude, ihertzright, 200, igenright, 1
imsleft                         =                       0.2 * 1000
imsright                        =                       0.21 * 1000
adelayleft                      vdelay                  ag * aenvelope, imsleft, imsleft + 100
adelayright                     vdelay                  ag * aenvelope, imsright, imsright + 100
asignal                         =                       adeclick * (agleft + adelayleft + agright + adelayright)
                                ; Highpass filter to exclude speaker cone excursions.
asignal1                        butterhp                asignal, 32.0
asignal2                        balance                 asignal1, asignal
aoutleft, aoutright             pan2                    asignal2 * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giEnhancedFMBell_ffitch1_1      ftgen                   0, 0, 65536,     10,     1
giEnhancedFMBell_ffitch2_2      ftgen                   0, 0, 8193,     5,      1, 1024, 0.01
giEnhancedFMBell_ffitch3_3      ftgen                   0, 0, 8193,     5,      1, 1024, 0.001

                                instr                   EnhancedFMBell
                                //////////////////////////////////////////////////////
                                // Original by John ffitch.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iattack                         =                       0.005
isustain                        =                       p3
irelease                        =                       0.25
i_duraton                       =                       15; isustain + iattack + irelease
p3                              =                       i_duration
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
ifrequency                      =                       cpsmidinn(i_midikey)
; Normalize so iamplitude for p5 of 80 == ampdb(80).
iamplitude                      =                       ampdb(i_midivelocity) 
idur                            =                       50
iamp                            =                       iamplitude
iffitch1                        =                       giEnhancedFMBell_ffitch1_1
iffitch2                        =                       giEnhancedFMBell_ffitch2_2
iffitch3                        =                       giEnhancedFMBell_ffitch3_3
ifenv                           =                       iffitch2                       ; BELL SETTINGS:
ifdyn                           =                       iffitch3                       ; AMP AND INDEX ENV ARE EXPONENTIAL
ifq1                            =                       cpsmidinn(i_midikey) ;* 5              ; DECREASING, N1:N2 IS 5:7, imax=10
if1                             =                       iffitch1                               ; DURATION = 15 sec
ifq2                            =                       cpsmidinn(i_midikey) * 5/7
if2                             =                       iffitch1
imax                            =                       10
aenv                            oscili                  iamp, 1 / idur, ifenv           ; ENVELOPE
adyn                            oscili                  ifq2 * imax, 1 / idur, ifdyn    ; DYNAMIC
anoise                          rand                    50
amod                            oscili                  adyn + anoise, ifq2, if2        ; MODULATOR
acar                            oscili                  aenv, ifq1 + amod, if1          ; CARRIER
                                timout                  0.5, idur, noisend
knenv                           linsegr                 iamp, 0.2, iamp, 0.3, 0
anoise3                         rand                    knenv
anoise4                         butterbp                anoise3, iamp, 100
anoise5                         balance                 anoise4, anoise3
noisend:
arvb                            nreverb                 acar, 2, 0.1
aenvelope                       transeg                 1, idur, -3, 0
asignal                         =                       aenvelope * (acar + arvb) ;+ anoise5
aoutleft, aoutright             pan2                    asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giFenderRhodesModel_sine_1      ftgen                   0, 0, 65536,    10,     1
giFenderRhodesModel_cosine_2    ftgen                   0, 0, 65536,    11,     1 
giFenderRhodesModel_cookblank_3 ftgen                   0, 0, 65536,     10,     0 ; Blank wavetable for some Cook FM opcodes.

                                instr                   FenderRhodesModel
                                //////////////////////////////////////////////////////
                                // Original by Perry Cook.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iattack                         =                       0.01
isustain                        =                       p3
irelease                        =                       0.125
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
iindex                          =                       4
icrossfade                      =                       3
ivibedepth                      =                       0.2
iviberate                       =                       6
isine                           =                       giFenderRhodesModel_sine_1
icosine                         =                       giFenderRhodesModel_cosine_2
icookblank                      =                       giFenderRhodesModel_cookblank_3
ifn1                            =                       isine
ifn2                            =                       icosine
ifn3                            =                       isine
ifn4                            =                       icookblank
ivibefn                         =                       isine
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 6
asignal                         fmrhode                 iamplitude, ifrequency, iindex, icrossfade, ivibedepth, iviberate, ifn1, ifn2, ifn3, ifn4, ivibefn
aoutleft, aoutright		        pan2			        asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giFilteredSines_bergeman_1      ftgen                   0, 0, 65536,     10,     0.28, 1, 0.74, 0.66, 0.78, 0.48, 0.05, 0.33, 0.12, 0.08, 0.01, 0.54, 0.19, 0.08, 0.05, 0.16, 0.01, 0.11, 0.3, 0.02, 0.2 ; Bergeman f1

                                instr                   FilteredSines
                                //////////////////////////////////////////////////////
                                // Original by Michael Bergeman.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////////////
                                ; Original pfields
                                ; p1 p2 p3 p4 p5 p6 p7 p8 p9
                                ; ins st dur db func at dec freq1 freq2
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iattack                         =                       0.03
isustain                        =                       p3
irelease                        =                       0.52
p3                              =                       p3 + iattack + irelease
i_duration                      =                       p3
adeclick                        linsegr                  0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
ip4                             =                       i_midivelocity
idb                             =                       ampdb(i_midivelocity) * 4
ibergeman                       =                       giFilteredSines_bergeman_1
ip5                             =                       ibergeman
ip3                             =                       i_duration
ip6                             =                       i_duration * 0.25
ip7                             =                       i_duration * 0.75
ip8                             =                       cpsmidinn(i_midikey - 0.01)
ip9                             =                       cpsmidinn(i_midikey + 0.01)
isc                             =                       idb * 0.333
k1                              line                    40, p3, 800
k2                              line                    440, p3, 220
k3                              linen                   isc, ip6, p3, ip7
k4                              line                    800, ip3, 40
k5                              line                    220, ip3, 440
k6                              linen                   isc, ip6, ip3, ip7
k7                              linen                   1, ip6, ip3, ip7
a5                              oscili                  k3, ip8, ip5
a6                              oscili                  k3, ip8 * 0.999, ip5
a7                              oscili                  k3, ip8 * 1.001, ip5
a1                              =                       a5 + a6 + a7
a8                              oscili                  k6, ip9, ip5
a9                              oscili                  k6, ip9 * 0.999, ip5
a10                             oscili                  k6, ip9 * 1.001, ip5
a11                             =                       a8 + a9 + a10
a2                              butterbp                a1, k1, 40
a3                              butterbp                a2, k5, k2 * 0.8
a4                              balance                 a3, a1
a12                             butterbp                a11, k4, 40
a13                             butterbp                a12, k2, k5 * 0.8
a14                             balance                 a13, a11
a15                             reverb2                 a4, 5, 0.3
a16                             reverb2                 a4, 4, 0.2
                                ; Constant-power pan.
ipi                             =                       4.0 * taninv(1.0)
iradians                        =                       i_pan * ipi / 2.0
itheta                          =                       iradians / 2.0
; Translate angle in [-1, 1] to left and right gain factors.
irightgain                      =                       sqrt(2.0) / 2.0 * (cos(itheta) + sin(itheta))
ileftgain                       =                       sqrt(2.0) / 2.0 * (cos(itheta) - sin(itheta))
a17                             =                       (a15 + a4) * ileftgain * k7
a18                             =                       (a16 + a4) * irightgain * k7
aoutleft                        =                       a17 * adeclick
aoutright                       =                       a18 * adeclick

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giFlute_kellyflute_1            ftgen                   0, 0, 65536,     10,     1, 0.25, 0.1 ; Kelley flute.
giFlute_sine_2                  ftgen                   0, 0, 65536,    10,     1

                                instr                   Flute
                                //////////////////////////////////////////////////////
                                // Original by James Kelley.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 1000
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ikellyflute                     =                       giFlute_kellyflute_1
; Do some phasing.
icpsp1                          =                       cpsmidinn(i_midikey - 0.0002)
icpsp2                          =                       cpsmidinn(i_midikey + 0.0002)
ip6                             =                       ampdb(i_midivelocity)
iattack                         =                       0.04
isustain                        =                       p3
irelease                        =                       0.15
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                  0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
ip4                             =                       0
                                if                      (ip4 == int(ip4 / 2) * 2) goto initslurs
                                ihold
initslurs:
iatttm                          =                       0.09
idectm                          =                       0.1
isustm                          =                       p3 - iatttm - idectm
idec                            =                       ip6
ireinit                         =                       -1
                                if                      (ip4 > 1) goto checkafterslur
ilast                           =                       0
checkafterslur:
                                if                      (ip4 == 1 || ip4 == 3) goto doneslurs
idec                            =                       0
ireinit                         =                       0
; KONTROL
doneslurs:
                                if                      (isustm <= 0)   goto simpleenv
kamp                            linsegr                  ilast, iatttm, ip6, isustm, ip6, idectm, idec, 0, idec
                                goto                    doneenv
simpleenv:
kamp                            linsegr                  ilast, p3 / 2,ip6, p3 / 2, idec, 0, idec
doneenv:
ilast                           =                       ip6
; Some vibrato.
kvrandamp                       rand                    0.1
kvamp                           =                       (8 + p4) *.06 + kvrandamp
kvrandfreq                      rand                    1
kvfreq                          =                       5.5 + kvrandfreq
isine                           =                       giFlute_sine_2
kvbra                           oscili                  kvamp, kvfreq, isine, ireinit
kfreq1                          =                       icpsp1 + kvbra
kfreq2                          =                       icpsp2 + kvbra
; Noise for burst at beginning of note.
knseenv                         expon                   ip6 / 4, 0.2, 1
; AUDIO
anoise1                         rand                    knseenv
anoise                          tone                    anoise1, 200
a1                              oscili                  kamp, kfreq1, ikellyflute, ireinit
a2                              oscili                  kamp, kfreq2, ikellyflute, ireinit
a3                              =                       a1 + a2 + anoise
aoutleft, aoutright             pan2                    a3 * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giFMModerateIndex_sine_1        ftgen                   0, 0, 65536,    10,     1

                                instr                   FMModerateIndex
                                //////////////////////////////////////////////////////
                                // By Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 3
icarrier                        =                       1
iratio                          =                       1.25
ifmamplitude                    =                       8
index                           =                       5.4
iattack                         =                       0.01
isustain                        =                       p3
irelease                        =                       0.05
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
ifrequencyb                     =                       ifrequency * 1.003
icarrierb                       =                       icarrier * 1.004
kindenv                         transeg                 0, iattack, -4, 1,  isustain, -2, 0.125, irelease, -4, 0
kindex                          =                       kindenv * index * ifmamplitude
isine                           =                       giFMModerateIndex_sine_1
aouta                           foscili                 1, ifrequency, icarrier, iratio, index, isine
aoutb                           foscili                 1, ifrequencyb, icarrierb, iratio, index, isine
asignal                         =                       (aouta + aoutb) * kindenv
aoutleft, aoutright		        pan2			        asignal * iamplitude * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giFMModerateIndex2_sine_1       ftgen                   0, 0, 65536,    10,     1

                                instr                   FMModerateIndex2
                                //////////////////////////////////////////////////////
                                // By Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 3
icarrier                        =                       1
iratio                          =                       1
ifmamplitude                    =                       6
index                           =                       2.5
iattack                         =                       0.02
isustain                        =                       p3
irelease                        =                       0.05
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
ifrequencyb                     =                       ifrequency * 1.003
icarrierb                       =                       icarrier * 1.004
kindenv                         expseg                  0.000001, iattack, 1.0, isustain, 0.0125, irelease, 0.000001
kindex                          =                       kindenv * index * ifmamplitude - 0.000001
isine                           =                       giFMModerateIndex2_sine_1
aouta                           foscili                 1, ifrequency, icarrier, iratio, index, isine
aoutb                           foscili                 1, ifrequencyb, icarrierb, iratio, index, isine
asignal                         =                       (aouta + aoutb) * kindenv
aoutleft, aoutright		        pan2			        asignal * iamplitude * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giFMModulatedChorusing_ln_1     ftgen                   0, 0, 65536,     -12,    20.0 ; Unscaled ln(I(x)) from 0 to 20.0.
giFMModulatedChorusing_cosine_2 ftgen                   0, 0, 65536,    11,     1 ; Cosine wave. Get that noise down on the most widely used table!
giFMModulatedChorusing_sine_3   ftgen                   2, 0, 65536,    10,     1

                                instr                   FMModulatedChorusing
                                //////////////////////////////////////////////
                                // Original by Thomas Kung.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iattack                         =                       0.333333
irelease                        =                       0.1
isustain                        =                       p3
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                  0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
iamplitude                      =                       ampdb(i_midikey) / 1200
; Level correction
iamplitude                      =                       iamplitude * .5
ip6                             =                       0.3
ip7                             =                       2.2
                                ; shift it.
ishift                          =                       4.0 / 12000
                                ; convert parameter 5 to cps.
ipch                            =                       cpsmidinn(i_midikey)
                                ; convert parameter 5 to oct.
ioct                            =                       i_midikey
kadsr                           linen                   1.0, iattack, irelease, 0.01
kmodi                           linseg                  0, iattack, 5, isustain, 2, irelease, 0
                                ; r moves from ip6 to ip7 in p3 secs.
kmodr                           linseg                  ip6, p3, ip7
a1                              =                       kmodi * (kmodr - 1 / kmodr) / 2
                                ; a1*2 is argument normalized from 0-1.
a1ndx                           =                       abs(a1 * 2 / 20)
a2                              =                       kmodi * (kmodr + 1 / kmodr) / 2
                                ; Look up table is in f43, normalized index.
iln                             =                       giFMModulatedChorusing_ln_1
a3                              tablei                  a1ndx, iln, 1
icosine                         =                       giFMModulatedChorusing_cosine_2
ao1                             oscili                  a1, ipch, icosine
a4                              =                       exp(-0.5 * a3 + ao1)
                                ; Cosine
ao2                             oscili                  a2 * ipch, ipch, icosine
isine                           =                       giFMModulatedChorusing_sine_3
                                ; Final output left
aoutl                           oscili                  1 * kadsr * a4, ao2 + cpsmidinn(ioct + ishift), isine
                                ; Final output right
aoutr                           oscili                  1 * kadsr * a4, ao2 + cpsmidinn(ioct - ishift), isine
asignal                         =                       aoutl + aoutr
asignal                         =                       asignal * iamplitude
aoutleft, aoutright		        pan2			        asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giFMWaterBell_cosine_1          ftgen                   0, 0, 65536, 11, 1

                                instr                   FMWaterBell
                                //////////////////////////////////////////////
                                // Original by Steven Yi.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
                                if ((p2 > ((60 * 3) + 15)) && (p2 < ((60 * 3) + 45))) then
                                prints                  "Adjusting FMWaterBell loudness here..."
p5                              =                       p5 + 8
                                endif
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       5
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ipch                            =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 2.0
ipch2                           =                       ipch
kpchline 	                    line                    ipch, i_duration, ipch2
iamp 	                        =                       2
ienvType	                    =                       2
kenv 	                        init 	                0
                                if ienvType == 0 kgoto env0  ; adsr
                                if ienvType == 1 kgoto env1  ; pyramid
                                if ienvType == 2 kgoto env2  ; ramp
env0:
kenv	                        adsr	                .3, .2, .9, .5
                                kgoto                   endEnvelope
env1:
kenv 	                        linseg	                0, i_duration * .5, 1, i_duration * .5, 0
                                kgoto                   endEnvelope
env2:
kenv	                        linseg 	                0, i_duration - .1, 1, .1, 0	
kgoto                           endEnvelope
endEnvelope:
kc1                             =                       5
kc2                             =                       5
kvdepth                         =                       0.005
kvrate                          =                       6
icosine                         =                       giFMWaterBell_cosine_1
ifn1                            =                       icosine
ifn2                            =                       icosine
ifn3                            =                       icosine
ifn4                            =                       icosine
ivfn                            =                       icosine
asignal                         fmbell	                iamp, kpchline, kc1, kc2, kvdepth, kvrate, ifn1, ifn2, ifn3, ifn4, ivfn
iattack                         =                       0.003
isustain                        =                       p3
irelease                        =                       0.06
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
aoutleft, aoutright             pan2                    iamplitude * asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giGranularGrainTable            ftgen                   0, 0, 65536,    10,     1, .3, .1, 0, .2, .02, 0, .1, .04
giGranularWindowTable           ftgen                   0, 0, 65536,    10,     1, 0, .5, 0, .33, 0, .25, 0, .2, 0, .167

                                instr                   Granular
                                //////////////////////////////////////////////
                                // Original by Hans Mikelson.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) / 175
; Level correction
iamplitude                      =                       iamplitude * 1.5
                                ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                ; f1  0 65536 1 "hahaha.aif" 0 4 0
                                ; f2  0 1024  7 0 224 1 800 0
                                ; f3  0 8192  7 1 8192 -1
                                ; f4  0 1024  7 0 512 1 512 0
                                ; f5  0 1024 10 1 .3 .1 0 .2 .02 0 .1 .04
                                ; f6  0 1024 10 1 0 .5 0 .33 0 .25 0 .2 0 .167
                                ; a0 14 50
                                ; p1   p2     p3    p4    p5    p6     p7      p8      p9    p10
                                ;      Start  Dur  Amp    Freq  GrTab  WinTab  FqcRng  Dens  Fade
                                ; i1   0.0    6.5  700    9.00  5      4       .210    200   1.8
                                ; i1   3.2    3.5  800    7.08  .      4       .042    100   0.8
                                ; i1   5.1    5.2  600    7.10  .      4       .032    100   0.9
                                ; i1   7.2    6.6  900    8.03  .      4       .021    150   1.6
                                ; i1  21.3    4.5  1000   9.00  .      4       .031    150   1.2
                                ; i1  26.5   13.5  1100   6.09  .      4       .121    150   1.5
                                ; i1  30.7    9.3  900    8.05  .      4       .014    150   2.5
                                ; i1  34.2    8.8  700   10.02  .      4       .14     150   1.6
igrtab                          =                       giGranularGrainTable
iwintab                         =                       giGranularWindowTable
iHz                             =                       ifrequency
ip4                             =                       iamplitude
ip5                             =                       iHz
ip6                             =                       igrtab
ip7                             =                       iwintab
ip8                             =                       0.033
ip9                             =                       150
ip10                            =                       1.6
idur                            =                       p3
iamp                            =                       iamplitude ; p4
ifqc                            =                       iHz ; cpspch(p5)
igrtab                          =                       ip6
iwintab                         =                       ip7
ifrng                           =                       ip8
idens                           =                       ip9
ifade                           =                       ip10
igdur                           =                       0.2
kamp                            linseg                  0, ifade, 1, idur - 2 * ifade, 1, ifade, 0
;                                                       Amp   Fqc    Dense  AmpOff PitchOff      GrDur  GrTable   WinTable  MaxGrDur
aoutl                           grain                   ip4,  ifqc,  idens, 100,   ifqc * ifrng, igdur, igrtab,   iwintab,  5
aoutr                           grain                   ip4,  ifqc,  idens, 100,   ifqc * ifrng, igdur, igrtab,   iwintab,  5
aoutleft                        =                       aoutl * kamp * iamplitude
aoutright                       =                       aoutr * kamp * iamplitude

                                outleta                 "outleft", aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

                                instr                   Guitar
                                //////////////////////////////////////////////
                                // By Michael Gogins.
                                //////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
while i_midikey > 80 do
    i_midikey = i_midikey - 12
od
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) / 8.0
iattack                         =                       0.01
isustain                        =                       p3
irelease                        =                       0.05
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
iamplitude                      =                       ampdb(p5) * 20
kamp                            linsegr                 0.0, iattack, iamplitude, isustain, iamplitude, irelease, 0.0
asigcomp                        pluck                   1, 440, 440, 0, 1
asig                            pluck                   1, ifrequency, ifrequency, 0, 1
af1                             reson                   asig, 110, 80
af2                             reson                   asig, 220, 100
af3                             reson                   asig, 440, 80
aout                            balance                 0.6 * af1+ af2 + 0.6 * af3 + 0.4 * asig, asigcomp
kexp                            expseg                  1.0, iattack, 2.0, isustain, 1.0, irelease, 1.0
kenv                            =                       kexp - 1.0
asignal                         =                       aout * kenv * kamp
aoutleft, aoutright             pan2                    asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

                                instr                   Guitar2
                                //////////////////////////////////////////////////////
                                // By Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
while i_midikey > 89 do
    i_midikey = i_midikey - 12
od
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 12
iattack                         =                       0.01
isustain                        =                       p3
irelease                        =                       0.05
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
kamp                            linsegr                  0.0, iattack, 1, isustain, 1, irelease, 0.0
asigcomp                        pluck                   kamp, 440, 440, 0, 1
asig                            pluck                   kamp, ifrequency, ifrequency, 0, 1
af1                             reson                   asig, 110, 80
af2                             reson                   asig, 220, 100
af3                             reson                   asig, 440, 80
aout                            balance                 0.6 * af1+ af2 + 0.6 * af3 + 0.4 * asig, asigcomp
kexp                            expseg                  1.0, iattack, 2.0, isustain, 1.0, irelease, 1.0
kenv                            =                       kexp - 1.0
asignal                         =                       aout * kenv
asignal                         dcblock                 asignal
aoutleft, aoutright		        pan2			        asignal * iamplitude * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giHarpsichord_harptable_1       ftgen                   0, 0, 65536,  7, -1, 1024, 1, 1024, -1

                                instr 			        Harpsichord
                                //////////////////////////////////////////////
                                // Original by James Kelley.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
insno           		        =                       p1
itime           		        =                       p2
iduration       		        =                       p3
ikey            		        =                       p4
ivelocity                       =                       p5
iphase                          =                       p6
ipan                            =                       p7
idepth                          =                       p8
iheight                         =                       p9
ipcs                            =                       p10
ihomogeneity                    =                       p11
gkHarpsichordGain               =                       .25
gkHarpsichordPan                =                       .5
iattack                         =                       .005
isustain                        =                       p3
irelease                        =                       .3
p3                              =                       iattack + isustain + irelease
iHz                             =                       cpsmidinn(ikey)
kHz                             =                       k(iHz)
iamplitude                      =                       ampdb(ivelocity) * 36
aenvelope               	    transeg                 1.0, 20.0, -10.0, 0.05
apluck                  	    pluck                   1, kHz, iHz, 0, 1
iharptable                      =                       giHarpsichord_harptable_1
aharp                   	    poscil                  1, kHz, iharptable
aharp2                  	    balance                 apluck, aharp
asignal			                =                       (apluck + aharp2) * iamplitude * aenvelope * gkHarpsichordGain
adeclick                        linsegr                 0, iattack, 1, isustain, 1, irelease, 0
aoutleft, aoutright             pan2                    asignal * adeclick, ipan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giHeavyMetalModel_sine_1        ftgen                   0, 0, 65536,    10,     1
giHeavyMetalModel_cosine_2      ftgen                   0, 0, 65536,    11,     1 ; Cosine wave. Get that noise down on the most widely used table!
giHeavyMetalModel_exponentialrise_3 ftgen                   0, 0, 65536,     5,      0.001, 513, 1 ; Exponential rise.
giHeavyMetalModel_thirteen_4    ftgen                   0, 0, 65536,     9,      1, 0.3, 0

                                instr                   HeavyMetalModel
                                //////////////////////////////////////////////
                                // Original by Perry Cook.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
                                ;pset                    0, 0, 1000
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iattack                         =                       0.01
idecay                          =                       2.0
isustain                        =                       i_duration
irelease                        =                       0.125
p3                              =                       iattack + iattack + idecay + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, idecay + isustain, 1.0, irelease, 0.0
iindex                          =                       1
icrossfade                      =                       3
ivibedepth                      =                       0.02
iviberate                       =                       4.8
isine                           =                       giHeavyMetalModel_sine_1
icosine                         =                       giHeavyMetalModel_cosine_2
iexponentialrise                =                       giHeavyMetalModel_exponentialrise_3
ithirteen                       =                       giHeavyMetalModel_thirteen_4
ifn1                            =                       isine
ifn2                            =                       iexponentialrise
ifn3                            =                       ithirteen
ifn4                            =                       isine
ivibefn                         =                       icosine
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 48.0
adecay                          transeg                 0.0, iattack, 4, 1.0, idecay + isustain, -4, 0.1, irelease, -4, 0.0
asignal                         fmmetal                 0.1, ifrequency, iindex, icrossfade, ivibedepth, iviberate, ifn1, ifn2, ifn3, ifn4, ivibefn
asignal                         =                       asignal * iamplitude
aoutleft, aoutright             pan2                    asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giHypocycloid_sine_1            ftgen                   0, 0, 65536, 10, 1
giHypocycloid_cosine_2          ftgen                   0, 0, 65536, 11, 1

                                instr                   Hypocycloid
                                //////////////////////////////////////////////
                                // Original by Hans Mikelson.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
                                ; This set of parametric equations defines the path traced by
                                ; a point on a circle of radius B rotating inside a circle of
                                ; radius A.
                                ;   p1  p2     p3   p4    p5     p6   p7   p8
                                ;       Start  Dur  Amp   Frqc   A    B    Hole
                                ; i 3   16     6    8000  8.00  10    2    1
                                ; i 3   20     4    .     7.11   5.6  0.4  0.8
                                ; i 3   +      4    .     8.05   2    8.5  0.7
                                ; i 3   .      2    .     8.02   4    5    0.6
                                ; i 3   .      2    .     8.02   5    0.5  1.2
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 4
iHz                             =                       ifrequency
ifqc                            init                    iHz        
ip4                             init                    iamplitude
ifqci                           init                    iHz
ia                              =                       0.6 ; p6
ib                              =                       0.4 ; p7
ihole                           =                       0.8 ; p8
iscale                          =                       (ia < ib ? 1 / ib : 1 / ia)
kampenv                         linseg                  0, .1, ip4 * iscale, p3 - .2, ip4 * iscale, .1, 0
kptchenv                        linseg                  ifqci, .2 * p3, ifqc, .8 * p3, ifqc
kvibenv                         linseg                  0, .5, 0, .2, 1, .2, 1
isine                           =                       giHypocycloid_sine_1
icosine                         =                       giHypocycloid_cosine_2
kvibr                           oscili                  20, 8, icosine
kfqc                            =                       kptchenv+kvibr*kvibenv
                                ; Sine and Cosine
acos1                           oscili                  ia - ib, kfqc, icosine
acos2                           oscili                  ib * ihole, (ia - ib) / ib * kfqc, icosine
ax                              =                       acos1 + acos2
asin1                           oscili                  ia-ib, kfqc, isine
asin2                           oscili                  ib, (ia - ib) / ib * kfqc, isine
ay                              =                       asin1 - asin2
aoutleft                        =                       kampenv * ax
aoutright                       =                       kampenv * ay
                                ; Constant-power pan.
ipi                             =                       4.0 * taninv(1.0)
iradians                        =                       i_pan * ipi / 2.0
itheta                          =                       iradians / 2.0
                                ; Translate angle in [-1, 1] to left and right gain factors.
irightgain                      =                       sqrt(2.0) / 2.0 * (cos(itheta) + sin(itheta))
ileftgain                       =                       sqrt(2.0) / 2.0 * (cos(itheta) - sin(itheta))
aoutleft                        =                       aoutleft * ileftgain
aoutright                       =                       aoutright * irightgain

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giModerateFM_sinetable_1        ftgen                   0, 0, 65536, 10, 1, 0, .02

                                instr			        ModerateFM
                                //////////////////////////////////////////////
                                // By Michael Gogins.
                                //////////////////////////////////////////////
                                ;pset                    0, 0, 1000
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iattack			                =			            0.002
isustain		                =			            p3
idecay				            =			            8
irelease		                =			            0.05
iHz                             =                       cpsmidinn(i_midikey)
idB                             =                       i_midivelocity
iamplitude                      =                       ampdb(idB) * 4.0
icarrier                	    =                       1
imodulator              	    =                       0.5
ifmamplitude            	    =                       0.25
index                   	    =                       .5
ifrequencyb             	    =                       iHz * 1.003
icarrierb               	    =                       icarrier * 1.004
aindenv                 	    transeg                 0.0, iattack, -11.0, 1.0, idecay, -7.0, 0.025, isustain, 0.0, 0.025, irelease, -7.0, 0.0
aindex                  	    =                       aindenv * index * ifmamplitude
isinetable                      =                       giModerateFM_sinetable_1
; ares                  	    foscili                 xamp, kcps, xcar, xmod, kndx, ifn [, iphs]
aouta                   	    foscili                 1.0, iHz, icarrier, imodulator, index / 4., isinetable
aoutb                   	    foscili                 1.0, ifrequencyb, icarrierb, imodulator, index, isinetable
; Plus amplitude correction.
asignal               		    =                       (aouta + aoutb) * aindenv
adeclick                        linsegr                 0, iattack, 1, isustain, 1, irelease, 0
asignal                         =                       asignal * iamplitude
aoutleft, aoutright             pan2                    asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giModulatedFM_ln_1              ftgen                   0, 0, 65536, -12, 20.0
giModulatedFM_cosine_2          ftgen                   0, 0, 65536, 11, 1
giModulatedFM_sine_3            ftgen                   0, 0, 65536, 10, 1

                                instr 			        ModulatedFM
                                //////////////////////////////////////////////
                                // Original by Thomas Kung.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
                                ;pset                    0, 0, 1000
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iattack                         =                       .25
isustain                        =                       p3
irelease                        =                       .33333333
p3                              =                       iattack + isustain + irelease
iHz                             =                       cpsmidinn(i_midikey)
kHz                             =                       k(iHz)
idB                             =                       i_midivelocity
iamplitude                      =                       ampdb(i_midivelocity)
adeclick                        linsegr                 0, iattack, 1, isustain, 1, irelease, 0
ip6                     	    =                       0.3
ip7                     	    =                       2.2
ishift      		    	    =           		    4.0 / 12000.0
kpch       		                =           		    kHz
koct        		    	    =           		    octcps(kHz)
aadsr                   	    linen                   1.0, iattack, irelease, 0.01
amodi                   	    linseg                  0, iattack, 5, p3, 2, irelease, 0
; r moves from ip6 to ip7 in p3 secs.
amodr                   	    linseg                  ip6, p3, ip7
a1                      	    =                       amodi * (amodr - 1 / amodr) / 2
; a1*2 is argument normalized from 0-1.
a1ndx                   	    =                       abs(a1 * 2 / 20)
a2                      	    =                       amodi * (amodr + 1 / amodr) / 2
; Unscaled ln(I(x)) from 0 to 20.0.
iln                             =                       giModulatedFM_ln_1
a3                      	    tablei                  a1ndx, iln, 1
icosine                         =                       giModulatedFM_cosine_2
ao1                     	    poscil                  a1, kpch, icosine
a4                      	    =                       exp(-0.5 * a3 + ao1)
; Cosine
ao2                     	    poscil                  a2 * kpch, kpch, icosine
isine                           =                       giModulatedFM_sine_3
; Final output left
aleft                   	    poscil                  a4, ao2 + cpsoct(koct + ishift), isine
; Final output right
aright                  	    poscil                  a4, ao2 + cpsoct(koct - ishift), isine
asignal                         =                       (aleft + aright) * iamplitude
aoutleft, aoutright             pan2                    asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

                                instr                   PlainPluckedString
                                //////////////////////////////////////////////////////
                                // By Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
iattack                         =                       0.002
isustain                        =                       p3
irelease                        =                       0.075
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 6
aenvelope                       transeg                 0, iattack, -4, iamplitude,  isustain, -4, iamplitude / 10.0, irelease, -4, 0
asignal1                        pluck                   1, ifrequency, ifrequency * 1.002, 0, 1
asignal2                        pluck                   1, ifrequency * 1.003, ifrequency, 0, 1
asignal                         =                       (asignal1 + asignal2) * aenvelope
aoutleft, aoutright		        pan2			        asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giStringPad_wave_1              ftgen                   0, 0, 65536,    10,     1, 0.5, 0.33, 0.25,  0.0, 0.1,  0.1, 0.1

                                instr                   StringPad
                                //////////////////////////////////////////////
                                // Original by Anthony Kozar.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
                                ; String-pad borrowed from the piece "Dorian Gray",
                                ; http://akozar.spymac.net/music/ Modified to fit my needs
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ihz                             =                       cpsmidinn(i_midikey)
iamp                            =                       ampdb(i_midivelocity) * 3
idb                             =                       i_midivelocity
ipos                            =                       i_pan
                                ; Slow attack and release
akctrl                           linsegr                     0, i_duration * 0.5, iamp, i_duration *.5, 0
                                ; Slight chorus effect
iwave                           =                       giStringPad_wave_1
afund                           oscili                      akctrl, ihz,        iwave       ; audio oscillator
acel1                           oscili                      akctrl, ihz - 0.1,  iwave       ; audio oscillator - flat
acel2                           oscili                      akctrl, ihz + 0.1,  iwave       ; audio oscillator - sharp
asig                            =                           afund + acel1 + acel2
                                ; Cut-off high frequencies depending on midi-velocity
                                ; (larger velocity implies more brighter sound)
asignal                         butterlp                asig, (i_midivelocity - 60) * 40 + 900
iattack                         =                       0.005
isustain                        =                       p3
irelease                        =                       0.06
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
aoutleft, aoutright             pan2                    asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giToneWheelOrgan_tonewheel1_1   ftgen                   0, 0, 65536,     10,     1, 0.02, 0.01
giToneWheelOrgan_tonewheel2_2   ftgen                   0, 0, 65536,     10,     1, 0,    0.2, 0, 0.1, 0, 0.05, 0, 0.02
giToneWheelOrgan_tonewheel3_3   ftgen                   0, 0, 65536,     7,      0, 110, 0, 18, 1, 18, 0, 110, 0
giToneWheelOrgan_tonewheel4_4   ftgen                   0, 0, 65536,     7,      0, 80, 0.2, 16, 1, 64, 1, 16, 0.2, 80, 0
giToneWheelOrgan_tonewheel5_5   ftgen                   0, 0, 65536,     8,      -.8, 336, -.78,  800, -.7, 5920, 0.7,  800, 0.78, 336, 0.8
giToneWheelOrgan_tonewheel6_6   ftgen                   0, 0, 65536,     8,       -.8, 336, -.76, 3000, -.7, 1520, 0.7, 3000, 0.76, 336, 0.8
giToneWheelOrgan_cosine_7       ftgen                   0, 0, 65536, 11, 1

                                instr                   ToneWheelOrgan
                                //////////////////////////////////////////////
                                // Original by Hans Mikelson.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
                                ;pset                    0, 0, 1000
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) / 8.0
iattack                         =                       0.02
isustain                        =                       i_duration
irelease                        =                       0.1
i_duration                      =                       iattack + isustain + irelease
p3                              =                       i_duration
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
                                ; Rotor Tables
itonewheel1                     =                       giToneWheelOrgan_tonewheel1_1
itonewheel2                     =                       giToneWheelOrgan_tonewheel2_2
                                ; Rotating Speaker Filter Envelopes
itonewheel3                     =                       giToneWheelOrgan_tonewheel3_3
itonewheel4                     =                       giToneWheelOrgan_tonewheel4_4
                                ; Distortion Tables
itonewheel5                     =                       giToneWheelOrgan_tonewheel5_5
itonewheel6                     =                       giToneWheelOrgan_tonewheel6_6
icosine                         =                       giToneWheelOrgan_cosine_7
iphase                          =                       p2
ikey                            =                       12 * int(i_midikey - 6) + 100 * (i_midikey - 6)
ifqc                            =                       ifrequency
                                ; The lower tone wheels have increased odd harmonic content.
iwheel1                         =                       ((ikey - 12) > 12 ? itonewheel1 : itonewheel2)
iwheel2                         =                       ((ikey +  7) > 12 ? itonewheel1 : itonewheel2)
iwheel3                         =                        (ikey       > 12 ? itonewheel1 : itonewheel2)
iwheel4                         =                       icosine
;   insno Start Dur   Amp   Pitch  SubFund  Sub3rd  Fund  2nd  3rd  4th  5th  6th  8th
; i 1     0     6     200   8.04   8        8       8     8    3    2    1    0    4
asubfund                        oscili                  8, 0.5    * ifqc,  iwheel1, iphase / (ikey - 12)
asub3rd                         oscili                  8, 1.4983 * ifqc,  iwheel2, iphase / (ikey +  7)
afund                           oscili                  8,          ifqc,  iwheel3, iphase /  ikey
a2nd                            oscili                  8, 2      * ifqc,  iwheel4, iphase / (ikey + 12)
a3rd                            oscili                  3, 2.9966 * ifqc,  iwheel4, iphase / (ikey + 19)
a4th                            oscili                  2, 4      * ifqc,  iwheel4, iphase / (ikey + 24)
a5th                            oscili                  1, 5.0397 * ifqc,  iwheel4, iphase / (ikey + 28)
a6th                            oscili                  0, 5.9932 * ifqc,  iwheel4, iphase / (ikey + 31)
a8th                            oscili                  4, 8      * ifqc,  iwheel4, iphase / (ikey + 36)
asignal                         =                       iamplitude * (asubfund + asub3rd + afund + a2nd + a3rd + a4th + a5th + a6th + a8th)
aoutleft, aoutright             pan2                    asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giTubularBellModel_sine_1       ftgen                   0, 0, 65536,    10,     1
giTubularBellModel_cosine_2     ftgen                   0, 0, 65536,    11,     1 ; Cosine wave. Get that noise down on the most widely used table!
giTubularBellModel_cook3_3      ftgen                   0, 0, 65536,    10,     1, 0.4, 0.2, 0.1, 0.1, 0.05

                                instr                   TubularBellModel
                                //////////////////////////////////////////////////////
                                // Original by Perry Cook.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 4
iattack                         =                       0.003
isustain                        =                       p3
irelease                        =                       0.125
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
iindex                          =                       1
icrossfade                      =                       2
ivibedepth                      =                       0.2
iviberate                       =                       6
isine                           =                       giTubularBellModel_sine_1
icosine                         =                       giTubularBellModel_cosine_2
icook3                          =                       giTubularBellModel_cook3_3
ifn1                            =                       isine
ifn2                            =                       icook3
ifn3                            =                       isine
ifn4                            =                       isine
ivibefn                         =                       icosine
asignal                         fmbell                  1.0, ifrequency, iindex, icrossfade, ivibedepth, iviberate, ifn1, ifn2, ifn3, ifn4, ivibefn
aoutleft, aoutright		        pan2	                asignal * iamplitude * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giWaveguideGuitar_cosine_1      ftgen                   0, 0, 65536,    11,     1.0                        

                                instr                   WaveguideGuitar
                                //////////////////////////////////////////////////////
                                // Original by Jeff Livingston.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////////////
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) / 16
iHz                             =                       ifrequency
                                ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                ; The model takes pluck position, and pickup position (in % of string length), and generates
                                ; a pluck excitation signal, representing the string displacement.  The pluck consists 
                                ; of a forward and backward traveling displacement wave, which are recirculated thru two 
                                ; separate delay lines, to simulate the one dimensional string waveguide, with 
                                ; fixed ends.
                                ;
                                ; Losses due to internal friction of the string, and with air, as well as
                                ; losses due to the mechanical impedance of the string terminations are simulated by 
                                ; low pass filtering the signal inside the feedback loops.
                                ; Delay line outputs at the bridge termination are summed and fed into an IIR filter
                                ; modeled to simulate the lowest two vibrational modes (resonances) of the guitar body.
                                ; The theory implies that force due to string displacement, which is equivalent to 
                                ; displacement velocity times bridge mechanical impedance, is the input to the guitar
                                ; body resonator model. Here we have modified the transfer fuction representing the bridge
                                ; mech impedance, to become the string displacement to bridge input force transfer function.
                                ; The output of the resulting filter represents the displacement of the guitar's top plate,
                                ; and sound hole, since thier respective displacement with be propotional to input force.
                                ; (based on a simplified model, viewing the top plate as a force driven spring).
                                ;
                                ; The effects of pluck hardness, and contact with frets during pluck release,
                                ; have been modeled by injecting noise into the initial pluck, proportional to initial 
                                ; string displacement.
                                ;
                                ; Note on pluck shape: Starting with a triangular displacment, I found a decent sounding
                                ; initial pluck shape after some trial and error.  This pluck shape, which is a linear
                                ; ramp, with steep fall off, doesn't necessarily agree with the pluck string models I've 
                                ; studied.  I found that initial pluck shape significantly affects the realism of the 
                                ; sound output, but I the treatment of this topic in musical acoustics literature seems
                                ; rather limited as far as I've encountered.  
                                ;
                                ; Original pfields
                                ; p1     p2   p3    p4    p5    p6      p7      p8       p9        p10         p11    p12   p13
                                ; in     st   dur   amp   pch   plklen  fbfac	pkupPos	 pluckPos  brightness  vibf   vibd  vibdel
                                ; i01.2	 0.5  0.75  5000  7.11	.85     0.9975	.0	    .25	       1	       0	  0	 0
ip4                             init                    iamplitude
ip6                             init                    0.85
ip7                             init                    0.9975
ip8                             init                    0
ip9                             init                    0.25
ip10                            init                    1.0
ip11                            init                    0.001
ip12                            init                    0.0
ip13                            init                    0.0
afwav                           init                    0
abkwav                          init                    0
abkdout                         init                    0
afwdout                         init                    0 
iEstr	                        init                    1.0 / cpspch(6.04)
ifqc                            init                    iHz ; cpspch(p5)
                                ; note:delay time=2x length of string (time to traverse it)
idlt                            init                    1.0 / ifqc		
ipluck                          =                       0.5 * idlt * ip6 * ifqc / cpspch(8.02)
ifbfac = ip7  			        ; feedback factor
                                ; (exponentialy scaled) additive noise to add hi freq content
ibrightness                     =                       ip10 * exp(ip6 * log(2)) / 2 
ivibRate                        =                       ip11	
ivibDepth                       pow                     2, ip12 / 12
                                ; vibrato depth, +,- ivibDepth semitones
ivibDepth                       =                       idlt - 1.0 / (ivibDepth * ifqc)	
                                ; vibrato start delay (secs)
ivibStDly                       =                       ip13 
                                ; termination impedance model
                                ; cutoff freq of LPF due to mech. impedance at the nut (2kHz-10kHz)
if0                             =                       10000 
                                ; damping parameter of nut impedance
iA0                             =                       ip7  
ialpha                          =                       cos(2 * 3.14159265 * if0 * 1 / sr)
                                ; FIR LPF model of nut impedance,  H(z)=a0+a1z^-1+a0z^-2
ia0                             =                       0.3 * iA0 / (2 * (1 - ialpha))
ia1                             =                       iA0 - 2 * ia0
                                ; NOTE each filter pass adds a sampling period delay,so subtract 1/sr from tap time to compensate
                                ; determine (in crude fashion) which string is being played
                                ; icurStr = (ifqc > cpspch(6.04) ? 2 : 1)
                                ; icurStr = (ifqc > cpspch(6.09) ? 3 : icurStr)
                                ; icurStr = (ifqc > cpspch(7.02) ? 4 : icurStr)
                                ; icurStr = (ifqc > cpspch(7.07) ? 5 : icurStr)
                                ; icurStr = (ifqc > cpspch(7.11) ? 6 : icurStr)
ipupos                          =                       ip8 * idlt / 2 ; pick up position (in % of low E string length)
ippos                           =                       ip9 * idlt / 2 ; pluck position (in % of low E string length)
isegF                           =                       1 / sr
isegF2                          =                       ipluck
iplkdelF                        =                       (ipluck / 2 > ippos ? 0 : ippos - ipluck / 2)
isegB                           =                       1 / sr
isegB2                          =                       ipluck
iplkdelB                        =                       (ipluck / 2 > idlt / 2 - ippos ? 0 : idlt / 2 - ippos - ipluck / 2)
                                ; EXCITATION SIGNAL GENERATION
                                ; the two excitation signals are fed into the fwd delay represent the 1st and 2nd 
                                ; reflections off of the left boundary, and two accelerations fed into the bkwd delay 
                                ; represent the the 1st and 2nd reflections off of the right boundary.
                                ; Likewise for the backward traveling acceleration waves, only they encouter the 
                                ; terminations in the opposite order.
ipw                             =                       1
ipamp                           =                       ip4 * ipluck ; 4 / ipluck
aenvstrf                        linseg                  0, isegF, -ipamp / 2, isegF2, 0
adel1	                        delayr                  (idlt > 0) ? idlt : 0.01
                                ; initial forward traveling wave (pluck to bridge)
aenvstrf1                       deltapi                 iplkdelF        
                                ; first forward traveling reflection (nut to bridge) 
aenvstrf2                       deltapi                 iplkdelB + idlt / 2 
                                delayw                  aenvstrf
                                ; inject noise for attack time string fret contact, and pre pluck vibrations against pick 
anoiz                           rand	                ibrightness
aenvstrf1                       =                       aenvstrf1 + anoiz*aenvstrf1
aenvstrf2                       =                       aenvstrf2 + anoiz*aenvstrf2
                                ; filter to account for losses along loop path
aenvstrf2	                    filter2                 aenvstrf2, 3, 0, ia0, ia1, ia0 
                                ; combine into one signal (flip refl wave's phase)
aenvstrf                        =                       aenvstrf1 - aenvstrf2
                                ; initial backward excitation wave  
aenvstrb                        linseg                  0, isegB, - ipamp / 2, isegB2, 0  
adel2	                        delayr                  (idlt > 0) ? idlt : 0.01
                                ; initial bdwd traveling wave (pluck to nut)
aenvstrb1                       deltapi                 iplkdelB        
                                ; first forward traveling reflection (nut to bridge) 
aenvstrb2                       deltapi                 idlt / 2 + iplkdelF 
                                delayw                  aenvstrb
                                ; initial bdwd traveling wave (pluck to nut)
;  aenvstrb1	delay	aenvstrb,  iplkdelB
                                ; first bkwd traveling reflection (bridge to nut)
;  aenvstrb2	delay	aenvstrb, idlt/2+iplkdelF
                                ; inject noise
aenvstrb1                       =                       aenvstrb1 + anoiz*aenvstrb1
aenvstrb2                       =                       aenvstrb2 + anoiz*aenvstrb2
                                ; filter to account for losses along loop path
aenvstrb2	                    filter2                 aenvstrb2, 3, 0, ia0, ia1, ia0
                                ; combine into one signal (flip refl wave's phase)
aenvstrb	                    =	                    aenvstrb1 - aenvstrb2
                                ; low pass to band limit initial accel signals to be < 1/2 the sampling freq
ainputf                         tone                    aenvstrf, sr * 0.9 / 2
ainputb                         tone                    aenvstrb, sr * 0.9 / 2
                                ; additional lowpass filtering for pluck shaping\
                                ; Note, it would be more efficient to combine stages into a single filter
ainputf                         tone                    ainputf, sr * 0.9 / 2
ainputb                         tone                    ainputb, sr * 0.9 / 2
                                ; Vibrato generator
icosine                         =                       giWaveguideGuitar_cosine_1
avib                            poscil                  ivibDepth, ivibRate, icosine
avibdl		                    delayr		            (((ivibStDly * 1.1)) > 0.0) ? (ivibStDly * 1.1) : 0.01
avibrato	                    deltapi	                ivibStDly
                                delayw		            avib
                                ; Dual Delay line, 
                                ; NOTE: delay length longer than needed by a bit so that the output at t=idlt will be interpolated properly        
                                ;forward traveling wave delay line
afd  		                    delayr                  (((idlt + ivibDepth) * 1.1) > 0.0) ? ((idlt + ivibDepth) * 1.1) : 0.01
                                ; output tap point for fwd traveling wave
afwav  	                        deltapi                 ipupos    	
                                ; output at end of fwd delay (left string boundary)
afwdout	                        deltapi                 idlt - 1 / sr + avibrato	
                                ; lpf/attn due to reflection impedance		
afwdout	                        filter2                 afwdout, 3, 0, ia0, ia1, ia0  
                                delayw                  ainputf + afwdout * ifbfac * ifbfac
                                ; backward trav wave delay line
abkwd  	                        delayr                  (((idlt + ivibDepth) * 1.1) > 0) ? ((idlt + ivibDepth) * 1.1) : 0.01
                                ; output tap point for bkwd traveling wave
abkwav  	                    deltapi                 idlt / 2 - ipupos		
                                ; output at the left boundary
; abkterm	deltapi	idlt/2				
                                ; output at end of bkwd delay (right string boundary)
abkdout	                        deltapi                 idlt - 1 / sr + avibrato	
abkdout	                        filter2                 abkdout, 3, 0, ia0, ia1, ia0  	
                                delayw                  ainputb + abkdout * ifbfac * ifbfac
                                ; resonant body filter model, from Cuzzucoli and Lombardo
                                ; IIR filter derived via bilinear transform method
                                ; the theoretical resonances resulting from circuit model should be:
                                ; resonance due to the air volume + soundhole = 110Hz (strongest)
                                ; resonance due to the top plate = 220Hz
                                ; resonance due to the inclusion of the back plate = 400Hz (weakest)
aresbod                         filter2                 (afwdout + abkdout), 5, 4, 0.000000000005398681501844749, .00000000000001421085471520200, -.00000000001076383426834582, -00000000000001110223024625157, .000000000005392353230604385, -3.990098622573566, 5.974971737738533, -3.979630684599723, .9947612723736902
asignal                         =                       (1500 * (afwav + abkwav + aresbod * .000000000000000000003)) ; * adeclick
aoutleft, aoutright             pan2                    asignal * iamplitude, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin
                                
giXing_sine_1                   ftgen                   0, 0, 65536,    10,     1

                                instr                   Xing
                                //////////////////////////////////////////////
                                // Original by Andrew Horner.
                                // Adapted by Michael Gogins.
                                // p4 pitch in octave.pch
                                // original pitch        = A6
                                // range                 = C6 - C7
                                // extended range        = F4 - C7
                                //////////////////////////////////////////////
insno           		        =                       p1
itime           		        =                       p2
iduration       		        =                       p3
ikey            		        =                       p4
ivelocity                       =                       p5
iphase                          =                       p6
ipan                            =                       p7
idepth                          =                       p8
iheight                         =                       p9
ipcs                            =                       p10
ihomogeneity                    =                       p11
kgain			    	=                       1.25
iHz                             =                       cpsmidinn(ikey)
kHz                             =                       k(iHz)
iattack                         =                       (440.0 / iHz) * 0.01
                                print                   iHz, iattack
isustain                        =                       p3
irelease                        =                       .3
p3                              =                       iattack + isustain + irelease
iduration                       =                       p3
iamplitude                      =                       ampdb(ivelocity) * 8.
isine                           =                       giXing_sine_1
kfreq                           =                       cpsmidinn(ikey)
iamp                            =                       1
inorm                           =                       32310
aamp1                           linseg                  0,.001,5200,.001,800,.001,3000,.0025,1100,.002,2800,.0015,1500,.001,2100,.011,1600,.03,1400,.95,700,1,320,1,180,1,90,1,40,1,20,1,12,1,6,1,3,1,0,1,0
adevamp1                        linseg                  0, .05, .3, iduration - .05, 0
adev1                           poscil                  adevamp1, 6.7, isine, .8
amp1                            =                       aamp1 * (1 + adev1)
aamp2                           linseg                  0,.0009,22000,.0005,7300,.0009,11000,.0004,5500,.0006,15000,.0004,5500,.0008,2200,.055,7300,.02,8500,.38,5000,.5,300,.5,73,.5,5.,5,0,1,1
adevamp2                        linseg                  0,.12,.5,iduration-.12,0
adev2                           poscil                  adevamp2, 10.5, isine, 0
amp2                            =                       aamp2 * (1 + adev2)
aamp3                           linseg                  0,.001,3000,.001,1000,.0017,12000,.0013,3700,.001,12500,.0018,3000,.0012,1200,.001,1400,.0017,6000,.0023,200,.001,3000,.001,1200,.0015,8000,.001,1800,.0015,6000,.08,1200,.2,200,.2,40,.2,10,.4,0,1,0
adevamp3                        linseg                  0, .02, .8, iduration - .02, 0
adev3                           poscil                  adevamp3, 70, isine ,0
amp3                            =                       aamp3 * (1 + adev3)
awt1                            poscil                  amp1, kfreq, isine
awt2                            poscil                  amp2, 2.7 * kfreq, isine
awt3                            poscil                  amp3, 4.95 * kfreq, isine
asig                            =                       awt1 + awt2 + awt3
arel                            linenr                  1,0, iduration, .06
; asignal                         =                       asig * arel * (iamp / inorm) * iamplitude * kgain
asignal                         =                       asig * (iamp / inorm) * iamplitude * kgain
adeclick                        linsegr                 0, iattack, 1, isustain, 1, irelease, 0
asignal                         =                       asignal
aoutleft, aoutright		        pan2			        asignal * adeclick, .875;ipan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

giZakianFlute_f1_1              ftgen                   0, 0, 65536,    10,     1
giZakianFlute_f2_2              ftgen                   0, 0, 16,       -2,     40, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 10240
giZakianFlute_f26_3             ftgen                   0, 0, 65536,    -10,    2000, 489, 74, 219, 125, 9, 33, 5, 5
giZakianFlute_f27_4             ftgen                   0, 0, 65536,    -10,    2729, 1926, 346, 662, 537, 110, 61, 29, 7
giZakianFlute_f28_5             ftgen                   0, 0, 65536,    -10,    2558, 2012, 390, 361, 534, 139, 53, 22, 10, 13, 10
giZakianFlute_f29_6             ftgen                   0, 0, 65536,    -10,    12318, 8844, 1841, 1636, 256, 150, 60, 46, 11
giZakianFlute_f30_7             ftgen                   0, 0, 65536,    -10,    1229, 16, 34, 57, 32
giZakianFlute_f31_8             ftgen                   0, 0, 65536,    -10,    163, 31, 1, 50, 31
giZakianFlute_f32_9             ftgen                   0, 0, 65536,    -10,    4128, 883, 354, 79, 59, 23
giZakianFlute_f33_10            ftgen                   0, 0, 65536,    -10,    1924, 930, 251, 50, 25, 14
giZakianFlute_f34_11            ftgen                   0, 0, 65536,    -10,    94, 6, 22, 8
giZakianFlute_f35_12            ftgen                   0, 0, 65536,    -10,    2661, 87, 33, 18
giZakianFlute_f36_13            ftgen                   0, 0, 65536,    -10,    174, 12
giZakianFlute_f37_14            ftgen                   0, 0, 65536,    -10,    314, 13

                                instr			        ZakianFlute
                                //////////////////////////////////////////////
                                // Original by Lee Zakian.
                                // Adapted by Michael Gogins.
                                //////////////////////////////////////////////
if1                             =                       giZakianFlute_f1_1
iwtsin				            init			        if1
if2                             =                       giZakianFlute_f2_2
if26                            =                       giZakianFlute_f26_3
if27                            =                       giZakianFlute_f27_4
if28                            =                       giZakianFlute_f28_5
if29                            =                       giZakianFlute_f29_6
if30                            =                       giZakianFlute_f30_7
if31                            =                       giZakianFlute_f31_8
if32                            =                       giZakianFlute_f32_9
if33                            =                       giZakianFlute_f33_10
if34                            =                       giZakianFlute_f34_11
if35                            =                       giZakianFlute_f35_12
if36                            =                       giZakianFlute_f36_13
if37                            =                       giZakianFlute_f37_14
                                ;pset                    0, 0, 3600
i_instrument                    =                       p1
i_time                          =                       p2
i_duration                      =                       p3
i_midikey                       =                       p4
i_midivelocity                  =                       p5
i_phase                         =                       p6
i_pan                           =                       p7
i_depth                         =                       p8
i_height                        =                       p9
i_pitchclassset                 =                       p10
i_homogeneity                   =                       p11
ifrequency                      =                       cpsmidinn(i_midikey)
iamplitude                      =                       ampdb(i_midivelocity) * 4
iattack                         =                       .25
isustain                        =                       p3
irelease                        =                       .33333333
p3                              =                       iattack + isustain + irelease
iHz                             =                       ifrequency
kHz                             =                       k(iHz)
idB                             =                       i_midivelocity
adeclick77                        linsegr                 0, iattack, 1, isustain, 1, irelease, 0
ip3                     	    =                       (p3 < 3.0 ? p3 : 3.0)
; parameters
; p4    overall amplitude scaling factor
ip4                     	    init                    iamplitude
; p5    pitch in Hertz (normal pitch range: C4-C7)
ip5                     	    init                    iHz
; p6    percent vibrato depth, recommended values in range [-1., +1.]
ip6                     	    init                    1
;        0.0    -> no vibrato
;       +1.     -> 1% vibrato depth, where vibrato rate increases slightly
;       -1.     -> 1% vibrato depth, where vibrato rate decreases slightly
; p7    attack time in seconds
;       recommended value:  .12 for slurred notes, .06 for tongued notes
;                            (.03 for short notes)
ip7                     	    init                    .08
; p8    decay time in seconds
;       recommended value:  .1 (.05 for short notes)
ip8                     	    init                    .08
; p9    overall brightness / filter cutoff factor
;       1 -> least bright / minimum filter cutoff frequency (40 Hz)
;       9 -> brightest / maximum filter cutoff frequency (10,240Hz)
ip9                     	    init                    5
; initial variables
iampscale               	    =                       ip4                              ; overall amplitude scaling factor
ifreq                   	    =                       ip5                              ; pitch in Hertz
ivibdepth               	    =                       abs(ip6*ifreq/100.0)             ; vibrato depth relative to fundamental frequency
iattack                 	    =                       ip7 * (1.1 - .2*giseed)          ; attack time with up to +-10% random deviation
giseed                  	    =                       frac(giseed*105.947)             ; reset giseed
idecay                  	    =                       ip8 * (1.1 - .2*giseed)          ; decay time with up to +-10% random deviation
giseed                  	    =                       frac(giseed*105.947)
ifiltcut                	    tablei                  ip9, if2                          ; lowpass filter cutoff frequency
iattack                 	    =                       (iattack < 6/kr ? 6/kr : iattack)               ; minimal attack length
idecay                  	    =                       (idecay < 6/kr ? 6/kr : idecay)                 ; minimal decay length
isustain                	    =                       p3 - iattack - idecay
p3                      	    =                       (isustain < 5/kr ? iattack+idecay+5/kr : p3)    ; minimal sustain length
isustain                	    =                       (isustain < 5/kr ? 5/kr : isustain)
iatt                    	    =                       iattack/6
isus                    	    =                       isustain/4
idec                    	    =                       idecay/6
iphase                  	    =                       giseed                          ; use same phase for all wavetables
giseed                  	    =                       frac(giseed*105.947)
; vibrato block
; kvibdepth               	    linseg                  .1, .8*p3, 1, .2*p3, .7
kvibdepth               	    linseg                  .1, .8*ip3, 1, isustain, 1, .2*ip3, .7
kvibdepth               	    =                       kvibdepth* ivibdepth            ; vibrato depth
kvibdepthr              	    randi                   .1*kvibdepth, 5, giseed         ; up to 10% vibrato depth variation
giseed                  	    =                       frac(giseed*105.947)
kvibdepth               	    =                       kvibdepth + kvibdepthr
ivibr1                  	    =                       giseed                          ; vibrato rate
giseed                  	    =                       frac(giseed*105.947)
ivibr2                  	    =                       giseed
giseed                  	    =                       frac(giseed*105.947)

                                if                      ip6 < 0 goto            vibrato1
kvibrate                	    linseg                  2.5+ivibr1, p3, 4.5+ivibr2      ; if p6 positive vibrato gets faster
                                goto                    vibrato2
vibrato1:
ivibr3                  	    =                       giseed
giseed                  	    =                       frac(giseed*105.947)
kvibrate                	    linseg                  3.5+ivibr1, .1, 4.5+ivibr2, p3-.1, 2.5+ivibr3   ; if p6 negative vibrato gets slower
vibrato2:
kvibrater               	    randi                   .1*kvibrate, 5, giseed          ; up to 10% vibrato rate variation
giseed                  	    =                       frac(giseed*105.947)
kvibrate                	    =                       kvibrate + kvibrater
kvib                    	    oscili                  kvibdepth, kvibrate, iwtsin
ifdev1                  	    =                       -.03 * giseed                           ; frequency deviation
giseed                  	    =                       frac(giseed*105.947)
ifdev2                  	    =                       .003 * giseed
giseed                  	    =                       frac(giseed*105.947)
ifdev3                  	    =                       -.0015 * giseed
giseed                  	    =                       frac(giseed*105.947)
ifdev4                  	    =                       .012 * giseed
giseed                  	    =                       frac(giseed*105.947)
kfreqr                  	    linseg                  ifdev1, iattack, ifdev2, isustain, ifdev3, idecay, ifdev4
kfreq                   	    =                       kHz * (1 + kfreqr) + kvib
                                if                      ifreq <  427.28 goto    range1                          ; (cpspch(8.08) + cpspch(8.09))/2
                                if                      ifreq <  608.22 goto    range2                          ; (cpspch(9.02) + cpspch(9.03))/2
                                if                      ifreq <  1013.7 goto    range3                          ; (cpspch(9.11) + cpspch(10.00))/2
                                goto                    range4
; wavetable amplitude envelopes
range1:                 	    ; for low range tones
kamp1                   	    linseg                  0, iatt, 0.002, iatt, 0.045, iatt, 0.146, iatt,  \
                                                        0.272, iatt, 0.072, iatt, 0.043, isus, 0.230, isus, 0.000, isus, \
                                                        0.118, isus, 0.923, idec, 1.191, idec, 0.794, idec, 0.418, idec, \
                                                        0.172, idec, 0.053, idec, 0
kamp2                   	    linseg                  0, iatt, 0.009, iatt, 0.022, iatt, -0.049, iatt,  \
                                                        -0.120, iatt, 0.297, iatt, 1.890, isus, 1.543, isus, 0.000, isus, \
                                                        0.546, isus, 0.690, idec, -0.318, idec, -0.326, idec, -0.116, idec, \
                                                        -0.035, idec, -0.020, idec, 0
kamp3                   	    linseg                  0, iatt, 0.005, iatt, -0.026, iatt, 0.023, iatt,    \
                                                        0.133, iatt, 0.060, iatt, -1.245, isus, -0.760, isus, 1.000, isus,  \
                                                        0.360, isus, -0.526, idec, 0.165, idec, 0.184, idec, 0.060, idec,   \
                                                        0.010, idec, 0.013, idec, 0
iwt1                    	    =                       if26                                      ; wavetable numbers
iwt2                    	    =                       if27
iwt3                    	    =                       if28
inorm                   	    =                       3949
                                goto                    end
range2:                 	    ; for low mid-range tones
kamp1                   	    linseg                  0, iatt, 0.000, iatt, -0.005, iatt, 0.000, iatt, \
                                                        0.030, iatt, 0.198, iatt, 0.664, isus, 1.451, isus, 1.782, isus, \
                                                        1.316, isus, 0.817, idec, 0.284, idec, 0.171, idec, 0.082, idec, \
                                                        0.037, idec, 0.012, idec, 0
kamp2                   	    linseg                  0, iatt, 0.000, iatt, 0.320, iatt, 0.882, iatt,      \
                                                        1.863, iatt, 4.175, iatt, 4.355, isus, -5.329, isus, -8.303, isus,   \
                                                        -1.480, isus, -0.472, idec, 1.819, idec, -0.135, idec, -0.082, idec, \
                                                        -0.170, idec, -0.065, idec, 0
kamp3                   	    linseg                  0, iatt, 1.000, iatt, 0.520, iatt, -0.303, iatt,     \
                                                        0.059, iatt, -4.103, iatt, -6.784, isus, 7.006, isus, 11, isus,      \
                                                        12.495, isus, -0.562, idec, -4.946, idec, -0.587, idec, 0.440, idec, \
                                                        0.174, idec, -0.027, idec, 0
iwt1                    	    =                       if29
iwt2                    	    =                       if30
iwt3                    	    =                       if31
inorm                   	    =                       27668.2
                                goto                    end
range3:                 	    ; for high mid-range tones
kamp1                   	    linseg                  0, iatt, 0.005, iatt, 0.000, iatt, -0.082, iatt,      \
                                                        0.36, iatt, 0.581, iatt, 0.416, isus, 1.073, isus, 0.000, isus,       \
                                                        0.356, isus, .86, idec, 0.532, idec, 0.162, idec, 0.076, idec, 0.064, \
                                                        idec, 0.031, idec, 0
kamp2                   	    linseg                  0, iatt, -0.005, iatt, 0.000, iatt, 0.205, iatt,      \
                                                        -0.284, iatt, -0.208, iatt, 0.326, isus, -0.401, isus, 1.540, isus,   \
                                                        0.589, isus, -0.486, idec, -0.016, idec, 0.141, idec, 0.105, idec,    \
                                                        -0.003, idec, -0.023, idec, 0
kamp3                   	    linseg                  0, iatt, 0.722, iatt, 1.500, iatt, 3.697, iatt,       \
                                                        0.080, iatt, -2.327, iatt, -0.684, isus, -2.638, isus, 0.000, isus,   \
                                                        1.347, isus, 0.485, idec, -0.419, idec, -.700, idec, -0.278, idec,    \
                                                        0.167, idec, -0.059, idec, 0
iwt1                    	    =                       if32
iwt2                    	    =                       if33
iwt3                    	    =                       if34
inorm                   	    =                       3775
                                goto                    end
range4:                                                 ; for high range tones
kamp1                   	    linseg                  0, iatt, 0.000, iatt, 0.000, iatt, 0.211, iatt,         \
                                                        0.526, iatt, 0.989, iatt, 1.216, isus, 1.727, isus, 1.881, isus,        \
                                                        1.462, isus, 1.28, idec, 0.75, idec, 0.34, idec, 0.154, idec, 0.122,    \
                                                        idec, 0.028, idec, 0
kamp2                   	    linseg                  0, iatt, 0.500, iatt, 0.000, iatt, 0.181, iatt,         \
                                                        0.859, iatt, -0.205, iatt, -0.430, isus, -0.725, isus, -0.544, isus,    \
                                                        -0.436, isus, -0.109, idec, -0.03, idec, -0.022, idec, -0.046, idec,    \
                                                        -0.071, idec, -0.019, idec, 0
kamp3                   	    linseg                  0, iatt, 0.000, iatt, 1.000, iatt, 0.426, iatt,         \
                                                        0.222, iatt, 0.175, iatt, -0.153, isus, 0.355, isus, 0.175, isus,       \
                                                        0.16, isus, -0.246, idec, -0.045, idec, -0.072, idec, 0.057, idec,      \
                                                        -0.024, idec, 0.002, idec, 0
iwt1                    	    =                       if35
iwt2                    	    =                       if36
iwt3                    	    =                       if37
inorm                   	    =                       4909.05
                                goto                    end
end:
kampr1                  	    randi                   .02*kamp1, 10, giseed                   ; up to 2% wavetable amplitude variation
giseed                  	    =                       frac(giseed*105.947)
kamp1                   	    =                       kamp1 + kampr1
kampr2                  	    randi                   .02*kamp2, 10, giseed                   ; up to 2% wavetable amplitude variation
giseed                  	    =                       frac(giseed*105.947)
kamp2                   	    =                       kamp2 + kampr2
kampr3                  	    randi                   .02*kamp3, 10, giseed                   ; up to 2% wavetable amplitude variation
giseed                  	    =                       frac(giseed*105.947)
kamp3                   	    =                       kamp3 + kampr3
awt1                    	    poscil                  kamp1, kfreq, iwt1, iphase              ; wavetable lookup
awt2                    	    poscil                  kamp2, kfreq, iwt2, iphase
awt3                    	    poscil                  kamp3, kfreq, iwt3, iphase
asig                    	    =                       awt1 + awt2 + awt3
asig                    	    =                       asig*(iampscale/inorm)
kcut                    	    linseg                  0, iattack, ifiltcut, isustain, ifiltcut, idecay, 0     ; lowpass filter for brightness control
afilt                   	    tone                    asig, kcut
asignal                    	    balance                 afilt, asig
iattack                         =                       0.005
isustain                        =                       p3
irelease                        =                       0.06
p3                              =                       isustain + iattack + irelease
adeclick                        linsegr                 0.0, iattack, 1.0, isustain, 1.0, irelease, 0.0
aoutleft, aoutright             pan2                    asignal * adeclick, i_pan

                                outleta                 "outleft",  aoutleft
                                outleta                 "outright", aoutright
                                prints                  "instr %4d t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f\n", p1, p2, p3, p4, p5, p7
                                endin

                                //////////////////////////////////////////////
                                // OUTPUT INSTRUMENTS MUST GO BELOW HERE
                                //////////////////////////////////////////////

                                instr                   Reverberation
                                //////////////////////////////////////////////
                                // By Michael Gogins.
                                //////////////////////////////////////////////
ainleft                         inleta                  "inleft"
ainright                        inleta                  "inright"
if (gkReverberationEnabled == 0) goto thenclause
goto elseclause
thenclause:
aoutleft                        =                       ainleft
aoutright                       =                       ainright
kdry				            =			            1.0 - gkReverberationWet
goto endifclause
elseclause:
awetleft, awetright             reverbsc                ainleft, ainright, gkReverberationDelay, 18000.0
aoutleft			            =			            ainleft *  kdry + awetleft  * gkReverberationWet
aoutright			            =			            ainright * kdry + awetright * gkReverberationWet
endifclause:
                                outleta                 "outleft", aoutleft
                                outleta                 "outright", aoutright
                                endin

                                instr                   MasterOutput
                                //////////////////////////////////////////////
                                // By Michael Gogins.
                                //////////////////////////////////////////////
ainleft                         inleta                  "inleft"
ainright                        inleta                  "inright"
aoutleft                        =                       gkMasterLevel * ainleft
aoutright                       =                       gkMasterLevel * ainright

                                outs                    aoutleft, aoutright
                                endin

;; Original Instruments
                                
            </CsInstruments>
<CsScore>

i 14 0 8.177163 32 45 0.8015077 0.1719293 0 0.921981 1 1 
i 14 5.929556 8.177163 41 45 0.3273503 0.2489306 0 0.5424985 1 1 
i 16 10.00781 8.177163 90 47 0.9435932 0.2195438 0 0.9468152 1 1 
i 14 13.57773 8.177163 50 45 0.7032551 0.9209254 0 0.9329987 1 1 
i 16 13.85524 8.177163 46 47 0.7682953 0.1488756 0 0.3173265 1 1 
i 14 14.78251 8.177163 41 45 0.1512181 0.05430514 0 0.625787 1 1 
i 16 15.05422 8.177163 55 47 0.5032964 0.8405876 0 0.7681358 1 1 
i 14 15.65239 8.177163 31 45 0.2407319 0.3751646 0 0.6632236 1 1 
i 4 15.86986 8.177163 37 45 0.7165825 0.4088647 0 0.4772828 1 1 
i 16 19.05264 16.20762 63 47 0.2064787 0.4298789 0 0.3217218 1 1 
i 14 23.65128 8.177163 58 45 0.3348954 0.7675519 0 0.8351859 1 1 
i 14 23.74364 12.34076 50 45 0.9446616 0.1842026 0 0.7897129 1 1 
i 16 24.33056 8.177163 54 47 0.737375 0.1626645 0 0.4915301 1 1 
i 4 24.99226 8.177163 46 45 0.163307 0.647245 0 0.2391882 1 1 
i 16 25.60085 15.44181 72 47 0.08279713 0.09609478 0 0.417858 1 1 
i 14 27.86917 8.177163 39 45 0.488812 0.4621902 0 0.7645775 1 1 
i 14 31.78676 19.01443 40 45 0.7767779 0.8787873 0 0.6851968 1 1 
i 16 32.38226 8.177163 90 47 0.6896335 0.05253659 0 0.6295649 1 1 
i 27 35.13038 8.177163 86 40.75 0.7465254 0.4604295 0 0.5663792 1 1 
i 14 35.32488 8.177163 58 45 0.7773579 0.8390817 0 0.06599651 1 1 
i 4 36.75867 8.177163 54 45 0.7887567 0.7891214 0 0.8960666 1 1 
i 14 37.20512 8.177163 67 45 0.4308486 0.4213999 0 0.572861 1 1 
i 14 37.44119 8.177163 50 45 0.7355581 0.1922518 0 0.2571405 1 1 
i 16 37.84202 15.37762 63 47 0.9396694 0.7787611 0 0.3492035 1 1 
i 4 38.61219 8.177163 46 45 0.06218521 0.3198485 0 0.2455141 1 1 
i 14 39.68616 8.177163 29 45 0.813621 0.8666282 0 0.9095158 1 1 
i 4 39.95047 8.177163 35 45 0.9387137 0.7510079 0 0.1108358 1 1 
i 16 39.97043 8.177163 54 47 0.5850532 0.7642378 0 0.7095189 1 1 
i 14 40.10597 30.01449 59 45 0.6618378 0.6757096 0 0.4030884 1 1 
i 13 40.28504 8.177163 41 33 0.2372613 0.5554017 0 0.5246343 1 1 
i 16 41.50719 8.177163 44 47 0.3674862 0.4137877 0 0.5835415 1 1 
i 16 41.54605 8.177163 90 47 0.9184697 0.3707106 0 0.1889946 1 1 
i 27 41.89138 8.177163 50 40.75 0.3985663 0.4054174 0 0.7042592 1 1 
i 16 42.59842 8.177163 71 47 0.8847436 0.3997128 0 0.4425058 1 1 
i 14 44.87247 37.18045 48 45 0.608324 0.8264104 0 0.1575925 1 1 
i 16 46.80716 8.177163 53 47 0.3561977 0.4747611 0 0.5268578 1 1 
i 27 47.24832 8.177163 59 40.75 0.9395415 0.6944906 0 0.6984441 1 1 
i 14 50.61001 33.59443 67 45 0.5049487 0.8713197 0 0.5524419 1 1 
i 4 52.25645 8.177163 63 45 0.4662268 0.502871 0 0.5419328 1 1 
i 4 52.39854 14.5827 54 45 0.8190059 0.452826 0 0.5938083 1 1 
i 16 53.4884 8.177163 72 47 0.931933 0.4986898 0 0.08088558 1 1 
i 16 53.82336 8.177163 90 47 0.3768678 0.9293018 0 0.6615677 1 1 
i 13 54.31949 8.177163 50 33 0.8202876 0.3616101 0 0.09055359 1 1 
i 16 55.51479 8.177163 62 47 0.7249469 0.6441075 0 0.1696964 1 1 
i 16 55.63823 8.177163 71 47 0.1358197 0.9341245 0 0.304406 1 1 
i 16 55.80141 15.53269 63 47 0.1198013 0.7719003 0 0.6146459 1 1 
i 14 55.82496 8.177163 75 45 0.6622584 0.05728452 0 0.5305398 1 1 
i 27 56.02138 8.177163 67 40.75 0.2295961 0.4448001 0 0.1742012 1 1 
i 27 58.00725 8.177163 59 40.75 0.7361792 0.3940996 0 0.086424 1 1 
i 4 58.74551 14.20422 44 45 0.5042939 0.2776604 0 0.7903445 1 1 
i 14 59.29673 8.177163 40 45 0.7911099 0.9335508 0 0.3216446 1 1 
i 14 62.24207 10.40415 38 45 0.2730628 0.09314986 0 0.5396505 1 1 
i 16 62.5296 20.45448 90 47 0.3594556 0.8489535 0 0.07994171 1 1 
i 16 63.08968 15.09807 53 47 0.8396275 0.1965849 0 0.2392717 1 1 
i 16 63.22132 8.177163 72 47 0.4931978 0.2954777 0 0.2510779 1 1 
i 16 64.59142 8.177163 80 47 0.909449 0.4912713 0 0.6367718 1 1 
i 27 65.10688 8.177163 86 40.75 0.4417211 0.7317533 0 0.5475931 1 1 
i 27 65.7543 8.177163 67 40.75 0.3402597 0.09783731 0 0.4144832 1 1 
i 14 66.55249 39.86974 57 45 0.7782235 0.8675903 0 0.2824663 1 1 
i 15 68.78871 8.177163 82 45 0.4939942 0.1668619 0 0.3906504 1 1 
i 4 70.21583 15.53269 63 45 0.3090245 0.696623 0 0.611092 1 1 
i 14 71.21789 14.48156 76 45 0.3312572 0.7806866 0 0.3971395 1 1 
i 16 71.5905 8.177163 61 47 0.7841922 0.3595368 0 0.6523566 1 1 
i 16 71.66773 12.77057 72 47 0.4626472 0.391477 0 0.3232526 1 1 
i 14 72.21063 8.177163 59 45 0.4897559 0.8695085 0 0.7302109 1 1 
i 13 72.42167 8.177163 59 33 0.4042309 0.1472557 0 0.8331681 1 1 
i 4 73.10851 8.177163 71 45 0.8754061 0.3747746 0 0.539425 1 1 
i 4 73.4717 8.177163 55 45 0.2298856 0.1761295 0 0.9040326 1 1 
i 14 74.62828 8.86818 38 45 0.2660684 0.941099 0 0.06486853 1 1 
i 4 74.91293 14.70952 44 45 0.7517204 0.3997537 0 0.4789743 1 1 
i 13 75.27323 8.177163 50 33 0.06660576 0.5361242 0 0.8601649 1 1 
i 14 76.60426 8.177163 27 45 0.8473983 0.2250458 0 0.4471011 1 1 
i 4 76.9255 8.177163 33 45 0.2655522 0.1830461 0 0.7696879 1 1 
i 13 77.33213 8.177163 40 33 0.1308408 0.4757136 0 0.6300955 1 1 
i 27 77.38419 8.177163 86 40.75 0.575944 0.6197573 0 0.7039889 1 1 
i 11 77.84685 8.177163 47 43 0.662366 0.3691743 0 0.6865894 1 1 
i 27 78.79412 8.177163 68 40.75 0.1703627 0.1960957 0 0.4546005 1 1 
i 14 78.86164 10.4573 37 45 0.7676277 0.08784871 0 0.2008002 1 1 
i 14 79.92773 10.32109 49 45 0.3424672 0.7980929 0 0.6402019 1 1 
i 16 79.99987 8.177163 64 47 0.7537182 0.428171 0 0.1518733 1 1 
i 14 81.17614 41.54895 68 45 0.2134157 0.9441812 0 0.7366383 1 1 
i 16 81.65485 8.620459 52 47 0.2697244 0.2893286 0 0.1406646 1 1 
i 27 82.06859 8.177163 59 40.75 0.3034646 0.3598951 0 0.9217763 1 1 
i 16 83.32245 8.177163 80 47 0.5953354 0.2423955 0 0.2538962 1 1 
i 16 83.40103 17.08558 92 47 0.4114143 0.220915 0 0.4219616 1 1 
i 16 83.9659 8.177163 44 47 0.3191466 0.2031371 0 0.6228217 1 1 
i 27 84.43283 8.177163 49 40.75 0.7423189 0.1864167 0 0.8455717 1 1 
i 16 84.70754 46.75557 73 47 0.06625964 0.9132994 0 0.5745617 1 1 
i 4 84.90443 19.342 52 45 0.345018 0.7401687 0 0.1661595 1 1 
i 15 85.0239 8.177163 56 45 0.8123987 0.3127647 0 0.257688 1 1 
i 14 85.9687 19.49336 76 45 0.05143811 0.5017769 0 0.06128492 1 1 
i 14 87.13235 19.23675 47 45 0.1388291 0.3079279 0 0.3962797 1 1 
i 4 87.42603 15.16543 64 45 0.6979738 0.3698138 0 0.1089006 1 1 
i 16 92.90902 8.686203 61 47 0.4315805 0.5124733 0 0.6708211 1 1 
i 27 93.38412 8.177163 68 40.75 0.4113676 0.2785488 0 0.2880289 1 1 
i 4 93.73141 8.177163 71 45 0.4752727 0.8908255 0 0.5998246 1 1 
i 27 93.8329 8.177163 85 40.75 0.5130676 0.4895617 0 0.2705173 1 1 
i 16 95.56281 8.177163 52 47 0.5021261 0.3497047 0 0.5864521 1 1 
i 27 96.099 8.177163 57 40.75 0.9166978 0.3291404 0 0.2591603 1 1 
i 13 96.2644 8.177163 68 33 0.3402797 0.7222138 0 0.3364089 1 1 
i 13 96.48301 8.177163 59 33 0.5312489 0.9072248 0 0.8346246 1 1 
i 15 96.77772 8.177163 64 45 0.6504027 0.8911424 0 0.3107205 1 1 
i 11 99.43832 8.177163 54 43 0.2586998 0.108972 0 0.6842513 1 1 
i 16 99.84973 8.177163 90 47 0.4299965 0.2255967 0 0.8924496 1 1 
i 14 100.4047 103.5106 77 45 0.6288629 0.1779955 0 0.4754589 1 1 
i 16 100.792 8.177163 65 47 0.2698877 0.4432396 0 0.5053275 1 1 
i 4 102.1778 8.177163 70 45 0.1188536 0.8662497 0 0.6925135 1 1 
i 14 102.2786 33.09231 56 45 0.2201758 0.6054844 0 0.4072836 1 1 
i 16 103.0801 8.177163 92 47 0.8222894 0.2998977 0 0.8680429 1 1 
i 14 103.586 46.39823 46 45 0.2127368 0.6055512 0 0.5135213 1 1 
i 14 103.6904 8.177163 65 45 0.373392 0.7921685 0 0.7845053 1 1 
i 4 103.9129 17.44955 53 45 0.204906 0.3140494 0 0.4935106 1 1 
i 14 104.2485 8.177163 94 40 0.8278767 0.2257923 0 0.2337775 1 1 
i 4 104.269 15.07543 65 45 0.8402455 0.604645 0 0.4032222 1 1 
i 16 105.0138 11.58099 80 47 0.2000284 0.9019195 0 0.9246897 1 1 
i 27 105.4972 8.177163 85 40.75 0.588532 0.5575684 0 0.1680997 1 1 
i 16 105.9488 8.177163 61 47 0.5489422 0.5600997 0 0.329441 1 1 
i 13 106.2476 8.177163 49 33 0.7140117 0.158584 0 0.2684513 1 1 
i 14 106.2632 8.177163 66 45 0.2169284 0.7905732 0 0.3314092 1 1 
i 13 106.3377 8.177163 58 33 0.6567917 0.8435529 0 0.7153678 1 1 
i 27 106.4239 8.177163 68 40.75 0.1684054 0.1062889 0 0.1309576 1 1 
i 14 106.9418 8.177163 49 40 0.4608594 0.057483 0 0.620873 1 1 
i 16 107.714 8.177163 70 47 0.4980825 0.5267867 0 0.3522181 1 1 
i 14 107.781 14.6706 58 40 0.2603079 0.277541 0 0.2058467 1 1 
i 27 108.2596 8.177163 77 40.75 0.7042038 0.5863829 0 0.8398158 1 1 
i 15 108.9501 8.177163 82 45 0.261289 0.6864438 0 0.4801962 1 1 
i 15 109.8175 8.177163 65 45 0.7225981 0.06639059 0 0.6392512 1 1 
i 14 110.2765 13.77726 37 45 0.4754922 0.1202602 0 0.9468276 1 1 
i 14 110.5799 21.32973 68 40 0.8594924 0.5288878 0 0.7456108 1 1 
i 16 111.3638 10.10634 90 47 0.8663978 0.2333837 0 0.3873313 1 1 
i 4 111.6269 11.6033 42 45 0.3440293 0.81328 0 0.344063 1 1 
i 4 113.494 16.20184 73 45 0.5780885 0.6432896 0 0.09408488 1 1 
i 16 113.8829 8.177163 78 47 0.492577 0.7429691 0 0.4893656 1 1 
i 16 114.9557 8.177163 61 47 0.4052549 0.4362139 0 0.7488084 1 1 
i 14 115.1637 96.93584 77 40 0.7236546 0.9272789 0 0.6210518 1 1 
i 13 115.52 8.177163 49 33 0.620027 0.1149622 0 0.5260286 1 1 
i 27 115.8703 14.25279 85 40.75 0.3427951 0.3165509 0 0.6180079 1 1 
i 27 117.6367 8.177163 56 40.75 0.514443 0.9450549 0 0.7020085 1 1 
i 27 117.7401 8.177163 68 40.75 0.3491052 0.2710173 0 0.7237482 1 1 
i 14 118.2817 8.177163 49 45 0.6631442 0.6646698 0 0.1209581 1 1 
i 16 119.5288 8.177163 92 47 0.6783322 0.4630427 0 0.1110963 1 1 
i 14 119.9107 14.59181 94 40 0.5243385 0.112226 0 0.2858439 1 1 
i 14 119.9825 14.84983 65 45 0.1677249 0.4850376 0 0.1349967 1 1 
i 14 121.1861 10.51889 66 45 0.9228535 0.1583435 0 0.7306535 1 1 
i 16 121.4625 8.177163 80 47 0.7024154 0.2913587 0 0.4241863 1 1 
i 5 121.8344 8.177163 90 41 0.09989728 0.4772466 0 0.6023697 1 1 
i 16 121.8871 8.177163 90 47 0.8640274 0.4915573 0 0.8916928 1 1 
i 4 121.9836 9.612716 61 45 0.3234427 0.876449 0 0.7610916 1 1 
i 16 123.2135 8.859157 61 47 0.4540567 0.2536047 0 0.509098 1 1 
i 16 123.8138 8.177163 53 47 0.6906927 0.2575687 0 0.9360073 1 1 
i 13 123.8942 8.177163 68 33 0.2954003 0.6396919 0 0.472114 1 1 
i 14 125.2224 12.96287 58 40 0.4517815 0.05270108 0 0.9238497 1 1 
i 15 125.3989 8.177163 82 45 0.2435981 0.3241972 0 0.2728266 1 1 
i 14 125.6051 8.177163 68 45 0.487734 0.7758059 0 0.4167517 1 1 
i 14 126.2981 8.177163 49 40 0.5362269 0.2440664 0 0.7007218 1 1 
i 5 126.567 8.177163 56 41 0.5385759 0.3053245 0 0.7303322 1 1 
i 4 126.9631 8.177163 65 45 0.7715867 0.3487796 0 0.8213652 1 1 
i 11 127.2878 8.177163 63 43 0.8464484 0.06340253 0 0.5837782 1 1 
i 14 127.3983 8.771075 58 45 0.8823883 0.1275955 0 0.8723476 1 1 
i 4 128.5152 9.186814 53 45 0.8600199 0.9326811 0 0.9047431 1 1 
i 4 128.6551 8.177163 74 45 0.05749184 0.8910698 0 0.7500146 1 1 
i 14 128.7728 8.177163 67 40 0.5790893 0.07336299 0 0.6348027 1 1 
i 13 128.9032 8.177163 58 33 0.1499714 0.3454128 0 0.1618454 1 1 
i 5 130.3169 8.177163 63 41 0.5299065 0.9265236 0 0.6460703 1 1 
i 14 130.3367 9.240266 38 45 0.1118647 0.6874319 0 0.23536 1 1 
i 16 130.6269 8.177163 79 47 0.06007333 0.4400867 0 0.1123363 1 1 
i 4 130.6826 8.771075 41 45 0.3310071 0.3587556 0 0.9434684 1 1 
i 16 131.0429 8.177163 91 47 0.8213596 0.2921579 0 0.3700498 1 1 
i 13 131.1205 8.177163 50 33 0.8103706 0.7220915 0 0.2454248 1 1 
i 11 131.6748 8.177163 54 43 0.6909734 0.918457 0 0.9344011 1 1 
i 13 131.7144 8.177163 49 33 0.2239343 0.7339091 0 0.3673306 1 1 
i 4 131.7457 13.62691 42 45 0.7988611 0.2266703 0 0.9073353 1 1 
i 14 131.7704 13.23332 37 45 0.8257719 0.740633 0 0.4600974 1 1 
i 14 132.5023 38.52631 66 45 0.804434 0.3494155 0 0.5488779 1 1 
i 4 132.8349 34.23705 73 45 0.2409654 0.158779 0 0.4921153 1 1 
i 16 133.0871 8.177163 73 47 0.7335603 0.4186893 0 0.3713578 1 1 
i 14 133.3321 8.177163 25 45 0.9333864 0.7446075 0 0.8115455 1 1 
i 27 133.3547 8.177163 77 40.75 0.380475 0.8011595 0 0.09556335 1 1 
i 16 133.4012 35.91613 92 47 0.8083304 0.9280054 0 0.8325143 1 1 
i 27 133.46 8.177163 85 40.75 0.07913846 0.1179486 0 0.6025386 1 1 
i 4 133.7226 8.177163 32 45 0.603975 0.7121123 0 0.6463671 1 1 
i 13 134.2168 8.177163 37 33 0.2892988 0.906138 0 0.8774472 1 1 
i 11 134.8424 8.177163 44 43 0.3317591 0.1565569 0 0.8349039 1 1 
i 14 134.9194 63.88166 94 40 0.7772526 0.2602793 0 0.5508786 1 1 
i 16 134.9875 8.177163 61 47 0.5033331 0.8992331 0 0.6497751 1 1 
i 13 135.2104 8.177163 68 33 0.9259059 0.1876692 0 0.6203902 1 1 
i 27 135.4626 8.177163 68 40.75 0.1167285 0.2107848 0 0.4954773 1 1 
i 14 135.595 8.177163 65 45 0.4682118 0.7573819 0 0.8447285 1 1 
i 14 135.6343 8.177163 49 45 0.8629169 0.632632 0 0.05537625 1 1 
i 14 136.1034 8.177163 66 40 0.5765807 0.6742056 0 0.1448252 1 1 
i 14 136.3041 13.32603 68 40 0.2569928 0.9412879 0 0.3496171 1 1 
i 5 136.4581 14.99021 73 41 0.9152787 0.6704446 0 0.2539392 1 1 
i 5 137.8482 8.177163 65 41 0.8386216 0.8818173 0 0.4385701 1 1 
i 16 139.0291 12.64793 80 47 0.5558922 0.4737987 0 0.8038539 1 1 
i 16 139.6399 8.177163 70 47 0.6253504 0.183478 0 0.9460343 1 1 
i 14 140.4997 12.68528 56 45 0.4455665 0.6254547 0 0.07554951 1 1 
i 4 140.6838 11.30448 61 45 0.5273305 0.1248358 0 0.7538014 1 1 
i 14 141.4059 21.48308 56 40 0.2211449 0.7129807 0 0.2856445 1 1 
i 16 141.4245 8.177163 73 47 0.4640242 0.4570203 0 0.6562734 1 1 
i 4 142.134 8.177163 53 45 0.2190478 0.2092578 0 0.1368381 1 1 
i 14 142.4571 8.177163 85 40 0.2032938 0.3414871 0 0.07226067 1 1 
i 5 142.8179 8.177163 90 41 0.2703455 0.7975787 0 0.1778315 1 1 
i 16 143.2068 9.336553 61 47 0.3227097 0.1898978 0 0.08021509 1 1 
i 27 143.6523 8.177163 68 40.75 0.7548113 0.4183604 0 0.4490347 1 1 
i 27 143.9833 8.177163 85 40.75 0.3072059 0.06573939 0 0.906257 1 1 
i 9 145.3952 8.177163 85 28 0.1347348 0.2041835 0 0.2965206 1 1 
i 16 145.6956 9.397932 53 47 0.7278047 0.2312275 0 0.7707915 1 1 
i 27 146.1985 8.859157 56 40.75 0.4972951 0.5774758 0 0.5818759 1 1 
i 13 146.4921 8.177163 56 33 0.2165882 0.5619124 0 0.861491 1 1 
i 14 146.6585 9.090874 65 45 0.2879388 0.07201593 0 0.7542637 1 1 
i 15 146.835 8.177163 65 45 0.5505939 0.5897319 0 0.1368795 1 1 
i 14 147.3564 8.177163 66 40 0.4085522 0.8841083 0 0.08591265 1 1 
i 15 147.4362 8.177163 82 45 0.4218934 0.5766051 0 0.1665801 1 1 
i 16 149.1864 8.177163 41 47 0.09986577 0.5980451 0 0.8134662 1 1 
i 27 149.7539 8.177163 46 40.75 0.4865129 0.4568575 0 0.1107022 1 1 
i 13 150.3715 8.177163 68 33 0.6299283 0.2489053 0 0.9282486 1 1 
i 15 150.4723 8.177163 53 45 0.7128252 0.9044081 0 0.4287683 1 1 
i 16 151.3816 8.177163 58 47 0.6675945 0.6671605 0 0.2581396 1 1 
i 5 151.412 8.177163 92 41 0.4420338 0.9452973 0 0.7194134 1 1 
i 5 152.399 8.177163 73 41 0.3766242 0.3542455 0 0.9408949 1 1 
i 4 152.7509 19.24067 61 45 0.803756 0.7025705 0 0.4082319 1 1 
i 14 153.243 8.177163 68 40 0.2044426 0.8165262 0 0.9026548 1 1 
i 14 154.3191 8.177163 37 45 0.3963908 0.365807 0 0.409757 1 1 
i 5 154.6911 8.177163 65 41 0.8023258 0.6965298 0 0.9057868 1 1 
i 27 155.4974 12.50514 85 40.75 0.6480731 0.1075833 0 0.2086804 1 1 
i 14 155.5688 8.177163 85 40 0.2015599 0.3838623 0 0.1518824 1 1 
i 14 156.0192 8.177163 46 40 0.4059651 0.1873161 0 0.4068276 1 1 
i 5 156.3461 8.177163 53 41 0.455029 0.07516805 0 0.1386894 1 1 
i 9 156.7598 8.177163 58 28 0.4696853 0.874416 0 0.7242159 1 1 
i 14 157.0749 12.28187 56 45 0.7126763 0.7063964 0 0.6408935 1 1 
i 14 157.3068 9.090874 46 45 0.09634338 0.2442229 0 0.1028287 1 1 
i 16 157.5171 8.177163 61 47 0.7793694 0.4292661 0 0.3351372 1 1 
i 16 159.372 9.871658 80 47 0.4181973 0.8456825 0 0.2314103 1 1 
i 13 161.1226 8.177163 68 33 0.1097964 0.0729497 0 0.6587571 1 1 
i 4 161.6634 9.679625 53 45 0.3464704 0.1229679 0 0.3826218 1 1 
i 16 161.9043 9.419281 70 47 0.2088367 0.7671717 0 0.1334394 1 1 
i 14 162.2794 8.533491 66 40 0.308216 0.1036531 0 0.6943873 1 1 
i 27 162.416 12.73749 77 40.75 0.8646121 0.4941877 0 0.4140978 1 1 
i 16 162.542 8.177163 73 47 0.3875222 0.1106387 0 0.8137207 1 1 
i 5 162.612 8.177163 73 41 0.1783501 0.3782373 0 0.4295296 1 1 
i 14 162.7686 8.177163 44 45 0.9473373 0.8320663 0 0.5303232 1 1 
i 5 162.9261 8.177163 92 41 0.3490397 0.4051097 0 0.0887286 1 1 
i 15 163.0637 8.177163 82 45 0.2126238 0.8208638 0 0.3499845 1 1 
i 13 163.6687 8.177163 56 33 0.3447146 0.4382279 0 0.2773313 1 1 
i 14 164.1371 8.177163 57 40 0.1314695 0.1684085 0 0.9069288 1 1 
i 16 164.2485 8.177163 81 47 0.570343 0.30159 0 0.367709 1 1 
i 11 164.3053 8.177163 63 43 0.7490661 0.3352661 0 0.3831616 1 1 
i 5 164.5124 8.177163 62 41 0.05992212 0.2223378 0 0.403047 1 1 
i 4 164.7281 8.177163 62 45 0.9385549 0.866006 0 0.6880311 1 1 
i 9 164.9875 8.177163 67 28 0.6089491 0.6241575 0 0.8825853 1 1 
i 16 165.4562 8.177163 62 47 0.06750709 0.8426731 0 0.6983647 1 1 
i 27 166.0336 8.177163 65 40.75 0.10086 0.5819691 0 0.6249004 1 1 
i 4 166.53 15.70299 74 45 0.468104 0.3168244 0 0.9031399 1 1 
i 14 166.6079 40.43438 55 45 0.5030693 0.5467392 0 0.1700028 1 1 
i 15 166.7646 8.177163 74 45 0.8825901 0.6918128 0 0.8631939 1 1 
i 16 167.6898 8.177163 77 47 0.3577721 0.6226631 0 0.647288 1 1 
i 14 167.7975 8.177163 67 40 0.2933944 0.6436494 0 0.6280521 1 1 
i 14 168.1956 37.67322 65 45 0.7719914 0.4363284 0 0.3489652 1 1 
i 14 168.4712 12.8414 86 40 0.6388502 0.689456 0 0.2665897 1 1 
i 16 168.5065 34.69278 91 47 0.5934536 0.8772055 0 0.3949209 1 1 
i 14 168.8519 8.177163 58 45 0.4955043 0.7521129 0 0.8465545 1 1 
i 27 170.9235 8.177163 86 40.75 0.300144 0.4081888 0 0.2682737 1 1 
i 5 171.0911 43.17729 91 41 0.4054885 0.05134216 0 0.2317976 1 1 
i 14 171.4073 50.92427 65 40 0.5976111 0.7448673 0 0.7511512 1 1 
i 5 171.7398 16.0985 74 41 0.3960601 0.499453 0 0.7976578 1 1 
i 14 172.21 8.177163 50 40 0.2746985 0.1651615 0 0.5028373 1 1 
i 14 172.6429 8.177163 75 40 0.930818 0.1653903 0 0.7041824 1 1 
i 5 173.0248 8.177163 79 41 0.6643193 0.2510591 0 0.4538372 1 1 
i 4 173.2216 10.79369 62 45 0.868429 0.654853 0 0.8048859 1 1 
i 16 173.3446 8.177163 79 47 0.9059632 0.2407005 0 0.8401025 1 1 
i 9 173.5082 8.177163 86 28 0.9236421 0.6881633 0 0.4306185 1 1 
i 14 173.6488 9.364108 46 45 0.1339874 0.8035488 0 0.656084 1 1 
i 9 174.1154 8.177163 67 28 0.6777424 0.1385335 0 0.2443562 1 1 
i 16 174.2986 8.177163 74 47 0.1226629 0.750047 0 0.414771 1 1 
i 14 175.1724 18.84478 58 40 0.1265294 0.2367414 0 0.5172643 1 1 
i 4 175.233 8.177163 51 45 0.1994812 0.9306386 0 0.876627 1 1 
i 13 175.7358 8.177163 58 33 0.5728004 0.3242767 0 0.8094899 1 1 
i 13 176.2837 8.177163 67 33 0.05541219 0.399265 0 0.05892784 1 1 
i 16 176.9611 8.177163 82 47 0.4040439 0.2285355 0 0.2268685 1 1 
i 11 179.3277 8.177163 53 43 0.7517651 0.8017325 0 0.1194831 1 1 
i 11 179.4663 8.177163 63 43 0.2979322 0.3122779 0 0.6095354 1 1 
i 5 179.5888 8.177163 62 41 0.2528765 0.8790541 0 0.306316 1 1 
i 16 181.4094 8.662907 82 47 0.3450174 0.1261704 0 0.09688895 1 1 
i 27 181.8627 8.177163 87 40.75 0.5401155 0.5997959 0 0.3631134 1 1 
i 14 182.2668 17.74787 86 40 0.9498752 0.5242699 0 0.8197244 1 1 
i 16 182.413 9.7059 70 47 0.1659249 0.8626671 0 0.6042363 1 1 
i 14 183.9127 8.177163 55 40 0.9390477 0.7973074 0 0.6310567 1 1 
i 4 184.2851 17.47493 62 45 0.7316347 0.7252991 0 0.6499798 1 1 
i 14 184.3687 15.70825 46 45 0.7072292 0.5360248 0 0.7517676 1 1 
i 27 184.4534 8.177163 77 40.75 0.5843828 0.4897823 0 0.9204558 1 1 
i 16 184.6016 9.106853 79 47 0.08165223 0.1205399 0 0.3849902 1 1 
i 4 184.6685 15.60607 74 45 0.7557687 0.1201221 0 0.4794079 1 1 
i 4 184.766 17.63917 51 45 0.3036138 0.4465872 0 0.5151245 1 1 
i 9 185.0223 8.177163 86 28 0.1561023 0.3810745 0 0.5887625 1 1 
i 15 185.101 8.177163 82 45 0.4709995 0.06492443 0 0.1458573 1 1 
i 13 186.7994 9.090874 58 33 0.1282028 0.3548562 0 0.2346422 1 1 
i 13 186.8964 8.177163 67 33 0.4791013 0.5160011 0 0.0816712 1 1 
i 5 187.0842 11.68432 79 41 0.9055525 0.8178252 0 0.8176419 1 1 
i 13 187.6036 8.177163 46 33 0.8260767 0.9322703 0 0.7891763 1 1 
i 14 188.4649 8.177163 67 40 0.5884086 0.8767617 0 0.794097 1 1 
i 16 189.7272 8.177163 77 47 0.8718247 0.5338455 0 0.2010041 1 1 
i 5 192.0669 8.177163 74 41 0.8208501 0.06699975 0 0.807239 1 1 
i 14 193.7368 8.177163 68 40 0.7065667 0.7593683 0 0.7110927 1 1 
i 4 194.1415 8.177163 41 45 0.5569069 0.577817 0 0.8818093 1 1 
i 16 194.1733 10.83945 80 47 0.1411899 0.5947135 0 0.6071131 1 1 
i 14 194.428 15.93598 56 40 0.3118678 0.7104381 0 0.3367624 1 1 
i 14 194.5635 14.54738 84 40 0.6271374 0.5665035 0 0.09987951 1 1 
i 14 194.615 17.96143 92 40 0.7632649 0.9319233 0 0.6252622 1 1 
i 14 194.991 8.177163 75 40 0.9254111 0.3418937 0 0.5047115 1 1 
i 14 195.7354 14.1153 44 40 0.5626506 0.8792991 0 0.4517119 1 1 
i 5 196.0623 8.177163 53 41 0.1581771 0.9180189 0 0.9364315 1 1 
i 16 196.2199 8.177163 68 47 0.5872963 0.2569728 0 0.563313 1 1 
i 27 196.7315 8.177163 77 40.75 0.4209643 0.7381793 0 0.1002963 1 1 
i 14 197.169 10.38295 44 45 0.2048371 0.5054015 0 0.5261234 1 1 
i 27 197.2889 8.177163 87 40.75 0.6644165 0.5310653 0 0.475185 1 1 
i 5 197.7997 8.177163 72 41 0.3870635 0.347733 0 0.1741333 1 1 
i 14 199.3065 8.177163 32 45 0.3463695 0.7064508 0 0.6444561 1 1 
i 5 199.582 11.15732 63 41 0.8666471 0.1073692 0 0.1441923 1 1 
i 9 200.0276 8.177163 68 28 0.4032682 0.1393263 0 0.6491024 1 1 
i 15 200.3862 8.177163 72 45 0.4661529 0.4960386 0 0.2401577 1 1 
i 9 200.4484 8.177163 87 28 0.1469521 0.4185525 0 0.490389 1 1 
i 15 200.5272 8.177163 80 45 0.3727389 0.1506407 0 0.3115962 1 1 
i 5 202.0708 8.177163 51 41 0.2831259 0.3450232 0 0.5406865 1 1 
i 9 202.5737 8.177163 56 28 0.06560736 0.1126686 0 0.1714273 1 1 
i 16 203.2102 8.177163 63 47 0.7180903 0.8568859 0 0.5932969 1 1 
i 16 205.4302 9.106853 80 47 0.8727466 0.9075968 0 0.2681163 1 1 
i 14 205.5269 8.177163 68 40 0.4488202 0.8618609 0 0.5564141 1 1 
i 16 205.8204 8.177163 91 47 0.08301028 0.3660234 0 0.557405 1 1 
i 16 205.9196 8.177163 68 47 0.7445497 0.3624702 0 0.6171705 1 1 
i 14 206.6853 8.177163 57 40 0.8257942 0.8533478 0 0.3538509 1 1 
i 5 206.975 10.97539 62 41 0.6659253 0.6474037 0 0.4100729 1 1 
i 14 207.384 119.9401 86 40 0.0536774 0.0665529 0 0.4012687 1 1 
i 14 207.5341 11.93446 93 40 0.2300581 0.9499474 0 0.6152385 1 1 
i 14 207.6459 8.177163 77 45 0.8175042 0.8775343 0 0.7950578 1 1 
i 27 207.9885 9.106853 76 40.75 0.2982827 0.8812501 0 0.6305088 1 1 
i 27 208.0872 8.177163 86 40.75 0.9428414 0.4263693 0 0.473671 1 1 
i 27 208.8068 8.177163 65 40.75 0.8899971 0.6578023 0 0.5639927 1 1 
i 14 208.8159 10.49159 65 45 0.3380415 0.4445081 0 0.8739175 1 1 
i 14 208.9116 10.71115 74 40 0.3017346 0.9338973 0 0.06779949 1 1 
i 4 209.1084 8.177163 72 45 0.8475327 0.3526688 0 0.1501387 1 1 
i 5 209.1208 8.177163 89 41 0.5223424 0.3308696 0 0.1574376 1 1 
i 5 209.2935 13.01016 81 41 0.3313879 0.7347888 0 0.3100883 1 1 
i 14 210.4498 11.759 57 45 0.4204914 0.3432293 0 0.287019 1 1 
i 4 210.7799 10.17819 62 45 0.2540377 0.4877765 0 0.8881708 1 1 
i 14 211.0225 8.226143 76 40 0.9288275 0.09009227 0 0.7546961 1 1 
i 13 211.1978 8.177163 69 33 0.2425664 0.4554714 0 0.1427882 1 1 
i 9 211.3876 8.177163 86 28 0.5324785 0.5404914 0 0.6511414 1 1 
i 9 212.0213 10.1341 76 28 0.6320996 0.12275 0 0.1811682 1 1 
i 5 212.7343 8.177163 72 41 0.8765956 0.4127831 0 0.4311019 1 1 
i 14 212.7414 9.322043 45 45 0.6040888 0.6280839 0 0.7696824 1 1 
i 14 213.0624 8.177163 64 40 0.6802223 0.4484703 0 0.7871577 1 1 
i 4 213.114 8.816761 50 45 0.1001348 0.2888248 0 0.6867382 1 1 
i 16 213.3101 8.827946 81 47 0.9205748 0.770599 0 0.9428466 1 1 
i 16 213.3792 8.177163 89 47 0.8809365 0.08801393 0 0.1065384 1 1 
i 5 213.4666 8.177163 69 41 0.1649471 0.3933406 0 0.4452445 1 1 
i 13 213.5856 8.177163 57 33 0.836603 0.0577423 0 0.8483127 1 1 
i 16 213.8829 8.177163 69 47 0.3251999 0.5166458 0 0.4067559 1 1 
i 14 214.0972 8.177163 53 40 0.2524544 0.4123762 0 0.5461696 1 1 
i 11 214.1826 8.177163 63 43 0.3841084 0.3073057 0 0.09451374 1 1 
i 13 214.2252 9.090874 58 33 0.6091687 0.7827287 0 0.3297311 1 1 
i 4 214.2589 9.812706 51 45 0.4377325 0.2561871 0 0.7020651 1 1 
i 14 214.2855 10.38295 46 45 0.6639005 0.8258614 0 0.5694932 1 1 
i 16 214.6259 8.177163 82 47 0.2003807 0.7717716 0 0.6237449 1 1 
i 16 214.8126 8.911618 70 47 0.8684395 0.9424229 0 0.4764998 1 1 
i 16 214.8906 8.177163 82 47 0.858033 0.5991337 0 0.6912326 1 1 
i 14 215.1976 29.53642 94 40 0.5248918 0.4330159 0 0.2557303 1 1 
i 16 215.459 8.177163 62 47 0.1479065 0.8551232 0 0.9049173 1 1 
i 14 215.9673 10.83345 34 45 0.07522796 0.4082721 0 0.45236 1 1 
i 4 216.3878 10.38295 41 45 0.8517783 0.2353254 0 0.8963686 1 1 
i 5 216.8896 18.04627 91 41 0.1746777 0.1183793 0 0.5125949 1 1 
i 13 216.92 9.812706 46 33 0.7756434 0.8303779 0 0.8060414 1 1 
i 14 217.461 13.7221 77 40 0.7920811 0.8749635 0 0.6405584 1 1 
i 11 217.5937 9.090874 53 43 0.3007017 0.8632344 0 0.7918175 1 1 
i 5 218.3896 8.177163 79 41 0.6951478 0.6649679 0 0.7527929 1 1 
i 14 218.4465 8.177163 58 45 0.7257264 0.6460346 0 0.2319269 1 1 
i 5 219.1238 8.177163 74 41 0.6337482 0.5619046 0 0.6780248 1 1 
i 14 220.0186 13.09075 74 40 0.7782172 0.623769 0 0.7604047 1 1 
i 14 220.5217 8.177163 26 45 0.07439699 0.7028394 0 0.9077884 1 1 
i 4 220.9962 8.177163 29 45 0.2380232 0.1727303 0 0.1676529 1 1 
i 9 221.1159 8.177163 86 28 0.8500398 0.8342539 0 0.05573258 1 1 
i 13 221.5969 8.177163 38 33 0.787821 0.6758211 0 0.6389707 1 1 
i 11 222.3572 8.177163 41 43 0.7098061 0.4641154 0 0.5602594 1 1 
i 14 222.9552 13.70578 65 40 0.5231125 0.4493949 0 0.5593146 1 1 
i 14 223.3197 8.177163 50 45 0.5864313 0.6268136 0 0.7404811 1 1 
i 4 224.5379 8.177163 53 45 0.1707932 0.6602961 0 0.2757478 1 1 
i 14 226.0023 8.177163 75 40 0.2151196 0.3482192 0 0.5856992 1 1 
i 5 226.3604 8.177163 82 41 0.3598924 0.7468714 0 0.3158374 1 1 
i 9 226.8138 8.177163 87 28 0.4787014 0.8635457 0 0.5664993 1 1 
i 14 227.1113 12.92391 46 40 0.1930615 0.2954566 0 0.6769294 1 1 
i 14 229.8918 16.978 55 40 0.1752333 0.2242637 0 0.1460266 1 1 
i 16 230.0521 8.177163 82 47 0.3149899 0.3222223 0 0.05361956 1 1 
i 5 230.2438 8.177163 62 41 0.719577 0.1743393 0 0.2052973 1 1 
i 5 230.5763 8.177163 79 41 0.7734288 0.6358077 0 0.5171758 1 1 
i 16 230.6877 8.177163 91 47 0.6776312 0.1296666 0 0.8285189 1 1 
i 14 231.6899 12.19014 77 40 0.850257 0.4598195 0 0.5288127 1 1 
i 5 232.2553 8.177163 51 41 0.2130132 0.3410762 0 0.5532217 1 1 
i 16 232.3884 10.21318 82 47 0.389577 0.2745944 0 0.1421083 1 1 
i 9 232.7581 8.177163 58 28 0.277996 0.9382249 0 0.4262319 1 1 
i 27 232.8136 8.177163 87 40.75 0.7090583 0.4430099 0 0.3954573 1 1 
i 14 234.4867 9.422728 58 40 0.81393 0.3157288 0 0.1640553 1 1 
i 14 234.597 12.82404 75 40 0.7565999 0.4792373 0 0.948237 1 1 
i 16 234.7633 11.00621 70 47 0.07456958 0.7650209 0 0.9133818 1 1 
i 5 234.9551 8.177163 82 41 0.09231874 0.3519438 0 0.595344 1 1 
i 27 235.2432 9.757636 77 40.75 0.6722598 0.9266515 0 0.6570361 1 1 
i 5 235.3453 10.38884 91 41 0.2380053 0.2307305 0 0.8270144 1 1 
i 15 235.8506 8.177163 82 45 0.699008 0.8289836 0 0.06815133 1 1 
i 14 236.9505 16.55655 65 40 0.3319349 0.387186 0 0.4984748 1 1 
i 5 237.2624 8.177163 74 41 0.693447 0.706265 0 0.1634138 1 1 
i 9 237.5134 8.177163 77 28 0.3477087 0.770552 0 0.4223245 1 1 
i 14 237.5639 91.86396 74 40 0.603871 0.3462573 0 0.3325167 1 1 
i 9 237.6121 8.177163 87 28 0.6534036 0.7554602 0 0.5336878 1 1 
i 16 238.0944 10.42153 62 47 0.7068044 0.6994718 0 0.6255337 1 1 
i 27 238.6359 9.841308 65 40.75 0.1418091 0.6225791 0 0.1532506 1 1 
i 5 239.0447 8.177163 62 41 0.4702669 0.5504718 0 0.7192439 1 1 
i 5 239.2183 8.177163 79 41 0.7673583 0.1325493 0 0.5410744 1 1 
i 15 239.3214 9.106853 74 45 0.2352244 0.9234215 0 0.8607075 1 1 
i 9 239.4902 8.177163 67 28 0.7609746 0.7116048 0 0.407735 1 1 
i 5 239.7189 8.177163 70 41 0.1531264 0.8681923 0 0.18425 1 1 
i 16 240.1891 8.177163 77 47 0.167443 0.2432368 0 0.1124204 1 1 
i 5 241.2649 10.3201 69 41 0.5870692 0.1305774 0 0.9112492 1 1 
i 14 241.3174 43.31179 93 40 0.8200137 0.1587609 0 0.7900075 1 1 
i 9 241.7765 8.177163 76 28 0.655343 0.4748319 0 0.1976055 1 1 
i 14 242.1255 8.177163 69 40 0.2674323 0.7393493 0 0.533624 1 1 
i 16 242.7834 8.177163 50 47 0.2349299 0.700675 0 0.5757121 1 1 
i 5 242.835 16.74708 81 41 0.4615031 0.18003 0 0.4254335 1 1 
i 5 242.9041 60.62024 89 41 0.4558487 0.4062775 0 0.6118643 1 1 
i 27 243.3946 8.177163 57 40.75 0.7545448 0.5540959 0 0.3211138 1 1 
i 15 244.1682 8.177163 62 45 0.6819702 0.7756923 0 0.3592844 1 1 
i 16 245.1475 8.177163 69 47 0.3983046 0.06854423 0 0.3358021 1 1 
i 16 245.4312 8.177163 72 47 0.4419147 0.1711252 0 0.7217623 1 1 
i 14 246.1571 12.64982 76 40 0.9475489 0.05843436 0 0.3033708 1 1 
i 16 246.3871 8.177163 74 47 0.7588116 0.9234238 0 0.9303522 1 1 
i 9 247.4812 8.177163 86 28 0.3991436 0.1885572 0 0.8762859 1 1 
i 14 248.0644 8.177163 64 40 0.463769 0.5033401 0 0.1568003 1 1 
i 16 250.7195 8.177163 81 47 0.813692 0.5896006 0 0.1868169 1 1 
i 9 253.9632 8.177163 76 28 0.6885854 0.626709 0 0.1231223 1 1 
i 14 254.1147 8.177163 53 40 0.05304387 0.698613 0 0.8231246 1 1 
i 27 255.2412 8.177163 69 40.75 0.224303 0.9373787 0 0.3527939 1 1 
i 14 255.7037 8.177163 45 40 0.8691881 0.6028857 0 0.07271176 1 1 
i 5 256.1009 8.177163 50 41 0.1902175 0.1600396 0 0.720815 1 1 
i 9 258.2796 12.23609 86 28 0.1537681 0.1408205 0 0.8951706 1 1 
i 14 258.4138 14.33775 65 40 0.2033398 0.5002962 0 0.8207565 1 1 
i 14 258.7245 8.177163 96 40 0.5243431 0.3489627 0 0.4509754 1 1 
i 14 259.8218 9.422728 57 40 0.1882747 0.3337054 0 0.3334414 1 1 
i 5 259.8576 21.17251 69 41 0.4122829 0.8176287 0 0.8951744 1 1 
i 5 259.9356 63.4677 81 41 0.74225 0.4186919 0 0.3767939 1 1 
i 5 260.1738 9.422728 62 41 0.8326597 0.6271368 0 0.8845859 1 1 
i 14 261.6 12.0574 64 40 0.8866655 0.4221002 0 0.4441876 1 1 
i 14 263.3268 14.56443 77 40 0.367668 0.7778131 0 0.3519962 1 1 
i 9 264.7681 13.69956 76 28 0.1372785 0.3432427 0 0.6982247 1 1 
i 5 264.8862 8.177163 72 41 0.419347 0.05575593 0 0.8612873 1 1 
i 16 265.3755 8.177163 81 47 0.8552554 0.1060682 0 0.4635904 1 1 
i 9 266.1127 10.22531 65 28 0.43069 0.3536392 0 0.246029 1 1 
i 14 267.1915 8.177163 53 40 0.2734297 0.1376039 0 0.4556494 1 1 
i 5 267.6193 8.177163 60 41 0.425491 0.4386131 0 0.4472164 1 1 
i 16 268.8463 8.177163 72 47 0.05910333 0.1878477 0 0.7243321 1 1 
i 9 272.7468 8.177163 86 28 0.8683108 0.9071337 0 0.1167875 1 1 
i 14 274.5418 8.177163 45 40 0.1312721 0.5118529 0 0.07891114 1 1 
i 5 275.912 8.177163 74 41 0.1849302 0.1559737 0 0.8174524 1 1 
i 14 276.5137 10.40672 57 40 0.8719825 0.2713971 0 0.7613683 1 1 
i 14 277.5096 53.18203 64 40 0.3633039 0.9109059 0 0.6683532 1 1 
i 14 279.3647 10.29462 65 40 0.4246481 0.5788037 0 0.9273978 1 1 
i 14 279.3919 44.82258 96 40 0.5587731 0.05588769 0 0.7189958 1 1 
i 5 281.3888 12.53986 69 41 0.2985338 0.72145 0 0.8914597 1 1 
i 9 281.8687 11.744 76 28 0.5098856 0.7767941 0 0.4252769 1 1 
i 5 282.4511 9.795199 60 41 0.9437577 0.1901454 0 0.3078972 1 1 
i 14 282.6635 15.89516 77 40 0.7545891 0.2915861 0 0.1047362 1 1 
i 9 283.0059 8.177163 86 28 0.6125252 0.6891106 0 0.8394327 1 1 
i 14 283.6413 8.177163 53 40 0.6959171 0.3597 0 0.4501882 1 1 
i 9 284.6106 8.177163 65 28 0.5906407 0.2129679 0 0.4284173 1 1 
i 16 285.2961 8.177163 72 47 0.3150781 0.05994923 0 0.6266045 1 1 
i 16 286.0429 8.177163 81 47 0.7271143 0.6786144 0 0.4495693 1 1 
i 14 290.1925 8.177163 69 40 0.1364458 0.8339577 0 0.8211012 1 1 
i 5 294.8983 30.10469 69 41 0.4804022 0.3279563 0 0.5716889 1 1 
i 14 295.3584 9.455412 53 40 0.07044771 0.3300726 0 0.5370932 1 1 
i 9 296.9504 9.422728 76 28 0.3149649 0.7471302 0 0.7004371 1 1 
i 5 297.0644 8.177163 60 41 0.7151813 0.552381 0 0.2494089 1 1 
i 9 297.4731 8.177163 86 28 0.3915111 0.8429813 0 0.3439141 1 1 
i 9 297.606 8.177163 65 28 0.9041028 0.6328873 0 0.4977593 1 1 
i 16 300.3778 9.422728 72 47 0.9110416 0.1480875 0 0.8826124 1 1 
i 16 300.5101 8.177163 81 47 0.4683927 0.3883835 0 0.3977014 1 1 
i 16 301.4741 8.177163 62 47 0.2508715 0.9075279 0 0.8618501 1 1 
i 5 305.4743 15.26609 89 41 0.6500837 0.05961088 0 0.8937961 1 1 
i 14 306.9029 24.33754 53 40 0.0519917 0.06532187 0 0.07214455 1 1 
i 5 307.3308 18.36701 60 41 0.1418879 0.5947412 0 0.4000786 1 1 
i 9 307.5076 10.29462 76 28 0.595363 0.206192 0 0.6054103 1 1 
i 9 307.6002 8.177163 86 28 0.2796238 0.295796 0 0.08021787 1 1 
i 9 308.275 10.40672 65 28 0.6991737 0.3541916 0 0.9081277 1 1 
i 9 310.3866 8.177163 53 28 0.8379776 0.1470813 0 0.4545881 1 1 
i 5 317.4274 8.177163 50 41 0.7334494 0.2849276 0 0.7744147 1 1 
i 14 322.9897 8.177163 41 40 0.3629953 0.2672571 0 0.710292 1 1 
i 14 327.4 8.177163 96 40 0.8501101 0.2952306 0 0.7497919 1 1 
i 14 328.5165 11.63124 86 40 0.1824369 0.3120527 0 0.4624678 1 1 
i 5 328.7957 8.177163 93 41 0.7579137 0.7067528 0 0.6968425 1 1 
i 14 330.0756 13.82464 74 40 0.06480869 0.2986498 0 0.3933582 1 1 
i 5 330.3906 11.33218 81 41 0.3550139 0.5351823 0 0.802609 1 1 
i 9 330.7894 8.177163 86 28 0.8093162 0.4955364 0 0.8696802 1 1 
i 14 332.2624 14.76231 64 40 0.3801032 0.5533595 0 0.7589761 1 1 
i 5 332.6179 13.0599 69 41 0.5612444 0.3307731 0 0.1261972 1 1 
i 9 333.0679 10.90495 77 28 0.8925427 0.1158366 0 0.5264926 1 1 
i 16 333.6376 8.177163 81 47 0.6241736 0.6448717 0 0.5848727 1 1 
i 14 335.3407 14.33292 53 40 0.7927912 0.9436162 0 0.6034373 1 1 
i 5 335.7419 13.28892 60 41 0.5185686 0.7166151 0 0.6984021 1 1 
i 9 336.2498 11.96742 65 28 0.664281 0.1605834 0 0.6961605 1 1 
i 16 336.8927 10.29462 72 47 0.6497035 0.6572795 0 0.4755398 1 1 
i 14 337.7064 8.177163 77 40 0.3884917 0.1016168 0 0.6122308 1 1 
i 14 339.6866 12.28334 41 40 0.5303026 0.1528684 0 0.6391367 1 1 
i 5 340.1394 11.79819 50 41 0.9099213 0.9110252 0 0.8574563 1 1 
i 9 340.7126 11.18408 57 28 0.1805532 0.8655389 0 0.3887076 1 1 
i 16 341.4381 10.40672 62 47 0.5299835 0.6639487 0 0.6940033 1 1 
i 14 342.3565 9.422728 69 40 0.05383341 0.7915908 0 0.6629266 1 1 
i 5 343.5191 8.177163 74 41 0.8762353 0.7170815 0 0.5722141 1 1 
i 14 345.8369 8.177163 33 40 0.8034567 0.06880603 0 0.5873595 1 1 
i 5 346.3479 8.177163 38 41 0.3336543 0.2972918 0 0.05525575 1 1 
i 9 346.9948 8.177163 45 28 0.20163 0.4836747 0 0.3875947 1 1 
i 16 347.8136 8.177163 50 47 0.9283721 0.5119711 0 0.2010087 1 1 
i 14 348.8501 8.177163 57 40 0.7042418 0.8302575 0 0.4198792 1 1 
i 5 350.1621 8.177163 62 41 0.3770405 0.6621169 0 0.3159966 1 1 
i 9 351.8228 8.177163 69 28 0.6713062 0.4649534 0 0.1867908 1 1 

</CsScore>
</CsoundSynthesizer>
