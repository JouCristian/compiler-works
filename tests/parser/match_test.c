#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "parser.h"

static bool check_read_error(Parser *parser, const char *mode) {
	const char *expected = "Expected ';',got '+'";
	if(parser_program(parser) || !parser->failed || parser->error_line != 2 ||
	   strcmp(parser->error_message, expected) != 0) {
		fprintf(stderr, "FAIL: %s: read expected line 2: %s; got line %d: %s\n",
			mode, expected, parser->error_line, parser->error_message);
		return false;
	}
	printf("PASS: %s: read requires a semicolon\n", mode);
	return true;
}

int main(void) {
	const Token tokens[] = {
		{TOKEN_DELIMITER, "{", 1},
		{TOKEN_KEYWORD, "read", 2},
		{TOKEN_IDENTIFIER, "a", 2},
		{TOKEN_OPERATOR, "+", 2},
		{TOKEN_DELIMITER, "}", 3}
	};
	Parser parser;
	parser_init(&parser, tokens, ARRLEN(tokens), NULL);
	bool check_ok = check_read_error(&parser, "PARSE_CHECK");

	SemanticContext semantic = {0};
	parser_init_mode(&parser, tokens, ARRLEN(tokens), &semantic, PARSE_IR);
	bool ir_ok = check_read_error(&parser, "PARSE_IR");
	return check_ok && ir_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
