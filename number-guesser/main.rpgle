// This is old RPG
C     *ENTRY    PLIST
C               PARM                    GUESS

C     RANDNUM  SFLD
C     GUESS    SFLD

C     Z-ADD    0          RANDNUM
C     MULT     RANDNUM    101

CLOOP  DOW       1
C               EXFMT     PROMPT
C               IF        GUESS > RANDNUM
C               WRITE     LOWER
C               ELSEIF    GUESS < RANDNUM
C               WRITE     HIGHER
C               ELSE
C               WRITE     CORRECT
C               LEAVE
C               ENDIF
C               ENDDO

C     *INLR    SETON

// This is new RPG
**FREE
ctl-opt dftactgrp(*no) actgrp(*new);

dcl-s target int(3);
dcl-s guess  int(3);

target = %rand() * 101;

dow *in03 = *off;
    dsply 'Guess: ' guess;

    if %error() = *on;
        dsply 'Please enter a valid number';
        iter;
    endif;

    if guess > target;
        dsply 'Lower';
    elseif guess < target;
        dsply 'Higher';
    else;
        dsply ('You got it! The number was ' + %char(target));
        leave;
    endif;
enddo;

*inlr = *on;