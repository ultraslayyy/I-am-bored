       IDENTIFICATION DIVISION
       PROGRAM-ID. GUESS.

       DATA DIVISION.
       WORKING-STORAGE SECTION.
       01 NUM        PIC 99.
       01 GUESS      PIC 99.
       01 SEED       PIC 9(9).

       PROCEDURE DIVISION.
           ACCEPT SEED FROM TIME
           COMPUTE NUM = FUNCTION RANDOM(SEED) * 100 + 1

           PERFORM UNTIL GUESS = NUM
               DISPLAY "Guess: "
               ACCEPT GUESS

               IF GUESS > NUM
                   DISPLAY "Lower"
               ELSE IF GUESS < NUM
                   DISPLAY "Higher"
               END-IF
           END-PERFORM

           DISPLAY "You got it! The number was " NUM
           STOP RUN.