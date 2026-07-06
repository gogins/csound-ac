<CsoundSynthesizer>
<CsOptions>
-o SmootherReverberator-test.wav -W -f -d
</CsOptions>
<CsInstruments>
sr = 48000
ksmps = 64
nchnls = 2
0dbfs = 1

instr TestExcitation
    iFrequency = p4
    iPan limit p5, 0, 1
    aEnvelope transeg 0.000001, 0.003, -8, 1, 0.060, -6, 0.15, p3 - 0.063, -8, 0.000001
    aTone poscil 0.32, iFrequency
    aUpper poscil 0.12, iFrequency * 2.017
    aNoise rand 0.025
    aSignal = (aTone + aUpper + aNoise) * aEnvelope
    aLeft = aSignal * sqrt(1 - iPan)
    aRight = aSignal * sqrt(iPan)
    outleta "outleft", aLeft
    outleta "outright", aRight
endin

#include "SmootherReverberator.inc"

instr MasterOutput
    aLeft inleta "inleft"
    aRight inleta "inright"
    outs aLeft, aRight
endin

connect "TestExcitation", "outleft", "SmootherReverberator", "inleft"
connect "TestExcitation", "outright", "SmootherReverberator", "inright"
connect "SmootherReverberator", "outleft", "MasterOutput", "inleft"
connect "SmootherReverberator", "outright", "MasterOutput", "inright"
</CsInstruments>
<CsScore>
i "SmootherReverberator" 0 40
i "MasterOutput" 0 40
i "TestExcitation" 0.10 0.40 220 0.18
i "TestExcitation" 1.20 0.45 329.6276 0.82
i "TestExcitation" 2.55 0.50 493.8833 0.35
i "TestExcitation" 4.10 0.55 739.9888 0.68
f 0 40
</CsScore>
</CsoundSynthesizer>
