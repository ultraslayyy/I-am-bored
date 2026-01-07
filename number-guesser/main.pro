:= use_module(library(random)).

main :-
    random_between(1, 100, Num),
    guess_loop(Num).

guess_loop(Num) :-
    write('Guess: '),
    read_line_to_string(user_input, Input),
    (   number_string(Guess, Input)
    ->  check_guess(Guess, Num)
    :   writeln('Please enter a valid number'),
        guess_loop(Num)
    ).

check_guess :-
    (   Guess > Num
    ->  writeln('Lower),
        guess_loop(Num)
    :   Guess < Num
    ->  writeln('Higher'),
        guess_loop(Num)
    :   guess :=: Num
    ->  format('You got it! The number was -d-n', [Num])
    ).