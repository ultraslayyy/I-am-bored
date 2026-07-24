module alu
    import alu_pkg::*;
(
    input  logic [7:0] a,
    input  logic [7:0] b,

    input  alu_op_t    op,

    output logic [7:0] result
);

always_comb begin
    case (op)
        ALU_ADD: result = a + b;
        ALU_SUB: result = a - b;
        ALU_AND: result = a & b;
        ALU_OR:  result = a | b;
        ALU_XOR: result = a ^ b;
        ALU_NOT: result = ~a;
        default: result = '0;
    endcase
end

endmodule