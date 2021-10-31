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
#include "alloc.h"
#include "cease/cease.h"

Allocator alloc_init(AllocatorFunc *alloc, AllocatorFreeFunc *free, CeasePoint *cease_point, char *def_ctx) {
	return (Allocator){
		.alloc = alloc,
		.free = free,
		.point = cease_point,
		.ctx = def_ctx,
		.node = NULL
	};
}

void *alloc_new(Allocator *allocator, size_t size) {
	AllocatorFunc *aalloc = allocator->alloc;
	AllocatorFreeFunc *afree = allocator->free;
	struct AllocatorNode *node = aalloc(sizeof *node);
	if (!node) goto nomem;
	node->ptr = aalloc(size);
	if (!node->ptr) {
		afree(node);
		goto nomem;
	}
	node->prev = allocator->node;
	allocator->node = node;
	return node->ptr;
	
	nomem: // alloc function returned NULL
	if (allocator->point) cease_mem(allocator->point, allocator->ctx);
	return NULL;
}

void *alloc_ctx(Allocator *allocator, size_t size, char *ctx) {
	if (!ctx) return alloc_new(allocator, size);
	char *def_ctx = allocator->ctx;
	allocator->ctx = ctx;
	void *ptr = alloc_new(allocator, size);
	allocator->ctx = def_ctx;
	return ptr;
}

void alloc_free(Allocator *allocator, void *ptr) {
	AllocatorFreeFunc *afree = allocator->free;
	struct AllocatorNode *node = allocator->node;
	struct AllocatorNode *next_node = NULL;
	while (node) {
		if (node->ptr != ptr) {
			next_node = node;
			node = node->prev;
			continue;
		}
		if (next_node) {
			next_node->prev = node->prev;
		} else {
			// node == allocator->node
			allocator->node = node->prev;
		}
		afree(ptr);
		afree(node);
		break;
	}
}

void alloc_free_all(Allocator *allocator) {
	AllocatorFreeFunc *afree = allocator->free;
	struct AllocatorNode *node = allocator->node;
	struct AllocatorNode *prev_node;
	while (node) {
		prev_node = node->prev;
		afree(node->ptr);
		afree(node);
		node = prev_node;
	}
	allocator->node = NULL;
}
