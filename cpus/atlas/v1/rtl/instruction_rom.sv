module instruction_rom (
    input  logic [31:0] address,

    output logic [31:0] instruction
);

logic [31:0] memory [0:255];

initial begin
    $readmemh(
        "program.hex",
        memory
    );
end

assign instruction = memory[address >> 2];

endmodule