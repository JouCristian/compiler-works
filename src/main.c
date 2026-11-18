#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "lexer.h"
#include "ansi.h"
#include "parser.h"
#include "ir.h"
#include "semantic.h"

#define IR_OUTPUT_PATH "docs/semantic-analysis/results/quadruples.txt"

static void print_usage(const char *prog) {
	fprintf(stderr,
		ANSI_FMT("[USAGE]: %s <source> [--all-tokens]\n",ANSI_FG_CYAN),
		prog);
}

static bool print_operand(FILE *out, Operand operand, const SymbolTable *symbols) {
	switch(operand.kind) {
	case OPERAND_NONE:
		return fputc('_',out) != EOF;
	case OPERAND_IMMEDIATE:
		return fprintf(out,"%d",operand.data.value) >= 0;
	case OPERAND_VARIABLE:
		if(operand.data.index < symbols->count)
			return fputs(symbols->symbols[operand.data.index].name,out) != EOF;
		break;
	case OPERAND_TEMP:
		return fprintf(out,"t%zu",operand.data.index) >= 0;
	case OPERAND_LABEL:
		return fprintf(out,"L%zu",operand.data.index) >= 0;
	}
	errno = EINVAL;
	return false;
}

static bool print_ir(FILE *out, const SemanticContext *semantic) {
	static const char *const ops[] = {
		[IR_ADD] = "+", [IR_SUB] = "-", [IR_MULT] = "*", [IR_DIV] = "/", [IR_ASSIGN] = "=",
		[IR_GT] = ">", [IR_LT] = "<", [IR_GET] = ">=", [IR_LET] = "<=", [IR_EQ] = "==", [IR_NEQ] = "!=",
		[IR_READ] = "read", [IR_WRITE] = "write", [IR_BR] = "br", [IR_BRT] = "brt",
		[IR_BRF] = "brf", [IR_LABEL] = "label"
	};
	for(size_t i = 0; i < semantic->irs.count; i++) {
		const IRquad *quad = &semantic->irs.quads[i];
		size_t op = (size_t)quad->op;
		if(op >= ARRLEN(ops) || ops[op] == NULL) {
			errno = EINVAL;
			return false;
		}
		if(fprintf(out,"%04zu (%s, ",i,ops[op]) < 0 ||
		   !print_operand(out,quad->opr1,&semantic->symbols) || fputs(", ",out) == EOF ||
		   !print_operand(out,quad->opr2,&semantic->symbols) || fputs(", ",out) == EOF ||
		   !print_operand(out,quad->result,&semantic->symbols) || fputs(")\n",out) == EOF)
			return false;
	}
	return true;
}

