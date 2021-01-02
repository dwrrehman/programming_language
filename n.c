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

struct el {
	i32 begin;
	i16 ind;
	i8 type;
	i8 done;
};

#define whitespace(c) (uint8_t)c < 33

static inline void print_program(i16* program, i16 p, int depth, i8* context) { // debug
	for (int i = 0; i < depth; i++)  printf(".   ");
	i16 ind = program[64 * p];
	i16 count = program[64 * p + 1];
	printf("[%d] : (%d) : %.*s\n\n", ind, count, 
		context[128 * ind], context + 128 * ind + 1);
	for (i16 i = 0; i < count; i++) 
		print_program(program, program[64 * p + i + 2], depth + 1, context);
}

// static inline void copy_replace(struct expression def, struct expression call, struct expression* out,
// 				struct context* context, struct stack_element* stack, nat top) {

// 	if (def.index >= call.index - call.count and 
// 	    def.index <  call.index) { 

// 		*out = call.args[call.count - (call.index - def.index)];

// 	} else {
// 		*out = def;
// 		out->args = calloc((size_t) def.count, sizeof(struct expression));

// 		for (nat i = 0; i < def.count; i++) 
// 			copy_replace(def.args[i], call, out->args + i, context, stack, top);
// 	}

// }

// static inline void expand_macro(struct context* context, struct stack_element* stack, nat top) {
// 	struct expression call = stack[top].data;	
// 	copy_replace(context->names[call.index].def, call, &stack[top].data, context, stack, top);
// }


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
	
	i8* context = malloc(32768 * 128);               
	i16* program = malloc(32768 * 128);
	i16* stack_data = malloc(32768 * 128);
	struct el* stack = malloc(32768 * 8);
	i16* macros = malloc(32768 * 2);
	i16* indicies = malloc(32768 * 2);

	i32 top = 0, program_count = 0, index_count = 0;

	i8 top_level = 2;

	const char* spellings[] = {
		"\5int\1\0\2", // 0
		"\1.\1",
		"\7param\1\1\1",
		"\6join\2\2\2",
		"\3nop\2",
		"\2!\1\1", // 5
		"\2a\1\1",
		"\2b\1\1",
		"\2c\1\1",
		"\2d\1\1",
		"\2@\1\1", // 10
		"\2A\1\1",
		"\2B\1\1",
		"\2C\1\1",
		"\2D\1\1", // 14
	NULL};

	i16 intrinsics[] = {1, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 4, 0, 3, 2};

	for (int i = 0; spellings[i]; i++) { 
		memcpy(context + 128 * index_count, spellings[i], (size_t) (spellings[i][0] + 2));
		macros[index_count] = 0;
		index_count++; 
	}

	memcpy(indicies, intrinsics, sizeof(i16) * (size_t) index_count);

	const char* reason = NULL;
	i32 begin = 0, best = 0;
	i16 index = 0, candidate = 0;
	i8 done = 0;
	
	while (begin < length and whitespace(input[begin])) begin++;
	if (begin > best) best = begin;
	*stack = (struct el) {.begin = begin, .ind = (i16) index_count, .type = top_level};
try:
	if (not stack[top].ind) {
		if (not top) { reason = "unresolved expression"; goto error; } 
		else { top--; goto try; }
	}
	stack[top].ind--;
	stack_data[64 * top + 1] = 0;
	done = 0;
	begin = stack[top].begin;	
