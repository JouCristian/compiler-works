#ifndef IR_H
#define IR_H

#include <stddef.h>
#include "lexer.h"

typedef enum {
	IR_ADD, IR_SUB, IR_MULT, IR_DIV,// '+' '-' '*' '/'
	IR_ASSIGN,			// '='
	IR_GT, IR_LT, IR_GET, IR_LET,	// '>' '<' '>=' '<='
	IR_EQ, IR_NEQ,			// '==' '!='
	IR_READ, IR_WRITE,		// 'read' 'write'
	IR_BR, IR_BRT, IR_BRF		// 'jump'
} IRop;

typedef enum  {
	OPERAND_IMMEDIATE,
	OPERAND_VARIABLE,
	OPERAND_TEMP,
	OPERAND_LABLE,
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

#endif
