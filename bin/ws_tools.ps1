param (
    [string]$command,
    [switch]$option
)

switch ($command) {
    "build" {
        gcc ..\src\win_shell.c -o ..\win_shell.exe
    }
    "run" {
        if ($option) {
            Start-Process ..\win_shell.exe
        } else {
            & ..\win_shell.exe
        }
    }
    default {
        Write-Host "Command not recognised"
    }
}