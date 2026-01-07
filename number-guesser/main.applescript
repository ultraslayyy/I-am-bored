set num to (random number from 1 to 100)

repeat
    display dialog "Guess:" default answer ""
    set guessText to text returned of result

    try
        set guess to guessText as integer
    on error
        display dialog "You must enter a valid number"
        next repeat
    end try

    if guess > num then
        display dialog "Lower"
    else if guess < num then
        display dialog "Higher"
    else
        display dialog "You got it! The number was " & num
        exit repeat
    end if
end repeat
