<CsoundSynthesizer>
<CsOptions>
-odac -d
</CsOptions>
<CsInstruments>

; Async orchestra compile — modelprompt_orc_async / modelprompt_orc_result.
;
; Requires:
;   export ANTHROPIC_API_KEY=...
;   modelprompt plugin on OPCODE7DIR64 / CS_USER_PLUGINDIR
;
; Run:
;   csound anthropic_sonnet_orc_async.csd
;
; The orchestra request runs in a worker thread. Csound keeps performing
; (here: a soft tick) until modelprompt_orc_result compiles the returned
; fragment on the Csound thread, then a short score is injected.
;
; Prefer -odac (realtime) on the first run so the model has wall-clock time
; to answer before the score ends. For offline -o renders, freeze with
; iregenerate=0 after a successful cache is on disk.

sr = 48000
ksmps = 32
nchnls = 2
0dbfs = 1

gSProvider = "anthropic"
gSModel = "claude-sonnet-5"

giOrcHandle init 0
gkOrcDone init 0

instr Boot
    prints("\n=== modelprompt_orc_async demo ===\n\n")

    giOrcHandle = modelprompt_orc_async(gSProvider, gSModel, {{
Write a complete Csound orchestra fragment.

CRITICAL RULES:
- outleta must be: outleta "name", asig  (a-rate variable only; never 0).
- inleta: aL inleta "leftin"
- Do not use outs. No score, markdown, or commentary.

instr Tone
  ; p4 amp 0..1, p5 Hz. oscili + linen. outleta leftout/rightout (identical).
endin

instr Reverb
  ; aL/aR inleta; reverbsc; wet ~0.35; outleta leftout/rightout.
endin

instr Master
  ; aL/aR inleta; outc aL, aR
endin

connect "Tone", "leftout", "Reverb", "leftin"
connect "Tone", "rightout", "Reverb", "rightin"
connect "Reverb", "leftout", "Master", "leftin"
connect "Reverb", "rightout", "Master", "rightin"
alwayson "Reverb"
alwayson "Master"
}})

    prints("Orchestra request started (handle %d).\n", giOrcHandle)
    schedule("Waiter", 0, 120)
    schedule("Heartbeat", 0, 120)
    ; Realtime budget for the first (uncached) model round-trip.
    event("e", 0, 60)
endin

instr Heartbeat
    ; Audible activity while the model request is in flight.
    aenv linen 0.03, 0.01, 0.08, 0.05
    asig oscili aenv, 880
    outc(asig, asig)
    schedule("Heartbeat", 1.0, 0.1)
    turnoff
endin

instr Waiter
    kstatus, Sorc modelprompt_orc_result giOrcHandle

    if kstatus == 1 then
        if gkOrcDone == 0 then
            gkOrcDone = 1
            printf("\n=== Orchestra compiled ===\n%s\n", 1, Sorc)
            scoreline({{
i "Tone" 0 0.8 0.25 220
i "Tone" 0.9 0.8 0.22 277
i "Tone" 1.8 1.2 0.2 330
i "Tone" 3.2 1.5 0.18 440
}}, 1)
            ; Allow notes + reverb tail, then end.
            event("e", 0, 8)
            turnoff
        endif
    elseif kstatus < 0 then
        printf("\n=== Orchestra request failed (status %d) ===\n%s\n",
               1, kstatus, Sorc)
        event("e", 0, 0.5)
        turnoff
    endif
endin

schedule("Boot", 0, 1)

</CsInstruments>
<CsScore>
</CsScore>
</CsoundSynthesizer>
