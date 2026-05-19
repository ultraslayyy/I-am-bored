get_gain:
  mov eax, 2
  ret

apply_gain:
  mov eax, edi
  imul eax, esi
  ret

process_audio:
  push rbp     ; Push 8 bytes, now 16 byte aligned
  mov rbp, rsp ; Set up frame

  push rdi   ; Dave input_sample (8 bytes)
  sub rsp, 8 ; Dummy subtract to stay 16 bit alligned
             ; Since we added 8 from rdi, subtract 8 to
             ; Align back to 16 bytes

  call get_gain

  mov esi, eax
  add rsp, 8 ; Add back dummy space, since we're about to
             ; pop out 8 bytes back
  pop rdi

  call apply_gain

  ret