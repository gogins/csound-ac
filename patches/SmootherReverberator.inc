; SmootherReverberator-v0.4.0.inc
; Stability-first, self-contained stereo reverb module for Csound modular patches.
; Version 0.4.0 uses a strictly passive late-field core:
;   - fixed, non-interpolated main FDN delay taps;
;   - an energy-normalized 8 x 8 Hadamard scattering matrix;
;   - feedback gains strictly below unity, calibrated from RT60;
;   - first-order complementary low/high decay shaping;
;   - four input allpass diffusers per channel;
;   - predelay, decorrelated early reflections, and multitap stereo output.
; No reverbsc, freeverb, convolution, or external plugin is used.
;
; Modular connections:
;   connect "Source", "outleft",  "SmootherReverberator", "inleft"
;   connect "Source", "outright", "SmootherReverberator", "inright"
;   connect "SmootherReverberator", "outleft",  "MasterOutput", "inleft"
;   connect "SmootherReverberator", "outright", "MasterOutput", "inright"
;   alwayson "SmootherReverberator"
;
; The modulation controls are retained for interface compatibility but are
; intentionally bypassed in version 0.4.0. Modulation should be reintroduced
; only after this passive reference implementation is confirmed stable.

gk_SmootherReverberator_decay_time chnexport "gk_SmootherReverberator_decay_time", 3
gk_SmootherReverberator_wet chnexport "gk_SmootherReverberator_wet", 3
gk_SmootherReverberator_delay_modulation chnexport "gk_SmootherReverberator_delay_modulation", 3
gk_SmootherReverberator_modulation_rate chnexport "gk_SmootherReverberator_modulation_rate", 3
gk_SmootherReverberator_frequency_cutoff chnexport "gk_SmootherReverberator_frequency_cutoff", 3
gk_SmootherReverberator_high_decay_ratio chnexport "gk_SmootherReverberator_high_decay_ratio", 3
gk_SmootherReverberator_bandwidth chnexport "gk_SmootherReverberator_bandwidth", 3
gk_SmootherReverberator_predelay chnexport "gk_SmootherReverberator_predelay", 3
gk_SmootherReverberator_early_level chnexport "gk_SmootherReverberator_early_level", 3
gk_SmootherReverberator_density chnexport "gk_SmootherReverberator_density", 3
gk_SmootherReverberator_size chnexport "gk_SmootherReverberator_size", 3
gk_SmootherReverberator_stereo_width chnexport "gk_SmootherReverberator_stereo_width", 3
gk_SmootherReverberator_input_highpass chnexport "gk_SmootherReverberator_input_highpass", 3
gk_SmootherReverberator_reverb_gain chnexport "gk_SmootherReverberator_reverb_gain", 3

gk_SmootherReverberator_decay_time init 22
gk_SmootherReverberator_wet init 0.5
gk_SmootherReverberator_delay_modulation init 0
gk_SmootherReverberator_modulation_rate init 0.13
gk_SmootherReverberator_frequency_cutoff init 6000
gk_SmootherReverberator_high_decay_ratio init 0.45
gk_SmootherReverberator_bandwidth init 15000
gk_SmootherReverberator_predelay init 0.045
gk_SmootherReverberator_early_level init 0.10
gk_SmootherReverberator_density init 0.92
gk_SmootherReverberator_size init 1
gk_SmootherReverberator_stereo_width init 1.10
gk_SmootherReverberator_input_highpass init 30
gk_SmootherReverberator_reverb_gain init 0.72

opcode SmootherReverberatorAllpass, a, akik
    aInput, kDelayTime, iMaximumDelay, kFeedback xin
    setksmps 1
    kSafeDelay limit kDelayTime, 1 / sr, iMaximumDelay - (1 / sr)
    kSafeFeedback limit kFeedback, -0.75, 0.75
    aDelayBuffer delayr iMaximumDelay
    aDelayed deltap kSafeDelay
    aOutput = aDelayed - (kSafeFeedback * aInput)
    delayw aInput + (kSafeFeedback * aOutput)
    xout aOutput
endop

