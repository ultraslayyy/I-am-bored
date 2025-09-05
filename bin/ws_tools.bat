@echo off
if "%1"=="build" (
    gcc ../src/win_shell.c -o ../win_shell.exe
) else if "%1"=="run" (
    if "%2"=="-o" (
        start ../win_shell.exe
    ) else "../win_shell.exe"
) else (
    echo Command not recognised
)