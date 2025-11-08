<CsoundSynthesizer>
<CsOptions>
-d -odac
</CsOptions>
<CsInstruments>
sr     = 48000
ksmps  = 64
nchnls = 2
0dbfs  = 1

#include "impart_harmony.inc"

instr 1
  aNoise rand 0.5
  aOut init 0.
  kWet  init 1.
  kCut  init 6000.
  kReverb init 4.
  i1 init 24
    i2 init 52  
    i3 init 67
  ; C4–E4–G4, leave the last two as default -1 (or pass -1 explicitly)
  aOut ParallelCombNotes5 aNoise, kWet, kCut, kReverb, i1, i2, i3, -1, -1
  outs aOut, aOut
endin

</CsInstruments>
<CsScore>
i1 0 6
e
</CsScore>
</CsoundSynthesizer>
