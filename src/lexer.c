#include "lexer.h"
#include "ansi.h"
#include "common.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static const char *const keywords[] = {
	"if", "else", "for", "while",
	"int", "write", "read"
};

static const int delimiters[] = {
	',','(',')',
	';','{','}'
};

static const int operator_starts[] = {
	'+','-','*',
	'=','>','<','!'
};
static const char *const tokentypes[] = {
	"KEYWORD","IDENTIFIER",
	"NUMBER","OPERATOR",
	"DELIMITER","WRONG"
};

static const char *const typecolors[] = {
	[TOKEN_KEYWORD]    = ANSI_FG_GREEN,
	[TOKEN_IDENTIFIER] = ANSI_FG_CYAN,
	[TOKEN_NUMBER]	   = ANSI_FG_YELLOW,
	[TOKEN_OPERATOR]   = ANSI_FG_BLUE,
	[TOKEN_DELIMITER]  = ANSI_FG_MAGENTA,
	[TOKEN_WRONG]      = ANSI_FG_RED
};

static void write_token(FILE *out, const Token *token) {
	if(token->type == TOKEN_WRONG) {
		return;
	}
	fprintf(out, "%-10s\t%-24s\t(%d)\n",
		tokentypes[token->type],
		token->lexeme,
		token->line);
}

static void print_token(FILE *out, 
		const bool overflow, 
		const Token *token) {

	const int column_width = 16;
	const char *label = tokentypes[token->type];
	int used = (int)strlen(label) + 2;
	int padding = column_width - used;

	fprintf(out,"%s[%s]" ANSI_NONE "%*s\t%*s%s\tline(%d)\n",
			typecolors[token->type],
			tokentypes[token->type],
			padding,"",
			overflow ? 20 : (int)strlen(token->lexeme),
			token->lexeme,
			overflow ? "..." : "",
			token->line);
}

static void emit_token(const Token *token,
	       	       const bool overflow,
	       	       LexerState *lexerstate,
		       FILE *lex_out,
		       TerminalMode mode) {

	if(token->type == TOKEN_WRONG) {
		lexerstate->error_count++;
		if(lexerstate->last_error_line != token->line) {
			lexerstate->error_lines++;
			lexerstate->last_error_line = token->line;
		}
	} else {
		lexerstate->token_count++;
		if(token->type == TOKEN_KEYWORD) {
			lexerstate->keyword_count++;
		}
	}
	FILE *out = (token->type == TOKEN_WRONG)
		? stderr
		: lex_out;
	if(out == stderr) {	
		print_token(out,overflow,token);
	} else {
		if(mode == OUTPUT_ALL_TOKENS) {
			print_token(stderr,overflow,token);
		}
		write_token(out,token);
	}
}

static void emit_lexer_statistics(const LexerState *lexerstate) {
	printf("\n===== Lexer Statistics =====\n");
	printf("Valid Tokens : %u\n", lexerstate->token_count);
	printf("Keywords     : %u\n", lexerstate->keyword_count);
	printf("Errors       : %u\n", lexerstate->error_count);
	printf("Error Lines  : %u\n", lexerstate->error_lines);	
}

static bool is_keyword(const char* word) {
	size_t count = ARRLEN(keywords);
	for(size_t i = 0; i < count; i ++) {
		if(strcmp(word,keywords[i]) == 0) {
			return true;
		}
	}
	return false;
}

static bool is_delimiter(int ch) {
	size_t count = ARRLEN(delimiters);
	for(size_t i = 0; i < count; i ++) {
		if(ch == delimiters[i]) {
			return true;
		}
	}
	return false;
}

static bool is_operator_start(int ch) {
	size_t count = ARRLEN(operator_starts);
	for(size_t i = 0; i < count; i ++) {
		if(ch == operator_starts[i]) {
			return true;
		}
	}
	return false;
}

static void skip_line_comment(FILE *fp, int *line) {
	int ch;
	while((ch = fgetc(fp)) != EOF) {
		if(ch == '\n') {
			(*line)++;
			return;
		}
	}
}

static bool skip_block_comment(FILE *fp, int *line) {
	int ch;
	int prev = 0;
	while((ch = fgetc(fp)) != EOF) {
		if(ch == '\n') {
			(*line)++;
		}
		if(prev == '*' && ch == '/') {
			return true;
		}
		prev = ch;
	}
	return false;
}

