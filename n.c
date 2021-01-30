#include <stdio.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>

int main() {

	const char* input = "daniel";
	const char* context = "\nbob\ncat\ndaniel\n";
	const int length = (int) strlen(input), count = (int) strlen(context);

	int a = 0, b = 0, c = 0, d = 0;
	int stack[4096] = {0};

	while (a < length and input[a] < 33) a++;

	stack[c + 0] = a; // ranges in input length
	stack[c + 1] = b; // ranges in context length
	stack[c + 2] = c; // ranges in MAX context length   // is this really necccessary?...
	stack[c + 3] = d; // ranges in stack

_0:	if (b == count) {
		if (not c) return 1;
		c -= 2; 
		goto _0;
	}
_1:	b++; 
	if (b < count and context[b] != 10) goto _1;
	a = stack[c];
_2:	if (context[b] == 32) {
		goto _0;
	}
	if (a >= length) goto _0;
	if (context[b] != input[a]) goto _0;
_3:	a++;
	if (a < length and input[a] < 33) goto _3;
	b++;
	if (context[b] != 10) goto _2;
	if (c) { goto _2; }
	if (a != length) goto _0;

	puts("compile successful.");
}
