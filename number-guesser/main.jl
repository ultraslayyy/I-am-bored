@main function main(args...)
    num = rand(1:100)
    
    while true
        print("Guess: ")
        input = readline()
        guess = tryparse(Int, input)

        if isnothing(guess)
            println("Please enter a valid number")
            continue
        end

        if guess > num
            println("Lower")
        elseif guess < num
            println("Higher")
        else
            println("You got it! The number was $num")
            break
        end
    end
end