void lexer_scan_file(FILE *fp, FILE *lex_out, TerminalMode mode) {
	fprintf(lex_out,"%-10s\t%-24s\t%s\n",
		"type",
		"character",
		"line");
	int ch;
	int line = 1;
	LexerState lexerstate = {0};

	while((ch = fgetc(fp)) != EOF) {
		if((unsigned char)ch == '\n') {
			line++;
			continue;
		}
		if(isspace((unsigned char)ch)) {
			continue;
		}
		if(isalpha((unsigned char)ch)) {
			Token token = {0};
			size_t len = 0;
			bool overflow = false;
			token.line = line;

			token.lexeme[len++] = (char)ch;
			while((ch = fgetc(fp)) != EOF &&
					isalnum((unsigned char)ch)) {
				if(len < MAX_TOKEN_LEN - 1) {
					token.lexeme[len++] = (char)ch;
				} else {
					overflow = true;
				}
			}
			token.lexeme[len] = '\0';

			if(ch != EOF) {
				ungetc(ch,fp);
			}

			if(overflow) {
				token.type = TOKEN_WRONG;
			} else {
				token.type = is_keyword(token.lexeme)
					? TOKEN_KEYWORD
					: TOKEN_IDENTIFIER;
			}

			emit_token(&token,overflow,&lexerstate,lex_out,mode);
		}		
		else if(isdigit((unsigned char)ch)) {
			Token token = {0};
			size_t len = 0;
			bool overflow = false;
			bool leading_zero = (ch == '0');

			token.type = TOKEN_NUMBER;
			token.line = line;
			token.lexeme[len++] = (char)ch;
			while((ch = fgetc(fp)) != EOF &&
					isalnum((unsigned char)ch)) {
				if(len < MAX_TOKEN_LEN - 1) {
					if(isalpha((unsigned char)ch)) {
						token.type = TOKEN_WRONG;
					}
					token.lexeme[len++] = (char)ch;
				} else {
					overflow = true;
				}
			}
			token.lexeme[len] = '\0';
			if(ch != EOF) {
				ungetc(ch,fp);
			}

			if(overflow || (leading_zero && (len > 1))) {
				token.type = TOKEN_WRONG;
			}

			emit_token(&token,overflow,&lexerstate,lex_out,mode);
		}
		else if(is_delimiter(ch)) {
			Token token = {0};
			size_t len = 0;
			token.type = TOKEN_DELIMITER;
			token.line = line;
			token.lexeme[len++] = (char)ch;
			token.lexeme[len] = '\0';

			emit_token(&token,false,&lexerstate,lex_out,mode);
		}
		else if(is_operator_start(ch)) {
			Token token = {0};
			token.type = TOKEN_OPERATOR;
			token.line = line;
			token.lexeme[0] = (char)ch;
			token.lexeme[1] = '\0';

			if(ch == '>' || ch == '<' ||
			   ch == '=' || ch == '!') {
				int next = fgetc(fp);
				if(next == '=') {
					token.lexeme[1] = '=';
					token.lexeme[2] = '\0';
				} else {
					if(next != EOF) {
						ungetc(next,fp);
					}
					if(ch == '!') {
						token.type = TOKEN_WRONG;
					}
				}
			}

			emit_token(&token,false,&lexerstate,lex_out,mode);
		}
		else if(ch == '/') {
			int next = fgetc(fp);
			if(next == '/') {
				skip_line_comment(fp,&line);
			} else if(next == '*') {
				int start_line = line;
				if(!skip_block_comment(fp,&line)) {
					Token token = {0};
					token.type = TOKEN_WRONG;
					token.line = start_line;
					token.lexeme[0] = '/';
					token.lexeme[1] = '*';

					emit_token(&token,false,&lexerstate,lex_out,mode);
					fprintf(stderr,
						"Unterminated comment at line %d\n",
						start_line);
				}
			} else {
				Token token = {0};
				token.type = TOKEN_OPERATOR;
				token.line = line;
				token.lexeme[0] = (char)ch;
				token.lexeme[1] = '\0';
				if(next != EOF) {
					ungetc(next,fp);
				}

				emit_token(&token,false,&lexerstate,lex_out,mode);
			}
		}
		else {
			Token token = {0};
			token.type = TOKEN_WRONG;
			token.line = line;
			token.lexeme[0] = (char)ch;
			token.lexeme[1] = '\0';

			emit_token(&token,false,&lexerstate,lex_out,mode);
		}
	}
	emit_lexer_statistics(&lexerstate);
}
