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

void btucsr() {
try:
	if (b == C_length) {
		if (c == 0) return error;
		c -= 3;
		b = C[c + 1];
		goto try;
	}
	while (B[b] != 10) b++; b++;
	a = C[c];
parent:
	if (B[b] == 32) {
		C[c + 1] = b;
		C[c + 3] = a;
		C[c + 5] = c;
		b = 0;
		c += 3;
		goto try;
	}
	if (B[b] != A[a]) goto try;
	do a++; while (A[a] < 33);
	b++;
	if (B[b] != 10) goto parent;

	int d = C[c + 2];
	if (d != -1) {
		C[c + 1] = b;
		C[c + 3] = a;
		C[c + 5] = C[d + 2];
		c += 3;
		b = C[d + 1] + 1;
		goto parent;
	}
	if (a != A_length) goto try;
	return parse_success, {C, c};
}

