## Registers
There are multiple types of registers: GPRs (General Purpose Registers); Pointer and Indicator Registers; Segment Registers; SIMD and Floating-Point Registers; System, Control, and Debug Registers; and Model Specific Registers (MSRs).

### GPRs
| 64-bit Name | 32-bit Name | 16-bit Name | 8-bit (Low) | 8-bit (High)    | Purpose / Historic Purpose        |
| ----------- | ----------- | ----------- | ----------- | --------------- | --------------------------------- |
| RAX         | EAX         | AX          | AL          | AH              | Accumulator (Return values)       |
| RBX         | EBX         | BX          | BL          | BH              | Base Register                     |
| RCX         | ECX         | CX          | CL          | CH              | Counter (loops, shift counts)     |
| RDX         | EDX         | DX          | DL          | DH              | Data Register (I/O, divisons)     |
| RSI         | ESI         | SI          | SIL         | *N/A*           | Source Index (String ops)         |
| RDI         | EDI         | DI          | DIL         | *N/A*           | Destination Index (String ops)    |
| RBP         | EBP         | BP          | BPL         | *N/A*           | Base Pointer (stack frames)       |
| RSP         | ESP         | SP          | SPL         | *N/A*           | Stack Pointer (current top)       |
| R8          | R8D         | R8W         | R8B         | *N/A*           | General Purpose (Added in 64-bit) |
| R9          | R9D         | R9W         | R9B         | *N/A*           | General Purpose (Added in 64-bit) |
| R10         | R10D        | R10W        | R10B        | *N/A*           | General Purpose (Added in 64-bit) |
| R11         | R11D        | R11W        | R11B        | *N/A*           | General Purpose (Added in 64-bit) |
| R12         | R12D        | R12W        | R12B        | *N/A*           | General Purpose (Added in 64-bit) |
| R13         | R13D        | R13W        | R13B        | *N/A*           | General Purpose (Added in 64-bit) |
| R14         | R14D        | R14W        | R14B        | *N/A*           | General Purpose (Added in 64-bit) |
| R15         | R15D        | R15W        | R15B        | *N/A*           | General Purpose (Added in 64-bit) |

Note: On APX (Intel Advanced Performance Extensions), this list is expanded to 32 GPRs instead of only 16; introducing registers R16 through R31.

### Pointer and Indicator Registers
- RIP / EIP / IP (Instruction Pointer): Holds the memory address of the next instruction to execute. In 64-bit mode, it is 64-bits wide (RIP).
- RFLAGS / EFLAGS / FLAGS (Status/Flags Register): A collection of single-bit flags that reflect the outcomes of mathematical operation or control CPU behaviour.
  - Status Flags: `CF` (Carry), `ZF` (Zero), `SF` (Sign), `OF` (Overflow), `PF` (Parity), `AF` (Auxiliary Carry).
  - Control/System Flags: `DF` (Direction), `IF` (Interrupt Enable), `TF` (Trap/Debug), `IOPL` (I/O Privilege Level), `AC` (Alignment Check), `ID` (CPU ID Identification flag).

### Segment Registers
- CS: Code Segment
- DS: Data Segment
- SS: Stack Segment
- ES: Extra Segment
- FS: General Purpose / Thread Local Storage (TLS) pointer in modern OSs
- GS: General Purpose / Core Operating System data pointer

### SIMD and Floating-Point Registers (Vector Math)
- x87 FPU / MMX Registers
  - ST(0) through ST(7)
  - MMX0 through MMX7
- SSE / AVX / AVX-512 Registers
  - XMM0 to XMM15 (Extended to XMM31 in 64-bit): 128-bit registers used for streaming SIMD extensions (SSE)
  - YMM0 to YMM15 (Extended to YMM31 in 64-bit): 256-bit registers used for AVX (Advanced Vector Extensions). The lower 128-bits overlap with `XMM`.
  - ZMM0 to ZMM32: 512-bit registers used for AVX-512. The lower 256-bits overlap with `YMM`
  - K0 to K7 (Opmask Registers): Eight 64-bit registers used in AVX-512 for conditionally masking vector lanes.
  - MXCSR: A 32-bit control and status register for SSE/AVX math operations (handles rounding control and precision exceptions).

### System, Control, and Debug Registers (Privileged)
Registers used by the kernel-level code, and cannot be modified by user-level programs.
- Control Registers
  - Used to define the current operating mode of the processor, control paging, and manage security parameters.
  - CR0: Controls operating mode (Protected mode, Paging enable, WP).
  - CR1: Reserved
  - CR2: Contains the Page Fault Linear Address (where a memory page fault just occurred).
  - CR3: Page Directory Base Register (points to the root of the page tables).
  - CR4: Controls the architectural extensions (e.g., Virtual Mode Extensions, SMAP, SMEP).
  - CR8: Task Priority Register (TPR), used to manage interrupt priorities in 64-bit mode.
- Debug Registers:
  - Used for setting hardware breakpoints during software analysis.
  - DR0, DR1, DR2, DR3: Linear addresses for up to 4 hardware breakpoints
  - DR4, DR5: Reserved (aliases to DR6/DR7)
  - DR6: Debug Status Register (indicates which breakpoint triggered)
  - DR7: Debug Control Register (activates and configures breakpoint conditions)
- Descriptor Table Registers
  - These hold the base addresses and size limits for memory structural tables used by the processor.
  - GDTR: Global Descriptor Table Register
  - IDTR: Interrupt Descriptor Table Register
  - LDTR: Local Descriptor Table Register (contains a selector pointing to the GDT)
  - TR: Task Register (holds the selector for the current Task State Segment)

### Model-Specific Registers (MSRs)
> Processors contain hundreds of MSRs used for toggling system features, debugging, performance monitoring, and managing execution states (like the IA32_EFER which activates 64-bit execution, or IA32_KERNEL_GS_BASE for quick OS kernel switches). They aren't referenced by fixed assembly names, but are instead indexed numerically and read/written using special commands (RDMSR and WRMSR).