global matmul_scalar

section .text

; rdi = a
; rsi = b
; rdx = c
; ecx = n (extended to r8)
matmul_scalar:
  test ecx, ecx
  jle .done

  ; Convert n to 64-bit for loop indexing and address calculation
  movsxd r8, ecx ; r8 = n

  xor r9, r9                ; r9 = i = 0

.loop_i:
  cmp r9, r8                ; i == n
  jge .done
  xor r10, r10              ; r10 = j = 0

.loop_j:
  cmp r10, r8               ; j == n
  jge .next_i

  xorps xmm0, xmm0          ; xmm0 = sum = 0.0f
  xor r11, r11              ; r11 = k = 0

.loop_k:
  cmp r11, r8               ; k == n
  jge .store_result

  mov rax, r9               ; rax = i
  imul rax, r8              ; rax = i * n
  add rax, r11              ; rax = i * n + k
  movss xmm1, [rdi + rax*4] ; xmm1 = a[i * n + k]

  mov rax, r11              ; rax = k
  imul rax, r8              ; rax = k * n
  add rax, r10              ; rax = k * n + j
  movss xmm2, [rdi + rax*4] ; xmm2 = a[k * n + j]

  mulss xmm1, xmm2          ; xmm1 = a[...] * b[...]
  addss xmm0, xmm1          ; sum += xmm1

  inc r11                   ; ++k
  jmp .loop_k

.store_result:
  mov rax, r9               ; rax = i
  imul rax, r8              ; rax = i * n
  add rax, r10              ; rax = i * n + j
  movss [rdx + rax*4]       ; c[i * n + j] = sum

  inc r10                   ; ++j
  jmp .loop_j

.next_i:
  inc r9                    ; ++i
  jmp .loop_i

.done:
  ret