; Created:  Tuesday, May 12, 2026 5:24:53 PM
; Finished: Tuesday, May 12, 2026 5:33:19 PM

bubble_sort:
  cmp esi, 2   ; if n < 2 then array is sorted
  jl .end

  xor r8d, r8d ; int i = 0

.outer_loop:
  xor r9d, r9d   ; int j = 0

  mov edx, esi ; n
  sub edx, r8d ; n - i
  sub edx, 1   ; n - i - 1

.inner_loop:
  mov eax, [rdi + r9*4]     ; eax = arr[j]
  mov ebx, [rdi + r9*4 + 4] ; ebx = arr[j + 1]

  cmp eax, ebx ; if (arr[j] > arr[j + 1])
  jle .no_swap

  mov [rdi + r9*4], ebx     ; arr[j] = ebx
  mov [rdi + r9*4 + 4], eax ; arr[j + 1] = eax

.no_swap:
  inc r9d      ; j++
  cmp r9d, edx ; j < (n - i - 1)
  jl .inner_loop

  ; Inner loop now finished
  inc r8d      ; i++
  mov eax, esi
  sub eax, 1   ; eax = n - 1
  cmp r8d, eax ; i < n - 1
  jl .outer_loop

.end:
  ret