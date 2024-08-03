// 1202407302.211405  dwrr
// the parser for the programming language,
// the new version with function defs and riscv isa ish...
//



NOTE


	there is a difference between macros and functions:  i just figured it out now:
		   ---------------------------------------


			it has to do with labels:


					if you want to be able to jump from one label defined in a top level scope, for example, 


						but you want to jump to it, FROM a goto inside of a "function" ("macro", really)

								then we need to actually do the instruction subsitution for the call, 


									BEFOREEEE we execute it.  i think this is the only real way to do this. 



				NOTE:   allowing for these types of jumps is quite useful, becuse then it literally means that  you can redefine 


				like,    a branch if equal,    or a less than branch              TO BE called ANYTHING YOU WANT


							ie, these are true macros, now, 



								 that just HAPPENNNN to be veryyyy hygenic becuase of the way we made them lol. 

											so yeah. 





				i think this is the right way to go, i thinkkkkkk












				



/*
copyb insert ./build
copya insert ./run
copyb do ,./build
copya do ,./run

rename todo:
add these names to the isa:

		incr
		decr
		zero

those are pretty important lol... so yeah. i think i want those names too.





language isa:
===================


set d r

add d r
sub d r

mul d r
muh d r
mhs d r
div d r
dvs d r
rem d r
rms d r

and d r
or  d r
eor d r
sr  d r
srs d r
sl  d r

incr d
zero d
decr d
not d

lt  l r s
ge  l r s
lts l r s
ges l r s
eq  l r s
ne  l r s

ld  d p t
st  p r t

lf  f
at  l 

reg r
rdo r
ctk r

env

def o
ar r
ret
obs





example code, usage of the "obs" instruction


		def local_scope

			def actual_logic ar x

				...something with x...

				ret


			def public_interface obs ar x
				actual_logic x
				ret


		ret


		set my_x 4
		public_interface my_x





*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <iso646.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

typedef uint64_t nat;

enum language_isa {
	nullins,
	zero, incr, decr, set, add, sub, mul, div_, rem, sl, sr, and_, or_, eor, not_, 
	lt, ge, ne, eq, env, at, def, ret, ar, lf,
	isa_count
};

static const char* ins_spelling[isa_count] = {
	"()",
	"zero", "incr", "decr", "set", "add", "sub", "mul", "div", "rem", "sl", "sr", "and", "or", "eor", "not", 
	"lt", "ge", "ne", "eq", "env", "at", "def", "ret", "ar", "lf", 
};


enum language_builtins {
	nullvar,
	stackpointer, stacksize,
	builtin_count
};

static const char* builtin_spelling[builtin_count] = {
	"(nv)",
	"stackpointer", "stacksize",
};




struct instruction {
	nat* args;
	nat count;
};

struct function {
	nat arity;
	nat* arguments;
	struct instruction* body;
	nat body_count;
};

struct dictionary {
	char** names;
	nat* values;
	nat count;
};

struct scope {
	nat** list;
	nat* count;
	nat function;
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

static nat arity(nat i) {
	if (not i) return 0;
	if (i == ret or i == env) return 0; 
	if (i == incr or i == decr or i == zero or i == not_ or
	i == def or i == ar or i == lf or i == at) return 1;
	if (i == lt or i == ge or i == ne or i == eq) return 3;
	return 2;
}

static void debug_instructions(struct instruction* ins, nat ins_count, struct dictionary d) {
	printf("instructions: (%llu count) \n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("%5llu:", i);
		for (nat a = 0; a < ins[i].count; a++) {
			 printf("  %20s : %-5lld", d.names[ins[i].args[a]], ins[i].args[a]);
		}
		puts("");
	}
	puts("done\n");
}


static void debug_dictionary(struct dictionary d) {
	printf("dictionary: (%llu count)\n", d.count);
	for (nat i = 0; i < d.count; i++) {
		printf("%5llu: .name = %20s, .value = %5llu\n", i, d.names[i], d.values[i]);
	}
	puts("done\n");
}

static void debug_scopes(struct scope* scopes, nat scope_count) {
	printf("scope stack: (%llu count)\n", scope_count);
	for (nat i = 0; i < scope_count; i++) {
		printf("\tscope %5llu: \n", i);
		for (nat t = 0; t < 2; t++) {
			printf("\t\t[%4llu]: ", t);
			for (nat n = 0; n < scopes[i].count[t]; n++) 
				printf("%4llu ", scopes[i].list[t][n]);
			puts("");
		}
		puts("");
	}
	puts("done\n");
}

static void debug_functions(struct function* functions, nat function_count, struct dictionary d) {
	printf("functions: (%llu count)\n", function_count);
	for (nat f = 0; f < function_count; f++) {
		printf("%5llu: .args = (%llu)[ ", f, functions[f].arity);
		for (nat a = 0; a < functions[f].arity; a++) {
			printf("%5llu ", functions[f].arguments[a]);
		}
		puts("]");
		puts("body: ");
		debug_instructions(functions[f].body, functions[f].body_count, d);
		puts("[end-body]");
	}
	puts("done\n");
}



static void debug_registers(nat* r, nat count) {
	printf("registers: (%llu count)\n", count);
	for (nat i = 0; i < count; i++) {
		if (i % 4 == 0) puts("");
		printf("%5llu: 0x%016llx %5lld      ", i, r[i], r[i]);
	}
	puts("\ndone\n");
}


int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run <file.s>"));

	puts("this assembler is currently a work in progress, "
		"backend is currently not fully implemented yet..."
	);

	const char* given_filename = argv[1];

	struct function* functions = NULL;
	nat function_count = 0;

	nat scope_count = 1;
	struct scope* scopes = calloc(1, sizeof(struct scope));
	scopes[0].list = calloc(2, sizeof(nat*));
	scopes[0].count = calloc(2, sizeof(nat));
	scopes[0].function = 0;

	struct dictionary dictionary = {0};

	nat stack_count = 1;
	struct file stack[4096] = {0};
	
{
	int file = open(given_filename, O_RDONLY);
	if (file < 0) { puts(given_filename); perror("open"); exit(1); }
	const nat text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(text_length + 1, 1);
	read(file, text, text_length);
	close(file);

	stack[0].filename = given_filename;
	stack[0].text = text;
	stack[0].text_length = text_length;
	stack[0].index = 0;
}

	for (nat i = 0; i < isa_count; i++) {

		const nat a = arity(i);

		functions = realloc(functions, sizeof(struct function) * (function_count + 1));
		functions[function_count++] = (struct function) {
			.arity = a,
			.arguments = calloc(a, sizeof(nat)),
			.body = NULL,
			.body_count = 0,
		};

		scopes[0].list[0] = realloc(scopes[0].list[0], sizeof(nat) * (scopes[0].count[0] + 1));
		scopes[0].list[0][scopes[0].count[0]++] = dictionary.count;

		dictionary.names = realloc(dictionary.names, sizeof(char*) * (dictionary.count + 1));
		dictionary.values = realloc(dictionary.values, sizeof(nat) * (dictionary.count + 1));
		dictionary.names[dictionary.count] = strdup(ins_spelling[i]);
		dictionary.values[dictionary.count++] = function_count - 1;
	}

	for (nat i = 0; i < builtin_count; i++) {
		scopes[0].list[1] = realloc(scopes[0].list[1], sizeof(nat) * (scopes[0].count[1] + 1));
		scopes[0].list[1][scopes[0].count[1]++] = dictionary.count;

		dictionary.names = realloc(dictionary.names, sizeof(char*) * (dictionary.count + 1));
		dictionary.values = realloc(dictionary.values, sizeof(nat) * (dictionary.count + 1));
		dictionary.names[dictionary.count] = strdup(builtin_spelling[i]);
		dictionary.values[dictionary.count++] = 0;
	}

	const char* included_files[4096] = {0};
	nat included_file_count = 0;

process_file:;
	nat word_length = 0, word_start = 0, in_args = 0;

	const nat starting_index = 	stack[stack_count - 1].index;
	const nat text_length = 	stack[stack_count - 1].text_length;
	char* text = 			stack[stack_count - 1].text;
	const char* filename = 		stack[stack_count - 1].filename;

	printf("info: now processing file: %s...\n", filename);

	for (nat index = starting_index; index < stack[stack_count - 1].text_length; index++) {
		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++; 
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;
		char* word = strndup(text + word_start, word_length);
		
		// printf("%llu:%llu: @ word: %s..\n", word_start, word_length, word);

		nat name = (nat) -1;
		for (nat s = scope_count; s--;) {
			nat* list = scopes[s].list[in_args];
			nat count = scopes[s].count[in_args];
			for (nat i = 0; i < count; i++) {
				if (not strcmp(dictionary.names[list[i]], word) and 
					in_args != dictionary.values[list[i]]) {

					// puts("found name!");
					// puts(word);
					// printf("in_args = %llu\n", in_args);
					name = list[i];
					goto found;
				}
			}
		}

		if (not in_args) {
			puts("unknown operation: ");
			puts(word);
			printf("in_args = %llu\n", in_args);
			abort();

		} else {

			const nat op = functions[scopes[scope_count - 1].function]
				.body[functions[scopes[scope_count - 1]
				.function].body_count - 1]
				.args[0];
	
			if (op == def or op == ar) goto found;

			puts("defining a new name:");
			puts(word);


			const nat t = 1;
			scopes[scope_count - 1].list[t] = 
			realloc(scopes[scope_count - 1].list[t], 
			sizeof(nat) * (scopes[scope_count - 1].count[t] + 1));
			scopes[scope_count - 1].list[t][
			scopes[scope_count - 1].count[t]++] = dictionary.count;

			dictionary.names = realloc(dictionary.names, sizeof(char*) * (dictionary.count + 1));
			dictionary.values = realloc(dictionary.values, sizeof(nat) * (dictionary.count + 1));
			dictionary.names[dictionary.count] = word;
			dictionary.values[dictionary.count++] = 0;
			name = dictionary.count - 1;
		}
	found:
		if (not in_args) {

			functions[scopes[scope_count - 1].function].body = realloc(
			functions[scopes[scope_count - 1].function].body, 
			sizeof(struct instruction) * (
			functions[scopes[scope_count - 1].function].body_count + 1));
			functions[scopes[scope_count - 1].function].body[
			functions[scopes[scope_count - 1].function].body_count++
			] = (struct instruction) {0};

			in_args = 1;
		}


		functions[scopes[scope_count - 1].function].body[
		functions[scopes[scope_count - 1].function].body_count - 1].args = realloc(
		functions[scopes[scope_count - 1].function].body[
		functions[scopes[scope_count - 1].function].body_count - 1].args,
		sizeof(nat) * (functions[scopes[scope_count - 1].function].body[
		functions[scopes[scope_count - 1].function].body_count - 1].count + 1));

		functions[scopes[scope_count - 1].function].body[
		functions[scopes[scope_count - 1].function].body_count - 1]
		.args[functions[scopes[scope_count - 1].function].body[
		functions[scopes[scope_count - 1].function].body_count - 1]
		.count++] = name;

		const nat op = functions[scopes[scope_count - 1].function]
		.body[functions[scopes[scope_count - 1].function]
		.body_count - 1].args[0];

		if (functions[scopes[scope_count - 1].function].body[
		functions[scopes[scope_count - 1].function].body_count - 1]
		.count >= functions[dictionary.values[op]].arity + 1) {

			if (op == lf) {
				functions[scopes[scope_count - 1].function].body_count--;

				for (nat i = 0; i < included_file_count; i++) {
					if (not strcmp(included_files[i], word)) {
						printf("warning: %s: this file has already been included, ignoring.\n", word);
						goto skip_include;
					}
				}

				printf("including file %s...\n", word);
				included_files[included_file_count++] = word;

				int file = open(word, O_RDONLY);
				if (file < 0) { puts(word); perror("open"); exit(1); }
				const nat new_text_length = (nat) lseek(file, 0, SEEK_END);
				lseek(file, 0, SEEK_SET);
				char* new_text = calloc(new_text_length + 1, 1);
				read(file, new_text, new_text_length);
				close(file);

				stack[stack_count - 1].index = index;
				stack[stack_count].filename = word;
				stack[stack_count].text = new_text;
				stack[stack_count].text_length = new_text_length;
				stack[stack_count++].index = 0;
				goto process_file;

				skip_include:;
			}

			if (op == ret) {
				puts("executing ret....");
				functions[scopes[scope_count - 1].function].body_count--;
				scope_count--;
				functions[scopes[scope_count - 1].function].body_count--;
			}

			if (op == ar) {
				functions[scopes[scope_count - 1].function].body_count--;
				puts("executing ar....");

				const nat t = 1;
				scopes[scope_count - 1].list[t] = 
				realloc(scopes[scope_count - 1].list[t], 
				sizeof(nat) * (scopes[scope_count - 1].count[t] + 1));
				scopes[scope_count - 1].list[t][
				scopes[scope_count - 1].count[t]++] = dictionary.count;

				dictionary.names = realloc(dictionary.names, sizeof(char*) * (dictionary.count + 1));
				dictionary.values = realloc(dictionary.values, sizeof(nat) * (dictionary.count + 1));
				dictionary.names[dictionary.count] = word;
				dictionary.values[dictionary.count++] = 0;

				functions[function_count - 1].arguments = realloc(
				functions[function_count - 1].arguments, sizeof(nat) * (
				functions[function_count - 1].arity + 1));
				functions[function_count - 1].arguments[
				functions[function_count - 1].arity++] = dictionary.count - 1;

				// debug_dictionary(dictionary);
				// debug_functions(functions, function_count, dictionary);
				// debug_scopes(scopes, scope_count);
			}

			if (op == def) {

				puts("EXECUTING DEF!!!");
				puts("defining a new name:");
				puts(word);

				//functions[scopes[scope_count - 1].function].body_count--;

				functions = realloc(functions, sizeof(struct function) * (function_count + 1));
				functions[function_count++] = (struct function) {0};

				const nat t = 0;
				scopes[scope_count - 1].list[t] = 
				realloc(scopes[scope_count - 1].list[t], 
				sizeof(nat) * (scopes[scope_count - 1].count[t] + 1));
				scopes[scope_count - 1].list[t][
				scopes[scope_count - 1].count[t]++] = dictionary.count;

				dictionary.names = realloc(dictionary.names, sizeof(char*) * (dictionary.count + 1));
				dictionary.values = realloc(dictionary.values, sizeof(nat) * (dictionary.count + 1));
				dictionary.names[dictionary.count] = word;
				dictionary.values[dictionary.count++] = function_count - 1;
				functions[scopes[scope_count - 1].function].body[
				functions[scopes[scope_count - 1].function].body_count - 1]
				.args[functions[scopes[scope_count - 1].function].body[
				functions[scopes[scope_count - 1].function].body_count - 1]
				.count - 1] = dictionary.count - 1;

				scopes = realloc(scopes, sizeof(struct scope) * (scope_count + 1));
				scopes[scope_count++] = (struct scope) {0};
				scopes[scope_count - 1].list = calloc(2, sizeof(nat*));
				scopes[scope_count - 1].count = calloc(2, sizeof(nat));
				scopes[scope_count - 1].function = function_count - 1;

			}
			in_args = 0;
		}


		// puts("finished with word");
		// puts(word);

		// debug_dictionary(d);
		// debug_functions(functions, function_count, d);
		// debug_scopes(scopes, scope_count);

		word_length = 0;
	}

	stack_count--;

	if (not stack_count) {
		puts("processing_file: finished last file.");
		// do nothing. 
	} else {
		puts("processing next file in the stack...");
		goto process_file;
	}

	debug_dictionary(dictionary);
	debug_functions(functions, function_count, dictionary);
	debug_scopes(scopes, scope_count);

	puts("done parsing! finding ats...");

	nat* R = calloc(dictionary.count, sizeof(nat));

	for (nat f = 0; f < function_count; f++) {
		for (nat pc = 0; pc < functions[f].body_count; pc++) {




			printf("executing: %5llu:", pc);
			for (nat a = 0; a < functions[f].body[pc].count; a++) {
				 printf("  %20s : %-5lld", dictionary.names[functions[f].body[pc].args[a]], functions[f].body[pc].args[a]);
			}
			puts("");


		



			const nat op = functions[f].body[pc].args[0];
			if (op == at) {
				const nat d = functions[f].body[pc].args[1];
				printf("executed at: assigned R[%llu] = %llu...\n", d, pc);
				R[d] = pc;
			}
		}
	}

	puts("executing instructions now...");

	nat call_stack_count = 1;
	nat call_stack[4096] = {0};
	nat return_address_stack[4096] = {0};
	nat* argumentlist_stack[4096] = {0};

	// add something to help with implmenting exec arguments.

	nat last_used = 0;

execute_function:;

	const nat f = call_stack[call_stack_count - 1];
	const nat init_pc = return_address_stack[call_stack_count - 1];
	nat* call_arguments = argumentlist_stack[call_stack_count - 1];

	for (nat pc = init_pc; pc < functions[f].body_count; pc++) {

		const nat op = functions[f].body[pc].args[0];

		nat d = functions[f].body[pc].count >= 2 ? functions[f].body[pc].args[1] : 0;
		nat r = functions[f].body[pc].count >= 3 ? functions[f].body[pc].args[2] : 0;
		nat s = functions[f].body[pc].count >= 4 ? functions[f].body[pc].args[3] : 0;

		for (nat a = 0; a < functions[f].arity; a++) {
			if (d == functions[f].arguments[a]) d = call_arguments[a];
			if (r == functions[f].arguments[a]) r = call_arguments[a];
			if (s == functions[f].arguments[a]) s = call_arguments[a];
		}


		printf("executing: %5llu:", pc);
		for (nat a = 0; a < functions[f].body[pc].count; a++) {
			  printf("  %20s : %-5lld", dictionary.names[functions[f].body[pc].args[a]], functions[f].body[pc].args[a]);
		}
		puts("");


		if (false) {}
		else if (op == at) {}// { puts("executed at: IGNORING"); }
		else if (op == zero) R[d] = 0;
		else if (op == incr) R[d]++;
		else if (op == decr) R[d]--;
		else if (op == not_) R[d] = ~R[d];
		else if (op == set)  R[d]  = R[r];
		else if (op == add)  R[d] += R[r];
		else if (op == sub)  R[d] -= R[r];
		else if (op == mul)  R[d] *= R[r];
		else if (op == div_) R[d] /= R[r];
		else if (op == rem)  R[d] %= R[r];
		else if (op == sl)   R[d]<<= R[r];
		else if (op == sr)   R[d]>>= R[r];
		else if (op == and_) R[d] &= R[r];
		else if (op == or_)  R[d] |= R[r];
		else if (op == eor)  R[d] ^= R[r];
		else if (op == lt) { if (R[r] < R[s]) { pc = R[d]; } } 
		else if (op == ge) { if (R[r] >= R[s]) { pc = R[d]; } } 
		else if (op == ne) { if (R[r] != R[s]) { pc = R[d]; } } 
		else if (op == eq) { if (R[r] == R[s]) { pc = R[d]; } } 
		else if (op == env) printf("\033[32;1mdebug:   0x%llx : %lld\033[0m\n", R[last_used], R[last_used]); 
		else if (op >= isa_count) {
			printf("executing user-defined function: %s...\n", dictionary.names[op]);
			return_address_stack[call_stack_count - 1] = pc + 1;
			return_address_stack[call_stack_count] = 0;

			argumentlist_stack[call_stack_count] = functions[f].body[pc].args + 1;
			call_stack[call_stack_count++] = dictionary.values[op];
			goto execute_function;
			
		} else {
			printf("error: executing unknown instruction: %llu (%s)\n", op, dictionary.names[op]);
			abort();
		}
		last_used = d;
	}
	call_stack_count--;
	if (call_stack_count) goto execute_function;
	debug_registers(R, dictionary.count);
	exit(0);
}


// exit















		/*printf("executing: %5llu: %20s : %-5lld %20s : %-5lld %20s : %-5lld %20s : %-5lld\n", 
			pc, 
			dictionary.names[op], op,
			dictionary.names[d], d,
			dictionary.names[r], r,
			dictionary.names[s], s
		);*/






	/*for () {

			}
			printf("parsing:    %5llu: %20s : %-5lld %20s : %-5lld %20s : %-5lld %20s : %-5lld\n", 
				i, 
				dictionary.names[functions[f].body[i].args[0]], functions[f].body[i].args[0],
				dictionary.names[functions[f].body[i].args[1]], functions[f].body[i].args[1],
				dictionary.names[functions[f].body[i].args[2]], functions[f].body[i].args[2],
				dictionary.names[functions[f].body[i].args[3]], functions[f].body[i].args[3]
			);*/

			
			//const nat r  = functions[f].body[i].args[2];
			//const nat s  = functions[f].body[i].args[3];























