#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "ir.h"

static bool is_temp(Operand operand, size_t index) {
	return operand.kind == OPERAND_TEMP && operand.data.index == index;
}

static bool test_temp_sequence(void) {
	IRTable table;
	IRTable other;
	ir_table_init(&table);
	ir_table_init(&other);

	Operand first = ir_new_temp(&table);
	Operand second = ir_new_temp(&table);
	Operand other_first = ir_new_temp(&other);
	Operand third = ir_new_temp(&table);
	bool success = is_temp(first,0) && is_temp(second,1) && is_temp(third,2) &&
		is_temp(other_first,0) && table.temp_count == 3 && other.temp_count == 1 &&
		table.quads == NULL && table.count == 0 && table.capacity == 0;

	ir_table_free(&table);
	ir_table_free(&other);
	return success;
}

static bool test_temp_boundary(void) {
	ir_table_init(NULL);
	ir_table_free(NULL);
	Operand invalid = ir_new_temp(NULL);

	IRTable table = {.temp_count = SIZE_MAX - 1};
	Operand last = ir_new_temp(&table);
	Operand exhausted = ir_new_temp(&table);
	Operand retry = ir_new_temp(&table);
	bool success = invalid.kind == OPERAND_NONE && is_temp(last,SIZE_MAX - 1) &&
		exhausted.kind == OPERAND_NONE && retry.kind == OPERAND_NONE &&
		table.temp_count == SIZE_MAX;

	ir_table_free(&table);
	return success;
}

static bool test_temp_reset(void) {
	IRTable table = {.temp_count = 1};
	ir_table_init(&table);
	Operand first = ir_new_temp(&table);
	ir_table_free(&table);
	bool cleared = table.temp_count == 0;

	ir_table_init(&table);
	Operand next = ir_new_temp(&table);
	ir_table_free(&table);
	return is_temp(first,0) && cleared && is_temp(next,0);
}

int main(void) {
	const struct {
		const char *name;
		bool (*run)(void);
	} tests[] = {
		{"temporary indices increase independently without allocating quads", test_temp_sequence},
		{"null tables and exhausted counters return OPERAND_NONE", test_temp_boundary},
		{"initialization and release reset the temporary counter", test_temp_reset}
	};

	size_t failures = 0;
	for(size_t i = 0; i < ARRLEN(tests); i++) {
		if(tests[i].run()) {
			printf("PASS: %s\n",tests[i].name);
		} else {
			fprintf(stderr,"FAIL: %s\n",tests[i].name);
			failures++;
		}
	}
	printf("IR temporary tests: %zu/%zu passed\n",ARRLEN(tests) - failures,ARRLEN(tests));
	return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
