#include <stdio.h>
#include "parser.h"

int main(void) {
    Token tokens[] = {
        {TOKEN_KEYWORD, "read", 1},
        {TOKEN_IDENTIFIER, "a", 1},
        {TOKEN_DELIMITER, "+", 1}
    };

    Parser parser;
    parser_init(&parser, tokens, 3);

    bool success =
        parser_expect(&parser, TOKEN_KEYWORD, "read") &&
        parser_expect(&parser, TOKEN_IDENTIFIER, NULL) &&
        parser_expect(&parser, TOKEN_DELIMITER, ";");

    if (success && parser.pos == parser.count) {
        printf("Syntax analysis successful!\n");
    } else {
        printf("Syntax error at line %d: %s\n",
               parser.error_line,
               parser.error_message);
    }

    return 0;
}
