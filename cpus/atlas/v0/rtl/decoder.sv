module decoder
    import opcodes::*;
(
    input logic [15:0] instruction,

    output opcodes_t   opcode,

    output logic       is_src_reg,
    output logic [1:0] src_reg,
    output logic [1:0] dst_reg,

    output logic [7:0] immediate
);

assign opcode = instruction[15:12];
assign src_reg = instruction[11:10];
assign dst_reg = instruction[9:8];
assign immediate = instruction[7:0];

always_comb begin
    case (opcode)
        OP_MOVR,
        OP_ADDR,
        OP_SUBR,
        OP_CMPR,
        OP_STORE:
            is_src_reg = 1;
        default:
            is_src_reg = 0;
    endcase
end

endmodule