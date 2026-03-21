val () =
  Random.seed (Random.newgen ());
  val num = Random.randRange (0,100, Random.newgen ())

  fun loop () =
    (print "Guess: ";
      case Int.fromString (TextIO.inputLine TextIO.stdIn) of
          NONE => (print "Please enter a valid number\n"; loop ())
        | SOME guess =>
            if guess > num then (print "Lower\n"; loop ())
            else if guess < num then (print "Higher\n"; loop ())
            else print ("You got it! The number was " ^ Int.toString num ^ "\n")
    )
  in loop () end