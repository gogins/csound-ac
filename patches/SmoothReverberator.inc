; SmoothReverberator.inc
; Self-contained stereo, modulated eight-line feedback-delay-network reverb.
; This module can coexist with patches/Reverberator.inc because the file,
; exported controls, UDOs, and instrument all use the SmoothReverberator name.
;
; Typical modular-patch connections:
;
;   connect "Source", "outleft",  "SmoothReverberator", "inleft"
;   connect "Source", "outright", "SmoothReverberator", "inright"
;   connect "SmoothReverberator", "outleft",  "MasterOutput", "inleft"
;   connect "SmoothReverberator", "outright", "MasterOutput", "inright"
;   alwayson "SmoothReverberator"
;
; The implementation uses no reverbsc, freeverb, convolution, or plug-in opcodes.
; The late field is an eight-line unitary Hadamard FDN with line-specific RT60
; gains, feedback damping, slow delay modulation, stereo input diffusion, and
; orthogonal stereo output projections.
;
; Control channels:
;   gk_SmoothReverberator_decay_time       nominal low-frequency RT60, seconds
;   gk_SmoothReverberator_wet              wet fraction, 0 to 1
;   gk_SmoothReverberator_delay_modulation peak delay modulation, seconds
;   gk_SmoothReverberator_modulation_rate  basic modulation rate, Hz
;   gk_SmoothReverberator_frequency_cutoff feedback low-pass cutoff, Hz
;   gk_SmoothReverberator_stereo_width     wet-signal M/S width, 0 to 2
;   gk_SmoothReverberator_input_highpass   input and wet-output high-pass, Hz
;   gk_SmoothReverberator_reverb_gain      gain applied to the wet field

gk_SmoothReverberator_decay_time chnexport "gk_SmoothReverberator_decay_time", 3
gk_SmoothReverberator_wet chnexport "gk_SmoothReverberator_wet", 3
gk_SmoothReverberator_delay_modulation chnexport "gk_SmoothReverberator_delay_modulation", 3
gk_SmoothReverberator_modulation_rate chnexport "gk_SmoothReverberator_modulation_rate", 3
gk_SmoothReverberator_frequency_cutoff chnexport "gk_SmoothReverberator_frequency_cutoff", 3
gk_SmoothReverberator_stereo_width chnexport "gk_SmoothReverberator_stereo_width", 3
gk_SmoothReverberator_input_highpass chnexport "gk_SmoothReverberator_input_highpass", 3
gk_SmoothReverberator_reverb_gain chnexport "gk_SmoothReverberator_reverb_gain", 3

gk_SmoothReverberator_decay_time init 22
gk_SmoothReverberator_wet init 0.5
gk_SmoothReverberator_delay_modulation init 0.00065
gk_SmoothReverberator_modulation_rate init 0.17
gk_SmoothReverberator_frequency_cutoff init 7500
gk_SmoothReverberator_stereo_width init 1.15
gk_SmoothReverberator_input_highpass init 25
gk_SmoothReverberator_reverb_gain init 0.8

opcode SmoothReverberatorDiffuser, a, aik
    aInput, iDelayTime, kFeedback xin
    aDelayBuffer delayr iDelayTime + 0.01
    aDelayed deltap3 iDelayTime
    aOutput = aDelayed - (kFeedback * aInput)
    delayw aInput + (kFeedback * aOutput)
    xout aOutput
endop

