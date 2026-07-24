module program_counter (
    input logic clk,
    input logic reset,
    
    input logic jump,
    input logic jump_rel,
    input logic signed [7:0] jump_addr,

    output logic [7:0] address
);

always_ff @(posedge clk or posedge reset) begin
    if (reset)
        address <= 8'b0;
    else if (jump) begin
        if (jump_rel)
            address <= address + jump_addr;
        else
            address <= jump_addr;
    end else
        address <= address + 1;
end

endmodule