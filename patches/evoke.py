#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Evoke rewrite: multiple opcode invocations (one per chord), summed to output.

Opcode signature (unchanged):

    a_output evoke_harmony_convolution k_kernel_duration, i_impulse_gain,
      i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3,
      i_pitch_class_4, i_pitch_class_5

Design:
- Each *chord* is rendered by a **separate instrument instance** that calls the
  opcode once (all pitch classes are i‑rate), producing its audio contribution.
- Instances sum into a **global audio bus** `ga_output`.
- A dedicated **mixer** instrument (200) runs the whole time, outputs `ga_output`
  each control cycle, then clears it. This ensures proper summing without DC
  buildup.

Type prefixes: a_ (audio), k_ (control), i_ (init), g* (globals).
"""

import ctcsound

OUT_WAV = "evoke_harmony_convolution_sum.wav"


def build_orc(sr=48000, nchnls=2):
    orc = f"""
    sr = {sr}
    ksmps = 32
    nchnls = {nchnls}
    0dbfs = 1

    #include "evoke_harmony_convolution.inc"

    ;==================== Globals ====================
    ga_input init 0
    ga_output init 0
    gk_kernel_duration init 1.0
    gi_impulse_gain    init 0.4
    gi_dirac_level     init 0.2

    instr 1
    
    endin


    ;-------------------- Per‑chord generator ---------------------
    instr 100
      ; p4..p8 are i‑rate pitch classes for this chord (use -1 for unused)
      k_kernel_duration = gk_kernel_duration
        i_impulse_gain    = gi_impulse_gain
        i_dirac_level     = gi_dirac_level
        ip4 = p4
        ip5 = p5
        ip6 = p6
        ip7 = p7
        ip8 = p8
      a_part evoke_harmony_convolution ga_input, k_kernel_duration, gi_impulse_gain, gi_dirac_level, ip4, ip5, ip6, ip7, ip8
      ga_output = ga_output + a_part
    endin

    ;------------------------ Mixer -------------------------------
    instr 200
      a_out = ga_output
      outs a_out, a_out
      clear ga_output
    endin
    """
    return orc


def build_score():
    sco_lines = []

    # Start mixer for full duration
    total_dur = 6.0
    sco_lines.append(f"i 200 0 {total_dur}")

    # Schedule multiple chords as separate opcode invocations (instrument 100)
    #  Format: i 100 <start> <dur> <pc1> <pc2> <pc3> <pc4> <pc5>
    t = 0.0
    chords = [
        (2.0, [0, 4, 7, -1, -1]),    # C major
        (2.0, [2, 5, 9, -1, -1]),    # D minor
        (2.0, [11, 2, 6, 8, -1]),    # B, D, F#, G#
    ]
    for dur, pcs in chords:
        p = " ".join(str(x) for x in pcs)
        sco_lines.append(f"i 100 {t:.3f} {dur:.3f} {p}")
        t += dur

    sco_lines.append(f"f 0 {total_dur}")
    return "\n".join(sco_lines)


def main():
    cs = ctcsound.Csound()
    orc = build_orc()
    print("Orc:")
    print(orc)
    sco = build_score()
    print(sco)
 
    cs.setOption(f"-RWdfo{OUT_WAV}")
    cs.compileOrc(orc)
    cs.readScore(sco)

    if cs.start() != ctcsound.CSOUND_SUCCESS:
        raise RuntimeError("Csound failed to start")

    cs.perform()
    cs.cleanup()
    cs.reset()
    print(f"Render complete: {OUT_WAV}")


if __name__ == "__main__":
    main()
