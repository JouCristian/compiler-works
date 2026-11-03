#include <stdio.h>
#include "parser.h"

#define TK(type, value) {type, value, 1}
#define N(x)  TK(TOKEN_NUMBER, x)
#define ID(x) TK(TOKEN_IDENTIFIER, x)
#define OP(x) TK(TOKEN_OPERATOR, x)
#define D(x)  TK(TOKEN_DELIMITER, x)

#define CASE(name, is_bool, expected, ...) do {         \
    Token tokens[] = {__VA_ARGS__};                     \
    Parser parser;                                       \
    parser_init(&parser, tokens,                        \
                sizeof(tokens) / sizeof(tokens[0]));    \
    bool ok = (is_bool)                                 \
        ? parser_bool_expression(&parser)          \
        : parser_arithmetic_expression(&parser);   \
    ok = ok && parser_peek(&parser) == NULL;             \
    if (ok != (expected)) {                             \
        printf("FAIL: %s\n", name);                      \
        failures++;                                     \
    } else {                                            \
        printf("PASS: %s\n", name);                      \
    }                                                   \
} while (0)

int main(void) {
    int failures = 0;

    CASE("2+3*4", 0, true,
         N("2"), OP("+"), N("3"), OP("*"), N("4"));

    CASE("(2+3)*4", 0, true,
         D("("), N("2"), OP("+"), N("3"),
         D(")"), OP("*"), N("4"));

    CASE("a-b/2", 0, true,
         ID("a"), OP("-"), ID("b"), OP("/"), N("2"));

    CASE("a+2>=b*3", 1, true,
         ID("a"), OP("+"), N("2"),
         OP(">="), ID("b"), OP("*"), N("3"));

    CASE("a+*b", 0, false,
         ID("a"), OP("+"), OP("*"), ID("b"));

    CASE("(2+3", 0, false,
         D("("), N("2"), OP("+"), N("3"));

    CASE("2+", 0, false,
         N("2"), OP("+"));

    CASE("a>b>c", 1, false,
         ID("a"), OP(">"), ID("b"),
         OP(">"), ID("c"));

    CASE("2+3;", 0, false,
         N("2"), OP("+"), N("3"), D(";"));

    CASE("a=b", 1, false,
         ID("a"), OP("="), ID("b"));

    CASE("a>", 1, false,
         ID("a"), OP(">"));

    CASE("-3", 0, false,
         OP("-"), N("3"));

    CASE(";", 0, false,
         D(";"));

    printf("\nFailed: %d\n", failures);
    return failures ? 1 : 0;
}
