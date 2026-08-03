#pragma once

#include <stddef.h>

typedef enum {
    TOKEN_EOF,

    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_CARET,

    TOKEN_LPAREN,
    TOKEN_RPAREN,

    TOKEN_COMMA,

    TOKEN_EQUAL,

    TOKEN_INVALID
} token_type_t;

typedef struct {
    token_type_t type;
    const char  *start;
    size_t       length;
    double       number;
} token_t;

const char *token_type_name(token_type_t type);