module guess_game (
    input clk,
    input reset,
    input [6:0] guess,

    output reg higher,
    output reg lower,
    output reg correct
);

reg [6:0] lfsr = 7'b1010101;
reg [6:0] target = 7'd0;

reg [1:0] state;

localparam IDLE       = 2'd0;
localparam GENERATE   = 2'd1;
localparam LOCKED     = 2'd2;

wire feedback;
assign feedback = lfsr[6] ^ lfsr[5];

always @(posedge clk) begin
    lfsr <= {lfsr[5:0], feedback};
end

always @(posedge clk) begin
    case (state)

        IDLE: begin
            if (reset)
                state <= GENERATE;
        end

        GENERATE: begin
            if (lfsr <= 7'd100) begin
                target <= lfsr;
                state <= LOCKED;
            end
        end

        LOCKED: begin
            if (reset)
                state <= GENERATE;
        end

    endcase
end

always @(*) begin
    if (guess > target) begin
        higher  = 0;
        lower   = 1;
        correct = 0;
    end
    else if (guess < target) begin
        higher  = 1;
        lower   = 0;
        correct = 0;
    end
    else begin
        higher  = 0;
        lower   = 0;
        correct = 1;
    end
end

endmodule