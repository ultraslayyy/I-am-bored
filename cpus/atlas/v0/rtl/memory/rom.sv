module rom (
    input logic [7:0] address,

    output logic [15:0] data
);

logic [15:0] memory [0:255];

initial begin
    memory[0] = 16'h1234;
    memory[1] = 16'h5678;
end

assign data = memory[address];

endmodule