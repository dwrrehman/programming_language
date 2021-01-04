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
typedef uint8_t u8;
typedef int16_t i16;
typedef int32_t i32;

struct el {
	i32 begin;
	i16 ind;
	i8 type;
	i8 done;
};

static inline void print_program(i16* program, i16 p, int depth, i8* context) { // debug
	for (int i = 0; i < depth; i++)  printf(".   ");
	i16 ind = program[64 * p];
	i16 count = program[64 * p + 1];
	printf("[i=%d] : (c=%d) : %.*s\n\n", ind, count, 
		context[128 * ind], context + 128 * ind + 1);
	for (i16 i = 0; i < count; i++) 
		print_program(program, program[64 * p + i + 2], depth + 1, context);
}

int main(int argc, const char** argv) {

	if (argc < 2) return 1;
	const char* filename = argv[1];
	struct stat file_data = {0};
	int file = open(filename, O_RDONLY);

	if (file < 0 or stat(filename, &file_data) < 0) {
		fprintf(stderr, "error: %s: ", filename);
		perror("open");
		exit(3);
	}

	i32 length = (i32) file_data.st_size;
	char* input = not length ? 0 : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) {
		fprintf(stderr, "error: %s: ", filename);
		perror("mmap");
		exit(4);
	}
	close(file);

	const size_t P = 32768;
	const size_t K = 32768;
	const size_t T = 32768;
	// NOTE: must all be positive integers less than or equal to 32768.    (> 0)

	const i16 S = 128; 
	// NOTE: must be a positive power of 2 less than or equal to 128.       (> 0)
	// 	ie, can be either:    128,    64,     32   (or technically 16, or 8)

	i8* context = malloc(K * S * sizeof(i8));
	i16* macros = malloc(K * sizeof(i16));
	i16* indicies = malloc(K * sizeof(i16));
	i16* program = malloc(P * S * sizeof(i16));
	i16* stack_data = malloc(T * S * sizeof(i16));
	struct el* stack = malloc(T * sizeof(struct el));
	
	i32 top = 0, program_count = 0, index_count = 0;
	const char* reason = NULL;
	i32 begin = 0, best = 0;
	i16 index = 0, candidate = 0;
	i8 done = 0;

	{
		const char* spellings[] = {
			"\0\1",   // should be at index 127
  			"\4name\1", // stay
			"\5data0\1", // stay
			"\5decl\3\2", // after y256
			"\5def\3\2\2", // after 256
			"\6join\2\2\2", // after 256
			"\5data8\1", // stay
			"\6data16\1", // stay
			"\3nop\2",   // after 256.
			"\41(\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1)\3", // should at index. 255
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0", "\0\0", 
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0",
		NULL};

		// printf("%lu\n", sizeof(spellings) / 8);
		// abort();

		for (int i = 0; spellings[i]; i++) { 
			memcpy(context + S * index_count, spellings[i], (size_t) (spellings[i][0] + 2));
			macros[index_count++] = 0;
		}

		int k = (int) index_count;
		int limit = 256;

		for (int i = 33; i < limit; i++) {
			if (i == '(' or i == ')') {
				char string[11] = "\10n3zqx2l \1";
				string[8] = (char) i;
				memcpy(context + S * index_count, string, 11);
				macros[index_count++] = 0;
			} else {
				char string[4] = "\1 \1";
				string[1] = (char) i;
				memcpy(context + S * index_count, string, 4);
				macros[index_count++] = 0;
			}
		}

		{
			int count = 0;
			indicies[count++] = (i16) 0; 
			for (int i = k; i < k + (limit - 33); i++) {
				if (i == '(' or i == ')') continue;
				indicies[count++] = (i16) i;
			}
			indicies[count++] = '(';
			indicies[count++] = ')';
			for (int i = 1; i < k; i++) indicies[count++] = (i16) i; 
		}
		
	}

	while (begin < length and (u8)input[begin] < 33) begin++;
	if (begin > best) best = begin;
	*stack = (struct el) {.begin = begin, .ind = (i16) index_count, .type = 2};
