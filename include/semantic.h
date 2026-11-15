#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdbool.h>
#include <stddef.h>

#include "lexer.h"

#define SB_MAX_LEN 32768

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

typedef enum {
	SYMBOL_OK,
	SYMBOL_SEMANTIC_WRONG,
	SYMBOL_NO_MEMORY
} SymbolInsertResult;

void symbol_table_init(SymbolTable *table);

bool symbol_table_find(const SymbolTable *table,
		       const char *name,
		       size_t *index);

SymbolInsertResult symbol_table_insert(SymbolTable *table,
				       const char *name,
				       int line,
				       size_t *index);

void symbol_table_free(SymbolTable *table);

#endif
