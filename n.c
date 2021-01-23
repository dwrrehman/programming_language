#include <stdio.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mach-o/loader.h>

/*
i=212 index=674  |  (length=3) [ a n y  (650)  ] 
i=213 index=702  |  (length=3) [ (  (650) )  (668)  ] 
i=214 index=650  |  (length=4) [ t y p e  (650)  ] 
i=215 index=656  |  (length=4) [ n o n e  (650)  ] 
i=216 index=662  |  (length=4) [ u n i t  (650)  ] 
i=217 index=668  |  (length=4) [ n a m e  (650)  ] 
i=218 index=707  |  (length=4) [ (  (668)  (650) )  (668)  ] 
i=219 index=21  |  (length=5) [ ( z 3 n q  (668)  ] 
i=220 index=28  |  (length=5) [ ) z 3 n q  (668)  ] 
i=221 index=679  |  (length=5) [ d e f  (668)  (674)  (662)  ] 
i=222 index=713  |  (length=5) [ (  (668)  (668)  (650) )  (668)  ] 
i=223 index=686  |  (length=6) [ u n d e f  (674)  (662)  ] 
i=224 index=694  |  (length=6) [ j o i n  (662)  (662)  (662)  ] 
*/

static const int type_type = 650;
static const int none_type = 656;
static const int unit_type = 662;
static const int name_type = 668;
static const int any_type = 674;
static const int define = 679;
static const int undefine = 686;

static inline void print_program(int* program, int* context, int p, int depth) {
	for (int i = 0; i < depth; i++) printf(".   ");
	int index = program[p], count = program[p + 1];
	printf("p=%d:  [i=%d] : (c=%d) : ", p, index, count);
	fflush(stdout);
	int* n = context + index;
	for (int j = 0; j <= n[0]; j++) {
		int c = n[j + 1];
		if (c < 33) printf(" char{%d} ", c);
		else if (c < 128) printf("%c ", c);
		else if (c < 256) printf(" unicode{%d} ", c);
		else printf(" (%d) ", c);
	}
	puts("\n");
	for (int i = 0; i < count; i++) print_program(program, context, program[p + i + 2], depth + 1);
}

