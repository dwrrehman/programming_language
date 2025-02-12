/*

targets applications: 
---------------------------------------------
	- embedded risc low power devices, 
	- modern high performance risc machines in data centers


target ISAs:
---------------------------------------------
	- ARM64
	- ARM32
	- RISC-V 64
	- RISC-V 32
	- MSP430


performance advantages over C / LLVM:
---------------------------------------------

	- full program ins-level optimizations (no basic blocks)

	- no SSA representation: stateful optimizations


	- conservative memory aliasing semantics

	- no implicit memory allocations, register level semantics instead

	- no unneccessary ABI requirements on functions, no link-time optimization

	- no unneccessary overhead: no abstractions present
	  which don't easily map onto hardware instructions 


	- bit-width for variables providing more use of registers

	- more powerful compiletime evaluation semantics
	
	- vectorization primitives in the language 






*/

// a new parser and ct system written on 1202502016.131044 dwrr
// 1202501116.181306 new parser   dwrr



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
	
	set, add, sub, mul, div_, 
	and_, or_, eor, si, sd,
	lt, eq, ge, ne, do_, at, 
	ld, st, sc, rt, lf, eoi,

	isa_count,
	set_imm,  add_imm,  sub_imm,
	mul_imm,  div_imm,
	and_imm,  or_imm,   eor_imm,
	si_imm,   sd_imm,
	lt_imm,   gt_imm,   eq_imm,
};

static const char* ins_spelling[] = {
	"__ERROR_null_ins_unused__", 
	
	"set", "add", "sub", "mul", "div", 
	"and", "or", "eor", "si", "sd",
	"lt", "eq", "ge", "ne", "do", "at", 
	"ld", "st", "sc", "rt", "lf", "eoi",

	"__ISA_COUNT__ins_unused__",
	"set_imm", "add_imm", "sub_imm",
	"mul_imm", "div_imm",
	"and_imm", "or_imm",  "eor_imm",
	"si_imm",  "sd_imm",
	"lt_imm",  "gt_imm", "eq_imm",
};

struct instruction {
	nat op;
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

enum arm64_ins_set {
	addsrlsl, 
	addsrlsr,
	subsrlsl, 
	subsrlsr,
	andsrlsl, 
	andsrlsr,
	orrsrlsl, 
	orrsrlsr,
	eorsrlsl, 
	eorsrlsr,
	movz,
	addi,
	madd,
	msub,
	arm_isa_count,
};

static const char* mi_spelling[arm_isa_count] = {
	"addsrlsl", 
	"addsrlsr",
	"subsrlsl", 
	"subsrlsr",
	"andsrlsl",
	"andsrlsr",
	"orrsrlsl", 
	"orrsrlsr",
	"eorsrlsl", 
	"eorsrlsr",
	"movz",
	"addi",
	"madd",
	"msub",
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

static const nat is_label = 1LLU << 63LLU;

static nat get_call_output_count(nat n) {
	if (n == system_exit) return 0;
	if (n == system_read) return 2;
	if (n == system_write) return 2;
	if (n == system_close) return 1;
	if (n == system_open) return 2;
	abort();
}

static void print_index(const char* text, nat text_length, nat begin, nat end) {
	printf("\n\t@%llu..%llu: ", begin, end); 
	for (nat i = 0; i < text_length; i++) {
		if (i == begin) printf("\033[32;1m[");
		putchar(text[i]);
		if (i == end) printf("]\033[0m");
	}
	puts("\n");
}

static void print_dictionary(char** names, nat* locations, nat name_count) {
	puts("found dictionary: { \n");
	for (nat i = 0; i < name_count; i++) {
		printf("\t%3llu: name = \"%-10s\", location = %3lld, \n", 
			i, names[i], locations[i]
		);
	}
	puts("}");
}

static void print_instruction(struct instruction this, char** names, nat name_count) {
	printf("  %8s { ", ins_spelling[this.op]);
	for (nat a = 0; a < 3; a++) {
		if (this.args[a] < 256) printf("%3llu", this.args[a]); 
		else printf("0x%016llx", this.args[a]);
		printf("('%8s') ", this.args[a] < name_count ? names[this.args[a]] : "");
	}

	printf("} : {.f=#");
	if (this.gotos[0] == ~is_label or this.gotos[0] == (nat) -1) {} 
	else if (this.gotos[0] < 256) printf("%3llu", this.gotos[0]); 
	else printf("0x%016llx", this.gotos[0]);
	printf(", .t=#");
	if (this.gotos[1] == ~is_label or this.gotos[1] == (nat) -1) {} 
	else if (this.gotos[1] < 256) printf("%3llu", this.gotos[1]); 
	else printf("0x%016llx", this.gotos[1]);
	printf("}");
}

static void print_machine_instruction(struct instruction this, char** names, nat name_count) {
	printf("  %8s { ", mi_spelling[this.op]);
	for (nat a = 0; a < 5; a++) {
		if (this.args[a] < 256) printf("%3llu", this.args[a]); 
		else printf("0x%016llx", this.args[a]);
		printf("('%8s') ", this.args[a] < name_count ? names[this.args[a]] : "");
	}

	printf("} : {.f=#");
	if (this.gotos[0] == ~is_label or this.gotos[0] == (nat) -1) {} 
	else if (this.gotos[0] < 256) printf("%3llu", this.gotos[0]); 
	else printf("0x%016llx", this.gotos[0]);
	printf(", .t=#");
	if (this.gotos[1] == ~is_label or this.gotos[1] == (nat) -1) {} 
	else if (this.gotos[1] < 256) printf("%3llu", this.gotos[1]); 
	else printf("0x%016llx", this.gotos[1]);
	printf("}");
}

static void print_instructions(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count
) {
	puts("found instructions: {");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		puts("");
	}
	puts("}");
}

static void print_stack(nat* stack, nat stack_count) {
	printf("stack: %llu { \n", stack_count);
	for (nat i = 0; i < stack_count; i++) {
		printf("stack[%llu] =  %llu\n", 
			i, stack[i]
		);
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
}*/

static void print_machine_instructions(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count
) {
	printf("MACHINE INSTRUCTIONS: (%llu count) {\n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_machine_instruction(ins[i], names, name_count);
		puts("");
	}
	puts("}");
}



static void print_instruction_index(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count,
	nat here, const char* message
) {
	printf("%s: at index: %llu: { \n", message, here);
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		if (i == here)  printf("  <------ %s\n", message); else puts("");
	}
	puts("}");
}

static void print_instructions_ct_values_index(
	struct instruction* ins, const nat ins_count,
	char** names, const nat name_count, nat* locations,
	nat* execution_state_ctk, nat* execution_state_values,
	nat pc, const char* message
) {
	printf("@%lld: %s: (%llu instructions)\n", pc, message, ins_count);
	for (nat i = 0; i < ins_count; i++) {
		bool found = false;
		for (nat l = 0; l < name_count; l++) {
			if (i == locations[l]) { printf("LABEL{%s}: ", names[l]); found = true; } 
		}
		if (found) puts("");
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		nat* values = execution_state_values + name_count * i;
		nat* ctk = execution_state_ctk + name_count * i;
		printf(" -- ct { ");
		for (nat n = 0; n < name_count; n++) {
			if (not ctk[n]) continue;
			printf("%s:%lld ", names[n], values[n]);
		}
		printf("}  ");
		if (i == pc) printf("    <--- %s\n", message); else puts("");
	}
	printf("done printing index \"%s\"\n", message);
}

static nat* compute_predecessors(struct instruction* ins, const nat ins_count, const nat pc, nat* pred_count) {
	nat* result = NULL;
	nat count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].gotos[0] == pc or ins[i].gotos[1] == pc) {
			result = realloc(result, sizeof(nat) * (count + 1));	
			result[count++] = i;
		}
	}
	*pred_count = count;
	return result;
}

static void print_ct_values(char** names, nat name_count, nat* is_runtime, nat* values) {
	printf("ct values: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (is_runtime[i]) continue;
		printf("\tvalues[%s] = 0x%016llx\n", names[i], values[i]);
	}
	puts("}");
}

static nat locate_data_instruction(
	nat expected_op, nat expected_arg0, nat expected_arg1,
	nat use_arg0, nat use_arg1,
	nat starting_from,
	struct instruction* ins, const nat ins_count,
	char** names, nat name_count
) {
	nat pc = starting_from;

	while (pc < ins_count) {
		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];

		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);

		if (is_branch) { printf("FOUND RT BRANCH @ %llu\n", pc); break; }

		print_instruction_index(ins, ins_count, names, name_count, pc, "FOLLOWING");
		printf("following pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");
		if (
			op == expected_op and 
			(not use_arg0 or expected_arg0 == arg0) and 
			(not use_arg1 or expected_arg1 == arg1)
		) {
			printf("SUCCESS: FOUND: WE FOUND WHAT WE WERE LOOKING FOR!!!\n");
			return pc;

		} else {
			printf(
				"failed: we didnt find a match here: \n"
				"\t expected op: %s      found op: %s  --->  [%s]\n"
				"\t expected arg0: %lld     found arg0: %lld   ---> [%s]\n"
				"\t expected arg1: %lld     found arg1: %lld  ---> [%s]\n"
				"\n\n", 
				ins_spelling[expected_op], ins_spelling[op],   op == expected_op ? "MATCHES" : "mismatch",
				expected_arg0, arg0, (not use_arg0 or expected_arg0 == arg0) ? "MATCHES" : "mismatch",
				expected_arg1, arg1, (not use_arg1 or expected_arg1 == arg1) ? "MATCHES" : "mismatch"
			); 
			if (use_arg0 and (arg0 == expected_arg0 or arg1 == expected_arg0)) {
			puts("WARNING: dest: FOUND DATA COLLISON ON DESTINATION ARGUMENT\n"); getchar();
				break;
			}
				
				if (use_arg1 and arg0 == expected_arg1) {
				puts("WARNING: source: FOUND DATA COLLISON ON SOURCE ARGUMENT\n"); getchar();
				break;
			}
		}
		pc = ins[pc].gotos[0];
	}
	printf("SELECTION: FOUND A PC OF: %llu\n", pc); getchar();
	return (nat) -1;
}

