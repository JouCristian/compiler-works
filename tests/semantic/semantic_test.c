#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "parser.h"

typedef struct {
	const char *name;
	const char *source;
	size_t error_count;
	Symbol symbols[4];
} SemanticTest;

static bool check_semantic(const SemanticTest *test, const SemanticContext *semantic) {
	bool success = true;
	if(semantic->error_count != test->error_count) {
		fprintf(stderr, "FAIL: %s: expected %zu semantic errors, got %zu\n",
			test->name, test->error_count, semantic->error_count);
		success = false;
	}

	size_t count = 0;
	for(; count < ARRLEN(test->symbols); count++) {
		const Symbol *expected = &test->symbols[count];
		if(expected->name[0] == '\0') break;

		size_t index;
		if(!symbol_table_find(&semantic->symbols, expected->name, &index)) {
			fprintf(stderr, "FAIL: %s: missing symbol '%s'\n", test->name, expected->name);
			success = false;
			continue;
		}
		const Symbol *actual = &semantic->symbols.symbols[index];
		if(actual->initialized != expected->initialized ||
		   actual->declaration_line != expected->declaration_line) {
			fprintf(stderr,
				"FAIL: %s: '%s' expected initialized=%d, line=%d; got initialized=%d, line=%d\n",
				test->name, expected->name, expected->initialized, expected->declaration_line,
				actual->initialized, actual->declaration_line);
			success = false;
		}
	}
	if(semantic->symbols.count != count) {
		fprintf(stderr, "FAIL: %s: expected %zu symbols, got %zu\n",
			test->name, count, semantic->symbols.count);
		success = false;
	}
	return success;
}

static bool run_test(const SemanticTest *test) {
	FILE *source = NULL;
	FILE *lex_out = NULL;
	TokenBuffer buffer;
	LexerState state = {0};
	SemanticContext semantic = {0};
	bool success = false;
	token_buffer_init(&buffer);
	symbol_table_init(&semantic.symbols);
	ir_table_init(&semantic.irs);

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
	if(fputs(test->source, source) == EOF || fseek(source, 0, SEEK_SET) != 0) {
		perror("prepare test source");
		goto finish;
	}
	if(!lexer_scan_file(source, lex_out, OUTPUT_ERRORS_ONLY, &buffer, &state) ||
	   state.error_count != 0) {
		fprintf(stderr, "FAIL: %s: lexer failed (%u lexical errors)\n",
			test->name, state.error_count);
		goto finish;
	}

	Parser parser;
	parser_init(&parser, buffer.tokens, buffer.count, &semantic);
	bool parsed = parser_program(&parser);
	if(semantic.resource_failed) {
		fprintf(stderr, "FAIL: %s: semantic analysis ran out of memory\n", test->name);
		goto finish;
	}
	if(!parsed || parser.failed || parser.pos != parser.count) {
		fprintf(stderr, "FAIL: %s: expected valid syntax, stopped at token %zu/%zu, line %d: %s\n",
			test->name, parser.pos, parser.count, parser.error_line, parser.error_message);
		goto finish;
	}
	success = check_semantic(test, &semantic);

finish:
	if(source != NULL) fclose(source);
	if(lex_out != NULL) fclose(lex_out);
	token_buffer_free(&buffer);
	symbol_table_free(&semantic.symbols);
	ir_table_free(&semantic.irs);
	return success;
}

