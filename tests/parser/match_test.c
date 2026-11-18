#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "parser.h"

int main(void) {
	const Token tokens[] = {
		{TOKEN_DELIMITER, "{", 1},
		{TOKEN_KEYWORD, "read", 2},
		{TOKEN_IDENTIFIER, "a", 2},
		{TOKEN_OPERATOR, "+", 2},
		{TOKEN_DELIMITER, "}", 3}
	};
	const char *expected = "Expected ';',got '+'";
	Parser parser;
	parser_init(&parser, tokens, ARRLEN(tokens), NULL);

	if(parser_program(&parser) || !parser.failed || parser.error_line != 2 ||
	   strcmp(parser.error_message, expected) != 0) {
		fprintf(stderr, "FAIL: read expected line 2: %s; got line %d: %s\n",
			expected, parser.error_line, parser.error_message);
		return EXIT_FAILURE;
	}

	printf("PASS: read requires a semicolon\n");
	return EXIT_SUCCESS;
}
