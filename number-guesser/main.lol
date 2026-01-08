HAI 1.2
  I HAS A NUM ITZ SUM OF RANDOM AN 101
  I HAS A GUESS
  I HAS A INPUT

  IM IN YR GUESSLOOP
    VISIBLE "Guess:"
    GIMMEH INPUT

    GUESS R INPUT + 0

    BOTH SAEM GUESS AN NUM, O RLY?
      YA RLY
        VISIBLE "You got it! The number was " NUM
        GTFO
      NO WAI
        DIFFRINT GUESS AN NUM, O RLY?
          YA RLY
            GUESS BIGGER THAN NUM, O RLY?
              YA RLY
                VISIBLE "Lower"
              NO WAI
                VISIBLE "Higher"
            OIC
        OIC
    OIC
  IM OUTTA YR GUESSLOOP
KTHXBYE