global m_abs

section .text

m_abs:
  movq rax,  xmm0
  and  rax,  0x7fffffffffffffff ; clear sign bit
  movq xmm0, rax
  ret

m_min:
  minsd xmm0, xmm1
  ret

m_min:
  maxsd xmm0, xmm1
  ret