static inline void print_context(int alphabet, int index_count, int context_count, int* context, int* indicies) {

	printf("context indexes: (index_count = %d, context_count = %d, alphabet = %d)\n", 
		index_count, context_count, alphabet);

	for (int i = 0; i < index_count; i++) {
		int index = indicies[i];
		int* n = context + index;
		printf("i=%d index=%d  |  (length=%d) [ ", i, index, n[0]);
		for (int j = 0; j <= n[0]; j++) {
			int c = n[j + 1];
			if (c < 33) printf(" char{%d} ", c);
			else if (c < 128) printf("%c ", c);
			else if (c < 256) printf(" unicode{%d} ", c);
			else printf(" (%d) ", c);
		}
		printf(" ] \n");
	}
	printf("-----------------------------\n\n");
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

int main(int argc, const char** argv) {

	if (argc < 2) return 1;
	const char* filename = argv[1], * reason = NULL;
	const int 
		program_limit = 4096,
		context_limit = 4096,
		index_limit = 4096,
		argument_limit = 4096,
		stack_limit = 4096;
	
	int* program = malloc(program_limit * sizeof(int));
	int* context = malloc(context_limit * sizeof(int));
	int* indicies = malloc(index_limit * sizeof(int));
	int* arguments = malloc(argument_limit * sizeof(int));
	int* stack = malloc(stack_limit * sizeof(int));

	int program_count = 0, context_count = 0, index_count = 0, alphabet = 0,
	    arg = 0, top = 0, begin = 0, index = 0, count = 0, done = 0, best = 0, candidate = 0;

	int length = 0;
	unsigned char* input = open_file(filename, &length);

	{
		int base_length = 0;
		int* base = open_file("i.i", &base_length);
		index_count = base[0];
		context_count = base[1];
		alphabet = base[2];
		memcpy(indicies, base + 3, sizeof(int) * (size_t) index_count);
		memcpy(context, base + index_count + 3, sizeof(int) * (size_t) context_count);
		munmap(base, (size_t) base_length);
	}

	while (begin < length and input[begin] < 33) begin++;
	if (begin > best) best = begin;
	stack[top] = index_count;
	stack[top + 2] = any_type;
	stack[top + 3] = begin;
	stack[top + 7] = program_count;
try:
	if (not stack[top]) { 
		if (not top) { reason = "unresolved expression"; goto error; }
		top -= 8; 
		goto try; 
	}
	stack[top]--;
	index = indicies[stack[top]];
	
	int actual_type = context[index + context[index] + 1],  expected_type = stack[top + 2];
	if (actual_type == none_type) goto try;
	if (expected_type != any_type and expected_type != actual_type) goto try; 

	done = 0;
	count = 0;
	begin = stack[top + 3];
	program_count = stack[top + 7];
parent:
	while (done < context[index]) {
		int element = context[index + done + 1];
		done++;

		if (element >= alphabet) {
			top += 8;
			if (top + 7 >= stack_limit) { reason = "stack limit exceeded"; goto error; } 
			stack[top] = index_count;
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
		if (begin > best) { best = begin; candidate = index; } 
	}

	if (index == undefine) { 
		int a = program[arguments[arg]]; 
		context[a + context[a] + 1] = none_type; 
	}

	if (index == define) {
		int name_length = program[arguments[arg] + 1] - 1;
		int place = index_count;
		while (place and name_length < context[indicies[place - 1]]) place--;
		memmove(indicies + place + 1, indicies + place, sizeof(int) * (size_t) (index_count - place));
		indicies[place] = context_count; index_count++;
		for (int i = 0; i <= top; i += 8) 
			if (place <= stack[i]) stack[i]++;
		context[context_count++] = name_length;
		for (int i = 0; i <= name_length; i++) {
			int this = program[arguments[arg] + i + 2];
			int ind = program[this];
			if (program[this + 1] == 0)
				context[context_count++] = ind < alphabet ? context[ind + 1] : ind;
			else if (program) {
				define_intrin();
			}
		}
	}

	// if (index_count >= index_limit) { reason = "index limit exceeded"; goto error; }
	// if (context_count + name_length + 3 >= context_limit) { reason = "context limit exceeded"; goto error; }

	if (program_count + 2 + count > program_limit) { reason = "program limit exceeded"; goto error; } 
	program[program_count] = index;
	program[program_count + 1] = count;
	memcpy(program + program_count + 2, arguments + arg, sizeof(int) * (size_t) count);

	if (top) {
		int save = count;
		done = stack[top + 1];
		index = stack[top + 4];
		count = stack[top + 5];
		arg = stack[top + 6];
		if (arg + count >= argument_limit) { reason = "argument limit exceeded"; goto error; } 
		arguments[arg + count] = program_count;
		count++;
		top -= 8;
		program_count += 2 + save;
		goto parent;
	}
	if (begin != length) goto try;
	printf("\n\tcompile successful.\n\n");

	printf("generating code...\n");

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
			printf("\n\033[90m%5d\033[0m\033[32m │ \033[0m", l);
		if ((i == length or input[i] != '\n') and l >= b and l <= e) {
			if (l == line and c == column) printf("\033[1;31m");
			if (i < length) printf("%c", input[i]);
			else if (l == line and c == column) printf("<EOF>");
			if (l == line and c == column) printf("\033[m");
		}
		if (i < length and input[i] == '\n') { l++; c = 1; } else c++;
	}
	printf("\n\n  did you mean:   ");
	int* n = context + candidate;
	for (int j = 0; j <= n[0]; j++) {
		int c = n[j + 1];
		if (c < 33) printf(" char{%d} ", c);
		else if (c < 128) printf("%c ", c);
		else if (c < 256) printf(" unicode{%d} ", c);
		else printf(" (%d) ", c);
	}
	puts("\n");
	return 1;
final:
	print_context(alphabet, index_count, context_count, context, indicies);
	print_program(program, context, program_count, 0);
	munmap(input, (size_t) length);
	free(context);
	free(program);
	free(arguments);
	free(stack);
	free(indicies);
}

