package opcodes;

typedef enum logic [3:0] {
    OP_NOP,
    OP_MOVI,
    OP_MOVR,
    OP_ADDI,
    OP_ADDR,
    OP_SUBI,
    OP_SUBR,
    OP_JMP,
    OP_CMPI,
    OP_CMPR,
    OP_JZ,
    OP_JNZ,
    OP_JZ,
    OP_STORE,
    OP_LOAD
} opcodes_t;

endpackage