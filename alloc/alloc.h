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

#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include "cease/cease.h"

typedef void *AllocatorFunc(size_t);
typedef void AllocatorFreeFunc(void *);

struct AllocatorNode {
	// FIFO for better performance
	void *ptr;
	struct AllocatorNode *prev;
};

struct Allocator {
	struct AllocatorNode *node;
	AllocatorFunc *alloc;
	AllocatorFreeFunc *free;
	CeasePoint *point;
	char *ctx;
};
typedef struct Allocator Allocator;

Allocator alloc_init(AllocatorFunc *alloc, AllocatorFreeFunc *free, CeasePoint *cease_point, char *def_ctx);
void *alloc_new(Allocator *allocator, size_t size);
void *alloc_ctx(Allocator *allocator, size_t size, char *ctx);
void alloc_free(Allocator *allocator, void *ptr);
void alloc_free_all(Allocator *allocator);

#endif
