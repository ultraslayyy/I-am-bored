#pragma once

#include "ast.h"
#include "lexer.h"

typedef struct {
    lexer_t  lexer;
    token_t  current;
    arena_t *arena;
} parser_t;

void        parser_init (parser_t *parser, const char *source, arena_t *arena);
ast_node_t *parser_parse(parser_t *parser);