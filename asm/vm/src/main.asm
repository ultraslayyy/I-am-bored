%include "syscalls.inc"

global _start

extern strlen
extern itoa

section .data
  program:
    db 1, 5 ; PUSH 5
    db 1, 3 ; PUSH 3
    db 2    ; ADD
    db 3    ; PRINT
    db 0    ; HALT
  program_end:

section .bss
  stack resq 64
  print_buf resb 64

section .text

_start:
  mov rsi, program ; instruction pointer (ip)
  mov rbx, stack   ; stack pointer (sp)

dispatch_loop:
  movzx rax, byte [rsi] ; load opcode
  inc rsi

  cmp rax, 0
  je op_halt

  cmp rax, 1
  je op_push

  cmp rax, 2
  je op_add

  cmp rax, 3
  je op_print

  cmp rax, 4
  je op_mul

  cmp rax, 5
  je op_div

  cmp rax, 6
  je op_sub

  cmp rax, 10
  je op_jmp

  jmp dispatch_loop

; HLT/HALT
op_halt:
  mov rax, 60
  xor rdi, rdi
  syscall

; PUSH
op_push:
  movzx rax, byte [rsi] ; value
  inc rsi

  mov [rbx], rax ; *sp = value
  add rbx, 8     ; sp++

  jmp dispatch_loop

; ADD
; pop a, pop b, push a+b
op_add:
  sub rbx, 8
  mov rax, [rbx]

  sub rbx, 8
  mov rcx, [rbx]

  add rax, rcx

  mov [rbx], rax
  add rbx, 8

  jmp dispatch_loop

; PRINT
op_print:
  sub rbx, 8
  mov rax, [rbx] ; pop value

  mov rdi, rax
  mov rdx, rsi ; save IP

  mov rsi, print_buf
  call itoa

  mov rdi, rax
  call strlen

  mov rdx, rax ; length

  mov rax, SYS_WRITE
  mov rdi, STDOUT
  mov rsi, print_buf
  syscall
  
  mov rsi, rdx ; restore IP

  jmp dispatch_loop

; MUL (signed multiplication)
op_mul:
  sub rbx, 8
  mov rax, [rbx]

  sub rbx, 8
  mov rcx, [rbx]

  imul rax, rcx

  mov [rbx], rax
  add rbx, 8

  jmp dispatch_loop

; DIV (signed division)
op_div:
  sub rbx, 8
  mov rax, [rbx]
  cqo

  sub rbx, 8
  mov rcx, [rbx]

  idiv rcx ; rax = value, rdx = remainder

  mov [rbx], rax
  add rbx, 8

  jmp dispatch_loop

; SUB
op_sub:
  sub rbx, 8
  mov rax, [rbx]

  sub rbx, 8
  mov rcx, [rbx]

  sub rax, rcx

  mov [rbx], rax
  add rbx, 8

  jmp dispatch_loop

; JMP
op_jmp:
  movzx rax, byte [rsi]
  mov rsi, program
  add rsi, rax
  jmp dispatch_loop
