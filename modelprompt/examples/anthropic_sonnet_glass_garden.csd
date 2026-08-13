<CsoundSynthesizer>
<CsOptions>
-odac -d -m165
</CsOptions>
<CsInstruments>

; Glass Garden — a small generative showcase for modelprompt.
;
; Requires:
;   export ANTHROPIC_API_KEY=...
;   modelprompt plugin on OPCODE7DIR64 / CS_USER_PLUGINDIR
;
; Run:
;   csound anthropic_sonnet_glass_garden.csd
;
; Caching (default auto): omit iregenerate to reuse caches after the first run.
; Pass iregenerate=1 on a call to force a fresh model response.
; Pass iregenerate=0 to require a cache hit (fail if missing).
;
; Pipeline (up to 6 model calls, prompt numbers 1–6):
;   1. poetic title                -> S
;   2. pitch set                   -> i[]
;   3. Section I score (Glass/Pad) -> S
;   4. Section II (Spark enters)   -> modelprompt_async
;   5. Section III (Pulse enters)  -> modelprompt_async
;   6. connected orchestra         -> modelprompt_orc (last)
;
; Form: slow garden → brighter Spark at a higher tempo →
;       quieter Pulse at a still higher tempo.
; Each new voice is layered on; earlier instruments keep sounding.

sr = 48000
ksmps = 32
nchnls = 2
0dbfs = 1

gSProvider = "anthropic"
gSModel = "claude-sonnet-5"

giSec2Handle init 0
giSec3Handle init 0
gkSec2Injected init 0
gkSec3Injected init 0
giComposeDone init 0

