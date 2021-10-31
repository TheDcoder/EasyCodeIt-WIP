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

%define parse.trace

%code requires {
	#define _GNU_SOURCE /* Required to enable (v)asprintf */
	#include <stdbool.h>
	#include "parser/tree.h"
	#include "parser/parser_internal.h"
}

%union {
	double num;
	struct {
		char *str;
		size_t len;
	} str;
	bool boolean;
	struct Expression expr;
	struct ExpressionList expr_list;
}

%token UNKNOWN

%token WS

%token COMMENT
%token DIRECTIVE

%token <num> NUMBER
%token <str> STRING
%token <boolean> BOOL

%token <str> WORD

%token <str> MACRO
%token <str> VARIABLE

%token OPERATOR
%token BRACKET
%token DOT
%token COMMA

 /* Operators */
%precedence '?'
%precedence ':'
%left AND "And" OR "Or"
%left LT '<' GT '>' LTE "<=" GTE ">=" EQU '=' NEQ "<>" SEQU "=="
%left '&'
%left '+' '-'
%left '*' '/'
%left '^'
%left NOT "Not"
%precedence INVERSION
%precedence '.'
 /* WORKAROUND: Bison can't handle "sandwhich" operators which surround the 2nd part of a binary expression */
%precedence '['
%precedence '('
%precedence GROUPING

%type <expr> expression
%type <expr_list> expression_list

%{
int yylex();
void yyerror(const char *s);
%}

%%

top: /* nothing */
	| expression_list {print_expr($1.expression);}

expression:
	  BOOL {$$ = expr_from_prim(&(struct Primitive){.type = PRI_BOOLEAN, .boolean = $1});}
	| NUMBER {$$ = expr_from_prim(&(struct Primitive){.type = PRI_NUMBER, .number = $1});}
	| STRING {$$ = expr_from_str($1.str, $1.len);}
	| WORD {$$ = expr_from_ident($1.str, $1.len);}
	| MACRO {$$ = expr_from_ident($1.str, $1.len);}
	| VARIABLE {$$ = expr_from_ident($1.str, $1.len);}
	| expression '?' expression ':' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3, &$5}, 3, OP_CON);}
	| expression "And" expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_AND);}
	| expression "Or" expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_OR);}
	| expression '<' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_LT);}
	| expression '>' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_GT);}
	| expression '=' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_EQU);}
	| expression "<=" expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_LTE);}
	| expression ">=" expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_GTE);}
	| expression "<>" expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_NEQ);}
	| expression "==" expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_SEQU);}
	| expression '&' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_CAT);}
	| expression '+' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_ADD);}
	| expression '-' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_SUB);}
	| expression '*' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_MUL);}
	| expression '/' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_DIV);}
	| expression '^' expression {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_EXP);}
	| "Not" expression {$$ = expr_from_expr((struct Expression *[]){&$2}, 1, OP_NOT);}
	| '-' expression %prec INVERSION {$$ = expr_from_expr((struct Expression *[]){&$2}, 1, OP_INV);}
	/*| expression '.' WORD {$$ = expr_from_expr((struct Expression *[]){&$1, &(struct Expression){expr_from_ident($3.str, $3.len)}}, 2, OP_ACC);}*/
	| expression '.' WORD {
		struct Expression ident_expr = expr_from_ident($3.str, $3.len);
		$$ = expr_from_expr((struct Expression *[]){&$1, &ident_expr}, 2, OP_ACC);
	}
	| expression '[' expression ']' {$$ = expr_from_expr((struct Expression *[]){&$1, &$3}, 2, OP_ACC);}
	| expression '(' expression_list ')' {$$ = expr_from_call(&$1, &$3);}
	| expression '(' ')' {$$ = expr_from_call(&$1, NULL);}
	/* | expression '(' expression_list ')' %prec CALL {$$ = expr_from_call(&$1, &$3);}
	| expression '(' ')' %prec CALL {$$ = expr_from_call(&$1, NULL);} */
	| '(' expression ')' %prec GROUPING {$$ = $2;}

expression_list:
	/*  expression {$$ = (struct ExpressionList){.expression = , .list = NULL};}
	| expression ',' expression_list {$$ = (struct ExpressionList){.expression = &$1, .list = &$3};} */
	  expression {$$ = exprlist_from_expr(&$1, NULL);}
	| expression ',' expression_list {$$ = exprlist_from_expr(&$1, &$3);}

%%

#include "parser/parser.c"
