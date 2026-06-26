global strlen

section .text

strlen:
  xor rax, rax
  cmp byte [rdi], 0
  jz .end

.loop:
  inc rax
  cmp byte [rdi + rax], 0
  jnz .loop

.end:
  ret