opcode SmoothReverberator, aa, aakkkkkkk
    aInputLeft, aInputRight, kDecayTime, kModulationDepth, kModulationRate, kCutoff, kStereoWidth, kInputHighpass, kReverbGain xin
    aHighpassedLeft butterhp aInputLeft, kInputHighpass
    aHighpassedRight butterhp aInputRight, kInputHighpass
    aDiffusedLeft1 SmoothReverberatorDiffuser aHighpassedLeft, 0.004771, 0.72
    aDiffusedLeft2 SmoothReverberatorDiffuser aDiffusedLeft1, 0.007921, 0.70
    aDiffusedLeft3 SmoothReverberatorDiffuser aDiffusedLeft2, 0.012347, 0.68
    aDiffusedRight1 SmoothReverberatorDiffuser aHighpassedRight, 0.005317, 0.72
    aDiffusedRight2 SmoothReverberatorDiffuser aDiffusedRight1, 0.008879, 0.70
    aDiffusedRight3 SmoothReverberatorDiffuser aDiffusedRight2, 0.013681, 0.68
    aMidInput = (aDiffusedLeft3 + aDiffusedRight3) * 0.5
    aSideInput = (aDiffusedLeft3 - aDiffusedRight3) * 0.5
    aModulation1 oscili kModulationDepth, kModulationRate * 0.731, -1, 0.00
    aModulation2 oscili kModulationDepth, kModulationRate * 0.853, -1, 0.13
    aModulation3 oscili kModulationDepth, kModulationRate * 0.967, -1, 0.27
    aModulation4 oscili kModulationDepth, kModulationRate * 1.091, -1, 0.39
    aModulation5 oscili kModulationDepth, kModulationRate * 1.213, -1, 0.53
    aModulation6 oscili kModulationDepth, kModulationRate * 1.337, -1, 0.66
    aModulation7 oscili kModulationDepth, kModulationRate * 1.459, -1, 0.79
    aModulation8 oscili kModulationDepth, kModulationRate * 1.577, -1, 0.91
    iDelay1 = 0.031103
    iDelay2 = 0.037853
    iDelay3 = 0.043273
    iDelay4 = 0.047921
    iDelay5 = 0.053911
    iDelay6 = 0.059317
    iDelay7 = 0.067733
    iDelay8 = 0.073291
    aState1 init 0
    aState2 init 0
    aState3 init 0
    aState4 init 0
    aState5 init 0
    aState6 init 0
    aState7 init 0
    aState8 init 0
    iHadamardNormalization = 0.3535533905932738
    aMatrix1 = (aState1 + aState2 + aState3 + aState4 + aState5 + aState6 + aState7 + aState8) * iHadamardNormalization
    aMatrix2 = (aState1 - aState2 + aState3 - aState4 + aState5 - aState6 + aState7 - aState8) * iHadamardNormalization
    aMatrix3 = (aState1 + aState2 - aState3 - aState4 + aState5 + aState6 - aState7 - aState8) * iHadamardNormalization
    aMatrix4 = (aState1 - aState2 - aState3 + aState4 + aState5 - aState6 - aState7 + aState8) * iHadamardNormalization
    aMatrix5 = (aState1 + aState2 + aState3 + aState4 - aState5 - aState6 - aState7 - aState8) * iHadamardNormalization
    aMatrix6 = (aState1 - aState2 + aState3 - aState4 - aState5 + aState6 - aState7 + aState8) * iHadamardNormalization
    aMatrix7 = (aState1 + aState2 - aState3 - aState4 - aState5 - aState6 + aState7 + aState8) * iHadamardNormalization
    aMatrix8 = (aState1 - aState2 - aState3 + aState4 - aState5 + aState6 + aState7 - aState8) * iHadamardNormalization
    iInjectionGain = 0.18
    aInjection1 = (aMidInput + aSideInput) * iInjectionGain
    aInjection2 = (aMidInput - aSideInput) * iInjectionGain
    aInjection3 = (-aMidInput + aSideInput) * iInjectionGain
    aInjection4 = (-aMidInput - aSideInput) * iInjectionGain
    aInjection5 = (aMidInput + (0.5 * aSideInput)) * iInjectionGain
    aInjection6 = ((0.5 * aMidInput) - aSideInput) * iInjectionGain
    aInjection7 = ((-0.5 * aMidInput) - aSideInput) * iInjectionGain
    aInjection8 = (-aMidInput + (0.5 * aSideInput)) * iInjectionGain
    aDelay1 vdelayxw aMatrix1 + aInjection1, iDelay1 + aModulation1, 0.10, 16
    aDelay2 vdelayxw aMatrix2 + aInjection2, iDelay2 + aModulation2, 0.10, 16
    aDelay3 vdelayxw aMatrix3 + aInjection3, iDelay3 + aModulation3, 0.10, 16
    aDelay4 vdelayxw aMatrix4 + aInjection4, iDelay4 + aModulation4, 0.10, 16
    aDelay5 vdelayxw aMatrix5 + aInjection5, iDelay5 + aModulation5, 0.10, 16
    aDelay6 vdelayxw aMatrix6 + aInjection6, iDelay6 + aModulation6, 0.10, 16
    aDelay7 vdelayxw aMatrix7 + aInjection7, iDelay7 + aModulation7, 0.10, 16
    aDelay8 vdelayxw aMatrix8 + aInjection8, iDelay8 + aModulation8, 0.10, 16
    aDamped1 tone aDelay1, kCutoff
    aDamped2 tone aDelay2, kCutoff * 0.97
    aDamped3 tone aDelay3, kCutoff * 1.03
    aDamped4 tone aDelay4, kCutoff * 0.94
    aDamped5 tone aDelay5, kCutoff * 1.06
    aDamped6 tone aDelay6, kCutoff * 0.91
    aDamped7 tone aDelay7, kCutoff * 1.09
    aDamped8 tone aDelay8, kCutoff * 0.88
    kLog001 = -6.907755278982137
    kGain1 = exp(kLog001 * iDelay1 / kDecayTime)
    kGain2 = exp(kLog001 * iDelay2 / kDecayTime)
    kGain3 = exp(kLog001 * iDelay3 / kDecayTime)
    kGain4 = exp(kLog001 * iDelay4 / kDecayTime)
    kGain5 = exp(kLog001 * iDelay5 / kDecayTime)
    kGain6 = exp(kLog001 * iDelay6 / kDecayTime)
    kGain7 = exp(kLog001 * iDelay7 / kDecayTime)
    kGain8 = exp(kLog001 * iDelay8 / kDecayTime)
    aState1 = aDamped1 * kGain1
    aState2 = aDamped2 * kGain2
    aState3 = aDamped3 * kGain3
    aState4 = aDamped4 * kGain4
    aState5 = aDamped5 * kGain5
    aState6 = aDamped6 * kGain6
    aState7 = aDamped7 * kGain7
    aState8 = aDamped8 * kGain8
    iOutputProjection = 0.3535533905932738
    aWetRawLeft = (aDelay1 - aDelay2 + aDelay3 + aDelay4 - aDelay5 + aDelay6 + aDelay7 - aDelay8) * iOutputProjection
    aWetRawRight = (aDelay1 + aDelay2 - aDelay3 + aDelay4 + aDelay5 - aDelay6 + aDelay7 - aDelay8) * iOutputProjection
    aWetHighpassedLeft butterhp aWetRawLeft, kInputHighpass
    aWetHighpassedRight butterhp aWetRawRight, kInputHighpass
    aWetMid = (aWetHighpassedLeft + aWetHighpassedRight) * 0.5
    aWetSide = (aWetHighpassedLeft - aWetHighpassedRight) * 0.5 * kStereoWidth
    aWetLeft = (aWetMid + aWetSide) * kReverbGain
    aWetRight = (aWetMid - aWetSide) * kReverbGain
    xout aWetLeft, aWetRight
