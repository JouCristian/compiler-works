#include "ir.h"

#include <stdlib.h>
#include <stdint.h>

void ir_table_init(IRTable *table) {
	if(table == NULL) return;
	*table = (IRTable){0};
}

Operand ir_new_temp(IRTable *table) {
	if(table == NULL || table->temp_count == SIZE_MAX)
		return (Operand){.kind = OPERAND_NONE};
	return (Operand){.kind = OPERAND_TEMP, .data.index = table->temp_count++};
}

Operand ir_new_label(IRTable *table) {
	if(table == NULL || table->next_label >= IRTB_MAX_LEN)
		return (Operand){.kind = OPERAND_NONE};
	return (Operand){.kind = OPERAND_LABEL, .data.index = table->next_label++};
}

static bool ir_is_branch(IRop op) {
	return op == IR_BR || op == IR_BRT || op == IR_BRF;
}

static bool ir_valid_target(const IRTable *table, const IRquad *quad) {
	if(quad->result.kind != OPERAND_LABEL || quad->result.data.index >= table->next_label ||
	   quad->opr2.kind != OPERAND_NONE)
		return false;
	if(quad->op == IR_LABEL || quad->op == IR_BR) return quad->opr1.kind == OPERAND_NONE;
	return quad->opr1.kind == OPERAND_TEMP || quad->opr1.kind == OPERAND_VARIABLE ||
		quad->opr1.kind == OPERAND_IMMEDIATE;
}

bool ir_table_append(IRTable *table, const IRquad *quad) {
	if(table == NULL || quad == NULL || table->count >= IRTB_MAX_LEN) return false;
	if((quad->op == IR_LABEL || ir_is_branch(quad->op)) && !ir_valid_target(table,quad))
		return false;
	IRquad entry = *quad;
	if(table->count == table->capacity) {
		size_t new_capacity = table->capacity == 0 ? 2 : table->capacity * 2;
		if(new_capacity > IRTB_MAX_LEN) new_capacity = IRTB_MAX_LEN;
		if(new_capacity > SIZE_MAX / sizeof(IRquad)) return false;
		IRquad *new_quads = realloc(table->quads,new_capacity * sizeof(IRquad));
		if(new_quads == NULL) return false;
		table->quads = new_quads;
		table->capacity = new_capacity;
	}
	table->quads[table->count++] = entry;
	if(entry.op == IR_LABEL) table->label_count++;
	else table->quad_count++;
	return true;
}

bool ir_table_validate(const IRTable *table) {
	if(table == NULL || table->count > IRTB_MAX_LEN || table->count > table->capacity ||
	   (table->count != 0 && table->quads == NULL) || table->next_label > IRTB_MAX_LEN)
		return false;

	bool defined[IRTB_MAX_LEN] = {false};
	size_t definitions = 0;
	for(size_t i = 0; i < table->count; i++) {
		const IRquad *quad = &table->quads[i];
		if(quad->op == IR_LABEL) {
			if(!ir_valid_target(table,quad) || defined[quad->result.data.index]) return false;
			defined[quad->result.data.index] = true;
			definitions++;
		}
	}
	if(definitions != table->label_count || definitions != table->next_label ||
	   table->quad_count != table->count - definitions)
		return false;
	for(size_t i = 0; i < table->count; i++) {
		const IRquad *quad = &table->quads[i];
		if(ir_is_branch(quad->op) &&
		   (!ir_valid_target(table,quad) || !defined[quad->result.data.index]))
			return false;
	}
	return true;
}

void ir_table_free(IRTable *table) {
	if(table == NULL) return;
	free(table->quads);
	*table = (IRTable){0};
}
