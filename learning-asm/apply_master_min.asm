; Created:  Saturday, May 16, 2026 10:51:55 PM
; Finished: Tuesday,  May 19, 2026 06:55:56 PM

section .data
  master_multiplier: dd 2

section .text
  global apply_master_mix

apply_master_mix:
  test rsi, rsi
  jz .end

  mov eax, dword [master_multiplier] ; int multiplier = master_multiplier

  xor rcx, rcx

.outer_loop:
  mov r8, rcx
  shl r8, 4 ; rcx * 16 (data size)
  mov r9, qword [rdi + r8 + 8] ; int *current_samples = tracks[i].samples

  ; movzx to zero-extend the 32-bit count into r9 (64-bit) safely
  movzx r10, [rdi + r8 + 4] ; int count = tracks[i].sample_count

  xor rdx, rdx
  test r10, r10
  jz .inc_outer

.inner_loop:
  ; r11d since samples are 32-bit integers
  mov r11d, dword [r9 + rdx*4] ; r11d = current_samples[j]
  imul r11d, eax ; r11d = current_samples[j] * multiplier
  mov dword [r9 + rdx*4], r11d ; current_samples[j] = r11d

  inc rdx
  cmp rdx, r9
  jl .inner_loop

.inc_outer:
  inc rcx
  cmp rcx, rsi
  jl .outer_loop

.end:
  ret