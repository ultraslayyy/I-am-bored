package Main api;

import Cpp library "random_nums.h"

fn random_number(min: i32, max: i32) -> i32;

fn Main() -> i32 {
    var number: i32 = random_number(1, 100);
    while (true) {
        Print("Guess: ");
        var guess: i32 = ReadInt();

        if (guess < number) {
            Print("Higher");
        } else if (guess > number) {
            Print("Lower");
        } else {
            Print("You got it!");
            break;
        }
    }

    return 0;
}