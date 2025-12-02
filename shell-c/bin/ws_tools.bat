@echo off
if "%1"=="build" (
    gcc ..\mysh\src\main.c ..\mysh\src\builtins\*.c ..\mysh\src\utils\*.c -I..\mysh\include -o ..\mysh-win32.exe
) else if "%1"=="run" (
    if "%2"=="-o" (
        start ../mysh-win32.exe
    ) else (
        "../mysh-win32.exe"
    )
) else (
    echo Command not recognised
)