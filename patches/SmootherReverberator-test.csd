<CsoundSynthesizer>
<CsOptions>
-o SmootherReverberator-test-v0.4.0.wav -W -f -d
</CsOptions>
<CsInstruments>
sr = 48000
ksmps = 64
nchnls = 2
0dbfs = 1

instr TestExcitation
    iPan limit p4, 0, 1
    aEnvelope transeg 0.000001, 0.002, -8, 1, 0.060, -7, 0.000001
    aNoise rand 0.18
    aTone poscil 0.12, 523.251
    aSignal = (aNoise + aTone) * aEnvelope
    aLeft = aSignal * sqrt(1 - iPan)
    aRight = aSignal * sqrt(iPan)
    outleta "outleft", aLeft
    outleta "outright", aRight
endin

#include "SmootherReverberator.inc"

instr TestControls
    gk_SmootherReverberator_decay_time = 22
    gk_SmootherReverberator_wet = 1
    gk_SmootherReverberator_delay_modulation = 0
    gk_SmootherReverberator_modulation_rate = 0.13
    gk_SmootherReverberator_frequency_cutoff = 6000
    gk_SmootherReverberator_high_decay_ratio = 0.45
    gk_SmootherReverberator_bandwidth = 15000
    gk_SmootherReverberator_predelay = 0.045
    gk_SmootherReverberator_early_level = 0.08
    gk_SmootherReverberator_density = 0.92
    gk_SmootherReverberator_size = 1
    gk_SmootherReverberator_stereo_width = 1.10
    gk_SmootherReverberator_input_highpass = 30
    gk_SmootherReverberator_reverb_gain = 0.72
endin

instr MasterOutput
    aLeft inleta "inleft"
    aRight inleta "inright"
    kRmsLeft rms aLeft
    kRmsRight rms aRight
    kTime timeinsts
    printks "t=%7.2f rmsL=%12.8f rmsR=%12.8f\n", 1, kTime, kRmsLeft, kRmsRight
    outch 1, aLeft, 2, aRight
endin

connect "TestExcitation", "outleft", "SmootherReverberator", "inleft"
connect "TestExcitation", "outright", "SmootherReverberator", "inright"
connect "SmootherReverberator", "outleft", "MasterOutput", "inleft"
connect "SmootherReverberator", "outright", "MasterOutput", "inright"
</CsInstruments>
<CsScore>
i "TestControls" 0 40
i "SmootherReverberator" 0.02 39.98
i "MasterOutput" 0.02 39.98
i "TestExcitation" 0.12 0.062 0.35
f 0 40
</CsScore>
</CsoundSynthesizer>
