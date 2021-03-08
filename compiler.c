#include <stdio.h>    // a compiler written in c, for my programming language 
#include <iso646.h>          // made by daniel warren riaz rehman.
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>


static void print_vector(int* v, int l) {
	printf("{ ");
	for (int i = 0; i < l; i++) {
		printf("%d ", v[i]);
	}
	printf("}\n");
}

static void print_indexed_vector(int* stack, int count) {
	printf("\n------------(%d):-----------\n{\n", count);
	for (int j = 0; j < count; j++) {
		printf(" %10d: %10d\n", j, stack[j]);
	}
	printf("}\n");
}


static void print_output(int* output, int top, int index) {
	puts("\n------- output: -------");
	for (int i = 0; i < top + 4; i += 4) {
		printf("%c%c %10d :   %10di %10dp %10db %10dd \n", 
			i != top ? ' ' : '>',
			i != index ? ' ' : '@', i, 
			output[i], output[i + 1], output[i + 2], output[i + 3]);
	}
	puts("---------------------\n");
}


static void print_index(const char* m, const char* string, int length, int index) {
	printf("\n%s\t\t", m);
	for (int i = 0; i < length; i++) {
		char c = string[i];
		if (i == index) printf("\033[1;31m[%c]\033[m", c);
		else printf("%c", c);
	} 
	if (index == length) printf("\033[1;31m[T]\033[m"); 
	else printf("T"); 
	printf("\n");
}


static void debug(const char* m, const char* input, int* output, 
		  int length, int begin, int top, int index, int done) {
	printf("\n\n\n\n\n-------------%s---------------:\n",m);

	printf("\n<<<variables:>>>\n\t "
		"length = %d\n\t "
		"begin = %d\n\t "
		"top = %d\n\t "
		"index = %d\n\t "
		"done = %d\n\n", 
		length, begin, top, index, done);

	print_output(output, top, index);
	print_index("\n\n<<<begin:>>>\n\n", input, length, begin);
	print_index("\n\n<<<done:>>>\n\n", input, length, done);
	// printf("continue? "); getchar();
}

static inline void print_tree(int* program, int index, int depth, const char* input, int* output) {
	for (int _ = 0; _ < depth; _++) printf(".   ");

	int b = output[program[index]];
	if (b == 4096) {
		printf("\"");
		for (int ii = output[program[index] + 2]; input[ii] != ';'; ii++) {
			if (input[ii] == '\\') { putchar(input[ii]); ii++; }
			putchar(input[ii]);
		}
		printf("\"");
	} else {
		for (int ii = output[b + 2]; input[ii] != ';'; ii++) {
			if (input[ii] == '\\') { putchar(input[ii]); ii++; }
			putchar(input[ii]);
		} 
	}
	printf("  :  (%di,%dc)\n\n", program[index], program[index + 1]);
	for (int i = 0; i < program[index + 1]; i++) {
		print_tree(program, program[index + i + 2], depth + 1, input, output);
	}
}


static inline int is(const char* string, const char* input, int start) {
	int ii = start;
	while (input[ii] != ';' or *string != ';') {
		if (input[ii] != *string) return 0;
		ii++;
		string++;
	}
	return 1;
}


int main(const int argc, const char** argv) {
	typedef unsigned char uc;
	if (argc != 2) return printf("usage: ./compiler <input>\n");
	const int limit = 4096;
	int* output = malloc(limit * sizeof(int));
	memset(output, 0x0F, limit * sizeof(int));
	int index = 0, top = 0, begin = 0, done = 0;
	int var = 0, length = 0, where = 0, best = 0;
	struct stat file_data = {0};
	int file = open(argv[1], O_RDONLY);
	if (file < 0 or stat(argv[1], &file_data) < 0) { perror("open"); exit(3); }
	length = (int) file_data.st_size;
	char* input = not length ? 0 : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) { perror("mmap"); exit(4); }
	close(file);

i0: 	if (input[begin] == 59) goto i3;
	if (input[begin] != 92) goto i2;
