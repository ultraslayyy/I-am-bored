module register8_tb;

logic clk;

logic [7:0] d;
logic [7:0] q;


register8 uut(
    .clk(clk),
    .d(d),
    .q(q)
);

initial begin
    clk = 0;

    forever #5 clk = ~clk;
end

initial begin
    d = 8'h00;

    #10;

    d = 8'h42;

    #10;

    assert(q == 8'h42);

    $display("Register passed");

    $finish;
end

endmodule