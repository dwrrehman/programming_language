#include <stdio.h>

int main(void) {
	puts("");
	for (int i = 0; i < 256; i++) {
		if (!(i % 10)) puts("");
		printf("%d %d ctincr r1 %d\n", i + 1, i, i + 1);
	}
	puts("");
}
