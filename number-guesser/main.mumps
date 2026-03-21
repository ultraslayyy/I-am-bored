NEW num, guess
SET num=$R(101)

FOR  DO  QUIT:guess=num
. WRITE "Guess: "
. READ guess
. IF +guess'=guess WRITE "Please enter a valid number",! QUIT
. IF guess>num WRITE "Lower",! QUIT
. IF guess<num WRITE "Higher",! QUIT
. WRITE "You got it! The number was ",num,!,!