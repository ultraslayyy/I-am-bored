module program_counter_tb;

logic clk;
logic reset;

logic [7:0] address;

program_counter uut(
    .clk(clk),
    .reset(reset),
    .address(address)
);

initial begin
    clk = 0;

    forever #5 clk = ~clk;
end

initial begin
    reset = 1;

    #10;

    reset = 0;

    #10;

    assert(address == 1);

    #10;

    assert(address == 2);

    $display("PC passed");

    $finish;
end

endmodule