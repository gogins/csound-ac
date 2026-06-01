<CsoundSynthesizer>
<CsOptions>
-n -d -m162 -+msg_color=0
</CsOptions>
<CsInstruments>

sr = 96000
ksmps = 100
nchnls = 2
0dbfs = 2000

; Control case for ftgenonce_signalflowgraph_mwe.csd: the same instrument
; works when the two tables are created globally with ftgen.
giGrainTable  ftgen 0, 0, 65536, 10, 1, .3, .1, 0, .2, .02, 0, .1, .04
giWindowTable ftgen 0, 0, 65536, 10, 1, 0, .5, 0, .33, 0, .25, 0, .2, 0, .167

connect "Granular", "outleft", "Output", "inleft"
connect "Granular", "outright", "Output", "inright"
alwayson "Output"

instr Granular
  igrtab = giGrainTable
  iwintab = giWindowTable

  ifqc = 440
  iamp = 0.1
  idens = 150
  ifrng = 0.033
  igdur = 0.2
  kamp linseg 0, 0.1, 1, p3 - 0.2, 1, 0.1, 0

  aoutl grain iamp, ifqc, idens, 100, ifqc * ifrng, igdur, igrtab, iwintab, 5
  aoutr grain iamp, ifqc, idens, 100, ifqc * ifrng, igdur, igrtab, iwintab, 5

  outleta "outleft",  aoutl * kamp
  outleta "outright", aoutr * kamp
endin

instr Output
  ainleft  inleta "inleft"
  ainright inleta "inright"
  outs ainleft, ainright
endin

</CsInstruments>
<CsScore>
i "Granular" 0 1
e
</CsScore>
</CsoundSynthesizer>
