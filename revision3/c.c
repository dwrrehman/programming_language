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

enum word_type {
	null_word,
	label_definition,
	variable_definition,
};

enum instruction_type {
	nop, 
	add,
	bne,
};





struct word {
	char* name;
	nat length;
	nat type;
};

struct instruction {
	nat operation;
	nat arg0;
	nat arg1;
	nat arg2;
};

static nat arguments[16] = {0};
static nat dictionary_count = 0;
static struct word* dictionary = NULL;
static nat instruction_count = 0;
static struct instruction* instructions = NULL;

static void ins(nat op, nat arg0, nat arg1, nat arg2) {
	instructions = realloc(instructions, sizeof(struct instruction) * (instruction_count + 1));
	instructions[instruction_count++] = (struct instruction){op, arg0, arg1, arg2};
}

static void interpret(char* string, nat length) {
	
	nat count = 0, start = 0;
	for (nat i = 0; i < length; i++) {
		if (not isspace(string[i])) { 
			if (not count) start = i; 
			count++; continue; 
		} else if (not count) continue;
		process_word:; 
		char* word = string + start;

		     if (not strncmp(word, "nop", count)) ins(nop,0,0,0);
		else if (not strncmp(word, "add", count)) ins(add,0,0,0);
		else if (not strncmp(word, "bne", count)) ins(bne,0,0,0);

		else {
			printf("found label: "); 
			for (nat _ = 0; _ < count; _++) putchar(string[_ + start]);
			puts("");

			for (nat entry = 0; entry < dictionary_count; entry++) {
				if (strncmp(dictionary[entry].name, word, count)) continue;
				printf("found existing label!\n");
				goto found;
			}

			printf("making a new label (Adding to dict!)....\n");
			dictionary = realloc(dictionary, sizeof(struct word) * (dictionary_count + 1));
			dictionary[dictionary_count++] = (struct word) {.name = strndup(word, count), .length = count, .type = label_definition};
			found:;
		}
		count = 0;
	}
	if (count) goto process_word;
}

int main() {
	puts("a repl for my programming language.");
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

