#include "semantic.h"

#include <string.h>

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
