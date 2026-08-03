#pragma once

#include <stddef.h>

typedef struct {
    char  *name;
    double value;
} variable_t;

typedef struct {
    variable_t *variables;
    size_t count;
} context_t;

void context_init(context_t *ctx);
void context_set (context_t *ctx, const char *name, double  value);
int  context_get (context_t *ctx, const char *name, double *value);
void context_free(context_t *ctx);