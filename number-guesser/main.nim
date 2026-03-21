import std/[random, strutils]

randomize()
let num = rand(100)

while true:
    stdout.write("Guess: ")
    let input = readLine(stdin)

    try:
        let guess = parseInt(input)

        if guess > num:
            echo "Lower"
        elif guess < num:
            echo "Higher"
        else:
            echo "You got it! The number was ", num
            break

    except ValueError:
        echo "Please enter a valid number"