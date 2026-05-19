max:
  mov eax, esi   ; Assume 'b' is the winner
  cmp edi, esi   ; Compare 'a' and 'b'
  cmovg eax, edi ; 'Conditional Move if Greater': if a > b move 'a' into eax
  ret            ; Return