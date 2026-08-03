#pragma once

#include "arena.h"

typedef enum {
    AST_NUMBER,
    AST_VARIABLE,
    AST_BINARY,
    AST_UNARY,
    AST_FUNCTION,
    AST_EQUATION
} ast_type_t;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_POW
} binary_operator_t;

typedef enum {
    UN_NEGATE,
    UN_POSITIVE
} unary_operator_t;

typedef struct ast_node_t ast_node_t;

struct ast_node_t {
    ast_type_t type;

    union {
        struct {
            double value;
        } number;

        struct {
            char *name;
        } variable;

        struct {
            binary_operator_t op;

            ast_node_t *left;
            ast_node_t *right;
        } binary;

        struct {
            unary_operator_t op;
            ast_node_t *operand;
        } unary;

        struct {
            char *name;
            ast_node_t **arguments;
            int argument_count;
        } function;

        struct {
            ast_node_t *left;
            ast_node_t *right;
        } equation;
    };
};

ast_node_t *ast_number  (arena_t    *arena, double            value);
ast_node_t *ast_variable(arena_t    *arena, char             *name);
ast_node_t *ast_binary  (arena_t    *arena, binary_operator_t op,   ast_node_t  *left, ast_node_t *right);
ast_node_t *ast_unary   (arena_t    *arena, unary_operator_t  op,   ast_node_t  *operand);
ast_node_t *ast_function(arena_t    *arena, char             *name, ast_node_t **args, int         count);
ast_node_t *ast_equation(arena_t    *arena, ast_node_t       *left, ast_node_t  *right);
void        ast_print   (ast_node_t *node,  int               depth);