parent:
	index = indicies[stack[top].ind];
	stack_data[64 * top] = index;
	i8* name = context + 128 * index;	
	if (stack[top].type and stack[top].type != name[*name + 1]) goto try;
	while (done < *name) {
		i8 c = name[++done];
		if (whitespace(c)) {
			if (top == 32767) { reason = "depth limit exceeded (32767)"; goto error; }
			top++;
			stack[top].begin = begin;
			stack[top].ind = (i16) index_count;
			stack[top].type = c;
			stack[top].done = done;
			goto try;
		}
		if (begin >= length or c != input[begin]) goto try;
		do begin++; while (begin < length and whitespace(input[begin]));
		if (begin > best) { best = begin; candidate = index; } 
	}
	
	if (not index) {
		printf("CALLED INTRINISC:\n");
		if (index_count == 32767) { reason = "context limit exceeded (32767)"; goto error; }
		i8* new = context + 128 * index_count;
		*new = 0;
		i16 second = stack_data[64 * top + 3];
		i16 p = stack_data[64 * top + 2];
		while (program[64 * p + 1]) {
			
			if (*new == 127) { reason = "signature limit exceeded (127)"; goto error; }
			i16 count = program[64 * p + 1];
			i16 i = count == 2 ? program[64 * p + 2] : p;
			i8 c = context[128 * program[64 * i] + 1];
			if (count == 2) c -= 64; 
			new[++*new] = c;
			printf("just appended: %d : %c\n", (int) c, (char) c);
			p = program[64 * p + count + 1];
		}
		if (not *new) {
			printf("undefining signature...\n");
			i16 i = 128 * program[64 * second];
			context[i + context[i]] = 0;
		} else {
			printf("DEFINING signature...\n");
			--*new;
			i32 place = index_count;
			while (place and *new < context[128 * indicies[place - 1]]) place--;
			memmove(indicies + place + 1, indicies + place, 
				sizeof(i16) * (size_t) (index_count - place));
			indicies[place] = (i16) index_count;
			for (i16 s = 0; s <= top; s++) if (place <= stack[s].ind) stack[s].ind++;

			if (program[64 * second]) printf("giving macro definition!! = %d\n", second);

			macros[index_count++] = program[64 * second] ? second : 0;
		}
	}

	if (program_count == 32767) { reason = "expression limit exceeded (32767)"; goto error; }
	memcpy(program + 64 * program_count, stack_data + 64 * top,
		(size_t) (2 * (stack_data[64 * top + 1] + 2)));
	program_count++;

	if (top) {
		done = stack[top].done; top--;
		if (stack_data[64 * top + 1] >= 62) { reason = "argument limit exceeded (62)"; goto error; }
		stack_data[64 * top + stack_data[64 * top + 1] + 2] = (i16) program_count - 1;
		stack_data[64 * top + 1]++;
		goto parent;
	} 
	if (begin != length) goto try;

	printf("\n\tcompile successful.\n\n");

	const char* file_head = 
	"	.section	__TEXT,__text,regular,pure_instructions\n"
	"	.build_version macos, 11, 0	sdk_version 11, 1\n"
	"	.globl	_main\n"
	"	.p2align	4, 0x90\n"
	"_main:\n";

	const char* file_tail = 
	"	mov $5, %rax\n"
	"	retq\n"
	"\n";

	int fd = open("out.s", O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0) {
		printf("compile: error: %s: ", "filename");
		perror("open");
		exit(1);
	}

	write(fd, file_head, strlen(file_head));
	
	i16 stack_count = 0;
	stack[stack_count++].ind = (i16) program_count - 1;
	
	while (stack_count) {
		i16 e = stack[--stack_count].ind;
		index = program[64 * e];
			printf("stack_count=%d | (expr=%d) : looking at %d (%.*s) (count=%d)\n", 
			stack_count, e, index, 
			context[128 * index],
			context + 128 * index + 1, 
			program[64 * e + 1]);
		// }

		for (i16 i = program[64 * e + 1]; i--;) 
			stack[stack_count++].ind = program[64 * e + 2 + i];
	}

	write(fd, file_tail, strlen(file_tail));
	close(fd);

	goto final;
error:;

	i32 at = 0, line = 1, column = 1;
	while (at < best) {
		if (input[at++] == '\n') { line++; column = 1; } else column++;
	}

	fprintf(stderr, "\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n", argv[1], line, column, reason);

	if (candidate) {
		i8* n = context + 128 * candidate;
		printf("candidate: ");
		for (i8 s = 0; s < *n + 1; s++) {
			i8 c = n[s];
			if (whitespace(c)) printf(" (%d) ", c);
			else putchar(c);
		}
		puts("");
	}

	for (i32 b = line > 2 ? line - 2 : 0, e = line + 2, i = 0, l = 1, c = 1; i < length + 1; i++) {
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
	printf("\n--------- program: -------- \n");
	for (int i = 0; i < program_count; i++) {
		i16* e = program + 64 * i;
		printf("%d | index=%d : \"%.*s\", count=%d, [ ", i, *e, context[128 * *e + 0], context + 128 * *e + 1, e[1]);
		for (int j = 0; j < e[1]; j++) 
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
		printf("%d | (length=%d) [ ", i, *n);
		for (int j = 0; j < *n + 2; j++) {
			i8 c = n[j];
			if (whitespace(c)) printf(" (%d) ", c);
			else putchar(c);
		}
		printf(" ] \n");
		if (macros[i]) {
			printf("MACRO DEF: \n");
			print_program(program, macros[i], 0, context);
			printf("END MACRO\n");
		}
	}
	printf("-----------------------------\n\n");
	if (program_count) print_program(program, (i16) program_count - 1, 0, context);
	munmap(input, (size_t) length);
	free(context);
	free(program);
	free(stack_data);
	free(stack);	
	free(macros);
	free(indicies);
}




// im thinking about not having types anymore... hmm...... i mean... floating point and integer types are basically the only two types, lol... and i guess labels!! ..? hmm yeah. and unit, and thats it. and names too.


// types:            u, n, f, s, k,          unit, int, float, label, name



// okay maybe we wont ditch the type system. its actually extremely useful!! 
// espeically in the case that you except a source location, as opposed to an integer. those ints mean differnet thigns. 

// and obviously, having types makes NAME oarsing much more reliable. 

// and indeed, everything becomes just a little bit faster with types. i think. 

// unit is pretty common,if an instruction doesnt return a value. 

// however, there need to be functions to cast a value to void, i guess? well, maybe not... i dont know.. 

// thinking in terms of expressions is quite different from assembly.... hmm....












// if (index == i_del) context[128 * i_end + 1] = 
		// context[128 * program[64 * stack_data[64 * top + 2]] + 1];

	// if (index == i_do) {
		// 	printf("found a nop instruction...\n");
		// 	const char* string = "	nop\n";
		// 	write(fd, string, strlen(string));

		// } else { 
			// printf("found an unknown instruction...\n");
