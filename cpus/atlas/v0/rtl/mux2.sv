module mux2 (
    input logic a,
    input logic b,
    input logic sel,

    output logic out
);
    
always_comb begin
    if (sel)
        out = b
    else
        out = a
end

endmodule