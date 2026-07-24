module alu_tb;

import alu_pkg::*;

logic [7:0] a;
logic [7:0] b;

alu_op_t    op;

logic [7:0] result;

alu uut(
    .a(a),
    .b(b),

    .op(op),

    .result(result)
);

initial begin
    // ADD
    a = 8'd5;
    b = 8'd3;

    op = ALU_ADD;

    #1;

    assert(result == 8'd8);

    $display("ADD passed");


    // SUB
    a = 8'd5;
    b = 8'd3;
    op = ALU_SUB;

    #1;

    assert(result == 8'd2);

    $display("SUB passed");


    // AND
    a = 8'd5;
    b = 8'd3;
    op = ALU_AND;

    #1;

    assert(result == 8'd1);

    $display("AND passed");


    // OR
    a = 8'd5;
    b = 8'd3;
    op = ALU_OR;

    #1;

    assert(result == 8'd7);

    $display("OR passed");


    // XOR
    a = 8'd5;
    b = 8'd3;
    op = ALU_XOR;

    #1;

    assert(result == 8'd6);

    $display("XOR passed");


    // NOT
    a = 8'd5;
    op = ALU_NOT;

    #1;

    assert(result == 8'b11111010);

    $display("NOT passed");


    $display("All passed");

    $finish;
end

endmodule