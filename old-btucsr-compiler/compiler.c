#include <stdio.h>    // a compiler written in c, for my programming language 
#include <iso646.h>          // made by daniel warren riaz rehman.
#include <stdlib.h>		// phase 2 began on 2104106.112610
#include <string.h>             // syntax changed to "()" on 2112186.1604
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>

typedef uint8_t uc;

static inline void print_vector(int* v, int l) {
	printf("{ ");
	for (int i = 0; i < l; i++) {
		printf("%d ", v[i]);
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
	// return;	
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
	getchar();
}




int main(const int argc, const char** argv) {
	if (argc != 2) return printf("usage: ./compiler <input>\n");
	const int limit = 8192, args_limit = 64;
	int* output = malloc(limit * sizeof(int));
	memset(output, 0x0F, limit * sizeof(int));
	const char* err = NULL;
	int index = 0, top = 0, begin = 0, done = 0, var = 0, length = 0, where = 0, best = 0;
	struct stat file_data = {0};
	int file = open(argv[1], O_RDONLY);
	if (file < 0 or stat(argv[1], &file_data) < 0) { perror("open"); exit(3); }
	length = (int) file_data.st_size;
	char* input = not length ? 0 : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) { perror("mmap"); exit(4); }
	close(file);
	if (not length) goto error;
	




iloop_name: debug("iloop_name", input, output, length, begin, top, index, done);
	if (input[begin] == '.') goto iend_name; 
_i19: 	debug("_i19", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "expected char in sig"; goto error;}
	if ((uc)input[begin] < 33) goto _i19;
	goto iloop_name;
iend_name:
	debug("iend_name", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto push_initial;
	if ((uc)input[begin] < 33) goto iend_name;
push_initial:
	if (top + 7 >= limit) goto error;
	output[top] = limit;
	output[top + 2] = 0;
	output[top + 3] = 0;
	output[top + 5] = 0;
	output[top + 6] = begin;
	top += 4;
	best = begin;


begin:	debug("begin", input, output, length, begin, top, index, done);
	begin = output[top + 2];
check_character: 
	debug("check_character", input, output, length, begin, top, index, done);
	if (input[done] == '.') goto publish; 
	if (input[done] != '_') goto match; 
	if (top + 7 >= limit) goto error;
	output[top] = index;
	output[top + 3] = done;
	output[top + 5] = top;
	output[top + 6] = begin;
	top += 4;
	index = 0;
	done = 0;
	goto begin;
match:	debug("match", input, output, length, begin, top, index, done);
	if (input[done] != '\\') goto _12;
_11: 	done++;
	if ((uc)input[done] < 33) goto _11;
_12:	if (begin >= length) goto backtrack;
	if (input[done] != input[begin]) goto backtrack;
_13: 	begin++;
	if (begin >= length) goto _14;
	if ((uc)input[begin] < 33) goto _13;
_14: 	done++;
	if ((uc)input[done] < 33) goto _14;
	if (begin <= best) { best = begin; where = done; }
	goto check_character;
read_name: debug("read_name", input, output, length, begin, top, index, done);
	index = limit; 
loop_name: debug("loop_name", input, output, length, begin, top, index, done);
	if (input[begin] == '.') goto end_name; 
	if (input[begin] != '\\') goto _19;
_18: 	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _18;
_19: 	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _19;
	goto loop_name;
end_name: 
	begin++;
	if (begin >= length) goto check_if_best;
	if ((uc)input[begin] < 33) goto end_name;
check_if_best: 
	if (begin <= best) goto publish; 
	best = begin;
	where = done;
publish:
	debug("publish", input, output, length, begin, top, index, done);
	output[top] = index;
	output[top + 3] = done;
	var = output[top + 1];
	if (not var) goto check_success;
	if (top + 7 >= limit) goto error;
	top += 4;
	output[top + 1] = output[var + 1];
	output[top + 2] = begin;
	index = output[var];
	done = output[var + 3];
	do done++; while ((uc)input[done] < 33);
	goto check_character;
check_success: 
	if (begin == length) goto success;
backtrack: 
	debug("backtrack", input, output, length, begin, top, index, done);
	if (index == limit) goto pop;
	var = output[index + 2];
find_first_arg: 
	debug("find_first_arg", input, output, length, begin, top, index, done);
	if (input[var] == '.') goto check_if_first;
	if (input[var] == '_') goto check_if_first;
	if (var == done) goto next;
	var++;
	goto find_first_arg;
check_if_first:
	if (var == done) goto next;
pop:	debug("pop", input, output, length, begin, top, index, done);
	if (not top) {err = "unresolved expression"; goto error;}
	top -= 4;
	index = output[top];
	done = output[top + 3];
	goto backtrack;
next:	debug("next", input, output, length, begin, top, index, done);
	index += 4;
	if (index >= top) goto pop;
	if (output[index] != limit) goto next;
	done = output[index + 2];
	goto begin;
success: debug("success", input, output, length, begin, top, index, done); 
	top += 4;
	puts("success: compile successful."); 



	int this = 0, next = 0, count = 0;
	int* args = malloc(args_limit * sizeof(int));
	
code:	if (this >= top) goto out;
	if (output[this] == limit) {
		if ((1)) {
		printf("\n\n\n------------------------- %d ---------------------------\n", this);
		printf(" %10d : %10di %10dp %10db %10dd   : UDS :   ", 
			this, output[this + 0], output[this + 1], output[this + 2], output[this + 3]);
		int s = output[this + 2];
		int c = 0;
		do {
			putchar(input[s]);
			if (input[s] == '.') break;
			s++;
		} while (1);
		printf("\n");
		}
		goto move;
	}
	if (input[output[this + 3]] != ')') goto move;

	if ((1)) {
	printf("\n\n\n------------------------- %d ---------------------------\n", this);
	printf(" %10d : %10di %10dp %10db %10dd   :   ", 
		this, output[this + 0], output[this + 1], output[this + 2], output[this + 3]);
	int s = output[output[this] + 2];
	int c = 0;
	do {
		putchar(input[s]);
		if (input[s] == '.') break;
		s++;
	} while (1);
	printf("\n");
	}


	next = this;
	count = 0;
next_child:
	index = output[next];
	if (index == limit) goto first;
	done = output[next + 3];
	var = output[index + 2];
rskip_type: if (input[var] == '(') goto rfind_first_arg;
	var++;
	goto rskip_type;
rfind_first_arg: var++;
	if (input[var] == ')') goto rcheck_if_first;
	if (input[var] == '(') goto rcheck_if_first;
	 if (var == done) goto first;
	goto rfind_first_arg;
rcheck_if_first: if (var == done) goto first;
	args[count++] = next - 4;
	next = output[next - 3];
	goto next_child;
first:;
	print_vector(args, count);

move: 	this += 4;
	goto code;

out:	



	goto clean_up;
error:; 
	int at = 0, line = 0, column = 0, wat = 0, wline = 0, wcolumn = 0;
	while (at < best and at < length) {
		if (input[at++] != 10) { column++; } 
		else { line++; column = 0; }
	}
	while (wat < where and wat < length) {
		if (input[wat++] != 10) { wcolumn++; } 
		else { wline++; wcolumn = 0; }
	}
	fprintf(stderr, "%u %u %u  %u %u %u  %u %u  %s\n", 
			line, column, at, wline, wcolumn, wat, top, limit, err);
clean_up:
	munmap(input, (size_t) length);
	free(output);
}






/*

 iread_type_in_name: 
	debug("iread_type_in_name", input, output, length, begin, top, index, done);
	if (input[begin] == '(') goto _i15;
	begin++;
	if (begin >= length) {err = "expected ("; goto error;}
	goto iread_type_in_name;
_i15:	debug("_i15", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "eof after ( in sig"; goto error;}
	if ((uc)input[begin] < 33) goto _i15;
iloop_name: 
	debug("iloop_name", input, output, length, begin, top, index, done);
	if (input[begin] == ')') goto iend_name; 
	if (input[begin] != '(') goto iread_reg; 
_i16:	debug("_i16", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "expected argument in sig"; goto error;}
	if (input[begin] != ')') goto _i16;
_i17:	debug("_i17", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "expected ) in arg"; goto error;}
	if ((uc)input[begin] < 33) goto _i17;
	goto iloop_name;
iread_reg: 
	debug("iread_reg", input, output, length, begin, top, index, done);
	if (input[begin] != '\\') goto _i19;
_i18: 	debug("_i18", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "expected )"; goto error;}
	if ((uc)input[begin] < 33) goto _i18;
_i19: 	debug("_i19", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "expected char in sig"; goto error;}
	if ((uc)input[begin] < 33) goto _i19;
	goto iloop_name;
iend_name:
	debug("iend_name", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto push_initial;
	if ((uc)input[begin] < 33) goto iend_name;

push_initial: 
	if (top + 7 >= limit) goto error;
	output[top] = limit;
	output[top + 2] = 0;
	output[top + 3] = 0;
	output[top + 5] = 0;
	output[top + 6] = begin;
	top += 4;
	best = begin;

begin:	debug("begin", input, output, length, begin, top, index, done);
	var = output[top + 1];
	if (not var) goto type_check; 
	var = output[var + 3];
_5: 	var++;
	if ((uc)input[var] < 33) goto _5;
	if (input[var] == ')') goto read_name;
type_check: 
	debug("type_check", input, output, length, begin, top, index, done);
	if (input[done] != '(') goto compare_types;
	if (input[var] == ')') goto types_match;
	if (input[var] == '(') goto types_match;
compare_types: 
	debug("compare_types", input, output, length, begin, top, index, done);
	if (input[done] != input[var]) goto next;
_6:	var++;
	if (var >= length) goto next;
	if ((uc)input[var] < 33) goto _6;
_7: 	done++; 
	if (done >= length) goto next;
	if ((uc)input[done] < 33) goto _7;
	goto type_check;
types_match: 
	debug("types_match", input, output, length, begin, top, index, done);
	done++; 
	if ((uc)input[done] < 33) goto types_match;
	begin = output[top + 2];
check_character: if (input[done] == ')') goto publish; 
	if (input[done] != '(') goto match; 
	if (top + 7 >= limit) goto error;
	output[top] = index;
	output[top + 3] = done;
	output[top + 5] = top;
	output[top + 6] = begin;
	top += 4;
	index = 0;
	done = 0;
	goto begin;
match:	debug("match", input, output, length, begin, top, index, done);
	if (input[done] != '\\') goto _12;
_11: 	done++;
	if ((uc)input[done] < 33) goto _11;
_12:	if (begin >= length) goto backtrack;
	if (input[done] != input[begin]) goto backtrack;
_13: 	begin++;
	if (begin >= length) goto _14;
	if ((uc)input[begin] < 33) goto _13;
_14: 	done++;
	if ((uc)input[done] < 33) goto _14;
	if (begin <= best) goto skip_update_best; 
	best = begin;
	where = done;
skip_update_best: goto check_character;
read_name: 
	debug("read_name", input, output, length, begin, top, index, done);
	index = limit; 
read_type_in_name: 
	debug("read_type_in_name", input, output, length, begin, top, index, done);
	if (input[begin] == '(') goto _15;
	begin++;
	if (begin >= length) goto next;
	goto read_type_in_name;
_15:	debug("_15", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _15;
loop_name: 
	debug("loop_name", input, output, length, begin, top, index, done);
	if (input[begin] == ')') goto end_name; 
	if (input[begin] != '(') goto read_reg; 
_16:	debug("_16", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if (input[begin] != ')') goto _16;
_17:	debug("_17", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _17;
	goto loop_name;
read_reg: debug("read_reg", input, output, length, begin, top, index, done);
	if (input[begin] != '\\') goto _19;
_18: 	debug("_18", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _18;
_19: 	debug("_19", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _19;
	goto loop_name;
end_name: debug("end_name", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto check_if_best;
	if ((uc)input[begin] < 33) goto end_name;
check_if_best: if (begin <= best) goto publish; 
	best = begin;
	where = done;
publish: 
	debug("publish", input, output, length, begin, top, index, done);
	output[top] = index;
	output[top + 3] = done;
	var = output[top + 1];
	if (not var) goto check_success;
	if (top + 7 >= limit) goto error;
	top += 4;
	output[top + 1] = output[var + 1];
	output[top + 2] = begin;
	index = output[var];
	done = output[var + 3];
_20: 	done++;
	if ((uc)input[done] < 33) goto _20;
_23:	if (input[done] == ')') goto _26;
_25: 	done++;
	if ((uc)input[done] < 33) goto _25;
	goto _23;
_26:	done++;
	if ((uc)input[done] < 33) goto _26;
	goto check_character;
check_success: if (begin == length) goto success;
backtrack: 
	debug("backtrack", input, output, length, begin, top, index, done);
	if (index == limit) goto pop;
	var = output[index + 2];
skip_type: 
	debug("skip_type", input, output, length, begin, top, index, done);
	if (input[var] == '(') goto find_first_arg;
	var++;
	goto skip_type;
find_first_arg: 
	debug("find_first_arg", input, output, length, begin, top, index, done);
	var++;
	if (input[var] == ')') goto check_if_first;
	if (input[var] == '(') goto check_if_first;
	if (var == done) goto next;
	goto find_first_arg;
check_if_first:
	debug("check_if_first", input, output, length, begin, top, index, done);
	if (var == done) goto next;
pop:	debug("pop", input, output, length, begin, top, index, done);
	if (not top) {err = "unresolved expression"; goto error;}
	top -= 4;
	index = output[top];
	done = output[top + 3];
	goto backtrack;
next:	debug("next", input, output, length, begin, top, index, done);
	index += 4;
	if (index >= top) goto pop;
	if (output[index] != limit) goto next;
	done = output[index + 2];
	goto begin;
success: top += 4;
	puts("success: compile successful."); 
	debug("success", input, output, length, begin, top, index, done);



*/

