#include <stdio.h>
#include "arena.h"
#include "lexer.h"
#include "include/parser.h"

int main() {
    /*
    lexer_t lexer;

    lexer_init(&lexer, "x^2 + 3*x + 2 = 6");

    while (1) {
        token_t token = lexer_next(&lexer);

        printf("%s\n", token_type_name(token.type));

        if (token.type == TOKEN_EOF) break;
    } */

    arena_t *arena = arena_create(1024 * 1024);

    parser_t parser;

    parser_init(&parser, "x^2+3x+2=6", arena);

    ast_node_t *tree = parser_parse(&parser);

    ast_print(tree, 0);

    arena_destroy(arena);
}