int main(void) {
	const SemanticTest tests[] = {
		{
			"declaration lists start uninitialized",
			"{\n"
			"int a, b;\n"
			"int c;\n"
			"}\n",
			0, {{"a", false, 2}, {"b", false, 2}, {"c", false, 3}}
		},
		{
			"duplicate declaration keeps the first symbol",
			"{\n"
			"int a;\n"
			"int a, b;\n"
			"}\n",
			1, {{"a", false, 2}, {"b", false, 3}}
		},
		{
			"undeclared assignment, read and expression identifiers",
			"{\n"
			"int a;\n"
			"b = 1; read c; a = d;\n"
			"}\n",
			3, {{"a", false, 2}}
		},
		{
			"assignment and read initialize declared variables",
			"{\n"
			"int a, b, c;\n"
			"read a; b = a + 1; write b;\n"
			"}\n",
			0, {{"a", true, 2}, {"b", true, 2}, {"c", false, 2}}
		},
		{
			"invalid assignments leave their targets uninitialized",
			"{\n"
			"int a, b, c;\n"
			"a = a + 1; b = a; c = 1; write c;\n"
			"}\n",
			2, {{"a", false, 2}, {"b", false, 2}, {"c", true, 2}}
		},
		{
			"if and else both initialize the same variable",
			"{\n"
			"int a;\n"
			"if (1 < 2) a = 1; else read a;\n"
			"write a;\n"
			"}\n",
			0, {{"a", true, 2}}
		},
		{
			"if without else preserves the incoming state",
			"{\n"
			"int a, keep;\n"
			"read keep;\n"
			"if (keep < 2) { a = 1; write a; }\n"
			"write a; write keep;\n"
			"}\n",
			1, {{"a", false, 2}, {"keep", true, 2}}
		},
		{
			"if and else retain only variables initialized on both paths",
			"{\n"
			"int a, b;\n"
			"if (1 < 2) a = 1; else b = 1;\n"
			"write a; write b;\n"
			"}\n",
			2, {{"a", false, 2}, {"b", false, 2}}
		},
		{
			"else starts with the state before the then branch",
			"{\n"
			"int a;\n"
			"if (1 < 2) a = 1; else { write a; a = 2; }\n"
			"write a;\n"
			"}\n",
			1, {{"a", true, 2}}
		},
		{
			"while condition is checked before the body",
			"{\n"
			"int a;\n"
			"while (a < 2) a = 1;\n"
			"}\n",
			1, {{"a", false, 2}}
		},
		{
			"while body can initialize locally and the loop restores the entry state",
			"{\n"
			"int a, keep;\n"
			"read keep;\n"
			"while (keep < 2) { a = 1; write a; }\n"
			"write a; write keep;\n"
			"}\n",
			1, {{"a", false, 2}, {"keep", true, 2}}
		},
		{
			"for initializer is visible in the condition, body and after the loop",
			"{\n"
			"int i;\n"
			"for (i = 0; i < 3; i = i + 1) write i;\n"
			"write i;\n"
			"}\n",
			0, {{"i", true, 2}}
		},
		{
			"for initializer with an invalid right-hand side leaves its target uninitialized",
			"{\n"
			"int i, a;\n"
			"for (i = a; i < 3; i = 0) ;\n"
			"}\n",
			2, {{"i", false, 2}, {"a", false, 2}}
		},
		{
			"for update sees initialization in the body",
			"{\n"
			"int i, a;\n"
			"for (i = 0; i < 3; i = a) a = 1;\n"
			"}\n",
			0, {{"i", true, 2}, {"a", false, 2}}
		},
		{
			"for update reports an uninitialized operand once",
			"{\n"
			"int i, a;\n"
			"for (i = 0; i < 3; i = a) ;\n"
			"}\n",
			1, {{"i", true, 2}, {"a", false, 2}}
		},
		{
			"for update requires a declared target",
			"{\n"
			"int i;\n"
			"for (i = 0; i < 3; a = i) ;\n"
			"}\n",
			1, {{"i", true, 2}}
		},
		{
			"for body cannot use initialization from the update",
			"{\n"
			"int i, a;\n"
			"for (i = 0; i < 3; a = 1) write a;\n"
			"}\n",
			1, {{"i", true, 2}, {"a", false, 2}}
		},
		{
			"for restores the state after initialization and before the body",
			"{\n"
			"int i, body, update, keep;\n"
			"read keep;\n"
			"for (i = 0; i < 3; update = body) { body = keep; write body; }\n"
			"write i + keep; write body; write update;\n"
			"}\n",
			2, {{"i", true, 2}, {"body", false, 2}, {"update", false, 2}, {"keep", true, 2}}
		}
	};

	size_t failures = 0;
	for(size_t i = 0; i < ARRLEN(tests); i++) {
		if(run_test(&tests[i])) {
			printf("PASS: %s (%zu semantic errors)\n", tests[i].name, tests[i].error_count);
		} else {
			fprintf(stderr, "FAIL: %s\n", tests[i].name);
			failures++;
		}
	}
	printf("Semantic tests: %zu/%zu passed\n", ARRLEN(tests) - failures, ARRLEN(tests));
	return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
