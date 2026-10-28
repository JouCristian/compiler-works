#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ansi.h"

static void print_usage(const char *prog) {
	fprintf(stderr,
		ANSI_FMT("[USAGE]: %s <source> [--all-tokens]\n",ANSI_FG_CYAN),
		prog);
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr,
			ANSI_FMT("[ERROR]:%s should be follow with a file for testing!\n",ANSI_FG_RED),
			argv[0]);
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	if(argc > 3) {
		fprintf(stderr,
			ANSI_FMT("[ERROR]:Too many arguments!\n",ANSI_FG_RED));
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	TerminalMode mode = OUTPUT_ERRORS_ONLY;
	if(argc == 3) {
		if(strcmp(argv[2],"--all-tokens") == 0) {
			mode = OUTPUT_ALL_TOKENS;
		} else {
			fprintf(stderr,
				ANSI_FMT("The second argument should be \"--all-tokens\" or default ERRORS_ONLY!\n",ANSI_FG_RED));
			print_usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	FILE *fp = fopen(argv[1],"r");
	if (fp == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	FILE *lex_out = fopen("docs/lex.txt","w");
	if(lex_out == NULL) {
		perror("fopen docs/lex.txt");
		fclose(fp);
		return EXIT_FAILURE;
	}

	lexer_scan_file(fp,lex_out,mode);
	
	fclose(fp);

	if(fclose(lex_out) == EOF) {
		perror("fclose docs/lex.txt");
		return EXIT_FAILURE;
	}

	return 0;
}
