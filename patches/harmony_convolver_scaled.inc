/**
# harmony_convolver family

All variants build a sparse pitch-class x octave lattice of enveloped sinusoids,
add a Dirac at sample 0, L2-normalize the whole impulse response, and convolve
once with `ftconv`.

Per-partial window length (highs not over-long):
```
T(f) = min(T, n_max / f),   n_max = T * 440
```
Each partial uses a **causal** half-cosine fade-out on \([0,T(f)]\)
(starts at 1, ends at 0). A full Hann would peak mid-window and delay the
audible/visible onset of the grain.

Per-partial inverse IEC A-weighting (gain 1 at 1 kHz), with a rumble shelf and
boost cap so wind does not dominate field recordings:
```
f'   = max(f, 150 Hz)              ; shelf: no extra rise below 150 Hz
w(f) = min( R_A(1000)/R_A(f'),  10^(6/20) )   ; cap boost at +6 dB
```

## Duration normalization

Levelling the octaves happens in two stages: first make every partial equally
*loud in physical terms*, then let the inverse A-weighting above convert that
into equal *subjective* loudness.

For the first stage, what counts is grain energy, not amplitude and not the
window integral. The sources this is used on are broadband (clicks, noise, field
recordings), so for excitation of power spectral density S0 the output power
contributed by partial k is, by Parseval,
```
P_k = S0 * integral |G_k(f)|^2 df = S0 * integral g_k(t)^2 dt = S0 * E_k
```
Every grain is narrower than one auditory filter — at most 0.74 ERB at the
bottom of the lattice and under 0.5 ERB elsewhere — so loudness tracks that
power rather than spectral density, and equal loudness means equal E_k.

The window integral A(f) T(f) governs only the *peak* transfer gain |H(f_k)|.
Equalizing it over-brightens, because the grain bandwidth ~1/T(f) widens as
T(f) shrinks: flat peak gain across a widening band is rising power.

The variants differ only in the exponent `p` of the duration correction
```
A(f) = c * ( T / T(f) )^p = c * max(1, f/440)^p
```
where `c` is chosen so total grain energy is independent of `p`. This keeps
`i_impulse_gain` and `i_dirac_level` meaning the same thing in every variant.

| opcode                             | p   | equalizes                        |
|------------------------------------|-----|----------------------------------|
| `harmony_convolver_equalamplitude` | 0   | peak amplitude (original)        |
| `harmony_convolver_equalenergy`    | 0.5 | energy = output power per partial|
| `harmony_convolver_equalintegral`  | 1   | peak transfer gain (bright)      |

`harmony_convolver_scaled` is an alias for the equal-energy variant, which is
the correct target for broadband sources and the current default.

## Critical-band density correction

Equal power per partial is not the same as equal power per auditory filter:
partials closer together than one ERB are heard as one band and sum. The lattice
is unresolved at the *bottom*, since an ERB spans about 27 semitones at 20 Hz
but only 1.9 semitones above 4 kHz, so the low octaves crowd several chord tones
into one band while the high octaves separate all but semitone neighbours.

This is a different goal from equal loudness per partial, so it is a selectable
option rather than the default: pass `i_per_band` = 1 (or call
`harmony_convolver_criticalband`) to equalize auditory filters instead of
partials. Intermediate values blend the two. Per-partial keeps each lattice tone
reading equally; per-band makes the harmony sit evenly across the spectrum, at
the cost of pulling down the crowded low octaves by 4 to 6 dB.

The correction accumulates a density for each partial using the roex(p)
approximation of the auditory filter,
```
ERB(f) = 24.7 * (4.37 f / 1000 + 1)      Hz
p      = 4 f / ERB(f)
g_jk   = |f_j - f_k| / f_k
D_k    = sum_j (1 + p g_jk) exp(-p g_jk)
```
and scales that partial by D_k^(-1/2). A roex kernel is used rather than a count
of partials inside the ERB because a hard count alternates abruptly between 0
and -3 dB as semitone neighbours cross the band edge.

Total grain energy is held constant across every combination of exponent and
correction, so `i_impulse_gain` and `i_dirac_level` keep their meaning
throughout.

## Syntax

```
a_output harmony_convolver_scaled a_input, i_impulse_duration, i_impulse_gain,
  i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3,
  i_pitch_class_4, i_pitch_class_5 [, i_per_band]
```

Pitch classes are reduced modulo 12, so scale degrees written above the octave
work as expected: 12 is C, 14 is D, 17 is F, 21 is A. Unused pitch classes may
be omitted or passed as any negative value.

`i_per_band` defaults to 0 (equalize partials); 1 equalizes auditory filters.
Since the pitch classes are optional, all five must be present before
`i_per_band` can be given.

*/

