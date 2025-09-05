param (
    [string]$command,
    [switch]$option
)

switch ($command) {
    "build" {
        gcc ..\mysh\main.c ..\mysh\cd.c ..\mysh\exit.c ..\mysh\pwd.c ..\mysh\history.c ..\mysh\utils\history.c -o ..\mysh.exe
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