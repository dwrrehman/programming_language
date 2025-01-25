// 1202501245.202705  : dwrr
// an interpreter for the programming language, 
// to allow for testing of using it, 
// without the entire compiler being finished lol.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
typedef uint64_t nat;

enum {
	nullins, 
	zero, incr, decr, 
	not_, and_, or_, eor, si, sd,
	set, add, sub, mul, div_,
	lt, eq, ld, st, sc,
	do_, at, lf, ge, ne, ct, rt,
	isa_count,
};

static const char* ins_spelling[] = {
	"__ERROR_null_ins_unused__", 
	"zero", "incr", "decr", 
	"not", "and", "or", "eor", "si", "sd",
	"set", "add", "sub", "mul", "div", 
	"lt", "eq", "ld", "st", "sc",
	"do", "at", "lf", "ge", "ne", "ct", "rt",
	"__ISA_COUNT__ins_unused__",
};

struct instruction {
	nat op;
	nat ct;
	nat gotos[2];
	nat args[7];
};

enum language_systemcalls {
	system_exit,
	system_read, system_write, 
	system_open, system_close,
	systemcall_count
};

static const char* systemcall_spelling[systemcall_count] = {
	"system_exit",
	"system_read", "system_write", 
	"system_open", "system_close",
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
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

static void print_dictionary(char** names, nat* active, 
	nat* definition, nat* ctk, nat* values, 
	nat* locations, nat* bit_count, nat name_count
) {
	puts("found dictionary: { \n");
	for (nat i = 0; i < name_count; i++) {
		printf("\t%3llu: name = \"%-10s\", active = %3llu, "
			"def = %3llu, ctk = %3llu, value = %3llu, "
			"location = %3lld, bit_count = %3llu\n",
			i, names[i], active[i], definition[i], ctk[i], 
			values[i], locations[i], 
			bit_count[i]
		);
	}
	puts("}");
}

static const nat ct_is_generated_do = 8;
static const nat ct_is_unreachable = 4;
static const nat ct_is_ctbranch_side = 2;
static const nat ct_is_compiletime = 1;

static const bool use_color = 1;

static void print_instruction(struct instruction this, char** names, nat name_count) {

	if (use_color) {
		if (this.ct & ct_is_unreachable) printf("\033[38;5;239m");
		else if (this.ct & ct_is_compiletime) printf("\033[38;5;101m");
	}

	printf("[.ct=%llx]", this.ct);
	printf("  %8s { ", ins_spelling[this.op]);

	for (nat a = 0; a < 3; a++) {
		if (this.args[a] < 256) printf("%3llu", this.args[a]); 
		else printf("0x%016llx", this.args[a]);
		printf("('%6s') ", this.args[a] < name_count ? names[this.args[a]] : "");
	}

	printf("} : {.f=#%lld .t=#%lld} "
		" %s"
		" %s"
		" {ct_side=%u}"
		" %s",
		this.gotos[0], this.gotos[1],
		!!(this.ct & ct_is_generated_do) ? "[machine-do]" : "", 
		!!(this.ct & ct_is_unreachable) ? "[unreachable]" : "",
		!!(this.ct & ct_is_ctbranch_side), 
		!!(this.ct & ct_is_compiletime) ? "[compiletime]" : ""
	);

	if (use_color) {
		if (this.ct & ct_is_unreachable) printf("\033[0m");
		else if (this.ct & ct_is_compiletime) printf("\033[0m");
	}
}

static void print_instructions(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count
) {
	puts("found instructions: { \n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		puts("");
	}
	puts("}");
}

static void print_instruction_index(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count,
	nat here, const char* message
) {
	printf("%s: at index: %llu: { \n\n", message, here);
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		if (i == here)  printf("  <------ %s\n", message); else puts("");
	}
	puts("}");
}

/*static void print_stack(nat* stack, nat stack_count) {
	printf("stack: %llu { \n", stack_count);
	for (nat i = 0; i < stack_count; i++) {
		printf("stack[%llu] =  %llu\n", 
			i, stack[i]
		);
	}
	puts("}");
}*/

static void print_ct_values(char** names, nat name_count, nat* ctk, nat* values) {
	printf("ct values: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (not ctk[i]) continue;
		printf("\tvalues[%s] = 0x%016llx\n", names[i], values[i]);
	}
	puts("}");
}

/*static void print_machine_instructions(struct instruction* mi, nat mi_count) {
	printf("printing machine instructions: (%llu)\n", mi_count);
	for (nat i = 0; i < mi_count; i++) {
		printf("#%llu: MACHINE INSTRUCTION:  "
			" %s  :  %llu %llu %llu %llu %llu \n",
			i, mi_spelling[mi[i].op],
			mi[i].args[0], mi[i].args[1],
			mi[i].args[2], mi[i].args[3], 
			mi[i].args[4]
		);
	}
}
*/
static const nat is_label = 1LLU << 63LLU;

