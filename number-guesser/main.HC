U0 Main()
{
    Seed(TSC());
    I64 num, guess;
    C8 input[100];
    
    num = Rand() % 101;

    while (TRUE) {
        "Guess: ";
        Gets(input);

        if (!Str2I(input, &guess)) {
            "Please enter a valid number\n";
            continue;
        }

        if (guess > num) {
            "Lower\n";
        } else if (guess < num) {
            "Higher\n";
        } else {
            "You got it! The number was %d\n", num;
            break;
        }
    }
}