endop

instr SmoothReverberator
    kDecayTime limit gk_SmoothReverberator_decay_time, 0.25, 60
    kWet limit gk_SmoothReverberator_wet, 0, 1
    kDry = 1 - kWet
    kModulationDepth limit gk_SmoothReverberator_delay_modulation, 0, 0.0025
    kModulationRate limit gk_SmoothReverberator_modulation_rate, 0.01, 2
    kMaximumCutoff = (sr * 0.45) / 1.09
    kCutoff limit gk_SmoothReverberator_frequency_cutoff, 500, kMaximumCutoff
    kStereoWidth limit gk_SmoothReverberator_stereo_width, 0, 2
    kInputHighpass limit gk_SmoothReverberator_input_highpass, 5, 1000
    kReverbGain limit gk_SmoothReverberator_reverb_gain, 0, 4
    aInputLeft inleta "inleft"
    aInputRight inleta "inright"
    aWetLeft, aWetRight SmoothReverberator aInputLeft, aInputRight, kDecayTime, kModulationDepth, kModulationRate, kCutoff, kStereoWidth, kInputHighpass, kReverbGain
    aOutputLeft = (aInputLeft * kDry) + (aWetLeft * kWet)
    aOutputRight = (aInputRight * kDry) + (aWetRight * kWet)
    outleta "outleft", aOutputLeft
    outleta "outright", aOutputRight
    kRmsLeft rms aOutputLeft
    kRmsRight rms aOutputRight
    prints "%-24s i %9.4f t %9.4f d %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, active(p1)
    printks "%-24s i %9.4f t %9.4f d %9.4f rt60 %8.3f cutoff %9.2f mod %8.6f rate %7.3f wet %6.3f #%3d l %9.6f r %9.6f\n", 1, nstrstr(p1), p1, p2, p3, kDecayTime, kCutoff, kModulationDepth, kModulationRate, kWet, active(p1), kRmsLeft, kRmsRight
endin
