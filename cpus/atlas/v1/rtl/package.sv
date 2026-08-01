package atlas_pkg

typedef enum logic [3:0] {
    ALU_ADD,
    ALU_SUB,
    ALU_AND,
    ALU_OR,
    ALU_XOR,
    ALU_SLL,
    ALU_SRL,
    ALU_SRA,
    ALU_SLT,
    ALU_SLTU
} alu_op_t;

typedef struct packed {
    logic [4:0] rs1;
    logic [4:0] rs2;
    logic [4:0] rd;

    logic [2:0] funct3;
    logic [6:0] funct7;

    logic [31:0] immediate;

    logic uses_rs1;
    logic uses_rs2;

    logic writes_rd;

    alu_op_t alu_operation;
} decoded_instruction_t;

endpackage