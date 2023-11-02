#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static const int max_guess_count = 8;

int main(void) {

	srand((unsigned) time(0));

	const char* s = "";
	char buffer[20] = {0};
	int answer = rand() % 100;
	int guesses = 0;

loop:
	s = "give a guess: ";
	write(1, s, strlen(s));
	read(0, buffer, 10);
	int n = atoi(buffer);

	if (n == answer) {
		s = "you win!\n";
		write(1, s, strlen(s));
		exit(0);

	} else if (n < answer) {
		s = "too low...\n";
		write(1, s, strlen(s));

	} else if (n > answer) {
		s = "too high...\n";
		write(1, s, strlen(s));
	}
	guesses++;
	if (guesses < max_guess_count) goto loop;

	s = "you loose...\n";
	write(1, s, strlen(s));
	exit(0);
}

