#ifndef BUILTINS_H
#define BUILTINS_H

int builtin_cd(int argc, char **argv);
int builtin_exit();
int builtin_history();
int builtin_pwd();
int builtin_alias(int argc, char **argv);
int builtin_unalias(int argc, char **argv);
const char *alias_lookup(const char *name);
int builtin_env();
int builtin_set(int argc, char **argv);

#endif