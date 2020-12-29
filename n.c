#include <stdio.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

// typedef i8 name[64]; // bug: this is missing:  ".def"
// typedef i16 expr[32];

struct expr {
	i16 args[30];
	i16 index;
	i16 count;
};

struct name {
	i8 syntax[60];
	i16 def;
	i8 length; // 0 through 60. 
	i8 type; //  0 through 32
};

struct el {     // whats in a stack?
	struct expr data; // nicety for now.
	i32 begin; // in file.	
	i8 type; // 0 through 32
	i8 done; // 0 through 60
	i16 __padding;
};

int main(int argc, const char** argv) {

	if (argc < 2) return 1;
	const char* filename = argv[1];

	struct stat file_data = {0};
	int file = open(filename, O_RDONLY);

	if (file < 0 or stat(filename, &file_data) < 0) {
		fprintf(stderr, "compiler: error: %s: ", filename);
		perror("open");
		exit(3);
	}

	i32 length = (i32) file_data.st_size;
	char* input = mmap(0, length, PROT_READ, MAP_SHARED, file, 0);
	if (text == MAP_FAILED) {
		fprintf(stderr, "compiler: error: %s: ", filename);
		perror("mmap");
		exit(4);
	}
	close(file);
	
	i8 top_level_type = 1;
	
	struct el* stack = malloc(65536 * sizeof(struct el));
	i16 top = 0;

	struct expr* program = malloc(65536 * sizeof(struct expr)); 
	i16 program_count = 0; 

	struct name* context = malloc(65536 * sizeof(struct expr)); 
	i16 context_count = 0; 

	context[context_count++] = (struct name) {
		.syntax = "undef", .def = 0, .length = 5, .type = 1,
	};

	context[context_count++] = (struct name) {
		.syntax = "unit", .def = 0, .length = 4, .type = 1,
	};

	context[context_count++] = (struct name) {
		.syntax = "join\x01\x01", .def = 0, .length = 6, .type = 1,
	};

	context[context_count++] = (struct name) {
		.syntax = "hello", .def = 0, .length = 5, .type = 1,
	};

	context[context_count++] = (struct name) {
		.syntax = "define\x01", .def = 0, .length = 7, .type = 1,
	};

	context[context_count++] = (struct name) {
		.syntax = "wef", .def = 0, .length = 3, .type = 1,
	};

	i32 begin = 0;	
	i32 best = 0;

	i16 index = 0;
	i16 candidate = 0;

	i8 done = 0;
	i8 error = 0;

	while (begin < length and input[begin] <= 32) begin++;
	if (begin > best) best = begin;
	
	stack->data = (struct expr) {.index = context_count};
	stack->type = top_level_type;
	stack->begin = begin;

try:
	if (not stack[top].data.index) {
		if (not top) {
			error = 1;
			goto end;
		}
		top--;
		goto next;
	}
	stack[top].data.index--;
	done = 0;
	begin = stack[top].begin;
parent:
	index = stack[top].data.index;
	struct name name = context[index];
	if (stack[top].type != name.type) goto next;
	while (done < name.length) {
		i8 c = name.syntax[done++];
		if (c < 33) {
			top++;
			stack[top].data.index = context_count;
			stack[top].data.count = 0;
			stack[top].type = c;
			stack[top].done = done;
			stack[top].begin = begin;
			goto try;
		}
		if (begin >= length or c != input[begin]) goto next;
		do begin++; while (begin < length and input[begin] <= 32);
		if (begin > best) { best = begin; candidate = index; } 
	}
	if (top) {
		done = stack[top--].done;
		stack[top].data.args[stack[top].data.count++] = program_count;
		program[program_count++] = stack[top + 1].data;
		goto parent;
	}
	if (begin == length) goto end;
next:
	stack[top].data.count = 0;
	goto try;

end:
	program[program_count++] = stack[top].data;

	printf("\n--------- program: -------- \n");
	for (int i = 0; i < program_count; i++) {
		struct expr e = program[i];
		printf("%d | index=%d, count=%d, [ ", i, e.index, e.count);
		for (int j = 0; j < e.count; j++) 
			printf("%d ", e.args[j]);
		printf("]\n");
	}

	printf("\n--------- context: -------- \n");
	for (int i = 0; i < context_count; i++) {
		struct name n = context[i];
		printf("%d | type=%d, length=%d, def=%d, [ ", i, n.type, n.length, n.def);
		for (int j = 0; j < n.length; j++) 
			if (n.syntax[j] >= 33) printf("%c ", n.syntax[j]);
			else printf("(%d) ", n.syntax[j]);
		printf("]\n");
	}
	printf("-----------------------------\n\n");

	if (error) {

		i32 at = 0, line = 1, column = 1;
		while (at < best) {
			if (input[at] == '\n') { line++; column = 1; } else column++;
			at++;
		}
	
		fprintf(stderr, "compiler: %s: %u:%u: error: unresolved %c\n",
		"filename", line, column, best == length ? ' ' : input[best]);
		
		struct name candidate_name = context[candidate];
		
		printf("...did you mean:  ");
		for (i8 s = 0; s < candidate_name.length; s++) {
			i8 c = candidate_name.syntax[s];
			if (c < 33) printf("(%d) ", c);
			else printf("%c ", c);
		}

		printf(" which has type (%d)\n", candidate_name.type);

	} else {
		printf("\n\tcompile successful.\n\n");
	}

	free(stack);
	free(program);
	free(context);
}
