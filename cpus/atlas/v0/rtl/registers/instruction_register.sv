module instruction_register (
    input  logic        clk,
    input  logic [15:0] d,
    output logic [15:0] q
);

always_ff @(posedge clk) begin
    q <= d;
end

endmodule