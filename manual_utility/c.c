#include <stdio.h> // written by dwrr on 202312052.144458
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>

struct dictionary_entry {
	const char* name;
	const char* description;
	const char* usage;
	const char* notes;
};

static const struct dictionary_entry manual[] = {
	{
		.name = "movzx", 
		.usage = " imm16 sh2 Rd movzx",
                .description = "64 bit move and zero. argument0 is the destination runtime register. argument1 is the 2-bit shift amount, which should be a register 0 through 3. argument2 is the 16-bit immediate value. the destination is first zeroed, and then the shifted immediate is loaded into it. sh2 == r1 shifts the imm16 immediate left by 16 bits, sh2 == r2 shifts by 32 bits, and r3 shifts by 48 bits.",
                .notes = "this instruction is used in combination with the movkx/w instructions in order to load a full 64 bit immediate value into a register. use a single movz, followed by 0 or more movk instructions to do this.",
	},
	
	{
		.name = "movzw", 
		.usage = "imm16 sh2 Rd movzw",
		.description = "32 bit move and zero. for semantics, see movzx. sh2 should only be r1 or r0.",
		.notes = "",
	},

	{
                .name = "movkw", 
                .usage = "imm16 sh2 Rd movkw",
                .description = "32 bit move and keep. for semantics see movkx. sh2 should only be r1 or r0.",
                .notes = "",
	},

	{
                .name = "movkx", 
                .usage = "imm16 sh2 Rd movkx",
                .description = "64 bit move and keep. argument0 is the destination runtime register. argument1 is the 2-bit shift amount, which should be a register 0 through 3. argument2 is the 16-bit immediate value. the destination register bits above the shifted immediate value are preserved. the shift argument sh2 has the same semantics as the sh2 argument2 of movzx.",
                .notes = "",
        },
	
	{
		.name = "ctadd",
		.usage = "Cm Cn Cd ctadd",
		.description = "compiletime add. adds the compiletime registers Cm and Cn and puts the sum in Cd. all registers are unsigned.",
		.notes = "uses the + operator in C, and has the same semantics as such.",
	}
};

static void print_string_wrapped(const char* string) {

	printf("\t\t\033[1m"); 
        for (size_t i = 0, w = 0; i < strlen(string); i++, w++) {
                if (w >= 60 and string[i] == 32) { printf(" \n\t\t"); w = 0; }
                else putchar(string[i]);
        }
        puts("\033[0m\n");
}

static void print_entry(struct dictionary_entry word) {
	printf("\n\n\t\033[1m%s\033[0m\n\n\n", word.name);	
	printf("\n\t\033[1;31musage:\033[0m\t\t\033[1m%s\033[0m\n\n", word.usage);

	printf("\n\t\033[1;32mdescription:\033[0m\n\n");
	print_string_wrapped(word.description);

	printf("\n\t\033[1;34mnotes:\033[0m\n\n");	
	print_string_wrapped(word.notes);
}

static void print_list(size_t count) {	
	printf("all words:\n\t");
	for (size_t i = 0; i < count; i++) {
		if (i % 8 == 0) puts("");
		printf("%s ", manual[i].name);
	}
	printf("\n\n[end]\n");
}


int main(int argc, const char** argv) {
	
	const size_t count = sizeof manual / sizeof(*manual);	

	if (argc != 1) {
	        for (size_t i = 0; i < count; i++) {
	                if (not strcmp(manual[i].name, argv[1])) {
	                        print_entry(manual[i]);
				exit(0);
                	}
        	}
	        printf("error: word not found: %s\n", argv[1]);
		exit(1);
	}

	char buffer[128] = {0};	
	printf("this is a utilty to look up the documentation for a given word in the dictionary, in my programming language.\n");
loop:
	printf(":: ");
	fgets(buffer, sizeof buffer, stdin);
	buffer[strlen(buffer) - 1] = 0;

	if (not strcmp(buffer, "_quit")) exit(0);
	else if (not strcmp(buffer, "_clear")) { printf("\033[2J\033[H"); goto loop; }
	else if (not strcmp(buffer, "_list")) { print_list(count); goto loop; }

	for (size_t i = 0; i < count; i++) {
		if (not strcmp(manual[i].name, buffer)) {
			print_entry(manual[i]);
			goto loop;
		}
	}
	
	printf("error: word not found: %s\n", buffer);	
	goto loop;
}

