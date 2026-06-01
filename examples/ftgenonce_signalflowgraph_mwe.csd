<CsoundSynthesizer>
<CsOptions>
-n -d -m162 -+msg_color=0
</CsOptions>
<CsInstruments>

sr = 96000
ksmps = 100
nchnls = 2
0dbfs = 2000

connect "Granular", "outleft", "Output", "inleft"
connect "Granular", "outright", "Output", "inright"
alwayson "Output"

instr Granular
  ; In Csound 7.0 (double samples, commit 489eef1075...), this minimal
  ; signal-flow-graph instrument reproduces the Blue Leaves crash when these
  ; tables are created with ftgenonce inside the instrument. Replacing these
  ; two lines with global ftgen tables avoids the crash.
  igrtab  ftgenonce 0, 0, 65536, 10, 1, .3, .1, 0, .2, .02, 0, .1, .04
  iwintab ftgenonce 0, 0, 65536, 10, 1, 0, .5, 0, .33, 0, .25, 0, .2, 0, .167

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
