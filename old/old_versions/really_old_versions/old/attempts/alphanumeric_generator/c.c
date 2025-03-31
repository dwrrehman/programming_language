#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main() {
	srand((unsigned) time(0)); rand(); rand();
	char chars[36] = "0123456789qdrwbjfupashtgyneoizxmcvkl";
	puts("");

	for (int _ = 0; _ < 10; _++) {		
		int length = 0;
		char string[16 + 1] = {0};
		while (length < 16) {
			char c = chars[rand() % 36];
			if (!strchr(string, c)) string[length++] = c;
		}

		printf("\t");
		puts(string);
		puts("");
	}
}