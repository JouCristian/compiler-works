#include "parser.h"
#include "common.h"
#include "ansi.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

//Announcement in advance
static bool parser_arithmetic_expression(Parser *parser, Operand *result);
static bool parser_statement(Parser *parser);
static bool parser_statement_list(Parser *parser);
static bool parser_declare_identifier(Parser *parser);

static bool parser_check_declared(Parser *parser,
				  const Token *token,
				  size_t *index);

void parser_init(Parser *parser,
		const Token *tokens,
		size_t count,
		SemanticContext *semantic) {
	parser_init_mode(parser,tokens,count,semantic,PARSE_CHECK);
}

void parser_init_mode(Parser *parser,
		const Token *tokens,
		size_t count,
		SemanticContext *semantic,
		ParseMode mode) {
	parser->tokens = tokens;
	parser->count = count;
	parser->pos = 0;
	parser->failed = false;
	parser->error_line = 0;
	parser->error_message[0] = '\0';
	parser->semantic = semantic;
	parser->mode = mode;
}

static bool parser_check_enabled(const Parser *parser) {
	return parser->mode == PARSE_CHECK && parser->semantic != NULL;
}

static bool parser_ir_enabled(const Parser *parser) {
	return parser->mode == PARSE_IR && parser->semantic != NULL;
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

static bool parser_operand_error(Parser *parser, const Token *token, const char *message) {
	parser->failed = true;
	parser->error_line = token->line;
	snprintf(parser->error_message,ERROR_MSG_LEN,"%s: '%.60s'",message,token->lexeme);
	return false;
}

static bool parser_factor(Parser *parser, Operand *result) {
	if(parser_match(parser,TOKEN_DELIMITER,"(")) {
		if(!parser_arithmetic_expression(parser,result)) return false;
		return parser_expect(parser,TOKEN_DELIMITER,")");
	}

	const Token *token = parser_peek(parser);

	if(parser_match(parser,TOKEN_IDENTIFIER,NULL)) {
		if(parser_check_enabled(parser)) {
			size_t index;
			if(parser_check_declared(parser,token,&index)) {
				if(!parser->semantic->symbols.symbols[index].initialized) {
					parser->semantic->error_count++;
					fprintf(stderr,
						ANSI_FMT("Semantic error at line %d: "
							 "Use of uninitialized variable '%s'\n",
							 ANSI_FG_RED),
						token->line,
						token->lexeme);
				}
			}
		} else if(parser_ir_enabled(parser)) {
			size_t index;
			if(!symbol_table_find(&parser->semantic->symbols,token->lexeme,&index))
				return parser_operand_error(parser,token,"Undeclared variable");
			*result = (Operand){.kind = OPERAND_VARIABLE, .data.index = index};
		}
		return true;
	}

	if(parser_match(parser,TOKEN_NUMBER,NULL)) {
		if(parser_ir_enabled(parser)) {
			char *end;
			errno = 0;
			long value = strtol(token->lexeme,&end,10);
			if(end == token->lexeme || *end != '\0')
				return parser_operand_error(parser,token,"Invalid integer literal");
			if(errno == ERANGE || value < INT_MIN || value > INT_MAX)
				return parser_operand_error(parser,token,"Integer literal out of int range");
			*result = (Operand){.kind = OPERAND_IMMEDIATE, .data.value = (int)value};
		}
		return true;
	}

	return parser_fail(parser,"'(',identifier or number");
}

static bool parser_emit_arithmetic(Parser *parser, IRop op, Operand *left, Operand right) {
	IRTable *table = &parser->semantic->irs;
	Operand temp = ir_new_temp(table);
	IRquad quad = {op, *left, right, temp};
	if(temp.kind == OPERAND_NONE || !ir_table_append(table,&quad)) {
		parser->semantic->resource_failed = true;
		return false;
	}
	*left = temp;
	return true;
}

static bool parser_term(Parser *parser, Operand *result) {
	if(!parser_factor(parser,result)) return false;

	while(true) {
		IRop op;
		if(parser_match(parser,TOKEN_OPERATOR,"*")) op = IR_MULT;
		else if(parser_match(parser,TOKEN_OPERATOR,"/")) op = IR_DIV;
		else break;

		Operand right;
		if(!parser_factor(parser,&right)) return false;
		if(parser_ir_enabled(parser) && !parser_emit_arithmetic(parser,op,result,right))
			return false;
	}

	return true;
}

static bool parser_arithmetic_expression(Parser *parser, Operand *result) {
	Operand unused;
	if(result == NULL) result = &unused;
	if(!parser_term(parser,result)) return false;

	while(true) {
		IRop op;
		if(parser_match(parser,TOKEN_OPERATOR,"+")) op = IR_ADD;
		else if(parser_match(parser,TOKEN_OPERATOR,"-")) op = IR_SUB;
		else break;

		Operand right;
		if(!parser_term(parser,&right)) return false;
		if(parser_ir_enabled(parser) && !parser_emit_arithmetic(parser,op,result,right))
			return false;
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
	return parser_arithmetic_expression(parser,NULL) &&
		parser_relop(parser) &&
		parser_arithmetic_expression(parser,NULL);
}

static bool parser_assignment_expression(Parser *parser) {
	const Token *target = parser_peek(parser);
	if(!parser_expect(parser,TOKEN_IDENTIFIER,NULL)) return false;
	if(parser_check_enabled(parser))
		parser_check_declared(parser,target,NULL);

	Operand right;
	if(!parser_expect(parser,TOKEN_OPERATOR,"=") ||
	   !parser_arithmetic_expression(parser,&right))
		return false;
	if(!parser_ir_enabled(parser)) return true;

	size_t index;
	if(!symbol_table_find(&parser->semantic->symbols,target->lexeme,&index))
		return parser_operand_error(parser,target,"Undeclared variable");
	IRquad quad = {
		IR_ASSIGN, right, {.kind = OPERAND_NONE},
		{.kind = OPERAND_VARIABLE, .data.index = index}
	};
	if(!ir_table_append(&parser->semantic->irs,&quad)) {
		parser->semantic->resource_failed = true;
		return false;
	}
	return true;
}

bool parser_assignment_stat(Parser *parser) {
	const Token *target = parser_peek(parser);
	bool checking = parser_check_enabled(parser);
	size_t errors_before = checking
		? parser->semantic->error_count
		: 0;
	if(!parser_assignment_expression(parser) ||
	   !parser_expect(parser,TOKEN_DELIMITER,";"))
		return false;
	if(checking &&
	   parser->semantic->error_count == errors_before) {
		size_t index;
		if(symbol_table_find(&parser->semantic->symbols,
					target->lexeme,&index)) {
			parser->semantic->symbols.symbols[index].initialized = true;
		}
	}
	return true;
}

static bool parser_read_stat(Parser *parser) {
	if(parser_expect(parser,TOKEN_KEYWORD,"read")) {
		const Token *token = parser_peek(parser);
		if(parser_expect(parser,TOKEN_IDENTIFIER,NULL)) {
			size_t index = 0;
			bool declared = false;
			if(parser_check_enabled(parser))
				declared = parser_check_declared(parser,token,&index);
			if(parser_expect(parser,TOKEN_DELIMITER,";")) {
				if(declared)
					parser->semantic->symbols.symbols[index].initialized = true;
				return true;
			}
		}
		return false;
	}
	return false;
}

static bool parser_write_stat(Parser *parser) {
	return parser_expect(parser,TOKEN_KEYWORD,"write") &&
		parser_arithmetic_expression(parser,NULL) &&
		parser_expect(parser,TOKEN_DELIMITER,";");
}

static bool parser_variable_list(Parser *parser) {
	while(parser_match(parser,TOKEN_DELIMITER,",")) {
		if(!parser_declare_identifier(parser)) return false;
	}
	return true;
}

static bool parser_declaration_stat(Parser *parser) {
	return parser_expect(parser,TOKEN_KEYWORD,"int") &&
		parser_declare_identifier(parser) &&
		parser_variable_list(parser) &&
		parser_expect(parser,TOKEN_DELIMITER,";");
}

static bool parser_if_stat(Parser *parser) {
	if(!parser_expect(parser,TOKEN_KEYWORD,"if") ||
	   !parser_expect(parser,TOKEN_DELIMITER,"(") ||
	   !parser_bool_expression(parser) ||
	   !parser_expect(parser,TOKEN_DELIMITER,")"))
		return false;
	SymbolTable *table = parser_check_enabled(parser)
		? &parser->semantic->symbols : NULL;
	size_t count = table != NULL ? table->count : 0;
	bool *before = NULL;
	bool *after_then = NULL;
	if(count > 0) {
		before = malloc(count * sizeof(*before));
		after_then = malloc(count * sizeof(*after_then));
		if(before == NULL || after_then == NULL) {
			free(before);
			free(after_then);
			parser->semantic->resource_failed = true;
			return false;
		}
		for(size_t i = 0; i < count; i++) {
			before[i] = table->symbols[i].initialized;
		}
	}
	//analysis Then branch
	if(!parser_statement(parser)) {
		free(before);
		free(after_then);
		return false;
	}

	for(size_t i = 0; i < count; i++) {
		after_then[i] = table->symbols[i].initialized;
		table->symbols[i].initialized = before[i];
	}

	//Else branch
	if(parser_match(parser,TOKEN_KEYWORD,"else")) {
		if(!parser_statement(parser)) {
			free(before);
			free(after_then);
			return false;
		}

		for(size_t i = 0; i < count; i++) {
			table->symbols[i].initialized = 
				after_then[i] && table->symbols[i].initialized;
		}
	}

	free(before);
	free(after_then);
	return true;
}

static bool parser_while_stat(Parser *parser) {
	if(!parser_expect(parser,TOKEN_KEYWORD,"while") ||
	   !parser_expect(parser,TOKEN_DELIMITER,"(") ||
	   !parser_bool_expression(parser) ||
	   !parser_expect(parser,TOKEN_DELIMITER,")"))
		return false;
	SymbolTable *table = parser_check_enabled(parser)
		? &parser->semantic->symbols : NULL;
	size_t count = table != NULL ? table->count : 0;
	bool *before = NULL;
	if(count > 0) {
		before = malloc(count * sizeof(*before));
		if(before == NULL) {
			parser->semantic->resource_failed = true;
			return false;
		}
		for(size_t i = 0; i < count; i++) {
			before[i] = table->symbols[i].initialized;
		}
	}
	bool success = parser_statement(parser);
	for(size_t i = 0; i < count; i++) {
		table->symbols[i].initialized = before[i];
	}
	free(before);
	return success;
}

static bool parser_for_stat(Parser *parser) {
	if(!parser_expect(parser,TOKEN_KEYWORD,"for") ||
	   !parser_expect(parser,TOKEN_DELIMITER,"(") ||
	   !parser_assignment_stat(parser) ||
	   !parser_bool_expression(parser) ||
	   !parser_expect(parser,TOKEN_DELIMITER,";"))
		return false;
	size_t update_pos = parser->pos;
	SemanticContext *semantic = parser->semantic;
	// The first look at the update checks syntax only.
	parser->semantic = NULL;
	bool update_ok = parser_assignment_expression(parser);
	parser->semantic = semantic;

	if(!update_ok ||
	   !parser_expect(parser,TOKEN_DELIMITER,")"))
		return false;
	SymbolTable *table = parser_check_enabled(parser)
		? &semantic->symbols : NULL;
	size_t count = table != NULL ? table->count : 0;
	bool *before = NULL;

	if(count > 0) {
		before = malloc(count * sizeof(*before));
		if(before == NULL) {
			semantic->resource_failed = true;
			return false;
		}
		for(size_t i = 0; i < count; i ++) {
			before[i] = table->symbols[i].initialized;
		}
	}
	bool success = parser_statement(parser);
	if(success) {
		size_t end_pos =  parser->pos;
		parser->pos = update_pos;
		success = parser_assignment_expression(parser);
		parser->pos = end_pos;
	}
	for(size_t i = 0; i < count; i++) {
		table->symbols[i].initialized = before[i];
	}
	free(before);
	return success;
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

static bool parser_parse_program(Parser *parser) {
	if(!parser_expect(parser,TOKEN_DELIMITER,"{") ||
		!parser_declaration_list(parser) ||
		!parser_statement_list(parser) ||
		!parser_expect(parser,TOKEN_DELIMITER,"}"))
		return false;
	if(parser_peek(parser) != NULL)
		return parser_fail(parser,"end of input");
	return true;
}

bool parser_program(Parser *parser) {
	if(!parser_ir_enabled(parser)) return parser_parse_program(parser);

	IRTable *table = &parser->semantic->irs;
	size_t count = table->count;
	size_t quad_count = table->quad_count;
	size_t temp_count = table->temp_count;
	if(parser_parse_program(parser)) return true;

	table->count = count;
	table->quad_count = quad_count;
	table->temp_count = temp_count;
	return false;
}

//Semantic analysis
static bool parser_declare_identifier(Parser *parser) {
	const Token *token = parser_peek(parser);
	if(!parser_expect(parser,TOKEN_IDENTIFIER,NULL)) return false;
	if(!parser_check_enabled(parser)) return true;

	SymbolInsertResult result = symbol_table_insert(
		&(parser->semantic->symbols),
		token->lexeme,
		token->line,
		NULL		
	);

	if(result == SYMBOL_SEMANTIC_WRONG) {
		parser->semantic->error_count++;
		fprintf(stderr,
			ANSI_FMT("Semantic error at line %d:"
				"Duplicate declaration of '%s'\n",ANSI_FG_RED),
			token->line,
			token->lexeme);
		return true;
	}
	else if(result == SYMBOL_NO_MEMORY) {
		parser->semantic->resource_failed = true;
		return false;
	}
	return true;
}

static bool parser_check_declared(Parser *parser,
				  const Token *token,
				  size_t *index) {
	if(!symbol_table_find(&parser->semantic->symbols,
				token->lexeme,index)) {
		parser->semantic->error_count++;
		fprintf(stderr,
			ANSI_FMT("Semantic error at line %d: "
				"Undeclared variables '%s' \n",
				ANSI_FG_RED),
			token->line,
			token->lexeme);
		return false;
	}
	return true;
}
