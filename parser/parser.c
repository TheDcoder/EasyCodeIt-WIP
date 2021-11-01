#define _GNU_SOURCE /* Required to enable (v)asprintf */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alloc/alloc.h"
#include "cease/cease.h"
#include "parser/parser_internal.h"
#include "parser/tree.h"

#include "jansson.h"

static Allocator parser_alloc;

static void *palloc(size_t size) {
	return alloc_new(&parser_alloc, size);
}

static void *palloc_ctx(size_t size, char *ctx) {
	return alloc_ctx(&parser_alloc, size, ctx);
}

Allocator *start_parser() {
	CeasePoint cease_point = cease_get_point();
	parser_alloc = alloc_init(malloc, free, &cease_point, "parsing code");
	if (setjmp(cease_point.jump)) {
		alloc_free_all(&parser_alloc);
		return NULL;
	}
	yyparse();
	return &parser_alloc;
}

void yyerror(char const *s) {
	fputs(s, stderr);
	fputs("\n", stderr);
}

struct Operand operand_from_prim(struct Primitive *primitive) {
	struct Operand operand = {.type = OPE_PRIMITIVE};
	operand.value = palloc(sizeof *operand.value);
	*operand.value = *primitive;
	return operand;
}

struct Operand operand_from_expr(struct Expression *expression) {
	if (expression->op == OP_NOP) return expression->operands[0];
	struct Operand operand = {.type = OPE_EXPRESSION};
	operand.expression = palloc(sizeof *operand.expression);
	*operand.expression = *expression;
	return operand;
}

struct Operand operand_from_exprlist(struct ExpressionList *expression_list) {
	struct Operand operand = {.type = OPE_EXPRESSION_LIST};
	operand.expression_list = palloc(sizeof *operand.expression_list);
	*operand.expression_list = *expression_list;
	return operand;
}

struct Expression expr_from_prim(struct Primitive *primitive) {
	// TODO: Make a copy of primitive
	struct Expression expression = {.op = OP_NOP};
	expression.operands = palloc(sizeof *expression.operands);
	expression.operands[0] = operand_from_prim(primitive);
	return expression;
}

struct Expression expr_from_str(char *str, size_t len) {
	struct Primitive value = {.type = PRI_STRING};
	value.string = palloc(len + 1);
	strncpy(value.string, str, len);
	return expr_from_prim(&value);
}

struct Expression expr_from_ident(char *ident, size_t len) {
	struct Expression expression = {.op = OP_NOP};
	expression.operands = palloc(sizeof *expression.operands);
	expression.operands[0].type = OPE_IDENTIFIER;
	expression.operands[0].identifier = palloc(len + 1);
	strncpy(expression.operands[0].identifier, ident, len + 1);
	return expression;
}

struct Expression expr_from_call(struct Expression *caller, struct ExpressionList *arguments) {
	struct Expression expression = {.op = OP_CALL};
	expression.operands = palloc(sizeof *expression.operands * 2);
	expression.operands[0] = operand_from_expr(caller);
	expression.operands[1] = operand_from_exprlist(arguments);
	return expression;
}

struct Expression expr_from_expr(struct Expression *exp_list[], unsigned short count, enum Operation op) {
	struct Expression expression = {.op = op};
	expression.operands = palloc(sizeof *expression.operands * count);
	for (unsigned short i = 0; i < count; ++i) {
		expression.operands[i] = operand_from_expr(exp_list[i]);
	}
	return expression;
}

struct ExpressionList exprlist_from_expr(struct Expression *expr, struct ExpressionList *list) {
	struct Expression *expr_copy = palloc(sizeof *expr_copy);
	*expr_copy = *expr;
	struct ExpressionList *list_copy;
	if (list) {
		list_copy = palloc(sizeof *list_copy);
		*list_copy = *list;
	} else list_copy = NULL;
	return (struct ExpressionList){.expression = expr_copy, .list = list_copy};
}

struct Expression binary_expr(struct Expression *a, struct Expression *b, enum Operation op) {
	struct Expression expression = {.op = op};
	expression.operands = palloc(sizeof *expression.operands * 2);
	expression.operands[0] = operand_from_expr(a);
	expression.operands[1] = operand_from_expr(b);
	return expression;
}

json_t *prim_to_json(struct Primitive *prim) {
	json_t *prim_json;
	switch (prim->type) {
		case PRI_BOOLEAN:
			prim_json = json_boolean(prim->boolean);
			break;
		case PRI_NUMBER:
			prim_json = json_real(prim->number);
			break;
		case PRI_STRING:
			prim_json = json_string(prim->string);
			break;
	}
	return prim_json;
}

json_t *expr_to_json(struct Expression *expr) {
	static char *op_names[] = {
		[OP_ERR] = "Error",
		[OP_NOP] = "No Operation",
		[OP_ASS] = "Assign",
		[OP_INV] = "Invert",
		[OP_ADD] = "Add",
		[OP_SUB] = "Subtract",
		[OP_MUL] = "Multiply",
		[OP_DIV] = "Divide",
		[OP_EXP] = "Raise",
		[OP_CAT] = "Concat",
		[OP_NOT] = "Not",
		[OP_AND] = "And",
		[OP_OR] = "Or",
		[OP_EQU] = "Equal",
		[OP_SEQU] = "Strictly Equal",
		[OP_NEQ] = "Not Equal",
		[OP_LT] = "Less Than",
		[OP_LTE] = "Less Than or Equal",
		[OP_GT] = "Greater Than",
		[OP_GTE] = "Greater Than or Equal",
		[OP_CON] = "Conditional",
		[OP_ACC] = "Access",
		[OP_CALL] = "Call",
	};
	
	json_t *expr_json = json_object();
	json_object_set_new(expr_json, "op", json_string(op_names[expr->op]));
	
	json_t *expr_args_json = json_array();
	short arg_count;
	if (expr->op == OP_NOP || expr->op == OP_NOT || expr->op == OP_INV) {
		arg_count = 1;
	} else if (expr->op == OP_CON) {
		arg_count = 3;
	} else {
		arg_count = 2;
	}
	for (short i = 0; i < arg_count; ++i) {
		json_t *arg = json_null();
		switch (expr->operands[i].type) {
			case OPE_EXPRESSION:
				arg = expr_to_json(expr->operands[i].expression);
				break;
			case OPE_EXPRESSION_LIST:
				arg = exprlist_to_json(expr->operands[i].expression_list);
				break;
			case OPE_PRIMITIVE:
				arg = prim_to_json(expr->operands[i].value);
				break;
			case OPE_IDENTIFIER:
				arg = json_object();
				json_object_set_new(arg, "ident", json_string(expr->operands[i].identifier));
				break;
		}
		json_array_append_new(expr_args_json, arg);
	}
	json_object_set_new(expr_json, "args", expr_args_json);
	
	return expr_json;
}

json_t *exprlist_to_json(struct ExpressionList *expr_list) {
	if (!expr_list) return json_null();
	json_t *expr_list_json = json_array();
	json_array_append_new(expr_list_json, expr_to_json(expr_list->expression));
	json_array_append_new(expr_list_json, exprlist_to_json(expr_list->list));
	return expr_list_json;
}

void print_expr(struct Expression *expr) {
	json_t *json = expr_to_json(expr);
	json_dumpf(json, stdout, JSON_INDENT(4));
	fputc('\n', stdout);
	json_decref(json);
}