int main(int argc, const char** argv) {

	struct instruction ins[4096] = {0};
	nat ins_count = 0;
	char* names[4096] = {0};
	nat name_count = 0;
	nat locations[4096] = {0};
	nat is_runtime[4096] = {0};
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
	printf("parsing this text: (%llu) \n", text_length);
	puts(text);
	puts("");

	{nat 	word_length = 0, 
		word_start = 0,
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
		printf("file:%llu: at \"%s\" @ %llu\n", index, word, word_start);

		if (not state) {
			arg_count = 0;
			if (not strcmp(word, "eoi")) break;
			for (; state < isa_count; state++) {
				if (not strcmp(word, ins_spelling[state])) goto next_word;
			}
			print_error:
			printf("%s:%llu:%llu: error: undefined %s \"%s\"\n",
				filename, word_start, index, 
				state == isa_count ? "operation" : "variable", word
			); 
			print_index(text, text_length, word_start, index);
			abort();

		} else {
			//const nat last = name_count;
			nat variable = 0;
			for (; variable < name_count; variable++) {
				if (not strcmp(word, names[variable])) goto variable_name_found;
			}

			const bool valid_op = 
				state == lt or 
				state == eq or 
				state == ge or 
				state == ne or 
				state == eor or 
				state == set;

			names[name_count++] = word; 

		variable_name_found:
			args[arg_count++] = variable;
			if (state == do_) {
				state = 0;
				struct instruction new = {
					.op = eq,
					.args = {0, 0, variable},
					.gotos = {0, variable | is_label},
				};
				ins[ins_count++] = new;
								
			} else if (state == at) {
				state = 0;
				locations[variable] = ins_count;

			} else if (state == rt) {
				state = 0;
				is_runtime[variable] = 1;
								
			} else if (state == set and arg_count == 2) {
				push_ins:; nat op = state; state = 0;
				
				struct instruction new = {
					.op = op,
					.gotos = {ins_count + 1, 
						op == lt or op == ge or 
						op == ne or op == eq ? 
						variable | is_label : (nat) ~is_label
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

			/*} else if (state == incr or state == decr or state == not_) {
				if (variable == last) goto print_error;
				goto push_ins;*/

			} else if (	state == add or state == sub or 
					state == mul or state == div_ or 
					state == and_ or state == or_ or 
					state == eor or state == si or state == sd) {
				if (arg_count < 2 and variable == last) goto print_error;
				if (arg_count == 2) goto push_ins;

			} else if (state == ld or state == st) {
				//if (arg_count < 3 and variable == last) goto print_error;
				if (arg_count == 3) goto push_ins;

			} else if (state == lt or state == eq or state == ge or state == ne) {
				if (arg_count < 3 and variable == last) goto print_error;
				if (arg_count == 3) goto push_ins;

			} else if (state == sc) {
				if (arg_count < 7 and variable == last) goto print_error;
				if (arg_count == 7) goto push_ins;
			} else {
				printf("error: parsing: %llu: %s\n", state, ins_spelling[state]);
				abort();
			}
		}	
		next_word: word_length = 0;
	}}

	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].gotos[0] & is_label) {
			ins[i].gotos[0]	= locations[ins[i].gotos[0] & ~is_label];
		}
		if (ins[i].gotos[1] & is_label) {
			ins[i].gotos[1]	= locations[ins[i].gotos[1] & ~is_label];
		}
	}

	print_dictionary(names, locations, name_count);
	print_instructions(ins, ins_count, names, name_count);
	getchar();

	nat stack_count = 1;
	nat* stack = calloc(ins_count, sizeof(nat));
	nat* visited = calloc(ins_count + 1, sizeof(nat));
	nat values[4096] = {0};
	nat ignore[4096] = {0};

	while (stack_count) {
		print_stack(stack, stack_count);
		const nat pc = stack[--stack_count];

		print_ct_values(names, name_count, is_runtime, values);
		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");
		getchar();

		visited[pc] = 1;
		const nat op = ins[pc].op;

		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];
		const nat goto0 = ins[pc].gotos[0];
		const nat goto1 = ins[pc].gotos[1];
		const nat ct0 = not is_runtime[arg0];
		const nat ct1 = not is_runtime[arg1];
		const nat rt0 = is_runtime[arg0];
		const nat rt1 = is_runtime[arg1];
		const nat val0 = values[arg0];
		const nat val1 = values[arg0];
		
		if (op == lt or op == eq) {
			if (ct0 and ct1) {
				nat c = 0;
				if (op == eq and val0 == val1 or op == lt and val0 < val1) c = 1;
				if (ins[pc].gotos[c] < ins_count) stack[stack_count++] = ins[pc].gotos[c];
				continue;

			} else if (ct0 or ct1) {
				if (ct0) ins[pc].args[0] = ins[pc].args[1];
				ins[pc].args[1] = val0;
				ins[pc].op = op == lt ? gt_imm : eq_imm;
			}

			if (goto0 < ins_count and not visited[goto0]) stack[stack_count++] = goto0;
			if (goto1 < ins_count and not visited[goto1]) stack[stack_count++] = goto1;
			continue;
		

		} else if (op == sc) {
			if (not rt0) { 
				puts("error: all system calls must be compile time known."); 
				abort();
			}

			const nat n = val0;
			const nat output_count = get_call_output_count(n);
			for (nat i = 0; i < output_count; i++) {
				if (not is_runtime[ins[pc].args[1 + i]]) { puts("system call ct rt out"); abort(); }
			}

			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, "CFG termination point here"
				);
				getchar();
				ins[pc].gotos[0] = (nat) -1;
				ins[pc].gotos[1] = (nat) -1;
				continue;
			} else {
				printf("info: found %s sc!\n", systemcall_spelling[n]);
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, systemcall_spelling[n]
				);
			}
			ins[pc].args[0] = values[arg0];
			continue;

		} else if (op >= isa_count or rt0 and rt1) goto next_ins;
		else if (ct0 and ct1) {
			ignore[pc] = 1;
			     if (op == set) values[arg0] = val1;
			else if (op == add) values[arg0] += val1;
			else if (op == sub) values[arg0] -= val1;
			else if (op == mul) values[arg0] *= val1;
			else if (op == div_)values[arg0] /= val1;
			else if (op == and_)values[arg0] &= val1;
			else if (op == or_) values[arg0] |= val1;
			else if (op == eor) values[arg0] ^= val1;
			else if (op == si)  values[arg0] <<= val1;
			else if (op == sd)  values[arg0] >>= val1;
			else {
				puts("internal error: op execution not specified");
				printf("op = %llu, op = %s\n", op, ins_spelling[op]);
				abort();
			}
			next_ins: if (goto0 < ins_count) stack[stack_count++] = goto0;
			continue;

		} else if (op >= isa_count or rt0 and rt1) goto next_ins;
			if (ct0 and rt1) { puts("error: ct destination must have ct source."); abort(); }

			ins[pc].args[1] = val1;
			if (op == set) ins[pc].op = set_imm;
			if (op == add) ins[pc].op = add_imm;
			if (op == sub) ins[pc].op = sub_imm;
			if (op == mul) ins[pc].op = mul_imm;
			if (op == div_)ins[pc].op = div_imm;
			if (op == and_)ins[pc].op = and_imm;
			if (op == or_) ins[pc].op = or_imm;
			if (op == eor) ins[pc].op = eor_imm;
			if (op == si)  ins[pc].op = si_imm;
			if (op == sd)  ins[pc].op = sd_imm;

			if (	ins[pc].op ==  set and arg0 == arg1 or
				ins[pc].op ==  or_ and arg0 == arg1 or 
				ins[pc].op == and_ and arg0 == arg1 or 
				ins[pc].op == add_imm and values[arg1] == 0 or 
				ins[pc].op == sub_imm and values[arg1] == 0 or 
				ins[pc].op == mul_imm and values[arg1] == 1 or 
				ins[pc].op == div_imm and values[arg1] == 1 or 
				ins[pc].op ==  or_imm and values[arg1] == 0 or
				ins[pc].op == eor_imm and values[arg1] == 0 or
				ins[pc].op ==  si_imm and values[arg1] == 0 or
				ins[pc].op ==  sd_imm and values[arg1] == 0) ignore[pc] = 1;
		} 
		if (goto0 < ins_count) stack[stack_count++] = goto0;
	}

















			//printf("arg0 = 0x%016llx\n", arg0);
			//printf("arg1 = 0x%016llx\n", arg1);
			//fflush(stdout);





	/*

		//print_ct_values(names, name_count, ctk, values);
		//print_instruction_index(ins, ins_count, names, name_count, pc, "PC");


		printf("executing pc #%llu    :    ", pc);
		print_instruction(ins[pc], names, name_count); puts("");


	nat* stack = calloc(ins_count, sizeof(nat));
	nat stack_count = 0;
	stack[stack_count++] = 0; 

	nat* visited = calloc(ins_count + 1, sizeof(nat));


	*/





	while (stack_count) {
		print_stack(stack, stack_count);
		const nat pc = stack[--stack_count];
		print_ct_values(names, name_count, ctk, values);
		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");

		getchar();

		visited[pc] = 1;
		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];

		if (op == lt or op == eq) {

			if (ins[pc].gotos[0] == ins[pc].gotos[1]) {
				ins[pc].ct = (1 << 1) | 1;
				ins[pc].op = eq;
				ins[pc].args[0] = 0;
				ins[pc].args[1] = 0;
				ins[pc].gotos[0] = (nat) -1;
				if (ins[pc].gotos[1] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[1];
				continue;

			} else if (ins[pc].args[0] == ins[pc].args[1]) {

				const nat condition = op == eq;
				ins[pc].ct = (condition << 1) | 1;
				ins[pc].op = eq;
				ins[pc].args[0] = 0;
				ins[pc].args[1] = 0;
				ins[pc].gotos[1] = ins[pc].gotos[condition];
				ins[pc].gotos[0] = (nat) -1;
				if (ins[pc].gotos[1] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[1];
				continue;


			} else if (ctk[arg0] and ctk[arg1]) {
				nat condition = 0;
				if (op == eq and values[arg0] == values[arg1]) condition = 1;
				if (op == lt and values[arg0] <  values[arg1]) condition = 1;
				ins[pc].ct = (condition << 1) | 1;
				if (ins[pc].gotos[condition] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[condition];
				continue;
			}

			if (ctk[arg0]) {
				nat t = ins[pc].args[0]; 
				ins[pc].args[0] = ins[pc].args[1]; 
				ins[pc].args[1] = t;
				if (op == lt) ins[pc].op = gt_imm;
				else ins[pc].op = eq_imm;
				ins[pc].args[1] = values[ins[pc].args[1]];

			} else if (ctk[arg1]) {
				if (op == lt) ins[pc].op = lt_imm;
				else ins[pc].op = eq_imm;
				ins[pc].args[1] = values[ins[pc].args[1]];
			}
			if (ins[pc].gotos[0] < ins_count and 
				not visited[ins[pc].gotos[0]]) 
				stack[stack_count++] = ins[pc].gotos[0];
			if (ins[pc].gotos[1] < ins_count and 
				not visited[ins[pc].gotos[1]]) 
				stack[stack_count++] = ins[pc].gotos[1];

			continue;

		} else if (op == sc) {
			if (not ctk[arg0]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg0];
			const nat output_count = get_call_output_count(n);
			for (nat i = 0; i < output_count; i++) {
				if (ctk[ins[pc].args[1 + i]]) { puts("system call ct rt out"); abort(); }
			}
			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, "CFG termination point here"
				);
				getchar();
				ins[pc].gotos[0] = (nat) -1;
				ins[pc].gotos[1] = (nat) -1;
				goto skip_next;
			} else {
				printf("info: found %s sc!\n", systemcall_spelling[n]);
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, systemcall_spelling[n]
				);
			}

			ins[pc].args[0] = values[arg0];

		} else if (op == zero) { 
			if (ctk[arg0]) { 
				values[arg0] = 0; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = set_imm; 
				ins[pc].args[1] = 0; 
			}

		} else if (op == incr) { 
			if (ctk[arg0]) { 
				values[arg0]++; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = add_imm; 
				ins[pc].args[1] = 1; 
			}
		} else if (op == decr) { 
			if (ctk[arg0]) { 
				values[arg0]--; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = sub_imm; 
				ins[pc].args[1] = 1; 
			}
		} else if (op == not_) { 
			if (ctk[arg0]) { 
				values[arg0] = ~values[arg0]; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = eor_imm; 
				ins[pc].args[1] = (nat) -1; 
			}
		} else {
			if (ctk[arg0] and ctk[arg1]) {
				ins[pc].ct = 1; 
				     if (op == set) values[arg0] = values[arg1];
				else if (op == add) values[arg0] += values[arg1];
				else if (op == sub) values[arg0] -= values[arg1];
				else if (op == mul) values[arg0] *= values[arg1];
				else if (op == div_)values[arg0] /= values[arg1];
				else if (op == and_)values[arg0] &= values[arg1];
				else if (op == or_) values[arg0] |= values[arg1];
				else if (op == eor) values[arg0] ^= values[arg1];
				else if (op == si)  values[arg0] <<= values[arg1];
				else if (op == sd)  values[arg0] >>= values[arg1];
				else {
					puts("internal error: op execution not specified");
					printf("op = %llu, op = %s\n", op, ins_spelling[op]);
					abort();
				}
				goto next_ins;
			}

			//printf("arg0 = 0x%016llx\n", arg0);
			//printf("arg1 = 0x%016llx\n", arg1);
			//fflush(stdout);

			if (op >= isa_count or not ctk[arg0] and not ctk[arg1]) goto next_ins;
			if (    ctk[arg0] and not ctk[arg1]) {
				puts("error: ct destination must have ct source."); 
				abort();
			}
			ins[pc].args[1] = values[arg1];
			if (op == set) ins[pc].op = set_imm;
			if (op == add) ins[pc].op = add_imm;
			if (op == sub) ins[pc].op = sub_imm;
			if (op == mul) ins[pc].op = mul_imm;
			if (op == div_)ins[pc].op = div_imm;
			if (op == and_)ins[pc].op = and_imm;
			if (op == or_) ins[pc].op = or_imm;
			if (op == eor) ins[pc].op = eor_imm;
			if (op == si)  ins[pc].op = si_imm;
			if (op == sd)  ins[pc].op = sd_imm;
			if (	ins[pc].op ==  set and arg0 == arg1 or
				ins[pc].op ==  or_ and arg0 == arg1 or 
				ins[pc].op == and_ and arg0 == arg1 or 
				ins[pc].op == add_imm and values[arg1] == 0 or 
				ins[pc].op == sub_imm and values[arg1] == 0 or 
				ins[pc].op == mul_imm and values[arg1] == 1 or 
				ins[pc].op == div_imm and values[arg1] == 1 or 
				ins[pc].op ==  or_imm and values[arg1] == 0 or
				ins[pc].op == eor_imm and values[arg1] == 0 or
				ins[pc].op ==  si_imm and values[arg1] == 0 or
				ins[pc].op ==  sd_imm and values[arg1] == 0) ins[pc].ct = 1;
		} 
		next_ins: if (ins[pc].gotos[0] < ins_count) 
			stack[stack_count++] = ins[pc].gotos[0]; skip_next:;
	}

	for (nat i = 0; i < ins_count; i++) if (not visited[i]) ins[i].ct = 4;

	print_ct_values(names, name_count, ctk, values);
	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);

	getchar();


	exit(1);



	*/












































	puts("starting ins sel..");
	struct instruction mi[4096] = {0};
	nat mi_count = 0;
	nat selected[4096] = {0};

	for (nat i = 0; i < ins_count; i++) {

		if (selected[i]) {
			printf("warning: [ins_index = %llu]: skipping instruction, it was already part of a pattern.\n", i);
			continue;
		}

		const nat op = ins[i].op;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1];

		print_instruction_index(ins, ins_count, names, name_count, i, "SELECTION ORIGIN");
		printf("selecting from i #%llu\n", i);
		print_instruction(ins[i], names, name_count); puts("");
		//getchar();

		/*if (op == set) {
			const nat b = locate_data_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count);
			printf("b = %lld\n", b);
			if (b == (nat) -1) goto next0;
		
			const nat c = locate_data_instruction(add, arg0, 0, 1, 0, b + 1, ins, ins_count, names, name_count);
			printf("c = %lld\n", c);
			if (c == (nat) -1) goto next0;
			
			const nat d = arg0;
			const nat n = ins[c].args[1];
			const nat m = ins[i].args[1];
			const nat k = ins[b].args[1];
			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("ADD_SR   "
				"d=%llu(%s), "
				"n=%llu(%s), "
				"m=%llu(%s) << "
				"k=%llu\n",
				d, names[d], 
				n, names[n], 
				m, names[m], 
				k
			);
			struct instruction new = {0};
			new.op = addsr_lsl;
			new.args[0] = d;
			new.args[1] = n;
			new.args[2] = m;
			new.args[3] = k;
			mi[mi_count++] = new;

			selected[i] = 1;
			selected[b] = 1;
			selected[c] = 1;
		}
		next0:;

		if (op == set_imm) {

			const nat d = arg0;
			const nat k = arg1;
			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("MOVZ   "
				"d=%llu(%s), "
				"k=%llu\n",
				d, names[d], 
				k
			);
			struct instruction new = {0};
			new.op = movz;
			new.args[0] = d;
			new.args[1] = k;
			mi[mi_count++] = new;

			selected[i] = 1;
		}*/

		if (op == set) { // msub

			const nat i1 = locate_data_instruction(mul, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count);
			printf("i1 = %lld\n", i1);
			if (i1 == (nat) -1) goto next2;

			const nat i2 = locate_data_instruction(set, 0, 0, 0, 0, i1 + 1, ins, ins_count, names, name_count); 
								// TODO: BUG:   s must be != to d.
			printf("i2 = %lld\n", i2);
			if (i2 == (nat) -1) goto next2;

			const nat i3 = locate_data_instruction(sub, ins[i2].args[0], arg0, 1, 1, i2 + 1, ins, ins_count, names, name_count);
			printf("i3 = %lld\n", i3);
			if (i3 == (nat) -1) goto next2;
			
			const nat d = ins[i2].args[0];
			const nat a = ins[i2].args[1];
			const nat m = arg1;
			const nat n = ins[i1].args[1];

			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("MSUB   "
				"d=%llu(%s), "
				"a=%llu(%s) - "
				"m=%llu(%s) * "
				"n=%llu(%s)\n",
				d, names[d], 
				a, names[a], 
				m, names[m], 
				n, names[n]
			);
			struct instruction new = {0};
			new.op = msub;
			new.args[0] = d;
			new.args[1] = n;
			new.args[2] = m;
			new.args[3] = a;
			mi[mi_count++] = new;

			selected[i] = 1;
			selected[i1] = 1;
			selected[i2] = 1;
			selected[i3] = 1;
			goto finish_mi_instruction;
		} 
		next2: 
		if (op == set) {

			const nat b = locate_data_instruction(mul, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count);
			printf("b = %lld\n", b);
			if (b == (nat) -1) goto next3;
		
			const nat c = locate_data_instruction(add, arg0, 0, 1, 0, b + 1, ins, ins_count, names, name_count);
			printf("c = %lld\n", c);
			if (c == (nat) -1) goto next3;
			
			const nat d = arg0;
			const nat m = arg1;
			const nat n = ins[b].args[1];
			const nat a = ins[c].args[1];
			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("MADD   "
				"d=%llu(%s), "
				"n=%llu(%s) * "
				"m=%llu(%s) + "
				"a=%llu(%s)\n",
				d, names[d], 
				n, names[n], 
				m, names[m], 
				a, names[a]
			);
			struct instruction new = {0};
			new.op = madd;
			new.args[0] = d;
			new.args[1] = n;
			new.args[2] = m;
			new.args[3] = a;
			mi[mi_count++] = new;

			selected[i] = 1;
			selected[b] = 1;
			selected[c] = 1;
			goto finish_mi_instruction;
		} 

		next3:

		if (op == zero) {
			const nat d = arg0;
			const nat k = 0;
			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("MOVZ   "
				"d=%llu(%s), "
				"k=%llu\n",
				d, names[d], 
				k
			);
			struct instruction new = {0};
			new.op = movz;
			new.args[0] = d;
			new.args[1] = k;
			mi[mi_count++] = new;

			selected[i] = 1;
			goto finish_mi_instruction;
		} 
		//next4:
		if (op == incr) {
			const nat d = arg0;
			const nat n = d;
			const nat k = 1;

			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("ADDI   "
				"d=%llu(%s), "
				"n=%llu(%s), "
				"k=%llu\n",
				d, names[d], 
				n, names[n], 
				k
			);
			struct instruction new = {0};
			new.op = addi;
			new.args[0] = d;
			new.args[1] = n;
			new.args[2] = k;
			mi[mi_count++] = new;

			selected[i] = 1;
			goto finish_mi_instruction;
		}


	finish_mi_instruction:;
		getchar();

	}

	print_machine_instructions(mi, mi_count, names, name_count);

	for (nat i = 0; i < ins_count; i++) {
		//const nat op = ins[i].op;
		if (not selected[i]) {
			puts("error: instruction was not able to be processed by instruction selection: internal error");
			puts("not selected instruction: ");
			print_instruction_index(
				ins, ins_count, 
				names, name_count, 
				i, "this instruction failed to be lowered during instruction selection."
			); abort();
		}
	}

	print_dictionary(names, locations, name_count);
	print_instructions(ins, ins_count, names, name_count);
	print_machine_instructions(mi, mi_count, names, name_count);
	puts("stopped after ins sel.");
	exit(0);
}

































