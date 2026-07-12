import rand

fn main() {
    numer := rand.intn(101) or { 0 }

    for {
        println('Guess:')
        guess := input('').int()

        if guess < number {
            println('Higher')
        } else if guess > number {
            println('Lower')
        } else {
            println('You got it!')
            break
        }
    }
}