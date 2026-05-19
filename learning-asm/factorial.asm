; Created:  Wednesday, May 13, 2026 09:35:11 PM
; Finished: Wednesday, May 13, 2026 11:46:02 PM

factorial:
  push rbp
  mov rbp, rsp

  cmp edi, 1
  jl .base_case

  push rdi

  sub edi, 1
  call factorial

  pop rdi
  imul eax, rdi

  leave ; shorthand for 'mov rsp, rbp' and 'pop rbp'
  ret
  
.base_case:
  mov eax, 1
  leave
  ret