/*{const nat ct = ins[i].ct;
		const bool unreachable = ct & ct_is_unreachable;
		const bool compiletime = ct & ct_is_compiletime;
		const bool generated_do = ct & ct_is_generated_do;
		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);

		if (unreachable or compiletime and (not is_branch or is_branch and not generated_do)) continue;}
		*/




		//const nat arg0 = ins[i].args[0];
		//const nat arg1 = ins[i].args[1];


/*		const nat ct = ins[i].ct;
		const bool unreachable = ct & ct_is_unreachable;
		const bool compiletime = ct & ct_is_compiletime;
		const bool generated_do = ct & ct_is_generated_do;
		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);

		if (unreachable or compiletime and (not is_branch or is_branch and not generated_do)) continue;}
*/










































/*








movz	setimm x k 
addsr	set d m siimm d k add d n 
lslv	set d m si d n
madd	set d m mul d n add d a

msub	set s m mul s n set d a sub d s
eori	set d n eorimm d k
eorsr	set d m siimm d k eor d n
eonsr	set d m siimm d k not d eor d n

addi	set d n addimm d k
andi	set d n andimm d k
orrsr	set d m siimm d k or d n
orri	set d n orimm d k

ornsr	set d m siimm d k not d or d n
subsr	set s m siimm s k set d n sub d s
subi	set d n subimm d k
udiv	set d n div d m







--------------------------- ARM64 INS SEL PATTERNS -----------------------------

movz	setimm x k 

lslv	set d m si d n
lsrv	set d m sd d n

udiv	set d n div d m

madd	set d m mul d n add d a
msub	set s m mul s n set d a sub d s



addsr	set d m siimm d k add d n 
subsr	set s m siimm s k set d n sub d s

andsr	set d m siimm d k and d n

orrsr	set d m siimm d k or d n
ornsr	set d m siimm d k not d or d n

eorsr	set d m siimm d k eor d n
eonsr	set d m siimm d k not d eor d n



addi	set d n addimm d k

subi	set d n subimm d k

andi	set d n andimm d k

orri	set d n orimm d k

eori	set d n eorimm d k



--------------------------- END OF ARM64 INS SEL PATTERNS -----------------------------









movz . setimm x k

addsrlsl . set d m siimm d k add d n 
addsrlsr . set d m sdimm d k add d n

eorsrlsl . set d m siimm d k eor d n 
eorsrlsr . set d m sdimm d k eor d n 
eonsrlsl . set d m siimm d k not d eor d n
eonsrlsr . set d m sdimm d k not d eor d n

orrsrlsl . set d m siimm d k or d n 
orrsrlsr . set d m sdimm d k or d n 
ornsrlsl . set d m siimm d k not d or d n
ornsrlsr . set d m sdimm d k not d or d n




lslv . set d m si d n

madd . set d m mul d n add d a 
msub . set s m mul s n set d a sub d s

addi . set d n addimm d k
andi . set d n andimm d k 
orri . set d n orimm d k
eori . set d n eorimm d k



*/



		//const nat ct = ins[pc].ct;

		//if (compiletime and generated_do) { printf("FOUND MACHINE DO @ %llu\n", pc); getchar(); break; }

		//const bool compiletime = ct & ct_is_compiletime;
		//const bool generated_do = ct & ct_is_generated_do;
		//if (not compiletime) 


























		//}
		//if (is_branch) pc = ins[pc].gotos[(ins[pc] >> 1) & 1];
		//else 




// things to layer on to the front end still: 
//   - including of multiple files
//   - comments

/*

things in the compiler to do:
	
	- ct-eval : recognize when a variable is compiletime known, automatically,
			by looking at the predeccessors, and keeping track of a variable/ctkness at each point in the program, and going back and looking at previous decisions, because of loops! push a new branch node to the branch stack, when we encounter a br, and check the decision that was made on all ctkness of all variables. store the values and ctk array on the branch stack. i think this is required. 


after that:


	- ins sel : add more patterns, for arm64
	- ins sel :    recognize control flow patterns:   implement  csel!!!!!!


	- RA : find the lifetime of variables   and the reads and writes to variables, in the mi listing!
	- code gen: generate the mi into machine code lol
		- work on the msp430 backend!

*/





/*

} else if (op == zero) { new.op = set_imm; new.args[1] = 0;
} else if (op == incr) { new.op = add_imm; new.args[1] = 1;
} else if (op == decr) { new.op = sub_imm; new.args[1] = 1;
} else if (op == not_) { new.op = eor_imm; new.args[1] = (nat) -1; }

*/



