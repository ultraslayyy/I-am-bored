#!/bin/bash

COMMAND=$1
OPTION=$2

case "$COMMAND" in
    build)
        if [ "$OPTION" = "--windows" ]; then
            x86_64-w64-mingw32-gcc ../mysh/src/main.c ../mysh/src/builtins/*.c ../mysh/src/utils/*.c -I../mysh/include -o ../mysh-win32.exe
        else
            gcc ../mysh/src/main.c ../mysh/src/builtins/*.c ../mysh/src/utils/*.c -I../mysh/include -o ../mysh-linux
        fi
        ;;
    run)
        if [ "$OPTION" = "-o" ]; then
            gnome-terminal -- bash -c "../mysh; exec bash"
        else
            ../mysh
        fi
        ;;
    *)
        echo "❌ Command not recognised. Use 'build' or 'run'."
        ;;
esac