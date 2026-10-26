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

	lexer_scan_file(fp);
	
	fclose(fp);

	return 0;
}
