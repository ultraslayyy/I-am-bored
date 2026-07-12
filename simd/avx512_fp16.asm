global matmul_avx512_fp16

section .text

matmul_avx512_fp16:
  vxorps zmm0, zmm0, zmm0 ; fp32 accumulator

.loop:
  cmp rcx, 32
  jl  .done

  ; Load 32 FP16 values
  vmovdqu16 zmm1, [rdi]
  vmovdqu16 zmm2, [rsi]

  ; Convert FP16 -> FP32
  vcvtph2ps zmm3, zmm1
  vcvtph2ps zmm4, zmm2

  ; Multiply
  vmulps zmm3, zmm3, zmm4

  ; accumulate
  vaddps zmm0, zmm0, zmm3

  add rdi, 64
  add rsi, 64

  sub rcx, 32
  jmp .loop

.done:
  ret