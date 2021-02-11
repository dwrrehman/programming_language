#include <stdio.h>    // the n programming language compiler, written in c.
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

static inline void print_vector(int* v, int l) {
	printf("[ ");
	for (int i = 0; i < l; i++) {
		if (!(i%4)) puts("");
		printf("%10d ", v[i]);
	}
	printf("\n]\n");
}

static inline void debug(int* output, int begin, int index, int top, const char* context, int count) {
	printf("DEBUG: begin = %d, index = %d, top = %d\n", begin, index, top);
	print_vector(output, top + 4);
	printf("printing parse tree in POST-DFS...\n");
	for (int i = 0; i < top; i += 4) {
		int r = output[i + 1], length = 0;
		if (not r) {
			for (int _ = 0; _ < (output[i + 2] + 4)/4; _++) printf(".   ");
			printf("<<<%s>>> : ","USER-DEFINED SIGNATURE");
			printf("begin=%d, index=%d, parent=%d, count=%d\n\n", 
				output[i + 0], output[i + 1], output[i + 2], output[i + 3]);
			continue;
		}

		if (r >= count or context[r] != 10) continue;
		do { r--; length++; } while (r and context[r] != 10);
		r++; length--;

		for (int _ = 0; _ < (output[i + 2] + 4)/4; _++) printf(".   ");
		printf("<<<%.*s>>> : ", length, context + r);
		printf("begin=%d, index=%d, parent=%d, count=%d\n\n", 
			output[i + 0], output[i + 1], output[i + 2], output[i + 3]);
	}
	printf("parse tree complete.\n");
}

static inline void print_index(const char* context, int index) {
	int start = index;
		while (start and context[start] != '\n') start--;
		start++;
		printf("\nINDEX: ");
		for (int i = start; context[i] != '\n'; i++) {
			if (i == index) printf("[%c]", context[i]);
			if (i != index) printf(" %c ", context[i]);
		}
		printf("\n\n");
		// usleep(100000);
}

static inline void* open_file(const char* filename, int* length) {

	struct stat file_data = {0};
	const int file = open(filename, O_RDONLY);

	if (file < 0 or stat(filename, &file_data) < 0) {
		fprintf(stderr, "error: %s: ", filename);
		perror("open");
		exit(3);
	}

	*length = (int) file_data.st_size;
	if (not *length) return NULL;
	void* input = mmap(0, (size_t) *length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) {
		fprintf(stderr, "error: %s: ", filename);
		perror("mmap");
		exit(4);
	}
	close(file);
	return input;
}

