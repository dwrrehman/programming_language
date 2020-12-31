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

enum {
	_index = 0,
	_length = 0,
	_count = 1,
	_syntax0 = 1,
};

struct el {
	i32 begin;
	i16 ind;
	i8 type;
	i8 done;
};

// struct cg_el {
// 	i16 index; 
// 	i8 _register; 
//  	i8 __padding; // problem spot
// };

/// note IN ASSEMBLY:    .label:   is local scope,    label:   is global scope.


static inline void print_program(i16* program, i16 p, int depth, i8* context) {

	for (int i = 0; i < depth; i++)  printf(".   ");

	i16 ind = program[128 * p + _index];
	i16 count = program[128 * p + _count];

	printf("[%d] : (%d) : %.*s\n\n", ind, count, 
		context[128 * ind + _length], 
		context + 128 * ind + _syntax0);

	for (i16 i = 0; i < count; i++) 
		print_program(program, program[128 * p + i + 2], depth + 1, context);
}

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

	i8* context = malloc(4194304);
	i16* program = malloc(4194304);
	i16* stack_data = malloc(4194304);
	struct el* stack = malloc(262144);
	i16* macros = malloc(131072);
	i16* indicies = malloc(65536);

	/*void* memory = malloc(13041664);
	i8* context = memory;
	i16* program = memory + 4194304;
	i16* stack_data = memory + 8388608;
	struct el* stack = memory + 12582912;
	i16* macros = memory + 12845056; 
	i16* indicies = memory + 12976128;*/
	
	i16 top = 0, macro_count = 0, program_count = 0, index_count = 0;

	enum { i_error, i_name, i_i0, i_a,  i_b,  i_c, i_end, i_join, i_nop, i_del, i_def,i_attach, i_count};
	const char* spellings[] = {"\5error\0", "\4name\1\1", "\2_\1\1", "\2a\1\1", "\2b\1\1", "\2c\1\1", "\1.\1", "\6join\2\2\2", "\3nop\2", "\4del\1\2", "\4def\1\2", "\7attach\0\2", NULL};
	i16 intrinsics[] = {i_error, i_end, i_a, i_b, i_c, i_i0, i_del, i_def, i_nop, i_join, i_name, i_attach};
	for (int i = 0; spellings[i]; i++) { 
		memcpy(context + 128 * index_count, spellings[i], (size_t) (spellings[i][0] + 2));
		index_count++; 
	} memcpy(indicies, intrinsics, sizeof(i16) * (size_t) index_count);

	const char* reason = NULL;
	i32 begin = 0, best = 0;
	i16 index = 0, candidate = 0;
	i8 done = 0;
	
	while (begin < length and input[begin] < 33) begin++;
	if (begin > best) best = begin;
	*stack = (struct el) {.begin = begin, .ind = index_count, .type = 2 };
try:
	if (not stack[top].ind) {
		if (not top) { reason = "unresolved"; goto error; }
		else { top--; goto try; }
	}
	stack[top].ind--;
	stack_data[128 * top + 1] = 0;
	done = 0;
	begin = stack[top].begin;
