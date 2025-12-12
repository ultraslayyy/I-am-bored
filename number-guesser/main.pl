#!/usr/bin/perl
use strict;
use warnings;

sub main {
    my $num = int(rand(100)) + 1;
    my $guess;

    while (1) {
        print "Guess: "
        chomp($guess = <STDIN>);

        if ($guess !~ /^\d+$/) {
            print "Please enter a valid number.\n";
            next;
        }

        if ($guess > $num) {
            print "Lower\n";
        } elsif ($guess < $num) {
            print "Higher\n";
        } else {
            print "You got it! The number was $num\n";
            last;
        }
    }
}

main();