square:
  mov eax, edi
  imul eax, eax
  ret

sum_square:
  push rbp
  mov rbp, rsp

  push rbx

  push r12

  mov rbx, rdi
  mov r12, rsi

  mov edi, ebx
  call square
  mov rbx, rax

  mov edi, r12d
  call square
  
  add eax, ebx

  pop r12
  pop rbx
  pop rbp

  ret