int main(int argc, const char** argv) {

	struct instruction ins[4096] = {0};
	nat ins_count = 0;

	char* names[4096] = {0};
	nat active[4096] = {0};
	nat name_count = 0;
	nat definition[4096] = {0};

	nat ctk[4096] = {0};
	nat bit_count[4096] = {0};
	nat values[4096] = {0};
	nat locations[4096] = {0};
	memset(locations, 255, sizeof locations);

	if (argc == 1) exit(puts("usage error"));

	const char* filename = argv[1];
	char* text = NULL;
	nat text_length = 0;
{
	int f = open(filename, O_RDONLY);
	text_length = (nat) lseek(f, 0, SEEK_END);
	text = calloc(text_length + 1, 1);
	lseek(f, 0, SEEK_SET);
	read(f, text, text_length);
	close(f);
}
	//printf("parsing this text: (%llu) \n", text_length);
	//puts(text);
	//puts("");

	{nat 	word_length = 0, 
		word_start = 0,
		unreachable = 0,
		state = 0,
		arg_count = 0;

	nat args[7] = {0};
	const nat starting_index = 0;

	for (nat index = starting_index; index < text_length; index++) {
		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++; 
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;
		char* word = strndup(text + word_start, word_length);
		//printf("filename:%llu: info: looking at \"%s\" @ %llu\n", 
		//	index, word, word_start
		//);

		if (not state) {
			arg_count = 0;
			if (not strcmp(word, "eoi")) break;
			for (; state < isa_count; state++) {
				if (not strcmp(word, ins_spelling[state])) {
					if (state == at) unreachable = 0;
					goto next_word; 
				}
			}
			print_error: 
			printf("%s:%llu:%llu: error: undefined %s \"%s\"\n", 
				filename, word_start, index, 
				state == isa_count ? "operation" : "variable", word
			); 
			print_index(text, text_length, word_start, index);
			abort();

		} else {
			const nat saved_name_count = name_count;
			nat variable = 0;
			for (; variable < name_count; variable++) {
				if (not strcmp(word, names[variable]) and active[variable]) goto found;
			}

			names[name_count++] = word; 
			active[name_count - 1] = 1;
			definition[name_count - 1] = ins_count;
		found:
			args[arg_count++] = variable;

			if (state == do_) {
				unreachable = 1; state = 0;
				struct instruction new = {
					.op = eq,
					.ct = unreachable ? 5 : 1,
					.args = {0, 0, variable},
					.gotos = {0, variable | is_label},
				};
				ins[ins_count++] = new;
								
			} else if (state == at) {
				state = 0;
				locations[variable] = ins_count;

			} else if (state == ct) {
				state = 0;
				ctk[variable] = 1;

			} else if (state == rt) {
				if (arg_count < 2) goto next_word;
				state = 0;
				ctk[args[0]] = 0;
				bit_count[args[0]] = variable;

			} else if (state == zero) {
				push_ins:; nat op = state; state = 0;
				
				struct instruction new = {
					.op = op,
					.ct = unreachable ? 4 : 0,
					.gotos = {ins_count + 1, 
						op == lt or op == ge or 
						op == ne or op == eq ? 
						variable | is_label : 0
					},
				};
				memcpy(new.args, args, sizeof args);
				if (op == ge or op == ne) { 
					new.gotos[0] = new.gotos[1]; 
					new.gotos[1] = ins_count + 1;
					if (op == ge) new.op = lt;
					if (op == ne) new.op = eq;
				}
				ins[ins_count++] = new;

			} else if (state == set) {				
				if (arg_count == 2) goto push_ins;

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
	}}

	puts("after parsing");
	print_instructions(ins, ins_count, names, name_count);
	getchar();

	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].gotos[0] & is_label) {
			ins[i].gotos[0]	= locations[ins[i].gotos[0] & ~is_label];
		}

		if (ins[i].gotos[1] & is_label) {
			ins[i].gotos[1]	= locations[ins[i].gotos[1] & ~is_label];
		} 
	}

	puts("after doing stage 1.5");
	print_instructions(ins, ins_count, names, name_count);
	getchar();

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == lt or op == eq) {} else ins[i].gotos[1] = (nat) -1;
	}
	
	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	puts("current ins listing after parsing..");
	print_instructions(ins, ins_count, names, name_count);
	puts("about to execute these instructions...");
	getchar();

	nat pc = 0;
	while (pc < ins_count) {		

		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];
		//const nat arg2 = ins[pc].args[2];

		if (op == lt or op == eq) {
			nat cond = 0;
			if (op == lt) cond = (values[arg0] < values[arg1]);
			if (op == eq) cond = (values[arg0] == values[arg1]);			
			pc = ins[pc].gotos[cond];
			continue;

		} else if (op == sc) {			
			const nat n = values[arg0];
			if (n == system_exit) exit((int) values[arg1]);
			else if (n == 9) printf("DEBUG: %llu : 0x%016llx\n", values[arg1], values[arg1]);
			else { 
				printf("info: found uniplemented %s sc!\n", systemcall_spelling[n < 4 ? n : 0]);
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, "SYSTEM CALL HERE"
				); 
				abort(); 
			}

		} else if (op == ld) {

		} else if (op == st) {

		}
		else if (op == zero) values[arg0] = 0; 
		else if (op == incr) values[arg0]++;
		else if (op == decr) values[arg0]--;
		else if (op == not_) values[arg0] = ~values[arg0]; 
		else if (op == set)  values[arg0] = values[arg1];
		else if (op == add)  values[arg0] += values[arg1];
		else if (op == sub)  values[arg0] -= values[arg1];
		else if (op == mul)  values[arg0] *= values[arg1];
		else if (op == div_) values[arg0] /= values[arg1];
		else if (op == and_) values[arg0] &= values[arg1];
		else if (op == or_)  values[arg0] |= values[arg1];
		else if (op == eor)  values[arg0] ^= values[arg1];
		else if (op == si)   values[arg0] <<= values[arg1];
		else if (op == sd)   values[arg0] >>= values[arg1];
		else {
			puts("internal error: op execution not specified");
			printf("op = %llu, op = %s\n", op, ins_spelling[op]);
			abort();
		}
		pc = ins[pc].gotos[0];
	}

	print_ct_values(names, name_count, ctk, values);
	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);
	exit(0);
}







