#include "token.h"

const char *token_type_name(token_type_t type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";

        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";

        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_CARET: return "CARET";

        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";

        case TOKEN_COMMA: return "COMMA";

        case TOKEN_EQUAL: return "EQUAL";

        case TOKEN_INVALID: return "INVALID";
    }
}