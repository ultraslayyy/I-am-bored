bits 32

global memset
global memcpy
global memmove
global memcmp

section .text

memset:
  push ebp
  mov ebp, esp
  push edi

  mov edi, [ebp+8]         ; dst
  movzx eax, byte [ebp+12] ; val (only need AL)
  mov ecx, [ebp+16]        ; size

  test ecx, ecx
  jz .memset_done

  rep stosb

.memset_done:
  mov eax, [ebp+8]

  pop edi
  pop ebp
  ret

memcpy:
  push ebp
  mov ebp, esp
  push edi
  push esi

  mov edi, [ebp+8]  ; dst
  mov esi, [ebp+12] ; src
  mov ecx, [ebp+16] ; size

  test ecx, ecx
  jz .memcpy_done

  cld ; Ensure Direction Flag (DF) is 0 (incrementing)
  rep movsb

.memcpy_done:
  mov eax, [ebp+8]
  pop esi
  pop edi
  pop ebp
  ret

memmove:
  push ebp
  mov ebp, esp
  push edi
  push esi

  mov edi, [ebp+8]  ; dst
  mov esi, [ebp+12] ; src
  mov ecx, [ebp+16] ; size

  test ecx, ecx
  jz .memmove_done

  cmp edi, esi
  jae .memmove_copy_backwards ; if dst >= src we must copy backwards

  ; Forward copy (dst < src)
  cld
  rep movsb
  jmp .memmove_done

.memmove_copy_backwards:
  je .memmove_done ; if dst == src do nothing

  ; To copy backwards, point esi and edi to the
  ; last byte of the buffers
  lea edi, [edi + ecx - 1]
  lea esi, [esi + ecx - 1]

  std ; Set direction flag to 1 (decrementing)
  rep movsb
  cld ; Reset DF

.memmove_done:
  mov eax, [ebp+8] ; return dst
  pop esi
  pop edi
  pop ebp
  ret

memcmp:
  push ebp
  mov ebp, esp
  push edi
  push esi

  mov esi, [ebp+8]  ; buf1
  mov edi, [ebp+12] ; buf2
  mov ecx, [ebp+16] ; size

  xor eax, eax ; default return value = 0
  test ecx, ecx
  jz .memcmp_done

  cld
  repe cmpsb ; compare bytes until mismatch or ecx == 0
  je .memcmp_done

  ; If they are not equal, cmpsb still incremented
  ; /decremented the pointers by 1 past the mismatch. We read
  ; the mismatched bytes
  mov al, [esi - 1]
  mov dl, [edi - 1]
  sub eax, edx ; eax = buf1[i] - buf2[i]

.memcmp_done:
  pop esi
  pop edi
  pop ebp
  ret