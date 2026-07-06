; SmootherReverberator.inc
; Self-contained stereo reverb module for Csound modular patches.
; It uses the same inlet/outlet convention as SmoothReverberator.inc:
;
;   connect "Source", "outleft",  "SmootherReverberator", "inleft"
;   connect "Source", "outright", "SmootherReverberator", "inright"
;   connect "SmootherReverberator", "outleft",  "MasterOutput", "inleft"
;   connect "SmootherReverberator", "outright", "MasterOutput", "inright"
;   alwayson "SmootherReverberator"
;
; The late field is an eight-line feedback-delay network with:
;   - sample-accurate feedback inside a local-ksmps UDO;
;   - four serial input allpasses per channel;
;   - an internal allpass diffuser in every feedback path;
;   - a normalized Hadamard feedback matrix;
;   - RT60-calibrated low/mid feedback and independently shorter high decay;
;   - subtle non-periodic modulation of selected internal diffusers;
;   - predelay, decorrelated early reflections, and multitap stereo outputs;
;   - parameter smoothing, DC blocking, and denormal protection.
;
; This is an independent implementation. It does not use reverbsc, freeverb,
; convolution, MVerb source code, or external plugins.
;
; Exported controls:
;   gk_SmootherReverberator_decay_time       low/mid RT60 in seconds
;   gk_SmootherReverberator_wet              wet fraction, 0 to 1
;   gk_SmootherReverberator_delay_modulation internal diffuser modulation, seconds
;   gk_SmootherReverberator_modulation_rate  modulation breakpoint rate, Hz
;   gk_SmootherReverberator_frequency_cutoff high-decay crossover, Hz
;   gk_SmootherReverberator_high_decay_ratio high RT60 / low-mid RT60
;   gk_SmootherReverberator_bandwidth        input low-pass cutoff, Hz
;   gk_SmootherReverberator_predelay         predelay in seconds
;   gk_SmootherReverberator_early_level      early-reflection level, 0 to 1
;   gk_SmootherReverberator_density          diffusion density, 0 to 1
;   gk_SmootherReverberator_size             delay-length scale, 0.5 to 1.5
;   gk_SmootherReverberator_stereo_width     wet M/S width, 0 to 2
;   gk_SmootherReverberator_input_highpass   input/output high-pass cutoff, Hz
;   gk_SmootherReverberator_reverb_gain      final wet-field gain

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
gk_SmootherReverberator_delay_modulation init 0.00012
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
    aDelayBuffer delayr iMaximumDelay
    aDelayed deltap3 kDelayTime
    aOutput = aDelayed - (kFeedback * aInput)
    delayw aInput + (kFeedback * aOutput)
    xout aOutput
endop

