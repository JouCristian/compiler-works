#include "semantic.h"

#include <string.h>
#include <stdlib.h>

void symbol_table_init(SymbolTable *table) {
	*table = (SymbolTable){0};
}

bool symbol_table_find(const SymbolTable *table,
		       const char *name,
		       size_t *index) {
	if(table == NULL || name == NULL) return false;
	for(size_t i = 0; i < table->count; i ++) {
		if(strcmp(table->symbols[i].name,name) == 0) {
			*index = i;
			return true;
		}
	}
	return false;
}

SymbolInsertResult symbol_table_insert(SymbolTable *table,
				       const char *name,
				       int line,
				       size_t *index) {
	if(symbol_table_find(table,name,NULL)) return SYMBOL_SEMANTIC_WRONG;
	if(table->count == table->capacity) {
		size_t new_capacity;
		if(table->capacity == 0) {
			new_capacity = 2;
		} else {
			new_capacity = table->count * 2;
			if(new_capacity > SB_MAX_LEN) return SYMBOL_NO_MEMORY;
		}
		Symbol *new_symbol = realloc(table->symbols,new_capacity * sizeof(Symbol));
		if(new_symbol == NULL) return SYMBOL_NO_MEMORY;
		table->symbols = new_symbol;
		table->capacity = new_capacity;
	}
	Symbol *symbol = &(table->symbols[table->count]);
	strcpy(symbol->name, name);
	symbol->initialized = false;
	symbol->declaration_line = line;

	if(index != NULL) *index = table->count;
	table->count++;

	return SYMBOL_OK;
}

void symbol_table_free(SymbolTable *table) {
	free(table->symbols);
	*table = (SymbolTable){0};
}
