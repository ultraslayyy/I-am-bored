package alu_pkg;

typedef enum logic [2:0] {
    ALU_ADD = 3'b000,
    ALU_SUB = 3'b001,
    ALU_AND = 3'b010,
    ALU_OR  = 3'b011,
    ALU_XOR = 3'b100,
    ALU_NOT = 3'b101
} alu_op_t;

endpackage