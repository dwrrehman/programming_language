#include <stdio.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>

int main() {

	const char* A = "daniel";
	const char* B = "\nbob\ncat \ndaniel\n";
	const int al = (int) strlen(in), bl = (int) strlen(env);

	int a = 0, b = 0, c = 0, d = 4096;
	int C[4096] = {0};

	while (a < al and A[a] < 33) a++;
	C[c] = a;

_0:	if (b == bl) {
		if (!c) goto error;
		c -= 2; 
		b = C[c + 1];
		goto _0;
	}
	while (b < bl and B[b] != 10) b++; b++; 
	a = C[c];
_2:	if (B[b] == 32) {
		C[c + 1] = b;
		C[c + 2] = a;
		c += 2;
		b = 0;
		goto _0;
	}
	if (a >= al) goto _0;
	if (A[a] != B[b]) goto _0;
	do a++; while (a < al and A[a] < 33);
	b++; 
	if (B[b] != 10) goto _2;

	if (d != -1) { 
		C[c + 1] = b;
		C[c + 2] = a;
		c += 2;
		b = C[d + 1] + 1;
		goto _2; 
	}
	if (a != al) goto _0;

	puts("compile successful.");
	return 0;
error:
	puts("compile error.");
	return 1;
}

















// ---------------- garbage ----------------------------


 	// ranges in input length
	//stack[c + 1] = b; // ranges in context length
	// stack[c + 2] = c; // ranges in MAX context length   // is this really necccessary?...
	// stack[c + 3] = d; // ranges in stack


// void btucsr() {
// try:
// 	if (b == C_length) {
// 		if (c == 0) return error;
// 		c -= 3;
// 		b = C[c + 1];
// 		goto try;
// 	}
// 	while (B[b] != 10) b++; b++;
// 	a = C[c];
// parent:
// 	if (B[b] == 32) {
// 		C[c + 1] = b;
// 		C[c + 3] = a;
// 		C[c + 5] = c;
// 		b = 0;
// 		c += 3;
// 		goto try;
// 	}
// 	if (B[b] != A[a]) goto try;
// 	do a++; while (A[a] < 33);
// 	b++;
// 	if (B[b] != 10) goto parent;

// 	int d = C[c + 2];
// 	if (d != -1) {
// 		C[c + 1] = b;
// 		C[c + 3] = a;
// 		C[c + 5] = C[d + 2];
// 		c += 3;
// 		b = C[d + 1] + 1;
// 		goto parent;
// 	}
// 	if (a != A_length) goto try;
// 	return parse_success, {C, c};
// }