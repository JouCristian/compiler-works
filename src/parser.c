#include "parser.h"

#include <string.h>
#include <stdio.h>

void parser_init(Parser *parser,
		const Token *tokens,
		size_t count) {
	parser->tokens = tokens;
	parser->count = count;
	parser->pos = 0;
	parser->failed = false;
	parser->error_line = 0;
	parser->error_message[0] = '\0';
}

const Token *parser_peek(const Parser *parser) {
	if(parser->pos >= parser->count) return NULL;
	return &parser->tokens[parser->pos];
}

void parser_advance(Parser *parser) {
	if(parser->pos < parser->count) {
		parser->pos++;
	}
}

bool parser_match(Parser *parser,
		TokenType type,
		const char *lexeme) {
	if(parser->failed) return false;
	
	const Token *token = parser_peek(parser);
	if(token == NULL ||
		token->type != type ||
		(lexeme != NULL &&
		 strcmp(token->lexeme,lexeme) != 0)) {
		return false;
	}

	parser_advance(parser);
	return true;
}

bool parser_expect(Parser *parser,
		TokenType type,
		const char *lexeme) {
	if(parser->failed) return false;
	if(parser_match(parser,type,lexeme)) return true;

	const Token *token = parser_peek(parser);
	parser->failed = true;
	parser->error_line = token != NULL
		? token->line
		: (parser->count > 0
			? parser->tokens[parser->count - 1].line
			: 1);
	const char *expected = lexeme != NULL
		? lexeme
		: (type == TOKEN_IDENTIFIER
			? "identifier"
			: (type == TOKEN_NUMBER
				? "number"
				: "token"));
	snprintf(parser->error_message,
		ERROR_MSG_LEN,
		"Expected '%s',got '%s'",
		expected,
		token != NULL ? token->lexeme : "EOF");

	return false;
		
}
