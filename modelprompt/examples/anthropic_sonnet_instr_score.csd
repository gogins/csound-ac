<CsoundSynthesizer>
<CsOptions>
-odac -d
</CsOptions>
<CsInstruments>

; Requires:
;   export ANTHROPIC_API_KEY=...
;   modelprompt plugin on OPCODE7DIR64 / CS_USER_PLUGINDIR
;     (e.g. ~/Library/csound/7.0/plugins64/modelprompt.dylib)
;
; Run:
;   csound anthropic_sonnet_instr_score.csd

sr = 48000
ksmps = 64
nchnls = 2
0dbfs = 1

gSProvider = "anthropic"
gSModel = "claude-sonnet-5"

instr Compose
    ; 1) Ask Sonnet for instrument-body source, print it, then compile.
    Sbody = modelprompt(gSProvider, gSModel, {{
Write the body of a Csound instrument for a short inharmonic modal bell.

p4 is amplitude (0 to 1).
p5 is fundamental frequency in Hz.

Requirements:
- Use functional / expression syntax where possible.
- Excite about 8 parallel modes (mode or reson / tone pairs are fine).
- Higher modes must decay faster than lower modes.
- Scale the final signal by p4 and a linen envelope over p3.
- Send identical audio to both channels with outc(asig, asig).
- Do not use outs (deprecated).
- Do not use instr / endin, markdown, or commentary.
- The instrument must produce audible non-zero output for
  typical p4 around 0.2 and p5 around 220.
}})

    prints("Generated instrument body:\n%s\n", Sbody)
    Voice:InstrDef = create(Sbody)
    inum = nstrnum(Voice)
    prints("Compiled model instrument as instr %d\n", inum)

    ; 2) Generate a score that targets that instrument number.
    Sprompt = sprintf({{
Write exactly 12 valid Csound i-statements for instrument %d.

p2 = onset time in seconds
p3 = duration in seconds (0.8 to 2.0)
p4 = amplitude from 0.15 to 0.4
p5 = frequency in Hz between 110 and 660

Make a sparse, rising arpeggio lasting at most 12 seconds.
Return only the i-statements, one per line.
No comments, markdown, or f/e statements.
}}, inum)

    Sscore = modelprompt(gSProvider, gSModel, Sprompt)
    prints("Generated score:\n%s\n", Sscore)
    scorelinei(Sscore)

    event("e", 0, 14)
endin

schedule("Compose", 0, 1)

</CsInstruments>
<CsScore>
</CsScore>
</CsoundSynthesizer>
