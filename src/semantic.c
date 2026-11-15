#include "semantic.h"

#include <string.h>
#include <stdlib.h>

bool symbol_table_find(const SymbolTable *table,
		       const char *name,
		       int *index) {
	if(table == NULL || name == NULL || index == NULL) return false;
	for(size_t i = 0; i < table->count; i ++) {
		if(strcmp(table->symbol[i].name,name) == 0) {
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
	if(table->count == table->capacity) {
		size_t new_capacity;
		if(table->capacity == 0) {
			new_capacity = 2;
		} else {
			new_capacity = table->count * 2;
			if(new_capacity > SB_MAX_LEN) return SYMBOL_NO_MEMORY;
		}
		Symbol *new_symbol = realloc(table->symbols,new_capacisty * sizeof(Symbol));
		if(new_symbol == NULL) return SYMBOL_NO_MEMOEY;
		table->symbols = new_symbols;
		table->capacity = new_capacity;
	}
	Symbol *symbol = &(table->symbols[table->count]);
	strcpy(symbol->name, name);
	symbol->initialize = false;
	symbol->declaration_line = line;

	if(index != NULL) *index = table->count;
	table->count++;

	return SYMBOL_OK;
}
