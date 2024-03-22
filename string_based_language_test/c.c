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

typedef uint64_t nat;

static bool equals(const char* literal, char* text, nat start) {
	nat i = start, j = 0;
	for (; text[i] != '"' and literal[j]; i++) {
		//printf("%c", text[i]);
		if ((unsigned char) text[i] < 33) continue;
		if (text[i] != literal[j]) return false;
		j++;
	}
	return text[i] == '"' and not literal[j];
}

int main(int argc, const char** argv) {
	if (argc == 1) exit(puts("usage error, please give a file."));
	int file = open(argv[1], O_RDONLY);
	if (file < 0) { perror("open"); exit(1); } 
	struct stat st;
	fstat (file, &st);
	const nat text_length = (nat) st.st_size;
	char* text = mmap(0, text_length, PROT_READ, MAP_PRIVATE, file, 0);
	close(file);


	nat sn = 0;
	nat arguments[4096] = {0};
	nat names[4096] = {0};
	nat lengths[4096] = {0};
	nat values[4096] = {0};
	nat name_count = 0, length = 0, start = 0, argument_count = 0;

	puts("parsing file now...");

	//puts("------------------------");
	for (nat index = 0; index < text_length; index++) {

		if ((unsigned char) text[index] < 33) printf("\033[7;33m");
		//printf("%c", text[index]);
		if ((unsigned char) text[index] < 33) printf("\033[0m");
		if ((unsigned char) text[index] < 33) continue;

		if (start) { length++; } 

		if (text[index] == '"') {
			if (not start) { start = index + 1; length = 0; } 
			else {
				nat spot = name_count;
				for (nat i = 0; i < name_count; i++) {
					
					if (length >= lengths[i]) { spot = i; break; } //  puts("FOUND"); 
					//puts("LOOKED");
				}

				//printf("[start=%llu,length=%llu,spot=%llu]", start, length, spot);
				memmove(lengths + spot + 1, lengths + spot, sizeof(nat) * (name_count - spot));
				memmove(names + spot + 1, names + spot, sizeof(nat) * (name_count - spot));
				memmove(values + spot + 1, values + spot, sizeof(nat) * (name_count - spot));

				lengths[spot] = length - 1;
				names[spot] = start;
				values[spot] = argument_count ? arguments[argument_count - 1] : 0;

				name_count++; start = 0;
			}
			//if (start) printf("\033[32m"); else printf("\033[0m");
			continue;
		}

		if (start) continue;

		for (nat j = 0; j < name_count; j++) {
			//printf("@%llu:%llu ", names[j], lengths[j]);
			nat c = names[j], i = 0;
			for (; text[c] != '"'; ) {
				//printf("\n\tchecking @(index=%llu)\"%c\" vs @(name=%llu)\"%c\"...\n", 
				//	index, text[index + i], c, text[c]);				

				if ((unsigned char) text[index + i] < 33) { i++; continue; }
				if ((unsigned char) text[c] < 33) { c++; continue; }
				if (text[index + i] != text[c]) goto next_name;
				i++; c++;
			}
			printf("[found the name: <<<");
			for (nat cc = names[j]; text[cc] != '"'; cc++) putchar(text[cc]);
			
			printf(">>>, ");
			index += i;
			printf("moved along %llu chars...\n", i);
			
			     if (equals("snincr",  text, names[j])) { sn++; }
			else if (equals("snzero",  text, names[j])) { sn = 0; } 
			else if (equals("snshl",   text, names[j])) { sn <<= 1; } 
			else if (equals("0",   text, names[j])) { sn <<= 1; } 
			else if (equals("1",   text, names[j])) { sn++; sn <<= 1; } 
			else if (equals("#",   text, names[j])) { arguments[argument_count++] = sn; } 
			else if (equals("snpush",  text, names[j])) { arguments[argument_count++] = sn; } 
			else if (equals("snprint", text, names[j])) { printf("sn = %llu\n", sn); } 
			else if (equals("argprint", text, names[j])) { printf("debug: %llu\n", arguments[argument_count - 1]); } 
			else if (equals("debugarguments", text, names[j])) { 

				printf("debug: .args={ ");
				for (nat a = 0; a < argument_count; a++) {
					printf("%llu ", arguments[a]);
				}
				puts("}");

			} else { 
				arguments[argument_count++] = values[j];
			}

			goto found_name;
			next_name:;
		}
		printf("error: could not resolve symbol near %llu, aborting.\n", index);
		abort();
		found_name:;
		
	}
	//puts("------------------------");

	puts("current names:");
	for (nat i = 0; i < name_count; i++) {
		printf("#%llu: @%llu:len=%llu:val=%llu ", i, names[i], lengths[i], values[i]);
		for (nat c = names[i]; text[c] != '"'; c++) putchar(text[c]);
		puts("");
	}

	munmap(text, length);
}

/*


//puts("toggling name state!");
			//puts(defining ? "[defining]" : "[executing]");









*/

