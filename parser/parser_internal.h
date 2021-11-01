#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include "jansson.h"

struct Operand operand_from_prim(struct Primitive *primitive);
struct Operand operand_from_expr(struct Expression *expression);
struct Operand operand_from_exprlist(struct ExpressionList *expression_list);
struct Expression expr_from_prim(struct Primitive *primitive);
struct Expression expr_from_str(char *str, size_t len);
struct Expression expr_from_ident(char *ident, size_t len);
struct Expression expr_from_call(struct Expression *caller, struct ExpressionList *arguments);
struct Expression expr_from_expr(struct Expression *exp_list[], unsigned short count, enum Operation op);
struct ExpressionList exprlist_from_expr(struct Expression *expr, struct ExpressionList *list);
struct Expression binary_expr(struct Expression *a, struct Expression *b, enum Operation op);
json_t *prim_to_json(struct Primitive *prim);
json_t *expr_to_json(struct Expression *expr);
json_t *exprlist_to_json(struct ExpressionList *expr_list);
void print_expr(struct Expression *expr);

#endif
