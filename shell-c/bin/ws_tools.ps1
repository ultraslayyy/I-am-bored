param (
    [string]$command,
    [switch]$option
)

switch ($command) {
    "build" {
        gcc $(find ..\mysh\src -name '*.c') -I..\mysh\include -o ..\mysh.exe

    }
    "run" {
        if ($option) {
            Start-Process ..\mysh.exe
        } else {
            & ..\mysh.exe
        }
    }
    default {
        Write-Host "Command not recognised"
    }
}