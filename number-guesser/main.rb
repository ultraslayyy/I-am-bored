#!/usr/bin/env ruby

def main
    num = Random.rand(1..100)

    while true do
        print "Guess: "
        guess = gets.chomp.to_i

        if guess == 0
            puts "Please enter a valid number"
            next
        end

        if guess > num
            puts "Lower"
        elsif guess < num
            puts "Higher"
        else
            puts "You got it! The number was " + num.to_s
            break
        end
    end
end

main if __FILE__ == $PROGRAM_NAME