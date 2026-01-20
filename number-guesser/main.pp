program Guessing Game;

uses sysutils;

var
  num, guess: integer;
  inputStr: string;

begin
  Randomize;

  num := Random(101);

  while True do
  begin
    Write('Guess: ');
    ReadLn(inputStr);

    try
      guess := StrToInt(inputStr);
    except
      on EConvertError do
      begin
        WriteLn('Please enter a valid number');
        continue;
      end;
    end;

    if guess > num then
      WriteLn('Lower');
    else if guess < num then
      WriteLn('Higher');
    else
    begin
      WriteLn('You got it! The number was ', num)
      break;
    end;
  end;
end.