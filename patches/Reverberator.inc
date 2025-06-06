gk_Reverberator_feedback chnexport "gk_Reverberator_feedback", 3
gk_Reverberator_wet chnexport "gk_Reverberator_wet", 3
gk_Reverberator_delay_modulation chnexport "gk_Reverberator_delay_modulation", 3
gk_Reverberator_frequency_cutoff chnexport "gk_Reverberator_frequency_cutoff", 3

gk_Reverberator_feedback init 0.875
gk_Reverberator_wet init 0.5
gk_Reverberator_delay_modulation init 0.0075
gk_Reverberator_frequency_cutoff init 15000

instr Reverberator
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Authors: Sean Costello and J. Lato
; Adapted by Michael Gogins
; p4 = gain of reverb. Adjust empirically
; for desired reverb time. .6 gives
; a good small "live" room sound, .8
; a small hall, .9 a large hall,
; .99 an enormous stone cavern.

; p5 = amount of random pitch modulation
; for the delay lines. 1 is the "normal"
; amount, but this may be too high for
; held pitches such as piano tones.
; Adjust to taste.

; p6 = cutoff frequency of lowpass filters
; in feedback loops of delay lines,
; in Hz. Lower cutoff frequencies results
; in a sound with more high-frequency
; damping.

; p7 = wet fraction (0 all dry, 1 all wet).

; 8 delay line FDN reverb, with feedback matrix based upon 
; physical modeling scattering junction of 8 lossless waveguides
; of equal characteristic impedance. Based on Julius O. Smith III, 
; "A New Approach to Digital Reverberation using Closed Waveguide
; Networks," Proceedings of the International Computer Music 
; Conference 1985, p. 47-53 (also available as a seperate
; publication from CCRMA), as well as some more recent papers by
; Smith and others.
kwet = gk_Reverberator_wet
kdry = 1. - gk_Reverberator_wet
kgain = gk_Reverberator_feedback 
kpitchmod = gk_Reverberator_delay_modulation
ktone = gk_Reverberator_frequency_cutoff 
ain1 inleta "inleft"
ain2 inleta "inright"
asignal = (ain1 + ain2) * 0.5
afilt1 init 0
afilt2 init 0
afilt3 init 0
afilt4 init 0
afilt5 init 0
afilt6 init 0
afilt7 init 0
afilt8 init 0
; TODO: should sr always be 44100 here?
idel1 = (2473.000/sr)
idel2 = (2767.000/sr)
idel3 = (3217.000/sr)
idel4 = (3557.000/sr)
idel5 = (3907.000/sr)
idel6 = (4127.000/sr)
idel7 = (2143.000/sr)
idel8 = (1933.000/sr)
; k1-k8 are used to add random pitch modulation to the
; delay lines. Helps eliminate metallic overtones
; in the reverb sound.
k1 randi .001, 3.1, .06
k2 randi .0011, 3.5, .9
k3 randi .0017, 1.11, .7
k4 randi .0006, 3.973, .3
k5 randi .001, 2.341, .63
k6 randi .0011, 1.897, .7
k7 randi .0017, 0.891, .9
k8 randi .0006, 3.221, .44
; apj is used to calculate "resultant junction pressure" for 
; the scattering junction of 8 lossless waveguides
; of equal characteristic impedance. If you wish to
; add more delay lines, simply add them to the following 
; equation, and replace the .25 by 2/N, where N is the 
; number of delay lines.
apj = .25 * (afilt1 + afilt2 + afilt3 + afilt4 + afilt5 + afilt6 + afilt7 + afilt8)

; aoutput vdelayxw asignal, adelayseconds, imaximumdelay, interpolationframes
adel1 vdelayxw asignal + apj - afilt1, a(idel1 + k1 * kpitchmod), 2, 8
adel2 vdelayxw asignal + apj - afilt2, a(idel2 + k2 * kpitchmod), 2, 8
adel3 vdelayxw asignal + apj - afilt3, a(idel3 + k3 * kpitchmod), 2, 8
adel4 vdelayxw asignal + apj - afilt4, a(idel4 + k4 * kpitchmod), 2, 8
adel5 vdelayxw asignal + apj - afilt5, a(idel5 + k5 * kpitchmod), 2, 8
adel6 vdelayxw asignal + apj - afilt6, a(idel6 + k6 * kpitchmod), 2, 8
adel7 vdelayxw asignal + apj - afilt7, a(idel7 + k7 * kpitchmod), 2, 8
adel8 vdelayxw asignal + apj - afilt8, a(idel8 + k8 * kpitchmod), 2, 8

; 1st order lowpass filters in feedback
; loops of delay lines.
afilt1 tone adel1 * kgain, ktone
afilt2 tone adel2 * kgain, ktone
afilt3 tone adel3 * kgain, ktone
afilt4 tone adel4 * kgain, ktone
afilt5 tone adel5 * kgain, ktone
afilt6 tone adel6 * kgain, ktone
afilt7 tone adel7 * kgain, ktone
afilt8 tone adel8 * kgain, ktone
; The outputs of the delay lines are summed
; and sent to the stereo outputs. This could
; easily be modified for a 4 or 8-channel 
; sound system.
aout1 = (afilt1 + afilt3 + afilt5 + afilt7)
aout2 = (afilt2 + afilt4 + afilt6 + afilt8)
; To the master output.
aoutleft = ain1 * kdry + aout1 * kwet
aoutright = ain2 * kdry + aout2 * kwet
outleta "outleft", aoutleft
outleta "outright", aoutright
prints  "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
printks "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d l%9.4f r%9.4f\n", 1, nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1), k(aoutleft), k(aoutright)
endin
