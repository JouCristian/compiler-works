#ifndef IR_H
#define IR_H

#include <stddef.h>
#include "lexer.h"

#define IRTB_MAX_LEN 32768

typedef enum {
	IR_ADD, IR_SUB, IR_MULT, IR_DIV,// '+' '-' '*' '/'
	IR_ASSIGN,			// '='
	IR_GT, IR_LT, IR_GET, IR_LET,	// '>' '<' '>=' '<='
	IR_EQ, IR_NEQ,			// '==' '!='
	IR_READ, IR_WRITE,		// 'read' 'write'
	IR_BR, IR_BRT, IR_BRF,		// 'jump'
	IR_LABEL			//label
} IRop;

typedef enum  {
	OPERAND_IMMEDIATE,
	OPERAND_VARIABLE,
	OPERAND_TEMP,
	OPERAND_LABEL,
	OPERAND_NONE
} OperandKind;

typedef struct {
	OperandKind kind;
	union {
		int value;
		size_t index;
	} data;
} Operand;

typedef struct {
	IRop op;
	Operand opr1;
	Operand opr2;
	Operand result;
} IRquad;

typedef struct {
	IRquad *quads;
	size_t count;
	size_t capacity;
	size_t quad_count;
	size_t label_count;
} IRTable;

void ir_table_init(IRTable *table);
bool ir_table_append(IRTable *table, const IRquad *quad);
void ir_table_free(IRTable *table);

#endif
