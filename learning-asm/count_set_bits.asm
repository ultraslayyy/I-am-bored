; Created:  Tuesday, May 12, 2026 10:24:04 PM
; Finished: Tuesday, May 12, 2026 08:29:56 PM

count_set_bits:
  xor eax, eax
  test edi, edi ; is edi 0? Works like bitwise and so and against itself checks 0
  jz .end       ; Jump because zero

.loop_body:
  test edi, 1   ; Same as 'and edi, 1', but doesn't change value
  jz .skip_inc  ; Jump to .shift if Zero Flag (ZF) == 1, meaning it is 0

  inc eax

.skip_inc:
  shr edi, 1
  jnz .loop_body ; Jump to .loop_body if Zero Flag (ZF) == 0, meaning it is not 0

.end:
  ret