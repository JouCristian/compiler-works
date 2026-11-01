CC := gcc

CFLAGS := -std=c11 -Wall -Wextra -g -O0 -Iinclude

SRCS := src/main.c src/lexer.c src/parser.c
INCLUDES := include/lexer.h include/ansi.h include/common.h include/parser.h
TARGET := build/lexer
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
