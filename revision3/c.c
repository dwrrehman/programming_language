#include <stdio.h>
#include <stdbool.h>
#include <iso646.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef size_t nat;


static void interpret(char* string, nat length) {

	for (nat i = 0; i < length; i++) {
		if (isspace(string[i])) {
			printf("SPACE");
		} else {
			printf("%c", string[i]);
		}
		puts("");
	}
}


int main() {
	puts("a repl for my programminglanguage.");

	char line[4096] = {0};

loop: 	printf(" : ");
	fgets(line, sizeof line, stdin);
	nat length = strlen(line);
	line[--length] = 0;
	
	if (not strcmp(line, "")) {}
	else if (not strcmp(line, "clear")) printf("\033[H\033[J");
	else if (not strcmp(line, "quit")) goto done;
	else interpret(line, length);
	goto loop; done:;
}






























/*


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

