#ifndef BUILTINS_ALIAS_H
#define BUILTINS_ALIAS_H

#define MAX_ALIASES 128
#define MAX_ALIAS_NAME 64
#define MAX_ALIAS_VALUE 256

typedef struct {
    char name[MAX_ALIAS_NAME];
    char value[MAX_ALIAS_VALUE];
} Alias;

extern Alias aliases[MAX_ALIASES];
extern int alias_count;

void builtin_alias(int argc, char **argv);
void builtin_unalias(int argc, char **argv);
const char *alias_lookup(const char *name);

#endif