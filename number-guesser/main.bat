@echo off
setlocal enabledelayedexpansion

set /a "num=(%RANDOM% %% 100)"

:GameLoop
    set "guess="
    set /p "guess=Guess: "

    if not defined guess (
        echo Please enter a valid number
        goto :GameLoop
    )
    
    echo !guess! | findstr /R "[,. ]" >nul
    if not errorlevel 1 (
        echo Please enter a valid number
        goto :GameLoop
    )

    if !guess! LSS !num! (
        echo Higher
    ) else if !guess! GTR !num! (
        echo Lower
    ) else (
        if not "!guess!"=="!num!" (
            echo Please enter a valid number
            goto :GameLoop
        )

        echo You got it^^! The number was !num!^^!
        goto :End
    )

    goto :GameLoop

:End
endlocal