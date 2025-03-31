#include <iso646.h>  // wrote on 2209117.003323 by dwrr.  a minimalist experimental 
#include <stdio.h>   // programming language with a very small repl/interpreter.
#include <string.h>  // still work in progress.
typedef unsigned long long nat;
static nat count = 0, i = 0, r[128] = {0}, exec[4096] = {0};
int main() {
	puts("my language repl/interpreter v2");
	char line[4096] = {0};	
	_: printf(" • ");
	fgets(line, sizeof line, stdin);
	nat line_length = strlen(line);
	char* string = strdup(line);
	string[line_length - 1] = 0;
	if (not strcmp(string, "quit")) goto done;
	else if (not strcmp(string, "clear")) printf("\033[H\033[J");
	else if (not strcmp(string, "debug")) {for (nat _ = 0; _ < count; _++) printf("%04llx ",exec[_]); puts("");}
	else for (nat ip = 0; ip < line_length; ip++) {
		if (line[ip] == 'a') i = 0;
		else if (line[ip] == 's') i++;
		else if (line[ip] == 'h') r[i] = 0;
		else if (line[ip] == 't') r[i]++; 
		else if (line[ip] == 'n') r[127] = 0; 
		else if (line[ip] == 'e') r[127]++;
		else if (line[ip] == 'o') { if (r[i] < r[127]) ip -= r[0]; }
		else if (line[ip] == 'i') { if (r[i] < r[127]) ip++; }
		else if (line[ip] == 'u') r[i] = (nat) line[ip++];
		else if (line[ip] == 'r') exec[count++] = r[i]; } 
	goto _; done:;
}