i1: 	begin++;
	if ((uc)input[begin] < 33) goto i1;
i2: 	begin++;
	if ((uc)input[begin] < 33) goto i2;
	goto i0;
i3: 	begin++;
	if (begin >= length) goto i4;
	if ((uc)input[begin] < 33) goto i3;
i4:	output[top] = limit;
	output[top + 2] = 0;
	output[top + 3] = 0;
	output[top + 5] = 0;
	output[top + 6] = begin;
	top += 4;
	best = begin;
_0:  	//debug("begin", input, output, length, begin, top, index, done);
	var = output[top + 1];
	if (var) goto _1;
	goto _3;
_1:	//debug("child", input, output, length, begin, top, index, done);
	var = output[var + 3];
_2: 	var++;
	if ((uc)input[var] < 33) goto _2;
	if (input[var] == 58) goto _16;
_3:	//debug("type", input, output, length, begin, top, index, done);
	if (input[done] == 58 and input[var] == 58) goto _8;
	if (input[done] != input[var]) goto _35;
	if (input[done] != 92) goto _6;
_4: 	done++; 
	if ((uc)input[done] < 33) goto _4;
_5: 	var++;
	if ((uc)input[var] < 33) goto _5;
_6:	var++; 
	if ((uc)input[var] < 33) goto _6;
_7: 	done++; 
	if ((uc)input[done] < 33) goto _7;
	goto _3;
_8: 	//debug("valid", input, output, length, begin, top, index, done);
	done++;
	if ((uc)input[done] < 33) goto _8;
	begin = output[top + 2];
_9:	//debug("parent", input, output, length, begin, top, index, done);
	if (input[done] == 59) goto _21;
	if (input[done] != 58) goto _10;
	if (top + 7 >= limit) goto out_of_memory;
	output[top] = index;
	output[top + 3] = done;
	output[top + 5] = top;
	output[top + 6] = begin;
	top += 4;
	index = 0;
	done = 0;
	goto _0;
_10:	//debug("match", input, output, length, begin, top, index, done);
	if (input[done] != 92) goto _12;
_11: 	done++;
	if ((uc)input[done] < 33) goto _11;
_12:	if (begin >= length) goto _28;
	if (input[done] != input[begin]) goto _28;
_13: 	begin++;
	if (begin >= length) goto _14;
	if ((uc)input[begin] < 33) goto _13;
_14: 	done++;
	if ((uc)input[done] < 33) goto _14;
	if (begin <= best) goto _15; 
	best = begin; 
	where = done;
_15:	goto _9;
_16:	//debug("new", input, output, length, begin, top, index, done);
	index = limit;
_17:	if (input[begin] == 59) goto _20;
	if (input[begin] != 92) goto _19;
_18: 	begin++;
	if ((uc)input[begin] < 33) goto _18;
_19: 	begin++;
	if ((uc)input[begin] < 33) goto _19;
	goto _17;
_20:	begin++;
	if (begin >= length) goto _20_;
	if ((uc)input[begin] < 33) goto _20;
_20_:	if (begin <= best) goto _21; 
	best = begin;
	where = done;
_21:	//debug("done", input, output, length, begin, top, index, done);
	output[top] = index;
	output[top + 3] = done;
	var = output[top + 1];
	if (not var) goto _27;
	if (top + 7 >= limit) goto out_of_memory;
	top += 4;
	output[top + 1] = output[var + 1];
	output[top + 2] = begin;
	index = output[var];
	done = output[var + 3];
_22: 	done++;
	if ((uc)input[done] < 33) goto _22;
_23:	if (input[done] == 58) goto _26;
	if (input[done] != 92) goto _25;
_24: 	done++; 
	if ((uc)input[done] < 33) goto _24;
_25: 	done++;
	if ((uc)input[done] < 33) goto _25;
	goto _23;
_26:	done++;
	if ((uc)input[done] < 33) goto _26;
	goto _9;
_27:	if (begin == length) goto success;
_28:	//debug("fail", input, output, length, begin, top, index, done);
	if (index == limit) goto _34;
	var = output[index + 2];
