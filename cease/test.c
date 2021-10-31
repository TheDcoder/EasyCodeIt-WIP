#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include "cease.h"

CeasePoint point;

void foo() {
	puts("Cease?");
	if (getc(stdin) != 'y') return;
	cease(&point, "Foo is ceasing!", false);
	return;
}

int main(void) {
	point = cease_get_point();
	if (setjmp(point.jump) > 0) {
		puts("FATAL ERROR!");
		puts(point.msg);
		return 1;
	}
	foo();
	puts("Terminating normally");
	return 0;
}
