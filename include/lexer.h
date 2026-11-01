#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_TOKEN_LEN 128
#define TB_MAX_LEN 32768

typedef enum {
	OUTPUT_ERRORS_ONLY,
	OUTPUT_ALL_TOKENS
} TerminalMode;

typedef enum {
	TOKEN_KEYWORD,
	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,
	TOKEN_OPERATOR,
	TOKEN_DELIMITER,
	TOKEN_WRONG
} TokenType;

typedef struct {
	TokenType type;
	char lexeme[MAX_TOKEN_LEN];
	int line;
} Token;

typedef struct {
	unsigned int token_count;
	unsigned int keyword_count;
	unsigned int error_count;
	unsigned int error_lines;
	int last_error_line;
} LexerState;

typedef struct {
	Token *token;
	size_t count;
	size_t capacity;
} TokenBuffer;

void lexer_scan_file(FILE *fp, FILE *lex_out, TerminalMode mode, TokenBuffer *token, LexerState *state);

void token_buffer_init(TokenBuffer *buffer);

bool token_buffer_push(TokenBuffer *buffer, const Token *token);

void token_buffer_free(TokenBuffer *buffer);

#endif
