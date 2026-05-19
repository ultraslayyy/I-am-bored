to_upper:
  xor rcx, rcx

  mov al, [rdi + rcx]
  test al, al
  jz .end

.loop_body:
  cmp al, 97
  jl .inc_loop

  cmp al, 122
  jg .inc_loop

  sub al, 32
  mov [rdi + rcx], al

.inc_loop:
  inc rcx

  mov al, [rdi + rcx]
  test al, al
  jnz .loop_body

.end:
