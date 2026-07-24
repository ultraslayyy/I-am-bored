## Optimisations

### Loop Unrolling
Instead of:
```asm
.loop:
  add eax, [rsi]
  add rsi, 4
  dec ecx
  jnz .loop
```

Unroll it by 4:
```asm
.loop:
  add eax, [rsi]
  add eax, [rsi+4]
  add eax, [rsi+8]
  add eax, [rsi+12]

  add rsi, 16
  sub ecx, 4
  jnz .loop
```

That is, process 4 per loop iteration instead of one. This decreases the number of jumps required for the loop by a factor of 4.

Advantages:
- Fewer branches
- Less loop overhead
- Better instruction scheduling

Downside:
- Larger code size

Also applies to C:
```c
for (...) {
    sum += a[i];
    sum += a[i+1];
    sum += a[i+2];
    sum += a[i+3];
}
```

### Strength reduction
Replace expensive ops with cheaper ones.

Instead of:
```asm
imul eax, 8
```
Use:
```asm
shl eax, 3
```

Instead of:
```asm
idiv ebx
```
Sometimes possible:
```asm
shr eax, 1
```
Or multiply by a reciprocal:
```
reciprocal = 1.0 / divisor
result = value * reciprocal
```
Example:
```asm
mov eax, 1234 ; value to divide
mov edx, 0    ; clear high part for mul

; Precomputed magic number for division by 10 (unsigned)
; magic = floor((2^32 + divisor - 1) / divisor)
mov ecx, 0xCCCCCCCD ; 2^35/10

mul eax      ; EDX:EAX = EAX * ECX (unsigned 64-bit result)
mov eax, edx ; quotient is in high 32 bits

; eax now contains floor(1234/10) = 123
```

### Keep variables in registers
Memory is slow.

Bad
```asm
mov eax, [counter]
add eax, 1
mov [counter], eax
```

Better
```asm
mov eax, [counter]

.loop:
  inc eax
  ...
mov [counter], eax
```

Registers are much faster than RAM.

### Reduce memory accesses
Memory loads are expensive.

Bad
```asm
mov eax, [value]
add eax, [value]
```

Better
```asm
mov eax, [value]
add eax, eax
```

Try to reduce the amount of memory access as much as possible.

### Use LEA for Arithmetic
`lea` can perform addition and scaled indexing without affecting flags.

Instead of
```asm
mov eax, ebx
shl eax, 2
add eax, ecx
```

Use
```asm
lea eax, [ecx + ebx*4]
```

### Eliminate common subexpressions
Instead of computing something twice
```asm
imul eax, 5
...
imul eax, 5
```

Compute once
```asm
imul eax, 5
mov ebx, eax
```

### Move invariant code outside loops
Bad
```asm
.loop:
  mov eax, 123
  add ebx, eax
```

Better
```asm
mov eax, 123

.loop:
  add ebx, eax
```

Called Loop-Invariant Code Motion (LICM).

### Reduce branches
Branches hurt when mispredicted.

Instead of
```asm
cmp eax, ebx
jl less
```

Sometimes use
```asm
cmovl eax, ebx
```
or
```asm
setl al
```

### Predictable branches
If a branch is almost always taken, structure code accordingly.

Instead of
```asm
if (rare)
```

Prefer
```asm
if (common)
```

Modern CPUs predict repeated patterns well.

### Align loops
Align hot loops.

```asm
align 16
.loop:
```

Benefits instruction fetch.
Mostly used for frequently executed loops.

### Use `INC`/`DEC` carefully
On modern x86, `add reg, 1` is often equally good or better because `inc`/`dec` don't update the carry flag, with can complicate dependency handling.

### Avoid partial register writes
Bad
```asm
mov al, 5
```
then
```asm
mov ebx, eax
```

Better
```asm
mov eax, 5
```

### Fuse address calculations
Instead of
```asm
add rsi, 8
mov eax, [rsi]
```

Sometimes
```asm
mov eax, [rsi+8]
```

### Minimise dependencies
Bad
```asm
add eax, 1
add eax, 2
add eax, 3
add eax, 4
```
Every instruction waits for the previous.

Better
```asm
add eax, 1
add ebx, 2
add ecx, 3
add edx, 4
```
Independent instructions let the CPU execute them in parallel (instruction-level parallelism).

