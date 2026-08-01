module pc (
    input  logic clk,
    input  logic reset,

    output logic [31:0] address
);

always_ff @(posedge clk) begin
    if (reset)
        address <= 32'h00000000;
    else
        address <= address + 4;
end

endmodule