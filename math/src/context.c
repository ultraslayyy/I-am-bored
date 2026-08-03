#include <stdlib.h>
#include <string.h>
#include "context.h"

char *strdup(const char *s) {
    if (s == NULL) {
        return NULL;
    }

    size_t len = strlen(s) + 1;

    char *copy = malloc(len);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, s, len);

    return copy;
}

void context_init(context_t *ctx) {
    ctx->variables = NULL;
    ctx->count = 0;
}

void context_set(context_t *ctx, const char *name, double value) {
    for (size_t i = 0; i < ctx->count; ++i) {
        if (strcmp(ctx->variables[i].name, name) == 0) {
            ctx->variables[i].value = value;
            return;
        }
    }

    ctx->variables = realloc(ctx->variables, sizeof(variable_t) * (ctx->count + 1));

    ctx->variables[ctx->count].name  = strdup(name);
    ctx->variables[ctx->count].value = value;

    ctx->count++;
}

int context_get(context_t *ctx, const char *name, double *value) {
    for (size_t i = 0; i < ctx->count; ++i) {
        if (strcmp(ctx->variables[i].name, name) == 0) {
            *value = ctx->variables[i].value;
            return 1;
        }
    }

    return 0;
}

void context_free(context_t *ctx) {
    for (size_t i = 0; i < ctx->count; ++i) {
        free(ctx->variables[i].name);
    }
    
    free(ctx->variables);
}