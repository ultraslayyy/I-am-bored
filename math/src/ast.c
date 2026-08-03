#include <stdio.h>
#include <string.h>
#include "ast.h"

ast_node_t *ast_number(arena_t *arena, double value) {
    ast_node_t *node = arena_alloc(arena, sizeof(ast_node_t));

    node->type = AST_NUMBER;
    node->number.value = value;

    return node;
}

ast_node_t *ast_variable(arena_t *arena, char *name) {
    ast_node_t *node = arena_alloc(arena, sizeof(ast_node_t));

    node->type = AST_VARIABLE;
    node->variable.name = name;

    return node;
}

ast_node_t *ast_binary(arena_t *arena, binary_operator_t op, ast_node_t *left, ast_node_t *right) {
    ast_node_t *node = arena_alloc(arena, sizeof(ast_node_t));

    node->type         = AST_BINARY;
    node->binary.op    = op;
    node->binary.left  = left;
    node->binary.right = right;

    return node;
}

ast_node_t *ast_unary(arena_t *arena, unary_operator_t op, ast_node_t *operand) {
    ast_node_t *node = arena_alloc(arena, sizeof(ast_node_t));

    node->type          = AST_UNARY;
    node->unary.op      = op;
    node->unary.operand = operand;

    return node;
}

ast_node_t *ast_function(arena_t *arena, char *name, ast_node_t **args, int count) {
    ast_node_t *node = arena_alloc(arena, sizeof(ast_node_t));

    node->type = AST_FUNCTION;
    node->function.name = name;
    node->function.arguments = args;
    node->function.argument_count = count;

    return node;
}

ast_node_t *ast_equation(arena_t *arena, ast_node_t *left, ast_node_t *right) {
    ast_node_t *node = arena_alloc(arena, sizeof(ast_node_t));

    node->type           = AST_EQUATION;
    node->equation.left  = left;
    node->equation.right = right;

    return node;
}

static void print_indent(int depth) {
    for (int i = 0; i < depth; ++i) {
        printf("    ");
    }
}

static const char *binary_name(binary_operator_t op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_POW: return "^";
    }

    return "?";
}

static const char *unary_name(unary_operator_t op) {
    switch (op) {
        case UN_NEGATE: return "-";
        case UN_POSITIVE: return "+";
    }

    return "?";
}

void ast_print(ast_node_t *node, int depth) {
    if(node == NULL) {
        return;
    }

    print_indent(depth);


    switch(node->type) {
        case AST_NUMBER:
            printf("Number(%g)\n", node->number.value);
            break;
        case AST_VARIABLE:
            printf("Variable(%s)\n", node->variable.name);
            break;
        case AST_BINARY:
            printf("Binary(%s)\n", binary_name(node->binary.op));
            ast_print(node->binary.left,  depth + 1);
            ast_print(node->binary.right, depth + 1);
            break;
        case AST_UNARY:
            printf("Unary(%s)\n", unary_name(node->unary.op));
            ast_print(node->unary.operand, depth + 1);
            break;
        case AST_FUNCTION:
            printf("Function(%s)\n", node->function.name);
            for(int i = 0; i < node->function.argument_count; ++i) {
                ast_print(node->function.arguments[i], depth + 1);
            }
            break;
        case AST_EQUATION:
            printf("Equation\n");
            ast_print(node->equation.left,  depth + 1);
            ast_print(node->equation.right, depth + 1);
            break;
    }
}