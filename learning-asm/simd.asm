section .bss
  arr3: resd 8

section .data
  arr1: dd 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0
  arr2: dd 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0

section .text
  global _start

_start:
  mov eax, 1 ; Set CPUID function leaf to 1 (Feature Info)
  cpuid ; Sets eax, ebx, ecx, edx

  ; In leaf 1, AVX support is the 28th bit of ecx
  bt eax, 28  ; Copies bit 28 into the carry flag
  jc .run_avx ; Runs avx if avx support is there

  bt edx, 25  ; Bit 25 of the EDX register for basic SSE
  jc .run_sse

.run_scalar:
  xor rcx, rcx
.scalar_loop:
  mov rax, [arr1 + rcx*4]
  mul [arr2 + rcx*4]
  mov [arr3 + rcx], rax

  add rcx, 1

  cmp rcx, 8
  jl .scalar_loop
  jmp .end

.run_sse:
  xor rcx, rcx
.sse_loop:
  movups xmm0, [arr1 + rcx*4]
  movups xmm1, [arr2 + rcx*4]
  mulps xmm0, xmm1
  movups [arr3 + rcx*4], xmm0

  add rcx, 1

  cmp rcx, 2
  jl .sse_loop
  jmp .end

.run_avx:
  vmovups ymm0, [arr1]
  vmovups ymm1, [arr2]
  vmulps ymm0, ymm1
  vmovups [arr3], ymm0

.end:
  mov rax, 60
  xor rdi, rdi
  syscall