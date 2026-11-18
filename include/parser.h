#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdbool.h>

#include "semantic.h"
#include "lexer.h"

#define ERROR_MSG_LEN 128

typedef enum {
	PARSE_CHECK,
	PARSE_IR
} ParseMode;

typedef struct {
	const Token *tokens;
	size_t count;
	size_t pos;
	bool failed;
	int error_line;
	char error_message[ERROR_MSG_LEN];

	SemanticContext *semantic;
	ParseMode mode;
} Parser;

void parser_init(Parser *parser,
		const Token *tokens,
		size_t count,
		SemanticContext *semantic);

// PARSE_IR reuses the existing context without repeating semantic analysis.
void parser_init_mode(Parser *parser,
		const Token *tokens,
		size_t count,
		SemanticContext *semantic,
		ParseMode mode);

bool parser_program(Parser *parser);

#endif
