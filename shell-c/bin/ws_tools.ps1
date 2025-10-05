param (
    [string]$command,
    [switch]$option
)

switch ($command) {
    "build" {
        if ($options -eq "--linux") {
            gcc ../mysh/src/main.c ../mysh/src/builtins/*.c ../mysh/src/utils/*.c -I../mysh/include -o ../mysh-linux
        } else {
            gcc ..\mysh\src\main.c ..\mysh\src\builtins\*.c ..\mysh\src\utils\*.c -I..\mysh\include -o ..\mysh-win32.exe
        }
    }
    "run" {
        if ($option -eq "-o") {
            Start-Process ..\mysh.exe
        } else {
            & ..\mysh.exe
        }
    }
    default {
        Write-Host "Command not recognised"
    }
}