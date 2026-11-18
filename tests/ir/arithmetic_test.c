#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "parser.h"

#define IMM(n) { .kind = OPERAND_IMMEDIATE, .data.value = (n) }
#define VAR(n) { .kind = OPERAND_VARIABLE, .data.index = (n) }
#define TEMP(n) { .kind = OPERAND_TEMP, .data.index = (n) }
#define LABEL(n) { .kind = OPERAND_LABEL, .data.index = (n) }
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
	size_t labels = 0;
	for(size_t i = 0; i < test->count; i++) {
		if(test->quads[i].op == IR_LABEL) labels++;
	}
	if(table->count != test->count || table->quad_count != test->count - labels ||
	   table->temp_count != test->temp_count || table->label_count != labels ||
	   table->next_label != labels || !ir_table_validate(table)) {
		fprintf(stderr,"FAIL: %s: unexpected IR count, label state, or branch targets\n",test->name);
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
	   semantic.irs.count != 0 || semantic.irs.temp_count != 0 || semantic.irs.next_label != 0) {
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
		{IR_READ, NONE, NONE, VAR(1)},
		{IR_MULT, IMM(3), IMM(4), TEMP(0)},
		{IR_ADD, VAR(1), TEMP(0), TEMP(1)},
		{IR_DIV, IMM(8), IMM(2), TEMP(2)},
		{IR_SUB, TEMP(1), TEMP(2), TEMP(3)},
		{IR_ASSIGN, TEMP(3), NONE, VAR(1)},
		{IR_WRITE, VAR(1), NONE, NONE}
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
		{IR_DIV, TEMP(3), IMM(2), TEMP(4)},
		{IR_WRITE, TEMP(4), NONE, NONE}
	};
	const IRquad loop[] = {
		{IR_ADD, IMM(1), IMM(2), TEMP(0)},
		{IR_ASSIGN, TEMP(0), NONE, VAR(0)},
		{IR_LABEL, NONE, NONE, LABEL(0)},
		{IR_ADD, VAR(0), IMM(1), TEMP(1)},
		{IR_MULT, IMM(5), IMM(2), TEMP(2)},
		{IR_LT, TEMP(1), TEMP(2), TEMP(3)},
		{IR_BRF, TEMP(3), NONE, LABEL(1)},
		{IR_MULT, VAR(0), IMM(4), TEMP(4)},
		{IR_WRITE, TEMP(4), NONE, NONE},
		{IR_ADD, VAR(0), IMM(3), TEMP(5)},
		{IR_ASSIGN, TEMP(5), NONE, VAR(0)},
		{IR_BR, NONE, NONE, LABEL(0)},
		{IR_LABEL, NONE, NONE, LABEL(1)},
		{IR_WRITE, VAR(0), NONE, NONE}
	};
	const IRquad relations[] = {
		{IR_ADD, IMM(2), IMM(3), TEMP(0)},
		{IR_MULT, IMM(4), IMM(5), TEMP(1)},
		{IR_GT, TEMP(0), TEMP(1), TEMP(2)},
		{IR_BRF, TEMP(2), NONE, LABEL(0)},
		{IR_LABEL, NONE, NONE, LABEL(0)},
		{IR_LT, IMM(1), IMM(2), TEMP(3)},
		{IR_BRF, TEMP(3), NONE, LABEL(1)},
		{IR_LABEL, NONE, NONE, LABEL(1)},
		{IR_GET, IMM(1), IMM(2), TEMP(4)},
		{IR_BRF, TEMP(4), NONE, LABEL(2)},
		{IR_LABEL, NONE, NONE, LABEL(2)},
		{IR_LET, IMM(1), IMM(2), TEMP(5)},
		{IR_BRF, TEMP(5), NONE, LABEL(3)},
		{IR_LABEL, NONE, NONE, LABEL(3)},
		{IR_EQ, IMM(1), IMM(2), TEMP(6)},
		{IR_BRF, TEMP(6), NONE, LABEL(4)},
		{IR_LABEL, NONE, NONE, LABEL(4)},
		{IR_NEQ, IMM(1), IMM(2), TEMP(7)},
		{IR_BRF, TEMP(7), NONE, LABEL(5)},
		{IR_LABEL, NONE, NONE, LABEL(5)}
	};
	const IRquad if_only[] = {
		{IR_LT, IMM(2), IMM(1), TEMP(0)},
		{IR_BRF, TEMP(0), NONE, LABEL(0)},
		{IR_WRITE, IMM(9), NONE, NONE},
		{IR_LABEL, NONE, NONE, LABEL(0)},
		{IR_WRITE, IMM(7), NONE, NONE}
	};
	const IRquad if_else[] = {
		{IR_EQ, IMM(1), IMM(0), TEMP(0)},
		{IR_BRF, TEMP(0), NONE, LABEL(0)},
		{IR_WRITE, IMM(1), NONE, NONE},
		{IR_BR, NONE, NONE, LABEL(1)},
		{IR_LABEL, NONE, NONE, LABEL(0)},
		{IR_WRITE, IMM(2), NONE, NONE},
		{IR_LABEL, NONE, NONE, LABEL(1)},
		{IR_WRITE, IMM(3), NONE, NONE}
	};
	const IRquad nested_if[] = {
		{IR_EQ, IMM(1), IMM(1), TEMP(0)},
		{IR_BRF, TEMP(0), NONE, LABEL(0)},
		{IR_NEQ, IMM(0), IMM(0), TEMP(1)},
		{IR_BRF, TEMP(1), NONE, LABEL(1)},
		{IR_WRITE, IMM(1), NONE, NONE},
		{IR_BR, NONE, NONE, LABEL(2)},
		{IR_LABEL, NONE, NONE, LABEL(1)},
		{IR_WRITE, IMM(2), NONE, NONE},
		{IR_LABEL, NONE, NONE, LABEL(2)},
		{IR_LABEL, NONE, NONE, LABEL(0)},
		{IR_WRITE, IMM(3), NONE, NONE}
	};
	const IRquad while_loop[] = {
		{IR_ASSIGN, IMM(2), NONE, VAR(0)},
		{IR_LABEL, NONE, NONE, LABEL(0)},
		{IR_LT, VAR(0), IMM(2), TEMP(0)},
		{IR_BRF, TEMP(0), NONE, LABEL(1)},
		{IR_ADD, VAR(0), IMM(1), TEMP(1)},
		{IR_ASSIGN, TEMP(1), NONE, VAR(0)},
		{IR_BR, NONE, NONE, LABEL(0)},
		{IR_LABEL, NONE, NONE, LABEL(1)},
		{IR_WRITE, VAR(0), NONE, NONE}
	};
	const IRquad nested_loops[] = {
		{IR_ASSIGN, IMM(0), NONE, VAR(0)},
		{IR_ASSIGN, IMM(0), NONE, VAR(1)},
		{IR_LABEL, NONE, NONE, LABEL(0)},
		{IR_LT, VAR(0), IMM(2), TEMP(0)},
		{IR_BRF, TEMP(0), NONE, LABEL(1)},
		{IR_LABEL, NONE, NONE, LABEL(2)},
		{IR_LT, VAR(1), IMM(3), TEMP(1)},
		{IR_BRF, TEMP(1), NONE, LABEL(3)},
		{IR_ADD, VAR(1), IMM(1), TEMP(2)},
		{IR_ASSIGN, TEMP(2), NONE, VAR(1)},
		{IR_BR, NONE, NONE, LABEL(2)},
		{IR_LABEL, NONE, NONE, LABEL(3)},
		{IR_ADD, VAR(0), IMM(1), TEMP(3)},
		{IR_ASSIGN, TEMP(3), NONE, VAR(0)},
		{IR_BR, NONE, NONE, LABEL(0)},
		{IR_LABEL, NONE, NONE, LABEL(1)}
	};
	const IRquad read[] = {{IR_READ, NONE, NONE, VAR(1)}};
	const IRquad write_immediate[] = {{IR_WRITE, IMM(42), NONE, NONE}};
	const IRquad io_sequence[] = {
		{IR_READ, NONE, NONE, VAR(1)},
		{IR_READ, NONE, NONE, VAR(0)},
		{IR_WRITE, VAR(0), NONE, NONE},
		{IR_WRITE, IMM(7), NONE, NONE},
		{IR_MULT, VAR(1), VAR(0), TEMP(0)},
		{IR_WRITE, TEMP(0), NONE, NONE}
	};
	const IRquad int_max[] = {
		{IR_ADD, IMM(INT_MAX), IMM(0), TEMP(0)},
		{IR_WRITE, TEMP(0), NONE, NONE}
	};
	char limit_source[2 * MAX_TOKEN_LEN];
	char above_int_source[2 * MAX_TOKEN_LEN];
	char above_long_source[2 * MAX_TOKEN_LEN];
	snprintf(limit_source,sizeof(limit_source),"{ write %d + 0; }",INT_MAX);
	snprintf(above_int_source,sizeof(above_int_source),
		"{\nif (1 < 2) write 1 + 2;\nwrite %u;\n}",(unsigned int)INT_MAX + 1U);
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
		 "{ int i; for (i = 1 + 2; i + 1 < 5 * 2; i = i + 3) write i * 4; write i; }",
		 loop, ARRLEN(loop), 6, 0},
		{"six relations use operands from both arithmetic expressions",
		 "{ if (2 + 3 > 4 * 5) ; if (1 < 2) ; if (1 >= 2) ;"
		 " if (1 <= 2) ; if (1 == 2) ; if (1 != 2) ; }", relations, ARRLEN(relations), 8, 0},
		{"false if condition skips the body",
		 "{ if (2 < 1) write 9; write 7; }", if_only, ARRLEN(if_only), 1, 0},
		{"if and else branch to distinct targets and join",
		 "{ if (1 == 0) write 1; else write 2; write 3; }", if_else, ARRLEN(if_else), 1, 0},
		{"dangling else binds to the inner if",
		 "{ if (1 == 1) if (0 != 0) write 1; else write 2; write 3; }",
		 nested_if, ARRLEN(nested_if), 2, 0},
		{"while tests before the body and exits when false",
		 "{ int a; a = 2; while (a < 2) a = a + 1; write a; }",
		 while_loop, ARRLEN(while_loop), 2, 0},
		{"nested loops use distinct entry and exit labels",
		 "{ int a, b; a = 0; b = 0; while (a < 2) { while (b < 3) b = b + 1; a = a + 1; } }",
		 nested_loops, ARRLEN(nested_loops), 4, 0},
		{"read uses the target's symbol index without a temporary",
		 "{ int unused, a; read a; }", read, ARRLEN(read), 0, 0},
		{"write uses a literal as an immediate operand",
		 "{ write 42; }", write_immediate, ARRLEN(write_immediate), 0, 0},
		{"read and write preserve statement order and use the expression result",
		 "{ int a, b; read b; read a; write a; write 7; write b * a; }",
		 io_sequence, ARRLEN(io_sequence), 1, 0},
		{"INT_MAX is preserved", limit_source, int_max, ARRLEN(int_max), 1, 0},
		{"INT_MAX + 1 rolls back earlier control flow and all IR counters", above_int_source, NULL, 0, 0, 3},
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