/*	
				compiletime / runtime identification pass:

-------------apca-------------apca-------------apca-------------apca-------------apca-------------apca-------------apca


	nat previous_pc = (nat) -1, stack_count = 1;
	nat* stack = calloc(ins_count, sizeof(nat));
	nat* visited = calloc(ins_count + 1, sizeof(nat));
	nat* execution_state_values = calloc(ins_count * name_count, sizeof(nat));
	nat* execution_state_ctk = calloc(ins_count * name_count, sizeof(nat));
	memset(execution_state_values, 0xA5, sizeof(nat) * ins_count * name_count); // debug

	while (stack_count) {

		print_stack(stack, stack_count);
		const nat pc = stack[--stack_count];
		nat* values = execution_state_values + name_count * pc;
		nat* ctk = execution_state_ctk + name_count * pc;
		if (not pc) goto process;

		nat pred_count = 0;
		nat* preds = compute_predecessors(ins, ins_count, pc, &pred_count);
		for (nat n = 0; n < name_count; n++) {
			for (nat i = 0; i < pred_count; i++) {
				if (not visited[preds[i]]) continue;
				if (not execution_state_ctk[name_count * preds[i] + n]) goto one_rtk;
			}
			nat spot = 0;
			for (; spot < pred_count; spot++) {
				if (not visited[preds[spot]]) continue;
				if (preds[spot] == previous_pc) goto pred_found;
			}
			puts("fatal error: could not find previous_pc in predecessor list.");
			printf("previous_pc = %llu  :  { ", previous_pc);
			for (nat i = 0; i < pred_count; i++) {
				printf("%llu ", preds[i]);
			} 
			puts(" }");
			abort();
		pred_found:;
			printf("selecting pred[%llu] == previous_pc, which was %llu\n", spot, previous_pc);
			if (not execution_state_ctk[name_count * previous_pc + n]) abort();
			ctk[n] = 1;
			values[n] = execution_state_values[name_count * previous_pc + n];			
			goto next_name;
		one_rtk:;
			ctk[n] = 0;
			values[n] = 0x999999999;
			next_name:;
		}
		process:;
		visited[pc]++;

		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];

		if (op == lt or op == eq) {
			if (not ctk[arg0] or not ctk[arg1]) {

				if (ins[pc].gotos[0] < ins_count and 
					visited[ins[pc].gotos[0]] < 2) 
					stack[stack_count++] = ins[pc].gotos[0];
				if (ins[pc].gotos[1] < ins_count and 
					visited[ins[pc].gotos[1]] < 2) 
					stack[stack_count++] = ins[pc].gotos[1];
			} else {
				nat condition = 0;
				if (op == eq and values[arg0] == values[arg1]) condition = 1;
				if (op == lt and values[arg0] <  values[arg1]) condition = 1;
				if (ins[pc].gotos[condition] < ins_count) stack[stack_count++] = ins[pc].gotos[condition];
			}
			goto next_instruction;
		}
		
		if (op == 0) abort();
		else if (op == zero) { ctk[arg0] = 1; values[arg0] = 0; }
		else if (op == incr) { if (ctk[arg0]) values[arg0]++; } 
		else if (op == add) { 
			if (ctk[arg0] and ctk[arg1]) values[arg0] += values[arg1]; 
			else if (not ctk[arg0] and not ctk[arg1]) {}
			else if (not ctk[arg0]) {  set add_imm op code }
			else {
				puts("i think this is the point where we make the dest CT now, instead of RT.");
				abort();
			}
		} 
		else if (op == sc) {

			if (not ctk[arg0]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg0];
			const nat output_count = get_call_output_count(n);
			for (nat i = 0; i < output_count; i++) {
				const nat this = ins[pc].args[1 + i];
				ctk[this] = 0;
				values[this] = 0x999999999999333;
			}
			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instructions_ct_values_index(
					ins, ins_count, 
					names, name_count, locations, 
					execution_state_ctk, execution_state_values,
					pc, "CFG termination point here"
				);
				getchar();
				ins[pc].gotos[0] = (nat) -1;
				ins[pc].gotos[1] = (nat) -1;
				goto next_instruction;

			} else if (n == system_write) {			
				printf("warning: system_write syscall encountered\n");
				print_instructions_ct_values_index(
					ins, ins_count, 
					names, name_count, locations, 
					execution_state_ctk, execution_state_values,
					pc, "system_write"
				);
				getchar();

			} else {
				printf("info: found %s sc!\n", systemcall_spelling[n]);
				printf("ERROR: unknown syscall encountered\n");
				print_instructions_ct_values_index(
					ins, ins_count, 
					names, name_count, locations, 
					execution_state_ctk, execution_state_values,
					pc, "??????"
				);
				getchar();
			}
		} else {
			puts("FATAL_ERROR: unknown operation encountered, aborting.."); 
			abort();
		}
		
		if (ins[pc].gotos[0] < ins_count) 
			stack[stack_count++] = ins[pc].gotos[0];

	next_instruction:;
		previous_pc = pc;
		print_instructions_ct_values_index(
			ins, ins_count, 
			names, name_count, locations, 
			execution_state_ctk, execution_state_values,
			pc, "PC"
		);
		getchar();
	}


	print_instructions_ct_values_index(
		ins, ins_count, 
		names, name_count, locations, 
		execution_state_ctk, execution_state_values, 
		(nat) -1, ""
	);


-------------apca-------------apca-------------apca-------------apca-------------apca-------------apca-------------apca

	*/


































		//print_ct_values(names, name_count, ctk, values);
		//print_instruction_index(ins, ins_count, names, name_count, pc, "PC");


	/*
		printf("executing pc #%llu    :    ", pc);
		print_instruction(ins[pc], names, name_count); puts("");






	nat* stack = calloc(ins_count, sizeof(nat));
	nat stack_count = 0;
	stack[stack_count++] = 0; 

	nat* visited = calloc(ins_count + 1, sizeof(nat));




	while (stack_count) {
		print_stack(stack, stack_count);
		const nat pc = stack[--stack_count];
		print_ct_values(names, name_count, ctk, values);
		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");

		getchar();

		visited[pc] = 1;
		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];

		if (op == lt or op == eq) {

			if (ins[pc].gotos[0] == ins[pc].gotos[1]) {
				ins[pc].ct = (1 << 1) | 1;
				ins[pc].op = eq;
				ins[pc].args[0] = 0;
				ins[pc].args[1] = 0;
				ins[pc].gotos[0] = (nat) -1;
				if (ins[pc].gotos[1] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[1];
				continue;

			} else if (ins[pc].args[0] == ins[pc].args[1]) {

				const nat condition = op == eq;
				ins[pc].ct = (condition << 1) | 1;
				ins[pc].op = eq;
				ins[pc].args[0] = 0;
				ins[pc].args[1] = 0;
				ins[pc].gotos[1] = ins[pc].gotos[condition];
				ins[pc].gotos[0] = (nat) -1;
				if (ins[pc].gotos[1] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[1];
				continue;


			} else if (ctk[arg0] and ctk[arg1]) {
				nat condition = 0;
				if (op == eq and values[arg0] == values[arg1]) condition = 1;
				if (op == lt and values[arg0] <  values[arg1]) condition = 1;
				ins[pc].ct = (condition << 1) | 1;
				if (ins[pc].gotos[condition] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[condition];
				continue;
			}

			if (ctk[arg0]) {
				nat t = ins[pc].args[0]; 
				ins[pc].args[0] = ins[pc].args[1]; 
				ins[pc].args[1] = t;
				if (op == lt) ins[pc].op = gt_imm;
				else ins[pc].op = eq_imm;
				ins[pc].args[1] = values[ins[pc].args[1]];

			} else if (ctk[arg1]) {
				if (op == lt) ins[pc].op = lt_imm;
				else ins[pc].op = eq_imm;
				ins[pc].args[1] = values[ins[pc].args[1]];
			}
			if (ins[pc].gotos[0] < ins_count and 
				not visited[ins[pc].gotos[0]]) 
				stack[stack_count++] = ins[pc].gotos[0];
			if (ins[pc].gotos[1] < ins_count and 
				not visited[ins[pc].gotos[1]]) 
				stack[stack_count++] = ins[pc].gotos[1];

			continue;

		} else if (op == sc) {
			if (not ctk[arg0]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg0];
			const nat output_count = get_call_output_count(n);
			for (nat i = 0; i < output_count; i++) {
				if (ctk[ins[pc].args[1 + i]]) { puts("system call ct rt out"); abort(); }
			}
			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, "CFG termination point here"
				);
				getchar();
				ins[pc].gotos[0] = (nat) -1;
				ins[pc].gotos[1] = (nat) -1;
				goto skip_next;
			} else {
				printf("info: found %s sc!\n", systemcall_spelling[n]);
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, systemcall_spelling[n]
				);
			}

			ins[pc].args[0] = values[arg0];

		} else if (op == zero) { 
			if (ctk[arg0]) { 
				values[arg0] = 0; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = set_imm; 
				ins[pc].args[1] = 0; 
			}

		} else if (op == incr) { 
			if (ctk[arg0]) { 
				values[arg0]++; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = add_imm; 
				ins[pc].args[1] = 1; 
			}
		} else if (op == decr) { 
			if (ctk[arg0]) { 
				values[arg0]--; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = sub_imm; 
				ins[pc].args[1] = 1; 
			}
		} else if (op == not_) { 
			if (ctk[arg0]) { 
				values[arg0] = ~values[arg0]; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = eor_imm; 
				ins[pc].args[1] = (nat) -1; 
			}
		} else {
			if (ctk[arg0] and ctk[arg1]) {
				ins[pc].ct = 1; 
				     if (op == set) values[arg0] = values[arg1];
				else if (op == add) values[arg0] += values[arg1];
				else if (op == sub) values[arg0] -= values[arg1];
				else if (op == mul) values[arg0] *= values[arg1];
				else if (op == div_)values[arg0] /= values[arg1];
				else if (op == and_)values[arg0] &= values[arg1];
				else if (op == or_) values[arg0] |= values[arg1];
				else if (op == eor) values[arg0] ^= values[arg1];
				else if (op == si)  values[arg0] <<= values[arg1];
				else if (op == sd)  values[arg0] >>= values[arg1];
				else {
					puts("internal error: op execution not specified");
					printf("op = %llu, op = %s\n", op, ins_spelling[op]);
					abort();
				}
				goto next_ins;
			}

			//printf("arg0 = 0x%016llx\n", arg0);
			//printf("arg1 = 0x%016llx\n", arg1);
			//fflush(stdout);

			if (op >= isa_count or not ctk[arg0] and not ctk[arg1]) goto next_ins;
			if (    ctk[arg0] and not ctk[arg1]) {
				puts("error: ct destination must have ct source."); 
				abort();
			}
			ins[pc].args[1] = values[arg1];
			if (op == set) ins[pc].op = set_imm;
			if (op == add) ins[pc].op = add_imm;
			if (op == sub) ins[pc].op = sub_imm;
			if (op == mul) ins[pc].op = mul_imm;
			if (op == div_)ins[pc].op = div_imm;
			if (op == and_)ins[pc].op = and_imm;
			if (op == or_) ins[pc].op = or_imm;
			if (op == eor) ins[pc].op = eor_imm;
			if (op == si)  ins[pc].op = si_imm;
			if (op == sd)  ins[pc].op = sd_imm;
			if (	ins[pc].op ==  set and arg0 == arg1 or
				ins[pc].op ==  or_ and arg0 == arg1 or 
				ins[pc].op == and_ and arg0 == arg1 or 
				ins[pc].op == add_imm and values[arg1] == 0 or 
				ins[pc].op == sub_imm and values[arg1] == 0 or 
				ins[pc].op == mul_imm and values[arg1] == 1 or 
				ins[pc].op == div_imm and values[arg1] == 1 or 
				ins[pc].op ==  or_imm and values[arg1] == 0 or
				ins[pc].op == eor_imm and values[arg1] == 0 or
				ins[pc].op ==  si_imm and values[arg1] == 0 or
				ins[pc].op ==  sd_imm and values[arg1] == 0) ins[pc].ct = 1;
		} 
		next_ins: if (ins[pc].gotos[0] < ins_count) 
			stack[stack_count++] = ins[pc].gotos[0]; skip_next:;
	}

	for (nat i = 0; i < ins_count; i++) if (not visited[i]) ins[i].ct = 4;

	print_ct_values(names, name_count, ctk, values);
	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);

	getchar();


	exit(1);



	*/




