static bool save_ir(const SemanticContext *semantic) {
	char path[] = IR_OUTPUT_PATH ".XXXXXX";
	int fd = mkstemp(path);
	if(fd < 0) {
		perror("create " IR_OUTPUT_PATH);
		return false;
	}
	FILE *out = fdopen(fd,"w");
	if(out == NULL) {
		perror("open " IR_OUTPUT_PATH);
		close(fd);
		remove(path);
		return false;
	}
	bool written = print_ir(out,semantic);
	if(!written) perror("write " IR_OUTPUT_PATH);
	if(fclose(out) == EOF) {
		if(written) perror("close " IR_OUTPUT_PATH);
		written = false;
	}
	if(written && rename(path,IR_OUTPUT_PATH) == 0) return true;
	if(written) perror("replace " IR_OUTPUT_PATH);
	remove(path);
	return false;
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

	FILE *lex_out = fopen("docs/lexical-analysis/lex.txt","w");
	if(lex_out == NULL) {
		perror("fopen docs/lexical-analysis/lex.txt");
		fclose(fp);
		return EXIT_FAILURE;
	}

	TokenBuffer buffer;
	token_buffer_init(&buffer);

	SemanticContext semantic = {0};
	symbol_table_init(&semantic.symbols);
	ir_table_init(&semantic.irs);

	LexerState state = {0};
	int exit_code = EXIT_SUCCESS;

	bool scanned = lexer_scan_file(fp,lex_out,mode,&buffer,&state);
	if(ferror(fp)) {
		fprintf(stderr,"Cannot read source file '%s'\n",argv[1]);
		exit_code = EXIT_FAILURE;
		goto finish;
	}
	if(!scanned) {
		fprintf(stderr,
			ANSI_FMT("Memory Out: max memory size %d tokens\n", ANSI_FG_RED),
			TB_MAX_LEN);
		exit_code = EXIT_FAILURE;
		goto finish;
	}
	if(fflush(lex_out) == EOF || ferror(lex_out)) {
		fprintf(stderr,"Cannot write docs/lexical-analysis/lex.txt\n");
		exit_code = EXIT_FAILURE;
		goto finish;
	}
	printf(ANSI_FMT("\nNotice: %u valid tokens has been writen into \"docs/lexical-analysis/lex.txt\",check it!\n",ANSI_FG_CYAN),
			state.token_count);
	if(state.error_count > 0) {
		fprintf(stderr,
			ANSI_FMT("Parser Skipped: %u lexical errors\n",ANSI_FG_RED),
			state.error_count);
		exit_code = EXIT_FAILURE;
		goto finish;
	}

	Parser parser;
	parser_init(&parser,buffer.tokens,buffer.count,&semantic);
	printf(ANSI_FMT("Parser received %zu tokens!\n",ANSI_FG_GREEN),parser.count);

	if(!parser_program(&parser)) {
		if(semantic.resource_failed) {
			fprintf(stderr,
				ANSI_FMT("Semantic analysis failed: out of memory or symbol table capacity exhausted\n",ANSI_FG_RED));
			exit_code = EXIT_FAILURE;
			goto finish;
		}
		fprintf(stderr,
			ANSI_FMT("Grammar error at line %d: %s\n",ANSI_FG_RED),
			parser.error_line,
			parser.error_message);

		FILE *error_out = fopen("docs/syntax-analysis/results/syntax-errors.txt", "a");
		if(error_out == NULL) {
			perror("fopen docs/syntax-analysis/results/syntax-errors.txt");
		} else {
			fprintf(error_out, "%s:%d: %s\n",
				argv[1], parser.error_line, parser.error_message);
			fclose(error_out);
		}
		exit_code = EXIT_FAILURE;
		goto finish;
	}

	printf("\n==========Symbol Table==========\n");
	for(size_t i = 0; i < semantic.symbols.count; i ++) {
		Symbol symbol = semantic.symbols.symbols[i];
		printf("[%zu] %s, initialized = %s, line = %d\n",
			i,
			symbol.name,
			symbol.initialized ? "true" : "false",
			symbol.declaration_line);
	}
	printf("Semantic Errors: %zu\n",semantic.error_count);
	if(semantic.error_count > 0) {
		exit_code = EXIT_FAILURE;
		goto finish;
	}

	parser_init_mode(&parser,buffer.tokens,buffer.count,&semantic,PARSE_IR);
	if(!parser_program(&parser)) {
		if(semantic.resource_failed)
			fprintf(stderr,ANSI_FMT("IR generation failed: out of memory or IR capacity exhausted\n",ANSI_FG_RED));
		else if(parser.error_line > 0)
			fprintf(stderr,ANSI_FMT("IR generation error at line %d: %s\n",ANSI_FG_RED),
				parser.error_line,parser.error_message);
		else
			fprintf(stderr,ANSI_FMT("IR generation error: %s\n",ANSI_FG_RED),parser.error_message);
		exit_code = EXIT_FAILURE;
		goto finish;
	}
	if(!save_ir(&semantic)) {
		exit_code = EXIT_FAILURE;
		goto finish;
	}
	if(fputs("\n==========Quadruples==========\n",stdout) == EOF ||
	   !print_ir(stdout,&semantic) || fflush(stdout) == EOF) {
		perror("write IR to stdout");
		exit_code = EXIT_FAILURE;
		goto finish;
	}
	if(printf(ANSI_FMT("Grammar analysis successfully!\n",ANSI_FG_GREEN)) < 0 ||
	   printf("IR saved to %s\n",IR_OUTPUT_PATH) < 0 || fflush(stdout) == EOF) {
		perror("write status to stdout");
		exit_code = EXIT_FAILURE;
	}

finish:
	token_buffer_free(&buffer);
	symbol_table_free(&semantic.symbols);
	ir_table_free(&semantic.irs);
	
	fclose(fp);
	fclose(lex_out);

	return exit_code;
}
