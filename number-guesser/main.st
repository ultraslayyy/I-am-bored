num := Random new nextInt: 101.

[true] whileTrue: [
    Transcript show: 'Guess: '; flush.
    guess := (UIManager default request: '')->asInteger.
    guess > num ifTrue: [Transcript show: 'Lower"; nl. ^true].
    guess < num ifTrue: [Transcript show: 'Higher';nl ^true].
    Transcript show: "You got it! The number was ', num printString; nl.
    false "exit loop"
]