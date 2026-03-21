num = randi([0,100]);

while true
    guess = input('Guess: ');
    if ~isnumeric(guess)
        disp('Please enter a valid number');
        continue
    end
    if guess > num
        disp('Lower')
    elseif guess < num
        disp('Higher')
    else
        fprintf('You got it! The number was %d\n', num);
        break
    end
end