instr Compose
    if giComposeDone != 0 igoto ComposeSkip
    giComposeDone = 1

    prints("\n=== Glass Garden: composing with %s / %s ===\n\n",
           gSProvider, gSModel)

    ; ------------------------------------------------------------------
    ; 1) Title (string)
    ; ------------------------------------------------------------------
    Stitle = modelprompt(gSProvider, gSModel, {{
Invent a short poetic title (3 to 6 words) for a stereo miniature that
begins with glass chimes and soft pads, then is pierced by bright sparks
and finally by quick pulsing figures. Return only the title text,
no quotes or commentary.
}})
    prints("Title: %s\n\n", Stitle)

    ; ------------------------------------------------------------------
    ; 2) Pitch material (numeric array)
    ; ------------------------------------------------------------------
    ipchs:i[] = modelprompt(gSProvider, gSModel, {{
Return exactly 8 MIDI key numbers as a JSON array of numbers.
Use a quiet, luminous mode somewhere between D Dorian and A Aeolian,
centered around MIDI 60 to 76, with one or two notes below 60 for
bass color. No duplicate consecutive values. Return only the JSON array.
}})

    ilen = lenarray(ipchs)
    prints("Pitch set (%d MIDI keys):", ilen)
    indx = 0
    while indx < ilen do
        prints(" %.1f", ipchs[indx])
        indx += 1
    od
    prints("\n\n")

    Spitches = ""
    indx = 0
    while indx < ilen do
        Stmp = sprintf("%s %.1f", Spitches, ipchs[indx])
        Spitches = Stmp
        indx += 1
    od

    ; ------------------------------------------------------------------
    ; 3) Section I — Glass + Pad (slow)
    ; ------------------------------------------------------------------
    Sprompt1 = sprintf({{
Write valid Csound i-statements for instruments Glass and Pad only.

Use this MIDI pitch set (convert each to Hz with cpsmidinn):
%s

Constraints:
- Onset times from 0 to 10 seconds (Section I only; later sections continue these voices).
- About 8 to 12 Glass notes: short chimes (durations 0.5 to 1.8).
- About 3 to 5 Pad notes: longer tones (durations 3.0 to 6.0).
- p4 amplitudes: Glass 0.35–0.55 (prominent chimes), Pad 0.06–0.12.
- p5 must be frequency in Hz (not MIDI).
- Slow, sparse garden tempo — leave space between events.
- Prefer gentle rising and falling shapes through the pitch set.

Return only i-statements, one per line.
No comments, markdown, f-statements, or e-statement.
}}, Spitches)

    Sscore1 = modelprompt(gSProvider, gSModel, Sprompt1)
    prints("Section I (Glass/Pad) score:\n%s\n", Sscore1)

    ; ------------------------------------------------------------------
    ; 4) Section II — Spark enters (higher tempo), async
    ; ------------------------------------------------------------------
    Sprompt2 = sprintf({{
Write a second section as valid Csound i-statements.

Instruments allowed: Glass, Pad, and Spark.
Spark is a bright, short metallic attack — sonically opposite the soft Pad.
Glass and Pad must CONTINUE throughout this section (not stop when Spark enters).

Reuse this MIDI pitch set (convert to Hz with cpsmidinn):
%s

Constraints:
- Absolute onset times from 11 to 21 seconds.
- Spark is the new lead: about 16 to 22 Spark notes, durations 0.08 to 0.35.
- Noticeably faster and denser than Section I (quicker successive onsets).
- Continuity (required): about 8 to 12 Glass chimes spread across 11–21s,
  plus 3 to 4 overlapping Pad tones whose envelopes cover the whole window
  (e.g. Pad onsets near 11, 14, 17 with durations 4–7 so the pad bed never drops out).
- p4: Spark 0.10–0.20, Glass 0.18–0.32, Pad 0.05–0.10.
- p5 in Hz. Last onset by time 21; notes may ring a little past that.
- Return only i-statements, one per line.
- No comments, markdown, f-statements, or e-statement.
}}, Spitches)

    giSec2Handle = modelprompt_async(gSProvider, gSModel, Sprompt2)
    prints("Section II (Spark) requested asynchronously (handle %d).\n",
           giSec2Handle)

    ; ------------------------------------------------------------------
    ; 5) Section III — Pulse enters (still higher tempo), async
    ; ------------------------------------------------------------------
    Sprompt3 = sprintf({{
Write a third section as valid Csound i-statements.

Instruments allowed: Glass, Pad, Spark, and Pulse.
Pulse is a dark, gated low-mid pulse — sonically opposite bright Spark.
Glass, Pad, and Spark must CONTINUE throughout this section (layered under Pulse).

Reuse this MIDI pitch set (convert to Hz with cpsmidinn):
%s

Constraints:
- Absolute onset times from 22 to 34 seconds.
- Pulse is the new lead at double the prior Pulse tempo: about 28 to 40 Pulse
  notes, durations 0.03 to 0.11, with roughly half the inter-onset spacing of
  a moderate motoric pulse (very tight successive onsets).
- Prefer lower members of the pitch set for Pulse (bass / tenor).
- Continuity (required):
  - Pad: 3 to 4 overlapping long tones covering 22–34 (durations 4–8).
  - Glass: about 8 to 12 chimes spread across the window.
  - Spark: about 10 to 16 accents continuing (slightly less dense than Section II).
- p4 Pulse 0.04–0.10 (+6 dB vs the previous soft bed).
- p4 others: Spark 0.08–0.16, Glass 0.18–0.32, Pad 0.05–0.10.
- p5 in Hz. Last onset by time 34; piece may ring to ~36.
- Return only i-statements, one per line.
- No comments, markdown, f-statements, or e-statement.
}}, Spitches)

    giSec3Handle = modelprompt_async(gSProvider, gSModel, Sprompt3)
    prints("Section III (Pulse) requested asynchronously (handle %d).\n\n",
           giSec3Handle)

    ; ------------------------------------------------------------------
    ; 6) Orchestra graph last (compile + alwayson FX)
    ; ------------------------------------------------------------------
    Sorc = modelprompt_orc(gSProvider, gSModel, {{
Return ONLY the following Csound orchestra, with at most small numeric
tweaks to frequencies, bandwidths, delay time, or wet mixes. Do not invent
new opcodes. Do not use mode. Do not use outs. No markdown or commentary.

instr Glass
  iamp = p4
  ifreq = p5
  aenv linen iamp, 0.005, p3, p3 * 0.35
  aexc mpulse 1, 0
  a1 reson aexc, ifreq * 1.000, ifreq * 0.008, 2
  a2 reson aexc, ifreq * 2.756, ifreq * 0.012, 2
  a3 reson aexc, ifreq * 5.404, ifreq * 0.018, 2
  a4 reson aexc, ifreq * 8.933, ifreq * 0.025, 2
  a5 reson aexc, ifreq * 13.34, ifreq * 0.035, 2
  a6 reson aexc, ifreq * 18.64, ifreq * 0.045, 2
  a7 reson aexc, ifreq * 24.82, ifreq * 0.055, 2
  a8 reson aexc, ifreq * 31.87, ifreq * 0.070, 2
  asig = (a1 + a2*0.9 + a3*0.75 + a4*0.55 + a5*0.4 + a6*0.28 + a7*0.18 + a8*0.12) * aenv * 0.28
  outleta "leftout", asig
  outleta "rightout", asig
endin

instr Pad
  iamp = p4
  ifreq = p5
  aenv linen iamp, p3 * 0.35, p3, p3 * 0.35
  a1 oscili 0.45, ifreq * 0.997
  a2 oscili 0.45, ifreq * 1.003
  a3 oscili 0.25, ifreq * 2.001
  aL = (a1 + a3) * aenv
  aR = (a2 + a3) * aenv
  outleta "leftout", aL
  outleta "rightout", aR
endin

instr Spark
  ; Bright, short metallic contrast to Pad (FM-ish + noise tick).
  iamp = p4
  ifreq = p5
  aenv expon iamp, p3, iamp * 0.001
  amod oscili ifreq * 2.7, ifreq * 5.13
  acar oscili 0.7, ifreq + amod
  aclick mpulse 1, 0
  aclick = reson(aclick, ifreq * 6.0, ifreq * 0.4, 2) * 0.15
  asig = (acar + aclick) * aenv * 0.35
  aL = asig * 0.85
  aR = asig * 1.0
  outleta "leftout", aL
  outleta "rightout", aR
endin

instr Pulse
  ; Dark gated pulse contrast to Spark (low buzz + bandpass). Soft, goes via Echo.
  iamp = p4
  ifreq = p5
  aenv linen iamp, 0.005, p3, 0.02
  abuzz vco2 0.28, ifreq, 2, 0.35
  afilt resonz abuzz, ifreq * 1.5, ifreq * 0.35
  asig = (abuzz * 0.35 + afilt * 0.65) * aenv * 0.10
  outleta "leftout", asig * 0.95
  outleta "rightout", asig * 1.05
endin

instr Echo
  ; Stereo feedback delay. Mix into named outs (avoid inline outleta exprs).
  aL inleta "leftin"
  aR inleta "rightin"
  idel = 0.28
  ifb = 0.32
  iwet = 0.3
  aLfb init 0
  aRfb init 0
  aLfb delay aL + aLfb * ifb, idel
  aRfb delay aR + aRfb * ifb, idel
  aOutL = aL * (1 - iwet) + aLfb * iwet
  aOutR = aR * (1 - iwet) + aRfb * iwet
  outleta "leftout", aOutL
  outleta "rightout", aOutR
endin

instr Reverb
  ; Do not name reverb signals aX — that identifier breaks the right outleta path.
  aL inleta "leftin"
  aR inleta "rightin"
  aRevL, aRevR reverbsc aL, aR, 0.78, 12000
  iwet = 0.35
  aOutL = aL * (1 - iwet) + aRevL * iwet
  aOutR = aR * (1 - iwet) + aRevR * iwet
  outleta "leftout", aOutL
  outleta "rightout", aOutR
endin

instr Master
  aL inleta "leftin"
  aR inleta "rightin"
  aOutL = tanh(aL * 0.9)
  aOutR = tanh(aR * 0.9)
  outc aOutL, aOutR
endin

connect "Glass", "leftout", "Echo", "leftin"
connect "Glass", "rightout", "Echo", "rightin"
connect "Spark", "leftout", "Echo", "leftin"
connect "Spark", "rightout", "Echo", "rightin"
connect "Pulse", "leftout", "Echo", "leftin"
connect "Pulse", "rightout", "Echo", "rightin"
connect "Pad", "leftout", "Reverb", "leftin"
connect "Pad", "rightout", "Reverb", "rightin"
connect "Echo", "leftout", "Reverb", "leftin"
connect "Echo", "rightout", "Reverb", "rightin"
connect "Reverb", "leftout", "Master", "leftin"
connect "Reverb", "rightout", "Master", "rightin"
alwayson "Echo"
alwayson "Reverb"
alwayson "Master"
}})

    prints("Compiled orchestra:\n%s\n", Sorc)

    scorelinei(Sscore1)
    prints("Polling for Sections II–III while Section I performs...\n\n")

    schedule("SectionWatcher", 0, 50)
    event("e", 0, 38)
