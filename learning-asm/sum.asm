section .data
  numbers: dd 10, 20, 30, 40
  count: dd 4

section .text
sum_array:
  xor eax, eax
  xor ecx, ecx

  lea rdi, [numbers]
  mov edx, [count]

  cmp 0, [count]
  jge .end

.loop_body:
  add eax, [rdi + rcx*4]

  inc ecx
  cmp ecx, edx
  jl .loop_body

.end:
  ret