opcode SmootherReverberatorCore, aa, aakkkkkkkkkkkkk
    aInputLeft, aInputRight, kDecayTime, kModulationDepth, kModulationRate, kBandwidth, kHighCrossover, kHighDecayRatio, kPredelay, kEarlyLevel, kDensity, kSize, kStereoWidth, kInputHighpass, kReverbGain xin
    setksmps 1
    kDecaySafe limit kDecayTime, 0.25, 90
    kNyquistLimit = sr * 0.45
    kBandwidthSafe limit kBandwidth, 500, kNyquistLimit
    kHighCrossoverSafe limit kHighCrossover, 500, kNyquistLimit
    kHighDecayRatioSafe limit kHighDecayRatio, 0.05, 1
    kPredelaySafe limit kPredelay, 1 / sr, 0.150
    kDensitySafe limit kDensity, 0, 1
    kSizeSafe limit kSize, 0.5, 1.5
    kStereoWidthSafe limit kStereoWidth, 0, 2
    kInputHighpassSafe limit kInputHighpass, 5, 1000
    kReverbGainSafe limit kReverbGain, 0, 4
    kEarlyLevelSafe limit kEarlyLevel, 0, 1
    kDiffusion1 = 0.38 + (0.28 * kDensitySafe)
    kDiffusion2 = 0.36 + (0.27 * kDensitySafe)
    kDiffusion3 = 0.34 + (0.26 * kDensitySafe)
    kDiffusion4 = 0.32 + (0.25 * kDensitySafe)
    aInputHPLeft butterhp aInputLeft, kInputHighpassSafe
    aInputHPRight butterhp aInputRight, kInputHighpassSafe
    aInputBWLeft tone aInputHPLeft, kBandwidthSafe
    aInputBWRight tone aInputHPRight, kBandwidthSafe
    aPredelayBufferLeft delayr 0.300
    aPredelayedLeft deltap kPredelaySafe
    aEarlyLeft1 deltap kPredelaySafe + (0.0073 * kSizeSafe)
    aEarlyLeft2 deltap kPredelaySafe + (0.0137 * kSizeSafe)
    aEarlyLeft3 deltap kPredelaySafe + (0.0229 * kSizeSafe)
    aEarlyLeft4 deltap kPredelaySafe + (0.0361 * kSizeSafe)
    aEarlyLeft5 deltap kPredelaySafe + (0.0527 * kSizeSafe)
    aEarlyLeft6 deltap kPredelaySafe + (0.0743 * kSizeSafe)
    delayw aInputBWLeft
    aPredelayBufferRight delayr 0.300
    aPredelayedRight deltap kPredelaySafe
    aEarlyRight1 deltap kPredelaySafe + (0.0089 * kSizeSafe)
    aEarlyRight2 deltap kPredelaySafe + (0.0163 * kSizeSafe)
    aEarlyRight3 deltap kPredelaySafe + (0.0269 * kSizeSafe)
    aEarlyRight4 deltap kPredelaySafe + (0.0419 * kSizeSafe)
    aEarlyRight5 deltap kPredelaySafe + (0.0593 * kSizeSafe)
    aEarlyRight6 deltap kPredelaySafe + (0.0817 * kSizeSafe)
    delayw aInputBWRight
    aDiffusedLeft1 SmootherReverberatorAllpass aPredelayedLeft, 0.00317 * kSizeSafe, 0.008, kDiffusion1
    aDiffusedLeft2 SmootherReverberatorAllpass aDiffusedLeft1, 0.00493 * kSizeSafe, 0.010, kDiffusion2
    aDiffusedLeft3 SmootherReverberatorAllpass aDiffusedLeft2, 0.00761 * kSizeSafe, 0.014, kDiffusion3
    aDiffusedLeft4 SmootherReverberatorAllpass aDiffusedLeft3, 0.01129 * kSizeSafe, 0.020, kDiffusion4
    aDiffusedRight1 SmootherReverberatorAllpass aPredelayedRight, 0.00371 * kSizeSafe, 0.008, kDiffusion1
    aDiffusedRight2 SmootherReverberatorAllpass aDiffusedRight1, 0.00547 * kSizeSafe, 0.011, kDiffusion2
    aDiffusedRight3 SmootherReverberatorAllpass aDiffusedRight2, 0.00823 * kSizeSafe, 0.015, kDiffusion3
    aDiffusedRight4 SmootherReverberatorAllpass aDiffusedRight3, 0.01231 * kSizeSafe, 0.022, kDiffusion4
    aMidInput = (aDiffusedLeft4 + aDiffusedRight4) * 0.5
    aSideInput = (aDiffusedLeft4 - aDiffusedRight4) * 0.5
    iDelay1 = 0.04721
    iDelay2 = 0.05591
    iDelay3 = 0.06443
    iDelay4 = 0.07387
    iDelay5 = 0.08531
    iDelay6 = 0.09757
    iDelay7 = 0.11213
    iDelay8 = 0.12889
    kDelay1 = iDelay1 * kSizeSafe
    kDelay2 = iDelay2 * kSizeSafe
    kDelay3 = iDelay3 * kSizeSafe
    kDelay4 = iDelay4 * kSizeSafe
    kDelay5 = iDelay5 * kSizeSafe
    kDelay6 = iDelay6 * kSizeSafe
    kDelay7 = iDelay7 * kSizeSafe
    kDelay8 = iDelay8 * kSizeSafe
    aDelayBuffer1 delayr 0.090
    aLine1 deltap kDelay1
    aTap1A deltap iDelay1 * 0.317 * kSizeSafe
    aTap1B deltap iDelay1 * 0.683 * kSizeSafe
    aDelayBuffer2 delayr 0.100
    aLine2 deltap kDelay2
    aTap2A deltap iDelay2 * 0.271 * kSizeSafe
    aTap2B deltap iDelay2 * 0.719 * kSizeSafe
    aDelayBuffer3 delayr 0.115
    aLine3 deltap kDelay3
    aTap3A deltap iDelay3 * 0.349 * kSizeSafe
    aTap3B deltap iDelay3 * 0.641 * kSizeSafe
    aDelayBuffer4 delayr 0.130
    aLine4 deltap kDelay4
    aTap4A deltap iDelay4 * 0.293 * kSizeSafe
    aTap4B deltap iDelay4 * 0.757 * kSizeSafe
    aDelayBuffer5 delayr 0.145
    aLine5 deltap kDelay5
    aTap5A deltap iDelay5 * 0.331 * kSizeSafe
    aTap5B deltap iDelay5 * 0.697 * kSizeSafe
    aDelayBuffer6 delayr 0.165
    aLine6 deltap kDelay6
    aTap6A deltap iDelay6 * 0.257 * kSizeSafe
    aTap6B deltap iDelay6 * 0.733 * kSizeSafe
    aDelayBuffer7 delayr 0.185
    aLine7 deltap kDelay7
    aTap7A deltap iDelay7 * 0.373 * kSizeSafe
    aTap7B deltap iDelay7 * 0.617 * kSizeSafe
    aDelayBuffer8 delayr 0.210
    aLine8 deltap kDelay8
    aTap8A deltap iDelay8 * 0.307 * kSizeSafe
    aTap8B deltap iDelay8 * 0.769 * kSizeSafe
    aLow1 tone aLine1, kHighCrossoverSafe
    aLow2 tone aLine2, kHighCrossoverSafe * 0.97
    aLow3 tone aLine3, kHighCrossoverSafe * 1.03
    aLow4 tone aLine4, kHighCrossoverSafe * 0.94
    aLow5 tone aLine5, kHighCrossoverSafe * 1.06
    aLow6 tone aLine6, kHighCrossoverSafe * 0.91
    aLow7 tone aLine7, kHighCrossoverSafe * 1.09
    aLow8 tone aLine8, kHighCrossoverSafe * 0.88
    aHigh1 = aLine1 - aLow1
    aHigh2 = aLine2 - aLow2
    aHigh3 = aLine3 - aLow3
    aHigh4 = aLine4 - aLow4
    aHigh5 = aLine5 - aLow5
    aHigh6 = aLine6 - aLow6
    aHigh7 = aLine7 - aLow7
    aHigh8 = aLine8 - aLow8
    kLog001 = -6.907755278982137
    kHighDecayTime = kDecaySafe * kHighDecayRatioSafe
    kLowGain1 limit exp(kLog001 * kDelay1 / kDecaySafe), 0, 0.999
    kLowGain2 limit exp(kLog001 * kDelay2 / kDecaySafe), 0, 0.999
    kLowGain3 limit exp(kLog001 * kDelay3 / kDecaySafe), 0, 0.999
    kLowGain4 limit exp(kLog001 * kDelay4 / kDecaySafe), 0, 0.999
    kLowGain5 limit exp(kLog001 * kDelay5 / kDecaySafe), 0, 0.999
    kLowGain6 limit exp(kLog001 * kDelay6 / kDecaySafe), 0, 0.999
    kLowGain7 limit exp(kLog001 * kDelay7 / kDecaySafe), 0, 0.999
    kLowGain8 limit exp(kLog001 * kDelay8 / kDecaySafe), 0, 0.999
    kHighGain1 limit exp(kLog001 * kDelay1 / kHighDecayTime), 0, 0.999
    kHighGain2 limit exp(kLog001 * kDelay2 / kHighDecayTime), 0, 0.999
    kHighGain3 limit exp(kLog001 * kDelay3 / kHighDecayTime), 0, 0.999
    kHighGain4 limit exp(kLog001 * kDelay4 / kHighDecayTime), 0, 0.999
    kHighGain5 limit exp(kLog001 * kDelay5 / kHighDecayTime), 0, 0.999
    kHighGain6 limit exp(kLog001 * kDelay6 / kHighDecayTime), 0, 0.999
    kHighGain7 limit exp(kLog001 * kDelay7 / kHighDecayTime), 0, 0.999
    kHighGain8 limit exp(kLog001 * kDelay8 / kHighDecayTime), 0, 0.999
    aState1 = (aLow1 * kLowGain1) + (aHigh1 * kHighGain1)
    aState2 = (aLow2 * kLowGain2) + (aHigh2 * kHighGain2)
    aState3 = (aLow3 * kLowGain3) + (aHigh3 * kHighGain3)
    aState4 = (aLow4 * kLowGain4) + (aHigh4 * kHighGain4)
    aState5 = (aLow5 * kLowGain5) + (aHigh5 * kHighGain5)
    aState6 = (aLow6 * kLowGain6) + (aHigh6 * kHighGain6)
    aState7 = (aLow7 * kLowGain7) + (aHigh7 * kHighGain7)
    aState8 = (aLow8 * kLowGain8) + (aHigh8 * kHighGain8)
    denorm aState1, aState2, aState3, aState4, aState5, aState6, aState7, aState8
    iHadamardNormalization = 0.3535533905932738
    aMatrix1 = (aState1 + aState2 + aState3 + aState4 + aState5 + aState6 + aState7 + aState8) * iHadamardNormalization
    aMatrix2 = (aState1 - aState2 + aState3 - aState4 + aState5 - aState6 + aState7 - aState8) * iHadamardNormalization
    aMatrix3 = (aState1 + aState2 - aState3 - aState4 + aState5 + aState6 - aState7 - aState8) * iHadamardNormalization
    aMatrix4 = (aState1 - aState2 - aState3 + aState4 + aState5 - aState6 - aState7 + aState8) * iHadamardNormalization
    aMatrix5 = (aState1 + aState2 + aState3 + aState4 - aState5 - aState6 - aState7 - aState8) * iHadamardNormalization
    aMatrix6 = (aState1 - aState2 + aState3 - aState4 - aState5 + aState6 - aState7 + aState8) * iHadamardNormalization
    aMatrix7 = (aState1 + aState2 - aState3 - aState4 - aState5 - aState6 + aState7 + aState8) * iHadamardNormalization
    aMatrix8 = (aState1 - aState2 - aState3 + aState4 - aState5 + aState6 + aState7 - aState8) * iHadamardNormalization
    iInjectionGain = 0.12
    aInjection1 = (aMidInput + aSideInput) * iInjectionGain
    aInjection2 = (aMidInput - aSideInput) * iInjectionGain
    aInjection3 = (-aMidInput + aSideInput) * iInjectionGain
    aInjection4 = (-aMidInput - aSideInput) * iInjectionGain
    aInjection5 = (aMidInput + (0.5 * aSideInput)) * iInjectionGain
    aInjection6 = ((0.5 * aMidInput) - aSideInput) * iInjectionGain
    aInjection7 = ((-0.5 * aMidInput) - aSideInput) * iInjectionGain
    aInjection8 = (-aMidInput + (0.5 * aSideInput)) * iInjectionGain
    delayw aMatrix1 + aInjection1
    delayw aMatrix2 + aInjection2
    delayw aMatrix3 + aInjection3
    delayw aMatrix4 + aInjection4
    delayw aMatrix5 + aInjection5
    delayw aMatrix6 + aInjection6
    delayw aMatrix7 + aInjection7
    delayw aMatrix8 + aInjection8
    aEarlyLeft = (0.53 * aEarlyLeft1) + (0.37 * aEarlyRight2) - (0.31 * aEarlyLeft3) + (0.29 * aEarlyRight4) + (0.23 * aEarlyLeft5) - (0.19 * aEarlyRight6)
    aEarlyRight = (0.51 * aEarlyRight1) - (0.39 * aEarlyLeft2) + (0.33 * aEarlyRight3) + (0.27 * aEarlyLeft4) - (0.21 * aEarlyRight5) + (0.17 * aEarlyLeft6)
    iLateNormalization = 0.22
    aLateLeft = (aTap1A - aTap2B + aTap3A + aTap4B - aTap5A + aTap6B + aTap7A - aTap8B) * iLateNormalization
    aLateRight = (aTap1B + aTap2A - aTap3B + aTap4A + aTap5B - aTap6A + aTap7B - aTap8A) * iLateNormalization
    aWetRawLeft = aLateLeft + (aEarlyLeft * kEarlyLevelSafe)
    aWetRawRight = aLateRight + (aEarlyRight * kEarlyLevelSafe)
    aWetDCLeft dcblock2 aWetRawLeft, 8
    aWetDCRight dcblock2 aWetRawRight, 8
    aWetHPLeft butterhp aWetDCLeft, kInputHighpassSafe
    aWetHPRight butterhp aWetDCRight, kInputHighpassSafe
    aWetMid = (aWetHPLeft + aWetHPRight) * 0.5
    aWetSide = (aWetHPLeft - aWetHPRight) * 0.5 * kStereoWidthSafe
    aWetLeft = (aWetMid + aWetSide) * kReverbGainSafe
    aWetRight = (aWetMid - aWetSide) * kReverbGainSafe
    xout aWetLeft, aWetRight