try:
	if (not stack[top].ind) {
		if (not top) { reason = "unresolved expression"; goto error; } 
		else { top--; goto try; }
	}
	stack[top].ind--;
	stack_data[S * top + 1] = 0;
	done = 0;
	begin = stack[top].begin;	
parent:
	index = indicies[stack[top].ind];
	stack_data[S * top] = index;
	i8* name = context + S * index;	
	if (stack[top].type != name[*name + 1]) goto try;
	while (done < *name) {
		i8 c = name[++done];
		if ((u8)c < 33) {
			if (top == 32767) { reason = "depth limit exceeded (32767)"; goto error; }
			top++;
			stack[top].begin = begin;
			stack[top].ind = (i16) index_count;
			stack[top].type = c;
			stack[top].done = done;
			goto try;
		}
		if (begin >= length or c != input[begin]) goto try;
		do begin++; while (begin < length and (u8)input[begin] < 33);
		if (begin > best) { best = begin; candidate = index; } 
	}

	if (index == 3 or index == 4) {

		if (index_count == 32767) { reason = "context limit exceeded (32767)"; goto error; }

		i8* new = context + S * index_count; 
		*new = 0;

		i16 e = stack_data[S * top + 2];
		i8 i = 0;
		while (i < program[S * e + 1]) {
			i8 c = (i8) program[S * program[S * e + 2 + i++]];
			if (*new == S - 1) { reason = "signature limit exceeded (127)"; goto error; }
			if (c != 127) new[++*new] = c; else break;
		}


		{
			i8* n = new;
			printf(" ---> found (length=%d) [ ", *n);
			for (int j = 0; j < *n; j++) {
				i8 c = n[j + 1];
				if ((u8)c < 33) printf(" (%d) ", c);
				else putchar(c);
			}
			printf(" ]\n\n");
		}

		// if (not *new) {
		// 	i16 i = 128 * program[S * second];
		// 	context[i + context[i] + 1] = 0;

		--*new; i32 place = index_count;
		while (place and *new < context[S * indicies[place - 1]]) place--;
		memmove(indicies + place + 1, indicies + place, sizeof(i16) * (size_t) (index_count - place));
		indicies[place] = (i16) index_count;
		for (i16 s = 0; s <= top; s++) if (place <= stack[s].ind) stack[s].ind++;
		macros[index_count++] = 0; // program[S * second] ? second : 

	}
	//TODO: DONT PUSH expr with    index == 0
	if (program_count == 32767) { reason = "expression limit exceeded (32767)"; goto error; }
	memcpy(program + S * program_count, stack_data + S * top,
		(size_t) (2 * (stack_data[S * top + 1] + 2)));
	program_count++;

	if (top) {
		done = stack[top].done; top--;
		//TODO: DONT PUSH expr with    index == 0
		if (stack_data[S * top + 1] >= S - 2) { reason = "argument limit exceeded (126)"; goto error; }
		stack_data[S * top + stack_data[S * top + 1] + 2] = (i16) program_count - 1;
		stack_data[S * top + 1]++;
		goto parent;
	} 
	if (begin != length) goto try;

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
	// 	printf("error: %s: ", "filename");
	// 	perror("open");
	// 	exit(1);
	// }
	// write(fd, file_head, strlen(file_head));
	// i16 stack_count = 0;
	// stack[stack_count++].ind = (i16) program_count - 1;
	// while (stack_count) {
	// 	i16 e = stack[--stack_count].ind;
	// 	index = program[S * e];
	// 	// printf("stack_count=%d | (expr=%d) : looking at %d (%.*s) (count=%d)\n", 
	// 	// 	stack_count, e, index, context[S * index], context + S * index + 1, program[S * e + 1]);
	// 	for (i16 i = program[S * e + 1]; i--;) stack[stack_count++].ind = program[S * e + 2 + i];
	// }

	// write(fd, file_tail, strlen(file_tail));
	// close(fd);

	printf("\n\tcompile successful.\n\n");

	goto final;

