--- Maybe use own folder and use .ads?
with Ada.Text_IO; use Ada.Text_IO;
with Ada.Integer_Text_IO; use Ada.Integer_TEXT_IO;
with Ada.Numerics.Float_Random; use Ada.Numerics.Float_Random;

procedure Guess is
    Gen : Generator;
    Num : Integer := Integer (Random (Gen) * 101.0);
    Guess : Integer
begin
    loop
        Put("Guess: ");
        Get(Guess);
        if Guess > Num then
            Put_Line("Lower");
        elsif Guess < Num then
            Put_Line("Higher");
        else
            Put_Line("You got it! The number was " & Integer'Image(Num));
            exit;
        end if
    end loop
end Guess;