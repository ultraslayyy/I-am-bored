@echo off
if "%1"=="build" (
    gcc ..\mysh\src\main.c ..\mysh\src\builtins\*.c ..\mysh\src\utils\*.c -I..\mysh\include -o ..\mysh.exe
) else if "%1"=="run" (
    if "%2"=="-o" (
        start ../mysh.exe
    ) else "../mysh.exe"
) else (
    echo Command not recognised
)