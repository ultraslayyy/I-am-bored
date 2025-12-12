<?php
$num = rand(1, 100);
$stdin = fopen('php://stdin', 'r');

while (1) {
    echo "Guess: ";
    $input = trim(fgets($stdin));
    
    if ($input === '') {
        echo "Please enter a valid number\n";
        continue;
    }
    $guess = filter_var($input, FILTER_VALIDATE_INT);
    if ($guess === false) {
        echo "Please enter a valid number\n";
        continue;
    }
    
    if ($guess > $num) {
        echo "Lower\n";
    } else if ($guess < $num) {
        echo "Higher\n";
    } else {
        echo "You got it right! The number was " . $num;
        break;
    }
}
?>