extern printf, scanf, rand, srand, time

section .data
    prompt      db "Guess: ", 0
    fmt_in      db "%d", 0
    err_msg     db "Please enter a valid number", 10, 0
    higher_msg  db "Higher", 10, 0
    lower_msg   db "Lower", 10, 0
    win_msg     db "You got it! The number was %d", 10, 0

section .bss
    num resd 1
    guess resd 1

section .text
    global main

main:
    push rbp
    mov rbp, rsp

    xor edi, edi
    call time
    mov edi, eax
    call srand

    call rand
    xor rdx, rdx
    mov ecx, 101
    div ecx
    mov [num], edx

game_loop:
    mov rdi, prompt
    xor eax, eax
    call printf

    mov rdi, fmt_in
    mov rsi, guess
    xor eax, eax
    call scanf

    cmp eax, 1
    jne invalid_input

    mov eax, [guess]
    mov ebx, [num]

    cmp eax, ebx
    je  win
    jl  higher
    jg  lower

higher:
    mov rdi, higher_msg
    call printf
    jmp game_loop

lower:
    mov rdi, lower_msg
    call printf
    jmp game_loop

invalid_input:
    mov rdi, err_msg
    call printf
    jmp game_loop

win:
    mov rdi, win_msg
    mov esi, [num]
    xor eax, eax
    call printf

    mov eax, 0
    leave
    ret