#include <stdio.h>
#include <stdbool.h>
#include <iso646.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
typedef size_t nat;

static void print_word(char* string, nat start, nat count) {
	for (nat i = 0; i < count; i++) putchar(string[i + start]);
}

static void interpret(char* string, nat length) {
	nat count = 0, start = 0;
	for (nat i = 0; i < length; i++) {
		if (not isspace(string[i])) { 
			if (not count) start = i; 
			count++; continue; 
		} else if (not count) continue;
		here:; 
		char* word = string + start;

		if (not strncmp(word, "hello", count)) {
			printf("executing hello command...\n");

		} else if (not strncmp(word, "there", count)) {
			printf("executing there command...\n");

		} else {
			printf("found user-defined variable: "); 
			print_word(string, start, count); 
			puts("");
		}
		count = 0;
	}

	if (count) goto here;
}

int main() {
	puts("a repl for my programminglanguage.");
	char line[4096] = {0};
loop: 	printf(" : ");
	fgets(line, sizeof line, stdin);
	nat length = strlen(line);
	line[--length] = 0;
	if (not strcmp(line, "q")) goto done;
	else if (not strcmp(line, "o")) printf("\033[H\033[J");
	else interpret(line, length);
	goto loop; done:;
}






























/*

printf("%c [%lu] ", string[i], count);
puts("");





printf("unknown word found: [@start=%lu, count=%lu]\n", start, count);
				printf("ie, ---> ");
				print_word(string, start, count);
				puts("");


printf("---> ");
				print_word(string, start, count);
				puts("");










static char* read_file(const char* filename, size_t* out_length) {
	const int file = open(filename, O_RDONLY);
	if (file < 0) {
		perror("open"); 
		return NULL;
	}
	struct stat file_data = {0};
	if (stat(filename, &file_data) < 0) { 
		perror("stat"); 
		return NULL;
	}
	const size_t length = (size_t) file_data.st_size;
	char* buffer = not length ? NULL : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (buffer == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}
	close(file);
	*out_length = length;
	return buffer;
}





	else if (not strcmp(string, "file")) {

		char filename[4096] = {0};
		printf("filename: ");
		fgets(filename, sizeof filename, stdin);
		filename[strlen(filename) - 1] = 0;
		size_t length = 0;
		char* contents = read_file(filename, &length);
		if (contents) interpret(contents, length);
	}



*/

