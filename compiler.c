#include <stdio.h>    // the n programming language compiler, written in c.
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

static inline void print_index(const char* context, int index, int count) {
	if (index == 4096) { printf("{UD SIG}\n"); return; }
	if (index > count or index < 0) { printf("{error index}\n"); return; }
	for (int i = 0; i < count; i++) {
		char c = context[i];
		if (i == index) { if (c == 10) printf("[.]"); else printf("[%c]", c); }
		if (i != index) { if (c == 10) printf("."); else printf("%c", c); }
	} if (index == count) printf("[T]"); else printf("T"); puts("");
}

static inline void print_vector(int* output, int top, const char* context, int count) {
	for (int i = 0; i < top; i += 4) {
		printf("%10d :   %10di %10dp %10db %10dc  : ", i, output[i], output[i + 1], output[i + 2], output[i + 3]);
		print_index(context, output[i], count);
	}
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

int main(const int argc, const char** argv) {
	if (argc != 3) {
		printf("usage: ./compiler <input> <context>\n");
		return 1;
	}
	const int output_limit = 4096, context_limit = 4096;
	const char * filename = argv[1], * reason = NULL;

	int count = 0, length = 0;
	char* input = open_file(filename, &length);
	char* _base = open_file(argv[2], &count);
	char* context = malloc(context_limit);

	memset(context, 0x0F, context_limit);
	memcpy(context, _base, (size_t) count);

	munmap(_base, (size_t) count);

	int* output = malloc(output_limit * sizeof(int));
	memset(output, 0x0F, output_limit);

	int begin = 0, index = 0, top = 0, current = 0;
	int best = 0, candidate = 0, biggest = 0;

	printf("DEBUG ::::%.*s====%.*s::::\n", length, input, count, context);

	while (begin < length and input[begin] < 33) begin++;

	best = begin;
	biggest = count;

	output[top + 1] = 999; 
	output[top + 2] = begin;
	output[top + 3] = count;

begin:	begin = output[top + 2];
	count = output[top + 3];

	if (index >= count) goto fail;
	while (context[index] != 10) {
		if (index >= count) goto fail;
		index++;
	}
	index++;
	
	if (index >= count) {
	fail: 	if (not top) { reason = "unresolved expression"; goto error; }
		top -= 4;
		index = output[top]; 
		current = top;
		goto begin;
	}

	const char* expected = output[top + 1] == 999 ? "init " 
				: context + output[output[top + 1]] + 1;
	const char* undefined = "undefined ", * copy_expected = expected;

	while (*undefined != ' ') {
		if (*undefined != *copy_expected) goto non;
		copy_expected++; undefined++;
	}

	while (begin < length and input[begin] != ';') {
		if (input[begin] != '\\') {
			if (input[begin] == ':') context[count++] = ' ';
			else context[count++] = input[begin];
			do begin++; while (begin < length and input[begin] < 33);
		} else {
			do begin++; while (begin < length and input[begin] < 33);
			context[count++] = input[begin];
			do begin++; while (begin < length and input[begin] < 33);
		}
	}
	context[count++] = '\n';
	if (count > biggest) biggest = count;
	index = context_limit;
	do begin++; while (begin < length and input[begin] < 33);
	if (begin > best) { best = begin; candidate = index; }
	goto done;
non: 	
	while (context[index] != ' ') {
		if (index == count) goto fail;
		if (context[index] != *expected) goto begin;
		expected++; index++;
	}
	index++;

parent:	if (context[index] == 10) goto done;
	if (context[index] == 32) {
		if (top + 7 >= output_limit) { reason = "program limit exceeded"; goto error; }
		top += 4;
		output[current] = index;
		output[top + 1] = current;
		output[top + 2] = begin;
		output[top + 3] = count;
		current = top;
		index = 0;
		goto begin;
	}
	if (begin >= length or context[index] != input[begin]) goto begin;
	do begin++; while (begin < length and input[begin] < 33); 
	index++;
	if (begin > best) { best = begin; candidate = index; }
	
	goto parent;
done:	if (current) {
		output[current] = index;
		current = output[current + 1];
		index = output[current] + 1;
		while (context[index] != 32) {
			if (index == count) goto fail;
			index++; 
		}
		index++;
		goto parent;
	}
	if (begin != length) goto begin;
	puts("\n\t---> compile successful.\n");
	output[current] = index;
	goto final;
error:
	count = biggest;
	int at = 0, line = 1, column = 1;
	while (at < best and at < length) {
		if (input[at++] == '\n') { line++; column = 1; } else column++;
	}
	fprintf(stderr, "\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n", 
			filename, line, column, reason);

	int b = line > 2 ? line - 2 : 0, e = line + 2;
	for (int i = 0, l = 1, c = 1; i < length + 1; i++) {
		if (c == 1 and l >= b and l <= e) 
			fprintf(stderr, "\n\033[90m%5d\033[0m\033[32m │ \033[0m", l);
		if ((i == length or input[i] != '\n') and l >= b and l <= e) {
			if (l == line and c == column) fprintf(stderr, "\033[1;31m");
			if (i < length) fprintf(stderr, "%c", input[i]);
			else if (l == line and c == column) fprintf(stderr, "<EOF>");
			if (l == line and c == column) fprintf(stderr, "\033[m");
		}
		if (i < length and input[i] == 10) { l++; c = 1; } else c++;
	}
	if (not count) {
		printf("\n\nskipping error candidate...\n");
		goto skip_candidate;
	}
	int start = not candidate ? 1 : candidate;
	do start--; while (start and context[start] != 10); 
	++start;
	fprintf(stderr, "\n\n\033[1m candidate:\033[m  \033[1;94m");
	while (context[start] != ' ' and start < count) {
		fprintf(stderr, "%c", context[start]);
		start++;	
	}
	fprintf(stderr, "%c\033[m", context[start]);
	start++;
	for (int k = start; context[k] != 10 and k < count; k++) {
		if (context[k] == 32) {
			int p = k++;
			fprintf(stderr, p == candidate ? "\033[1;31m " : "\033[1;96m "); 
			while (context[k] != 32) {
				fprintf(stderr, "%c", context[k]);
				k++;
			}
			fprintf(stderr, p == candidate ? " \033[m" : " \033[m"); 
		} else {
			if (k == candidate) fprintf(stderr, "\033[1;31m");
			fprintf(stderr, "%c", context[k]);
			if (k == candidate) fprintf(stderr, "\033[m");
		}
	}
skip_candidate: 
	puts("\n");
final:
	printf("DEBUG ::::%.*s====%.*s::::\n", length, input, count, context);
	printf("debug: index=%d current=%d top=%d begin=%d count=%d\n", index, current, top, begin, count);
	print_vector(output, top + 16, context, count);
}
