#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "parser.h"

#define IMM(n) { .kind = OPERAND_IMMEDIATE, .data.value = (n) }
#define VAR(n) { .kind = OPERAND_VARIABLE, .data.index = (n) }
#define TEMP(n) { .kind = OPERAND_TEMP, .data.index = (n) }
#define NONE { .kind = OPERAND_NONE }

typedef struct {
	const char *name;
	const char *source;
	const IRquad *quads;
	size_t count;
	size_t temp_count;
	int error_line;
} ArithmeticTest;

static bool same_operand(Operand actual, Operand expected) {
	if(actual.kind != expected.kind) return false;
	if(expected.kind == OPERAND_NONE) return true;
	if(expected.kind == OPERAND_IMMEDIATE) return actual.data.value == expected.data.value;
	return actual.data.index == expected.data.index;
}

static bool check_quads(const ArithmeticTest *test, const IRTable *table) {
	if(table->count != test->count || table->quad_count != test->count ||
	   table->temp_count != test->temp_count || table->label_count != 0) {
		fprintf(stderr,"FAIL: %s: expected %zu quads and %zu temps, got %zu quads, %zu temps, %zu labels\n",
			test->name,test->count,test->temp_count,table->count,table->temp_count,table->label_count);
		return false;
	}
	for(size_t i = 0; i < test->count; i++) {
		const IRquad *actual = &table->quads[i];
		const IRquad *expected = &test->quads[i];
		if(actual->op != expected->op ||
		   !same_operand(actual->opr1,expected->opr1) ||
		   !same_operand(actual->opr2,expected->opr2) ||
		   !same_operand(actual->result,expected->result)) {
			fprintf(stderr,"FAIL: %s: unexpected operator or operand in quad %zu\n",test->name,i);
			return false;
		}
	}
	return true;
}

static bool run_test(const ArithmeticTest *test) {
	FILE *source = NULL;
	FILE *lex_out = NULL;
	TokenBuffer buffer;
	LexerState state = {0};
	SemanticContext semantic = {0};
	bool success = false;
	token_buffer_init(&buffer);

	source = tmpfile();
	if(source == NULL) {
		perror("tmpfile source");
		goto finish;
	}
	lex_out = tmpfile();
	if(lex_out == NULL) {
		perror("tmpfile lexer output");
		goto finish;
	}
	if(fputs(test->source,source) == EOF || fseek(source,0,SEEK_SET) != 0) {
		perror("prepare test source");
		goto finish;
	}
	if(!lexer_scan_file(source,lex_out,OUTPUT_ERRORS_ONLY,&buffer,&state) ||
	   state.error_count != 0) {
		fprintf(stderr,"FAIL: %s: lexer failed\n",test->name);
		goto finish;
	}

	Parser parser;
	parser_init(&parser,buffer.tokens,buffer.count,&semantic);
	if(!parser_program(&parser) || semantic.error_count != 0 || semantic.resource_failed ||
	   semantic.irs.count != 0 || semantic.irs.temp_count != 0) {
		fprintf(stderr,"FAIL: %s: PARSE_CHECK should accept the program without generating IR\n",test->name);
		goto finish;
	}

	parser_init_mode(&parser,buffer.tokens,buffer.count,&semantic,PARSE_IR);
	bool parsed = parser_program(&parser);
	if(test->error_line != 0) {
		if(parsed || !parser.failed || parser.error_line != test->error_line ||
		   strstr(parser.error_message,"Integer literal out of int range") == NULL) {
			fprintf(stderr,"FAIL: %s: expected out-of-range literal at line %d, got line %d: %s\n",
				test->name,test->error_line,parser.error_line,parser.error_message);
			goto finish;
		}
	} else if(!parsed || parser.failed || parser.pos != buffer.count) {
		fprintf(stderr,"FAIL: %s: PARSE_IR failed at line %d: %s\n",
			test->name,parser.error_line,parser.error_message);
		goto finish;
	}
	if(semantic.error_count != 0 || semantic.resource_failed) {
		fprintf(stderr,"FAIL: %s: unexpected semantic or resource error\n",test->name);
		goto finish;
	}
	success = check_quads(test,&semantic.irs);

finish:
	if(source != NULL) fclose(source);
	if(lex_out != NULL) fclose(lex_out);
	token_buffer_free(&buffer);
	symbol_table_free(&semantic.symbols);
	ir_table_free(&semantic.irs);
	return success;
}

