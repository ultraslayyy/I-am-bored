#include <windows.h>
#include <stdio.h>
#include "builtins.h"

int builtin_exit(int argc, char **argv) {
    exit(0);
}