opcode SmootherReverberator, aa, aakkkkkkkkkkkkk
    aInputLeft, aInputRight, kDecayTime, kModulationDepth, kModulationRate, kBandwidth, kHighCrossover, kHighDecayRatio, kPredelay, kEarlyLevel, kDensity, kSize, kStereoWidth, kInputHighpass, kReverbGain xin
    setksmps 1
    kNyquistLimit = sr * 0.45
    kPredelayRead limit kPredelay, 1 / sr, 0.150
    kBandwidthLimited limit kBandwidth, 500, kNyquistLimit
    kHighCrossoverLimited limit kHighCrossover, 500, kNyquistLimit
    kSizeLimited limit kSize, 0.5, 1.5
    kDensityLimited limit kDensity, 0, 1
    kHighDecayRatioLimited limit kHighDecayRatio, 0.05, 1
    kDiffusion1 = 0.40 + (0.30 * kDensityLimited)
    kDiffusion2 = 0.38 + (0.29 * kDensityLimited)
    kDiffusion3 = 0.36 + (0.28 * kDensityLimited)
    kDiffusion4 = 0.34 + (0.27 * kDensityLimited)
    kTankDiffusion = 0.30 + (0.25 * kDensityLimited)
    aHighpassedLeft butterhp aInputLeft, kInputHighpass
    aHighpassedRight butterhp aInputRight, kInputHighpass
    aBandwidthLeft tone aHighpassedLeft, kBandwidthLimited
    aBandwidthRight tone aHighpassedRight, kBandwidthLimited
    aPredelayBufferLeft delayr 0.300
    aPredelayedLeft deltap3 kPredelayRead
    aEarlyLeft1 deltap3 kPredelayRead + (0.0073 * kSizeLimited)
    aEarlyLeft2 deltap3 kPredelayRead + (0.0137 * kSizeLimited)
    aEarlyLeft3 deltap3 kPredelayRead + (0.0229 * kSizeLimited)
    aEarlyLeft4 deltap3 kPredelayRead + (0.0361 * kSizeLimited)
    aEarlyLeft5 deltap3 kPredelayRead + (0.0527 * kSizeLimited)
    aEarlyLeft6 deltap3 kPredelayRead + (0.0743 * kSizeLimited)
    delayw aBandwidthLeft
    aPredelayBufferRight delayr 0.300
    aPredelayedRight deltap3 kPredelayRead
    aEarlyRight1 deltap3 kPredelayRead + (0.0089 * kSizeLimited)
    aEarlyRight2 deltap3 kPredelayRead + (0.0163 * kSizeLimited)
    aEarlyRight3 deltap3 kPredelayRead + (0.0269 * kSizeLimited)
    aEarlyRight4 deltap3 kPredelayRead + (0.0419 * kSizeLimited)
    aEarlyRight5 deltap3 kPredelayRead + (0.0593 * kSizeLimited)
    aEarlyRight6 deltap3 kPredelayRead + (0.0817 * kSizeLimited)
    delayw aBandwidthRight
    aDiffusedLeft1 SmootherReverberatorAllpass aPredelayedLeft, 0.00317 * kSizeLimited, 0.008, kDiffusion1
    aDiffusedLeft2 SmootherReverberatorAllpass aDiffusedLeft1, 0.00493 * kSizeLimited, 0.010, kDiffusion2
    aDiffusedLeft3 SmootherReverberatorAllpass aDiffusedLeft2, 0.00761 * kSizeLimited, 0.014, kDiffusion3
    aDiffusedLeft4 SmootherReverberatorAllpass aDiffusedLeft3, 0.01129 * kSizeLimited, 0.020, kDiffusion4
    aDiffusedRight1 SmootherReverberatorAllpass aPredelayedRight, 0.00371 * kSizeLimited, 0.008, kDiffusion1
    aDiffusedRight2 SmootherReverberatorAllpass aDiffusedRight1, 0.00547 * kSizeLimited, 0.011, kDiffusion2
    aDiffusedRight3 SmootherReverberatorAllpass aDiffusedRight2, 0.00823 * kSizeLimited, 0.015, kDiffusion3
    aDiffusedRight4 SmootherReverberatorAllpass aDiffusedRight3, 0.01231 * kSizeLimited, 0.022, kDiffusion4
    aMidInput = (aDiffusedLeft4 + aDiffusedRight4) * 0.5
    aSideInput = (aDiffusedLeft4 - aDiffusedRight4) * 0.5
    kModulation1 randi kModulationDepth, kModulationRate * 0.79, 0.11, 1
    kModulation2 randi kModulationDepth, kModulationRate * 0.93, 0.29, 1
    kModulation3 randi kModulationDepth, kModulationRate * 1.17, 0.47, 1
    kModulation4 randi kModulationDepth, kModulationRate * 1.31, 0.73, 1
    iDelay1 = 0.04721
    iDelay2 = 0.05591
    iDelay3 = 0.06443
    iDelay4 = 0.07387
    iDelay5 = 0.08531
    iDelay6 = 0.09757
    iDelay7 = 0.11213
    iDelay8 = 0.12889
    kDelay1 = (iDelay1 * kSizeLimited) + kModulation1
    kDelay2 = iDelay2 * kSizeLimited
    kDelay3 = (iDelay3 * kSizeLimited) + kModulation2
    kDelay4 = iDelay4 * kSizeLimited
    kDelay5 = (iDelay5 * kSizeLimited) + kModulation3
    kDelay6 = iDelay6 * kSizeLimited
    kDelay7 = (iDelay7 * kSizeLimited) + kModulation4
    kDelay8 = iDelay8 * kSizeLimited
    aDelayBuffer1 delayr 0.090
    aLine1 deltap3 kDelay1
    aTap1A deltap3 iDelay1 * 0.317 * kSizeLimited
    aTap1B deltap3 iDelay1 * 0.683 * kSizeLimited
    aDelayBuffer2 delayr 0.100
    aLine2 deltap3 kDelay2
    aTap2A deltap3 iDelay2 * 0.271 * kSizeLimited
    aTap2B deltap3 iDelay2 * 0.719 * kSizeLimited
    aDelayBuffer3 delayr 0.115
    aLine3 deltap3 kDelay3
    aTap3A deltap3 iDelay3 * 0.349 * kSizeLimited
    aTap3B deltap3 iDelay3 * 0.641 * kSizeLimited
    aDelayBuffer4 delayr 0.130
    aLine4 deltap3 kDelay4
    aTap4A deltap3 iDelay4 * 0.293 * kSizeLimited
    aTap4B deltap3 iDelay4 * 0.757 * kSizeLimited
    aDelayBuffer5 delayr 0.145
    aLine5 deltap3 kDelay5
    aTap5A deltap3 iDelay5 * 0.331 * kSizeLimited
    aTap5B deltap3 iDelay5 * 0.697 * kSizeLimited
    aDelayBuffer6 delayr 0.165
    aLine6 deltap3 kDelay6
    aTap6A deltap3 iDelay6 * 0.257 * kSizeLimited
    aTap6B deltap3 iDelay6 * 0.733 * kSizeLimited
    aDelayBuffer7 delayr 0.185
    aLine7 deltap3 kDelay7
    aTap7A deltap3 iDelay7 * 0.373 * kSizeLimited
    aTap7B deltap3 iDelay7 * 0.617 * kSizeLimited
    aDelayBuffer8 delayr 0.210
    aLine8 deltap3 kDelay8
    aTap8A deltap3 iDelay8 * 0.307 * kSizeLimited
    aTap8B deltap3 iDelay8 * 0.769 * kSizeLimited
    aTankDiffuse1 SmootherReverberatorAllpass aLine1, (0.00371 * kSizeLimited) + (kModulation2 * 0.35), 0.010, kTankDiffusion
    aTankDiffuse2 SmootherReverberatorAllpass aLine2, 0.00419 * kSizeLimited, 0.011, -kTankDiffusion
    aTankDiffuse3 SmootherReverberatorAllpass aLine3, (0.00473 * kSizeLimited) + (kModulation3 * 0.35), 0.012, kTankDiffusion
    aTankDiffuse4 SmootherReverberatorAllpass aLine4, 0.00529 * kSizeLimited, 0.013, -kTankDiffusion
    aTankDiffuse5 SmootherReverberatorAllpass aLine5, (0.00593 * kSizeLimited) + (kModulation4 * 0.35), 0.014, kTankDiffusion
    aTankDiffuse6 SmootherReverberatorAllpass aLine6, 0.00661 * kSizeLimited, 0.015, -kTankDiffusion
    aTankDiffuse7 SmootherReverberatorAllpass aLine7, (0.00743 * kSizeLimited) + (kModulation1 * 0.35), 0.017, kTankDiffusion
    aTankDiffuse8 SmootherReverberatorAllpass aLine8, 0.00817 * kSizeLimited, 0.018, -kTankDiffusion
    aLow1 tone aTankDiffuse1, kHighCrossoverLimited
    aLow2 tone aTankDiffuse2, kHighCrossoverLimited * 0.97
    aLow3 tone aTankDiffuse3, kHighCrossoverLimited * 1.03
    aLow4 tone aTankDiffuse4, kHighCrossoverLimited * 0.94
    aLow5 tone aTankDiffuse5, kHighCrossoverLimited * 1.06
    aLow6 tone aTankDiffuse6, kHighCrossoverLimited * 0.91
    aLow7 tone aTankDiffuse7, kHighCrossoverLimited * 1.09
    aLow8 tone aTankDiffuse8, kHighCrossoverLimited * 0.88
    aHigh1 = aTankDiffuse1 - aLow1
    aHigh2 = aTankDiffuse2 - aLow2
    aHigh3 = aTankDiffuse3 - aLow3
    aHigh4 = aTankDiffuse4 - aLow4
    aHigh5 = aTankDiffuse5 - aLow5
    aHigh6 = aTankDiffuse6 - aLow6
    aHigh7 = aTankDiffuse7 - aLow7
    aHigh8 = aTankDiffuse8 - aLow8
    kLog001 = -6.907755278982137
    kEffectiveDelay1 = kDelay1 + (0.00371 * kSizeLimited)
    kEffectiveDelay2 = kDelay2 + (0.00419 * kSizeLimited)
    kEffectiveDelay3 = kDelay3 + (0.00473 * kSizeLimited)
    kEffectiveDelay4 = kDelay4 + (0.00529 * kSizeLimited)
    kEffectiveDelay5 = kDelay5 + (0.00593 * kSizeLimited)
    kEffectiveDelay6 = kDelay6 + (0.00661 * kSizeLimited)
    kEffectiveDelay7 = kDelay7 + (0.00743 * kSizeLimited)
    kEffectiveDelay8 = kDelay8 + (0.00817 * kSizeLimited)
    kLowGain1 = exp(kLog001 * kEffectiveDelay1 / kDecayTime)
    kLowGain2 = exp(kLog001 * kEffectiveDelay2 / kDecayTime)
    kLowGain3 = exp(kLog001 * kEffectiveDelay3 / kDecayTime)
    kLowGain4 = exp(kLog001 * kEffectiveDelay4 / kDecayTime)
    kLowGain5 = exp(kLog001 * kEffectiveDelay5 / kDecayTime)
    kLowGain6 = exp(kLog001 * kEffectiveDelay6 / kDecayTime)
    kLowGain7 = exp(kLog001 * kEffectiveDelay7 / kDecayTime)
    kLowGain8 = exp(kLog001 * kEffectiveDelay8 / kDecayTime)
    kHighDecayTime = kDecayTime * kHighDecayRatioLimited
    kHighGain1 = exp(kLog001 * kEffectiveDelay1 / kHighDecayTime)
    kHighGain2 = exp(kLog001 * kEffectiveDelay2 / kHighDecayTime)
    kHighGain3 = exp(kLog001 * kEffectiveDelay3 / kHighDecayTime)
    kHighGain4 = exp(kLog001 * kEffectiveDelay4 / kHighDecayTime)
    kHighGain5 = exp(kLog001 * kEffectiveDelay5 / kHighDecayTime)
    kHighGain6 = exp(kLog001 * kEffectiveDelay6 / kHighDecayTime)
    kHighGain7 = exp(kLog001 * kEffectiveDelay7 / kHighDecayTime)
    kHighGain8 = exp(kLog001 * kEffectiveDelay8 / kHighDecayTime)
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
    iInjectionGain = 0.16
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
    iLateNormalization = 0.20
    aLateLeft = (aTap1A - aTap2B + aTap3A + aTap4B - aTap5A + aTap6B + aTap7A - aTap8B + (0.45 * aTankDiffuse2) - (0.35 * aTankDiffuse5) + (0.30 * aTankDiffuse8)) * iLateNormalization
    aLateRight = (aTap1B + aTap2A - aTap3B + aTap4A + aTap5B - aTap6A + aTap7B - aTap8A - (0.45 * aTankDiffuse1) + (0.35 * aTankDiffuse4) + (0.30 * aTankDiffuse7)) * iLateNormalization
    aWetRawLeft = aLateLeft + (aEarlyLeft * kEarlyLevel)
    aWetRawRight = aLateRight + (aEarlyRight * kEarlyLevel)
    aWetDCLeft dcblock2 aWetRawLeft, 8
    aWetDCRight dcblock2 aWetRawRight, 8
    aWetHighpassedLeft butterhp aWetDCLeft, kInputHighpass
    aWetHighpassedRight butterhp aWetDCRight, kInputHighpass
    aWetMid = (aWetHighpassedLeft + aWetHighpassedRight) * 0.5
    aWetSide = (aWetHighpassedLeft - aWetHighpassedRight) * 0.5 * kStereoWidth
    aWetLeft = (aWetMid + aWetSide) * kReverbGain
    aWetRight = (aWetMid - aWetSide) * kReverbGain
    xout aWetLeft, aWetRight
