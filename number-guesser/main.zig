const std = @import("std");

pub fn main() void {
    var prng = std.rand.DefaultPrng.init(std.crypto.random.int(u64))
    const rand = prng.random();
    const num = random.intRangeAtMost(u8, 1, 100);

    const stdout = std.io.getStdOut().writer();
    const stdin = std.io.getStdIn().reader();
    var buf: [1024]u8 = undefined;

    while (true) {
        try stdout.print("Guess: ", .{});
        const line = (try stdin.readUntilDelimiterOrEof(&buf, '\n')) orelse break;
        const trimmed = std.mem.trim(u8, line, "\r\n");
        const guess = std.fmt.parseInt(u8, trimmed, 10) catch {
            try stdout.print("Please enter a valid number\n", .{});
            continue;
        };

        if (guess > num) {
            try stdout.print("Lower\n", .{});
        } else if (guess < num) {
            try stdout.print("Higher\n", .{});
        } else {
            try stdout.print("You got it! The number was {d}\n", .{num});
            break;
        }
    }
}