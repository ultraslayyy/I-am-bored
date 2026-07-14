mov eax, 1
mov ecx, 255
mov edi, 0xDEADBEEF

push eax
push ebp
push edi

pop esi
pop ebx

nop
ret
cli
sti
hlt
int3

mov eax, ebx
mov ebx, eax
mov ecx, edi
mov esi, edx
mov ebp, esp
mov edi, ecx

add eax, ecx
add ecx, eax
add edi, ebx
add ebp, esi

sub eax, ebx
sub esi, edi
sub edx, ecx

xor eax, eax
xor ebx, ecx
xor edi, esi

or eax, ebx
or esi, ecx

and eax, ebp
and edi, edx

cmp eax, ebx
cmp ecx, edx
cmp esi, edi

test eax, eax
test ebx, ecx

add eax, 10
add ebx, 5
add edi, 127

sub eax, 1
sub ecx, 10
sub esi, 64

cmp eax, 100
cmp ebx, 0
cmp edi, 42

and eax, 0xFF
or ecx, 1
xor edx, 0x55

test eax, 1
test ebx, 0x80

mov [eax], ebx
mov [ecx], eax
mov [esi], edi

mov eax, [ebx]
mov edi, [ecx]
mov edx, [esi]

add [eax], ebx
sub [edi], ecx

cmp [ebx], eax
test [ecx], edx

mov eax, [ebx+4]
mov ecx, [esi+8]
mov edi, [eax+16]

mov [ebp+8], eax
mov [esp+12], ebx

add eax, [edi+32]
cmp [esi+127], ebx

mov eax, [ebx+ecx]
mov eax, [ebx+ecx*2]
mov eax, [ebx+ecx*4]
mov eax, [ebx+ecx*8]

mov [eax+esi], ebx
mov [edi+ecx*4], eax

add eax, [esi+edi*2]
cmp [ebx+eax*8], ecx

inc eax
inc edi
dec ebx
dec esi

neg eax
neg ecx

not ebx
not edi

imul eax, ebx
imul ecx, esi

imul eax, ebx, 5
imul edi, ecx, 10

idiv ebx
mul ecx
div esi

lea eax, [ebx+4]
lea edi, [eax+ecx*4+16]

call 0
jmp 0
je 0
jne 0
jg 0
jl 0

loop 0

push ebp
mov ebp, esp
sub esp, 16

mov eax, [ebp+8]
mov ecx, [ebp+12]

add eax, ecx

mov [ebp-4], eax

xor eax, eax

leave
ret

mov eax, [ebp-8]
mov [esi-16], ebx
mov ecx, [edi+120]
mov eax, [ebx+300]