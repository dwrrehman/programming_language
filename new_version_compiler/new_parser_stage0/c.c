// 1202501116.181306 new parser   dwrr

// things to layer on to the front end still: 
//   - including of multiple files
//   - comments


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
	set_imm,  
	add_imm, 
	sub_imm,  
	mul_imm,   
	div_imm,
	and_imm,
	or_imm, 
	eor_imm, 
	si_imm, 
	sd_imm, 	
	lt_imm, 
	gt_imm, 
	eq_imm,  
};

static const char* ins_spelling[] = {
	"__ERROR_null_ins_unused__", 
	"zero", "incr", "decr", 
	"not", "and", "or", "eor", "si", "sd",
	"set", "add", "sub", "mul", "div", 
	"lt", "eq", "ld", "st", "sc",
	"do", "at", "lf", "ge", "ne", "ct", "rt",

	"__ISA_COUNT__ins_unused__",
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
	
};

struct instruction {
	nat op;
	nat gotos[2];
	nat args[7];
};

/*enum language_builtins {
	stacksize, stackpointer,
	builtin_count
};

static const char* builtin_spelling[builtin_count] = {
	"_stacksize",
	"_stackpointer", 
};*/

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

/*enum arm64_ins_set {
	addsr, addsr_k0,        //TODO: add more 
	arm_isa_count,
};

enum immediate_forms_of_instructions {
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

//static const nat write_access = (nat) (1LLU << 63LLU);

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

/*static nat isa_arity(nat i) {
	if (i == sc) return 7;
	if (i == eoi) return 0;
	if (i >= zero and i <= lf) return 1;
	if (i >= set  and i <= rt) return 2;
	if (i >= lt   and i <= st) return 3;
	abort();
}*/

static nat get_call_input_count(nat n) {
	if (n == system_exit) return 1;
	if (n == system_read) return 3;
	if (n == system_write) return 3;
	if (n == system_close) return 1;
	if (n == system_open) return 3;
	abort();
}

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


static void print_dictionary(char** names, nat* ctk, nat* values, nat* locations, nat* bit_count, nat name_count) {
	puts("found dictionary: { \n");
	for (nat i = 0; i < name_count; i++) {
		printf("\t%llu: name = \"%s\", ctk = %llu, value = %llu, location = %llu, bit_count = %llu\n",
			i, names[i], ctk[i], 
			values[i], locations[i], 
			bit_count[i]
		);
	}
	puts("}");
}

static void print_instruction(struct instruction this, char** names, nat name_count) {
	printf("%s {%lld(%s) %lld(%s) %lld(%s) %lld(%s)} : {.f=#%lld .t=#%lld}", 
		ins_spelling[this.op], 
		this.args[0], this.args[0] < name_count ? names[this.args[0]] : "",
		this.args[1], this.args[1] < name_count ? names[this.args[1]] : "",
		this.args[2], this.args[2] < name_count ? names[this.args[2]] : "",
		this.args[3], this.args[3] < name_count ? names[this.args[3]] : "",
		this.gotos[0], this.gotos[1]
	);
}

static void print_instructions(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count
) {
	puts("found instructions: { \n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%llu: ", i);
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
		printf("\t#%llu: ", i);
		print_instruction(ins[i], names, name_count);
		if (i == here)  printf("  <------ %s\n", message); else puts("");
	}
	puts("}");
}

int main(int argc, const char** argv) {

	struct instruction ins[4096] = {0};
	nat ins_count = 0;

	char* names[4096] = {0};
	nat ctk[4096] = {0};          //ctk[stacksize] = 1;
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
				if (arg_count < 2) goto next_word;
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
	}

	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].gotos[0] & is_label) {
			ins[i].gotos[0]	= locations[ins[i].gotos[0] & ~is_label];
		}
		if (ins[i].gotos[1] & is_label) {
			ins[i].gotos[1]	= locations[ins[i].gotos[1] & ~is_label];
		}
	}

	print_dictionary(names, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);

	uint8_t* visited = calloc(ins_count, 1);
	nat* stack = calloc(2 * ins_count, sizeof(nat));
	nat stack_count = 0;
	stack[stack_count++] = 0; 

	struct instruction rt_ins[4096] = {0};
	nat rt_count = 0;

	while (stack_count) { 

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

		const nat top = stack[--stack_count];

		print_instruction_index(ins, ins_count, names, name_count, top, "here");
		printf("visiting ins #%llu\n", top);
		print_instruction(ins[top], names, name_count); puts("");
		print_instructions(rt_ins, rt_count, names, name_count);

		getchar();

		visited[top] = 1;

		struct instruction new = ins[top];

		const nat 
			op = ins[top].op,
			arg0 = ins[top].args[0], 
			arg1 = ins[top].args[1],
			gt0 = ins[top].gotos[0],
			gt1 = ins[top].gotos[1];

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
			if ((1)) { // visited[top] == 

				//if (rt_ins[previous_top]) 
				
				nat HERE = rt_count - 1;

				if (rt_count) 
					rt_ins[HERE].gotos[0] = rt_count;

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

				} else {
					//puts("error: both arguments are runtime."); 
					//abort();
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



			} else {
				if (ctk[arg1]) {
					//puts("error: both arguments are compiletime."); 
					//abort();
				} else {
					puts("error: compiletime destination must have compiletime source."); 
					abort();
				}
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














	printf("found compiletime values of variables: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (not ctk[i]) continue;
		printf("\tvalues[%s] = %llu\n", names[i], values[i]);
	}
	puts("}");



/*	printf("found list: (%llu count)\n", list_count);
	for (nat i = 0; i < list_count; i++) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);
	}
	puts("done");
*/



	puts("found rt instruction listing after CT-eval:");
	print_instructions(rt_ins, rt_count, names, name_count);

	puts("compiled.");
	exit(0);
}
























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