endop

instr SmootherReverberator
    kDecayTimeRaw limit gk_SmootherReverberator_decay_time, 0.25, 90
    kWetRaw limit gk_SmootherReverberator_wet, 0, 1
    kModulationDepthRaw limit gk_SmootherReverberator_delay_modulation, 0, 0.00075
    kModulationRateRaw limit gk_SmootherReverberator_modulation_rate, 0.01, 1
    kMaximumCutoff = sr * 0.45
    kBandwidthRaw limit gk_SmootherReverberator_bandwidth, 500, kMaximumCutoff
    kHighCrossoverRaw limit gk_SmootherReverberator_frequency_cutoff, 500, kMaximumCutoff
    kHighDecayRatioRaw limit gk_SmootherReverberator_high_decay_ratio, 0.05, 1
    kPredelayRaw limit gk_SmootherReverberator_predelay, 1 / sr, 0.150
    kEarlyLevelRaw limit gk_SmootherReverberator_early_level, 0, 1
    kDensityRaw limit gk_SmootherReverberator_density, 0, 1
    kSizeRaw limit gk_SmootherReverberator_size, 0.5, 1.5
    kStereoWidthRaw limit gk_SmootherReverberator_stereo_width, 0, 2
    kInputHighpassRaw limit gk_SmootherReverberator_input_highpass, 5, 1000
    kReverbGainRaw limit gk_SmootherReverberator_reverb_gain, 0, 4
    kDecayTime lag kDecayTimeRaw, 0.10
    kWet lag kWetRaw, 0.03
    kModulationDepth lag kModulationDepthRaw, 0.20
    kModulationRate lag kModulationRateRaw, 0.20
    kBandwidth lag kBandwidthRaw, 0.08
    kHighCrossover lag kHighCrossoverRaw, 0.08
    kHighDecayRatio lag kHighDecayRatioRaw, 0.10
    kPredelay lag kPredelayRaw, 0.08
    kEarlyLevel lag kEarlyLevelRaw, 0.05
    kDensity lag kDensityRaw, 0.10
    kSize lag kSizeRaw, 0.25
    kStereoWidth lag kStereoWidthRaw, 0.08
    kInputHighpass lag kInputHighpassRaw, 0.08
    kReverbGain lag kReverbGainRaw, 0.05
    aInputLeft inleta "inleft"
    aInputRight inleta "inright"
    aWetLeft, aWetRight SmootherReverberator aInputLeft, aInputRight, kDecayTime, kModulationDepth, kModulationRate, kBandwidth, kHighCrossover, kHighDecayRatio, kPredelay, kEarlyLevel, kDensity, kSize, kStereoWidth, kInputHighpass, kReverbGain
    kDryGain = sqrt(1 - kWet)
    kWetGain = sqrt(kWet)
    aOutputLeft = (aInputLeft * kDryGain) + (aWetLeft * kWetGain)
    aOutputRight = (aInputRight * kDryGain) + (aWetRight * kWetGain)
    outleta "outleft", aOutputLeft
    outleta "outright", aOutputRight
endin