int main(const int argc, const char** argv) {
	if (argc != 3) {
		printf("usage: ./n <input> <context>\n");
		return 1;
	}
	const int output_limit = 4096, context_limit = 4096;
	const char * filename = argv[1], * reason = NULL;

	int count = 0, length = 0;
	int begin = 0, index = 0, top = 0;
	int best = 0, candidate = 0;

	char* input = open_file(filename, &length);
	char* _base = open_file(argv[2], &count);
	char* context = malloc(context_limit);
	memcpy(context, _base, (size_t) count);
	munmap(_base, (size_t) count);

	int* output = malloc(output_limit * sizeof(int));
	memset(output, 0x0F, output_limit);

	while (begin < length and input[begin] < 33) begin++;
	if (begin > best) best = begin;
	output[top + 0] = begin;
	output[top + 1] = 0;
	output[top + 2] = -4;
	output[top + 3] = count;

try:	

	// printf("\n\n------------------- TRY ------------------------\n\n");
	// printf("CURRENT CONTEXT: ::::%.*s::::\n", count, context);
	// printf("STATUS: begin = %d, index = %d, top = %d\n", begin, index, top);
	// print_vector(output, top + 4);
	
	// if (index < count) print_index(context, index);
	
	// 	else 
	// 		printf("\nERROR: could not print signature, because index == count!!!\n");


	// printf("continue? "); getchar();

	if (index >= count) {
		do {
		if (not top) { reason = "unresolved expression"; goto error; }
		top -= 4;
		}	while (
				context[
				output[
				top + 1]
				] 
			!= '\n');
		
		top = output[top + 2] + 4;
		index = output[top + 1];
		goto try;
	}
	
	begin = output[top]; 
	count = output[top + 3];

	const char* expected = output[top + 2] == -4 ? "init " : context + output[output[top + 2] + 1] + 1;
	const char* undefined = "undefined ", * copy_expected = expected;

	while (*undefined != ' ') {
		if (*undefined != *copy_expected) goto non;
		copy_expected++; undefined++;
	}

	context[count++] = 'g';
	context[count++] = ' ';

	while (begin < length and input[begin] != '.') {
		context[count++] = input[begin];
		do begin++; while (begin < length and input[begin] < 33);
	}
	context[count++] = '\n';
	do begin++; while (begin < length and input[begin] < 33);

	// printf("%c\n", input[begin]);
	// printf("i found it!!! an undef param.\n");
	// printf("NOW NEW: CONTEXT: ::::%.*s::::\n", count, context);
	// printf("NEW SIG: begin = %d, index = %d, top = %d\n", begin, index, top);
	// print_vector(output, top + 4);
	// usleep(1000000);
	// printf("top = %d, parent = %d\n", top, parent);
//printf("type mismatch ci=%hhu,%c, ex=%hhu,%c\n", context[index],context[index], *expected,*expected);
	// puts("\n\n\n");
	
	goto done;
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
non: 	
	while (index < count and context[index] != 10) index++; index++;
	while (context[index] != ' ') {
		if (context[index] != *expected) goto try;
		expected++; index++;

		// if (index < count) print_index(context, index);
	}
	index++;

parent:	
	// if (index < count) print_index(context, index);

	if (top + 7 >= output_limit) { reason = "program limit exceeded"; goto error; }
	if (context[index] == 10) goto done;
	if (context[index] == 32) {
		output[top + 1] = index;
		output[top + 4] = begin;
		output[top + 6] = top;
		output[top + 7] = count;
		top += 4; index = 0;
		goto try;
	}
	if (index >= count or begin >= length or context[index] != input[begin]) goto try;
	do begin++; while (begin < length and input[begin] < 33); index++;

	// if (index < count) print_index(context, index);

	if (begin > best) { best = begin; candidate = index; }
	goto parent;
done:;	
	int parent = output[top + 2];
	if (parent != -4) {
		output[top + 1] = index;
		output[top + 4] = begin;
		output[top + 6] = output[parent + 2];
		output[top + 7] = count;
		top += 4; index = output[parent + 1] + 1;
		while (index < count and context[index] != 32) index++; index++;
		goto parent;
	}

	if (begin != length) goto try;
	output[top + 0] = begin;
	output[top + 1] = index;
	output[top + 3] = count;
	top += 4;

	puts("\n\t---> compile successful.\n");
	// printf("generating code...\n");

	goto final;

error:;
	int at = 0, line = 1, column = 1;
	while (at < best) {
		if (input[at++] == '\n') { line++; column = 1; } else column++;
	}
	fprintf(stderr, "\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n", 
			filename, line, column, reason);

	int b = line > 2 ? line - 2 : 0, e = line + 2;
	for (int i = 0, l = 1, c = 1; i < length + 1; i++) {
		if (c == 1 and l >= b and l <= e) 
			fprintf(stderr, "\n\033[90m%5d\033[0m\033[32m │ \033[0m", l);
		if ((i == length or input[i] != '\n') and l >= b and l <= e) {
			if (l == line and c == column) fprintf(stderr, "\033[1;31m");
			if (i < length) fprintf(stderr, "%c", input[i]);
			else if (l == line and c == column) fprintf(stderr, "<EOF>");
			if (l == line and c == column) fprintf(stderr, "\033[m");
		}
		if (i < length and input[i] == 10) { l++; c = 1; } else c++;
	}
	int start = not candidate ? 1 : candidate;
	do start--; while (start and context[start] != 10); 
	++start;
	fprintf(stderr, "\n\n\033[1m candidate:\033[m  \033[1;96m");
	while (context[start] != 32) {
		fprintf(stderr, "%c", context[start]);
		start++;		
	}
	fprintf(stderr, "%c\033[m", context[start]);
	start++;
	for (int k = start; context[k] != 10; k++) {
		if (context[k] == 32) {
			int p = k++;
			fprintf(stderr, p == candidate ? "\033[1;31m " : "\033[1;96m "); 
			while (context[k] != 32) {
				fprintf(stderr, "%c", context[k]);
				k++;
			}
			fprintf(stderr, p == candidate ? " \033[m" : " \033[m"); 
		} else {
			if (k == candidate) fprintf(stderr, "\033[1;31m");
			fprintf(stderr, "%c", context[k]);
			if (k == candidate) fprintf(stderr, "\033[m");
		}
	}
	puts("\n");	
final:
	printf("DEBUG ::::%.*s====%.*s::::\n", length, input, count, context);
	debug(output, begin, index, top, context, count);
	munmap(input, (size_t) length);
	free(context);
	free(output);
}



// (int . name)
//      ^_	   ^\n




// ( int : name ) 

// int : name ;





// dont forget to make the undefined type actually just the empt ystring, ie, an empty string triggrs the define, not the string undefined.

	// easy fix. 


