-module(guess).
-export([start/0]).

start() ->
    {_, _} = rand:seed(exsplus, os:timestamp()),
    Num = rand:uniform(101) - 1,
    loop(Num).

loop(Num) ->
    io:format("Guess: "),
    case io:fread("", "~d") of
        {ok, [Guess]} ->
            if
                Guess > Num ->
                    io:format("Lower~n"),
                    loop(Num);
                Guess < Num ->
                    io:format("Higher~n"),
                    loop(Num);
                true ->
                    io:format("You got it! The number was ~p~n", [Num])
            end;
        _ ->
            io:format("Please enter a valid number~n"),
            loop(Num)
    end.