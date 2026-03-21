num = RandomInteger[{0, 100}];

While[True,
    guess = Input["Guess: "];
    If[!IntegerQ[guess], Print["Please enter a valid number"]; Continue[]];
    If[guess > num, Print["Lower"],
        If[guess < num, Print["Higher"],
            Print["You got it! The number was ", num]; Break[]
        ]
    ]
]