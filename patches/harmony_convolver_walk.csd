<CsoundSynthesizer>
<CsOptions>
-m163 -RWdo "harmony_convolver_walk.wav"  
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 32
nchnls  = 2
0dbfs   = 10

#include "harmony_convolver.inc"

instr source_sound
  a_input_left, a_input_right soundin "/Users/michaelgogins/Dropbox/imparting_harmonies/source_soundfiles/TASCAM_0101.normalized.wav"
  outleta "leftout", a_input_left
  outleta "rightout", a_input_right
endin

instr evoke
  a_input_left inleta "leftin"
  a_input_right inleta "rightin"
  a_convolved_left harmony_convolver a_input_left, .03, .1, .05, 0, 4, 7, 11, 14
  a_convolved_right harmony_convolver a_input_right, .03, .1, .05, 0, 4, 7, 11, 14
  a_convolved_left *= .3
  a_convolved_right *= .3
  outleta "leftout", a_convolved_left
  outleta "rightout", a_convolved_right
endin

instr master_output
  a_left inleta "leftin"
  a_right inleta "rightin"
  outs a_left, a_right
endin

connect "source_sound",   "leftout",     "evoke",     	"leftin"
connect "source_sound",   "rightout",    "evoke",     	"rightin"

connect "evoke",   "leftout",     "master_output",     	"leftin"
connect "evoke",   "rightout",    "master_output",     	"rightin"

alwayson "source_sound"
alwayson "master_output"


</CsInstruments>
<CsScore>
i "evoke" 0 481 
</CsScore>
</CsoundSynthesizer>
