#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "parser.h"

#define TK(type, value) {type, value, 1}
#define N(x) TK(TOKEN_NUMBER, x)
#define ID(x) TK(TOKEN_IDENTIFIER, x)
#define OP(x) TK(TOKEN_OPERATOR, x)
#define D(x) TK(TOKEN_DELIMITER, x)
#define K(x) TK(TOKEN_KEYWORD, x)

// Enclose the expression so that a trailing ';' cannot start a new statement.
#define WRITE(...) K("write"), D("("), __VA_ARGS__, D(")"), D(";")
#define IF(...) K("if"), D("("), __VA_ARGS__, D(")"), D(";")

static bool check_expression(const char *name, bool expected,
			     const Token *tokens, size_t count) {
	Parser parser;
	parser_init(&parser, tokens, count, NULL);
	bool success = parser_program(&parser);
	if(success != expected || parser.failed == expected) {
		fprintf(stderr, "FAIL: %s, expected %s, got %s%s\n", name,
			expected ? "success" : "syntax error",
			success ? "success" : "failure: ", parser.error_message);
		return false;
	}
	printf("PASS: %s\n", name);
	return true;
}

#define CASE(name, expected, ...) do { \
	const Token tokens[] = {D("{"), __VA_ARGS__, D("}")}; \
	if(!check_expression(name, expected, tokens, ARRLEN(tokens))) \
		failures++; \
} while(0)

int main(void) {
	int failures = 0;

	CASE("2+3*4", true, WRITE(N("2"), OP("+"), N("3"), OP("*"), N("4")));
	CASE("(2+3)*4", true, WRITE(D("("), N("2"), OP("+"), N("3"),
		D(")"), OP("*"), N("4")));
	CASE("a-b/2", true, WRITE(ID("a"), OP("-"), ID("b"), OP("/"), N("2")));
	CASE("a+2>=b*3", true, IF(ID("a"), OP("+"), N("2"),
		OP(">="), ID("b"), OP("*"), N("3")));
	CASE("a+*b", false, WRITE(ID("a"), OP("+"), OP("*"), ID("b")));
	CASE("(2+3", false, WRITE(D("("), N("2"), OP("+"), N("3")));
	CASE("2+", false, WRITE(N("2"), OP("+")));
	CASE("a>b>c", false, IF(ID("a"), OP(">"), ID("b"), OP(">"), ID("c")));
	CASE("2+3;", false, WRITE(N("2"), OP("+"), N("3"), D(";")));
	CASE("a=b", false, IF(ID("a"), OP("="), ID("b")));
	CASE("a>", false, IF(ID("a"), OP(">")));
	CASE("-3", false, WRITE(OP("-"), N("3")));
	CASE(";", false, WRITE(D(";")));

	printf("Expression tests failed: %d\n", failures);
	return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
