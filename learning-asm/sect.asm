section .data
  name: db "Mai", 0xA, 0 ; 0xA is null terminator, 0 is null terminator
  name_length: dq 4 ; or use dd and movzx rdx, dword [name_length] lol

section .text
print_name:
  mov rax, 1
  mov rdi, 1
  lea rsi, [name]
  mov rdx, [name_length]
  syscall