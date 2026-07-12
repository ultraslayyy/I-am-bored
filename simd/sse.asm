global matmul_sse

section .text

; cdecl
; [esp+4]  = a
; [esp+8]  = b
; [esp+12] = c
; [esp+16] = n
matmul_sse:
  push ebp
  mov  ebp, esp

  push ebx
  push esi
  push edi

  mov  eax, [ebp+8]  ; eax = a
  mov  ebx, [ebp+12] ; ebx = b
  mov  edi, [ebp+16] ; edi = c
  mov  ecx, [ebp+20] ; ecx = n

  xor  esi, esi      ; i = 0

.loop_i:
  cmp  esi, ecx
  jge  .done

  xor  edx, edx      ; j = 0

.loop_j:
  cmp  edx, ecx
  jge  .next_i

  pxor xmm0, xmm0    ; accumulator
  xor  ebp,  ebp     ; k = 0

.loop_k:
  mov  eax, ecx
  sub  eax, ebp
  cmp  eax, 4
  jl  .tail

  ; Load A[i*n+k .. i*n+k+3]
  mov  eax, esi
  imul eax, ecx
  add  eax, ebp
  shl  eax, 2

  movups xmm1, [a + eax]

  ; Load B column:
  ;
  ; xmm2 = {
  ;   b[k*n+j],
  ;   b[(k+1)*n+j],
  ;   b[(k+2)*n+j],
  ;   b[(k+3)*n+j]
  ; }

  ; b[k*n+j]
  mov  eax, ebp
  imul eax, ecx
  add  eax, edx
  shl  eax, 2

  movss xmm2, [b + eax]


  ; b[(k+1)*n+j]
  add  eax, ecx
  shl  ecx, 2        ; n*4
  movss xmm3, [b + eax + ecx]

  unpcklps xmm2, xmm3


  ; b[(k+2)*n+j]
  add  eax, ecx
  movss xmm4, [b + eax]


  ; b[(k+3)*n+j]
  add  eax, ecx
  movss xmm5, [b + eax]


  unpcklps xmm4, xmm5
  movlhps xmm2, xmm4

  ; multiply and accumulate
  mulps xmm1, xmm2
  addps xmm0, xmm1

  shr  ecx, 2

  add  ebp, 4
  jmp  .loop_k

.tail:
  cmp ebp, ecx
  jge .reduce

  mov eax, esi
  imul eax, ecx
  add eax, ebp
  movss xmm1, [a + eax*4]

  mov eax, ebp
  imul eax, ecx
  add eax, edx
  movss xmm2, [b + eax*4]

  mulss xmm1, xmm2
  addss xmm0, xmm1

  inc ebp
  jmp .tail

.reduce:
  movaps xmm1, xmm0
  movhlps xmm1, xmm0
  addps xmm0, xmm1

  movaps xmm1, xmm0
  shufps xmm1, xmm1, 1
  addss xmm0, xmm1

  ; c[i*n+j] = xmm0[0]
  mov eax, esi
  imul eax, ecx
  add eax, edx
  movss [edi + eax*4], xmm0

  inc edx
  jmp .loop_j

.next_i:
  inc esi
  jmp .loop_i

.done:
  pop edi
  pop esi
  pop ebx
  
  mov esp, ebp
  pop ebp
  ret