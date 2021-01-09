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
	fflush(stdout);
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

	const int 
		program_limit = 14,
		context_limit = 50,
		index_limit = 4,
		argument_limit = 4,
		stack_limit = 3 * 8;

	int* program = malloc(program_limit * sizeof(int));
	int* context = malloc(context_limit * sizeof(int));
	int* indicies = malloc(index_limit * sizeof(int));
	int* arguments = malloc(argument_limit * sizeof(int));
	int* stack = malloc(stack_limit * sizeof(int));

	int program_count = 0, context_count = 0, index_count = 0,
	    arg = 0, top = 0, begin = 0, index = 0, count = 0, type = 256, 
	    done = 0, best = 0, candidate = 0, * name = NULL, element = 0;

	int indtemplate[] = {0, 6, 13, 21};
	memcpy(indicies, indtemplate, sizeof indtemplate);
	index_count = sizeof indtemplate / sizeof(int);
	int template[] = {
		0xFFFF, 3, 'b', 'o', 'b', 256,
		0xFFFF, 4, 'c', 'a', 't', 256, 256,
		0xFFFF, 5, 'c', 'a', 't', 256, 256, 256,
		0xFFFF, 7, 'b', 'u', 'b', 'b', 'l', 'e', 's', 256,
	};
	memcpy(context, template, sizeof template);
	context_count = sizeof template / sizeof(int);

	while (begin < length and input[begin] < 33) begin++;
	if (begin > best) best = begin;
	stack[top] = index_count;
	stack[top + 3] = begin;
	stack[top + 7] = program_count;
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
	program_count = stack[top + 7];
parent:
	name = context + index;
	if (type != name[name[1] + 2]) goto try; 
	while (done < name[1]) {
		element = name[done + 2];
		done++;
		if (element >= 256) {
			top += 8;
			if (top + 7 >= stack_limit) { reason = "stack limit exceeded"; goto error; } 
			stack[top + 0] = index_count;
			stack[top + 1] = done;
			stack[top + 2] = element;
			stack[top + 3] = begin;
			stack[top + 4] = index;
			stack[top + 5] = count;
			stack[top + 6] = arg;
			stack[top + 7] = program_count;
			arg += count;
			goto try;
		}
		if (begin >= length or element != input[begin]) goto try;
		do begin++; while (begin < length and input[begin] < 33);
		if (begin > best) { best = begin; candidate = index;}
	}

	if (program_count + 2 + count > program_limit) { reason = "program limit exceeded"; goto error; } 
	program[program_count + 0] = index;
	program[program_count + 1] = count;
	memcpy(program + program_count + 2, arguments + arg, sizeof(int) * (size_t) count);

	if (top) {
		element = count;
		done = stack[top + 1]; 
		type = stack[top + 2];
		index = stack[top + 4];
		count = stack[top + 5];
		arg = stack[top + 6];
		if (arg + count >= argument_limit) { reason = "argument limit exceeded"; goto error; } 
		arguments[arg + count] = program_count;
		count++;
		top -= 8;
		program_count += 2 + element;
		goto parent;
	} 
	if (begin != length) goto try;
	printf("\n\tcompile successful.\n\n");
	print_program(program, context, program_count, 0);
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
	printf("program: "); print(program, program_count + 2 + count);	
	printf("indicies: "); print(indicies, index_count);
	printf("context: "); print(context, context_count);
	munmap(input, (size_t) length);
	free(context);
	free(program);
	free(arguments);
	free(stack);
	free(indicies);
}




/*
	todo list:
	
		x 1. get ucsr working with densely packed arrays. 

		2. make context printer?
		3. make context loader!!
			3.1. extract out a open file function. we just need it. use void pointers. 

		4. make the declare intrinsic!     (rename to def). even though its takes one arg.
		5.  test it

		6. get the other two intrinsic wworking:    attach,  and undef.
		7. test those 

		8. get macros with arguments working!!	

		9. get code generation working for simple intructions!!

		10. i think thats it, then we have a compiler!... lol

*/
