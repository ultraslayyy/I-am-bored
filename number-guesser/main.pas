program GuessGame;

uses
    SysUtils

var
    num, guess: Integer;

begin
    Randomise;
    num := Random(101)

    repeat
        Write('Guess: ')
        ReadLn(guess);

        if guess > num then
            Writeln('Lower')
        else if guess < num then
            Writeln('Higher')
        else
        begin
            Writeln('You got it! The number was ', num);
            Break;
        end;
    until (False);
end;