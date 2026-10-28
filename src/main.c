#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("[Error Usage]: %s <source file>\n",argv[0]);
		return 1;
	}

	FILE *fp = fopen(argv[1],"r");
	if (fp == NULL) {
		perror("fopen");
		return 1;
	}

	FILE *lex_out = fopen("docs/lex.txt","w");
	if(lex_out == NULL) {
		perror("fopen docs/lex.txt");
		fclose(fp);
		return 1;
	}

	lexer_scan_file(fp,lex_out);
	
	fclose(fp);

	if(fclose(lex_out) == EOF) {
		perror("fclose docs/lex.txt");
		return 1;
	}

	return 0;
}
