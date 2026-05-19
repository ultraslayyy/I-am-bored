; Created:  Wednesday, May 13, 2026 5:16:00 PM
; Finished: Wednesday, May 13, 2026 5:22:38 PM

section .bss
  buffer: resd 512 ; Reserves 2048 bytes (512 * 4) of RAM

clear_buffer:
  xor ecx, ecx

  cmp ecx, 512
  jge .end

.loop_body:
  mov dword [buffer + rcx*4], 0 ; Same as 'lea rdi, [buffer]' and 'mov [rdi + rcx*4], 0'

  inc ecx
  cmp ecx, 512
  jl .loop_body

.end:
