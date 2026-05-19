# x86 Assembly (NASM) Documentation
These are docs for me by me.<br>
They're so if I forget I can look back and check my notes, like summary notes. I created them using my own knowledge of assembly (no AI in the creation of these docs (besides *some* [register info](./registers.md) cause I am stupid) so if stuff is wrong I am sorry I'm still new).

> [!NOTE]
> These are by no means complete. I am missing loads of instruction I know, but don't know enough about or can't figure out the right wording for.
> But as I come up with ideas and ways to phrase things I will add more instructions to here (like `push`, `pull`, `mov`, `lea`, etc.).

## To Remember
- RAM pointers (or just pointers) are 64-bit. Meaning you need to store and handle with 64-bit registers, not 32-bit ones.

## Instructions
List of instructions (sorted alphabetically for now). I'm just getting info on the page before I forget; won't be alphabetical some day and will be much cleaner and better written & sorted.

### `call`
`call` is the equivalent of calling a function. The syntax is:
```asm
call func_name
```
(where func_name is a label name or an external function name)<br>
`call` does a few things. First, it pushes the current instruction's address to the top of the stack (`push RIP` / `push EIP`). It then jumps to the address of the function and starts executing from that point. When a [`ret`](#ret) occurs, the return address (`RIP`/`EIP`) is popped off the top of the stack (moving the CPU back to where the `call` occured), and continues execution like normal.<br>

### `cmov`
`cmov` (meaning conditional move) is used alongside `cmp` or `test` instructions to `mov` based on conditions. The suffix of `cmovX` tells what the condition is to match. The suffixes are:
- `g` Greater than
- `ge` Greater than or Equal to
- `e` Equal to
- `ne` Not Equal to
- `le` Less than or Equal to
- `l` Less than
- `z` Zero (Zero Flag (ZF) set from something like `test`)
- `nz` Not zero (Zero Flag (ZF) not set)

<small>e.g., `cmovg`, `cmovle`, etc.</small>

`cmov` performs a `mov` if the condition is true. <br>
For example:
```asm
cmp 0, 10
cmovge eax, 1 ; Moves 1 into 'eax' if 0 is greater than or equal to 10
```

### `cmp`
`cmp` (meaning compare) is a very basic instruction. It works as follows:
```asm
cmp left_operand, right_operand
```

From this you can use different jumps (such as `jl`, `jg`, `jge`, `jle`, `je`) to jump to different labels. There are also conditional instructions (like `cmov` which can be used according to the `cmp` output). <br>
(NOTE: this is comparing the left **to** the right operand, never the other way)

### `dec`
`dec` (meaning decrement) does exactly what it looks like. It decrements the operand by 1:
```asm
dec operand
```
For example
```asm
dec ecx ; Pretending ecx is i
```
Is the equivalent of:
```c
i-- // or --i
```

### `inc`
`inc` (meaning increment) does exactly what you think it does. It increments the operand by 1:
```asm
inc operand
```
For example:
```asm
inc ecx ; Pretending ecx is i
```
Is the equivalent of:
```c
i++ // or ++i
```

### `jmp`
The `jmp` instruction 'jumps' to another label. It works as follows:
```asm
jmp label_b ; Skips all of label_a and goes straight to the address of label_b

label_a:
  ; code

label_b:
```

There are also conditional jump instructions. These function the same as the other conditional instructions (like `cmov`), instead being formatted like `jX`, where X is one of:
- `g` Greater than
- `ge` Greater than or Equal to
- `e` Equal to
- `ne` Not Equal to
- `le` Less than or Equal to
- `l` Less than
- `z` Zero (Zero Flag (ZF) set from something like `test`)
- `nz` Not zero (Zero Flag (ZF) not set)

<small>e.g., `jz`, `jle`, etc.</small>

For example:
```asm
cmp 0, 10
jge error ; Jumps to the 'end' label if 0 is greater than or equal to 10
```

### `leave`
The `leave` instruction is great for the end of functions that utilise the stack. It does exactly what:
```asm
mov rsp, rbp
pop rbp
```
does, just in a single byte.

### `movzx`
`movzx` (meaning move zero-extended) acts like a standard `mov`, except for filling the rest of the register/data_object with zeros. A practical example of this is moving a 32-bit value into a 64-bit register. Such as the following:
```asm
section .data
  value: dd 50

section .text
  global _start

_start:
  ; Moving a 32-bit 'value' into a 64-bit register (r8)
  ; Fills excess space with zeros to ensure correct behaviour
  movzx r8, [value]
```

### `popcnt`
`popcnt` (meaning Population count) counts the non-zero bits of a value. It's syntax is like any other:
```asm
popcnt left_operand, right_operand
```
Where the number of non-zeros in the right_operand is stored in the left_operand. For example:
```asm
mov ecx, 10     ; 1010
popcnt eax, ecx ; Stores the number of non-zeros from ecx, in eax (2 in this case)
```
This instruction is not guaranteed on every system (virtual every system though: Intel since 2008, AMD since 2007, and all modern ARM processors), but is required on Windows as of Windows 11 24H2. Just in case you work on old hardware (or are curious to how it works), the following assembly code achieves the same result:
```asm
; Where edi is the value to check
popcnt:
  xor eax, eax
  test edi, edi ; is edi 0?
  jz .end       ; Jump to end because it's 0

loop_body:
  test edi, 1 ; Same as `and edi, 1`, but non-destructive (doesn't modify edi)
  jz skip_inc ; Jump to .shift if Zero Flag (ZF) == 1, meaning it's 0

  inc eax     ; Increase count

skip_inc:
  shr edi, 1    ; Divide edi by 2
  jnz loop_body ; Jum to .loop_body if Zero Flag (ZF) == 0, meaning shr edi, 1 is not 0

  ret ; Return count of 1 bits
```