ComposeSkip:
endin

instr SectionWatcher
    k2, S2 modelprompt_result giSec2Handle
    k3, S3 modelprompt_result giSec3Handle

    if k2 == 1 then
        if gkSec2Injected == 0 then
            gkSec2Injected = 1
            printf("\n=== Section II (Spark) arrived ===\n%s\n", 1, S2)
            scoreline(S2, 1)
        endif
    elseif k2 < 0 then
        if gkSec2Injected == 0 then
            gkSec2Injected = 1
            printf("\n=== Section II failed (status %d) ===\n%s\n",
                   1, k2, S2)
        endif
    endif

    if k3 == 1 then
        if gkSec3Injected == 0 then
            gkSec3Injected = 1
            printf("\n=== Section III (Pulse) arrived ===\n%s\n", 1, S3)
            scoreline(S3, 1)
        endif
    elseif k3 < 0 then
        if gkSec3Injected == 0 then
            gkSec3Injected = 1
            printf("\n=== Section III failed (status %d) ===\n%s\n",
                   1, k3, S3)
        endif
    endif

    if gkSec2Injected == 1 then
        if gkSec3Injected == 1 then
            turnoff
        endif
    endif
endin

schedule("Compose", 0, 1)

</CsInstruments>
<CsScore>
</CsScore>
</CsoundSynthesizer>
