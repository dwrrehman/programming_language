// 202405072.013925: by dwrr
// a program to look at the word lengths and word counts of english text.
// useful information for making my programming language, which disguises itself
// as human readable comments describing the code its interpreted as.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iso646.h>
#include <stdbool.h>

int main(int argc, const char** argv) {
	
	if (argc == 1) return puts("usage error: give a file to parse.");

	int file = open(argv[1], O_RDONLY);
	if (file < 0) { perror("open"); exit(1); }

	const size_t length = (size_t) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(length + 1, 1);
	read(file, text, length);
	close(file);

	printf("info: opened a file %u with text %p and had length %lu...\n", 
		file, (void*) text, length
	);

	size_t tallys[4096] = {0};
	size_t tallys_counts[4096] = {0};

	for (size_t c = 0, i = 0; i + 1 < length; i++) {
		printf("%lu:%lu: at char: %u(%c)\n", c, i, text[i], text[i]);
		if (not isspace(text[i])) { if (isspace(text[i + 1])) c++; }
		else if (isspace(text[i + 1]) and c) {
			printf("---> WORDCOUNT %lu \n", c);  
			tallys_counts[c]++;
			c = 0;
		}
        }


	puts("\n here are the tallys for the word lengths: ");

	puts("");
	for (size_t i = 0; i < 12; i++) {
		printf("%lu: %lu :: ", i, tallys[i]);
		for (size_t _ = 0; _ < tallys[i]; _++) {
			putchar('#');
		}
		puts("");
	}

	puts("\n");



	puts("\n here are the tallys for the WORD COUNTS: ");

	puts("");
	for (size_t i = 0; i < 50; i++) {
		printf("%lu: %lu :: ", i, tallys_counts[i]);
		for (size_t _ = 0; _ < tallys_counts[i]; _++) {
			putchar('#');
		}
		puts("");
	}

	puts("\n");


}



/*

do ./run
other.txt

do /usr/bin/clang
-g
-O0
-Weverything
-ferror-limit=2
b.c
-o
run
-fsanitize=address,undefined
-Wno-declaration-after-statement










	for (size_t i = 0; i < length; i++) {
		// printf("heres char %lu: %c (%u)\n", i, text[i], text[i]);

		const char c = text[i];

	}




do /usr/bin/man
2
read


do /bin/pwd

insert ls
insert pwd

insert ./run


insert ls -la
total 120
drwxr-xr-x@  6 dwrr  staff    192 May  7 01:47 .
drwxr-xr-x@ 11 dwrr  staff    352 May  7 01:47 ..
-rw-r--r--   1 dwrr  staff    871 May  7 01:47 b.c
-rw-r--r--   1 dwrr  staff    823 May  7 01:32 file.txt
-rwxr-xr-x   1 dwrr  staff  51792 May  7 01:47 run
drwxr-xr-x@  3 dwrr  staff     96 May  7 01:47 run.dSYM




*/

