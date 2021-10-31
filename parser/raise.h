#ifndef PARSER_RAISE_H
#define PARSER_RAISE_H

#include <stdbool.h>

struct ParserError {
	jmp_buf jump;
	char *msg;
	bool free_msg;
}

extern struct ParserError parse_error;

noreturn void raise_error(char *msg, bool free_msg);
noreturn void raise_error_fmt(char *def, char *fmt, ...);
noreturn void raise_mem(char *context);

#endif