### ret
The `ret` (meaning Return) instruction is used to return from a `call` and continue execution elsewhere. How it does this is by popping the return address off the top of the call stack (`RIP`/`EIP`) (having been pushed by `call`), telling the processor what code to run next. It then continues execution at the returned address. It does nothng else. It takes no inputs. Just used as:
```asm
ret
```

> In some calling conventions though (e.g., `__stdcall`), it takes an optional integer parameter, such as:
> ```asm
> ret 8
> ret 0x10
> ```
> Which pops the return address, *and* adds a specific number of bytes to the stack pointer, effectively clearing the functions arguments from the stack without needing to add (in turn using one less byte).

### `shl`/`shr`
`shl` and `shr` (shift left and shift right respectively) are used to bit shift (arithmetic shift) values to the left or the right.
> For a refresher on bit shifting:
> ```
>   00010111 (+23)   LEFT-SHIFT
> = 00101110 (+46)
>
>   10010111 (-105)  RIGHT-SHIFT
> = 11001011 (-53)   (notice the wrap-around?)
> ```
> All it does is shift the bits.

The syntax is fairly simple:
```asm
shl left_operand, right_operand
shr left_operand, right_operand
```
Where the left_operand is shifted the associated direction by the right operand. For instance:
```asm
mov al, 5 ; 5 = 00000101
shl al, 2 ; Shift by two:
           ;   00000101 (decimal 5)
           ; = 00010100 (decimal 20)
           ; al = 20
```
As you can see a hint of there, there are unique tricks you can do using shifts, particularly multiplication using `shl`. Take this:
```asm
shl al, 1 ; Multiplies by 2
shl al, 2 ; Multiplies by 4
shl al, 3 ; Multiplies by 8
shl al, 4 ; Multiplies by 16
; ...etc
```
This works because of how bits work, and how each bit is double the previous (moving right to left). So the inverse is true for division, so replacing `shl` with `shr` divides by those same numbers instead of multiplying. The mathematical equations for these are:<br><br>
`shl v, n` is v = v * 2<sup>n</sup><br>
`shr v, n` is v = floor(v / 2<sup>n</sup>)<br><br>
And this is actually useful.<br>
Let's say you have an 'object' (C struct for reference), and each object size is larger than 8 bytes (since assembly only allows access with `[reg + i*8]`, no 16 multiplier only a maximum of 8). You can traverse than using left shifts. Like this example:
```c
struct Obj {
    int one;    // 4 bytes  (+4)
    int two;    // 8 bytes  (+4)
    int *three; // 16 bytes (+8)
};

Obj *arr; // data size is 16 bytes
```
```asm
; Access index 2 of the arr

mov rcx, 2 ; Index to access
shl rcx, 4 ; Shift the index by 4 bits, multiplying by 16
mov rax, [arr + rcx] ; Use the new index which is already multiplied
```
Something to note is that this does modify the existing data. So if you use in a loop, you should use a temporary register to store the shifted value, so to not override the index (counter) and increment properly (else you'll increment the shifted value, then shift the value and it will blow up).

### `test`
The `test` instruction is useful and does quite a few things. It functions logically like a bitwise AND (if both bits are the same it ouputs 1, else 0). But crucially, the result of the AND is discarded. This is useful for multiple things. It calculates:
```asm
test left_operand, right_operand ; left_operand AND right_operand
```
What is does is set a few flags:
- Zero Flag (ZF): Set to `1` if the result is `0`
- Sign Flag (SF): Set to the most significant bit of the result
- Parity Flag (PF): Set based on the number of set bits in the least significant byte.
- Carry (CF) and Overflow (OF): Always overrided and set to `0`

Not a single register is modified by `test` besides the EFLAGS register. This instruction has many good use cases. For example, a more performant check of if a value is 0:
```asm
test eax, eax ; Zero bit set to 1 if eax is 0. Faster than cmp eax, 0
```
It can also be used for checking specific bits (useful in bit masking). For instance:
```asm
test al, 01h ; Checks if the lowest bit of AL is 1
```

### `xor`
The `xor` instruction is what it says on the tin. It's an XOR (Exclusive OR) bitwise operator. The syntax is just like any other instruction:
```asm
xor left_operand, right_operand
```
> As a refresher, an XOR bitwise operation looks like the following:
> ```
>     0101   (5)
> XOR 0011   (3)
>   = 0110   (6)
> ```
> Why? An XOR is an exclusive OR statement. This means that the outputted bit is only `1` if the two input bits are **different**. Meaning `1 0` or `0 1` is `1`, but `1 1` and `0 0` output `0`. For another example:
> ```
>     0010   (2)
> XOR 1010   (10)
>   = 1000   (8)
> ```
This is useful for a multitude of things, including but not limited cryptograhy and zero-ing out values. A note on that second one, it is very common and recommended to set a register to `0` by running an XOR on itself. Like the following:
```asm
xor ecx, ecx ; e.g., int i = 0
```
That is because an XOR on itself always results in `0` (you can see why from above), and on many architectures it uses fewer clock cycles and less memory than loading zero and saving it to a register that way (via something like `mov ecx, 0`).