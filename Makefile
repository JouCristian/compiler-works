CC := gcc

CFLAGS := -std=c11 -Wall -Wextra -g -O0 -Iinclude

SRCS := src/main.c src/lexer.c src/parser.c src/ir.c
INCLUDES := include/lexer.h include/ansi.h include/common.h include/parser.h include/ir.h
TARGET := build/compiler
TESTS ?= tests/lexer/test.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS) $(INCLUDES)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(TESTS)

clean:
	rm -rf build
	rm -f docs/syntax-analysis/results/syntax-errors.txt
