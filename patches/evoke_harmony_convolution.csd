<CsoundSynthesizer>
<CsOptions>
-m165 -RWdo "evoke_harmony_convolution.wav"  
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 32
nchnls  = 2
0dbfs   = 2

#include "evoke_harmony_convolution.inc"

instr demo
  ; source: band-limited noise burst
  k_gate linseg 0, 0.01, 1, 24, 1, 0.5, 0
  a_noise rand   0.5
  a_source   = a_noise * k_gate
  a_source   tone a_source, 4000

  ; parameters: kernel duration, impulse gain, dirac level, pitch classes
  ; Output loudness remains stable despite these values.
  a_convolved evoke_harmony_convolution a_source, .03, .1, .05, 0, 4, 7, 11, 14

  outs a_convolved, a_convolved
endin

</CsInstruments>
<CsScore>
i "demo" 0 30
</CsScore>
</CsoundSynthesizer>
