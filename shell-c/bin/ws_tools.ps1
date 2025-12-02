param (
    [string]$command,
    [switch]$option
)

switch ($command) {
    "build" {
        gcc ..\mysh\src\main.c ..\mysh\src\builtins\*.c ..\mysh\src\utils\*.c -I..\mysh\include -o ..\mysh-win32.exe
    }
    "run" {
        if ($option -eq "-o") {
            Start-Process ..\mysh-win32.exe
        } else {
            & ..\mysh-win32.exe
        }
    }
    default {
        Write-Host "Command not recognised"
    }
}