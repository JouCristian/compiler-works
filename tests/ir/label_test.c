#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "parser.h"

static bool test_branches(void) {
	IRTable table = {0};
	Operand first = ir_new_label(&table);
	Operand second = ir_new_label(&table);
	Operand none = {.kind = OPERAND_NONE};
	Operand condition = {.kind = OPERAND_IMMEDIATE, .data.value = 1};
	IRquad start = {IR_LABEL, none, none, first};
	IRquad end = {IR_LABEL, none, none, second};
	IRquad invalid = {IR_BR, none, none, {.kind = OPERAND_LABEL, .data.index = 2}};
	bool success = first.kind == OPERAND_LABEL && first.data.index == 0 &&
		second.kind == OPERAND_LABEL && second.data.index == 1 &&
		!ir_table_append(&table,&invalid) &&
		!ir_table_append(&table,&(IRquad){IR_BRT, none, none, second}) &&
		ir_table_append(&table,&start) &&
		ir_table_append(&table,&(IRquad){IR_BRT, condition, none, second}) &&
		ir_table_append(&table,&(IRquad){IR_BRF, condition, none, second}) &&
		ir_table_append(&table,&(IRquad){IR_BR, none, none, first}) &&
		!ir_table_validate(&table) &&
		ir_table_append(&table,&end) && ir_table_validate(&table) &&
		ir_table_append(&table,&end) && !ir_table_validate(&table);
	ir_table_free(&table);
	return success && table.label_count == 0 && table.next_label == 0;
}

static bool test_ir_capacity(void) {
	IRTable table = {0};
	const IRquad quad = {
		IR_WRITE, {.kind = OPERAND_IMMEDIATE, .data.value = 1},
		{.kind = OPERAND_NONE}, {.kind = OPERAND_NONE}
	};
	while(table.count < IRTB_MAX_LEN && ir_table_append(&table,&quad)) {}
	bool success = table.count == IRTB_MAX_LEN && table.quad_count == IRTB_MAX_LEN &&
		!ir_table_append(&table,&quad) && ir_table_validate(&table);
	table.next_label = IRTB_MAX_LEN;
	success = success && ir_new_label(NULL).kind == OPERAND_NONE &&
		ir_new_label(&table).kind == OPERAND_NONE && table.next_label == IRTB_MAX_LEN;
	ir_table_free(&table);
	return success;
}

static bool test_capacity_rollback(void) {
	const Token tokens[] = {
		{TOKEN_DELIMITER, "{", 1}, {TOKEN_KEYWORD, "while", 1},
		{TOKEN_DELIMITER, "(", 1}, {TOKEN_NUMBER, "1", 1},
		{TOKEN_OPERATOR, "<", 1}, {TOKEN_NUMBER, "2", 1},
		{TOKEN_DELIMITER, ")", 1}, {TOKEN_DELIMITER, ";", 1},
		{TOKEN_DELIMITER, "}", 1}
	};
	SemanticContext semantic = {0};
	IRTable *table = &semantic.irs;
	Operand none = {.kind = OPERAND_NONE};
	Operand one = {.kind = OPERAND_IMMEDIATE, .data.value = 1};
	Operand label = ir_new_label(table);
	Operand temp = ir_new_temp(table);
	const IRquad first = {IR_LABEL, none, none, label};
	const IRquad add = {IR_ADD, one, one, temp};
	const IRquad write = {IR_WRITE, temp, none, none};
	bool success = false;
	if(!ir_table_append(table,&first) || !ir_table_append(table,&add)) goto finish;
	while(table->count < IRTB_MAX_LEN - 3) {
		if(!ir_table_append(table,&write)) goto finish;
	}

	Parser parser;
	parser_init_mode(&parser,tokens,ARRLEN(tokens),&semantic,PARSE_IR);
	success = !parser_program(&parser) && semantic.resource_failed &&
		table->count == IRTB_MAX_LEN - 3 && table->quad_count == IRTB_MAX_LEN - 4 &&
		table->label_count == 1 && table->next_label == 1 && table->temp_count == 1 &&
		table->quads[table->count - 1].op == IR_WRITE && ir_table_validate(table);

finish:
	ir_table_free(table);
	return success;
}

int main(void) {
	bool branches = test_branches();
	bool capacity = test_ir_capacity();
	bool rollback = test_capacity_rollback();
	printf("%s: unique labels, all branch kinds, and target validation\n",branches ? "PASS" : "FAIL");
	printf("%s: IR table capacity and label exhaustion\n",capacity ? "PASS" : "FAIL");
	printf("%s: parser rolls back all counters at IR capacity\n",rollback ? "PASS" : "FAIL");
	return branches && capacity && rollback ? EXIT_SUCCESS : EXIT_FAILURE;
}
