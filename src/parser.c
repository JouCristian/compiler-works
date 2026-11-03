#include "parser.h"
#include "common.h"

#include <string.h>
#include <stdio.h>

//Announcement in advance
bool parser_arithmetic_expression(Parser *parser);

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
		const char *expected) {
	if(parser->failed) return false;
	
	const Token *token = parser_peek(parser);
	if(token == NULL ||
		token->type != type ||
		(expected != NULL &&
		 strcmp(token->lexeme,expected) != 0)) {
		return false;
	}

	parser_advance(parser);
	return true;
}

bool parser_expect(Parser *parser,
		TokenType type,
		const char *expected) {
	if(parser->failed) return false;
	if(parser_match(parser,type,expected)) return true;

	//Handle error message
	const Token *token = parser_peek(parser);
	parser->failed = true;
	parser->error_line = token != NULL
		? token->line
		: (parser->count > 0
			? parser->tokens[parser->count - 1].line
			: 1);
	const char *expected_lexeme = expected != NULL
		? expected
		: (type == TOKEN_IDENTIFIER
			? "identifier"
			: (type == TOKEN_NUMBER
				? "number"
				: "token"));
	snprintf(parser->error_message,
		ERROR_MSG_LEN,
		"Expected '%.48s',got '%.60s'",
		expected_lexeme,
		token != NULL ? token->lexeme : "EOF");

	return false;
		
}

static bool parser_fail(Parser *parser, const char *expected) {
	if(parser->failed) return false;

	const Token *token = parser_peek(parser);
	parser->failed = true;
	parser->error_line = token != NULL
		? token->line
		: (parser->count > 0
			? parser->tokens[parser->count - 1].line
			: 1);
	snprintf(parser->error_message,
		ERROR_MSG_LEN,
		"Expected '%.48s',got '%.60s'",
		expected,
		token != NULL ? token->lexeme : "EOF");
	return false;
}

static bool parser_factor(Parser *parser) {
	if(parser_match(parser,TOKEN_DELIMITER,"(")) {
		if(!parser_arithmetic_expression(parser)) return false;
		return parser_expect(parser,TOKEN_DELIMITER,")");
	}

	if(parser_match(parser,TOKEN_IDENTIFIER,NULL) ||
		parser_match(parser,TOKEN_NUMBER,NULL)) return true;

	return parser_fail(parser,"'(',identifier or number");
}

static bool parser_term(Parser *parser) {
	if(!parser_factor(parser)) return false;

	while(parser_match(parser,TOKEN_OPERATOR,"*") ||
		parser_match(parser,TOKEN_OPERATOR,"/")) {
		if(!parser_factor(parser)) return false;
	}

	return true;
}

bool parser_arithmetic_expression(Parser *parser) {
	if(!parser_term(parser)) return false;

	while(parser_match(parser,TOKEN_OPERATOR,"+") ||
		parser_match(parser,TOKEN_OPERATOR,"-")) {
		if(!parser_factor(parser)) return false;
	}

	return true;
}

static bool parser_relop(Parser *parser) {
	const char *ops[] = {">","<",">=","<=","==","!="};

	for(size_t i = 0; i < ARRLEN(ops); i++) {
		if(parser_match(parser,TOKEN_OPERATOR,ops[i])) return true;
	}

	return parser_fail(parser,"'>','<','>=','<=','==' or '!='");
}

bool parser_bool_expression(Parser *parser) {
	return parser_arithmetic_expression(parser) &&
		parser_relop(parser) &&
		parser_arithmetic_expression(parser);
}
