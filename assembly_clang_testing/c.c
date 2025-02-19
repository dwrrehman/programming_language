#include <stdio.h>
#include <stdint.h>
//  clang c.c -S -Os -Weverything
void calc(uint64_t a, uint64_t b, uint64_t c) {
	for (uint64_t i = 0; i < c; i++) {
		printf("%i\n", (a * i + b) % 5);
	}
}

