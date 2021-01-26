#include <stdio.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main() {s
	const char* initial = "bob\ncat\ndaniel\n", * input = "daniel";
	
	int
		length = (int) strlen(input),

		program_limit = 4096, 
		context_limit = 4096, 
		argument_limit = 4096, 
		stack_limit = 4096,

		context_count = (int) strlen(initial),
		program_count = 0, 
		arg = 0, 
		top = 0, 
		begin = 0, 
		count = 0, 
		index = 0, 
		element = 0;
	
	int* program = malloc(program_limit * sizeof(int));
	int* arguments = malloc(argument_limit * sizeof(int));
	int* stack = malloc(stack_limit * sizeof(int));
	char* context = malloc(context_limit);

	strcpy(context, initial);

	printf("input : <<<%s>>>\n", input);
	printf("context : <<<%s>>>\n", context);

	while (begin < length and input[begin] < 33) begin++;

	stack[top] = 0;
	stack[top + 3] = begin;
try:
	if (index == context_count) {
		if (not top) goto error;
		top -= 8; 
		goto try;
	}
	count = 0;
	begin = stack[top + 3];
parent:
	element = context[index++];
	if (element == ' ') {
		top += 8;
		stack[top + 1] = index;
		stack[top + 3] = begin;
		stack[top + 5] = count;
		stack[top + 6] = arg;
		arg += count;
		goto try;
	}

	if (begin >= length or element != input[begin]) goto try;

	do begin++; while (begin < length and input[begin] < 33);

	if (context[index] != '\n') goto parent;

	program[program_count] = index;
	program[program_count + 1] = count;
	memcpy(program + program_count + 2, arguments + arg, sizeof(int) * (size_t) count);

	if (top) {
		int save = count;
		index = stack[top + 1];
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
error:
	printf("\n\tcompile failure.\n\n");
final:
	for (int i = 0; i < program_count + count + 10; i++)
		printf("%d ", program[i]);
	free(context);
	free(program);
	free(arguments);
	free(stack);
}

