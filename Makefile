CC := gcc

CFLAGS := -std=c11 -Wall -Wextra -g -O0 -Iinclude

COMMON_SRCS := src/lexer.c src/parser.c src/ir.c src/semantic.c
SRCS := src/main.c $(COMMON_SRCS)
INCLUDES := include/lexer.h include/ansi.h include/common.h include/parser.h include/ir.h include/semantic.h
TARGET := build/compiler
TESTS ?= tests/lexer/test.c
TEST_TARGETS := build/tests/parser/match_test build/tests/parser/expression_test \
	build/tests/semantic/semantic_test build/tests/ir/temp_test build/tests/ir/arithmetic_test

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SRCS) $(INCLUDES)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(TESTS)

build/tests/%: tests/%.c $(COMMON_SRCS) $(INCLUDES)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< $(COMMON_SRCS) -o $@

test: $(TEST_TARGETS)
	@set -e; for test in $(TEST_TARGETS); do ./$$test; done

clean:
	rm -rf build
	rm -f docs/syntax-analysis/results/syntax-errors.txt
