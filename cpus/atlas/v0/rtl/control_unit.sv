module control_unit
    import alu_pkg::*;
    import opcodes::*;
(
    input opcodes_t opcode,

    output alu_op_t alu_operation,
    
    output logic reg_write,
    output logic jump,

    output logic mem_read,
    output logic, mem_write
);

always_comb begin
    alu_operation = ALU_ADD;

    reg_write = 0;
    jump = 0;

    mem_read = 0;
    mem_write = 0;

    case (opcode)
        OP_ADDI: begin
            alu_operation = ALU_ADD;
            reg_write = 1;
        end

        OP_SUBI: begin
            alu_operation = ALU_SUB;
            reg_write = 1;
        end

        OP_LOAD: begin
            alu_operation = ALU_ADD;
            reg_write = 1;
            mem_read = 1;
        end
    endcase
end

endmodule