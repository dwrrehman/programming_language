#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////          we need to implement contexts next!!!           ////////////////////////// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////




typedef uint64_t nat;

static char* text = NULL;
static nat text_length = 0;

static nat registers[4096] = {0}; 
static nat arguments[4096] = {0};

static nat names[4096] = {0};
static nat lengths[4096] = {0};
static nat values[4096] = {0};

static nat name_count = 0, length = 0, 
	start = 0, argument_count = 0, 
	r = 0, p = 1;

static bool equals(const char* literal, nat initial) {
	nat i = initial, j = 0;
	for (; text[i] != '"' and literal[j]; i++) {
		if ((unsigned char) text[i] < 33) continue;
		if (text[i] != literal[j]) return false;
		j++;
	}
	return text[i] == '"' and not literal[j];
}

static void process(nat n, nat name) {

	if (equals("0", n)) { p <<= 1; } 
	else if (equals("1", n)) { r += p; p <<= 1; } 
	else if (equals("#", n)) { arguments[argument_count++] = r; r = 0; p = 1; } 


	else if (equals("snpush", n)) { arguments[argument_count++] = r; } 
	else if (equals("snprint", n)) { printf("r = %llu, p = %llu\n", r, p); } 
	else if (equals("argprint", n)) { printf("debug: %llu\n", arguments[argument_count - 1]); } 
	else if (equals("debugarguments", n)) { 
		printf("debug: .args={ ");
		for (nat a = 0; a < argument_count; a++) printf("%llu ", arguments[a]);
		puts("}");
	} else arguments[argument_count++] = values[name];
}

int main(int argc, const char** argv) {

	if (argc < 2) exit(puts("usage: ./assembler <source.s>"));

	int file = open(argv[1], O_RDONLY);
	if (file < 0) { perror("open"); exit(1); } 
	struct stat st;
	fstat (file, &st);
	text_length = (nat) st.st_size;
	text = mmap(0, text_length, PROT_READ, MAP_PRIVATE, file, 0);
	close(file);

	for (nat index = 0; index < text_length; index++) {
		if ((unsigned char) text[index] < 33) goto next_char;
		if (text[index] == '"') {
			if (not start) { 
				start = index + 1; 
				length = 0; 
			} else {
				nat spot = 0;
				for (; spot < name_count; spot++) if (length >= lengths[spot]) break;
				memmove(lengths + spot + 1, lengths + spot, sizeof(nat) * (name_count - spot));
				memmove(names + spot + 1, names + spot, sizeof(nat) * (name_count - spot));
				memmove(values + spot + 1, values + spot, sizeof(nat) * (name_count - spot));
				lengths[spot] = length;
				names[spot] = start;
				values[spot] = argument_count ? arguments[argument_count - 1] : 0;
				name_count++; start = 0;
			}
			goto next_char;
		}
		if (start) { length++; goto next_char; } 
		nat imax = 0;
		for (nat name = 0; name < name_count; name++) {
			nat c = names[name], n = c, i = 0;
			for (; text[c] != '"'; ) {
				if (i > imax) imax = i;
				if (index + i >= text_length) goto next_name;
				if ((unsigned char) text[index + i] < 33) { i++; continue; }
				if ((unsigned char) text[c] < 33) { c++; continue; }
				if (text[index + i] != text[c]) goto next_name;
				i++; c++;
			}
			printf("[found the name: <<<");
			for (nat cc = names[name]; text[cc] != '"'; cc++) putchar(text[cc]);
			printf(">>>, ");
			printf("moved along %llu chars...\n", i);
			process(n, name);
			index += i - 1;
			goto next_char;
			next_name:;
		}
		printf("asm: \033[31;1merror:\033[0m %s:%llu:%llu: unresolved symbol\n", 
			argv[1], index, index + imax
		);
		exit(1);
		next_char:;
	}

	puts("current names: {");
	for (nat i = 0; i < name_count; i++) {
		printf("\t#%llu: @%llu:len=%llu:val=%llu ", i, names[i], lengths[i], values[i]);
		for (nat c = names[i]; text[c] != '"'; c++) putchar(text[c]);
		puts("");
	}
	puts("}");

	munmap(text, length);
}

    






















/*

//printf("[success]\n");
//printf("\n\tchecking name[%llu]=%llu::: @(index=%llu)\"%c\" vs @(name=%llu)\"%c\"...\n", j, names[j], index, text[index + i], c, text[c]);



	//puts("parsing file now...");

	//puts("------------------------");

//if ((unsigned char) text[index] < 33) printf("\033[7;33m");
		//printf("%c", text[index]);
		//if ((unsigned char) text[index] < 33) printf("\033[0m");

 //  puts("FOUND"); 
//puts("LOOKED");

//printf("[start=%llu,length=%llu,spot=%llu]", start, length, spot);



//if (start) printf("\033[32m"); else printf("\033[0m");
//puts("toggling name state!");
			//puts(defining ? "[defining]" : "[executing]");


//printf("@%llu:%llu ", names[j], lengths[j]);
//printf("\n\tchecking @(index=%llu)\"%c\" vs @(name=%llu)\"%c\"...\n", 
				//	index, text[index + i], c, text[c]);				

//printf("[found the name: <<<");
			//for (nat cc = names[j]; text[cc] != '"'; cc++) putchar(text[cc]);
			
			//printf(">>>, ");

//printf("moved along %llu chars...\n", i);

*/

