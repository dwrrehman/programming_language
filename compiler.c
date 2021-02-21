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

// .:::.:;

int main(const int argc, const char** argv) {
	if (argc != 2) return printf("usage: ./compiler <input>\n");
	const int output_limit = 4096; 
	const char* expected = NULL;
	int begin = 0, top = 0, index = 0, done = 0, length = 0, parent = 0;
	char* input = open_file(argv[1], &length);
	int* output = malloc(output_limit * sizeof(int));
	memset(output, 0x0F, output_limit * sizeof(int)); 

	while (input[begin] != ';') begin++; 
	begin++;

	output[top] = output_limit;
	output[top + 2] = 0;
	output[top + 5] = 0;
	output[top + 6] = begin;
	top += 4;
begin:  
	parent = output[top + 1];
	if (parent) goto child;
	expected = "top:";
	goto type;
child:
	expected = input + output[parent + 3] + 1;
	if (*expected == ':') goto new;
type:
	if (input[done] == ':' and *expected == ':') goto valid;
	if (input[done] != *expected) goto next;
	expected++;
	done++;
	goto type;
valid: 
	done++;
	begin = output[top + 2];
parent:
	if (input[done] == ';') goto done;
	if (input[done] != ':') goto match;
	if (top + 7 >= output_limit) goto output_limit_exceeded;
	output[top] = index;
	output[top + 3] = done;
	output[top + 5] = top;
	output[top + 6] = begin;
	top += 4;
	index = 0;
	goto begin;
match:	
	if (begin >= length) goto fail;
	if (input[done] != input[begin]) goto fail;
	begin++; 
	done++;
	goto parent;
new:
	index = output_limit; 
	done = 0;
	while (input[begin] != ';') begin++;
	begin++;
done:
	output[top] = index;
	output[top + 3] = done;
	parent = output[top + 1];
	if (parent) goto duplicate;
	if (begin == length) goto success; 
	goto fail;
duplicate:
	if (top + 7 >= output_limit) goto output_limit_exceeded;
	top += 4;
	output[top + 1] = output[parent + 1];
	output[top + 2] = begin;
	index = output[parent];
	done = output[parent + 3] + 1;
	while (input[done] != ':') done++; 
	done++;
	goto parent;
fail:
	prepare backtracking condition;
	if ( we dont need to backtrack ) goto next;

	if (not top) goto resolution_failure;
	top -= 4;
	index = output[top];
	done = output[top + 2];
	goto fail;

next:
	index++;
	if (index >= top) goto fail;
	if (output[index] != output_limit) goto next;
	done = output[index + 2];
	goto begin;
success: 
	puts("success: compile successful."); 



	printf("success: length=%d begin=%d top=%d index=%d done=%d\n",
			length, begin, top, index, done);
	print_output(output, top);

	for (int i = 4; i < top; i += 4) {
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
	goto clean_up;

output_limit_exceeded: 
	printf("error: output limit exceeded\n"); 
	goto clean_up;

resolution_failure: 
	printf("error: resolution failure\n"); 

clean_up: 	
	munmap(input, (size_t) length);
	free(output);
}


























// _0:	if (index == output_limit) goto _6;

// 	// int temp = done;
// 	// if (input[temp] != ':' and input[temp] != ';') goto _11;

// 	// do temp--; while (input[temp] != ':');
// 	// do {	
// 	// 	temp--;
// 	// 	if (input[temp] == ':') goto _6;
// 	// } while (input[temp] != ';');

// 	if (index < top) goto _8;

// _6: 	if (not top) goto resolution_failure;
// 	top -= 4; 
// 	index = output[top];
// 	goto _0;

// _8:; 	int p = output[top + 1];
// 	if (not p) goto _10;
// 	char c = input[output[p + 3]]; // replace with type checking- ie, checking for empty type string.
// 	if (c != '.') goto _10;
// 	index = output_limit; 
// 	done = 0;
// 	while (begin < length and input[begin] != ';') begin++;
// 	do begin++; while (begin < length and input[begin] < 33);
// 	goto _2;

// _10:	do index++; while(index < top + 4 and output[index] != output_limit);
// 	if (index >= top + 4) goto _0;
// 	begin = output[top + 2];
// 	done = output[index + 2];

// _1:	if (input[done] == ';') goto _2;
// 	if (input[done] != '.' and input[done] != ':') goto _7;
// 	if (top + 7 >= output_limit) goto output_limit_exceeded;
// 	output[top] = index;
// 	output[top + 3] = done;
// 	top += 4;
// 	output[top + 1] = top - 4;
// 	output[top + 2] = begin;
// 	index = -1;
// 	goto _0;

// _7:	if (begin >= length) goto _0;
// 	if (input[done] != input[begin]) goto _0;
// 	do begin++; while (begin < length and input[begin] < 33);
// 	do done++; while (done < length and input[done] < 33);
// 	goto _1;

// _2:	output[top] = index;
// 	output[top + 3] = done;
// 	int parent = output[top + 1];
// 	if (not parent) goto _3;
// 	if (top + 7 >= output_limit) goto output_limit_exceeded;
// 	top += 4;
// 	output[top + 1] = output[parent + 1];
// 	output[top + 2] = begin;
// 	index = output[parent];
// 	done = output[parent + 3];
// 	do done++; while (done < length and input[done] < 33);
// 	goto _1;

// _3:	if (begin != length) goto _0;
// 	top += 4;