_29:	if (input[var] == 58) goto _32;
	if (input[var] != 92) goto _31;
_30: 	var++;
	if ((uc)input[var] < 33) goto _30;
_31: 	var++;
	if ((uc)input[var] < 33) goto _31;
	goto _29;
_32:	var++;
	if ((uc)input[var] < 33) goto _32; 
	if (input[var] == 59) goto _33;
	if (input[var] == 58) goto _33;
	if (var == done) goto _35;
	goto _32;
_33:	if (var == done) goto _35;
_34:	//debug("bt", input, output, length, begin, top, index, done);
	if (not top) goto error;
	top -= 4;
	index = output[top];
	done = output[top + 3];
	goto _28;
_35:	//debug("next", input, output, length, begin, top, index, done);
	index += 4;
	if (index >= top) goto _34;
	if (output[index] != limit) goto _35;
	done = output[index + 2];
	goto _0;

success: top += 4;

	puts("success: compile successful."); 
	debug("success", input, output, length, begin, top, index, done);

	// unsigned char output_bytes[4096] = {0};

	int save = 0;
	int arguments_top = 0;
	int program_count = 0;

	// int memory[1000] = {0};
	int registers[8] = {0};
	memset(registers, 0x0A, sizeof registers);

	int arguments[8192] = {0};
	int program[8192] = {0};
	
	printf("\n---------------parsing output as tree:----------------\n\n");

	for (int i = 0; i < top; i += 4) {
		
		// printf("\n\n\n arguments = ");
		// print_vector(arguments, arguments_top + 1);
		// printf("\n program = ");
		// print_vector(program, program_count + 1);

		// printf("\nDEBUG: %10d : %10di %10dp %10db %10dd \n", i, 
			// output[i], output[i + 1], output[i + 2], output[i + 3]);

		index = output[i];
		var = output[i + 1];
		begin = output[i + 2];
		done = output[i + 3];
		
		if (index == limit) goto good;

		int index2 = output[index + 2];
		var = index2;

	fail:	if (input[var] == ':') goto more;
		if (input[var] != '\\') goto jj;
	kk: 	var++;
		if ((uc)input[var] < 33) goto kk;
	jj: 	var++; 
		if ((uc)input[var] < 33) goto jj;
		goto fail;
	more:	var++; 
		if ((uc)input[var] < 33) goto more; 
		if (input[var] == 59) goto check;
		if (input[var] == ':') goto check;
		if (var == done) goto good;
		goto more;
		
	check:	if (var == done) goto good;
		goto finished;
	good:
		arguments[++arguments_top] = 1000000;
		// stack_top++;
	finished:
		
		if (index == limit or input[done] == ';') {

			// for (int _ = 0; _ < stack_top; _++) printf(".   ");

			if (index != limit) {
				printf("%d: calling \"", i);
				for (int ii = output[index + 2]; input[ii] != ';'; ii++) {
					if (input[ii] == '\\') { putchar(input[ii]); ii++; }
					putchar(input[ii]);
				} 
				printf("\"  :  ");
			} else {
				printf("%d: UDS @ %db,  ", i, begin);
			}
			int count = 0;
			while (arguments[arguments_top] != 1000000) {
				count++;
				arguments_top--;
			}

			save = program_count;
			program[program_count++] = i;
			program[program_count++] = count;

			printf("count=%d  : ", count);
			print_vector(arguments + arguments_top + 1, count);
			printf("\n");

			for (int k = 0; k < count; k++) {
				int p = arguments[arguments_top + k + 1];
				program[program_count++] = p;
				printf("\npushed argument: %d\n", p);
			}

			if (index == limit) goto skip;

			int start = output[index + 2];

			if (is("unit:attr:label::unit:;", input, start)) {

				output[output[program[program[save + 2]]] + 3] = program[program[save + 3]];

				print_output(output, top, 0);
				printf("changed location %d...\n", output[program[program[save + 2]]] + 3);
				// getchar();
				

			} else if (is("unit:branch:label:;", input, start)) {
				printf("\nwe found an BRANCH instruction!!!\n");
				if (registers[0] < 5) {
					i = output[output[program[program[save + 2]]] + 3];
					if (not i) {
						printf("INTERNAL ERROR: branch not initialized.\n");
						getchar();
					}
					i -= 4;
				}
				
			} else if (is("unit:move:register:,:register:;", input, start)) {

				int dest = 0x0F0F0F0F, source = 0x0F0F0F0F;

				int arg1_start = output[output[program[program[save + 2]]] + 2];
				if (is("register:r0;", input, arg1_start)) dest = 0;
				else if (is("register:r1;", input, arg1_start)) dest = 1;
				else if (is("register:r2;", input, arg1_start)) dest = 2;
				else if (is("register:r3;", input, arg1_start)) dest = 3;
				else {
					printf("MOVE INS ERROR: invalid dest argument\n");
					getchar();
				}

				int arg2_start = output[output[program[program[save + 3]]] + 2];
				if (is("register:r0;", input, arg2_start)) source = 0;
				else if (is("register:r1;", input, arg2_start)) source = 1;
				else if (is("register:r2;", input, arg2_start)) source = 2;
				else if (is("register:r3;", input, arg2_start)) source = 3;
				else {
					printf("MOVE INS ERROR: invalid source argument\n");
					getchar();
				}

				registers[dest] = registers[source];

			} else if (is("unit:increment:register:;", input, start)) {
				int r = 0x0F0F0F0F;
				int arg1_start = output[output[program[program[save + 2]]] + 2];
				printf("found argument: %d");
				if (is("register:r0;", input, arg1_start)) r = 0;
				else if (is("register:r1;", input, arg1_start)) r = 1;
				else if (is("register:r2;", input, arg1_start)) r = 2;
				else if (is("register:r3;", input, arg1_start)) r = 3;
				else {
					printf("INCR INS ERROR: invalid argument\n");
					getchar();
				}

				registers[r]++;


			} else if (is("unit:decrement:register:;", input, start)) {
				int r = 0x0F0F0F0F;
				int arg1_start = output[output[program[program[save + 2]]] + 2];
				if (is("register:r0;", input, arg1_start)) r = 0;
				else if (is("register:r1;", input, arg1_start)) r = 1;
				else if (is("register:r2;", input, arg1_start)) r = 2;
				else if (is("register:r3;", input, arg1_start)) r = 3;
				else {
					printf("DECR INS ERROR: invalid argument\n");
					getchar();
				}

				registers[r]--;
			} else if (is("unit:zero:register:;", input, start)) {
				int r = 0x0F0F0F0F;
				int arg1_start = output[output[program[program[save + 2]]] + 2];
				if (is("register:r0;", input, arg1_start)) r = 0;
				else if (is("register:r1;", input, arg1_start)) r = 1;
				else if (is("register:r2;", input, arg1_start)) r = 2;
				else if (is("register:r3;", input, arg1_start)) r = 3;
				else {
					printf("ZERO INS ERROR: invalid argument\n");
					getchar();
				}

				registers[r] = 0;
			}
		skip: 	arguments_top--;
			arguments[++arguments_top] = save;
		}
	}

	print_tree(program, save, 0, input, output);

	printf("CT registers:\n{\n");
	for (int i = 0; i < 4; i++) {
		printf("\tregisters[%d] = %u\n", i, registers[i]);
	}
	printf("}\n");

	goto clean_up;

out_of_memory: 
	puts("output limit exceeded");
error:; 
	int at = 0, line = 1, column = 1;
loop: 	if (at >= best) goto done;
	if (at >= length) goto done;
	if (input[at++] == '\n') goto start;
	column++;
	goto don;
start:	line++;
	column = 1;
don:	goto loop;
done: 	fprintf(stderr, "%u %u %u parse error\n", at, line, column);
	debug("error", input, output, length, begin, top, index, done);
	print_index("left off at:", input, length, best);
	print_index("candidate:", input, length, where);
clean_up: 
	munmap(input, (size_t) length);
	free(output);
}

