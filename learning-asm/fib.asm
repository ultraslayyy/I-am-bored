fibonacci:
  cmp edi, 0 ; if (n < 0)
  jl .zero

  cmp edi, 2 ; if (n < 2)
  jl .less_than

  mov eax, 0 ; int a = 0;
  mov edx, 1 ; int b = 1;
  mov ecx, 2 ; for (int 1 = 2)

.loop_body:
  xchg eax, edx ; swap a and b, though mov r8d, eax and using r8d as the temp is faster
  add edx, eax  ; b += a; (since we swapped them, adding a back ends up with b = a + b, no temporary)

  jo .panic     ; Panic if overflow

  inc ecx          ; ++i / i++
  cmp ecx, edi     ; compare i and n
  jle .loop_body   ; i <= n

  mov eax, edx  ; Set EAX to the return value of b
  ret

.zero:
  mov eax, 0
  ret ; Return 0

.less_than:
  mov eax, edi
  ret ; Return n

.panic:
  ud2 ; 'Undefined instruction'