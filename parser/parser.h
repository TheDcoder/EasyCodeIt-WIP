#ifndef PARSER_H
#define PARSER_H

#include "alloc/alloc.h"

typedef char *(*source_reader)(char *file, size_t *size, bool once);

void scan(char *file, source_reader read_func);
Allocator *parse(char *file, source_reader read_func);
Allocator *start_parser(void);

#endif
