set num [expr {int(rand() * 101)}]

while {1} {
    puts -nonewline "Guess: "
    flush stdout
    gets stdin input

    if {![string is integer -strict $input]} {
        puts "Please enter a valid number"
        continue
    }

    set guess $input

    if {$guess > $num} {
        puts "Lower"
    } elseif {$guess < $num} {
        puts "Higher"
    } else {
        puts "You got it! The number was $num"
        break
    }
}