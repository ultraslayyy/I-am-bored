#include <ctype.h>
#include <stdlib.h>
#include "lexer.h"

static char peek(lexer_t *lexer) {
    return lexer->source[lexer->position];
}

static char advance(lexer_t *lexer) {
    return lexer->source[lexer->position++];
}

static void skip_whitespace(lexer_t *lexer) {
    while (isspace(peek(lexer))) {
        advance(lexer);
    }
}

void lexer_init(lexer_t *lexer, const char *source) {
    lexer->source   = source;
    lexer->position = 0;
}

static token_t make_token(lexer_t *lexer, token_type_t type, size_t start) {
    token_t token;
    token.type   = type;
    token.start  = lexer->source + start;
    token.length = lexer->position - start;
    token.number = 0;

    return token;
}

token_t lexer_next(lexer_t *lexer) {
    skip_whitespace(lexer);

    size_t start = lexer->position;

    char c = advance(lexer);

    if (c == '\0') {
        return make_token(lexer, TOKEN_EOF, start);
    }

    if (isdigit(c) || c == '.') {
        while (isdigit(peek(lexer)) || peek(lexer) == '.') {
            advance(lexer);
        }

        token_t token = make_token(lexer, TOKEN_NUMBER, start);
        token.number = strtod(token.start, NULL);

        return token;
    }

    if (isalpha(c) || c == '_') {
        while (isalnum(peek(lexer)) || peek(lexer) == '_') {
            advance(lexer);
        }

        return make_token(lexer, TOKEN_IDENTIFIER, start);
    }

    switch (c) {
        case '+':
            return make_token(lexer, TOKEN_PLUS, start);
        case '-':
            return make_token(lexer, TOKEN_MINUS, start);
        case '*':
            return make_token(lexer, TOKEN_STAR, start);
        case '/':
            return make_token(lexer, TOKEN_SLASH, start);
        case '^':
            return make_token(lexer, TOKEN_CARET, start);
        case '(':
            return make_token(lexer, TOKEN_LPAREN, start);
        case ')':
            return make_token(lexer, TOKEN_RPAREN, start);
        case ',':
            return make_token(lexer, TOKEN_COMMA, start);
        case '=':
            return make_token(lexer, TOKEN_EQUAL, start);
    }

    return make_token(lexer, TOKEN_INVALID, start);
}