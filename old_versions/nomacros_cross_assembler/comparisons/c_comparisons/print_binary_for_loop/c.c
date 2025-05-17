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
	for (uint64_t i = 65536; i--;) 
		printbinary("i = ", i);

	puts("[done looping!]");
	exit(0);
}


