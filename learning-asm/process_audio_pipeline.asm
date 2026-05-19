section .bss
  dynamic_audio_buffer: resd 256

section .data
  global_multiplier: dd 3
  status_success: db "PASS", 0
  status_fail: db "FAIL", 0

section .text
  global process_audio_pipeline

process_audio_pipeline:
  push rbp
  mov rbp, rsp ; Set up frame

  push rbx
  push r12
  push r13
  push r14
  push r15

  xor eax, eax ; int total_processed_samples = 0;
  xor ebx, ebx ; int processing_error = 0;

  xor ecx, ecx ; int i = 0;
  cmp esi, 0   ; compare channel_count
  jle .end     ; i >= channel_count, exit

.outer_loop:
  mov r9, rcx
  shl r9, 4

  movzx rdx, byte [rdi + r9 + 6]
  cmp rdx, 0
  je .inc_outer

  movzx r10d, word [rdi + r9 + 4]   ; int target = channels[i].volume_target
  mov r11, [rdi + r9 + 8]           ; const char* current_tag = channels[i].tag

  cmp r11, 0
  je .processing_error

  movzx edx, byte [r11]
  cmp edx, 0
  je .processing_error

  xor rdx, rdx

.inner_loop:
  mov r12d, eax

  mov r13d, edx
  imul r13d, [global_multiplier]
  add r13d, r10d

  push rax
  
  mov r8d, 2
  mov eax, r13d
  cdq
  idiv r8d

  lea r15, [dynamic_audio_buffer + r12*4]
  mov [r15], eax

  pop rax
  inc eax

.inc_inner:
  inc edx
  cmp edx, 10
  jl .inner_loop

.inc_outer:
  inc ecx
  cmp ecx, esi
  jl .outer_loop
  jmp .end

.processing_error:
  mov ebx, 1

.end:
  cmp ebx, 0
  cmovne eax, -1

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbx

  leave
  ret