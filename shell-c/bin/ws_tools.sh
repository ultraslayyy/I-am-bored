#!/bin/bash

COMMAND=$1
OPTION=$2

case "$COMMAND" in
    build)
            gcc ../mysh/src/main.c ../mysh/src/builtins/*.c ../mysh/src/utils/*.c -I../mysh/include -o ../mysh-linux
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