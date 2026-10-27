#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define MAX_TOKEN_LEN 128

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

void lexer_scan_file(FILE *fp);

#endif