/*
	nat pred_count[4096] = {0};

	for (nat n = 0; n < name_count; n++) {
		if (locations[n] != (nat) -1) {			
			printf("FOUND LABEL %s: WITH LOCATION: %llu\n", 
				names[n], locations[n]
			);
			const nat this = locations[n];
			nat found_count = 0;
			for (nat i = 0; i < ins_count; i++) {
				if (ins[i].gotos[0] == this and not (ins[i].ct & ct_is_unreachable)) { 
					found_count++;
				//print_instruction_index(
				//ins, ins_count, 
				//names, name_count, 
				//i, "occurence"
			//); getchar(); 
				}
				if (ins[i].gotos[1] == this and not (ins[i].ct & ct_is_unreachable)) { 
					found_count++;
				//print_instruction_index(
				//ins, ins_count, 
				//names, name_count, 
				//i, "occurence"
			//); getchar(); 
				}
			}
			printf(" ---> this label had %llu goto occurences "
				"of instructions which went to this location.\n",
				found_count
			);
			pred_count[n] = found_count;
		}
	}

	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op != eq or ins[i].args[0] != ins[i].args[1]) continue;
		puts("FOUND A DO INSTRUCTION!!!!");
		if (pred_count[ins[i].args[2]] >= 2) {
			puts("warning: this do statement will be generated in actual machine code");
			print_instruction_index(
				ins, ins_count, 
				names, name_count, 
				i, "GENERATED IN MACHINE CODE."
			);
			ins[i].ct |= 8;
		} else {
			printf("warning: this do statement will be optimized away, "
				"as it only has %llu pred.\n", pred_count[ins[i].args[2]]
			);
			print_instruction_index(
				ins, ins_count, 
				names, name_count, 
				i, "IGNORED, OPTIMIZED AWAY."
			);
		}
		getchar();
	}

	print_ct_values(names, name_count, ctk, values);
	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);

	puts("starting ins sel..");
	struct instruction mi[4096] = {0};
	nat mi_count = 0;
	nat selected[4096] = {0};


	for (nat i = 0; i < ins_count; i++) {

		if (selected[i]) {
			printf("warning: [ins_index = %llu]: skipping instruction, it was already part of a pattern.\n", i);
			continue;
		}


		const nat op = ins[i].op;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1];

		{const nat ct = ins[i].ct;
		const bool unreachable = ct & ct_is_unreachable;
		const bool compiletime = ct & ct_is_compiletime;
		const bool generated_do = ct & ct_is_generated_do;
		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);

		if (unreachable or compiletime and (not is_branch or is_branch and not generated_do)) continue;}

		print_instruction_index(ins, ins_count, names, name_count, i, "SELECTION ORIGIN");
		printf("selecting from i #%llu\n", i);
		print_instruction(ins[i], names, name_count); puts("");
		getchar();


		if (op == set) {
			const nat b = locate_data_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count);
			printf("b = %lld\n", b);
			if (b == (nat) -1) goto next0;
		
			const nat c = locate_data_instruction(add, arg0, 0, 1, 0, b + 1, ins, ins_count, names, name_count);
			printf("c = %lld\n", c);
			if (c == (nat) -1) goto next0;
			
			const nat d = arg0;
			const nat n = ins[c].args[1];
			const nat m = ins[i].args[1];
			const nat k = ins[b].args[1];
			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("ADD_SR   "
				"d=%llu(%s), "
				"n=%llu(%s), "
				"m=%llu(%s) << "
				"k=%llu\n",
				d, names[d], 
				n, names[n], 
				m, names[m], 
				k
			);
			struct instruction new = {0};
			new.op = addsr_lsl;
			new.args[0] = d;
			new.args[1] = n;
			new.args[2] = m;
			new.args[3] = k;
			mi[mi_count++] = new;

			selected[i] = 1;
			selected[b] = 1;
			selected[c] = 1;
		}
		next0:;


		if (op == set_imm) {

			const nat d = arg0;
			const nat k = arg1;
			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("MOVZ   "
				"d=%llu(%s), "
				"k=%llu\n",
				d, names[d], 
				k
			);
			struct instruction new = {0};
			new.op = movz;
			new.args[0] = d;
			new.args[1] = k;
			mi[mi_count++] = new;

			selected[i] = 1;
		}

	
	}


	for (nat i = 0; i < ins_count; i++) {

		{const nat op = ins[i].op;
		//const nat arg0 = ins[i].args[0];
		//const nat arg1 = ins[i].args[1];
		const nat ct = ins[i].ct;
		const bool unreachable = ct & ct_is_unreachable;
		const bool compiletime = ct & ct_is_compiletime;
		const bool generated_do = ct & ct_is_generated_do;
		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);

		if (unreachable or compiletime and (not is_branch or is_branch and not generated_do)) continue;}

		if (not selected[i]) {
			puts("error: instruction was not able to be processed by instruction selection: internal error");
			puts("not selected instruction: ");
			print_instruction_index(
				ins, ins_count, 
				names, name_count, 
				i, "this instruction failed to be lowered during instruction selection."
			); abort();
		}
	}

	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);
	print_machine_instructions(mi, mi_count);
	puts("stopped after ins sel.");
	//puts(text);
	exit(0);



	*/












































					//else if (unreachable) {
						/*printf("%s:%llu:%llu: warning: unreachable instruction\n", 
							filename, word_start, index
						);
						print_index(text, text_length, word_start, index);*/
					//}




/*
		const nat expecting_op = si;
		const nat expecting_arg0 = 

		const nat r = locate_data_instruction(
			expecting_op, expecting_arg0, expecting_arg1, 
			i, ins, ins_count, names, name_count
		);
		printf("r = %llu\n", r);


*/




/*	const nat start_from = 0;
	const nat expecting_op = si;
	const nat expecting_arg0 = ins[0].args[0];
	const nat expecting_arg1 = 0;
	
	const nat r = locate_data_instruction(
		expecting_op, expecting_arg0, expecting_arg1, 1, 0,
		start_from + 1, ins, ins_count, names, name_count
	);
	printf("r = %lld\n", r);

	exit(1);
*/
















/*


			} else if (state == ct) {
				state = 0;
				ctk[variable] = 1;

			} else if (state == rt) {
				if (arg_count < 2) goto next_word;
				state = 0;
				ctk[args[0]] = 0;
				bit_count[args[0]] = variable;


*/














/*		if (compiletime and not is_branch) {
			pc = ins[i].gotos[0]; continue;

		} else if (compiletime and is_branch and not generated_do) {
			pc = ins[i].gotos[(ins[i].ct >> 1) & 1]; continue;
		}





		if (unreachable) {
			puts("hit an unrechable instruction!?!");
			getchar();
		}
	







	for (nat i = 0; i < ins_count; i++) {
		if ((ins[i].ct & ct_is_unreachable) or (ins[i].ct & ct_is_compiletime)) continue;
		for (nat a = 0; a < (ins[i].op == sc ? 7 : 2); a++) {
			if (ins[i].op >= isa_count and a == 1) continue;
			const nat n = ins[i].args[a];
			if (locations[n] == (nat) -1 and (ins[definition[n]].ct & ct_is_unreachable)) {
				printf("warning: in argument %llu, variable \"%s\" used with unreachable initialization\n", a, names[n]);
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					i, "initialization will never be executed"
				);
				getchar();			
			}
		}
	}

		^ this pass is not quite sound, because of ct-br conditional initialization. 
*/





















/*


	struct instruction mi[4096] = {0};
	nat mi_count = 0;
	memset(visited, 0, sizeof(nat) * (ins_count + 1));
	stack[stack_count++] = 0;

	while (stack_count) {

		print_stack(stack, stack_count);
		nat pc = stack[--stack_count];
		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");
		getchar();

		visited[pc] = 1;

		if (ins[pc].ct) goto done_with_instruction;

	//  set d m  si_imm d k  add d n
		{const nat op0 = ins[pc].op;
		const nat dest0 = ins[pc].args[0];
		const nat source0 = ins[pc].args[1];
		const nat gt0 = ins[pc].gotos[0];
		if (op0 != set) goto next0;

		const nat op1 = ins[gt0].op;
		const nat dest1 = ins[gt0].args[0];
		const nat source1 = ins[gt0].args[1];
		const nat gt1 = ins[gt0].gotos[0];
		if (op1 != si_imm or dest1 != dest0) goto next0;

		const nat op2 = ins[gt1].op;
		const nat dest2 = ins[gt1].args[0];
		const nat source2 = ins[gt1].args[1];
		if (op2 != add or dest2 != dest0) goto next0;


		// todo: 
		//ERROR ERROR 
		puts("we were in the middle of doing ins sel...");
		abort();


		const nat d = dest0;
		const nat n = source2;
		const nat m = source0;
		const nat k = source1;					
		printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
		printf("ADD_SR   "
			"d=%llu(%s), "
			"n=%llu(%s), "
			"m=%llu(%s) << "
			"k=%llu\n",
			d, names[d], 
			n, names[n], 
			m, names[m], 
			k
		);
		struct instruction new = {0};
		new.op = addsr;
		new.args[0] = d;
		new.args[1] = n;
		new.args[2] = m;
		new.args[3] = k;
		mi[mi_count++] = new;
		pc = gt1; } next0:;





	// set d m  add d n
		{const nat op0 = ins[pc].op;
		const nat dest0 = ins[pc].args[0];
		const nat source0 = ins[pc].args[1];
		const nat gt0 = ins[pc].gotos[0];
		if (op0 != set) goto next1;

		const nat op1 = ins[gt0].op;
		const nat dest1 = ins[gt0].args[0];
		const nat source1 = ins[gt0].args[1];
		if (op1 != add or dest1 != dest0) goto next1;

		const nat d = dest0;
		const nat n = source1;
		const nat m = source0;
		const nat k = 0;
		printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
		printf("ADD_SR   "
			"d=%llu(%s), "
			"n=%llu(%s), "
			"m=%llu(%s) << "
			"k=%llu\n",
			d, names[d], 
			n, names[n], 
			m, names[m], 
			k
		);
		struct instruction new = {0};
		new.op = addsr;
		new.args[0] = d;
		new.args[1] = n;
		new.args[2] = m;
		new.args[3] = k;
		mi[mi_count++] = new;
		pc = gt0; } next1:;

	// set_imm d k
		{const nat op0 = ins[pc].op;
		const nat dest0 = ins[pc].args[0];
		const nat source0 = ins[pc].args[1];
		if (op0 != set_imm) goto next2;

		const nat d = dest0;
		const nat k = source0;
		printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
		printf("MOVZ   "
			"d=%llu(%s), "
			"k=%llu\n",
			d, names[d], 
			k
		);
		struct instruction new = {0};
		new.op = movz;
		new.args[0] = d;
		new.args[1] = k;
		mi[mi_count++] = new; } next2:;

		done_with_instruction:;
		const nat op = ins[pc].op;
		const nat gt0 = ins[pc].gotos[0];
		const nat gt1 = ins[pc].gotos[1];
		if (op == lt or op == eq or op == lt_imm or op == gt_imm or op == eq_imm) 
		if (gt1 < ins_count and not visited[gt1]) stack[stack_count++] = gt1;
		if (gt0 < ins_count and not visited[gt0]) stack[stack_count++] = gt0;
	}

	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);
	print_machine_instructions(mi, mi_count);
	puts("stopped after ins sel.");

	exit(1);




	nat* list = calloc(8 * ins_count, sizeof(nat));
	nat list_count = 0;

	// .. loop over the mi instruction list, generating reads and writes for each instruction. 

	printf("found list: (%llu count)\n", list_count);
	for (nat i = 0; i < list_count; i++) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);
	}
	puts("done");

	bool* alive = calloc(name_count, sizeof(bool));

	for (nat i = list_count; i--;) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);

		const bool is_write = !!(list[i] >> 63);

		if (is_write) {			
			alive[variable_index] = 0;
		} else {
			alive[variable_index] = 1;			
		}
		
		printf("alive = { ");
		for (nat n = 0; n < name_count; n++) {
			if (alive[n]) printf("%s  ", names[n]);
		} 
		printf(" }\n");
		//getchar();
	}

	puts("compiled.");

	puts(text);
	exit(0);
}







*/





















	// serialize the cfg into instructions which use ip++?.. so that we can discover which goto's must reside. because then, obviously, if thats the case, we need to not allow instruction selection around that instruction. we need to treat it like an end, basically, if there are instructions after it. we are still going to represent this in the cfg as just .goto of some non ip++ thing, but we will actually note down the fact that this non-ip++ goto cannot be gotten rid of, because there are other people which go to the same place. so yeah. i think that should work. lets try to implement this lol. 


















/*
		// some additional checks: 
		// we need to make sure that the only instruction which goes to any of these later instructions after the first one,   are   only part of the pattern too. 
		// ie, we shouldnt let any   "at" statements happen in the middle here lol. 
		//


		alsooo i think theres  like a completely different way that we need to be doing our instruction selection.. 

			we need to take into account the actual data flow-   and just require control flow to be correct as well,  (ie, a straight line of executionnnn..)


			ie, we need some function to like, find the next instruction which has the right control flow, and alsoooo doesnt have data hazards in the way. thats the key. we'll follow the control flow, if see a branch, we abort (as thats lets say always a hazard!)

				and then if we find a hazard instruction (colliding data usage), then we know that we can't do this pattern. so yeah. we need to be doing that. 



			alsoooo... theres a bigger problem...

							i think uhh



					i think we need to schedule   the ir instructions, 


								(ie, form a linearizing sequence for thsi control flow graph. ie, doing like.. basic block ordering...)



							beforeeeee doing instruction selection. 





					because like, some of these... uhhh   "implicit do"    statements will actually have to end up being   actuallll machine instructions. because of course, the cpu that we target itself has those instructions, because it uses implicit ip++'s. so yeah. we need to be representing that, and literallyyyyy scheduling these instructions, in some ordering. 


						like, its just required. we just need that. 



					so yeah. i think i am going to look into how to like.. order basic blocks the best lol. 


				basically, the goal is to have blocks bleed into each other as much as posssible. thats the goal. 

				but like sometimes, you can't do that though. 


			hm
				so yeah. idk. we'll see how things go lol. 



*/






















	// here is where optimization is done!!!!

	// including: simplifying the ct execution, as well as the rt execution. both. 
	// eliminating memory variables, extraneous register variables, etc. 
	// simplifying control flow. you know that kind of stuff.








































































































































	/*

		addsr:  d  = n + (m << k);

			set d m
			si_imm d #k
			add d n

	//nat state = 0, dest = 0, source0 = 0, source1 = 0, immediate = 0;


	*/








