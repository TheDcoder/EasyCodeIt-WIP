/* 
 * This file is part of EasyCodeIt.
 * 
 * Copyright (C) 2021 TheDcoder <TheDcoder@protonmail.com>
 * 
 * EasyCodeIt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TREE_H
#define TREE_H

#include <stdbool.h>
#include <stddef.h>

enum TokenType {
	TOK_UNKNOWN,
	TOK_WHITESPACE,
	TOK_COMMENT,
	TOK_DIRECTIVE,
	TOK_NUMBER,
	TOK_STRING,
	TOK_BOOL,
	TOK_WORD,
	TOK_MACRO,
	TOK_VARIABLE,
	TOK_OPERATOR,
	TOK_BRACKET,
	TOK_DOT,
	TOK_COMMA,
	TOK_EOF,
};

enum Operator {
	// Invalid
	OPR_ERR,
	// '+', '-', '*', '/', '^',
	OPR_ADD, OPR_SUB, OPR_MUL, OPR_DIV, OPR_EXP,
	// '&',
	OPR_CAT,
	// '=', '<', '>',
	OPR_EQU, OPR_LES, OPR_GRT,
	// '?', ':',
	OPR_CON_MRK, OPR_CON_SEP,
	
	// Special
	OPR_AND, OPR_OR, // And, Or
};

enum Keyword {
	KWD_NONE, // Special: Not a keyword
	KWD_DIM,
	KWD_LOCAL,
	KWD_GLOBAL,
	KWD_ENUM,
	KWD_CONST,
	KWD_STATIC,
	KWD_CONT_CASE,
	KWD_CONT_LOOP,
	KWD_DEFAULT,
	KWD_NULL,
	KWD_DO,
	KWD_UNTIL,
	KWD_WHILE,
	KWD_END_WHILE,
	KWD_FOR,
	KWD_IN,
	KWD_TO,
	KWD_STEP,
	KWD_NEXT,
	KWD_EXIT,
	KWD_EXITLOOP,
	KWD_FUNC,
	KWD_RETURN,
	KWD_END_FUNC,
	KWD_IF,
	KWD_ELSE,
	KWD_ELSE_IF,
	KWD_END_IF,
	KWD_REDIM,
	KWD_SELECT,
	KWD_SWITCH,
	KWD_CASE,
	KWD_END_SELECT,
	KWD_END_SWITCH,
	KWD_AND,
	KWD_OR,
	KWD_NOT,
};

enum Operation {
	/* Error */
	OP_ERR,
	
	/* No Operation */
	OP_NOP,
	
	/* Assignment */
	OP_ASS,
	
	/* Invert (Additive inversion) */
	OP_INV,
	
	/* Addition, Substraction, Multiplcation, Division, Exponentiation */
	OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_EXP,
	
	/* Concatenation */
	OP_CAT,
	
	/* Negation, Logical "And", Logical "Or" */
	OP_NOT, OP_AND, OP_OR,
	/* Equality, Strict equality, Inequality*/
	OP_EQU, OP_SEQU, OP_NEQ,
	/* Less than, Less than or equal to, Greater than, Greater than or equal to */
	OP_LT, OP_LTE, OP_GT, OP_GTE,
	
	/* Conditional */
	OP_CON,
	
	/* Access, Call */
	OP_ACC, OP_CALL,
};

struct TokenOperator {
	enum Operator sym;
	enum Operation op;
	enum Operation equal_op; // For equable operations (+=, *=, -=, /= etc.)
	int precedence;
};

struct Token {
	enum TokenType type;
	char *data;
	size_t data_len;
	union {
		// Number
		double number;
		
		// String
		char quote;
		
		// Operator
		struct TokenOperator op_info;
		
		// Keyword
		enum Keyword keyword;
		
		// Generic
		void *info;
	};
};

struct Primitive {
	enum {
		PRI_NUMBER,
		PRI_STRING,
		PRI_BOOLEAN,
		// ...
	} type;
	union {
		double number;
		char *string;
		bool boolean;
	};
};

struct Expression;

struct Operand {
	enum {
		OPE_PRIMITIVE,
		OPE_IDENTIFIER,
		//OPE_VARIABLE,
		//OPE_MACRO,
		OPE_EXPRESSION,
		OPE_EXPRESSION_LIST,
	} type;
	union {
		struct Primitive *value;
		char *identifier;
		//struct Variable *variable;
		//struct Macro *macro;
		struct Expression *expression;
		struct ExpressionList *expression_list;
	};
};

struct Expression {
	enum Operation op;
	struct Operand *operands;
};

struct ExpressionList {
	struct Expression *expression;
	struct ExpressionList *list;
};

struct Declaration {
	enum {SCO_AUTO, SCO_LOCAL, SCO_GLOBAL} scope;
	bool is_constant : 1;
	bool is_static : 1;
	bool is_function : 1;
	char *name;
	union {
		// Variable or constant
		struct Expression *initializer;
		// Function
		struct {
			struct Statement *block;
			size_t size;
		} code;
	};
};

struct Statement {
	enum StatementType {
		SMT_DECLARATION,
		SMT_EXPRESSION,
	} type;
	union {
		struct Declaration *declaration;
		struct Expression *expression;
	};
};

struct Unit {
	enum UnitType {
		UNT_COMMENT,
		UNT_DIRECTIVE,
		UNT_STATEMENT,
	} type;
	union {
		struct Token *token;
		struct Statement *statement;
	};
};

#endif
