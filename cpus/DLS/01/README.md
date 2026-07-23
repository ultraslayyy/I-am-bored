# DLS-01

## Registers
| Register | Encoding |
| -------- | -------- |
| R0       | `00`     |
| R1       | `01`     |
| R2       | `10`     |
| R3       | `11`     |

### Special Registers
Special registers are not exposed externally, but used internally:
- IP — Instruction Pointer (8-bit)
- FLAGS — Status Register

### Flags
Also not exposed externally, but used internally:
| Flag | Description                                                  |
| ---- | ------------------------------------------------------------ |
| ZF   | Zero Flag. Set when an operation produces a result of zero.  |
| CF   | Carry Flag. Set when an unsigned carry or borrow occurs.     |
| SF   | Sign Flag. Mirrors bit 7 of the result.                      |
| OF   | Overflow Flag. Set when a signed arithmetic overflow occurs. |

Flags are updated by:
- `ADDI`
- `ADDR`
- `SUBI`
- `SUBR`
- `CMPI`
- `CMPR`

All other instructions leave the flags unchanged.

## Memory

Instruction Memory (ROM)
- 256 instructions
- 16-bit instruction width

Data Memory (RAM)
- 256 bytes
- 8-bit data width

Address space:
`0x00 – 0xFF`

## Instruction Encoding
Unless stated otherwise:
- A = Destination register
- B = 8-bit immediate value
- C = Source register
- D = Jump mode
  - `0` = Relative jump
  - `1` = Absolute jump

## Instruction Set
### NOP
`0000 0000 0000 0000`

Mnemonic:
`nop`

Performs no operation.

Flags affected:
None.

---

### MOVI
`0001 00AA BBBB BBBB`

Mnemonic:
`mov Rd, imm8`

Operation:
`Rd = imm8`

Flags affected:
None.

---

### MOVR
`0010 CCAA 0000 0000`

Mnemonic:
`mov Rd, Rs`

Operation:
`Rd = Rs`

Flags affected:
None.

---

### ADDI
`0011 00AA BBBB BBBB`

Mnemonic:
`add Rd, imm8`

Operation:
`Rd = Rd + imm8`

Flags affected:
ZF, CF, SF, OF

---

### ADDR
`0100 CCAA 0000 0000`

Mnemonic:
`add Rd, Rs`

Operation:
`Rd = Rd + Rs`

Flags affected:
ZF, CF, SF, OF

---

### SUBI
`0101 00AA BBBB BBBB`

Mnemonic:
`sub Rd, imm8`

Operation:
`Rd = Rd - imm8`

Flags affected:
ZF, CF, SF, OF

---

### SUBR
`0110 CCAA 0000 0000`

Mnemonic:
`sub Rd, Rs`

Operation:
`Rd = Rd - Rs`

Flags affected:
ZF, CF, SF, OF

---

### JMP
`0111 000D BBBB BBBB`

Mnemonic:
`jmp address`

Operation:
- If `D = 1`, `IP = address`
- If `D = 0`, `IP = IP + address`\*

<small>* Relative to position of current instruction</small>

Flags affected:
None.

---

### CMPI
`1000 00AA BBBB BBBB`

Mnemonic:
`cmp Rd, imm8`

Operation:
`temp = Rd - imm8`
The result is discarded.

Flags affected:
ZF, CF, SF, OF

---

### CMPR
`1001 CCAA 0000 0000`

Mnemonic:
`cmp Rd, Rs`

Operation:
`temp = Rd - Rs`
The result is discarded.

Flags affected:
ZF, CF, SF, OF

---

### JZ
`1010 000D BBBB BBBB`

Mnemonic:
`jz address`

Operation:
Jump if `ZF = 1`.

Flags affected:
None.

---

### JNZ
`1011 000D BBBB BBBB`

Mnemonic:
`jnz address`

Operation:
Jump if `ZF = 0`.

Flags affected:
None.

---

### JC
`1100 000D BBBB BBBB`

Mnemonic:
`jc address`

Operation:
Jump if `CF = 1`.

Flags affected:
None.

---

### STORE
`1101 00AA BBBB BBBB`

Mnemonic:
`store address, Rs`

Operation:
`RAM[address] = Rs`

Flags affected:
None.

---

### LOAD
`1110 00AA BBBB BBBB`

Mnemonic:
`load Rd, address`

Operation:
`Rd = RAM[address]`

Flags affected:
None.

## Execution Model
Each instruction is 16 bits wide.

Instructions are fetched from ROM using the Instruction Pointer (IP). After execution, the Instruction Pointer normally increments by one instruction. Jump instructions may replace or modify the Instruction Pointer depending on the jump mode.

## Notes
- Register operations are performed on unsigned 8-bit values.
- Arithmetic wraps on overflow.
- Conditional branches operate using the FLAGS register.
- The processor has no stack, interrupts, or memory protection.
- All instructions execute in a single step.