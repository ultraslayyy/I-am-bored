num ← ?101

:Repeat
    'Guess: ' ⎕←
    guess ← ⎕
    :If guess>num
        'Lower' ⎕←
    :ElseIf guess<num
        'Higher' ⎕←
    :Else
        ('You got it! The number was ', ⍕num) ⎕←
        :Leave
    :EndIf
:EndRepeat