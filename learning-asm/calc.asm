; Created:      Thursday, May 14, 2026 08:35:15 PM
; Last Updated: Saturday, May 23, 2026 03:27:44 PM
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
  operator: resd 1
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
  ; Save ABI
  push rbx
  push r12

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

  mov r10d, dword [operator]

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
  cmp r10, '//'
  je .integer_divide
  cmp r10, '^^'
  je .power
  cmp r10, '%'
  je .modulus
  cmp r10, 'nCr'
  je .ncr
  cmp r10, 'nPr'
  je .npr

  ; Bitwise
  cmp r10, '>>'
  je .shift_right
  cmp r10, '<<'
  je .shift_left
  cmp r10, '&'
  je .bit_and
  cmp r10, '|'
  je .bit_or
  cmp r10, '^'
  je .bit_xor
  ; TODO: Decimal division, logarithms (base, op (log), argument),
  ; TODO: nth root of num (nth, op (root), num)

  jmp .invalid_operator

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
  imul r9
  jmp .print

.integer_divide:
  test r9, r9
  jz .invalid_operator

  xor rdx, rdx
  cqo
  idiv r9
  jmp .print

.power:
  mov r8, rax
  mov rax, 1
  sub r9, rax
  jb .power_return
  mov rax, r8
  jz .power_return
.power_more:
  imul rax, r8
  dec r9
  jnz .power_more
.power_return:
  jmp .print

.modulus:
  xor rdx, rdx
  cqo
  idiv r9
  mov rax, rdx
  jmp .print

.ncr: ; nCr = n! / (r!(n-r)!)
  mov rdi, rax
  call factorial
  mov r10, rax

  sub rdi, r9
  call factorial
  mov r11, rax

  mov rdi, r9
  call factorial

  mov r12, rax

  imul r11, r12

  mov rax, r10
  mov rcx, r11
  cqo
  idiv rcx

  jmp .print

.npr: ; nPr = n! / (n - r)!
  mov rdi, rax
  call factorial
  mov r10, rax

  sub rdi, r9
  call factorial

  mov rcx, rax
  mov rax, r10
  cqo
  idiv rcx

  jmp .print

.shift_right:
  mov rcx, r9
  shr rax, cl
  jmp .print

.shift_left:
  mov rcx, r9
  shl rax, cl
  jmp .print

.bit_and:
  and rax, r9
  jmp .print

.bit_or:
  or rax, r9
  jmp .print

.bit_xor:
  xor rax, r9
  jmp .print

.invalid_operator:
  pop r12
  pop rbx

  mov rax, SYS_EXIT
  mov rdi, -1
  syscall

.print:
  call .itos
  jmp .exit

.exit:
  pop r12
  pop rbx

  mov rax, SYS_EXIT
  xor rdi, rdi
  syscall

.itos:
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
  ret

factorial:
  mov rax, 1

  cmp rdi, 0
  jl .factorial_end

  cmp rdi, 1
  jle .factorial_end

  mov rcx, rdi

.factorial_loop:
  imul rax, rcx
  dec rcx
  jnz .factorial_loop

.factorial_end:
  ret