/*





		if (state == 0) {
		retry:
			state = 0; 

			dest = 0; source0 = 0; source1 = 0; immediate = 0;

			if (op == set) { state = 1; dest = arg0; source1 = arg1; }
			else {}

		} else if (state == 1) {
			     if (op == si_imm and arg0 == dest) { state = 2; immediate = arg1; }
			else if (op == add and arg0 == dest) { immediate = 0; goto generate_addsr; }
			else goto retry;

		} else if (state == 2) {

			if (op == add and arg0 == dest) { 
			generate_addsr:
				source0 = arg1;
	
				printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
				printf("ADD_SR   dest=%llu(%s), source1=%llu(%s), source2=%llu(%s) << immediate=%llu\n",
					dest, names[dest], 
					source0, names[source0], 
					source1, names[source1], 
					immediate
				);
				struct instruction new = {0};
				new.op = addsr;
				new.args[0] = source0;
				new.args[1] = source1;
				new.args[2] = immediate;
				mi[mi_count++] = new;

				state = 0; 
			}
			else goto retry;

		} else if (state == 3) {

			goto retry;
		}



*/









/*


1202501061.175200
TODO:
		CURRENT STATE OF INS SEL:


	--->   redo where we are doing the pattern recognition to be in a seperate pass.

		instead of doing it in ct-eval stage, 



		1. generate a new list of ONLYYY RT instructions, 

						 (some of which the opcodes will 
						change to be elements in the  
							 "enum immediate_forms_instructions"!!!)
			{
	
				note, simply move pass (i++)   rt instructions, one generated. don't follow their execution.
					you only do this for compiletime branches. treat rt branches like  single nop instructions. 
			}
		FINE NOTE:

			if you encounter, a "do",  follow it, as its CT known. however, 
			just don't generate any RT instruction you've already visited before.
			this way, we will ignore unreachable rt code, 
			as well as avoid overtraversing the rt cfg.   NICEEEE YAYYYY




		2. then loop over this list, and doing pattern recognition on it. here, i don't think we should take into account the control flow of the RT instructions at all, so far.    
			this should generate a list of machine instructions,     the "mi" array above.  this ds uses the existing    "struct instruction"


		3. print out the generated machine instructions, and rt ins listing.

			then, you should start the process of looking at the reads and writes over those machine instructions. 

			this is when we start the process of register allocation. only here.  once the mi's have been generated in this mi[] list.
			
		
			3.1. we generate the list of reads and writes   based on the instruction semantics, 
					(note: we are doing it based on the mi instructions, in case a variable gets reduced away during ins sel.  very important. 


			3.2. then we go backwards through the reads and writes, generating the live-in lists, 
				keeping track of which variables are found in the same list, 
				and thus constructing the RIG from this information


			3.3.   we then use the RIG to do actual graph coloring based register allocation    ON THE     MI's. 


		4. generate machine code. we have everything we need now lol. op codes, and register numbers.    
			this step should be easy, as its already written lol.
			 
		5. done!!!! yayyyy


// TODO: recognize these three patterns:

ins sel   for      csinc     (conditional select increment)

RT COMPARISON:

ne X Y false
set d n
do done
at false
set d m incr d
at done


USING CONSTANTS IN COMPARISON:

ne X 3 false
set d n
do done
at false
set d m incr d
at done


NEGATING CONDITON:

eq X 3 false
set d m incr d
do done
at false
set d n
at done


	struct instruction mi[4096] = {0};  // arg[0] is in "enum arm64_ins_set". args are in order of assembly format.
	nat mi_count = 0;






ins sel patterns:	
	
	addsr {                 d = n + (m << k)

		set d m
		si d k
		add d n
		
		where k is ct, d, n and m are rt.
			k <= 63		
	}

	addsr (k = 0) {

		set d m
		add d n
		
		where d, n and m are rt. (k = 0)
	}










bad code:



		if (	top >= head and 
			top + 2 < ins_count and 
			ins[top + 0].args[0] == set and 
			ins[top + 1].args[0] == si and
			ins[top + 2].args[0] == add and
			
			ins[top + 0].args[1] == ins[top + 1].args[1] and
			ins[top + 1].args[1] == ins[top + 2].args[1] and
			ctk[ins[top + 1].args[2]]
		) {
			mi[mi_count++] = top;
			mi[mi_count++] = addsr;
			head += 3;
		}


		if (	top + 1 < ins_count and 
			ins[top + 0].args[0] == set and 
			ins[top + 1].args[0] == add and
			ins[top + 0].args[1] == ins[top + 1].args[1]
		) {
			mi[mi_count++] = top;
			mi[mi_count++] = addsr_k0;
			head += 2;
		}









use this code later:



		if (visited[top] == 1) {

			puts("found this instruction for the first time!!! : ");
			debug_instruction(ins[top], names);
			puts("");
			

			if (state == 0) {
			retry:
				state = 0;
				dest = 0; source1 = 0; source2 = 0; immediate = 0;

				// set d m
				if (op == set and not ctk[arg1] and not ctk[arg2]) { state = 1; dest = arg1; source2 = arg2; } 

				else {}

			} else if (state == 1) {

				// si d k
				      if (op == si and arg1 == dest and not ctk[arg1] and ctk[arg2]) { state = 2; immediate = values[arg2]; }
				else if (op == add and arg1 == dest and not ctk[arg1] and not ctk[arg2]) { immediate = 0; goto generate_addsr; }
				else goto retry;

			} else if (state == 2) {

				// add d n
				if (op == add and arg1 == dest and not ctk[arg1] and not ctk[arg2]) { 
				generate_addsr:
					source1 = arg2;

					printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
					printf("ADD_SR   dest=%llu(%s), source1=%llu(%s), source2=%llu(%s) << immediate=%llu\n",
							dest, names[dest], 
							source1, names[source1], 
							source2, names[source2], 
							immediate
					);				
					state = 0; 
				} 

				else goto retry;

			} else if (state == 3) {

				goto retry;
			}
			
		}
















			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;

	//nat top = 0;
	//struct instruction rt_ins[4096] = {0};
	//nat rt_count = 0;
	//struct instruction mi[4096] = {0}; 
	//nat mi_count = 0;

        // state =  0, dest = 0, source1 = 0, source2 = 0, immediate = 0;
	
	// top < ins_count

			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;


			//if (not ctk[arg2]) list[list_count++] = arg2;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;


			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg2]) list[list_count++] = arg2;

			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;


			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg2]) list[list_count++] = arg2;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;

			//rt_ins[rt_count++] = ins[top];
		//top++;		

	 

















*/




































/*


		if (is_branch(op)) {
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if ( true_side < ins_count and visited[true_side] < 1)  stack[stack_count++] = true_side;
			if (false_side < ins_count and visited[false_side] < 1) stack[stack_count++] = false_side;
		} else {
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if ( true_side < ins_count) stack[stack_count++] = true_side;
			if (false_side < ins_count) stack[stack_count++] = false_side;
		} 





	printf("found modified  ins instruction sequence {\n");
	for (nat i = 0; i < ins_count; i++) {
		const char* op_name = rt_ins[i].args[0] < isa_count ? 
			ins_spelling[rt_ins[i].args[0]] : 
			ins_imm_spelling[rt_ins[i].args[0] - isa_count];		
		printf("\trt[%llu] = { %llu(%s) %llu(%s) %llu %llu %llu %llu %llu %llu } \n",
			i,  
			rt_ins[i].args[0], op_name,
			rt_ins[i].args[1], names[rt_ins[i].args[1]],
			rt_ins[i].args[2],
			rt_ins[i].args[3],
			rt_ins[i].args[4],
			rt_ins[i].args[5],
			rt_ins[i].args[6],
			rt_ins[i].args[7]			
		);
	}





	const nat is_imm = this.args[0] >= isa_count;
	if (is_imm) printf(" %s ", ins_imm_spelling[this.args[0] - isa_count]);
	else printf(" %s ", ins_spelling[this.args[0]]);
	for (nat a = 1; a < this.count; a++) {
		if (a == 2 and is_imm) printf(" IMM=%llu ", this.args[a]);
		else printf(" %s ", names[this.args[a]]);
	}



	
			if (ctk[arg1] and ctk[arg2]) {
				if (condition) {
					ins[top].count = 2;
					ins[top].args[0] = do_;
					ins[top].args[1] = ins[top].args[3];
				} else {
					ins[top].count = 0;
				}
			}

*/

















































































/*


struct stack_entry {
	nat side;
	nat index;
};



	uint8_t* visited = calloc(ins_count + 1, 1);
	nat stack_count = 0;
	//stack[stack_count++] = 0; 
	struct instruction rt_ins[4096] = {0};
	nat rt_count = 0;


	struct stack_entry* stack = calloc(2 * ins_count, sizeof(struct stack_entry));
	nat pc = 0;
	while (stack_count or pc < ins_count) { 

		if (pc >= ins_count) {
			if (stack[stack_count - 1].side == 0 and not visited[ins[pc].gotos[1]]) {
				puts("PC AT END: trying other side of the TOS branch!");
				getchar();
				stack[stack_count - 1].side = 1;
				pc = ins[stack[stack_count - 1].index].gotos[1];

			} else {
				puts("PC AT END: popping off top element off stack!");
				getchar();
				stack_count--;
				if (not stack_count) {
					puts("PC AT END: unknown code state????");
					puts("breakinggggggg");
					break;
				} else {
					puts("PC AT END: setting pc to be a new value...");
					getchar();
					pc = ins[stack[stack_count].index].gotos[0];
				}
			}			
			
		}

		printf("stack: %llu { \n", stack_count);
		for (nat i = 0; i < stack_count; i++) {
			printf("stack[%llu] = { %llu %llu }\n", 
				i, stack[i].side, stack[i].index
			);
		}
		puts("}");

		printf("ct values: {\n");
		for (nat i = 0; i < name_count; i++) {
			if (not ctk[i]) continue;
			printf("\tvalues[%s] = %llu\n", names[i], values[i]);
		}
		puts("}");

		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");
		print_instructions(rt_ins, rt_count, names, name_count);
		getchar();

		visited[pc] = 1;

		struct instruction new = ins[pc];
		const nat 
			op = new.op,
			arg0 = new.args[0], 
			arg1 = new.args[1];
			//gt0 = new.gotos[0],
			//gt1 = new.gotos[1];

		if (op == lt or op == eq) {

			if (ctk[arg0] and ctk[arg1]) {
				bool condition = 0;
				if (op == eq and values[arg0] == values[arg1]) condition = 1;
				if (op == lt and values[arg0] <  values[arg1]) condition = 1;
				if (not condition) {
					pc = ins[pc].gotos[0];
				} else {
					pc = ins[pc].gotos[1];
				}

			} else if (not stack_count or stack[stack_count - 1].index != pc) {

				stack[stack_count++] =
				(struct stack_entry) {
					.side = 0,
					.index = pc
				};			
				if (not visited[ins[pc].gotos[0]]) pc = ins[pc].gotos[0];
				else stack[stack_count - 1].side++;

				puts("pushed new stack element!!");
				getchar();

				if (not ctk[arg0] and not ctk[arg1]) { }
				else if (ctk[arg0]) {
					nat t = new.args[0]; 
					new.args[0] = new.args[1]; 
					new.args[1] = t;
					if (op == lt) new.op = gt_imm; 
					else new.op = eq_imm;
					new.args[1] = values[new.args[1]];
				} else {
					if (op == lt) new.op = lt_imm; 
					else new.op = eq_imm;
					new.args[1] = values[new.args[1]];
				}

			push_rt_ins:;
				nat HERE = rt_count - 1;
				if (rt_count) rt_ins[HERE].gotos[0] = rt_count;
				rt_ins[rt_count++] = new;
				printf("JUST PUSHED NEW ELEMENT!!!\n");
				getchar();
			

			} else if (stack[stack_count - 1].side == 0 and not visited[ins[pc].gotos[1]]) {
				puts("trying other side of the TOS branch!");
				getchar();
				stack[stack_count - 1].side = 1;
				pc = ins[pc].gotos[1];

			} else {
				puts("popping off top element off stack!");
				getchar();
				stack_count--;
				if (not stack_count) {
					puts("NOTE: i think we found the end?..");
					puts("breaking out of loop.");
					break;
				} else {
					puts("note: setting pc to be new value...");
					getchar();
					pc = ins[stack[stack_count].index].gotos[0];
				}
			}

		} else {
			if (pc == ins_count) {
				continue;
			}

			printf("following .false=%llu side of ins...\n", ins[pc].gotos[0]);
			pc = ins[pc].gotos[0];

		if (op == zero) {
			if (ctk[arg0]) { values[arg0] = 0; }
			else {
				new.op = set_imm;
				new.args[1] = 0;
				goto push_rt_ins;
			}

		} else if (op == incr) {
			if (ctk[arg0]) { values[arg0]++; }
			else {
				new.op = add_imm;
				new.args[1] = 1;
				goto push_rt_ins;
			}

		} else if (op == decr) {
			if (ctk[arg0]) { values[arg0]--; }
			else {
				new.op = sub_imm;
				new.args[1] = 1;
				goto push_rt_ins;
			}

		} else if (op == not_) {
			if (ctk[arg0]) { values[arg0] = ~values[arg0]; }
			else {
				new.op = eor_imm;
				new.args[1] = (nat) -1;
				goto push_rt_ins;
			}

		} else if (op == add) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] += values[arg1];
		push_rt:;
			if (not ctk[arg0]) {
				if (ctk[arg1]) {
					new.args[1] = values[arg1];
					if (op == set) new.op = set_imm;
					if (op == add) new.op = add_imm;
					if (op == sub) new.op = sub_imm;
					if (op == mul) new.op = mul_imm;
					if (op == div_)new.op = div_imm;
					if (op == and_)new.op = and_imm;
					if (op == or_) new.op = or_imm;
					if (op == eor) new.op = eor_imm;
					if (op == si)  new.op = si_imm;
					if (op == sd)  new.op = sd_imm;
				}

				if (new.op == set and arg0 == arg1) { }
				else if (new.op == or_ and arg0 == arg1) { }
				else if (new.op == and_ and arg0 == arg1) { }
				else if (new.op == add_imm and values[arg1] == 0) { }
				else if (new.op == sub_imm and values[arg1] == 0) { }
				else if (new.op == mul_imm and values[arg1] == 1) { }
				else if (new.op == div_imm and values[arg1] == 1) { }
				else if (new.op == or_imm and values[arg1] == 0) { }
				else if (new.op == eor_imm and values[arg1] == 0) { }
				else if (new.op == si_imm and values[arg1] == 0) { }
				else if (new.op == sd_imm and values[arg1] == 0) { }
				else goto push_rt_ins;

			} else if (not ctk[arg1]) {
				puts("error: ct destination must have ct source."); 
				abort();
			}
		} else if (op == set) {
			if (ctk[arg1]) values[arg0] = values[arg1];
			goto push_rt;
		} else if (op == sub) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] -= values[arg1];
			goto push_rt;
		} else if (op == mul) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] *= values[arg1];
			goto push_rt;
		} else if (op == div_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] /= values[arg1];
			goto push_rt;
		} else if (op == and_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] &= values[arg1];
			goto push_rt;
		} else if (op == or_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] |= values[arg1];
			goto push_rt;
		} else if (op == eor) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] ^= values[arg1];
			goto push_rt;
		} else if (op == si) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] <<= values[arg1];
			goto push_rt;
		} else if (op == sd) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] >>= values[arg1];
			goto push_rt;

		} else {
			puts("internal error: unknown operation: execution not specified");
			printf("op = %llu, op = %s\n", op, ins_spelling[op]);
			abort();
		}


		}
	}

	printf("found compiletime values of variables: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (not ctk[i]) continue;
		printf("\tvalues[%s] = %llu\n", names[i], values[i]);
	}
	puts("}");

	puts("found rt instruction listing after CT-eval:");
	print_instructions(rt_ins, rt_count, names, name_count);

	puts("compiled.");
	exit(0);
}



*/



































