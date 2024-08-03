// 1202407302.211405  dwrr
// the parser for the programming language,
// the new version with function defs and riscv isa ish...
//
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
	zero, incr, decr, add, lt, def, ret, ar, lf,
	isa_count
};

static const char* ins_spelling[isa_count] = {
	"()",
	"zero", "incr", "decr", "add", "lt", "def", "ret", "ar", "lf", 
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
	nat args[4];
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
	if (i == ret) return 0; 
	if (i == incr or i == decr or i == zero or i == def or i == ar or i == lf) return 1;
	if (i == lt) return 3;
	return 2;
}

static void debug_instructions(struct instruction* ins, nat ins_count, struct dictionary d) {
	
	printf("instructions: (%llu count) \n", ins_count);

	for (nat i = 0; i < ins_count; i++) {

		printf("%5llu: %20s : %-5lld %20s : %-5lld %20s : %-5lld %20s : %-5lld\n", 
			i, 
			d.names[ins[i].args[0]], ins[i].args[0],
			d.names[ins[i].args[1]], ins[i].args[1],
			d.names[ins[i].args[2]], ins[i].args[2],
			d.names[ins[i].args[3]], ins[i].args[3]
		);
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

static void parse(const char* given_filename) {

	struct function* functions = NULL;
	nat function_count = 0;

	nat scope_count = 1;
	struct scope* scopes = calloc(1, sizeof(struct scope));

	scopes[0].list = calloc(2, sizeof(nat*));
	scopes[0].count = calloc(2, sizeof(nat));
	scopes[0].function = 0;

	struct dictionary d = {0};

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
		scopes[0].list[0][scopes[0].count[0]++] = d.count;

		d.names = realloc(d.names, sizeof(char*) * (d.count + 1));
		d.values = realloc(d.values, sizeof(nat) * (d.count + 1));
		d.names[d.count] = strdup(ins_spelling[i]);
		d.values[d.count++] = function_count - 1;
	}

	for (nat i = 0; i < builtin_count; i++) {
		scopes[0].list[1] = realloc(scopes[0].list[1], sizeof(nat) * (scopes[0].count[1] + 1));
		scopes[0].list[1][scopes[0].count[1]++] = d.count;

		d.names = realloc(d.names, sizeof(char*) * (d.count + 1));
		d.values = realloc(d.values, sizeof(nat) * (d.count + 1));
		d.names[d.count] = strdup(builtin_spelling[i]);
		d.values[d.count++] = 0;
	}


process_file:;

	nat word_length = 0, word_start = 0, in_args = 0, arg_count = 0;

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
		
		printf("%llu:%llu: @ word: %s..\n", word_start, word_length, word);

		nat name = -1;
		for (nat s = scope_count; s--;) {
			nat* list = scopes[s].list[in_args];
			nat count = scopes[s].count[in_args];
			for (nat i = 0; i < count; i++) {
				if (not strcmp(d.names[list[i]], word) and 
					in_args != d.values[list[i]]) {

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
			scopes[scope_count - 1].count[t]++] = d.count;

			d.names = realloc(d.names, sizeof(char*) * (d.count + 1));
			d.values = realloc(d.values, sizeof(nat) * (d.count + 1));
			d.names[d.count] = word;
			d.values[d.count++] = 0;
			name = d.count - 1;
		}
	found:
		if (not in_args) {

			functions[scopes[scope_count - 1].function].body = 
			realloc(functions[scopes[scope_count - 1].function].body
			, sizeof(struct instruction) * (functions[scopes[
			scope_count - 1].function].body_count + 1));
			functions[scopes[scope_count - 1].function].
			body[functions[scopes[scope_count - 1].
			function].body_count++] = (struct instruction) {0};

			in_args = 1;
		}

		functions[scopes[scope_count - 1].function].body[
		functions[scopes[scope_count - 1].function].body_count - 1]
		.args[arg_count++] = name;

		const nat op = functions[scopes[scope_count - 1].function]
		.body[functions[scopes[scope_count - 1].function]
		.body_count - 1].args[0];

		if (arg_count >= functions[d.values[op]].arity + 1) {

			if (op == lf) {
				functions[scopes[scope_count - 1].function].body_count--;
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
				scopes[scope_count - 1].count[t]++] = d.count;

				d.names = realloc(d.names, sizeof(char*) * (d.count + 1));
				d.values = realloc(d.values, sizeof(nat) * (d.count + 1));
				d.names[d.count] = word;
				d.values[d.count++] = 0;

				functions[function_count - 1].arguments = realloc(
				functions[function_count - 1].arguments, sizeof(nat) * (
				functions[function_count - 1].arity + 1));
				functions[function_count - 1].arguments[
				functions[function_count - 1].arity++] = d.count - 1;

				// debug_dictionary(d);
				// debug_functions(functions, function_count, d);
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
				scopes[scope_count - 1].count[t]++] = d.count;

				d.names = realloc(d.names, sizeof(char*) * (d.count + 1));
				d.values = realloc(d.values, sizeof(nat) * (d.count + 1));
				d.names[d.count] = word;
				d.values[d.count++] = function_count - 1;
				functions[scopes[scope_count - 1].function].body[
				functions[scopes[scope_count - 1].function].body_count - 1]
				.args[arg_count - 1] = d.count - 1;

				scopes = realloc(scopes, sizeof(struct scope) * (scope_count + 1));
				scopes[scope_count++] = (struct scope) {0};
				scopes[scope_count - 1].list = calloc(2, sizeof(nat*));
				scopes[scope_count - 1].count = calloc(2, sizeof(nat));
				scopes[scope_count - 1].function = function_count - 1;

			}
			in_args = 0; 
			arg_count = 0; 
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

	debug_dictionary(d);
	debug_functions(functions, function_count, d);
	debug_scopes(scopes, scope_count);

}


int main(int argc, const char** argv) {
	const char* root = argc == 2 ? argv[1] : "simple.s";
	parse(root);
}


// exit

















