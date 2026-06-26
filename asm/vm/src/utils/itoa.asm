global itoa

section .text

itoa: ; rdi = value, rsi = buf
  push rbx
  push rdx
  push r12
  push r13

  mov rax, rdi
  mov r12, rsi

  test rax, rax
  jnz .check_sign
  mov byte [rsi], '0'
  mov byte [rsi+1], '0'
  jmp .done

.check_sign:
  mov r13, 0
  jns .convert
  neg rax
  mov r13, 1

.convert:
  mov rbx, rsi

.loop:
  xor rdx, rdx
  mov rcx, 10
  div rcx

  add dl, '0'
  mov [rbx], dl
  inc rbx

  test rax, rax
  jnz .loop

  cmp r13, 0
  je .reverse_setup
  mov byte [rbx], '-'
  inc rbx

.reverse_setup:
  dec rbx
  mov rsi, r12

.reverse:
  cmp rsi, rbx
  jge .finish

  mov al, [rsi]
  mov dl, [rbx]
  mov [rsi], dl
  mov [rbx], al

  inc rsi
  dec rbx
  jmp .reverse

.finish:
  inc rbx
  mov byte [rbx], 0

.done:
  mov rax, r12

  pop r13
  pop r12
  pop rdx
  pop rbx
  ret