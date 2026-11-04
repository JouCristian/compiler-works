#include "parser.h"
#include "common.h"

#include <string.h>
#include <stdio.h>

//Announcement in advance
bool parser_arithmetic_expression(Parser *parser);
static bool parser_statement(Parser *parser);
static bool parser_statement_list(Parser *parser);

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
		if(!parser_term(parser)) return false;
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

static bool parser_assignment_expression(Parser *parser) {
	return parser_expect(parser,TOKEN_IDENTIFIER,NULL) &&
		parser_expect(parser,TOKEN_OPERATOR,"=") &&
		parser_arithmetic_expression(parser);
}

bool parser_assignment_stat(Parser *parser) {
	return parser_assignment_expression(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,";");
}

static bool parser_read_stat(Parser *parser) {
	return parser_expect(parser,TOKEN_KEYWORD,"read") &&
		parser_expect(parser,TOKEN_IDENTIFIER,NULL) &&
		parser_expect(parser,TOKEN_DELIMITER,";");
}

static bool parser_write_stat(Parser *parser) {
	return parser_expect(parser,TOKEN_KEYWORD,"write") &&
		parser_arithmetic_expression(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,";");
}

static bool parser_variable_list(Parser *parser) {
	while(parser_match(parser,TOKEN_DELIMITER,",")) {
		if(!parser_expect(parser,TOKEN_IDENTIFIER,NULL)) return false;
	}
	return true;
}

static bool parser_declaration_stat(Parser *parser) {
	return parser_expect(parser,TOKEN_KEYWORD,"int") &&
		parser_expect(parser,TOKEN_IDENTIFIER,NULL) &&
		parser_variable_list(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,";");
}

static bool parser_else_part(Parser *parser) {
	if(parser_match(parser,TOKEN_KEYWORD,"else")) return parser_statement(parser);
	return true;
}

static bool parser_if_stat(Parser *parser) {
	return parser_expect(parser,TOKEN_KEYWORD,"if") &&
		parser_expect(parser,TOKEN_DELIMITER,"(") &&
		parser_bool_expression(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,")") &&
		parser_statement(parser) &&
		parser_else_part(parser);
}

static bool parser_while_stat(Parser *parser) {
	return parser_expect(parser,TOKEN_KEYWORD,"while") &&
		parser_expect(parser,TOKEN_DELIMITER,"(") &&
		parser_bool_expression(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,")") &&
		parser_statement(parser);
}

static bool parser_for_stat(Parser *parser) {
	return parser_expect(parser,TOKEN_KEYWORD,"for") &&
		parser_expect(parser,TOKEN_DELIMITER,"(") &&
		parser_assignment_expression(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,";") &&
		parser_bool_expression(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,";") &&
		parser_assignment_expression(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,")") &&
		parser_statement(parser);
}

static bool parser_compound_stat(Parser *parser) {
	return parser_expect(parser,TOKEN_DELIMITER,"{") &&
		parser_statement_list(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,"}");
}

typedef bool (*StatHandler)(Parser *);
typedef struct{
	const char *keyword;
	StatHandler handler;
} KeywordEntry;
static const KeywordEntry keyword_table[] = {
	{"if",	  parser_if_stat},
	{"while", parser_while_stat},
	{"for",   parser_for_stat},
	{"read",  parser_read_stat},
	{"write", parser_write_stat},
};

static bool parser_statement(Parser *parser) {
	const Token *token = parser_peek(parser);
	if(token == NULL) return parser_fail(parser,"statement");

	if(token->type == TOKEN_KEYWORD) {
		for(size_t i = 0; i < ARRLEN(keyword_table); i++) {
			if(strcmp(token->lexeme,keyword_table[i].keyword) == 0)
				return keyword_table[i].handler(parser);
		}
	}

	if(token->type == TOKEN_IDENTIFIER) return parser_assignment_stat(parser);

	if(token->type == TOKEN_DELIMITER) {
		if(strcmp(token->lexeme,"{") == 0)
			return parser_compound_stat(parser);
		if(strcmp(token->lexeme,";") == 0)
			return parser_expect(parser,TOKEN_DELIMITER,";");
	}

	return parser_fail(parser,"statement");
}

static bool parser_statement_list(Parser *parser) {
	while(true) {
		const Token *token = parser_peek(parser);
		if(token == NULL) return true;
		if(token->type == TOKEN_DELIMITER &&
			strcmp(token->lexeme,"}") == 0)
			return true;
		if(!parser_statement(parser)) return false;
	}
}

static bool parser_declaration_list(Parser *parser) {
	while(true) {
		const Token *token = parser_peek(parser);
		if(token == NULL ||
			token->type != TOKEN_KEYWORD ||
			strcmp(token->lexeme,"int") != 0)
			return true;
		if(!parser_declaration_stat(parser)) return false;
	}
}

bool parser_program(Parser *parser) {
	if(!parser_expect(parser,TOKEN_DELIMITER,"{") ||
		!parser_declaration_list(parser) ||
		!parser_statement_list(parser) ||
		!parser_expect(parser,TOKEN_DELIMITER,"}"))
		return false;
	if(parser_peek(parser) != NULL)
		return parser_fail(parser,"end of input");
	return true;
}
