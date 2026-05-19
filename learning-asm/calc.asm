; Created:      Thursday, May 14, 2026 8:35:15 PM
; Last Updated: Thursday, May 19, 2026 8:36:16 PM
;
; This is an ongoing project of mine that I'm working on on the side
; No source C code as reference or AI involved, just my brain


%define SYS_READ  0
%define SYS_WRITE 1
%define SYS_EXIT  60

%define STDIN  0
%define STDOUT 1

section .bss
  number_1: resb 8
  number_1_end:
  operator: resb 2
  operator_end:
  number_2: resb 8
  number_2_end:
  result: resb 21

section .data
  prompt_1: db "Enter first number: ", 0
  prompt_1_len: equ $ - prompt_1
  prompt_2: db "Enter operator: ", 0
  prompt_2_len: equ $ - prompt_2
  prompt_3: db "Enter second number: ", 0
  prompt_3_len: equ $ - prompt_3
  negative: db "-", 0
  negative_len: equ $ - negative

section .text
  global _start

_start:
  ; Prompt first number
  mov rax, SYS_WRITE
  mov rdi, STDOUT
  mov rsi, prompt_1
  mov rdx, prompt_1_len
  syscall

  mov rsi, number_1
  mov rdx, number_1_end - number_1 ; Length of number_1 buffer
  call .read_input

  mov rdi, number_1
  call .stoi
  mov r8, rax

  ; Prompt operator
  mov rax, SYS_WRITE
  mov rdi, STDOUT
  mov rsi, prompt_2
  mov rdx, prompt_2_len
  syscall

  mov rsi, operator
  mov rdx, operator_end - operator
  call .read_input

  movzx r10, byte [operator]

  ; Prompt second number
  mov rax, SYS_WRITE
  mov rdi, STDOUT
  mov rsi, prompt_3
  mov rdx, prompt_3_len
  syscall

  mov rsi, number_2
  mov rdx, number_2_end - number_2
  call .read_input

  mov rdi, number_2
  call .stoi
  mov r9, rax

  mov rax, r8
  cmp r10, '+'
  je .plus
  cmp r10, '-'
  je .minus
  cmp r10, '*'
  je .multiply
  cmp r10, '/'
  je .divide
  
  jmp .exit

.read_input:
  mov rax, SYS_READ
  mov rdi, STDIN
  syscall
  mov byte [rsi + rax - 1], 0
  ret

.stoi:
  xor rax, rax

.stoi_loop:
  movzx rcx, byte [rdi]

  test rcx, rcx
  jz .stoi_done

  cmp rcx, 10
  je .stoi_done

  cmp rcx, 0
  je .stoi_done

  sub rcx, '0'
  imul rax, 10
  add rax, rcx

  inc rdi
  jmp .stoi_loop

.stoi_done:
  ret

.plus:
  add rax, r9
  jmp .print

.minus:
  sub rax, r9
  jmp .print

.multiply:
  mul r9
  jmp .print

.divide:
  xor rdx, rdx
  div r9
  jmp .print

.invalid_operator:
  mov rax, SYS_EXIT
  mov rdi, -1
  syscall

.print:
  call .itos
  jmp .exit

.exit:
  mov rax, SYS_EXIT
  mov rdi, 0
  syscall

.itos:
  push rbx
  mov rdi, result + 20
  mov byte [rdi], 10

  cmp rax, 0
  jge .itos_setup

.itos_negative:
  push rax

  mov rax, SYS_WRITE
  mov rdi, STDOUT
  mov rsi, negative
  mov rdx, negative_len
  syscall

  pop rax
  neg rax

.itos_setup:
  mov rbx, 10

.itos_loop:
  dec rdi

  cqo
  idiv rbx
  add dl, '0'
  mov [rdi], dl

  test rax, rax
  jnz .itos_loop

  mov rsi, rdi
  mov rdx, result + 21
  sub rdx, rdi

  mov rax, SYS_WRITE
  mov rdi, STDOUT
  syscall

  pop rbx
  ret