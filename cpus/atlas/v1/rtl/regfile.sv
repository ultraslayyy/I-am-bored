module regfile (
    input  logic        clk,

    input  logic [4:0]  rs1,
    input  logic [4:0]  rs2,

    output logic [31:0] rs1_data,
    output logic [31:0] rs2_data,

    input  logic        write_enable,
    input  logic [4:0]  rd,
    input  logic [31:0] write_data
);

logic [31:0] registers [0:31];

assign rs1_data = (rs1 == 0) ? 32'b0 : registers[rs1];
assign rs2_data = (rs2 == 0) ? 32'b0 : registers[rs2];

always_ff @(posedge clk) begin
    if (write_enable && rd != 0)
        registers[rd] <= write_data;
end

endmodule