int main(void) {
	const IRquad assignment[] = {{IR_ASSIGN, IMM(42), NONE, VAR(1)}};
	const IRquad precedence[] = {
		{IR_MULT, IMM(3), IMM(4), TEMP(0)},
		{IR_ADD, VAR(1), TEMP(0), TEMP(1)},
		{IR_DIV, IMM(8), IMM(2), TEMP(2)},
		{IR_SUB, TEMP(1), TEMP(2), TEMP(3)},
		{IR_ASSIGN, TEMP(3), NONE, VAR(1)}
	};
	const IRquad consecutive[] = {
		{IR_ASSIGN, IMM(7), NONE, VAR(0)},
		{IR_ASSIGN, VAR(0), NONE, VAR(1)},
		{IR_ADD, VAR(1), IMM(1), TEMP(0)},
		{IR_ASSIGN, TEMP(0), NONE, VAR(0)}
	};
	const IRquad parentheses[] = {
		{IR_ADD, IMM(2), IMM(3), TEMP(0)},
		{IR_MULT, TEMP(0), IMM(4), TEMP(1)},
		{IR_SUB, IMM(8), IMM(6), TEMP(2)},
		{IR_DIV, TEMP(1), TEMP(2), TEMP(3)},
		{IR_DIV, TEMP(3), IMM(2), TEMP(4)}
	};
	const IRquad loop[] = {
		{IR_ADD, IMM(1), IMM(2), TEMP(0)},
		{IR_ASSIGN, TEMP(0), NONE, VAR(0)},
		{IR_ADD, VAR(0), IMM(1), TEMP(1)},
		{IR_MULT, IMM(5), IMM(2), TEMP(2)},
		{IR_MULT, VAR(0), IMM(4), TEMP(3)},
		{IR_ADD, VAR(0), IMM(3), TEMP(4)},
		{IR_ASSIGN, TEMP(4), NONE, VAR(0)}
	};
	const IRquad int_max[] = {{IR_ADD, IMM(INT_MAX), IMM(0), TEMP(0)}};
	char limit_source[2 * MAX_TOKEN_LEN];
	char above_int_source[2 * MAX_TOKEN_LEN];
	char above_long_source[2 * MAX_TOKEN_LEN];
	snprintf(limit_source,sizeof(limit_source),"{ write %d + 0; }",INT_MAX);
	snprintf(above_int_source,sizeof(above_int_source),
		"{\nwrite 1 + 2;\nwrite %u;\n}",(unsigned int)INT_MAX + 1U);
	snprintf(above_long_source,sizeof(above_long_source),"{\nwrite %ld0;\n}",LONG_MAX);

	const ArithmeticTest tests[] = {
		{"simple assignment uses the target's symbol index without a temporary",
		 "{ int unused, a; a = 42; }", assignment, ARRLEN(assignment), 0, 0},
		{"arithmetic assignment uses the final temporary",
		 "{ int unused, a; read a; a = a + 3 * 4 - 8 / 2; write a; }",
		 precedence, ARRLEN(precedence), 4, 0},
		{"consecutive assignments preserve statement order and temporary numbering",
		 "{ int a, b; a = 7; b = a; a = b + 1; }", consecutive, ARRLEN(consecutive), 1, 0},
		{"parentheses and left associativity",
		 "{ write (2 + 3) * 4 / (8 - 6) / 2; }", parentheses, ARRLEN(parentheses), 5, 0},
		{"for initializer and update each assign once, with update after the body",
		 "{ int i; for (i = 1 + 2; i + 1 < 5 * 2; i = i + 3) write i * 4; }",
		 loop, ARRLEN(loop), 5, 0},
		{"INT_MAX is preserved", limit_source, int_max, ARRLEN(int_max), 1, 0},
		{"INT_MAX + 1 is rejected and partial IR is discarded", above_int_source, NULL, 0, 0, 3},
		{"strtol overflow is rejected", above_long_source, NULL, 0, 0, 2}
	};

	size_t failures = 0;
	for(size_t i = 0; i < ARRLEN(tests); i++) {
		if(run_test(&tests[i])) {
			printf("PASS: %s\n",tests[i].name);
		} else {
			fprintf(stderr,"FAIL: %s\n",tests[i].name);
			failures++;
		}
	}
	printf("Expression IR tests: %zu/%zu passed\n",ARRLEN(tests) - failures,ARRLEN(tests));
	return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
