module mux2_8 (
    input  logic [7:0] a,
    input  logic [7:0] b,
    input  logic       sel,

    output logic [7:0] out
);

always_comb begin
    out = a

    if (sel)
        out = b
end
    
endmodule