/*

		printf("visited: { ");
		for (nat i = 0; i < ins_count; i++) {
			printf("%hhu ", visited[i]);
		}
		puts("}");

*/





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













/*enum immediate_forms_of_instructions {
	null_imm_unused = isa_count,
};

static const char* ins_imm_spelling[] = {
	"null_imm_unused",
	"set_imm",
	"add_imm", 
	"sub_imm",  
	"mul_imm",   
	"div_imm",
	"and_imm",
	"or_imm", 
	"eor_imm", 
	"si_imm", 
	"sd_imm", 	
	"lt_imm", 
	"eq_imm", 
};*/







	/*while (stack_count or pc < ins_count) { 

		printf("stack: %llu { \n", stack_count);
		for (nat i = 0; i < stack_count; i++) {
			printf("stack[%llu] = %llu\n", i, stack[i]);
		}
		puts("}");
		printf("visited: { ");
		for (nat i = 0; i < ins_count; i++) {
			printf("%hhu ", visited[i]);
		}
		puts("}");
		printf("found compiletime values of variables: {\n");
		for (nat i = 0; i < name_count; i++) {
			if (not ctk[i]) continue;
			printf("\tvalues[%s] = %llu\n", names[i], values[i]);
		}
		puts("}");
		//const nat top = stack[--stack_count];
		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");
		print_instructions(rt_ins, rt_count, names, name_count);
		getchar();
		//visited[pc] = 1;
		struct instruction new = ins[pc];
		const nat 
			op = new.op,
			arg0 = new.args[0], 
			arg1 = new.args[1],
			gt0 = new.gotos[0],
			gt1 = new.gotos[1];

		if (op == lt or op == eq) {
			if (not ctk[arg0] or not ctk[arg1]) goto generate_rt_branch;			
			bool condition = 0;
			if (op == eq and values[arg0] == values[arg1]) condition = 1;
			if (op == lt and values[arg0] <  values[arg1]) condition = 1;
			if (not condition) {
				if (gt0 < ins_count) stack[stack_count++] = gt0;
			} else {
				if (gt1 < ins_count) stack[stack_count++] = gt1;
			}
			goto next_instruction;
			generate_rt_branch:;

			if (not ctk[arg0] and not ctk[arg1]) { }
			else if (ctk[arg0]) {
				nat t = new.args[0]; 
				new.args[0] = new.args[1]; 
				new.args[1] = t;
				if (op == lt) new.op = gt_imm; 
				else new.op = eq_imm;
				new.args[1] = values[new.args[1]];
			} else {
				if (op == lt) new.op = lt_imm; 
				else new.op = eq_imm;
				new.args[1] = values[new.args[1]];
			}

		push_rt_ins:;
			if ((1)) {
				//if (rt_ins[previous_top]) 				
				nat HERE = rt_count - 1;
				if (rt_count) rt_ins[HERE].gotos[0] = rt_count;
				rt_ins[rt_count++] = new;
			} else {
				printf("we are here again!!!\n");
				getchar();
			}

		} else if (op == zero) {
			if (ctk[arg0]) { values[arg0] = 0; }
			else {
				new.op = set_imm;
				new.args[1] = 0;
				goto push_rt_ins;
			}

		} else if (op == incr) {
			if (ctk[arg0]) { values[arg0]++; }
			else {
				new.op = add_imm;
				new.args[1] = 1;
				goto push_rt_ins;
			}

		} else if (op == decr) {
			if (ctk[arg0]) { values[arg0]--; }
			else {
				new.op = sub_imm;
				new.args[1] = 1;
				goto push_rt_ins;
			}

		} else if (op == not_) {
			if (ctk[arg0]) { values[arg0] = ~values[arg0]; }
			else {
				new.op = eor_imm;
				new.args[1] = (nat) -1;
				goto push_rt_ins;
			}

		} else if (op == add) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] += values[arg1];
		push_rt:;
			if (not ctk[arg0]) {
				if (ctk[arg1]) {
					new.args[1] = values[arg1];
					if (op == set) new.op = set_imm;
					if (op == add) new.op = add_imm;
					if (op == sub) new.op = sub_imm;
					if (op == mul) new.op = mul_imm;
					if (op == div_)new.op = div_imm;
					if (op == and_)new.op = and_imm;
					if (op == or_) new.op = or_imm;
					if (op == eor) new.op = eor_imm;
					if (op == si)  new.op = si_imm;
					if (op == sd)  new.op = sd_imm;
				}

				if (new.op == set and arg0 == arg1) { }
				else if (new.op == or_ and arg0 == arg1) { }
				else if (new.op == and_ and arg0 == arg1) { }
				else if (new.op == add_imm and values[arg1] == 0) { }
				else if (new.op == sub_imm and values[arg1] == 0) { }
				else if (new.op == mul_imm and values[arg1] == 1) { }
				else if (new.op == div_imm and values[arg1] == 1) { }
				else if (new.op == or_imm and values[arg1] == 0) { }
				else if (new.op == eor_imm and values[arg1] == 0) { }
				else if (new.op == si_imm and values[arg1] == 0) { }
				else if (new.op == sd_imm and values[arg1] == 0) { }
				else goto push_rt_ins;

			} else if (not ctk[arg1]) {
				puts("error: ct destination must have ct source."); 
				abort();
			}
		} else if (op == set) {
			if (ctk[arg1]) values[arg0] = values[arg1];
			goto push_rt;
		} else if (op == sub) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] -= values[arg1];
			goto push_rt;
		} else if (op == mul) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] *= values[arg1];
			goto push_rt;
		} else if (op == div_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] /= values[arg1];
			goto push_rt;
		} else if (op == and_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] &= values[arg1];
			goto push_rt;
		} else if (op == or_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] |= values[arg1];
			goto push_rt;
		} else if (op == eor) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] ^= values[arg1];
			goto push_rt;
		} else if (op == si) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] <<= values[arg1];
			goto push_rt;
		} else if (op == sd) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] >>= values[arg1];
			goto push_rt;
		} else if (op == sc) {
			if (not ctk[arg0]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg0];
			//const nat input_count = get_call_input_count(n);
			const nat output_count = get_call_output_count(n);
			//for (nat i = 0; i < input_count; i++) {
			//	//if (ctk[ins[top].args[1 + i]]) { puts("system call ct rt in"); abort(); }
			//	//list[list_count++] = ins[top].args[1 + i];
			//}
			for (nat i = 0; i < output_count; i++) {
				if (ctk[ins[top].args[1 + i]]) { puts("system call ct rt out"); abort(); }
				//list[list_count++] = write_access | ins[top].args[1 + i];
			}
			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(ins, ins_count, names, name_count, top, "CFG termination point here");
				goto next_instruction;
			} else {
				printf("info: found %s system call!\n", systemcall_spelling[n]);
				print_instruction_index(ins, ins_count, names, name_count, top, systemcall_spelling[n]);
			}
			//goto push_rt_ins;
			abort(); // we need to be generating a seperate intsruction for each system call.
			// this gets rid of the ct n param entirely, so that we don't need any ct var refs in the rt ins seq.
			// each syscall will have its own  rt ins.  with a specific arity, in and out. 
		} else {
			puts("internal error: unknown operation: execution not specified");
			abort();
		}

		if ((op == lt or op == eq) and gt1 < ins_count and not visited[gt1]) 
			stack[stack_count++] = gt1;

		if (gt0 < ins_count and not visited[gt0]) 
			stack[stack_count++] = gt0;

		next_instruction:;
	}


*/




































/*


	1202501116.214153

	basically the root of the problem here is that     we need to skip the line    when we are ct executing stuff. ie, when we traverse a compiletime known execution edge, we need to actually 


				like    NOTTTT use the graph traversal (GT) machinery  ie the whole stack machinery stuff 
					we need to like   just go to that instruction directly, and start executing it.  not using the stack for any of it. 



					in fact, i think we'd only ever use the stack for rt branches, right???
				i think so.  i think thats literally the only time we push to the stack.. woww..



					okay so basically we need to   revise the GT machinery 

						to make it only stack push  sides and also   only on rt brs 
					we'll just write our own GT stuff i think 


					shouldnt be that harddddd i think lol 

		hmm interestinggg




but yeah thats the root of the problem, i think. 



yay



						
*/


























/*enum language_builtins {
	stacksize, stackpointer,
	builtin_count
};

static const char* builtin_spelling[builtin_count] = {
	"_stacksize",
	"_stackpointer", 
};*/










/*	printf("found list: (%llu count)\n", list_count);
	for (nat i = 0; i < list_count; i++) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);
	}
	puts("done");
*/



/*

			bool in0 = false;
			bool in1 = false;
			for (nat i = 0; i < stack_count; i++) {
				if (stack[i] == gt0) in0 = true;
				if (stack[i] == gt1) in1 = true;
			}

*/

































	//bool* ctk = calloc(name_count, sizeof(bool));
	//nat* values = calloc(name_count, sizeof(nat));	
	//nat* bit_counts = calloc(name_count, sizeof(nat));	
	//nat* list = calloc(8 * ins_count, sizeof(nat));
	//nat list_count = 0;
	//stack[stack_count++] = 0; 
	//const struct instruction nop = {0};	


