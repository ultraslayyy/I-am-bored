main <- function() {
    num <- sample(1:100, 1)

    while (TRUE) {
        input_str <- readline(prompt = "Guess: ")
        guess <- as.numeric(input_str)

        if (is.na(guess) || length(guess) != 1 || !is.finite(guess)) {
            print("Please enter a valid number")
            next
        }

        if (guess > num) {
            print("Lower")
        } else if (guess < num) {
            print("Higher")
        } else {
            print(paste("You got it! The number was", num))
            break
        }
    }
}

main()