#include <stdio.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {

	const char* A = "daniel";
	const char* B = "\nbob\ncat\ndaniel\n";
	const int Al = strlen(A), Bl = strlen(B);

	int a = 0, b = 0, c = 0, d = 0, e = 0;
	int C[4096] = {0};

	while (a < Al and A[a] < 33) a++;

	C[c + 0] = a;
	C[c + 1] = b;

_0:
	if (b == Bl) {
		if (not c) return 1;
		c -= 8; 
		goto _0;
	}
	do b++; while (b < Bl and B[b] != '\n');
	a = C[c];
_1:
	e = B[b++];
	if (e == 32) { goto _0; }
	if (a >= Al or e != A[a]) goto _0;
	do a++; while (a < Al and A[a] < 33);
	if (B[b] != '\n') goto _1;
	if (c) { goto _1; }
	if (a != Al) goto _0;

	puts("compile successful.");
}

