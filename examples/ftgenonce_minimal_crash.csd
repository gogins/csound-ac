<CsoundSynthesizer>
<CsOptions>
-n -d -m162 -+msg_color=0
</CsOptions>
<CsInstruments>

sr = 48000
ksmps = 64
nchnls = 2
0dbfs = 1

instr Test
  isine ftgenonce 0, 0, 1024, 10, 1
  asig oscili 0.1, 440, isine
  outs asig, asig
endin

</CsInstruments>
<CsScore>
i "Test" 0 0.1
e
</CsScore>
</CsoundSynthesizer>
