#include <stdio.h>    // compiler wrriten in c,  for a programming language 
#include <iso646.h>        // made by daniel warren riaz rehman.
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
static void print_index(const char* context, int index, int count);
static void pretty_print_output(int* output, int top, const char* context, const char* input);
static void print_output(int* output, int top, const char* context, int count);
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

static void print_source(const char* A, int Al, int line, int column) {
	int b = line > 2 ? line - 2 : 0, e = line + 2;
	for (int i = 0, l = 1, c = 1; i < Al + 1; i++) {
		if (c == 1 and l >= b and l <= e) 
			fprintf(stderr, "\n\033[90m%5d\033[0m\033[32m │ \033[0m", l);
		if ((i == Al or A[i] != '\n') and l >= b and l <= e) {
			if (l == line and c == column) fprintf(stderr, "\033[1;31m");
			if (i < Al) fprintf(stderr, "%c", A[i]);
			else if (l == line and c == column) fprintf(stderr, "<EOF>");
			if (l == line and c == column) fprintf(stderr, "\033[m");
		}
		if (i < Al and A[i] == 10) { l++; c = 1; } else c++;
	}
}

static void print_candidate(const char* B, int Bl, int bb) {
	if (not Bl) { 
		fprintf(stderr, "\n\n{no candidate}\n\n"); 
		return; 
	}

	int s = not bb ? 1 : bb;
	do s--; while (s and B[s] != 10); 
	++s;

	fprintf(stderr, "\n\n\033[1m candidate:\033[m  \033[1;94m");
	while (B[s] != ' ' and s < Bl) {
		fprintf(stderr, "%c", B[s]);
		s++;	
	}

	fprintf(stderr, "%c\033[m", B[s]);
	s++;

	for (int k = s; B[k] != 10 and k < Bl; k++) {
		if (B[k] == 32) {
			int p = k++;
			fprintf(stderr, p == bb ? "\033[1;31m " : "\033[1;96m "); 
			while (B[k] != 32) {
				fprintf(stderr, "%c", B[k]);
				k++;
			}
			fprintf(stderr, p == bb ? " \033[m" : " \033[m"); 
		} else {
			if (k == bb) fprintf(stderr, "\033[1;31m");
			fprintf(stderr, "%c", B[k]);
			if (k == bb) fprintf(stderr, "\033[m");
		}
	}
	puts("\n");
}

int main(const int argc, const char** argv) {
	typedef unsigned char uc;
	if (argc != 3) return printf("usage: ./compiler <input> <context>\n");
	const char* filename = argv[1], * reason = NULL, * e = NULL;
	const int CL = 4096, BL = 4096; 
	int Al = 0, Bl = 0, Blb = 0, bb = 0, ab = 0;
	int a = 0, b = 0, c = 0, d = 0;
	char* A = open_file(filename, &Al);
	char* _ = open_file(argv[2], &Bl);
	char* B = malloc(BL);
	memset(B, 0x0F, (size_t) BL);
	memcpy(B, _, (size_t) Bl);
	munmap(_, (size_t) Bl);
	int* C = malloc(CL * sizeof(int));
	memset(C, 0x0F, CL * sizeof(int));
	while (a < Al and (uc)A[a] < 33) a++;
	ab = a; 
	Blb = Bl;
	C[c + 1] = -3; 
	C[c + 2] = a;

_0:	d = b;
	if (d == 0) goto _8;
	if (B[d] != 10 and B[d] != 32) goto _8;
	do d--; while (B[d] != 32);
_9: 	d--; 
	if (B[d] == 32) goto _6;
	if (B[d] != 10) goto _9; // problem: choosing to backtrack a node, which doesnt point to a space or newline?
	if (b < Bl) goto _8;
_6: 	if (not c) goto error;
	c -= 3; 
	b = C[c];
	if (b != BL) goto _0;
	Bl--; 
	do Bl--; while (B[Bl] != 10); 
	Bl++; 
	goto _0;
_8:	a = C[c + 2]; 
	while (B[b] != 10) b++; 
	b++;
	if (b >= Bl) goto _0;
	e = C[c + 1] == -3 ? "top " : B + C[C[c + 1]] + 1;
	if (*e != 32) goto _10; 
	b = BL;
	while (a < Al and A[a] != 59) {
		if (Bl + 1 >= BL) goto BLx;
		if (A[a] != 92) B[Bl++] = A[a] == 58 ? 32 : A[a];
		else { 
			do a++; while (a < Al and (uc)A[a] < 33); 
			B[Bl++] = A[a]; 
		}
		do a++; while (a < Al and (uc)A[a] < 33);
	} 
	if (a >= Al) goto _0; 
	B[Bl++] = 10;
	if (Bl > Blb) Blb = Bl;
	do a++; while (a < Al and (uc)A[a] < 33);
	if (a > ab) { 
		ab = a; 
		bb = b; 
	} 
	goto _2;
_10: 	while (B[b] != 32 or *e != 32) { 
		if (B[b] != *e) goto _0; 
		e++; 
		b++; 
	} 
	b++;
_1:	if (B[b] == 10) goto _2;
	if (B[b] != 32) goto _7;
	if (c + 5 >= CL) goto CLx;
	C[c] = b; 
	c += 3; 
	C[c + 1] = c - 3;
	C[c + 2] = a; 
	b = 0; 
	goto _0;
_7:	if (a >= Al or B[b] != A[a]) goto _0;
	do a++; while (a < Al and (uc)A[a] < 33); 
	if (a > ab) { 
		ab = a; 
		bb = b; 
	} b++; 
	goto _1;
_2:	C[c] = b; 
	d = C[c + 1];
	if (d == -3) goto _3;
	if (c + 5 >= CL) goto CLx;
	c += 3; 
	C[c + 1] = C[d + 1];
	C[c + 2] = a; 
	b = C[d] + 1;
	while (B[b] != 32) b++; 
	b++; 
	goto _1;
_3:	if (a != Al) goto _0; 
	c += 3;

	puts("success: compile successful."); 

	goto done;
BLx: 	reason = "context limit exceeded"; goto display;
CLx: 	reason = "program limit exceeded"; goto display;
error: 	reason = "unresolved expression";
display:
	Bl = Blb;
	int at = 0, line = 1, column = 1;
	while (at < ab and at < Al) {
		if (A[at++] == '\n') { 
			line++; column = 1; 
		} else column++;
	}
	fprintf(stderr, "\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n", 
		filename, line, column, reason);
	print_source(A, Al, line, column);
	print_candidate(B, Bl, bb);
done:	printf("DEBUG final context: \n<<<%.*s>>>\n", Bl, B);
	printf("debug: a=%d b=%d c=%d \n", a, b, c);
	print_output(C, c, B, Bl);
	pretty_print_output(C, c, B, A);
	munmap(A, (size_t) Al); free(B); free(C);
}



























//  --------------- debug functions ------------------

static void print_index(const char* context, int index, int count) {
	if (index == 4096) { printf("    {UD SIG}\n\n"); return; }
	if (index > count or index < 0) { printf("    {error index}\n\n"); return; }
	printf("    ");
	for (int i = 0; i < count; i++) {
		char c = context[i];
		if (i == index) { if (c == 10) printf("\033[1;31m[.]\033[m"); else printf("\033[1;31m[%c]\033[m", c); }
		if (i != index) { if (c == 10) printf("."); else printf("%c", c); }
	} 
	if (index == count) printf("\033[1;31m[T]\033[m"); else printf("T"); 
	puts("\n");
}

static void print_output(int* output, int top, const char* context, int count) {
	puts("");
	for (int i = 0; i < top; i += 3) {
		printf("%10d :   %10di %10dp %10db : ", i, output[i], output[i + 1], output[i + 2]);
		print_index(context, output[i], count);
	}
	puts("\n");
}

static void pretty_print_output(int* output, int top, 
				const char* context, const char*input) {

	printf("printing parse tree in POST-DFS...\n");
	for (int i = 0; i < top; i += 3) {
		int r = output[i + 0], length = 0;
		if (r == 4096) {
			for (int _ = 0; _ < (output[i + 1] + 3)/3; _++) printf(".   ");
			printf("UDS:  \"");
			
			int k = output[i + 2];
			while (input[k] != ';') {
				if ((unsigned char)input[k] >= 33) putchar(input[k]);
				k++;
			}
			printf(";\" \t\t\t\t: %di %dp %db \n\n", 
				output[i + 0], output[i + 1], output[i + 2]);
			continue;
		}

		if (context[r] != 10) continue;
		do { r--; length++; } while (r and context[r] != 10);
		r++; length--;

		for (int _ = 0; _ < (output[i + 1] + 3)/3; _++) printf(".   ");
		printf("(%.*s) \t\t\t\t", length, context + r);
		printf(" : %di %dp %db #%d\n\n", 
			output[i + 0], output[i + 1], output[i + 2], i);
	}
	printf("parse tree complete.\n");
}





























// -------------------- good code, that im going to put back soon!! ---------------------------


	// index = context_limit;
	// while (begin < length and input[begin] != ';') {	
	// 	if (count + 1 >= context_limit) { reason = "context limit exceeded"; goto error; }
	// 	if (input[begin] != '\\') {
	// 		context[count++] = input[begin] == ':' ? 32 : input[begin];
	// 	} else {
	// 		do begin++; while (begin < length and (unsigned char)input[begin] < 33);
	// 		context[count++] = input[begin];
	// 	}
	// 	do begin++; while (begin < length and (unsigned char)input[begin] < 33);
	// }
	// if (begin >= length) goto begin;
	// context[count++] = 10;
	// if (count > biggest) biggest = count;
	// do begin++; while (begin < length and (unsigned char)input[begin] < 33);
	// if (begin > best) { best = begin; candidate = index; }
	// goto done;



// -------------------------- code generator code, that im going to 
//                            rework after the front end is finished --------------------------

// // char output_bytes[4096] = {0};
// 	// int stack[4096] = {0};

// 	for (int i = 0; i < top; i += 4) {
// 		// printf("%d : %10d %10d %10d %10d \n", 
// 		// 	i, program[i], program[i + 1], program[i + 2], program[i + 3]);
// 		printf("%10d: %10d\n", i, program[i + 1]);
// 		// int index = program[i];
// 	}







//if (top + 7 >= program_limit) { reason = "program limit exceeded"; goto error; }

// if (*expected != 32) goto non;
	
// 	non: 	



// printf("DEBUG initial inputs: \n<<<<%.*s>>>>\n\n<<<<%.*s>>>>\n", length, input, count, context);





// 	int at = 0, line = 1, column = 1;
// 	while (at < best and at < Al) {
// 		if (A[at++] == '\n') { line++; column = 1; } else column++;
// 	}
// 	fprintf(stderr, "\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n", 
// 			filename, line, column, reason);

// 	int b = line > 2 ? line - 2 : 0, e = line + 2;
// 	for (int i = 0, l = 1, c = 1; i < Al + 1; i++) {
// 		if (c == 1 and l >= b and l <= e) 
// 			fprintf(stderr, "\n\033[90m%5d\033[0m\033[32m │ \033[0m", l);
// 		if ((i == Al or A[i] != '\n') and l >= b and l <= e) {
// 			if (l == line and c == column) fprintf(stderr, "\033[1;31m");
// 			if (i < Al) fprintf(stderr, "%c", A[i]);
// 			else if (l == line and c == column) fprintf(stderr, "<EOF>");
// 			if (l == line and c == column) fprintf(stderr, "\033[m");
// 		}
// 		if (i < Al and A[i] == 10) { l++; c = 1; } else c++;
// 	}
// 	if (not count) {
// 		printf("\n\nskipping error candidate...\n");
// 		goto skip_candidate;
// 	}
// 	int start = not candidate ? 1 : candidate;
// 	do start--; while (start and context[start] != 10); 
// 	++start;
// 	fprintf(stderr, "\n\n\033[1m candidate:\033[m  \033[1;94m");
// 	while (context[start] != ' ' and start < count) {
// 		fprintf(stderr, "%c", context[start]);
// 		start++;	
// 	}
// 	fprintf(stderr, "%c\033[m", context[start]);
// 	start++;
// 	for (int k = start; context[k] != 10 and k < count; k++) {
// 		if (context[k] == 32) {
// 			int p = k++;
// 			fprintf(stderr, p == candidate ? "\033[1;31m " : "\033[1;96m "); 
// 			while (context[k] != 32) {
// 				fprintf(stderr, "%c", context[k]);
// 				k++;
// 			}
// 			fprintf(stderr, p == candidate ? " \033[m" : " \033[m"); 
// 		} else {
// 			if (k == candidate) fprintf(stderr, "\033[1;31m");
// 			fprintf(stderr, "%c", context[k]);
// 			if (k == candidate) fprintf(stderr, "\033[m");
// 		}
// 	}
// skip_candidate: 
// 	puts("\n");









/*
 ------------------- prolgue ----------



	const int C_limit = 4096, B_limit = 4096; 
	const char * filename = argv[1];
	int Al = 0, Bl = 0, a = 0, b = 0, c = 0;
	char* A = open_file(filename, &Al);
	char* _base = open_file(argv[2], &Bl);
	char* B = malloc(B_limit);
	memset(B, 0x0F, B_limit);
	memcpy(B, _base, (size_t) Bl);
	munmap(_base, (size_t) Bl);
	int* C = malloc(C_limit * sizeof(int));
	memset(C, 0x0F, C_limit);
	while (a < Al and A[a] < 33) a++;
	C[c + 1] = -3;
	C[c + 2] = a;


if (argc != 3) {
		printf("usage: ./compiler <input> <context>\n");
		return 1;
	}
	const int C_limit = 4096, B_limit = 4096; 
	const char * filename = argv[1];

const int argc, const char** argv
// while (a < Al and A[a] < 33) a++;


while (a < Al and A[a] < 33) a++;
do a++ while (a < Al and A[a] < 33); 



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




*/





/*keep_backtracking: 
		if (not c) goto error;
		c -= 3;
		b = C[c];
		int i = b;
		do i--;
		while (i and B[i] != 10);
		while (B[b] != 32) {
		if (B[b] != *e) goto _0;
		e++; b++;
		goto keep_backtracking;		


*/


// if you back-tracked on a node which is zero, then undo the define. 
			// delete the last signature.
			// a define is always a FIRST node, so stop backtracking now.
		// go back on the index, until you find a space. 
			// (you are gaurenteed to find it!!)
		// then, go back on the index, further. 
		// if you hit a space, then you must 
		// continue backtracking, (goto keep_backtracking;)
		// however, if you hit a newline, that means that this is a "FIRST" node, 
		//    (ie, you bt'd over a first node)

		// that means that you need to stop backtracking, 
		// and set up the index variable accordingly, 	
		//   and then finish by going to begin.  now done.







	// int i = b;
	// do i--; while (B[i] != 32);
	// do { 
	// 	i--; 
	// 	if (B[i] == 32) goto _6; 
	// } while (B[i] != 10); 





// if you find the newline, 
	// then check to see if you still have other signatures to try in the context,

// move backwards to the most recent space.

// if you hit another space, 

 // then dont try more sigs, just backtrack.// keep going until you find a newline.

// if so, then skip over the back tracking code.


// printf("\n\n------------------- BEGIN ------------------------\n\n");
// 	printf("debug: a=%d b=%d c=%d \n", a, b, c);
// 	print_output(C, c + 3, B, Bl);
// 	print_index(B, b, Bl);
// 	printf("continue? "); getchar();






// printf("\n\n------------ PARENT ------------\n\n");
// 	printf("debug: a=%d b=%d c=%d \n", a, b, c);
// 	print_output(C, c + 3, B, Bl);
// 	print_index(B, b, Bl);
// 	printf("continue? "); getchar();





// int CL = 4096, BL = 4096;
// 	const char* A = "add add 57, 57, add 57, 5" , * B = "\ntop add top ,\ntop add top , top \ntop 5\ntop 57\n";




// int C[CL]; memset(C, 0x0F, sizeof C);
// 	int Al = (int)strlen(A), Bl = (int)strlen(B);
	

// 	while (a < Al and A[a] < 33) a++;
// 	C[c + 1] = -3; C[c + 2] = a;

