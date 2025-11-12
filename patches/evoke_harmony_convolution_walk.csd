<CsoundSynthesizer>
<CsOptions>
-m165 -RWdo "evoke_harmony_convolution_walk.wav"  
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 32
nchnls  = 2
0dbfs   = 200

#include "evoke_harmony_convolution.inc"

instr evoke
  a_input_left, a_input_right soundin "/Users/michaelgogins/Dropbox/imparting_harmonies/source_soundfiles/TASCAM_0101.normalized.wav"
  a_input = a_input_left + a_input_right
  a_convolved evoke_harmony_convolution a_input, .03, .1, .05, 0, 4, 7, 11, 14
  a_convolved = a_convolved / 5.
  outs a_convolved, a_convolved
endin

</CsInstruments>
<CsScore>
i "evoke" 0 481 
</CsScore>
</CsoundSynthesizer>
