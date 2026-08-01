module decoder
    import atlas_pkg::*;
(
    input logic [31:0] instruction,

    output decoded_instruction_t decoded
);

always_comb begin
    decoded = '0;

    decoded.rs1 = instruction[19:15];
    decoded.rs2 = instruction[24:20];
    decoded.rd  = instruction[11:7];

    decoded.funct3 = instruction[14:12];
    decoded.funct7 = instruction[31:25];

    case (instruction[6:0])
        // OP-IMM instructions
        7'b0010011: begin
            decoded.uses_rs1  = 1;
            decoded.writes_rd = 1;

            case (decoded.funct3)
                3'b000: begin
                    // ADDI
                    decoded.alu_operation = ALU_ADD;
                end
            endcase

            // Sign extend immediate
            decoded.immediate = {{20{instruction[31]}}, instruction[31:20]};
        end

        default: begin
            decoded = '0;
        end
    endcase
end

endmodule