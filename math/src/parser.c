#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/parser.h"

char *strndup(const char *s, size_t n) {
    size_t len = 0;

    while (len < n && s[len] != '\0') {
        len++;
    }

    char *copy = malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, s, len);
    copy[len] = '\0';

    return copy;
}

static void advance(parser_t *parser) {
    parser->current = lexer_next(&parser->lexer);
}

static int check(parser_t *parser, token_type_t type) {
    return parser->current.type == type;
}

static token_t consume(parser_t *parser, token_type_t type) {
    token_t token = parser->current;

    if (token.type != type) {
        printf("parser_t error: expected %s\n", token_type_name(type));
    }

    advance(parser);

    return token;
}

static int precedence(token_type_t type) {
    switch (type) {
        case TOKEN_EQUAL: return 1;
        case TOKEN_PLUS:
        case TOKEN_MINUS: return 10;
        case TOKEN_STAR:
        case TOKEN_SLASH: return 20;
        case TOKEN_CARET: return 30;
        default:          return 0;
    }
}

static int can_start_expression(token_type_t type) {
    return type == TOKEN_NUMBER || type == TOKEN_IDENTIFIER || type == TOKEN_LPAREN;
}

static ast_node_t *parse_expression(parser_t *parser, int min_precedence);

static ast_node_t *parse_primary(parser_t *parser) {
    token_t token = parser->current;

    if (check(parser, TOKEN_NUMBER)) {
        advance(parser);

        return ast_number(parser->arena, token.number);
    }

    if (check(parser, TOKEN_IDENTIFIER)) {
        advance(parser);

        char *name = strndup(token.start, token.length);

        if (check(parser, TOKEN_LPAREN)) {
            advance(parser);

            ast_node_t *args[16];
            int count = 0;

            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    args[count++] = parse_expression(parser, 0);
                } while (check(parser, TOKEN_COMMA) && (advance(parser), 1));
            }

            consume(parser, TOKEN_RPAREN);

            ast_node_t **allocated = arena_alloc(parser->arena, sizeof(ast_node_t *) * count);

            memcpy(allocated, args, sizeof(ast_node_t *)*count);

            return ast_function(parser->arena, name, allocated, count);
        }

        return ast_variable(parser->arena, name);
    }

    if (check(parser, TOKEN_LPAREN)) {
        advance(parser);

        ast_node_t *node = parse_expression(parser, 0);

        consume(parser, TOKEN_RPAREN);

        return node;
    }

    return NULL;
}

static ast_node_t *parse_unary(parser_t *parser) {
    if (check(parser, TOKEN_MINUS)) {
        advance(parser);

        return ast_unary(parser->arena, UN_NEGATE, parse_unary(parser));
    }

    if (check(parser, TOKEN_PLUS)) {
        advance(parser);

        return ast_unary(parser->arena, UN_POSITIVE, parse_unary(parser));
    }

    return parse_primary(parser);
}

static ast_node_t *parse_expression(parser_t *parser, int min_precedence) {
    ast_node_t *left = parse_unary(parser);

    while (1) {
        int implicit = can_start_expression(parser->current.type);
        int prec = implicit ? 20 : precedence(parser->current.type);

        if (prec < min_precedence) break;

        token_type_t op = TOKEN_STAR;

        if (!implicit) {
            op = parser->current.type;
            advance(parser);
        }

        ast_node_t *right = parse_expression(parser, prec + 1);

        binary_operator_t binary;

        switch (op) {
            case TOKEN_PLUS:
                binary = OP_ADD;
                break;
            case TOKEN_MINUS:
                binary = OP_SUB;
                break;
            case TOKEN_STAR:
                binary = OP_MUL;
                break;
            case TOKEN_SLASH:
                binary = OP_DIV;
                break;
            case TOKEN_CARET:
                binary = OP_POW;
                break;
            default:
                return left;
        }

        left = ast_binary(parser->arena, binary, left, right);
    }

    return left;
}

void parser_init(parser_t *parser, const char *source, arena_t *arena) {
    lexer_init(&parser->lexer, source);

    parser->arena = arena;

    advance(parser);
}

ast_node_t *parser_parse(parser_t *parser) {
    ast_node_t *left = parse_expression(parser, 1);

    if (check(parser, TOKEN_EQUAL)) {
        advance(parser);

        ast_node_t *right = parse_expression(parser, 1);

        return ast_equation(parser->arena, left, right);
    }

    return left;
}