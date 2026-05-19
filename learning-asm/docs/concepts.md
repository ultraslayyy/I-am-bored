## Buffers
### Top Level
Allocating buffers is simple. If allocating at the top level, use the bss section:
```asm
section .bss
  buffer_name: resb size
```

There are multiple sizes of buffers, being:
- `resb` (byte), one byte or 8 bits
- `resw` (word), two bytes or 16 bits
- `resd` (double word/dword), four bytes or 32 bits
- `resq` (qword), eight bytes of 64-bits

### Local
Allocating buffers locally is a little more complicated but not by much. To allocate a local buffer you simply subtract that amount from `rsp` (since a buffer/array is located on the stack). Take this example:
```asm
clear_local_buffer:
  push rbp ; Save the old stack state
  mov rbp, rsp

  ; Now, the space between the new rsp and old rbp is ours to use
  ; Since we allocate 2048 bytes on the stack for us to use
  ; By subtracting the amount to allocate from the rsp
  sub rsp, 2048

  xor ecx, ecx

.loop_body:
  ; Access array relative to the stack pointer
  ; rsp is the top of the stack, and since the buffer we just made is at the top, we access without needing to modify rsp
  mov dword [rsp + rcx*4], 0

  int ecx
  cmp ecx, 512
  jl .loop_body

  leave ; Automatically adds 2048 back to the rsp. Does mov rsp, rbp and pop rbp in one byte
  ret
```