/**
Shared implementation. `i_duration_exponent` selects the duration
normalization and `i_erb_correction` (0 to 1, default 0) blends in the
critical-band density correction; the pitch classes are required here (the
public wrappers supply the -1 defaults).
*/
opcode harmony_convolver_equalenergy_impl, a, aiiiiiiiiio
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_duration_exponent, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, i_erb_correction xin

  i_T = max(0.01, min(abs(i_impulse_duration), 1.5))
  i_f_pivot = 440.0
  i_n_max = i_T * i_f_pivot

  ; Reduce to pitch-classes, so that e.g. 14 means D and 17 means F. Any
  ; negative value stays negative and marks an unused slot.
  i_pc1 = (i_pitch_class_1 >= 0 ? i_pitch_class_1 % 12 : -1)
  i_pc2 = (i_pitch_class_2 >= 0 ? i_pitch_class_2 % 12 : -1)
  i_pc3 = (i_pitch_class_3 >= 0 ? i_pitch_class_3 % 12 : -1)
  i_pc4 = (i_pitch_class_4 >= 0 ? i_pitch_class_4 % 12 : -1)
  i_pc5 = (i_pitch_class_5 >= 0 ? i_pitch_class_5 % 12 : -1)

  i_nyquist_frequency = sr * 0.5 * 0.95
  i_frequency_list[] init 128
  i_loud_weight[] init 128
  i_window_length[] init 128
  i_partial_gain[] init 128
  i_frequency_count init 0

  ; IEC A-weighting R_A(f); weight = R_A(1000)/R_A(f'), capped.
  i_fref = 1000.0
  i_f_shelf = 150.0
  i_w_boost_max = 1.9952623149688796
  i_f2r = i_fref * i_fref
  i_f4r = i_f2r * i_f2r
  i_c1 = 12194.0 * 12194.0
  i_c2 = 20.6 * 20.6
  i_c3 = 107.7 * 107.7
  i_c4 = 737.9 * 737.9
  i_ra_1k = (i_c1 * i_f4r) / ((i_f2r + i_c2) * sqrt((i_f2r + i_c3) * (i_f2r + i_c4)) * (i_f2r + i_c1))

  i_midi_note = 0
  while (i_midi_note <= 127) do
    i_pitch_class = i_midi_note % 12
    i_is_selected = 0
    if (i_pc1 >= 0 && i_pitch_class == i_pc1) then
      i_is_selected = 1
    endif
    if (i_pc2 >= 0 && i_pitch_class == i_pc2) then
      i_is_selected = 1
    endif
    if (i_pc3 >= 0 && i_pitch_class == i_pc3) then
      i_is_selected = 1
    endif
    if (i_pc4 >= 0 && i_pitch_class == i_pc4) then
      i_is_selected = 1
    endif
    if (i_pc5 >= 0 && i_pitch_class == i_pc5) then
      i_is_selected = 1
    endif
    if (i_is_selected == 1) then
      i_frequency = cpsmidinn(i_midi_note)
      if (i_frequency >= 20 && i_frequency <= i_nyquist_frequency) then
        i_f_w = i_frequency
        if (i_f_w < i_f_shelf) then
          i_f_w = i_f_shelf
        endif
        i_f2 = i_f_w * i_f_w
        i_f4 = i_f2 * i_f2
        i_ra = (i_c1 * i_f4) / ((i_f2 + i_c2) * sqrt((i_f2 + i_c3) * (i_f2 + i_c4)) * (i_f2 + i_c1))
        if (i_ra < 1.0e-30) then
          i_ra = 1.0e-30
        endif
        i_w = i_ra_1k / i_ra
        if (i_w > i_w_boost_max) then
          i_w = i_w_boost_max
        endif
        i_T_f = min(i_T, i_n_max / i_frequency)
        i_frequency_list[i_frequency_count] = i_frequency
        i_loud_weight[i_frequency_count] = i_w
        i_window_length[i_frequency_count] = i_T_f
        i_partial_gain[i_frequency_count] = (i_T / i_T_f) ^ i_duration_exponent
        i_frequency_count = i_frequency_count + 1
      endif
    endif
    i_midi_note = i_midi_note + 1
  od

  ; Critical-band density: roex(p) approximation of the auditory filter, so a
  ; partial sharing a band with neighbours is attenuated by sqrt of the number
  ; of partials effectively summing there.
  if (i_erb_correction > 0) then
    i_index = 0
    while (i_index < i_frequency_count) do
      i_f_centre = i_frequency_list[i_index]
      i_erb = 24.7 * (4.37 * i_f_centre / 1000.0 + 1.0)
      i_roex_p = 4.0 * i_f_centre / i_erb
      i_density = 0.0
      i_other = 0
      while (i_other < i_frequency_count) do
        i_pg = i_roex_p * abs(i_frequency_list[i_other] - i_f_centre) / i_f_centre
        i_density = i_density + (1.0 + i_pg) * exp(-i_pg)
        i_other = i_other + 1
      od
      i_partial_gain[i_index] = i_partial_gain[i_index] * (max(i_density, 1.0) ^ (-0.5 * i_erb_correction))
      i_index = i_index + 1
    od
  endif

  ; Rescale the per-partial gains so that sum(A^2 * T_f) matches the plain
  ; equal-amplitude case. Total grain energy, hence the grain/Dirac balance
  ; after the joint L2 below, is then the same for every exponent and every
  ; correction, and only the spectral tilt changes.
  i_reference_energy = 0.0
  i_weighted_energy = 0.0
  i_index = 0
  while (i_index < i_frequency_count) do
    i_T_f = i_window_length[i_index]
    i_g = i_partial_gain[i_index]
    i_reference_energy = i_reference_energy + i_T_f
    i_weighted_energy = i_weighted_energy + (i_g * i_g * i_T_f)
    i_index = i_index + 1
  od
  i_gain_scale = sqrt(i_reference_energy / max(i_weighted_energy, 1.0e-12))
  i_index = 0
  while (i_index < i_frequency_count) do
    i_partial_gain[i_index] = i_partial_gain[i_index] * i_gain_scale
    i_index = i_index + 1
  od

  i_sample_count = int(i_T * sr + 0.5)
  if (i_sample_count < 2) then
    i_sample_count = 2
  endif

  i_impulse_response_table ftgen 0, 0, -i_sample_count, 7, 0, i_sample_count, 0

  if (i_frequency_count > 0) then
    i_two_pi = 6.283185307179586
    i_sample_index = 0
    while (i_sample_index < i_sample_count) do
      i_time_seconds = i_sample_index / sr
      i_sum_of_sines = 0.0
      i_frequency_index = 0
      while (i_frequency_index < i_frequency_count) do
        i_frequency = i_frequency_list[i_frequency_index]
        i_T_f = i_window_length[i_frequency_index]
        if (i_time_seconds < i_T_f) then
          ; Causal fade-out (half-cosine): 1 at t=0, 0 at t=T_f.
          ; A full Hann peaks at mid-window and makes the effect look late.
          i_hann = 0.5 + 0.5 * cos(i_two_pi * 0.5 * i_time_seconds / i_T_f)
          ; Per-partial inverse A-weight, duration and density corrections.
          i_amplitude = i_loud_weight[i_frequency_index] * i_partial_gain[i_frequency_index]
          i_sum_of_sines = i_sum_of_sines + i_amplitude * i_hann * sin(i_two_pi * i_frequency * i_time_seconds)
        endif
        i_frequency_index = i_frequency_index + 1
      od
      i_sample_value = i_sum_of_sines / i_frequency_count
      tableiw i_sample_value * i_impulse_gain, i_sample_index, i_impulse_response_table
      i_sample_index = i_sample_index + 1
    od
  endif

  ; Dirac in the IR, then joint L2 — same as the audible original.
  tableiw (table(0, i_impulse_response_table) + i_dirac_level), 0, i_impulse_response_table

  i_energy = 0.0
  i_idx = 0
  while (i_idx < i_sample_count) do
    i_val = table(i_idx, i_impulse_response_table)
    i_energy = i_energy + (i_val * i_val)
    i_idx = i_idx + 1
  od
  i_scale = 1.0 / sqrt(max(i_energy, 1.0e-12))
  i_idx = 0
  while (i_idx < i_sample_count) do
    tableiw table(i_idx, i_impulse_response_table) * i_scale, i_idx, i_impulse_response_table
    i_idx = i_idx + 1
  od

  a_convolved ftconv a_source, i_impulse_response_table, 128
  xout a_convolved
endop

/**
Equal peak amplitude per partial: the original `harmony_convolver_scaled`.
Output power per partial falls off as 1/f above 440 Hz.
*/
opcode harmony_convolver_equalamplitude, a, aiiijjjjjo
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, i_per_band xin
  a_output harmony_convolver_equalenergy_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, 0.0, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, i_per_band
  xout a_output
endop

/**
Equal energy per partial: A(f) ~ 1/sqrt(T(f)), so every partial delivers the
same output power from a broadband source.
*/
opcode harmony_convolver_equalenergy, a, aiiijjjjjo
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, i_per_band xin
  a_output harmony_convolver_equalenergy_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, 0.5, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, i_per_band
  xout a_output
endop

/**
Equal window integral per partial: A(f) ~ 1/T(f), so every partial contributes
the same peak transfer gain at its own frequency. Bright, because the grain
bandwidth widens as the window shortens.
*/
opcode harmony_convolver_equalintegral, a, aiiijjjjjo
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, i_per_band xin
  a_output harmony_convolver_equalenergy_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, 1.0, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, i_per_band
  xout a_output
endop

/**
Equal energy with the density correction fully on: equal power per auditory
filter rather than per partial. Same as passing i_per_band = 1 to
`harmony_convolver_scaled`, kept as a name for legibility at the call site.
*/
opcode harmony_convolver_criticalband, a, aiiijjjjj
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5 xin
  a_output harmony_convolver_equalenergy_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, 0.5, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, 1.0
  xout a_output
endop

/**
Default: equal energy, hence equal output power per partial for a broadband
source, with the inverse A-weighting on top.

`i_per_band` (optional, default 0) chooses which of the two levelling goals
applies: 0 equalizes each partial, 1 equalizes each auditory filter, and
intermediate values blend. Because the pitch classes are themselves optional,
all five must be given (use -1 for unused ones) before `i_per_band` can be
passed.
*/
opcode harmony_convolver_scaled, a, aiiijjjjjo
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, i_per_band xin
  a_output harmony_convolver_equalenergy a_source, i_impulse_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5, i_per_band
  xout a_output
endop