/*
	while (stack_count) { 
		const nat top = stack[--stack_count];
		printf("visiting ins #%llu\n", top);
		print_instruction_index(ins, ins_count, names, top, "here");
		debug_instruction(ins[top], names); puts("");
		getchar();
		visited[top]++;
		const nat op = ins[top].args[0], arg1 = ins[top].args[1], arg2 = ins[top].args[2];
		if (op == ld) { abort();
		} else if (op == st) { abort();
		} else if (op == ct) { ctk[arg1] = 1; ins[top].count = 0;
		} else if (op == rt) {
			if (not ctk[arg2]) { puts("error: rt instruction arg2 (bit count) must be ct."); abort(); }
			bit_counts[arg1] = values[arg2];
			ctk[arg1] = 0;
			ins[top].count = 0;
		} else if (is_branch(op)) {
			if (not ctk[arg1] or not ctk[arg2]) goto generate_rt_branch;
			bool condition = 0;
			if (op == eq and values[arg1] == values[arg2]) condition = 1;
			if (op == ne and values[arg1] != values[arg2]) condition = 1;
			if (op == lt and values[arg1] <  values[arg2]) condition = 1;
			if (op == ge and values[arg1] >= values[arg2]) condition = 1;
			rt_ins[rt_count++] = ins[top];
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if (not condition) {
				if (false_side < ins_count) stack[stack_count++] = false_side;
			} else {
				if ( true_side < ins_count) stack[stack_count++] = true_side;
			}
			goto next_instruction;
			generate_rt_branch:;
		} else if (op == do_) {
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if (false_side < ins_count) stack[stack_count++] = false_side;
			goto next_instruction;
		} else if (op == zero) {
			if (ctk[arg1]) { values[arg1] = 0; ins[top].count = 0; }
		} else if (op == incr) {
			if (ctk[arg1]) { values[arg1]++; ins[top].count = 0; }
		} else if (op == not_) {
			if (ctk[arg1]) { values[arg1] = ~values[arg1]; ins[top].count = 0; }
		} else if (op == add) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] += values[arg2];
		push_rt:;
			if (not ctk[arg1]) {
				if (ctk[arg2]) {
					ins[top].args[2] = values[arg2];
					if (op == set) ins[top].args[0] = set_imm;
					if (op == add) ins[top].args[0] = add_imm;
					if (op == sub) ins[top].args[0] = sub_imm;
					if (op == mul) ins[top].args[0] = mul_imm;
					if (op == div_)ins[top].args[0] = div_imm;
					if (op == and_)ins[top].args[0] = and_imm;
					if (op == or_) ins[top].args[0] = or_imm;
					if (op == eor) ins[top].args[0] = eor_imm;
					if (op == si)  ins[top].args[0] = si_imm;
					if (op == sd)  ins[top].args[0] = sd_imm;
				}
			} else {
				if (ctk[arg2]) {
					ins[top].count = 0;
				} else {
					puts("error: compiletime destination must have compiletime source."); abort();
				}
			}
		} else if (op == set) {
			if (ctk[arg2]) values[arg1] = values[arg2];
			goto push_rt;
		} else if (op == sub) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] -= values[arg2];
			goto push_rt;
		} else if (op == mul) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] *= values[arg2];
			goto push_rt;
		} else if (op == div_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] /= values[arg2];
			goto push_rt;
		} else if (op == and_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] &= values[arg2];
			goto push_rt;
		} else if (op == or_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] |= values[arg2];
			goto push_rt;
		} else if (op == eor) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] ^= values[arg2];
			goto push_rt;
		} else if (op == si) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] <<= values[arg2];
			goto push_rt;
		} else if (op == sd) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] >>= values[arg2];
			goto push_rt;
		} else if (op == sc) {
			if (not ctk[arg1]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg1];
			//const nat input_count = get_call_input_count(n);
			const nat output_count = get_call_output_count(n);
			//for (nat i = 0; i < input_count; i++) {
			//	//if (ctk[ins[top].args[2 + i]]) { puts("system call ct rt in"); abort(); }
			//	//list[list_count++] = ins[top].args[2 + i];
			//}
			for (nat i = 0; i < output_count; i++) {
				if (ctk[ins[top].args[2 + i]]) { puts("system call ct rt out"); abort(); }
				//list[list_count++] = write_access | ins[top].args[2 + i];
				ctk[ins[top].args[2 + i]] = false;
			}

			rt_ins[rt_count++] = ins[top];

			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(ins, ins_count, names, top, "CFG termination point here");
				goto next_instruction;

			} else {
				printf("info: found %s system call!\n", systemcall_spelling[n]);
				print_instruction_index(ins, ins_count, names, top, systemcall_spelling[n]);
			}
		}

		nat true_side = (nat) -1;
		const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
		if (is_branch(op)) {
			if ( true_side < ins_count and visited[true_side] < 1)  stack[stack_count++] = true_side;
			if (false_side < ins_count and visited[false_side] < 1) stack[stack_count++] = false_side;
		} else {
			if (false_side < ins_count) stack[stack_count++] = false_side;
		}
		next_instruction:;
	}



	puts("ins:");
	debug_instructions(ins, ins_count, names);

	puts("rt_ins:");
	debug_instructions(rt_ins, rt_count, names);






	puts("}");
	printf("found compiletime values of variables: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (not ctk[i]) continue;
		printf("\tvalues[%s] = %llu\n", names[i], values[i]);
	}
	puts("}");
	printf("found list: (%llu count)\n", list_count);
	for (nat i = 0; i < list_count; i++) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);
	}
	puts("done");



	bool* alive = calloc(name_count, sizeof(bool));

	for (nat i = list_count; i--;) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);

		const bool is_write = !!(list[i] >> 63);

		if (is_write) {			
			alive[variable_index] = 0;
		} else {
			alive[variable_index] = 1;			
		}
		
		printf("alive = { ");
		for (nat n = 0; n < name_count; n++) {
			if (alive[n]) printf("%s  ", names[n]);
		} 
		printf(" }\n");
		//getchar();
	}





*/








/*


		printf("stack: %llu { \n", stack_count);
		for (nat i = 0; i < stack_count; i++) {
			printf("stack[%llu] = %llu\n", i, stack[i]);
		}
		puts("}");

		printf("visited: { ");
		for (nat i = 0; i < ins_count; i++) {
			printf("%hhu ", visited[i]);
		}
		puts("}");

		printf("ctk: { ");
		for (nat i = 0; i < name_count; i++) {
			if (ctk[i]) printf("%s ", names[i]);
		}
		puts("}");

		printf("CT values of variables: {\n");
		for (nat i = 0; i < name_count; i++) {
			if (not ctk[i]) continue;
			printf("\tvalues[%s] = %llu\n", names[i], values[i]);
		}
		puts("}");


*/





/*	printf("performing unreachable analysis...\n");
	for (nat i = 0; i < rt_count; i++) {

		//const nat op = rt_ins[i].args[0], arg1 = rt_ins[i].args[1], arg2 = rt_ins[i].args[2];

		/if (not visited[i]) {
			printf("warning: instruction is unreachable\n");
			print_instruction_index(ins, ins_count, names, i, "unreachable");
			puts("");
			ins[i].count = 0;
		}

		if (ctk[arg1] and op != sc) {
			ins[i].count = 0;
		}
	}
*/


/*




				ERROR ERROR ERROR       we are in the middle of making this pass   generate   rt_ins    never edit main instruction sequence, ins. 

					this is because we want to avoid generating    the at loop's,  and also  get the immediate to be different for each add_imm that we generate. we can't do that since we are overwriting the original instruction, which we used for execution lol. i think. sometihng like that. basically we need to do the rt_ins lol. its a must. don't worry about unreachable instructions, we won't generate them anyways, becuase we are traversing the graph in order to know the right instructions to output. i think. CRAP 


							BUT WHAT ABOUT THE FACT THAT 




									WE WONT BE GENERATING THE INSTRUCTIONS IN THE CORRECT ORDERRRR CRAPPPP



									DUE TO IMPLICIT IP++'s    NOT LINING UPPPP



				BECAUSE OF OUR GRAPH TRAVERSAL ORDERINGGGG   CRAPPPPPP









uh oh lol 


uhh





hmm

*/


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



			if (ctk[arg0]) { // we therefore know arg1 is   NOT ctk. 


				if (new.op == lt) {

				} else {

				}


			} else {
				if (ctk[arg1]) {

					if (new.op == lt) {

					} else {

					}
				} else { // we know that both are ct. generate the branch, normally. 
					if (new.op == lt) {

					} else {

					}

				}
			}



*/




			/*if (ctk[arg0] ^ ctk[arg1] and new.op == eq) new.op = eq_imm;


			if (not ctk[arg0] and ctk[arg1] and new.op == lt) new.op = lt_imm;
			if (ctk[arg0] and not ctk[arg1] and new.op == lt) { 
				new.op = gt_imm; nat t = new.args[0]; 
				new.args[0] = new.args[1]; new.args[1] = t;
			} 

			//if (ctk[arg

			rt_ins[rt_count++] = new;
*/






				//state = 0;
				//if (unreachable) goto next_word;
				// this is not correct. 
				//ins[ins_count - 1].gotos[0] = variable | is_label;
				// nop do label  at skip nop  at label do label2
				//   the above code breaks this method. 








/*static nat isa_arity(nat i) {
	if (i == sc) return 7;
	if (i == eoi) return 0;
	if (i >= zero and i <= lf) return 1;
	if (i >= set  and i <= rt) return 2;
	if (i >= lt   and i <= st) return 3;
	abort();
}

static nat get_call_input_count(nat n) {
	if (n == system_exit) return 1;
	if (n == system_read) return 3;
	if (n == system_write) return 3;
	if (n == system_close) return 1;
	if (n == system_open) return 3;
	abort();
}*/





/*


		" %s"
		" %s"
		//" {ct_side=%u}"
		" %s",
		this.gotos[0], this.gotos[1],
		"",//!!(this.ct & ct_is_generated_do) ? "[machine-do]" : "", 
		"",//!!(this.ct & ct_is_unreachable) ? "[unreachable]" : "",
		//0,//!!(this.ct & ct_is_ctbranch_side), 
		""//!!(this.ct & ct_is_compiletime) ? "[compiletime]" : ""
	);



	if (use_color) {
		if (this.ct & ct_is_unreachable) printf("\033[0m");
		else if (this.ct & ct_is_compiletime) printf("\033[0m");
	}*/




/*

//printf("PRED UNIDENT: c=%u, a=%u, b=%u\n", c, a, b);
				//getchar();		
if (pred_ctk[n] and ctk[n]) {
						puts("found both CTK!!!! unknown merge method");
						abort();



					}

static const char* systemcall_spelling[systemcall_count] = {
	"system_exit",
	"system_read", "system_write", 
	"system_open", "system_close",
};


*/


		//printf("info:   --> pred_count = %llu\n", pred_count);
/*printf("FOUND PREDECESSOR!!!! (%llu total so far)\n", pred_count);
			print_instruction_index(ins, ins_count, names, name_count, i, "predecessor of pc");
			print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
			printf("DONE WITH PREDECESSOR\n");
			printf("COMPARING NAME VALUE CTK LISTS:\n");
			nat* pred_values = execution_state_values + name_count * i;
			nat* pred_ctk = execution_state_ctk + name_count * i;	


			for (nat n = 0; n < name_count; n++) {
				printf("%llu:   {PC: CTK[%llu],VALUES[%llu]}    |   {pred: CTK[%llu],VALUES[%llu]} \n",
					n,               ctk[n],values[n],          pred_ctk[n], pred_values[n]
				);
					if (not pred_ctk[n]) {

						// then we know that at least one of the predecessors of PC transfers RT known data about this variable.
						// if this is the case, we must assume that now, at this point, names[n] is RT here as well. 

						ctk[n] = 0;
						values[n] = 0x999999999999;
					}
				}
				puts("done comparing these two pred and pc lists.");
				getchar();
			}
			*/





		// 1. find list of pred of this instruction.
		
		// 2. synthesize the ctk/value listing for each other path. if both ctk, but mismatching values,
		//    then call it not rtk.

		// 3. ????
		
		//nat** CRAZY_ctk = calloc(2 * name_count, sizeof(nat*));
		//nat** CRAZY_values = calloc(2 * name_count, sizeof(nat*));



		/*for (nat i = 0; i < pred_count; i++) {
			print_instructions_ct_values_index(
				ins, ins_count, 
				names, name_count, locations, 
				execution_state_ctk, execution_state_values,
				preds[i], "PREDECESSOR"
			);
		}*/

/*static void print_instruction_index(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count,
	nat here, const char* message
) {
	printf("%s: at index: %llu: { \n", message, here);
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		if (i == here)  printf("  <------ %s\n", message); else puts("");
	}
	puts("}");
}

static void print_ct_values(char** names, nat name_count, nat* ctk, nat* values) {
	printf("ct values: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (not ctk[i]) continue;
		printf("\tvalues[%s] = 0x%016llx\n", names[i], values[i]);
	}
	puts("}");
}*/
/*	if (use_color) {
		if (this.ct & ct_is_unreachable) printf("\033[38;5;239m");
		else if (this.ct & ct_is_compiletime) printf("\033[38;5;101m");
	}*/

	//printf("[.ct=%llx]", this.ct);







//EOI




