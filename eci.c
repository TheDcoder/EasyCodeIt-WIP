/* 
 * This file is part of EasyCodeIt.
 * 
 * Copyright (C) 2020 TheDcoder <TheDcoder@protonmail.com>
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
#include <stdnoreturn.h>
#include <string.h>
#include "utils.h"
#include "parser/parser.h"

static char *provide_code(char *file, size_t *size, bool once) {
	static char **once_list = NULL;
	static size_t once_list_len = 0;
	if (once) {
		// Add the file to "include once" list
		if (!once_list) {
			once_list = malloc(sizeof(char *));
			if (!once_list) return NULL;
			++once_list_len;
		} else {
			char **expanded_list = realloc(once_list, sizeof(char *) * ++once_list_len);
			if (!expanded_list) {
				--once_list_len;
				return NULL;
			}
			once_list = expanded_list;
		}
		once_list[once_list_len - 1] = strdup(file);
		
		return NULL;
	} else {
		// Skip if the file is in the "include once" list
		for (size_t i = 0; i < once_list_len; ++i) {
			if (strcmp(once_list[i], file) == 0) return NULL;
		}
	}
	
	// Open the source file
	char *code;
	FILE *source_file = fopen(file, "r");
	if (source_file) {
		// Read the source file
		code = readfile(source_file);
		fclose(source_file);
		if (!code) die("Failed to read from source file!");
	} else {
		code = malloc(1);
		code[0] = '\0';
	}
	
	// The following is required because the parser needs a string with two null terminators
	*size = strlen(code);
	char *code_padded = realloc(code, *size + 2);
	if (!code_padded) {
		free(code);
		die("Failed to expand code buffer");
	}
	code_padded[*size + 1] = '\0';
	
	return code_padded;
}

int main(int argc, char *argv[]) {
	if (argc < 2) die("No arguments!");
	
	// Parse the code
	//scan(argv[1], provide_code);
	parse(argv[1], provide_code);
	
	return EXIT_SUCCESS;
}
