my $num = (0..100).pick;

loop {
    print "Guess: ";
    my $input = $*IN.get;

    unless $input ~~ /^\d+$/ {
        say "Please enter a valid number";
        next;
    }

    my $guess = $input.Int;

    if $guess > $num {
        say "Lower";
    }
    elsif $guess < $num {
        say "Higher"
    }
    else {
        say "You got it! The number was $num";
        last;
    }
}