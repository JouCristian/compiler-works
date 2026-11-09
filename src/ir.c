#include "ir.h"

#include <stdlib.h>
#include <stdint.h>

void ir_table_init(IRTable *table) {
	*table = (IRTable){0};
}

bool ir_table_append(IRTable *table, const IRquad *quad) {
	if(table == NULL || quad == NULL) return false;
	if(table->count == table->capacity) {
		size_t new_capacity;
		if(table->capacity == 0) {
			new_capacity = 2;
		} else {
			new_capacity = table->count * 2;
			if(new_capacity > IRTB_MAX_LEN) return false;
		}
		IRquad *new_quads = realloc(table->quads,new_capacity * sizeof(IRquad));
		if(new_quads == NULL) return false;
		table->quads = new_quads;
		table->capacity = new_capacity;
	}
	table->quads[table->count++] = *quad;
	if(quad->op == IR_LABEL) table->label_count++;
	else table->quad_count++;
	return true;
}

void ir_table_free(IRTable *table) {
	free(table->quads);
	*table = (IRTable){0};
}
