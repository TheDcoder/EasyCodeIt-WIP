%option batch noyywrap nounput nodefault yylineno

%{
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct BufferStack {
	struct BufferStack *prev;
	char *code;
	YY_BUFFER_STATE state;
	size_t line;
	char *file;
} *lex_buffer = NULL;

static char *token_str = NULL;
static size_t token_len = 0;
static size_t comment_level = 0;

#define YY_USER_ACTION token_str = yytext; token_len = yyleng;

#define return_string_type(type) yylval.str.str = yytext; yylval.str.len = yyleng; return type;

/*enum { // Token Types
	UNKNOWN = 420,
	WS,
	COMMENT, DIRECTIVE,
	NUMBER, STRING, BOOL,
	WORD,
	MACRO, VARIABLE,
	OPERATOR, BRACKET, DOT, COMMA,
};*/

#include "parser/parser.h"
#include "cease/cease.h"
#include "parser.tab.h"

static source_reader read_file;

static bool push_file(char *file);
static bool pop_file(void);

static bool parse_mode;

static void begin_default_state(void);

%}

%s SCAN_ONLY

%x INCLUDE
%x DIRECTIVE_LINE
%x ML_COMMENT

WS	[ \t\r\n]+
NL	\r?\n?

DIGIT	[0-9]
XDIGIT	[0-9A-Fa-f]

QUOTE	[\'\"]

%%

 /* Include File */
^"#include"{WS}[\"<]	BEGIN INCLUDE;
<INCLUDE>[^\n\">]+	%{
	int c;
	while((c = input()) && c != '\n') /* Eat up any leftover junk in the include line */;
	++yylineno;
	push_file(yytext);
	begin_default_state();
%}
<INCLUDE>.|\n	/* Ignore bad include line */;
"#include-once"	%{
	/* Add current file to "include once" list*/
	read_file(lex_buffer->file, NULL, true);
%}

 /* Whitespace */
{WS}	;

 /* Directive */
"#"	{BEGIN DIRECTIVE_LINE; yymore();};
<DIRECTIVE_LINE>.+	{begin_default_state(); return DIRECTIVE;};

 /* Comment */
<INITIAL,SCAN_ONLY,ML_COMMENT>("#cs"|"#comments-start"){WS}	{BEGIN ML_COMMENT; ++comment_level; yymore();};
<ML_COMMENT>"#ce"|"#comments-end"	%{
	if (--comment_level == 0) {
		begin_default_state();
		return COMMENT;
	}
%}
<ML_COMMENT>(?s:.)	yymore();
;[^\r\n]*	return COMMENT;

 /* Number */
{DIGIT}+(\.{DIGIT}+(e{DIGIT}+)?)?	|
0[xX]{XDIGIT}+	%{
	yylval.num = strtod(yytext, NULL);
	return NUMBER;
%}

 /* String */
(\"[^\n\"]*\"|\'[^\n\']*\')	return_string_type(STRING);

 /* Bool */
(?i:"True")	yylval.boolean = true; return BOOL;
(?i:"False")	yylval.boolean = false; return BOOL;

 /* Operator */
<SCAN_ONLY>[+\-*/^&=<>?:]	return OPERATOR;
<INITIAL>[+\-*/^&=<>?:]	return yytext[0];

(?i:"And")	return AND;
(?i:"Or")	return OR;
(?i:"Not")	return NOT;

"<="	return LTE;
">="	return GTE;
"<>"	return NEQ;
"=="	return SEQU;

 /* Word */
[A-Za-z][A-Za-z0-9]*	return_string_type(WORD);

 /* Macro or Variable */
[@$][A-Za-z0-9_]+	return_string_type(yytext[0] == '$' ? VARIABLE : MACRO);

 /* Misc */
<SCAN_ONLY>[[\]()]	return BRACKET;
<SCAN_ONLY>\.		return DOT;
<SCAN_ONLY>\,		return COMMA;
<INITIAL>[[\]().,]	return yytext[0];

 /* Pop file and terminate if top-level */
<<EOF>>	if(!pop_file()) yyterminate();

 /* Catch-all for everything else */
.	return UNKNOWN;

%%

#include <stddef.h>
#include <stdio.h>

static void begin_default_state() {
	if (parse_mode) {
		BEGIN INITIAL;
	} else {
		BEGIN SCAN_ONLY;
	}
}

static void print_token(char *str, int type) {
	puts("---### TOKEN ###---");
	char *token_type;
	switch (type) {
		case UNKNOWN:
			token_type = "Unknown";
			break;
		case WS:
			token_type = "Whitespace";
			break;
		case COMMENT:
			token_type = "Comment";
			break;
		case DIRECTIVE:
			token_type = "Directive";
			break;
		case NUMBER:
			token_type = "Number";
			break;
		case STRING:
			token_type = "String";
			break;
		case BOOL:
			token_type = "Boolean";
			break;
		case WORD:
			token_type = "Word";
			break;
		case MACRO:
			token_type = "Macro";
			break;
		case VARIABLE:
			token_type = "Variable";
			break;
		case AND:
		case OR:
		case NOT:
		case LTE:
		case GTE:
		case NEQ:
		case OPERATOR:
			token_type = "Operator";
			break;
		case BRACKET:
			token_type = "Bracket";
			break;
		case DOT:
			token_type = "Dot";
			break;
		case COMMA:
			token_type = "Comma";
			break;
		default:
			token_type = "Unnamed";
			break;
	}
	fputs("Type: ", stdout);
	puts(token_type);
	fputs("Data: ", stdout);
	puts(str);
}

void scan(char *file, source_reader read_func) {
	parse_mode = false;
	begin_default_state();
	read_file = read_func;
	push_file(file);
	int type;
	for (;;) {
		type = yylex();
		if (!type) break;
		print_token(token_str, type);
	}
}

Allocator *parse(char *file, source_reader read_func) {
	parse_mode = true;
	read_file = read_func;
	push_file(file);
	return start_parser();
}

static bool push_file(char *file) {
	size_t code_len;
	file = strdup(file);
	if (!file) return false;
	char *code = read_file(file, &code_len, false);
	if (!code) {
		free(file);
		return false;
	}
	struct BufferStack *new_buffer = malloc(sizeof *new_buffer);
	if (!new_buffer) return false;
	if (lex_buffer) lex_buffer->line = yylineno;
	*new_buffer = (struct BufferStack){
		.prev = lex_buffer,
		.code = code,
		.state = yy_scan_buffer(code, code_len + 2),
		.file = file,
		.line = yylineno = 1,
	};
	lex_buffer = new_buffer;
	yy_switch_to_buffer(lex_buffer->state);
	return true;
}

static bool pop_file() {
	struct BufferStack *prev_buffer = lex_buffer->prev;
	yy_delete_buffer(lex_buffer->state);
	free(lex_buffer->code);
	free(lex_buffer->file);
	free(lex_buffer);
	lex_buffer = prev_buffer;
	if (!lex_buffer) return false;
	yylineno = lex_buffer->line;
	yy_switch_to_buffer(lex_buffer->state);
	return true;
}
