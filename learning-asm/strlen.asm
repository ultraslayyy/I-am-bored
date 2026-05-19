strlen:
  xor eax, eax

  cmp byte [rdi + rax], 0
  jz .end

.while_body:
  inc eax

  cmp byte [rdi + rax], 0
  jnz .while_body

.end:
  ret