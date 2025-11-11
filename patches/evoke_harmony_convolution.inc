/**
# evoke_harmony_convolution

evoke_harmony_convolution -- evoke some harmonic content from an input signal.

## Description 

Evokes harmonic content from an input signal by convolving it with an impulse 
response containing the harmony in the form of a grain of sound containing 
each octave of one or more pitch-classes.

## Syntax

```
a_output evoke_harmony_convolution k_kernel_duration, i_impulse_gain, 
  i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, 
  i_pitch_class_4, i_pitch_class_5
```

## Initialization

`i_impulse_gain` -- the impulse response is multiplied by this before use.

`i_dirac_level` -- the amplitude of the first sample of the grain. If not 0, 
transmits the original input signal to the output signal at the specified 
gain.

`i_pitch_class_1` through `i_pitch_class_5 `-- a sinusoid is added to the 
impulse response at each octave of the pitch-class, from MIDI key numbers 0 
through 127. Note that the pitch-classes are not limited to integers in 
[0, 12), but can be any pitch in that range. The impulse response is 
normalized to the L2 norm of amplitude, and the impulse gain and Dirac level 
are relative to that. The pitch-classes are optional; parameters that are not 
specified or not greater than -1 are not used.

## Performance

`a_output` -- the monophonic audio output signal that results from convolving 
`a_input` with the impulse response containing the harmony.

`k_kernal_duration` -- the duration of the impulse response grain; determines 
the amount of time that energy in the input signal will 'ring' from the
harmony in the impulse response.

`a_input` -- a monophonic audio input signal.

Note that harmonic content can be evoked from the input signal _only_ when 
there is at least _some_ energy at _some_ octave of one of the pitch-classes.

*/
opcode evoke_harmony_convolution, a, akiijjjjj
  a_source, k_kernel_duration, i_impulse_gain, i_dirac_level, i_pitch_class_1, i_pitch_class_2, i_pitch_class_3, i_pitch_class_4, i_pitch_class_5 xin

  ; clamp duration and copy to i-rate for table sizing/envelope timing
  k_kernel_duration = max(0.01, min(k_kernel_duration, 1.0))
  i_kernel_duration = i(k_kernel_duration)

  ; cache pitch classes
  i_pc1 = i_pitch_class_1
  i_pc2 = i_pitch_class_2
  i_pc3 = i_pitch_class_3
  i_pc4 = i_pitch_class_4
  i_pc5 = i_pitch_class_5

  ; Compute a sinusoid for each octave of every specified pitch-class.
  i_nyquist_frequency   = sr * 0.5 * 0.95
  i_frequency_list[]    init 128
  i_frequency_count     init 0

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
        i_frequency_list[i_frequency_count] = i_frequency
        i_frequency_count = i_frequency_count + 1
      endif
    endif

    i_midi_note = i_midi_note + 1
  od

  ; build impulse response table
  i_sample_count = int(i_kernel_duration * sr + 0.5)
  if (i_sample_count < 2) then
    i_sample_count = 2
  endif

  i_impulse_response_table ftgenonce 0, 0, i_sample_count, 7, 0, 0, 1, 0

  if (i_frequency_count > 0) then
    i_two_pi = 6.283185307179586
    i_sample_index = 0
    while (i_sample_index < i_sample_count) do
      i_time_seconds = i_sample_index / sr
      ; Hanning window over [0, i_sample_count-1]
      i_hanning_window = 0.5 - 0.5 * cos( (i_two_pi * i_sample_index) / (i_sample_count - 1) )
      i_sum_of_sines = 0.0

      i_frequency_index = 0
      while (i_frequency_index < i_frequency_count) do
        i_sum_of_sines = i_sum_of_sines + sin(i_two_pi * i_frequency_list[i_frequency_index] * i_time_seconds)
        i_frequency_index = i_frequency_index + 1
      od

      i_sample_value = (i_frequency_count > 0 ? i_sum_of_sines / i_frequency_count : 0.0)
      i_sample_value = i_sample_value * i_hanning_window

      tableiw i_sample_value * i_impulse_gain, i_sample_index, i_impulse_response_table
      i_sample_index = i_sample_index + 1
    od
  endif

  ; Add the Dirac (direct component baked into IR) at the first sample.
  tableiw (table(0, i_impulse_response_table) + i_dirac_level), 0, i_impulse_response_table

  ; ------------------------------
  ; L2 normalization of the impulse
  ; ------------------------------
  i_energy = 0.0
  i_idx = 0
  while (i_idx < i_sample_count) do
    i_val = table(i_idx, i_impulse_response_table)
    i_energy = i_energy + (i_val * i_val)
    i_idx = i_idx + 1
  od

  ; compute scale so that sqrt(energy) == 1
  i_eps = 1.0e-12
  i_den = sqrt(max(i_energy, i_eps))
  i_scale = 1.0 / i_den

  ; apply normalization to the entire IR (including the Dirac)
  i_idx = 0
  while (i_idx < i_sample_count) do
    i_val = table(i_idx, i_impulse_response_table)
    tableiw (i_val * i_scale), i_idx, i_impulse_response_table
    i_idx = i_idx + 1
  od

  ; Partitioned convolution from the normalized table.
  i_partition_length = 128
  a_convolved ftconv a_source, i_impulse_response_table, i_partition_length

  xout a_convolved
endop
