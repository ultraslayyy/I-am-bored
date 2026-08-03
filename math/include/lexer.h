#pragma once

#include "token.h"

typedef struct {
    const char *source;
    size_t position;
} lexer_t;

void    lexer_init(lexer_t *lexer, const char *source);
token_t lexer_next(lexer_t *lexer);