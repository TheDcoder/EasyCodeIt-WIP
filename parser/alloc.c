#include <stdbool.h>
#include "dynarr/dynarr.h"

struct Allocator {
	bool init;
	dynarr pointers;
	void (*alloc)(size_t);
}
typedef struct Allocator Allocator;

//static struct ParserAllocator def_allocator = {.init = false};

Allocator alloc_init(void (*alloc)(size_t)) {
	return (Allocator){.alloc = alloc, .init = false};
}

void *alloc_new(Allocator *allocator, size_t size) {
	if (!allocator->init) {
		allocator->pointers = dynarr_init(sizeof(void *));
		allocator->init = true;
	}
	void *new = malloc(size);
	if (!new) return NULL;
	dynarr_push(&allocator->pointers, &new);
	return new;
}
