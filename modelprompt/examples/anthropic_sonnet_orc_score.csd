<CsoundSynthesizer>
<CsOptions>
-odac -d
</CsOptions>
<CsInstruments>

; Requires:
;   export ANTHROPIC_API_KEY=...
;   modelprompt plugin on OPCODE7DIR64 / CS_USER_PLUGINDIR
;
; Run:
;   csound anthropic_sonnet_orc_score.csd

sr = 48000
ksmps = 64
nchnls = 2
0dbfs = 1

gSProvider = "anthropic"
gSModel = "claude-sonnet-5"

instr Compose
    ; 1) Generate a small connected orchestra, compile it, and start alwayson nodes.
    Sorc = modelprompt_orc(gSProvider, gSModel, {{
Write a complete Csound orchestra fragment for a short stereo piece.

Include:
1. A named note instrument Tone with outleta "leftout" and "rightout".
   p4 = amplitude 0 to 1, p5 = frequency in Hz.
   Use oscili and a linen envelope. Do not use outs.
2. A named effect instrument Reverb with inleta "leftin"/"rightin"
   and outleta "leftout"/"rightout". Use a simple reverbsc or similar.
3. A named Soundfile (or Master) instrument that takes leftin/rightin
   and writes to the dac with outc.
4. connect statements wiring Tone -> Reverb -> Master.
5. alwayson for Reverb and Master only.

Return only orchestra code: instr/endin, connect, alwayson.
No score, markdown, or commentary.
}})

    prints("Compiled and activated orchestra:\n%s\n", Sorc)

    ; 2) Separate prompt for the score that plays Tone.
    Sscore = modelprompt(gSProvider, gSModel, {{
Write exactly 10 valid Csound i-statements for instrument Tone.

p2 = onset time in seconds
p3 = duration in seconds (0.6 to 1.5)
p4 = amplitude from 0.15 to 0.35
p5 = frequency in Hz between 146 and 523

Make a sparse rising arpeggio lasting at most 10 seconds.
Return only the i-statements, one per line.
No comments, markdown, or f/e statements.
}})

    prints("Generated score:\n%s\n", Sscore)
    scorelinei(Sscore)

    event("e", 0, 12)
endin

schedule("Compose", 0, 1)

</CsInstruments>
<CsScore>
</CsScore>
</CsoundSynthesizer>
