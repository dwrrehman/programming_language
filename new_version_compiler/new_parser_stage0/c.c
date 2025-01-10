#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdint.h>
#include <ctype.h>
typedef uint64_t nat;

enum {
	nullins, 
	zero, incr, decr, 
	not_, and_, or_, eor, si, sd,
	set, add, sub, mul, div_, 
	lt, eq, ld, st, sc,	
	do_, at, lf, ge, ne, ct, rt,
	isa_count
};

static const char* ins_spelling[] = {
	"nullins", 
	"zero", "incr", "decr", 
	"not", "and", "or", "eor", "si", "sd",
	"set", "add", "sub", "mul", "div", 
	"lt", "eq", "ld", "st", "sc",
	"do", "at", "lf", "ge", "ne", "ct", "rt" 
};

struct instruction {
	nat op;
	nat gotos[2];
	nat args[7];
};

static void print_index(const char* text, nat text_length, nat begin, nat end) {
	printf("\n\t@%llu..%llu: ", begin, end); 
	for (nat i = 0; i < text_length; i++) {
		if (i == begin) printf("\033[32;1m[");
		putchar(text[i]);
		if (i == end) printf("]\033[0m");
	}
	puts("\n");
}

int main(int argc, const char** argv) {


	struct instruction ins[4096] = {0};
	nat ins_count = 0;

	char* names[4096] = {0};
	nat ctk[4096] = {0};
	nat bit_count[4096] = {0};
	nat values[4096] = {0};
	nat name_count = 0;
	nat locations[4096] = {0};
	nat args[7] = {0};
	
	nat 
		word_length = 0, 
		word_start = 0,
		unreachable = 0,
		state = 0,
		arg_count = 0
	;

	const nat is_label = 1LLU << 63LLU;

	//const char* text = "ct 5 zero 5 zero i at loop incr i lt i 5 loop zero i";
	//const char* text = "do skip do done at skip zero i at done";
	//const char* text = "ct 5 zero 5 zero i at loop ge i 5 done incr i do loop at done zero i";
	//const char* text = "ct 5 zero 5 zero i at loop ge i 5 done do skip zero i incr i incr i at skip incr i do loop at done zero i";
	//const char* text = "ct 5 zero 5 zero i add i 5 add i 5 incr i";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";
	//const char* text = "add hello hello";
	//const char* text = "incr hello";
	//const char* text = "lt hello hello label";
	//const char* text = "zero i lt i i done";
	//const char* text = "sc 0  0 0 0  0 0 0 ";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";


	if (argc == 1) exit(puts("usage error"));
	const char* text = argv[1];

	const nat text_length = (nat) strlen(text);

	printf("parsing this text: (%llu) \n", text_length);
	puts(text);
	puts("");


	const char* filename = "file.s";

	for (nat index = 0; index < text_length; index++) {
		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++; 
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;

		char* word = strndup(text + word_start, word_length);

		printf("filename:%llu: info: looking at \"%s\" @ %llu\n", index, word, word_start);

		if (not state) {
			arg_count = 0;
			memset(args, 255, sizeof args);//optional
			if (not strcmp(word, "eoi")) break;
			for (; state < isa_count; state++) {
				if (not strcmp(word, ins_spelling[state])) {
					if (state == at) unreachable = 0;
					else if (unreachable) {
						printf("%s:%llu:%llu: warning: unreachable instruction\n", 
							filename, word_start, index
						);
						print_index(text, text_length, word_start, index);
					}
					goto next_word; 
				}
			}

			print_error: 
			printf("%s:%llu:%llu: error: undefined %s \"%s\"\n", 
				filename, word_start, index, 
				state == isa_count ? "operation" : "variable", 
				word
			); 
			print_index(text, text_length, word_start, index);
			abort();


		} else {
			const nat saved_name_count = name_count;
			nat variable = 0;
			for (; variable < name_count; variable++) 
				if (not strcmp(word, names[variable])) goto found;

			names[name_count++] = word;
		found:
			args[arg_count++] = variable;

			if (state == do_) {
				state = 0;
				if (unreachable) goto next_word;
				ins[ins_count - 1].gotos[0] = variable | is_label;
				unreachable = 1;
								
			} else if (state == at) {
				state = 0;
				locations[variable] = ins_count;

			} else if (state == ct) {
				state = 0;
				if (unreachable) goto next_word;
				ctk[variable] = 1;

			} else if (state == rt) {
				state = 0;
				if (unreachable) goto next_word;
				ctk[args[0]] = 0;
				bit_count[args[0]] = variable;

			} else if (state == zero) {
				push_ins:; nat op = state; state = 0;
				if (unreachable) goto next_word;
				struct instruction new = {
					.op = op,
					.gotos = {ins_count + 1, variable | is_label},
				};
				memcpy(new.args, args, sizeof args);
				if (op == ge or op == ne) { 
					new.gotos[0] = new.gotos[1]; 
					new.gotos[1] = ins_count + 1;
					if (op == ge) new.op = lt;
					if (op == ne) new.op = eq;
				}
				ins[ins_count++] = new;

			} else if (state == incr or state == decr or state == not_) {
				if (variable == saved_name_count) goto print_error;
				goto push_ins;
			} else if (	state == add or state == sub or 
					state == mul or state == div_ or 
					state == and_ or state == or_ or 
					state == eor or state == si or state == sd) {
				if (arg_count < 2 and variable == saved_name_count) goto print_error;
				if (arg_count == 2) goto push_ins;
			} else if (state == lt or state == eq or state == ge or state == ne) {
				if (arg_count < 3 and variable == saved_name_count) goto print_error;
				if (arg_count == 3) goto push_ins;
			} else if (state == sc) {
				if (arg_count < 7 and variable == saved_name_count) goto print_error;
				if (arg_count == 7) goto push_ins;
			} else {
				printf("error: parsing state unimplemented: %llu: %s\n", state, ins_spelling[state]);
				abort();
			}
		}	
		next_word: word_length = 0;
	}









	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].gotos[0] & is_label) {
			ins[i].gotos[0]	= locations[ins[i].gotos[0] & ~is_label];
		}
		if (ins[i].gotos[1] & is_label) {
			ins[i].gotos[1]	= locations[ins[i].gotos[1] & ~is_label];
		}
	}

	puts("found dictionary: { \n");
	for (nat i = 0; i < name_count; i++) {
		printf("\t%llu: name = \"%s\", ctk = %llu, value = %llu, location = %llu, bit_count = %llu\n",
			i, names[i], ctk[i], 
			values[i], locations[i], 
			bit_count[i]
		);
	}
	puts("}");

	puts("found instructions: { \n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%llu: %s : {%lld %lld %lld %lld} : {.f=#%lld .t=#%lld}\n", 
			i, ins_spelling[ins[i].op], 
			ins[i].args[0], ins[i].args[1], 
			ins[i].args[2], ins[i].args[3],
			ins[i].gotos[0], ins[i].gotos[1]
		);
	}
	puts("}");

	exit(0);
}






















				//printf("arg_count = %llu\n", arg_count);
				//printf("variable = %llu\n", variable);
				//printf("name_count = %llu\n", variable);






















/*

ct 5 
zero 5 

zero i 
at loop 
	incr i 
	lt i 5 loop 
zero i

*/






















































	// add names[] and ctk[] and values[] arrays. 

	// delete    ct and rt  ins first, 
	//  then figure out how we'll do unreachable analysis for  do instructions, 
	//  and also figure out how we'll only "execute" a do instruction if its reachable,
	//  and attribute the location into the .false side of the previous instruction. 

	// at the end of this whole process, we need to do the   branch complementation thingy 
	// to switch the .true and .false  if we are using a nonexistent condition:  ge ne

	// lf is handled already. same with eoi.     we need to make    at handled by 
	//  actually using like a mapping of   addresses to label names, i think!
	// and if we are using ip++, then we just need to figure out how to make the mapping between the frist instruction and the second in the chain   which ip++   connects. 








/*


struct instruction {
	nat op;
	nat label;
	nat gotos[2];
	nat args[7];
};








*/




/*static const nat arity[] = {
	0, 
	1, 1, 1, 
	1, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 
	3, 3, 3, 3, 7,
	0,
	1, 1, 1, 3, 3, 1, 2
};
*/