endop

instr SmootherReverberator
    prints "SmootherReverberator 0.4.0: passive fixed-delay FDN core\n"
    kDecayTime limit gk_SmootherReverberator_decay_time, 0.25, 90
    kWetTarget limit gk_SmootherReverberator_wet, 0, 1
    kModulationDepth limit gk_SmootherReverberator_delay_modulation, 0, 0.00075
    kModulationRate limit gk_SmootherReverberator_modulation_rate, 0.01, 1
    kMaximumCutoff = sr * 0.45
    kBandwidth limit gk_SmootherReverberator_bandwidth, 500, kMaximumCutoff
    kHighCrossover limit gk_SmootherReverberator_frequency_cutoff, 500, kMaximumCutoff
    kHighDecayRatio limit gk_SmootherReverberator_high_decay_ratio, 0.05, 1
    kPredelay limit gk_SmootherReverberator_predelay, 1 / sr, 0.150
    kEarlyLevel limit gk_SmootherReverberator_early_level, 0, 1
    kDensity limit gk_SmootherReverberator_density, 0, 1
    kSize limit gk_SmootherReverberator_size, 0.5, 1.5
    kStereoWidthTarget limit gk_SmootherReverberator_stereo_width, 0, 2
    kInputHighpass limit gk_SmootherReverberator_input_highpass, 5, 1000
    kReverbGainTarget limit gk_SmootherReverberator_reverb_gain, 0, 4
    kWet portk kWetTarget, 0.03
    kStereoWidth portk kStereoWidthTarget, 0.08
    kReverbGain portk kReverbGainTarget, 0.05
    aInputLeft inleta "inleft"
    aInputRight inleta "inright"
    aWetLeft, aWetRight SmootherReverberatorCore aInputLeft, aInputRight, kDecayTime, kModulationDepth, kModulationRate, kBandwidth, kHighCrossover, kHighDecayRatio, kPredelay, kEarlyLevel, kDensity, kSize, kStereoWidth, kInputHighpass, kReverbGain
    kDryGain = sqrt(1 - kWet)
    kWetGain = sqrt(kWet)
    aOutputLeft = (aInputLeft * kDryGain) + (aWetLeft * kWetGain)
    aOutputRight = (aInputRight * kDryGain) + (aWetRight * kWetGain)
    outleta "outleft", aOutputLeft
    outleta "outright", aOutputRight
endin
