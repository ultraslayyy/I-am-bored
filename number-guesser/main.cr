num = Random.new.rand(0..100)

loop do
    print "Guess: "
    input = gets
    next unless input

    begin
        guess = input.to_i
        if guess > num
            puts "Lower"
        elsif guess < num
            puts "Higher"
        else
            puts "You got it! The number was #{num}"
            break
        end
    rescure
        puts "Please enter a valid number"
    end
end