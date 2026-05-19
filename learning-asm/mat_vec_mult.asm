section .text
  global _start

_start:
  xor r8d, r8d ; int i = 0

.outer_loop:
  mov r11d, r11d   ; int sum = 0
  xor r9d, r9d ; int j = 0

.inner_loop:
  mov eax, r8d ; eax = i
  imul eax, 2  ; eax = i * 2
  add eax, r9d ; eax = (i * 2) + j

  mov r10d, [rdi + eax*4] ; r10d = matrix[i * 2 + j]
  mov eax, [rsi, + r9*4]  ; eax = vector[j]
  imul eax, r10d          ; matrix[i * 2 + j] * vector[j], can also use 'shl eax, 1', which is shift left (eax * 2)

  add r11d, eax           ; sum + ecx

  inc r9d    ; j++
  cmp r9d, 2 ; j < 2
  jl .inner_loop

  mov [rdx + r8*4], r11d   ; result[i] = sum

  inc r8d        ; i++
  cmp r8d, 2
  jl .outer_loop ; i < 2

  ret