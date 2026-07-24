module atlas;

logic clk;

initial begin
    clk = 0;

    forever #5 clk = ~clk;
end

logic [7:0]  pc;
logic [15:0] instruction;

program_counter pc_unit(
    .clk(clk),
    .address(pc)
);

rom instruction_memory(
    .address(pc),
    .data(instruction)
);

endmodule