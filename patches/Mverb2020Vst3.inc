; Use with, e.g.: gi_Mverb2020 vstinit "/home/mkg/.local/lib/Mverb2020.so", 1
; Obtain from: http://socalabs.com/effects/mverb2020/
;gi_Mverb2020 vst3init "/Library/Audio/Plug-Ins/VST3/Mverb2020.vst3", "Pianoteq 7", 1
;vst3info gi_Mverb2020
; Obtain from: http://socalabs.com/effects/mverb2020/

gk_Mverb2020_level chnexport "gk_Mverb2020_level", 3 ; 0
gk_Mverb2020_Mix chnexport "gk_Mverb2020_Mix", 3 ; .5
gk_Mverb2020_Pre_delay chnexport "gk_Mverb2020_Pre_delay", 3 ; 0.5
gk_Mverb2020_Early_late_mix chnexport "gk_Mverb2020_Early_late_mix", 3 ; 0.5
gk_Mverb2020_Size chnexport "gk_Mverb2020_Size", 3 ; 0.5
gk_Mverb2020_Density chnexport "gk_Mverb2020_Density", 3 ; 0.5
gk_Mverb2020_Bandwith_Frequency chnexport "gk_Mverb2020_Bandwith_Frequency", 3 ; 0.5
gk_Mverb2020_Decay chnexport "gk_Mverb2020_Decay", 3 ; 0.85
gk_Mverb2020_Damping_Frequency chnexport "gk_Mverb2020_Damping_Frequency", 3 ; 0.5
gk_Mverb2020_Gain chnexport "gk_Mverb2020_Gain", 3 ; 1
gi_Mverb2020_Program chnexport "gi_Mverb2020_Program", 3 ; 4

gk_Mverb2020_level init 0
gk_Mverb2020_Mix init .5
gk_Mverb2020_Pre_delay init 0.5
gk_Mverb2020_Early_late_mix init 0.5
gk_Mverb2020_Size init 0.5
gk_Mverb2020_Density init 0.5
gk_Mverb2020_Bandwith_Frequency init 0.5
gk_Mverb2020_Decay init 0.85
gk_Mverb2020_Damping_Frequency init 0.5
gk_Mverb2020_Gain init 1
gi_Mverb2020_Program init 4

; instr Reverb
; a_in_left inleta "inleft"
; a_in_right inleta "inright"
; k_old_size vst3paramget gi_vst3_handle_ambience, 0
; vst3paramset gi_vst3_handle_ambience, 0, .95
; vst3paramset gi_vst3_handle_ambience, 3, .9
; vst3presetsave gi_vst3_handle_ambience, "ambience.preset"
; a_out_left, a_out_right vst3audio gi_vst3_handle_ambience, a_in_left, a_in_right
; outleta "outleft", a_out_left
; outleta "outright", a_out_right
; prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
; endin

instr Mverb2020
;vst3progset gi_Mverb2020, gi_Mverb2020_Program
vst3paramset gi_Mverb2020, 1, gk_Mverb2020_Mix
;vstparamset gi_Mverb2020, 1, gk_Mverb2020_Pre_delay
;vstparamset gi_Mverb2020, 2, gk_Mverb2020_Early_late_mix
;vstparamset gi_Mverb2020, 3, gk_Mverb2020_Size
;vstparamset gi_Mverb2020, 4, gk_Mverb2020_Density
;vstparamset gi_Mverb2020, 5, gk_Mverb2020_Bandwith_Frequency
vst3paramset gi_Mverb2020, 6, gk_Mverb2020_Decay
;vstparamset gi_Mverb2020, 7, gk_Mverb2020_Damping_Frequency
vst3paramset gi_Mverb2020, 8, gk_Mverb2020_Gain
;vst3presetsave gi_Mverb2020, "Mverb2020.preset"
;k_gain = ampdb(gk_Mverb2020_level)
a_in_left inleta "inleft"
k_rms_left rms a_in_left
a_in_right inleta "inright"
k_rms_right rms a_in_right
a_out_left, a_out_right vst3audio gi_Mverb2020, a_in_left, a_in_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "hi\n"
printks "rms in %9.4f %9.4f\n", 1, k_rms_left, k_rms_right
prints  "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
prints "bye\n"
endin
