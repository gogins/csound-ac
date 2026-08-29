/**
# chord_convolver family

All variants build a sparse pitch-class x octave lattice of enveloped sinusoids,
add a Dirac at sample 0, L2-normalize the whole impulse response, and convolve
once with `ftconv`.

Terminology: a *grain* is one causal half-cosine windowed sinusoid
(pitch-class c, octave n). Its frequency response is a *kernel* `H_{c,n}`.
The *impulse response* is the sum of the loudness-balanced grains plus a
Dirac, then jointly L2-normalized. For a chord C those kernels form an
octave-related scaled family.

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
`i_impulse_gain` and `i_dirac_gain` meaning the same thing in every variant.

`i_compensation_type` selects the variant:

| type | p   | equalizes                        |
|------|-----|----------------------------------|
| 0    | 0   | peak amplitude                   |
| 1    | 0.5 | energy = output power per partial|
| 2    | 1   | peak transfer gain (bright)      |
| 3    | 0.5 | energy per auditory filter (ERB) |

Type 1 (equal energy) is the correct target for broadband sources. The named
opcodes `chord_convolver_equalamplitude`, `_equalenergy`, `_equalintegral`,
and `_criticalband` are aliases of `chord_convolver` with the same
signature; they honour `i_compensation_type` rather than overriding it.

## Critical-band density correction

Equal power per partial is not the same as equal power per auditory filter:
partials closer together than one ERB are heard as one band and sum. The lattice
is unresolved at the *bottom*, since an ERB spans about 27 semitones at 20 Hz
but only 1.9 semitones above 4 kHz, so the low octaves crowd several chord tones
into one band while the high octaves separate all but semitone neighbours.

This is a different goal from equal loudness per partial, so it is compensation
type 3 rather than the default. Per-partial keeps each lattice tone reading
equally; per-band makes the harmony sit evenly across the spectrum, at the cost
of pulling down the crowded low octaves by 4 to 6 dB.

The correction accumulates a density for each partial using the roex(p)
approximation of the auditory filter,
```
ERB(f) = 24.7 * (4.37 f / 1000 + 1)      Hz
p      = 4 f / ERB(f)
g_jk   = |f_j - f_k| / f_k
D_k    = sum_j (1 + p g_jk) exp(-p g_jk)
```
and scales that partial by D_k^(-1/2). A roex(p) auditory-filter shape is used
rather than a count of partials inside the ERB because a hard count alternates
abruptly between 0 and -3 dB as semitone neighbours cross the band edge.

Total grain energy is held constant across every combination of exponent and
correction, so `i_impulse_gain` and `i_dirac_gain` keep their meaning
throughout.

## Syntax

```
a_output chord_convolver a_input, i_impulse_duration, i_impulse_gain,
  i_dirac_gain, i_compensation_type [, i_pc1] [, i_pc2] ...
```

`i_impulse_duration` is T, the grain-duration cap (and the length of the
impulse-response table). `i_impulse_gain` scales the summed grains (the
lattice); `i_dirac_gain` is the dry path at sample 0.

Pitch-classes are the only optional arguments; any number may be given, or an
i-rate array. They are reduced modulo 12, so scale degrees written above the
octave work as expected: 12 is C, 14 is D, 17 is F, 21 is A. Negative values are
ignored (and, in a score, overwrite carried p-fields from a previous event).

`i_compensation_type` is required: 0 amplitude, 1 energy, 2 integral, 3
critical-band.

*/

/**
Shared implementation. Pitch-classes arrive as an i-rate array; public
overloads pack a variable-length argument list into that array. Csound UDOs
cannot use the native `m` type, so each public name is overloaded for 0–12
scalars plus an array form.
*/
opcode chord_convolver_impl, a, aiiiii[]
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials[] xin

  i_T = max(0.01, min(abs(i_impulse_duration), 1.5))
  i_f_pivot = 440.0
  i_n_max = i_T * i_f_pivot

  i_duration_exponent = 0.5
  i_erb_correction = 0.0
  if (i_compensation_type == 0) then
    i_duration_exponent = 0.0
  elseif (i_compensation_type == 2) then
    i_duration_exponent = 1.0
  elseif (i_compensation_type == 3) then
    i_erb_correction = 1.0
  endif

  ; Pitch-class set: reduce mod 12, ignore negatives.
  i_pc_used[] init 12
  i_pc_index = 0
  while (i_pc_index < 12) do
    i_pc_used[i_pc_index] = 0
    i_pc_index = i_pc_index + 1
  od
  i_pc_index = 0
  i_n_partials = lenarray(i_partials)
  while (i_pc_index < i_n_partials) do
    i_pc = i_partials[i_pc_index]
    if (i_pc >= 0) then
      i_pc_used[i_pc % 12] = 1
    endif
    i_pc_index = i_pc_index + 1
  od

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
    if (i_pc_used[i_pitch_class] == 1) then
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
  ; equal-amplitude case. Total grain energy, hence the lattice/Dirac balance
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
  tableiw (table(0, i_impulse_response_table) + i_dirac_gain), 0, i_impulse_response_table

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

opcode chord_convolver, a, aiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type xin
  i_partials[] init 0
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1 xin
  i_partials[] fillarray i_pc1
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2 xin
  i_partials[] fillarray i_pc1, i_pc2
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver, a, aiiiii[]
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials[] xin
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type xin
  i_partials[] init 0
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1 xin
  i_partials[] fillarray i_pc1
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2 xin
  i_partials[] fillarray i_pc1, i_pc2
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalamplitude, a, aiiiii[]
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials[] xin
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type xin
  i_partials[] init 0
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1 xin
  i_partials[] fillarray i_pc1
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2 xin
  i_partials[] fillarray i_pc1, i_pc2
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalenergy, a, aiiiii[]
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials[] xin
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type xin
  i_partials[] init 0
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1 xin
  i_partials[] fillarray i_pc1
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2 xin
  i_partials[] fillarray i_pc1, i_pc2
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_equalintegral, a, aiiiii[]
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials[] xin
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type xin
  i_partials[] init 0
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1 xin
  i_partials[] fillarray i_pc1
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2 xin
  i_partials[] fillarray i_pc1, i_pc2
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiiiiiiiiiiiiii
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12 xin
  i_partials[] fillarray i_pc1, i_pc2, i_pc3, i_pc4, i_pc5, i_pc6, i_pc7, i_pc8, i_pc9, i_pc10, i_pc11, i_pc12
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop

opcode chord_convolver_criticalband, a, aiiiii[]
  a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials[] xin
  a_out chord_convolver_impl a_source, i_impulse_duration, i_impulse_gain, i_dirac_gain, i_compensation_type, i_partials
  xout a_out
endop
