function main()
    local num = math.random(100)

    while (true) do
        io.write("Guess: ")
        io.flush()
        local input = io.read("*l")

        if input == nil or input == "" then
            io.write("Please enter a valid number")
        end
        local guess = tonumber(input);
        if guess == nil then
            io.write("Please enter a valid number")
        end

        if guess > num then
			io.write("Lower")
		elseif guess < num then
			io.write("Higher")
		else then
			io.write("You got it! The number was " .. num)
			break
        end
    end
end

main()