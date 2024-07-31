// trying to make a language that has no explicit argument passing,
// similar to the ua isa. not neccessarily intended to be the mosttt 
// useable language lol. currently only interpreted. 
// 202407265.205102: by dwrr

/*

language isa:  1202407265.214926
------------------------------------------

	meaning		spelling
.........................................
	i++		is
	i=0		iz
	i+=*i		im
	i+=*n		in

	*n++		ns
	*n=0		nz
	*n~		nn
	*n+=i		ni
	*n+=*i		nm

	*i++		ms
	*i=0		mz
	*i+=i		mi
	*i+=*n		mn
	
	*n < *i		lm
	*n == *i	em
	*n < i		li
	*n == i		ei

	setflag		sf
	clearflag	cf

	jump0		b0
	jump1		b1
	jump2		b2
	jump3		b3
	jump4		b4
	
	pc0		a0
	pc1		a1
	pc2		a2
	pc3		a3
	pc4		a4





copyb insert ./build
copya insert ./run


insert chmod 755 build
insert touch test.txt
insert cp test.txt build
insert ls -la
total 8
drwxr-xr-x   5 dwrr  staff   160 Jul 26 21:21 .
drwxr-xr-x@ 11 dwrr  staff   352 Jul 26 20:50 ..
-rwxr-xr-x   1 dwrr  staff     0 Jul 26 21:22 build
-rw-r--r--@  1 dwrr  staff  2177 Jul 26 21:23 c.c
-rw-r--r--   1 dwrr  staff     0 Jul 26 21:21 test.txt
total 16
drwxr-xr-x   6 dwrr  staff   192 Jul 26 21:28 .
drwxr-xr-x@ 11 dwrr  staff   352 Jul 26 20:50 ..
-rw-r--r--   1 dwrr  staff    65 Jul 26 21:28 1202407265.212814_19f404a5635cf9f2.txt
-rwxr-xr-x   1 dwrr  staff     0 Jul 26 21:22 build
-rw-r--r--@  1 dwrr  staff  2527 Jul 26 21:28 c.c
-rw-r--r--   1 dwrr  staff     0 Jul 26 21:21 test.txt

write clang -g -Weverything -O0 -fsanitize=address,undefined -o run c.c
do ,/bin/cp,-i,-v,1202407265.212814_19f404a5635cf9f2.txt,build
insert cat build


write #!/bin/zsh
clang -g -Weverything -O0 -fsanitize=address,undefined -o run c.c

insert ls
1202407265.212814_19f404a5635cf9f2.txt
1202407265.213155_22e4e6cf61ed31ee.txt
build
c.c
test.txt

insert cp -v 1202407265.213155_22e4e6cf61ed31ee.txt build
1202407265.213155_22e4e6cf61ed31ee.txt -> build

insert cat build
#!/bin/zsh
clang -g -Weverything -O0 -fsanitize=address,undefined -o run c.c

do ,/Users/dwrr/root/personal/executables/editor,./build









*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

typedef uint64_t nat;





static char* read_file(const char* name, nat* out_length) {
	int d = open(name, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }
	const int file = open(name, O_RDONLY, 0);
	if (file < 0) { read_error:; perror("open"); exit(1); }
	size_t length = (size_t) lseek(file, 0, SEEK_END);
	char* string = malloc(length);
	lseek(file, 0, SEEK_SET);
	read(file, string, length);
	close(file); 
	*out_length = length;
	return string;
}

int main(int argc, const char** argv) {

	const nat array_size = 0;
	nat array[array_size] = {0};
	nat pointer = 0;
	nat comparator = 0;
	nat flag = 0;
	nat pcreg[256] = {0};

	nat text_length = 0;
	const char* filename = argv[1];
	char* text = read_file(filename, &text_length);

	printf("read file: (length = %llu): \n<<<", text_length);
	fwrite(text, 1, text_length, stdout);
	puts(">>>");

	for (nat i = 0, s = 0, c = 0; i < text_length; i++) {
		if (not isspace(text[i])) {
			if (not c) s = i; c++; 
			if (i + 1 < text_length) continue;
		} else if (not c) continue;
		char* word = strndup(text + s, c);


		if (strcmp(word, "is")) {
			pointer++;

		} else if (strcmp(word, "iz")) {
			pointer = 0;

		} else if (strcmp(word, "im")) {
			pointer += *pointer;

		} else if (strcmp(word, "in")) {
			pointer += comparator;



		} else if (strcmp(word, "ns")) {
			comparator++;

		} else if (strcmp(word, "nz")) {
			comparator = 0;

		} else if (strcmp(word, "nn")) {
			comparator = ~comparator;

		} else if (strcmp(word, "ni")) {
			comparator += pointer;

		} else if (strcmp(word, "nm")) {
			comparator += *pointer;


		} else if (strcmp(word, "ms")) {
			(*pointer)++;

		} else if (strcmp(word, "mz")) {
			(*pointer) = 0;

		} else if (strcmp(word, "mi")) {
			(*pointer) += pointer;

		} else if (strcmp(word, "mn")) {
			(*pointer) += comparator;
		


	}

	printf("finished procressing text.\n");
	exit(0);
}




/*

		for (nat n = 0; n < isa_count; n++) 
			if (not strcmp(spelling[n], word)) goto ins;










exit



*/



