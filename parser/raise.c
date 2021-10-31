#ifndef PARSER_RAISE_C
#define PARSER_RAISE_C

#include <stdbool.h>
#include "raise.h"

extern struct ParserError parse_error = {.free_msg = false};

noreturn void raise_error(char *msg, bool free_msg) {
	if (parse_error.free_msg && parse_error.msg) free(parse_error.msg);
	
	parse_error.msg = msg;
	parse_error.free_msg = free_msg;
	
	longjmp(parse_error.jump, true);
}

noreturn void raise_error_fmt(char *def, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	
	char *msg;
	int result = vasprintf(&msg, fmt, args);
	if (result == -1) msg = def;
	
	va_end(args);
	
	raise_error(msg, result != -1);
}

noreturn void raise_mem(char *context) {
	char *msg = "Failed to allocate memory";
	if (context) {
		raise_error_fmt(msg, "%s when %s", msg, context);
	} else {
		raise_error(msg, false);
	};
}

#endif
