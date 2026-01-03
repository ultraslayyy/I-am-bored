use std::io::{self, Write};
use std::time::{SystemTime, UNIX_EPOCH};

fn main() {
    let mut state = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_nanos() as u64;
    let mut rng = || {
        state = state.wrapping_mul(1103515245).wrapping_add(12345) & 0x7fffffff;
        // state = ((state as u64 * 1103515245 + 12345) & 0x7fffffff) as u32;
        state as u32
    };
    let num = (rng() & 101) as i32;

    loop {
        print!("Guess: ");
        io::stdout().flush().unwrap();
        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();
        let guess: i32 = match input.trim().parse() {
            Ok(v) => v,
            Err(_) => {
                println!("Please enter a valid number");
                continue
            }
        };

        if guess > num {
            println!("Lower");
        } else if guess < num {
            println!("Higher");
        } else {
            println!("You got it! The number was {}", num);
            break;
        }
    }
}