parent:
	index = indicies[stack[top].ind];
	stack_data[128 * top] = index;
	i8* name = context + 128 * index;
	if (stack[top].type and stack[top].type != name[name[0] + 1]) goto try;
	while (done < name[0]) {
		i8 c = name[++done];
		if (c < 33) {
			if (top == 32767) { reason = "depth limit exceeded (32767)"; goto error; }
			top++;
			stack[top].begin = begin;
			stack[top].ind = index_count;
			stack[top].type = c;
			stack[top].done = done;
			goto try;
		}
		if (begin >= length or c != input[begin]) goto try;
		do begin++; while (begin < length and input[begin] < 33);
		if (begin > best) { best = begin; candidate = index; } 
	}
	if (index == i_del) context[128 * i_end + 1] = context[128 * program[128 * stack_data[128 * top + 2]] + 1];
	else if (index == i_def) {
		if (index_count == 32767) { reason = "context limit exceeded (32767)"; goto error; }
		i8* new = context + 128 * index_count;
		*new = 0;
		for (i16 p = stack_data[128 * top + 2]; program[128 * p + _count]; p = program[128 * p + 2]) {
			if (*new >= 127) { reason = "signature limit exceeded (127)"; goto error; }
			i16 i = program[128 * p + _index];
			new[++*new] = (i == i_i0) ? (i8) i : context[128 * i + 1];
		}
		if (not *new) { reason = "defining zero-length signature"; goto error; }
		--*new;
		i16 place = index_count;
		while (place and *new < context[128 * indicies[place - 1] + _length]) place--;
		memmove(indicies + place + 1, indicies + place, sizeof(i16) * (size_t) (index_count - place));
		indicies[place] = index_count;
		index_count++;
		for (i16 s = 0; s <= top; s++) if (place <= stack[s].ind) stack[s].ind++;

	} else if (index == i_attach) {
		reason = "unimplemented"; goto error;
	}

	if (program_count == 32767) { reason = "expression limit exceeded (32767)"; goto error; }
	memcpy(program + 128 * program_count, stack_data + 128 * top,
		(size_t) (2 * (stack_data[128 * top + 1] + 2)));
	program_count++;
	
	if (top) {
		done = stack[top].done; top--;
		if (stack_data[128 * top + 1] >= 62) { reason = "argument limit exceeded (62)"; goto error; }
		stack_data[128 * top + stack_data[128 * top + 1] + 2] = program_count - 1;
		stack_data[128 * top + 1]++;
		goto parent;
	}
	if (begin == length) goto success; goto try;
error:;
	i32 at = 0, line = 1, column = 1;
	while (at < best) {
		if (input[at++] == '\n') { line++; column = 1; } else column++;
	}

	fprintf(stderr, "compiler: %s:%u:%u:%c error: %s\n",
		argv[1], line, column, 
		best < length ? input[best] : ' ', reason);
	
	if (candidate) {
		i8* n = context + 128 * candidate;
		printf("suggestion: ");
		for (i8 s = 0; s < n[_length] + 1; s++) {
			i8 c = n[s];
			if (c < 33) printf(" (%d) ", c);
			else putchar(c);
		}
		puts("");
	}
	goto final;
success:
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
	// 		printf("stack_count=%d | (expr=%d) : looking at %d (%s) (count=%d)\n", 
	// 		stack_count, expr_index, program_index, 
	// 		context[program_index].syntax, program[expr_index].count);
	// 	}

	// 	for (i16 i = program[expr_index][_count] - 1; i >= 0; i--) 
	// 		cg_stack[stack_count++].index = program[expr_index].args[i];
	// }

	// write(fd, file_tail, strlen(file_tail));
	// close(fd);
final:
	printf("\n--------- program: -------- \n");
	for (int i = 0; i < program_count; i++) {
		i16* e = program + 128 * i;
		printf("%d | index=%d : \"%.*s\", count=%d, [ ", i, e[_index], context[128 * e[_index] + _length], context + 128 * e[_index] + _syntax0, e[_count]);
		for (int j = 0; j < e[_count]; j++) 
			printf("%d ", e[j + 2]);
		printf("]\n");
	}
	printf("\n--------- context: -------- \n");
	printf("indicies = (%d){ ", index_count);
	for (int i = 0; i < index_count; i++) 
		printf("%d ", indicies[i]);
	printf("}\n");
	for (int i = 0; i < index_count; i++) {
		i8* n = context + 128 * i;	
		printf("%d | (length=%d) [ ", i, n[_length]);
		for (int j = 0; j < n[_length] + 2; j++) {
			i8 c = n[j];
			if (c < 33) printf(" (%d) ", c);
			else putchar(c);
		}
		printf(" ] \n");
	}
	printf("-----------------------------\n\n");
	print_program(program, program_count - 1, 0, context);

	munmap(input, (size_t) length);
	free(stack);
	free(program);
	free(context);
	free(indicies);
	free(macros);
}
