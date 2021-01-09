#include <stdio.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

static inline void print(int* vector, int length) {
	printf("(%d){ ", length);
	for (int i = 0; i < length; i++)  {
		if (vector[i] >= 64 + 32 and vector[i] < 256) 
			printf("%c ", vector[i]);
		else printf("%d ", vector[i]);
	}
	printf("}\n");
}

static inline void print_program(int* program, int* context, int p, int depth) {
	for (int i = 0; i < depth; i++) printf(".   ");
	int index = program[p], count = program[p + 1];
	printf("p=%d:  [i=%d] : (c=%d) : ", p, index, count);
	print(context + index + 2, context[index + 1]);
	puts("");
	for (int i = 0; i < count; i++) {
		print_program(program, context, program[p + i + 2], depth + 1);
	}
}

int main(int argc, const char** argv) {

	if (argc < 2) return 1;
	const char* filename = argv[1], * reason = NULL;
	struct stat file_data = {0};
	int file = open(filename, O_RDONLY);

	if (file < 0 or stat(filename, &file_data) < 0) {
		fprintf(stderr, "error: %s: ", filename);
		perror("open");
		exit(3);
	}

	int length = (int) file_data.st_size;
	unsigned char* input = not length ? 0 : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) {
		fprintf(stderr, "error: %s: ", filename);
		perror("mmap");
		exit(4);
	}
	close(file);

	int* program = malloc(4 * 4096);
	int* context = malloc(4 * 4096);
	int* indicies = malloc(4 * 4096);
	int* arguments = malloc(4 * 4096);
	int* stack = malloc(4 * 4096);
	
	int program_count = 0, context_count = 0, index_count = 0,
	    arg = 0, top = 0, begin = 0, index = 0, count = 0, type = 256, 
	    done = 0, best = 0, candidate = 0, * name = NULL;
	// todo: add ind, to make ucsr more correct.

	indicies[index_count++] = 0;
	context[context_count++] = -100;
	context[context_count++] = 3;
	context[context_count++] = 'b';
	context[context_count++] = 'o';
	context[context_count++] = 'b';
	context[context_count++] = 256;

	indicies[index_count++] = 6;
	context[context_count++] = -100;
	context[context_count++] = 1;
	context[context_count++] = 'c';
	context[context_count++] = 256;

	indicies[index_count++] = 10;
	context[context_count++] = -100;
	context[context_count++] = 5;
	context[context_count++] = 'c';
	context[context_count++] = 'a';
	context[context_count++] = 't';
	context[context_count++] = 256;
	context[context_count++] = 256;
	context[context_count++] = 256;

	indicies[index_count++] = 18;
	context[context_count++] = -100;
	context[context_count++] = 7;
	context[context_count++] = 'b';
	context[context_count++] = 'u';
	context[context_count++] = 'b';
	context[context_count++] = 'b';
	context[context_count++] = 'l';
	context[context_count++] = 'e';
	context[context_count++] = 's';
	context[context_count++] = 256;

	while (begin < length and input[begin] < 33) begin++;
	if (begin > best) best = begin;
	stack[top] = index_count;
	stack[top + 3] = begin;
try:
	if (not stack[top]) { 
		if (not top) { reason = "unresolved expression"; goto error; }
		top -= 8; goto try; 
	}
	stack[top]--;
	index = indicies[stack[top]];
	done = 0;
	count = 0;
	begin = stack[top + 3];
parent:
	name = context + index;
	if (type != name[name[1] + 2]) goto try; 
	while (done < name[1]) {
		int element = name[done + 2];
		done++;
		if (element >= 256) {
			top += 8;
			stack[top + 0] = index_count;
			stack[top + 1] = done;
			stack[top + 2] = element;
			stack[top + 3] = begin;
			stack[top + 4] = index;
			stack[top + 5] = count;
			stack[top + 6] = arg;
			arg += count;
			goto try;
		}
		if (begin >= length or element != input[begin]) goto try;
		do begin++; while (begin < length and input[begin] < 33);
		if (begin > best) { best = begin; candidate = index;}
	}

	program[program_count + 0] = index;
	program[program_count + 1] = count;
	memcpy(program + program_count + 2, arguments + arg, sizeof(int) * (size_t) count);

	if (top) {
		int save = count; // todo: figure out how this can be eliminated.
		done = stack[top + 1]; 
		type = stack[top + 2];
		index = stack[top + 4];
		count = stack[top + 5];
		arg = stack[top + 6];
		arguments[arg + count] = program_count;
		count++;
		top -= 8;
		program_count += 2 + save;
		goto parent;
	} 
	if (begin != length) goto try;

	printf("\n\tcompile successful.\n\n");
	goto final;

error:;
	int at = 0, line = 1, column = 1;
	while (at < best) {
		if (input[at++] == '\n') { line++; column = 1; } else column++;
	}

	fprintf(stderr, "\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n", 
			filename, line, column, reason);

	printf("did you mean: ");
	print(context + candidate + 2, context[candidate + 1] + 1);

	int b = line > 2 ? line - 2 : 0, e = line + 2;
	for (int i = 0, l = 1, c = 1; i < length + 1; i++) {
		if (c == 1 and l >= b and l <= e) 
			printf("\n\033[90m%5d\033[0m\033[32m │ \033[0m", l);
		if ((i == length or input[i] != '\n') and l >= b and l <= e) {
			if (l == line and c == column) printf("\033[1;31m");
			if (i < length) printf("%c", input[i]);
			else if (l == line and c == column) printf("<EOF>");
			if (l == line and c == column) printf("\033[m");
		}
		if (i < length and input[i] == '\n') { l++; c = 1; } else c++;
	}
	printf("\n\n");

final:
	printf("program: ");
	print(program, program_count);
	
	printf("indicies: ");
	print(indicies, index_count);
	
	printf("context: ");
	print(context, context_count);

	printf("tree:\n\n");
	print_program(program, context, program_count, 0);

	munmap(input, (size_t) length);
	free(context);
	free(program);
	free(arguments);
	free(stack);
	free(indicies);
}




/*
	todo list:
		

	
		1. get ucsr working with densely packed arrays. 

		2. make context printer?

		3. make context loader!!

			3.1. extract out a open file function. we just need it. use void pointers. 
	
		
		4. make the declare intrinsic!     (rename to def). even though its takes one arg.

	
		5.  test it


		6. get the other two intrinsic wworking:    attach,  and undef.


		
		7. test those 


		8. get macros with arguments working!!
	

		9. get code generation working for simple intructions!!


		10. 







*/