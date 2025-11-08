;==============================================================
; ParallelCombNotes5
; aOut = ParallelCombNotes5(aIn, kWet, kCut, kR, iNote1, iNote2, iNote3, iNote4, iNote5)
; - iNote# are MIDI notes; pass -1 to disable that slot (default for j).
; - kWet in [0,1], kCut = lowpass cutoff (Hz) after each comb, kR = resonance
;==============================================================
opcode ParallelCombNotes5, a, akkkjjjjj
  aIn, kWet, kCut, kR, iN1, iN2, iN3, iN4, iN5 xin

  aSum init 0
  kCount init 0

  ; --- note 1 ---
  if (iN1 >= 0) then
    iF1  = cpsmidinn(iN1)
    iD1  = 1. / iF1
    aC1  comb aIn, kR, iD1
    aD1  tone aC1, kCut
    aSum = aSum + aD1
    kCount = kCount + 1
  endif

  ; --- note 2 ---
  if (iN2 >= 0) then
    iF2  = cpsmidinn(iN2)
    iD2  = 1. / iF2
    aC2  comb aIn, kR, iD2
    aD2  tone aC2, kCut
    aSum = aSum + aD2
    kCount = kCount + 1
  endif

  ; --- note 3 ---
  if (iN3 >= 0) then
    iF3  = cpsmidinn(iN3)
    iD3  = 1. / iF3
    aC3  comb aIn, kR, iD3
    aD3  tone aC3, kCut
    aSum = aSum + aD3
    kCount = kCount + 1
  endif

  ; --- note 4 ---
  if (iN4 >= 0) then
    iF4  = cpsmidinn(iN4)
    iD4  = 1. / iF4
    aC4  comb aIn, kR, iD4
    aD4  tone aC4, kCut
    aSum = aSum + aD4
    kCount = kCount + 1
  endif

  ; --- note 5 ---
  if (iN5 >= 0) then
    iF5  = cpsmidinn(iN5)
    iD5  = 1. / iF5
    aC5  comb aIn, kR, iD5
    aD5  tone aC5, kCut
    aSum = aSum + aD5
    kCount = kCount + 1
  endif

  aWet = aSum / kCount
  kDry = 1. - kWet
  aOut = (aWet * kWet) + (aIn * kDry)
  xout aOut
endop
