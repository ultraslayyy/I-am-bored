@echo off
if "%1"=="build" (
    gcc ../mysh/main.c ../mysh/cd.c ../mysh/exit.c -o ../mysh.exe
) else if "%1"=="run" (
    if "%2"=="-o" (
        start ../mysh.exe
    ) else "../mysh.exe"
) else (
    echo Command not recognised
)