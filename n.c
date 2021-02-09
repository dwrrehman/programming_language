#include <stdio.h>    // the n programming language compiler, written in c.
#include <iso646.h>
#include <stdlib.h>   
#include <string.h>

static inline void print_vector(int* v, int l) {
	printf("[ ");
	for (int i = 0; i < l; i++) {
		if (!(i%3)) puts("");
		printf("%10d ", v[i]);
	}
	printf("\n]\n");
}

static inline void debug(int* output, int begin, int index, int top, const char* context, int count) {
	printf("DEBUG: begin = %d, index = %d, top = %d\n", begin, index, top);
	print_vector(output, top + 6);
	printf("printing parse tree in POST-DFS...\n");
	for (int i = 0; i < top; i += 3) {
		int r = output[i + 1], length = 0;
		if (r >= count or context[r] != 10) continue;
		do { r--; length++; } while (r and context[r] != 10);		
		r++; length--; 
		for (int _ = 0; _ < (output[i + 2] + 3)/3; _++) printf(".   ");
		printf("<<<%.*s>>> : ", length, context + r);
		printf("begin=%d, index=%d, parent=%d\n\n", output[i], output[i + 1], output[i + 2]);
	}
	printf("parse tree complete.\n");
}

int main() {
	const char
		* input = "cat daniel cat daniel cat daniel hi ", 
		* context = "\nint hi\nstring daniel\nint cat string  int \ninit  int \n";
	const int length = (int) strlen(input), count = (int) strlen(context);
	int output[8192];
	memset(output, 0x0F, sizeof output);
	int begin = 0, index = 0, top = 0, best = 0, candidate = 0;
	while (begin < length and input[begin] < 33) begin++;
	output[top + 0] = begin;
	output[top + 1] = 0;
	output[top + 2] = -3;
try:	if (index == count) {
		if (not top) goto error;
		top -= 3; index = output[top + 1];
		goto try;
	}
	while (index < count and context[index] != 10) index++; index++;
	const char* expected = output[top + 2] == -3 ? "init" : context + output[output[top + 2] + 1] + 1;
	while (context[index] != ' ') {
		if (context[index] != *expected) goto try;
		expected++; index++;
	}
	index++; begin = output[top];
parent:	if (context[index] == 10) goto done;
	char c = context[index];
	if (c == 32) {
		output[top + 1] = index;
		output[top + 3] = begin;
		output[top + 5] = top;
		top += 3; index = 0;
		goto try;
	}
	if (index >= count or begin >= length or context[index] != input[begin]) goto try;
	do begin++; while (begin < length and input[begin] < 33); index++;
	if (begin > best) { best = begin; candidate = index; }
	goto parent;
done:;	int parent = output[top + 2];
	if (parent != -3) {
		output[top + 1] = index;
		output[top + 3] = begin;
		output[top + 5] = output[parent + 2];
		top += 3; index = output[parent + 1] + 1;
		while (index < count and context[index] != 32) index++; index++;
		goto parent;
	}
	if (begin != length) goto try;
	output[top + 1] = index;
	output[top + 3] = begin;
	top += 3;

	puts("\n\t---> compile successful.\n");
	debug(output, begin, index, top, context, count);
	return 0;
error:
	printf("n: error: %d: compile error parsing %d\n\n", best, candidate);
	debug(output, begin, index, top, context, count);
	return 1;
}
