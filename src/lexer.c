#include "lexer.h"
#include "ansi.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static const char *const keywords[] = {
	"if", "else", "for", "while",
	"int", "write", "read"
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

static bool is_keyword(const char* word) {
	size_t count = sizeof(keywords) / sizeof(keywords[0]);
	for(size_t i = 0; i < count; i ++) {
		if(strcmp(word,keywords[i]) == 0) {
			return true;
		}
	}
	return false;
}

void lexer_scan_file(FILE *fp) {
	int ch;
	int line = 1;

	while((ch = fgetc(fp)) != EOF) {
		if((unsigned char)ch == '\n') {
			line++;
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
				fprintf(stderr,ANSI_FMT("[ERROR]",ANSI_FG_RED)":The token is too long!\n");
			} else {
				token.type = is_keyword(token.lexeme)
					? TOKEN_KEYWORD
					: TOKEN_IDENTIFIER;

				//Test for Debug
				printf("%s[%s]"ANSI_NONE"\t%s\tline(%d)\n",
						typecolors[token.type],
						tokentypes[token.type],
						token.lexeme,
						token.line);
			}
		}		
		else if(isdigit((unsigned char)ch)) {
			Token token = {0};
			size_t len = 0;
			bool overflow = false;
			bool leading_zero = (ch == '0');

			token.type = TOKEN_NUMBER;
			token.line = line;
			token.lexeme[len++] = (char)ch;
			while(((ch = fgetc(fp)) != EOF) &&
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

			if(overflow) {
				fprintf(stderr,ANSI_FMT("[ERROR]",ANSI_FG_RED)":The token is too long!\n");
			} else if(token.type == TOKEN_WRONG) {
				fprintf(stderr,ANSI_FMT("[WRONG TOKEN]",ANSI_FG_RED)":%s can't be identified! line(%d)\n",
						token.lexeme,token.line);
			} else if(leading_zero && (len > 1)) {
				fprintf(stderr,ANSI_FMT("[WRONG TOKEN]",ANSI_FG_RED)":%s can't be identified! line(%d)\n",
						token.lexeme,token.line);
			} else {
				//Test for debug
				printf("%s[%s]"ANSI_NONE"\t%s\tline(%d)\n",
						typecolors[token.type],
						tokentypes[token.type],
						token.lexeme,
						token.line);
			}
		}
	}
}
