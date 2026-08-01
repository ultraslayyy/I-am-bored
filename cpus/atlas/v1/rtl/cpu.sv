module cpu
    import atlas_pkg::*;
(
    input logic clk,
    input logic reset
);

logic [31:0] pc;

logic [31:0] instruction;

decoded_instruction_t decoded;

logic [31:0] rs1_data;
logic [31:0] rs2_data;
logic        reg_write_enable;
logic [31:0] reg_write_data;

logic [31:0] alu_result;

pc u_pc (
    .clk(clk),
    .reset(reset),

    .address(pc)
);

instruction_rom u_rom (
    .address(pc),

    .instruction(instruction)
);

decoder u_decoder (
    .instruction(instruction),

    .decoded(decoded)
);

regfile u_regfile (
    .clk(clk),

    .rs1(decoded.rs1),
    .rs2(decoded.rs2),

    .rs1_data(rs1_data),
    .rs2_data(rs2_data),

    .write_enable(reg_write_enable),
    .rd(decoded.rd),
    .write_data(reg_write_data)
);

alu u_alu (
    .a(rs1_data),
    .b(decoded.immediate),

    .result(alu_result)
);

assign reg_write_enable = decoded.writes_rd;
assign reg_write_data   = alu_result;

endmodule