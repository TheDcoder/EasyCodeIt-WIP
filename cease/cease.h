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

#ifndef CEASE_H
#define CEASE_H

#include <setjmp.h>
#include <stdbool.h>
#include <stdnoreturn.h>

struct CeasePoint {
	jmp_buf jump;
	char *msg;
	bool free_msg;
};

typedef struct CeasePoint CeasePoint;

CeasePoint cease_get_point(void);
noreturn void cease(CeasePoint *point, char *msg, bool free_msg);
noreturn void cease_fmt(CeasePoint *point, char *def, char *fmt, ...);
noreturn void cease_mem(CeasePoint *point, char *context);

#endif
