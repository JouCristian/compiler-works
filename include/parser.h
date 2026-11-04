#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdbool.h>
#include "lexer.h"

#define ERROR_MSG_LEN 128

typedef struct {
	const Token *tokens;
	size_t count;
	size_t pos;
	bool failed;
	int error_line;
	char error_message[ERROR_MSG_LEN];
} Parser;

void parser_init(Parser *parser,
		const Token *tokens,
		size_t count);

bool parser_program(Parser *parser);

#endif
