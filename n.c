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
// typedef int64_t i64;

enum {
	_end,
	_i0,
	_a, 
	_b, 
	_c,
	_join,
	_nop, 
	_del,
	_def,
};

struct expr {
	i16 args[30];
	i16 index; // todo make this use args[count] instead of this member.
	i16 count;
};

struct name {
	i8 syntax[63];
	i8 length;
};

struct macro_def {
	i16 index;
	i16 def;
};

struct el {
	struct expr data; // problem spot
	i32 begin; 
	i8 type; 
	i8 done; 
	i16 __padding; // problem spot
};

struct cg_el {
	i16 index; 
	i8 _register; 
 	i8 __padding; // problem spot
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
	char* input = not length ? 0 : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) {
		fprintf(stderr, "compiler: error: %s: ", filename);
		perror("mmap");
		exit(4);
	}
	close(file);

	struct el* stack = malloc(32768 * sizeof(struct el));
	struct expr* program = malloc(32768 * sizeof(struct expr)); 
	struct name* context = malloc(32768 * sizeof(struct expr)); 
	i16 top = 0;
	i16 program_count = 0; 
	i16 context_count = 0;
	
	context[context_count++] = (struct name) {".\x01", 1}; 		// variable delimiter. 0
	context[context_count++] = (struct name) {"_\x01\x01", 2}; 	// i0 parameter designator.  1
	context[context_count++] = (struct name) {"a\x01\x01", 2}; 	// character literal 'a'. 2
	context[context_count++] = (struct name) {"b\x01\x01", 2}; 	// character literal 'a'. 2
	context[context_count++] = (struct name) {"c\x01\x01", 2}; 	// character literal 'a'. 2
	
	context[context_count++] = (struct name) {"join\x01\x01\x01", 6}; // join statements 3
	context[context_count++] = (struct name) {"nop\x01", 3}; // 4
	context[context_count++] = (struct name) {"del\x01\x01", 4}; // change delimiter. 5
	context[context_count++] = (struct name) {"def\x01\x01", 4}; // define symbol. 6

	// context[context_count++] = (struct name) {"attach\x00\x01", 7}; // attach definition.

	i32 begin = 0, best = 0;
	i16 index = 0, candidate = 0;
	i8 done = 0, error = 0;

	while (begin < length and input[begin] < 33) begin++;
	if (begin > best) best = begin;
	
	stack[0] = (struct el) {
		.data = (struct expr) {.index = context_count, .count = 0},
		.type = 1,
		.done = 0,
		.begin = begin,
	};
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
	if (stack[top].type and stack[top].type != name.syntax[name.length]) goto next;
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
		do begin++; while (begin < length and input[begin] < 33);
		if (begin > best) { best = begin; candidate = index; } 
	}

	if (index == _del) context[_end].syntax[0] = context[program[stack[top].data.args[0]].index].syntax[0];
	else if (index == _def) {
		struct name new = {0};
		for (i16 p = stack[top].data.args[0]; program[p].count; p = program[p].args[0]) {
			i16 c = program[p].index;

			if (c == _i0) new.syntax[new.length++] = (i8) c;
			else new.syntax[new.length++] = context[c].syntax[0];

			printf("------ DEBUG: p = %d, index = %d,  spellt: %s--------\n\n", 
				p, c, context[c].syntax);
		}
		new.length--;
		context[context_count++] = new;
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
		printf("%d | index=%d : \"%s\", count=%d, [ ", i, e.index, context[e.index].syntax, e.count);
		for (int j = 0; j < e.count; j++) 
			printf("%d ", e.args[j]);
		printf("]\n");
	}

	printf("\n--------- context: -------- \n");
	for (int i = 0; i < context_count; i++) {
		struct name n = context[i];
		printf("%d | (length=%d) [ ", i, n.length);
		for (int j = 0; j < n.length + 1; j++) 
			if (n.syntax[j] < 33) printf(" (%d) ", n.syntax[j]);
			else printf("%c", n.syntax[j]);

		printf("]\n");
	}
	printf("-----------------------------\n\n");

	if (error) {

		i32 at = 0, line = 1, column = 1;
		while (at < best) {
			if (input[at++] == '\n') { line++; column = 1; } else column++;
		}
	
		fprintf(stderr, "compiler: %s: %u:%u: error: unresolved %c\n",
		"filename", line, column, best == length ? ' ' : input[best]);
		
		struct name suggestion = context[candidate];
		
		printf("...did you mean: ");
		for (i8 s = 0; s < suggestion.length + 1; s++) {
			i8 c = suggestion.syntax[s];
			if (c < 33) printf(" (%d) ", c);
			else printf("%c", c);
		}
		printf("\n");

	} else {
		printf("\n\tcompile successful.\n\n");
		
		// const char* file_head = 
		// "	.section	__TEXT,__text,regular,pure_instructions\n"
		// "	.build_version macos, 11, 0	sdk_version 11, 1\n"
		// "	.globl	_main\n"
		// "	.p2align	4, 0x90\n"
		// "_main:\n";

		// const char* file_tail = 
		// "	mov $5, %rax\n"
		// "	retq\n"
		// "\n";

		// int fd = open("out.s", O_WRONLY | O_CREAT | O_TRUNC);
		// if (fd < 0) {
		// 	printf("compile: error: %s: ", "filename");
		// 	perror("open");
		// 	exit(1);
		// }
	
		// write(fd, file_head, strlen(file_head));
		
		// struct cg_el* cg_stack = malloc(65536 * sizeof(struct cg_el));
		// i16 stack_count = 0;
		// cg_stack[stack_count++].index = program_count - 1;
		
		// while (stack_count) {
		// 	i16 expr_index = cg_stack[--stack_count].index;
		// 	i16 program_index = program[expr_index].index;

		// 	if (program_index == _nop) {
		// 		printf("found a nop instruction...\n");
		// 		const char* string = "	nop\n";
		// 		write(fd, string, strlen(string));

		// 	} else { 
		// 		printf("found an unknown instruction...\n");
		// 		printf("stack_count=%d | (expr=%d) : looking at %d (%s) (count=%d)\n", stack_count, expr_index, program_index, context[program_index].syntax, program[expr_index].count);
		// 	}

		// 	for (int i = program[expr_index].count; i--; ) 
		// 		cg_stack[stack_count++].index = program[expr_index].args[i];
		// }

		// write(fd, file_tail, strlen(file_tail));
		// close(fd);
	}
	munmap(input, (size_t) length);
	free(stack);
	free(program);
	free(context);
	exit(error);
}




		// 	// if (c < 33) {
		// 	// 	// add argument.
		// 	// } else {
		// 	// 	// 
		// 	// }
		// 	// // struct expr d = program[index];
		// 	// // for (i16 i = 0; i < d.count; i++) {
		// new.syntax[new.length++] = d.args[i];
		// // 	// }
		// // }



