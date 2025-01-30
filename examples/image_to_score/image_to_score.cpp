#include <Composition.hpp>
#include <Eigen/Dense>
#include <functional>
#include <memory>
#include <MusicModel.hpp>
#include <random>
#include <ScoreNode.hpp>
#include <ImageToScore.hpp>
#include <VoiceleadingNode.hpp>
#include <vector>

/**
 * All composition and synthesis code is defined in the main function.
 * There is no need for any of this code to be in a separate file.
 */
int main(int argc, const char **argv)
{

    csound::MusicModel model;
    model.setCsoundOrchestra(R"(
sr = 48000
ksmps = 128
nchnls = 2
nchnls_i = 2
0dbfs = 1

connect "FMWaterBell", "outleft",  "ReverbSC", "inleft"
connect "FMWaterBell", "outright", "ReverbSC", "inright"
connect "Harpsichord", "outleft",  "ReverbSC", "inleft"
connect "Harpsichord", "outright", "ReverbSC", "inright"
connect "HeavyMetal", "outleft",  "ReverbSC", "inleft"
connect "HeavyMetal", "outright", "ReverbSC", "inright"
connect "Melody", "outleft",  "ReverbSC", "inleft"
connect "Melody", "outright", "ReverbSC", "inright"
connect "Sweeper", "outleft",  "ReverbSC", "inleft"
connect "Sweeper", "outright", "ReverbSC", "inright"
connect "WGPluck", "outleft",  "ReverbSC", "inleft"
connect "WGPluck", "outright", "ReverbSC", "inright"
connect "ZakianFlute", "outleft",  "ReverbSC", "inleft"
connect "ZakianFlute", "outright", "ReverbSC", "inright"
connect "ReverbSC", "outleft",  "MasterOutput", "inleft"
connect "ReverbSC", "outright", "MasterOutput", "inright"

alwayson "ReverbSC"
alwayson "MasterOutput"

//////////////////////////////////////////////
// Original by Steven Yi.
// Adapted by Michael Gogins.
//////////////////////////////////////////////
gk_FMWaterBell_level chnexport "gk_FMWaterBell_level", 3 ; 0
gi_FMWaterBell_attack chnexport "gi_FMWaterBell_attack", 3 ; 0.002
gi_FMWaterBell_release chnexport "gi_FMWaterBell_release", 3 ; 0.01
gi_FMWaterBell_sustain chnexport "gi_FMWaterBell_sustain", 3 ; 20
gi_FMWaterBell_sustain_level chnexport "gi_FMWaterBell_sustain_level", 3 ; .1
gk_FMWaterBell_index chnexport "gk_FMWaterBell_index", 3 ; .5
gk_FMWaterBell_crossfade chnexport "gk_FMWaterBell_crossfade", 3 ; .5
gk_FMWaterBell_vibrato_depth chnexport "gk_FMWaterBell_vibrato_depth", 3 ; 0.05
gk_FMWaterBell_vibrato_rate chnexport "gk_FMWaterBell_vibrato_rate", 3 ; 6
gk_FMWaterBell_midi_dynamic_range chnexport "gk_FMWaterBell_midi_dynamic_range", 3 ; 20

gk_FMWaterBell_level init 0
gi_FMWaterBell_attack init 0.002
gi_FMWaterBell_release init 0.01
gi_FMWaterBell_sustain init 20
gi_FMWaterBell_sustain_level init .1
gk_FMWaterBell_index init .5
gk_FMWaterBell_crossfade init .5
gk_FMWaterBell_vibrato_depth init 0.05
gk_FMWaterBell_vibrato_rate init 6
gk_FMWaterBell_midi_dynamic_range init 20

gk_FMWaterBell_space_left_to_right chnexport "gk_FMWaterBell_space_left_to_right", 3
gk_FMWaterBell_space_left_to_right init .5

gi_FMWaterBell_cosine ftgen 0, 0, 65537, 11, 1

instr FMWaterBell
i_instrument = p1
i_time = p2
i_duration = p3
; One of the envelopes in this instrument should be releasing, and use this:
i_sustain = 1000
xtratim gi_FMWaterBell_attack + gi_FMWaterBell_release
i_midi_key = p4
i_midi_dynamic_range = i(gk_FMWaterBell_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_FMWaterBell_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 80
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization * 1.6
k_gain = ampdb(gk_FMWaterBell_level)
i_releasing_attack = 3 / min(i_frequency, 256)
i_releasing_release = .01
a_signal fmbell	1, i_frequency, gk_FMWaterBell_index, gk_FMWaterBell_crossfade, gk_FMWaterBell_vibrato_depth, gk_FMWaterBell_vibrato_rate, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine, gi_FMWaterBell_cosine ;, gi_FMWaterBell_sustain
a_envelope transeg 0, gi_FMWaterBell_attack, 6,  1, gi_FMWaterBell_sustain, -6,  0
a_declicking cossegr 0, i_releasing_attack, 1, gi_FMWaterBell_sustain - 1, 1, i_releasing_release, 0
;;;a_signal = a_signal * i_amplitude * a_envelope * a_declicking * k_gain
a_signal = a_signal * i_amplitude * a_envelope * a_declicking * k_gain

a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
gk_Harpsichord_level chnexport "gk_Harpsichord_level", 3
gk_Harpsichord_pick chnexport "gk_Harpsichord_pick", 3
gk_Harpsichord_reflection chnexport "gk_Harpsichord_reflection", 3
gk_Harpsichord_pluck chnexport "gk_Harpsichord_pluck", 3
gk_Harpsichord_midi_dynamic_range chnexport "gk_Harpsichord_midi_dynamic_range", 3
gk_Harpsichord_space_left_to_right chnexport "gk_Harpsichord_space_left_to_right", 3

gk_Harpsichord_level init 0
gk_Harpsichord_pick init .075
gk_Harpsichord_reflection init .5
gk_Harpsichord_pluck init .75
gk_Harpsichord_midi_dynamic_range init 20
gk_Harpsichord_space_left_to_right init .5

gi_Harpsichord_harptable ftgen 0, 0, 65537, 7, -1, 1024, 1, 1024, -1

instr Harpsichord
i_instrument = p1
i_time = p2
; Make indefinite notes last no longer than the physical decay.
i_physical_decay = 40
if p3 == -1 then
i_duration = i_physical_decay
else
i_duration = p3
endif
i_midi_key = p4
i_midi_dynamic_range = i(gk_Harpsichord_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
if p7 == 0 then
k_space_left_to_right = gk_Harpsichord_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 66
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Harpsichord_level)
iHz = cpsmidinn(i_midi_key)
kHz = k(iHz)
a_physical_envelope transeg 1.0, i_physical_decay, -25.0, 0.0
apluck pluck i_amplitude * k_gain, kHz, iHz, 0, 1
aharp poscil a_physical_envelope, kHz, gi_Harpsichord_harptable
aharp2 balance apluck, aharp
a_signal	= (apluck + aharp2)
i_attack = .0005
i_sustain = p3
i_release = 0.01
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
i_declick_attack init .0008
i_declick_release init .01
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_duration, 1,  i_declick_release, 0
a_envelope = a_declicking_envelope
a_filtered_envelope tonex a_envelope, 40, 4
a_signal = a_signal * i_amplitude * a_filtered_envelope * k_gain 

a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
;printks "Harpsichord      %9.4f   %9.4f\n", 0.5, a_out_left, a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
gk_HeavyMetal_level chnexport "gk_HeavyMetal_level", 3
gk_HeavyMetal_level init 0
gk_HeavyMetal_midi_dynamic_range chnexport "gk_HeavyMetal_midi_dynamic_range", 3 ; 127
gk_HeavyMetal_midi_dynamic_range init 30
gk_HeavyMetal_space_left_to_right chnexport "gk_HeavyMetal_space_left_to_right", 3
gk_HeavyMetal_space_left_to_right init .5

gi_HeavyMetal_sine ftgen 0, 0, 65537, 10, 1
gi_HeavyMetal_cosine ftgen 0, 0, 65537, 11, 1
gi_HeavyMetal_exponentialrise ftgen 0, 0, 65536, 5, .001, 65536, 1 , 0; Exponential rise.
gi_HeavyMetal_thirteen ftgen 0, 0, 65537, 9, 1, .3, 0

instr HeavyMetal
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Author: Perry Cook and John ffitch
; Adapted by: Michael Gogins
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_dynamic_range = i(gk_HeavyMetal_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_HeavyMetal_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_overall_amps = (135 - 67) + 6
i_normalization = ampdb(-i_overall_amps) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
i_frequency = cpsmidinn(i_midi_key)
k_gain = ampdb(gk_HeavyMetal_level)
iindex = 1.1
icrossfade = 2
ivibedepth = 0.02
iviberate = 4.8
ifn1 = gi_HeavyMetal_sine
ifn2 = gi_HeavyMetal_exponentialrise
ifn3 = gi_HeavyMetal_thirteen
ifn4 = gi_HeavyMetal_sine
ivibefn = gi_HeavyMetal_cosine
iattack = 0.003
idecay = 3.0
isustain = p3
irelease = 0.05
xtratim iattack + idecay + irelease
adecay transeg 0.0, iattack, -4, 1.0, idecay, -4, 0.1, isustain, -4, 0.1, irelease, -4, 0.0
asignal fmmetal 1.0, i_frequency, iindex, icrossfade, ivibedepth, iviberate, ifn1, ifn2, ifn3, ifn4, ivibefn
adeclick linsegr 0, iattack, 1, isustain, 1, irelease, 0
a_signal = asignal * adeclick * i_amplitude * k_gain
i_attack = .002
i_sustain = p3
i_release = 0.01
xtratim i_attack + i_sustain + i_release
a_declicking linsegr 0, i_attack, 1, i_sustain, 1, i_release, 0
a_signal = a_signal * i_amplitude * a_declicking * k_gain

a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

gk_Melody_midi_dynamic_range chnexport "gk_Melody_midi_dynamic_range", 3 ; 127
gk_Melody_level chnexport "gk_Melody_level", 3 ; 0

gk_Melody_midi_dynamic_range init 20
gk_Melody_level init 30

gi_Melody_chebyshev ftgen 0, 0, 65537, -7, -1, 150, 0.1, 110, 0, 252, 0
gi_Melody_sine ftgen 0, 0, 65537, 10, 1
gi_Melody_cook3 ftgen 0, 0, 65537, 10, 1, .4, 0.2, 0.1, 0.1, .05
instr Melody
; Author: Jon Nelson
; Adapted by: Michael Gogins
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
i_midi_dynamic_range = i(gk_Melody_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
k_space_left_to_right = p7
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 67
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Melody_level)
ip3 init 3.0
iattack = 0.05
isustain = i_duration
irelease = 0.1
ip6 = gi_Melody_chebyshev
i1 = i_frequency
k100 randi 1,10
k101 poscil 1, 5 + k100, gi_Melody_sine
ak102 linseg 0, .5, 1, p3, 1
k100 = i1 + (k101 * ak102)
; Envelope for driving oscillator.
; k1 linenr 0.5, ip3 * .3, ip3 * 2, 0.01
k1 linseg 0, ip3 * .3, .5, ip3 * 2, 0.01, isustain, 0.01, irelease, 0
; k2 line 1, p3, .5
k2 linseg 1.0, ip3, .5, isustain, .5, irelease, 0
k1 = k2 * k1
; Amplitude envelope.
i_exponent = 4
a_physical_envelope transeg 0,   iattack, i_exponent,  1,   i_duration, 0, 1,  irelease, -i_exponent, 0

; Power to partials.
k20 linseg 1.485, iattack, 1.5, (isustain + irelease), 1.485
; a1-3 are for cheby with p6=1-4
a1 poscil k1, k100 - .025, gi_Melody_cook3
; Tables a1 to fn13, others normalize,
a2 tablei a1, ip6, 1, .5
a3 balance a2, a1
; Try other waveforms as well.
a4 foscili 1, k100 + .04, 1, 2.005, k20, gi_Melody_sine
a5 poscil 1, k100, gi_Melody_sine
a6 = ((a3 * .1) + (a4 * .1) + (a5 * .8)) * a_physical_envelope
a7 comb a6, .5, 1 / i1
a8 = (a6 * .9) + (a7 * .1)
a_signal balance a8, a1
; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .003
i_attack = .001 / i_frequency + i_declick_minimum
i_exponent = 7
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

aleft, aright pan2 a_signal * i_amplitude * k_gain, k_space_left_to_right

outleta "outleft", aleft
outleta "outright", aright
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
gk_Sweeper_midi_dynamic_range chnexport "gk_Sweeper_midi_dynamic_range", 3 ;  127
gk_Sweeper_attack chnexport "gk_Sweeper_attack", 3 ;  .125
gk_Sweeper_release chnexport "gk_Sweeper_release", 3 ;  .25
gk_Sweeper_britel chnexport "gk_Sweeper_britel", 3 ;  0.1
gk_Sweeper_briteh chnexport "gk_Sweeper_briteh", 3 ;  2.9
gk_Sweeper_britels chnexport "gk_Sweeper_britels", 3 ;  2
gk_Sweeper_britehs chnexport "gk_Sweeper_britehs", 3 ;  1
gk_Sweeper_level chnexport "gk_Sweeper_level", 3 ;  0

gk_Sweeper_midi_dynamic_range init 20
gk_Sweeper_attack init .125
gk_Sweeper_release init .25
gk_Sweeper_britel init .01
gk_Sweeper_briteh init 5
gk_Sweeper_britels init .5
gk_Sweeper_britehs init 1.75
gk_Sweeper_level init 0
gk_Sweeper_space_left_to_right chnexport "gk_Sweeper_space_left_to_right", 3
gk_Sweeper_space_left_to_right init .5

gi_Sweeper_sine ftgen 0, 0, 65537, 10, 1
gi_Sweeper_octfn ftgen 0, 0, 65537, -19, 1, 0.5, 270, 0.5

instr Sweeper
//////////////////////////////////////////////
// Original by Iain McCurdy.
// Adapted by Michael Gogins.
//////////////////////////////////////////////
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_dynamic_range = i(gk_Sweeper_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_Sweeper_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 34.2
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_Sweeper_level)

iattack = i(gk_Sweeper_attack)
irelease = i(gk_Sweeper_release)
isustain = p3
kenvelope transegr 0.0, iattack / 2.0, 1.5, i_amplitude / 2.0, iattack / 2.0, -1.5, i_amplitude, isustain, 0.0, i_amplitude, irelease / 2.0, 1.5, i_amplitude / 2.0, irelease / 2.0, -1.5, 0
ihertz = i_frequency
icps = ihertz
kamp expseg 0.001,0.02,0.2,p3-0.01,0.001
ktonemoddep jspline 0.01,0.05,0.2
ktonemodrte jspline 6,0.1,0.2
ktone poscil3 ktonemoddep, ktonemodrte, gi_Sweeper_sine
; kres rspline krangeMin, krangeMax, kcpsMin, kcpsMax
kbrite rspline gk_Sweeper_britel, gk_Sweeper_briteh, gk_Sweeper_britels, gk_Sweeper_britehs
ibasfreq init icps
ioctcnt init 3
iphs init 0
a1 hsboscil kenvelope, ktone, kbrite, ibasfreq, gi_Sweeper_sine, gi_Sweeper_octfn, ioctcnt, iphs
amod poscil3 0.25, ibasfreq*(1/3), gi_Sweeper_sine
arm = a1*amod
kmix expseg 0.001, 0.01, rnd(1), rnd(3)+0.3, 0.001
kmix=.25
a1 ntrpol a1, arm, kmix
kpanrte jspline 5, 0.05, 0.1
kpandep jspline 0.9, 0.2, 0.4
kpan poscil3 kpandep, kpanrte, gi_Sweeper_sine
;a1,a2 pan2 a1, kpan
a1,a2 pan2 a1, k_space_left_to_right
aleft delay a1, rnd(0.1)
aright delay a2, rnd(0.11)
a_signal = (aleft + aright)

; As with most software instruments that are modeled on an impulse exciting a 
; resonator, there should be two envelopes. The "physical" envelope must have a 
; fixed decay ending at zero.
i_declick_minimum = .003
i_attack = .001 / i_frequency + i_declick_minimum
i_declick_attack = i_attack
i_declick_release = i_declick_minimum * 2
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_duration, 1,  i_declick_release, 0
; The envelope of the instrument is the product of the physical envelope times 
; the declicking envelope. 
a_envelope = a_declicking_envelope
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_envelope, 40, 4
a_signal = a_signal * i_amplitude * a_filtered_envelope * k_gain *.001

prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
outs a_out_left, a_out_right
endin

gk_WGPluck_midi_dynamic_range chnexport "gk_WGPluck_midi_dynamic_range", 3 ;  127
gk_WGPluck_level chnexport "gk_WGPluck_level", 3 ;  0
gk_WGPluck_pick chnexport "gk_WGPluck_pick", 3 ;  .5
gk_WGPluck_reflection chnexport "gk_WGPluck_reflection", 3 ;  .5
gi_WGPluck_pluck chnexport "gi_WGPluck_pluck", 3 ;  0
gi_WGPluck_damping chnexport "gi_WGPluck_damping", 3 ;  10
gi_WGPluck_filter chnexport "gi_WGPluck_filter", 3 ;  1000

gk_WGPluck_midi_dynamic_range init 20
gk_WGPluck_level init 0
gk_WGPluck_pick init .5
gk_WGPluck_reflection init .5
gi_WGPluck_pluck init 0
gi_WGPluck_damping init 10
gi_WGPluck_filter init 1000

gk_WGPluck_space_left_to_right chnexport "gk_WGPluck_space_left_to_right", 3
gk_WGPluck_space_left_to_right init .5

gi_WGPluck_sine ftgen 0, 0, 65537, 10, 1, 0, 1

instr WGPluck
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_dynamic_range = i(gk_WGPluck_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.6 - i_midi_dynamic_range / 2)
k_space_front_to_back = p6
if p7 ==0 then
k_space_left_to_right = gk_WGPluck_space_left_to_right
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_frequency = cpsmidinn(i_midi_key)
; Adjust the following value until "overall amps" at the end of performance is about -6 dB.
i_level_correction = 71
i_normalization = ampdb(-i_level_correction) / 2
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_WGPluck_level)
iHz = cpsmidinn(i_midi_key)
kHz = k(iHz)
aenvelope transeg 1.0, 20.0, -10.0, 0.05
k_amplitude = 1
a_excitation poscil 1, 1, gi_WGPluck_sine
a_signal wgpluck i_frequency, i_amplitude, gk_WGPluck_pick, gi_WGPluck_pluck, gi_WGPluck_damping, gi_WGPluck_filter, a_excitation
i_attack = .002
i_sustain = p3
i_release = 0.01
xtratim i_attack + i_release
a_declicking linsegr 0, i_attack, 1, i_sustain, 1, i_release, 0
a_signal = a_signal * i_amplitude * a_declicking * k_gain

a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
;printks "WGPluck      %9.4f   %9.4f\n", 0.5, a_out_left, a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
gk_ZakianFlute_midi_dynamic_range chnexport "gk_ZakianFlute_midi_dynamic_range", 3 ;  20
gk_ZakianFlute_level chnexport "gk_ZakianFlute_level", 3 ;  0
gk_ZakianFlute_pan chnexport "gk_ZakianFlute_pan", 3 ;  .5
gi_ZakianFLute_seed chnexport "gi_ZakianFLute_seed", 3 ;  .5
gi_ZakianFLute_space_left_to_front chnexport "gi_ZakianFLute_space_left_to_front", 3 ;  .5

gk_ZakianFlute_midi_dynamic_range init 20
gk_ZakianFlute_level init 0
gk_ZakianFlute_pan init .5
gi_ZakianFLute_seed init .5
gi_ZakianFLute_space_left_to_front init .5

gi_ZakianFLute_f2  ftgen 0, 0, 16, -2, 40, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 10240
gi_ZakianFlute_f26 ftgen 0, 0, 65537, -10, 2000, 489, 74, 219, 125, 9, 33, 5, 5
gi_ZakianFlute_f27 ftgen 0, 0, 65537, -10, 2729, 1926, 346, 662, 537, 110, 61, 29, 7
gi_ZakianFlute_f28 ftgen 0, 0, 65537, -10, 2558, 2012, 390, 361, 534, 139, 53, 22, 10, 13, 10
gi_ZakianFlute_f29 ftgen 0, 0, 65537, -10, 12318, 8844, 1841, 1636, 256, 150, 60, 46, 11
gi_ZakianFlute_f30 ftgen 0, 0, 65537, -10, 1229, 16, 34, 57, 32
gi_ZakianFlute_f31 ftgen 0, 0, 65537, -10, 163, 31, 1, 50, 31
gi_ZakianFlute_f32 ftgen 0, 0, 65537, -10, 4128, 883, 354, 79, 59, 23
gi_ZakianFlute_f33 ftgen 0, 0, 65537, -10, 1924, 930, 251, 50, 25, 14
gi_ZakianFlute_f34 ftgen 0, 0, 65537, -10, 94, 6, 22, 8
gi_ZakianFlute_f35 ftgen 0, 0, 65537, -10, 2661, 87, 33, 18
gi_ZakianFlute_f36 ftgen 0, 0, 65537, -10, 174, 12
gi_ZakianFlute_f37 ftgen 0, 0, 65537, -10, 314, 13
gi_ZakianFlute_wtsin ftgen 0, 0, 65537, 10, 1

instr ZakianFlute
; Author: Lee Zakian
; Adapted by: Michael Gogins
i_instrument = p1
i_time = p2
if p3 == -1 then
i_duration = 1000
else
i_duration = p3
endif
i_midi_key = p4
i_midi_velocity = p5
k_space_front_to_back = p6
if p7 == 0 then
k_space_left_to_right = gi_ZakianFLute_space_left_to_front
else
k_space_left_to_right = p7
endif
k_space_bottom_to_top = p8
i_phase = p9
i_overall_amps = 65.2
i_normalization = ampdb(-i_overall_amps) / 2
i_midi_dynamic_range = i(gk_ZakianFlute_midi_dynamic_range)
i_midi_velocity = p5 * i_midi_dynamic_range / 127 + (63.5 - i_midi_dynamic_range / 2)
i_amplitude = ampdb(i_midi_velocity) * i_normalization
k_gain = ampdb(gk_ZakianFlute_level)
;;;xtratim iattack + irelease
iHz = cpsmidinn(i_midi_key)
kHz = k(iHz)
// Bug?
aenvelope transeg 1.0, 20.0, -10.0, 0.05
///aenvelope transegr 1.0, 20.0, -10.0, 0.05
ip3 = 3;;; (p3 < 3.0 ? p3 : 3.0)
; parameters
; p4 overall amplitude scaling factor
ip4 init i_amplitude
; p5 pitch in Hertz (normal pitch range: C4-C7)
ip5 init iHz
; p6 percent vibrato depth, recommended values in range [-1., +1.]
ip6 init 0.5
; 0.0 -> no vibrato
; +1. -> 1% vibrato depth, where vibrato rate increases slightly
; -1. -> 1% vibrato depth, where vibrato rate decreases slightly
; p7 attack time in seconds
; recommended value: .12 for slurred notes, .06 for tongued notes
; (.03 for short notes)
ip7 init .08
; p8 decay time in seconds
; recommended value: .1 (.05 for short notes)
ip8 init .08
; p9 overall brightness / filter cutoff factor
; 1 -> least bright / minimum filter cutoff frequency (40 Hz)
; 9 -> brightest / maximum filter cutoff frequency (10,240Hz)
ip9 init 5
; initial variables
iampscale = ip4 ; overall amplitude scaling factor
ifreq = ip5 ; pitch in Hertz
ivibdepth = abs(ip6*ifreq/100.0) ; vibrato depth relative to fundamental frequency
iattack = ip7 * (1.1 - .2*gi_ZakianFLute_seed) ; attack time with up to +-10% random deviation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947) ; reset gi_ZakianFLute_seed
idecay = ip8 * (1.1 - .2*gi_ZakianFLute_seed) ; decay time with up to +-10% random deviation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifiltcut tablei ip9, gi_ZakianFLute_f2 ; lowpass filter cutoff frequency
iattack = (iattack < 6/kr ? 6/kr : iattack) ; minimal attack length
idecay = (idecay < 6/kr ? 6/kr : idecay) ; minimal decay length
isustain = i_duration - iattack - idecay
;;;p3 = (isustain < 5/kr ? iattack+idecay+5/kr : i_duration) ; minimal sustain length
isustain = (isustain < 5/kr ? 5/kr : isustain)
iatt = iattack/6
isus = isustain/4
idec = idecay/6
iphase = gi_ZakianFLute_seed ; use same phase for all wavetables
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
; vibrato block
; kvibdepth linseg .1, .8*p3, 1, .2*p3, .7
kvibdepth linseg .1, .8*ip3, 1, isustain, 1, .2*ip3, .7
kvibdepth = kvibdepth* ivibdepth ; vibrato depth
kvibdepthr randi .1*kvibdepth, 5, gi_ZakianFLute_seed ; up to 10% vibrato depth variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kvibdepth = kvibdepth + kvibdepthr
ivibr1 = gi_ZakianFLute_seed ; vibrato rate
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ivibr2 = gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
if ip6 < 0 goto vibrato1
kvibrate linseg 2.5+ivibr1, isustain, 4.5+ivibr2 ; if p6 positive vibrato gets faster
goto vibrato2
vibrato1:
ivibr3 = gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kvibrate linseg 3.5+ivibr1, .1, 4.5+ivibr2,isustain-.1, 2.5+ivibr3 ; if p6 negative vibrato gets slower
vibrato2:
kvibrater randi .1*kvibrate, 5, gi_ZakianFLute_seed ; up to 10% vibrato rate variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kvibrate = kvibrate + kvibrater
kvib oscili kvibdepth, kvibrate, gi_ZakianFlute_wtsin
ifdev1 = -.03 * gi_ZakianFLute_seed ; frequency deviation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifdev2 = .003 * gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifdev3 = -.0015 * gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
ifdev4 = .012 * gi_ZakianFLute_seed
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kfreqr linseg ifdev1, iattack, ifdev2, isustain, ifdev3, idecay, ifdev4
kfreq = kHz * (1 + kfreqr) + kvib
if ifreq < 427.28 goto range1 ; (cpspch(8.08) + cpspch(8.09))/2
if ifreq < 608.22 goto range2 ; (cpspch(9.02) + cpspch(9.03))/2
if ifreq < 1013.7 goto range3 ; (cpspch(9.11) + cpspch(10.00))/2
goto range4
; wavetable amplitude envelopes
range1: ; for low range tones
kamp1 linseg 0, iatt, 0.002, iatt, 0.045, iatt, 0.146, iatt, \
0.272, iatt, 0.072, iatt, 0.043, isus, 0.230, isus, 0.000, isus, \
0.118, isus, 0.923, idec, 1.191, idec, 0.794, idec, 0.418, idec, \
0.172, idec, 0.053, idec, 0
kamp2 linseg 0, iatt, 0.009, iatt, 0.022, iatt, -0.049, iatt, \
-0.120, iatt, 0.297, iatt, 1.890, isus, 1.543, isus, 0.000, isus, \
0.546, isus, 0.690, idec, -0.318, idec, -0.326, idec, -0.116, idec, \
-0.035, idec, -0.020, idec, 0
kamp3 linseg 0, iatt, 0.005, iatt, -0.026, iatt, 0.023, iatt, \
0.133, iatt, 0.060, iatt, -1.245, isus, -0.760, isus, 1.000, isus, \
0.360, isus, -0.526, idec, 0.165, idec, 0.184, idec, 0.060, idec, \
0.010, idec, 0.013, idec, 0
iwt1 = gi_ZakianFlute_f26 ; wavetable numbers
iwt2 = gi_ZakianFlute_f27
iwt3 = gi_ZakianFlute_f28
inorm = 3949
goto end
range2: ; for low mid-range tones
kamp1 linseg 0, iatt, 0.000, iatt, -0.005, iatt, 0.000, iatt, \
0.030, iatt, 0.198, iatt, 0.664, isus, 1.451, isus, 1.782, isus, \
1.316, isus, 0.817, idec, 0.284, idec, 0.171, idec, 0.082, idec, \
0.037, idec, 0.012, idec, 0
kamp2 linseg 0, iatt, 0.000, iatt, 0.320, iatt, 0.882, iatt, \
1.863, iatt, 4.175, iatt, 4.355, isus, -5.329, isus, -8.303, isus, \
-1.480, isus, -0.472, idec, 1.819, idec, -0.135, idec, -0.082, idec, \
-0.170, idec, -0.065, idec, 0
kamp3 linseg 0, iatt, 1.000, iatt, 0.520, iatt, -0.303, iatt, \
0.059, iatt, -4.103, iatt, -6.784, isus, 7.006, isus, 11, isus, \
12.495, isus, -0.562, idec, -4.946, idec, -0.587, idec, 0.440, idec, \
0.174, idec, -0.027, idec, 0
iwt1 = gi_ZakianFlute_f29
iwt2 = gi_ZakianFlute_f30
iwt3 = gi_ZakianFlute_f31
inorm = 27668.2
goto end
range3: ; for high mid-range tones
kamp1 linseg 0, iatt, 0.005, iatt, 0.000, iatt, -0.082, iatt, \
0.36, iatt, 0.581, iatt, 0.416, isus, 1.073, isus, 0.000, isus, \
0.356, isus, .86, idec, 0.532, idec, 0.162, idec, 0.076, idec, 0.064, \
idec, 0.031, idec, 0
kamp2 linseg 0, iatt, -0.005, iatt, 0.000, iatt, 0.205, iatt, \
-0.284, iatt, -0.208, iatt, 0.326, isus, -0.401, isus, 1.540, isus, \
0.589, isus, -0.486, idec, -0.016, idec, 0.141, idec, 0.105, idec, \
-0.003, idec, -0.023, idec, 0
kamp3 linseg 0, iatt, 0.722, iatt, 1.500, iatt, 3.697, iatt, \
0.080, iatt, -2.327, iatt, -0.684, isus, -2.638, isus, 0.000, isus, \
1.347, isus, 0.485, idec, -0.419, idec, -.700, idec, -0.278, idec, \
0.167, idec, -0.059, idec, 0
iwt1 = gi_ZakianFlute_f32
iwt2 = gi_ZakianFlute_f33
iwt3 = gi_ZakianFlute_f34
inorm = 3775
goto end
range4: ; for high range tones
kamp1 linseg 0, iatt, 0.000, iatt, 0.000, iatt, 0.211, iatt, \
0.526, iatt, 0.989, iatt, 1.216, isus, 1.727, isus, 1.881, isus, \
1.462, isus, 1.28, idec, 0.75, idec, 0.34, idec, 0.154, idec, 0.122, \
idec, 0.028, idec, 0
kamp2 linseg 0, iatt, 0.500, iatt, 0.000, iatt, 0.181, iatt, \
0.859, iatt, -0.205, iatt, -0.430, isus, -0.725, isus, -0.544, isus, \
-0.436, isus, -0.109, idec, -0.03, idec, -0.022, idec, -0.046, idec, \
-0.071, idec, -0.019, idec, 0
kamp3 linseg 0, iatt, 0.000, iatt, 1.000, iatt, 0.426, iatt, \
0.222, iatt, 0.175, iatt, -0.153, isus, 0.355, isus, 0.175, isus, \
0.16, isus, -0.246, idec, -0.045, idec, -0.072, idec, 0.057, idec, \
-0.024, idec, 0.002, idec, 0
iwt1 = gi_ZakianFlute_f35
iwt2 = gi_ZakianFlute_f36
iwt3 = gi_ZakianFlute_f37
inorm = 4909.05
goto end
end:
kampr1 randi .02*kamp1, 10, gi_ZakianFLute_seed ; up to 2% wavetable amplitude variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kamp1 = kamp1 + kampr1
kampr2 randi .02*kamp2, 10, gi_ZakianFLute_seed ; up to 2% wavetable amplitude variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kamp2 = kamp2 + kampr2
kampr3 randi .02*kamp3, 10, gi_ZakianFLute_seed ; up to 2% wavetable amplitude variation
gi_ZakianFLute_seed = frac(gi_ZakianFLute_seed*105.947)
kamp3 = kamp3 + kampr3
awt1 poscil kamp1, kfreq, iwt1, iphase ; wavetable lookup
awt2 poscil kamp2, kfreq, iwt2, iphase
awt3 poscil kamp3, kfreq, iwt3, iphase
asig = awt1 + awt2 + awt3
asig = asig*(iampscale/inorm)
kcut linseg 0, iattack, ifiltcut, isustain, ifiltcut, idecay, 0 ; lowpass filter for brightness control
afilt tone asig, kcut
a_signal balance afilt, asig
i_attack = .002
i_sustain = i_duration
i_release = 0.01
i_declick_attack = i_attack
i_declick_release = i_declick_attack * 2
; The end of the note must be extended _past_ the end of the release segment.
xtratim 1
a_declicking_envelope cossegr 0, i_declick_attack, 1,  i_duration, 1,  i_declick_release, 0
; That envelope is then low-pass filtered to remove most discontinuities.
a_filtered_envelope tonex a_declicking_envelope, 40, 4
a_signal = a_signal * i_amplitude * a_filtered_envelope * k_gain 

prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)

a_signal *= .7
a_out_left, a_out_right pan2 a_signal, k_space_left_to_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
endin
gk_ReverbSC_feedback chnexport "gk_ReverbSC_feedback", 3
gk_ReverbSC_wet chnexport "gk_ReverbSC_wet", 3
gi_ReverbSC_delay_modulation chnexport "gi_ReverbSC_delay_modulation", 3
gk_ReverbSC_frequency_cutoff chnexport "gk_ReverbSC_frequency_cutoff", 3

gk_ReverbSC_feedback init 0.875
gk_ReverbSC_wet init 0.5
gi_ReverbSC_delay_modulation init 0.0075
gk_ReverbSC_frequency_cutoff init 15000

instr ReverbSC
gk_ReverbSC_dry = 1.0 - gk_ReverbSC_wet
aleftin init 0
arightin init 0
aleftout init 0
arightout init 0
aleftin inleta "inleft"
arightin inleta "inright"
aleftout, arightout reverbsc aleftin, arightin, gk_ReverbSC_feedback, gk_ReverbSC_frequency_cutoff, sr, gi_ReverbSC_delay_modulation
aleftoutmix = aleftin * gk_ReverbSC_dry + aleftout * gk_ReverbSC_wet
arightoutmix = arightin * gk_ReverbSC_dry + arightout * gk_ReverbSC_wet
outleta "outleft", aleftoutmix
outleta "outright", arightoutmix
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

gk_MasterOutput_level chnexport "gk_MasterOutput_level", 3 ; 0
gS_MasterOutput_filename chnexport "gS_MasterOutput_filename", 3 ; ""

gk_MasterOutput_level init 0
gS_MasterOutput_filename init ""

instr MasterOutput
aleft inleta "inleft"
aright inleta "inright"
k_gain = ampdb(gk_MasterOutput_level)
printks2 "Master gain: %f\n", k_gain
iamp init 1
aleft butterlp aleft, 18000
aright butterlp aright, 18000
outs aleft * k_gain, aright * k_gain
; We want something that will play on my phone.
i_amplitude_adjustment = ampdbfs(-3) / 32767
i_filename_length strlen gS_MasterOutput_filename
if i_filename_length > 0 then
prints sprintf("Output filename: %s\n", gS_MasterOutput_filename)
fout gS_MasterOutput_filename, 18, aleft * i_amplitude_adjustment, aright * i_amplitude_adjustment
endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

    )");
    // These fields determine output filenames and ID 3 tags.
    csound::System::setMessageLevel(15);
    model.setAuthor("Michael Gogins");
    model.setTitle("image_to_score");
    model.setAlbum("Silence");
    model.setYear("2022");
    model.setPerformanceRightsOrganization("Irreducible Productions, ASCAP");
    csound::Rescale rescale_node;
    model.addChild(&rescale_node);
    csound::ImageToScore2 image_to_score_node;
    rescale_node.addChild(&image_to_score_node);
    //image_to_score_node.setImageFilename("./43636356874_79c6d44f79_o.jpg");
    //image_to_score_node.setImageFilename("./51735821033_f930358ef4_o.jpg");
    image_to_score_node.setImageFilename("./44025833484_70440d3a59_o.jpg");
    image_to_score_node.threshhold(50);
    image_to_score_node.setMaximumVoiceCount(8);
    //image_to_score_node.condense(72);
    image_to_score_node.generateLocally();
    csound::Score &image_score = image_to_score_node.getScore();
    std::mt19937 mersenneTwister;
    std::uniform_real_distribution<> randomvariable(.1, .9);
    rescale_node.setRescale(csound::Event::TIME, true, false, 0., 0.);
    rescale_node.setRescale(csound::Event::INSTRUMENT, true, true, 1., 6.999);
    rescale_node.setRescale(csound::Event::KEY, true, true, 36., 60.);
    rescale_node.setRescale(csound::Event::VELOCITY, true, true, 60., 10.);
    model.generateAllNames();
    model.generate();
    csound::Score &score = model.getScore();
    for (int i = 0, n = image_score.size(); i < n; ++i) {
         score[i].setPan(randomvariable(mersenneTwister));
         score[i].setDuration(score[i].getDuration() * 10);
    }
    score.temper(12.);
    std::cout << "Move to origin duration:         " << score.getDuration() << std::endl;
    score.setDuration(240.0);
    std::cout << "set duration:                    " << score.getDuration() << std::endl;
    std::cout << "Before tieing overlapping notes: " << score.size() << std::endl;
    score.tieOverlappingNotes(true);
    std::cout << "After tieing overlapping notes:  " << score.size() << std::endl;
    score.findScale();
    std::cout << "set duration:                    " << score.getDuration() << std::endl;
    std::cout << "Generated score:" << std::endl << score.getCsoundScore() << std::endl;
    // We have post-processed the generated score, therefore we call perform 
    // here, rather than render.
    model.perform();
}

