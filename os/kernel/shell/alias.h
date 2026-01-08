#pragma once
#include <stddef.h>

#define MAX_ALIASES 32
#define MAX_ALIAS_NAME 32
#define MAX_ALIAS_VALUE 128
#define MAX_ALIAS_RECURSION 5

void alias_init(void);
void alias_set(const char *name, const char *value);
void alias_unset(const char *name);
const char *alias_lookup(const char *name);
void alias_list();
