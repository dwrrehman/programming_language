#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static void printbinary(const char* s, uint64_t n) {
	printf("%s", s);
	for (uint64_t i = 0; i < 64; i++) {
		putchar('0' + (char) ((n >> i) & 1));
	}
	puts("");
}

int main(void) {
	const int limit = 0x40000;
	uint64_t count = 0;
	for (uint64_t i = 2; i < limit; i++) {
		for (uint64_t j = 2; j < i; j++)
			if (i % j == 0) goto composite;
		count++; composite:;
	}

	printbinary("total prime count = ", count);

	puts("[done looping!]");
	exit(0);
}


