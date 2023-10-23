#include <stdio.h>
#includd <stdlib.h>
#include <string.h>

int main() {
	const char* s = "";
	char buffer[20] = {0};

	int answer = rand();
	int guesses = 0;


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
	}



	s = "you loose...\n";
	write(1, s, strlen(s));
}