error:;

	i32 at = 0, line = 1, column = 1;
	while (at < best) {
		if (input[at++] == '\n') { line++; column = 1; } else column++;
	}

	fprintf(stderr, "\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n", argv[1], line, column, reason);

	if (candidate) {
		i8* n = context + S * candidate;
		printf("candidate: ");
		for (i8 s = 0; s < *n + 1; s++) {
			i8 c = n[s + 1];
			if ((u8)c < 33) printf(" (%d) ", c);
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
	// goto finish;
final:
	printf("\n--------- program: -------- \n");
	for (int i = 0; i < program_count; i++) {
		i16* e = program + S * i;
		printf("%d | index=%d : \"%.*s\", count=%d, [ ", i, *e, context[S * *e + 0], context + S * *e + 1, e[1]);
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
		i8* n = context + S * i;	
		printf("%d | (length=%d) [ ", i, *n);
		for (int j = 0; j < *n + 2; j++) {
			i8 c = n[j];
			if ((u8)c < 33) printf(" (%d) ", c);
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
	if (program_count) 
		print_program(program, (i16) program_count - 1, 0, context);
// finish:
	munmap(input, (size_t) length);
	free(context);
	free(program);
	free(stack_data);
	free(stack);	
	free(macros);
	free(indicies);
}



/*
join int nop j24kj B n3zqx2l n3zqx2l undef
join int . j24kj A n3zqx2l n3zqx2l undef
join int a j24kj B . . undef
join nop
nop
*/




	// if (macros[index]) {
	
	// 	memcpy(stack_data + S * top, program[macros[index]], count);


	// 	i16 definition = macros[index]; // macro definition.
	// 	// stack_data[S * top + 2] // call.args0 is here!!

	// 	i16 call_count = stack_data + S * top  + 1;


	// 	i16 call_index = index;
	// 	i16* call = malloc(call_count * 2);
		
	
	// 	// memcpy(stack_data + S * top; // arg count for the call expression.

	// 	i16 stack_count = top + 1;

	// 	stack[stack_count++].ind = (i16) definition;

	// 	while (stack_count > top + 1) {
	// 		i16 e = stack[--stack_count].ind;
	// 		i16 def_index = program[S * e];

	// 		if (def_index >= call_index - call_count and def_index < call_index) { 
				
	// 			*out = call.args[call.count - (call.index - def.index)];

	// 		for (i16 i = program[S * e + 1]; i--;) 
	// 			stack[stack_count++].ind = program[S * e + 2 + i];
	// 	}

	// }


/*

// printf("stack_count=%d | (expr=%d) : looking at %d (%.*s) (count=%d)\n", 
		// 	stack_count, e, index, context[S * index], context + S * index + 1, program[64 * e + 1]);


		if (def.index >= call.index - call.count and 
		    def.index <  call.index) { 

			*out = call.args[call.count - (call.index - def.index)];

		} else {
			*out = def;
			out->args = calloc((size_t) def.count, sizeof(struct expression));

			for (nat i = 0; i < def.count; i++) 
				copy_replace(def.args[i], call, out->args + i, context, stack, top);
		}

static inline void expand_macro(struct context* context, struct stack_element* stack, nat top) {
	struct expression call = stack[top].data;	
	copy_replace(context->names[call.index].def, call, &stack[top].data, context, stack, top);
}
*/



		// while (program[64 * p + 1]) {
		// 	if (*new == 127) { reason = "signature limit exceeded (127)"; goto error; }
		// 	i16 count = program[S * p + 1], i = count == 2 ? program[S * p + 2] : p;
		// 	i8 c = context[S * program[S * i] + 1];
		// 	new[++*new] = count == 2 ? c - S : c;
		// 	p = program[S * p + count + 1];
		// }






/* 
		compiler needs 21MB to run: 
	
			32768 * S * 2 * 2 + 
			32768 * S + 
			32768 * 8 + 
			32768 * 2 * 2
	*/