### Reduce function calls
Inlining avoids
```asm
call foo
```
and
```asm
ret
```

Downside:
- Larger binary
- Worse instruction cache usage

### Use appropriate instructions
Examples:

Instead of
```asm
mov eax, 0
```

Use
```asm
xor eax, eax
```

Benefits:
- Shorter encoding
- Breaks false dependencies on modern CPUs
- Very common idiom

### Data locality
Process memory sequentially.

Good
```
1 2 3 4 5 6
```
Bad
```
1 10000 7 9000
```

Caches love sequential access

### Cache Blocking (Loop Tiling)
Instead of processing an entire matrix row/column, operate on smaller blocks that fit in cache.

This is especially effective for matrix multiplication and image processing.

For example:

Suppose you have a huge matrix:
```
10000 x 10000
```

The classic algorithm:
```c
for (i = 0; i < N; ++i)
    for (j = 0; j < N; ++j)
        for (k = 0; k < N; ++k)
            C[i][j] += A[i][k] * B[k][j];
```
Looks inoocent.

Imagine
```
A
AAAAAAAAAAAA

B
BBBBBBBBBBBB

C
CCCCCCCCCCCC
```
For each element of `C`, you're walking down an entire column of `B`.

Memory access looks something like:
```c
B[0][j]
B[1][j]
B[2][j]
B[3][j]
...
```

Those elements are far apart in memory (assuming row-major order like C uses), so the CPU constantly loads new cache lines and evicts old ones.

CACHE BLOCKING.
Instead, divide everything into smaller blocks.

Instead of
```
####################
####################
####################
####################
```

Split into
```
++++----++++----
++++----++++----
----++++----++++
----++++----++++
```
Now multiply one square at a time.

Example:
```c
const int BLOCK = 32;

for (int ii = 0; ii < N; ii += BLOCK)
    for (int jj = 0; jj < N; jj += BLOCK)
        for (int kk = 0; kk < N; kk += BLOCK)

            for (int i = ii; i < ii + BLOCK; i++)
                for (int j = jj; j < jj + BLOCK; j++)
                    for (int k = kk; k < kk + BLOCK; k++)
                        C[i][j] += A[i][k] * B[k][j];
```
Now instead of needing the whole matrix, the CPU only needs:
```
+----+
|AAAA|
|AAAA|
|AAAA|
|AAAA|
+----+

+----+
|BBBB|
|BBBB|
|BBBB|
|BBBB|
+----+

+----+
|CCCC|
|CCCC|
|CCCC|
|CCCC|
+----+
```
These three small blocks fit in cache.

In assembly, that would change the loop from:
```asm
i:
  j:
    k:
```
to:
```asm
ii:
  jj:
    kk:

      i:
        j:
          k:
```

### Profile before some optimisations
The most important 'optimisation':
- Measure performance
- Find the bottleneck
- Optimise the bottleneck
- Measure again

Many manual optimisations make code harder to read whilst providing negligible benefit if they target code that isn't performance critical.

However some optimisations are recommended and should be done regardless (e.g., [use appropriate instructions](#use-appropriate-instructions), [fuse address calculations](#fuse-address-calculations), [avoid partial writes](#avoid-partial-register-writes), [move invariant code](#move-invariant-code-outside-loops), [eliminate subexpressions](#eliminate-common-subexpressions), [reduce memory access](#reduce-memory-accesses), [lea arithmetic](#use-lea-for-arithmetic), [strength reduction (minus reciprocal multiplication)](#strength-reduction) and [keeping variables in registers](#keep-variables-in-registers)). These are essentially basic aspects of 'write good assembly'.

Some other optimisations that are essentially just for 'performance engineering' are:
- [Loop unrolling](#loop-unrolling) (trade-off: code size)
- [Loop alignment](#align-loops) (trade-off: code size, often arch-dependent)
- [Cache blocking](#cache-blocking-loop-tiling) (trade-off: much more complex code)
- [Branchless programming](#reduce-branches) (trade-off: sometimes slower than a well-predicted branch)
- SIMD (trade-off: complexity and portability)
- [Function inlining](#reduce-function-calls) (trade-off: code size)