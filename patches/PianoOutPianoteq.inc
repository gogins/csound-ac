// This must be initialized in the orc header before any #includes.

vstinfo gi_Pianoteq 

gk_PianoOutPianoteq_level chnexport "gk_PianoOutPianoteq_level", 3 ;  0
gi_PianoOutPianoteq_print chnexport "gi_PianoOutPianoteq_print", 3 ;  1
gk_PianoOutPianoteq_front_to_back chnexport "gk_PianoOutPianoteq_front_to_back", 3 ;  0
gk_PianoOutPianoteq_left_to_right chnexport "gk_PianoOutPianoteq_left_to_right", 3 ;  0.5
gk_PianoOutPianoteq_bottom_to_top chnexport "gk_PianoOutPianoteq_bottom_to_top", 3 ;  0

gk_PianoOutPianoteq_level init 0
gi_PianoOutPianoteq_print init 1
gk_PianoOutPianoteq_front_to_back init 0
gk_PianoOutPianoteq_left_to_right init 0.5
gk_PianoOutPianoteq_bottom_to_top init 0

instr PianoOutPianoteq
vstprogset gi_Pianoteq, 3
; Sustain off.
vstparamset gi_Pianoteq, 6, 1
; Reverb switch off.
vstparamset gi_Pianoteq, 93, 0
k_gain = ampdb(gk_PianoOutPianoteq_level)
i_overall_amps = 87
i_normalization = ampdb(-i_overall_amps) * 2
i_amplitude = ampdb(80) * i_normalization
if gi_PianoOutPianoteq_print == 1 then
  vstinfo gi_PianoOutPianoteq_print
endif
i_instrument = p1
i_time = p2
i_duration = p3
i_midi_key = p4
i_midi_velocity = p5
ainleft init 0
ainright init 0
aoutleft, aoutright vstaudio gi_Pianoteq, ainleft, ainright
a_signal = aoutleft + aoutright
a_signal *= k_gain
a_signal *= i_amplitude
a_out_left, a_out_right pan2 a_signal, gk_PianoOutPianoteq_left_to_right
; printks "vstaudio:       %9.4f   %9.4f\n", 0.5, aoutleft, aoutright

#ifdef USE_SPATIALIZATION
a_signal = a_out_left + a_out_right
a_spatial_reverb_send init 0
a_bsignal[] init 16
a_bsignal, a_spatial_reverb_send Spatialize a_signal, gk_PianoOutPianoteq_front_to_back, gk_PianoOutPianoteq_left_to_right, gk_PianoOutPianoteq_bottom_to_top
outletv "outbformat", a_bsignal
outleta "out", a_spatial_reverb_send
#else
; printks "PianoOutPt     L %9.4f R %9.4f l %9.4f\\n", 0.5, a_out_left, a_out_right, gk_Piano_level
outleta "outleft", a_out_left
outleta "outright", a_out_right
#endif
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin
