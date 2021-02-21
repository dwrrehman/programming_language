#include <stdio.h>    // compiler wrriten in c,  for a programming language 
#include <iso646.h>        // made by daniel warren riaz rehman.
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>


static void print_output(int* output, int top) {
	puts("");
	for (int i = 0; i < top; i += 4) {
		printf("%c %10d :   %10di %10dp %10db %10dd \n", 
			i < top - 4 ? ' ' : '>', i, 
			output[i], output[i + 1], output[i + 2], output[i + 3]);
	}
	puts("\n");
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
	if (argc != 2) return printf("usage: ./compiler <input>\n");
	const char* filename = argv[1], * reason = NULL;
	const int output_limit = 4096; 
	int begin = 0, top = 0, index = -1, done = 0, length = 0;
	char* input = open_file(filename, &length);
	int* output = malloc(output_limit * sizeof(int));
	memset(output, 0x0F, output_limit * sizeof(int)); // debug
	while (input[begin] != ';') begin++;
	do begin++; while (begin < length and input[begin] < 33);
	output[top] = output_limit;
	output[top + 2] = 0;
	output[top + 5] = 0;
	output[top + 6] = begin;
	top += 4;

_0:	if (index == output_limit) goto _6;

	// int temp = done;
	// if (input[temp] != ':' and input[temp] != ';') goto _11;

	// do temp--; while (input[temp] != ':');
	// do {	
	// 	temp--;
	// 	if (input[temp] == ':') goto _6;
	// } while (input[temp] != ';');

	if (index < top) goto _8;
_6: 	if (not top) goto resolution_failure;
	top -= 4; 
	index = output[top];
	goto _0;

_8:; 	int p = output[top + 1];
	if (not p) goto _10;
	char c = input[output[p + 3]]; // replace with type checking- ie, checking for empty type string.
	if (c != '.') goto _10;
	index = output_limit; 
	done = 0;
	while (begin < length and input[begin] != ';') begin++;
	do begin++; while (begin < length and input[begin] < 33);
	goto _2;

_10:	do index++; while(index < top + 4 and output[index] != output_limit);
	if (index >= top + 4) goto _0;
	begin = output[top + 2];
	done = output[index + 2];

_1:	if (input[done] == ';') goto _2;
	if (input[done] != '.' and input[done] != ':') goto _7;
	if (top + 7 >= output_limit) goto output_limit_exceeded;
	output[top] = index;
	output[top + 3] = done;
	top += 4;
	output[top + 1] = top - 4;
	output[top + 2] = begin;
	index = -1;
	goto _0;
_7:	if (begin >= length) goto _0;
	if (input[done] != input[begin]) goto _0;
	do begin++; while (begin < length and input[begin] < 33);
	do done++; while (done < length and input[done] < 33);
	goto _1;

_2:	output[top] = index;
	output[top + 3] = done;
	int parent = output[top + 1];
	if (not parent) goto _3;
	if (top + 7 >= output_limit) goto output_limit_exceeded;
	top += 4;
	output[top + 1] = output[parent + 1];
	output[top + 2] = begin;
	index = output[parent];
	done = output[parent + 3];
	do done++; while (done < length and input[done] < 33);
	goto _1;

_3:	if (begin != length) goto _0;
	top += 4;



	puts("success: compile successful."); 
	printf("success: length=%d begin=%d top=%d index=%d done=%d\n",
			length, begin, top, index, done);
	print_output(output, top);

	for (int i = 4; i < top; i += 4) {

		// printf("\t\t\t\t\t\t\t\t\t[ note: %10d : %10di %10dp %10db %10dd ]\n", i,  output[i], output[i + 1], output[i + 2], output[i + 3]);

		int t = output[i + 3];
		if (output[i] == output_limit) {
				for (int _ = 0; _ < (output[i + 1])/4; _++) printf(".   ");
				printf("found %d : [--> %d] UDS!!    ", i, output[i + 1]);
				printf("defining: ");
				for (int ii = output[i + 2]; input[ii] != ';'; ii++) {
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
						putchar(input[ii]);
					} 
					puts("\n");
				} else {
					// printf(" (intermediary): %10d : %10di %10dp \n", i,  output[i], output[i + 1]);
				}
			} else {
				// printf(" err: %10d : %10di %10dp %10db %10dd \n", i,  output[i], output[i + 1], output[i + 2], output[i + 3]);
			}
		}
		
	}
	goto done;

output_limit_exceeded: 
	printf("error: output limit exceeded\n"); 
	goto done;

resolution_failure: 
	printf("error: resolution failure\n"); 

done: 	munmap(input, (size_t) length);
	free(output);
}





















	// int temp = done;
	// if (input[temp] != ':' and input[temp] != ';') goto _11;

	// do temp--; while (input[temp] != ':');
	// do {	
	// 	temp--;
	// 	if (input[temp] == ':') goto _6;
	// } while (input[temp] != ';');








// printf("debug: a=%d b=%d c=%d \n", a, b, c);
// 	print_output(C, c, B, Bl);
// 	pretty_print_output(C, c, B, A);




//  --------------- debug functions ------------------

// static void print_index(const char* context, int index, int count) {
// 	if (index == 4096) { printf("    {UD SIG}\n\n"); return; }
// 	if (index > count or index < 0) { printf("    {error index}\n\n"); return; }
// 	printf("    ");
// 	for (int i = 0; i < count; i++) {
// 		char c = context[i];
// 		if (i == index) { if (c == 10) printf("\033[1;31m[.]\033[m"); else printf("\033[1;31m[%c]\033[m", c); }
// 		if (i != index) { if (c == 10) printf("."); else printf("%c", c); }
// 	} 
// 	if (index == count) printf("\033[1;31m[T]\033[m"); else printf("T"); 
// 	puts("\n");
// }

// static void print_output(int* output, int top, const char* context, int count) {
// 	puts("");
// 	for (int i = 0; i < top; i += 3) {
// 		printf("%10d :   %10di %10dp %10db : ", i, output[i], output[i + 1], output[i + 2]);
// 		print_index(context, output[i], count);
// 	}
// 	puts("\n");
// }

// static void pretty_print_output(int* output, int top, 
// 				const char* context, const char*input) {

// 	printf("printing parse tree in POST-DFS...\n");
// 	for (int i = 0; i < top; i += 3) {
// 		int r = output[i + 0], length = 0;
// 		if (r == 4096) {
// 			for (int _ = 0; _ < (output[i + 1] + 3)/3; _++) printf(".   ");
// 			printf("UDS:  \"");
			
// 			int k = output[i + 2];
// 			while (input[k] != ';') {
// 				if ((unsigned char)input[k] >= 33) putchar(input[k]);
// 				k++;
// 			}
// 			printf(";\" \t\t\t\t: %di %dp %db \n\n", 
// 				output[i + 0], output[i + 1], output[i + 2]);
// 			continue;
// 		}

// 		if (context[r] != 10) continue;
// 		do { r--; length++; } while (r and context[r] != 10);
// 		r++; length--;

// 		for (int _ = 0; _ < (output[i + 1] + 3)/3; _++) printf(".   ");
// 		printf("(%.*s) \t\t\t\t", length, context + r);
// 		printf(" : %di %dp %db #%d\n\n", 
// 			output[i + 0], output[i + 1], output[i + 2], i);
// 	}
// 	printf("parse tree complete.\n");
// }





























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

























// 	expected = C[c + 1] == -3 ? "top " : B + C[C[c + 1]] + 1;
// 	if (*e != 32) goto _10; 
// 	b = BL;
// 	while (a < Al and A[a] != 59) {
// 		if (Bl + 1 >= BL) goto BLx;
// 		if (A[a] != 92) B[Bl++] = A[a] == 58 ? 32 : A[a];
// 		else { 
// 			do a++; while (a < Al and (uc)A[a] < 33); 
// 			B[Bl++] = A[a]; 
// 		}
// 		do a++; while (a < Al and (uc)A[a] < 33);
// 	} 
// 	if (a >= Al) goto _0;
// 	B[Bl++] = 10;
// 	if (Bl > Blb) Blb = Bl;
// 	do a++; while (a < Al and (uc)A[a] < 33);
// 	if (a > ab) { 
// 		ab = a; 
// 		bb = b; 
// 	} 
// 	goto _2;
// _10: 	while (B[b] != 32 or *e != 32) { 
// 		if (B[b] != *e) goto _0; 
// 		e++; 
// 		b++; 
// 	} 
// 	b++;





// char output_bytes[4096] = {0};
// 	int stack[4096] = {0};

// 	for (int i = 0; i < c; i += 3) {
// 		printf("%d : %10d %10d %10d \n", 
// 			i, C[i], C[i + 1], C[i + 2]);
		
// 		if (C[i] == BL or B[C[i]] == 10) 
// 			printf("\t\tfound ---> %10d: %10d\n", i, C[i + 1]);

// 		if (C[i] == BL or B[C[i]] != 10)  continue;
	
// 		int s = C[i];
// 		do s--; while (s and B[s] != 10); 
// 		++s;
		
// 		const char* add = "top [x86]add\n";

// 		int k = s;
// 		while (B[s] != 10 or *add != 10) {
// 			if (B[s] != *add) goto next;
// 			add++; 
// 			s++; 
// 		}

// 		s = k;
// 		fprintf(stderr, "\n\n\033[1m found:\033[m  ");
// 		for (int l = s; B[l] != 10 and l < Bl; l++) 
// 			fprintf(stderr, "%c", B[l]);
// 		puts("\n");

// 		next: continue;
		
// 	}




// Bl = Blb;
// 	int at = 0, line = 1, column = 1;
// 	while (at < ab and at < Al) {
// 		if (A[at++] == '\n') { 
// 			line++; column = 1; 
// 		} else column++;
// 	}
// 	fprintf(stderr, "\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n", 
// 		filename, line, column, reason);
// 	print_source(A, Al, line, column);
// 	print_candidate(B, Bl, bb);






/*
j.and:finished;
	j
		hello . also : also : done ; 

	and 
		hello  

			     nop ;    

			also nop 
			also nop
		
		done
		
	finished

*/


