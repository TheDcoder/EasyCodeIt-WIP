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

#define _GNU_SOURCE /* Required to enable (v)asprintf */
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include "cease.h"

CeasePoint cease_get_point(void) {
	return (CeasePoint){.msg = NULL};
}

noreturn void cease(CeasePoint *point, char *msg, bool free_msg) {
	if (point->free_msg && point->msg) free(point->msg);
	
	point->msg = msg;
	point->free_msg = free_msg;
	
	longjmp(point->jump, true);
}

noreturn void cease_fmt(CeasePoint *point, char *def, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	
	char *msg;
	int result = vasprintf(&msg, fmt, args);
	if (result == -1) msg = def;
	
	va_end(args);
	
	cease(point, msg, result != -1);
}

noreturn void cease_mem(CeasePoint *point, char *context) {
	static char *msg = "Failed to allocate memory";
	if (context) {
		cease_fmt(point, msg, "%s when %s", msg, context);
	} else {
		cease(point, msg, false);
	};
}
