#include <stdio.h>    // the n programming language compiler, written in c.
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

static inline void print_index(const char* context, int index, int count) {
	if (index > count or index < 0) { printf("{error index}\n"); return; }
	for (int i = 0; i < count; i++) {
		char c = context[i];
		if (i == index) { if (c == 10) printf("[.]"); else printf("[%c]", c); }
		if (i != index) { if (c == 10) printf("."); else printf("%c", c); }
	} if (index == count) printf("[T]"); else printf("T"); puts("");
}

static inline void print_vector(int* output, int top, const char* context, int count) {
	for (int i = 0; i < top; i += 4) {
		printf("%10d :   %10di %10dp %10db %10dc  : ", i, output[i], output[i + 1], output[i + 2], output[i + 3]);
		print_index(context, output[i], count);
	}
}

int main() { 
	const char * input = "", * context = "\n\n";
	int length = (int) strlen(input), count = (int) strlen(context);
	int output[4096]; memset(output, 0xFF, sizeof output);
	int begin = 0, index = 0, top = 0, current = 0;
first:	if (begin >= length) goto step;
	if (input[begin] >= 33) goto step;
	begin++;
	goto first;
step: 	output[top + 1] = -1; 
	output[top + 2] = begin;
	output[top + 3] = count;




begin:	begin = output[top + 2]; 
	count = output[top + 3];
_0:	if (index == count) goto fail;
	if (context[index] == 10) goto _2;
	index++;
	goto _0;
_2:	index++;
	if (index < count) goto parent;
fail:	if (not top) goto error;
	top -= 4;
	index = output[top]; 
	current = top;
	goto begin;
parent:	if (context[index] == 10) goto done;
	if (context[index] != 32) goto match;
	top += 4;
	output[current] = index;
	output[top + 1] = current;
	output[top + 2] = begin;
	output[top + 3] = count;
	current = top;
	index = 0;
	goto begin;
match:	if (begin >= length) goto begin;
	if (context[index] != input[begin]) goto begin;
incr:	begin++; 
	if (begin >= length) goto incr2;
	if (input[begin] >= 33) goto incr2;
	goto incr;
incr2:	index++;
	goto parent;
done:	if (not current) goto check;
	output[current] = index;
	current = output[current + 1];
	index = output[current] + 1;
	goto parent;
check: 	if (begin != length) goto begin;
	puts("\n\t---> compile successful.\n");
	goto final;
error:
	printf("error: 1:1: unresolved expression\n");
final:
	printf("debug: index=%d current=%d top=%d begin=%d count=%d\n", index, current, top, begin, count);
	print_vector(output, top + 16, context, count);
}




















// print_index(context, index, count);
		// printf("debug: index=%d current=%d top=%d begin=%d count=%d\n", index, current, top, begin, count);
		// print_vector(output, top + 16, context, count);









// _5:	if (context[index] == 32) goto _6;
// 	index++;
// 	goto _5; 
// _6:	index++;




// _3: 	if (context[index] == 32) goto _4;
//  	index++;
// 	goto _3;
// _4:	index++;












	// printf("%c\n", input[begin]);
	// if (input[begin] != '(')  {
	// 	printf("error: expected ( before name.\n");
	// 	top -= 4; index = output[top + 1];
	// 	goto try;
	// }
	// do begin++; while (begin < length and input[begin] < 33);

	// if (input[begin] != '\\') {
	// } else {
		// do begin++; while (begin < length and input[begin] < 33);
		// context[count++] = input[begin];
		// do begin++; while (begin < length and input[begin] < 33);
	// }






// 	const char* expected = output[top + 2] == -4 ? "init " : context + output[output[top + 2] + 1] + 1;
// 	const char* undefined = "undefined ", * copy_expected = expected;

// 	while (*undefined != ' ') {
// 		if (*undefined != *copy_expected) goto non;
// 		copy_expected++; undefined++;
// 	}

// 	while (begin < length and input[begin] != '.') {
// 		if (input[begin] != '\\') {
// 			if (input[begin] == ':') context[count++] = '\t';
// 			else if (input[begin] == '_') context[count++] = ' ';
// 			else context[count++] = input[begin];
// 			do begin++; while (begin < length and input[begin] < 33);
// 		} else {
// 			do begin++; while (begin < length and input[begin] < 33);
// 			context[count++] = input[begin];
// 			do begin++; while (begin < length and input[begin] < 33);
// 		}
// 	}
// 	context[count++] = '\n';
// 	if (count > best_count) best_count = count;

// 	do begin++; while (begin < length and input[begin] < 33);

// 	index = context_limit;

// 	// if (begin > best) { best = begin; candidate = index; }
	
// 	// printf("i found it!!! an undef param.\n");
// 	// printf("NOW NEW: CONTEXT: ::::%.*s::::\n", count, context);
// 	// printf("NEW SIG: begin = %d, index = %d, top = %d\n", begin, index, top);
// 	// print_vector(output, top + 4);

// 	goto done;

// non: 






	// printf("\n\n------------------- PARENT ------------------------\n\n");
	// printf("CURRENT CONTEXT: ::::%.*s::::\n", count, context);
	// printf("STATUS: begin = %d, index = %d, top = %d\n", begin, index, top);
	// print_vector(output, top + 4);
	// if (index < count) print_index(context, index);
	// else printf("\nERROR: could not print signature, because index == count!!!\n");
	// printf("continue? "); getchar();



// const int argc, const char** argv



