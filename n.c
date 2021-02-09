#include <stdio.h>
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
	const char* input = "", * context = "";
	const int length = (int) strlen(input), count = (int) strlen(context);
	int output[4096]; 
	memset(output, 0x0F, sizeof output);
	int begin = 0, index = 0, top = 0;
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
	// type checking here! 
	begin = output[top];
parent:	if (context[index] == '\n') goto done;
	char c = context[index];
	if (c == ' ') {
		output[top + 1] = index;
		output[top + 3] = begin;
		output[top + 5] = top;
		top += 3; index = 0;
		goto try;
	}
	if (index >= count or begin >= length or context[index] != input[begin]) goto try;
	do begin++; while (begin < length and input[begin] < 33); index++;
	goto parent;
done:;	int parent = output[top + 2];
	if (parent != -3) {
		output[top + 1] = index;
		output[top + 3] = begin;
		output[top + 5] = output[parent + 2];
		top += 3;
		index = output[parent + 1] + 1;
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
	puts("n: error: 1:1: compile error!");
	debug(output, begin, index, top, context, count);
	return 1;
}












// this is basically the entire front end, EXECPT:

// 	1. we need to add type-strings, in artuments, and for prepended return types, in the context (seperated by space)

// 	2. we need to add   the define mode / intrinsic, of the algoirthm. where are NOT using the context to create the signatures. jus the file itself, basically. 

// 	3. as such, with #2, we need to catually have the escape characters we are going to use in order to make names:

// 		:      ;      (       )       bslsh        thats it          and possibly even less!  

//              :      ;      _       bslsh   

//      i think thats what we are going to have in order to make names builtin... i wish we could like configure this, or something... but its okay.... anyways. i think this will be good.

// 	4. finally, we also need to add the first couple of instructions we want to have:    also we need to add define, which takes a name, and also a value! then i think we are basically ready for code gen.






/*
int main() {
	const char* input = "", * context = "";
	const int length = (int) strlen(input), count = (int) strlen(context);
	int output[4096]; 
	int begin = 0, index = 0, top = 0;
	while (begin < length and input[begin] < 33) begin++;
	output[top + 0] = begin; 
	output[top + 1] = 0; 
	output[top + 2] = -3;
try:	
	if (index == count) {
		if (not top) goto error;
		top -= 3; index = output[top + 1];
		goto try;
	}
	while (index < count and context[index] != 10) index++; index++; 
	begin = output[top];
parent:	if (context[index] == '\n') goto done;
	char c = context[index];
	if (c == ' ') {
		output[top + 1] = index;
		output[top + 3] = begin;
		output[top + 5] = top;
		top += 3; index = 0;
		goto try;
	}
	if (index >= count or begin >= length or context[index] != input[begin]) goto try;
	do begin++; while (begin < length and input[begin] < 33);
	index++; goto parent;
done:;	int parent = output[top + 2];
	if (parent != -3) {
		output[top + 1] = index;
		output[top + 3] = begin;
		output[top + 5] = output[parent + 2];
		top += 3;
		index = output[parent + 1] + 1;
		goto parent;
	}
	if (begin != length) goto try;
	output[top + 1] = index;
	output[top + 3] = begin;
	top += 3;
	// use output, top.
	return 0;
error:
	puts("n: error: 1:1: compile error!");
	return 1;
}

*/

