#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdbool.h>
#include <stddef.h>

#include "lexer.h"

typedef struct {
	char name[MAX_TOKEN_LEN];
	bool initialized;
	int declaration_line;
} Symbol;

typedef struct {
	Symbol *symbols;
	size_t count;
	size_t capacity;
} SymbolTable;

void symbol_table_init(Symbol *symbol);

bool symbol_table_find(const SymbolTable *table,
		       const char *name,
		       int *index);

void symbol_table_free(Symbol *symbol);

#endif
