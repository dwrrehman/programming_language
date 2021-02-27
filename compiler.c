#include <stdio.h>    // a compiler written in c, for my programming language 
#include <iso646.h>          // made by daniel warren riaz rehman.
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

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


static void print_as_ast(const char* input, int length, int* output, int top, int limit) {
	for (int i = 4; i < top; i += 4) {
		int t = output[i + 3];
		if (output[i] == limit) {
				for (int _ = 0; _ < (output[i + 1])/4; _++) printf(".   ");
				printf("found %d : [--> %d] UDS!!    ", i, output[i + 1]);
				printf("defining: ");
				for (int ii = output[i + 2]; input[ii] != ';'; ii++) {
					if (input[ii] == '\\') { putchar(input[ii]); ii++; }
					putchar(input[ii]);
				} 
				puts("\n");
		} else {
			if (t < length) {
				if (input[t] == ';') {
					for (int _ = 0; _ < (output[i + 1])/4; _++) printf(".   ");
					printf("found(i=%d) : [--> parent=%d]   index=%d  ", i, output[i + 1], output[i]); 
					printf("calling: ");
					for (int ii = output[output[i] + 2]; input[ii] != ';'; ii++) {
						if (input[ii] == '\\') { putchar(input[ii]); ii++; }
						putchar(input[ii]);
					} 
					puts("\n");
				} else {
					// printf(" (intermediary): %10d : %10di %10dp \n", i,  output[i], output[i + 1]);
				}
			} else {
				printf(" err: %10d : %10di %10dp %10db %10dd \n", i,  output[i], output[i + 1], output[i + 2], output[i + 3]);
			}
		}
		
	}
}

static void* open_file(const char* filename, int* length) {
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

int main(const int argc, const char** argv) {
	typedef unsigned char uc;
	if (argc != 2) return printf("usage: ./compiler <input>\n");
	const int limit = 4096;
	int index = 0, top = 0, begin = 0, done = 0, var = 0, length = 0, where = 0, best = 0;
	char* input = open_file(argv[1], &length);
	int* output = malloc(limit * sizeof(int));
	memset(output, 0x0F, limit * sizeof(int));

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
	where = done;
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
	print_as_ast(input, length, output, top, limit);

	uc output_bytes[4096] = {0};

	int stack[4096] = {0};

	for (int i = 0; i < top; i += 4) {
		

		


		// printf("%10d : %10d %10d %10d %10d \n", i, 
		// 	output[i], output[i + 1], output[i + 2], output[i + 3]);
		
		index = output[i];
		done = output[i + 3];

		if (input[done] == ';') {
			printf("found instruction!     ");

			int start = output[index + 2];
			while (start < done) {
				putchar(input[start]);
				start++;
			}

			printf("\n");

			const char* nop_instruction = "unit:nop;";

			int ii = output[index + 2];
			while (input[ii] != ';' and *nop_instruction != ';') {
				if (input[ii] != *nop_instruction) goto next;
				ii++;
				nop_instruction++;
			}
			printf("we found an nop instruction!!!\n");
			
		}
		next: continue;
	}

	goto clean_up;

out_of_memory: 
	puts("output limit exceeded");
error:; 
	int at = 0, line = 1, column = 1;
loop: 	if (at >= best) goto done;
	if (at >= length) goto done;
	if (input[at++] == '\n') goto start;
	column++;
	goto fin;
start:	line++;
	column = 1;
fin:	goto loop;
done: 	fprintf(stderr, "%u %u %u parse error\n", at, line, column);
	// debug("error", input, output, length, begin, top, index, done);
	// print_index("left off at:", input, length, best);
	// print_index("candidate:", input, length, where);
clean_up: 	
	munmap(input, (size_t) length);
	free(output);
}




/*
lskdjflksdjflskj: ∑∑∑ ¥do it!¥ :: and , also this is çool! :: also  :: there  :top: done 
; 

∑∑∑ ¥do it!¥

	{guys, this is my ∑ type}: bob ; 

and, also this is çool!

	top: bob :{guys, this is my ∑ type}: alice ; 
also 
	top: bob :top: :top: alice\:\; ; 

there
	 bob
		bob   bob   alice 
 	 	 bob   bob   alice 
 	alice:;
done 


*/

