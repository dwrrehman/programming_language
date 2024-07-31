// 1202407302.211405  dwrr 
// the parser for the programming language,
// the new version with function defs and riscv isa ish...
//
/*
copyb insert ./build
copya do ,./run




rename todo:

add these names:


		incr

		decr

		zero


those are pretty important lol... so yeah. i think i want those names too.

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
	zero, incr, decr, add, def, ret, 
	isa_count
};

static const char* ins_spelling[isa_count] = {
	"_nullins_",
	"zero", "incr", "decr", "add", "def", "ret", 
};



enum language_builtins {
	nullvar,
	stackpointer, stacksize,
	builtin_count
};

static const char* builtin_spelling[builtin_count] = {
	"_nullvar_",
	"stackpointer", "stacksize",
};




struct instruction {
	nat args[4];
};

struct function {
	nat entry;
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
};


static nat arity(nat i) {
	if (i == ret) return 0; 
	if (i == incr or i == decr or i == zero or i == def) return 1;
	return 2;
}

static void debug_instructions(struct instruction* ins, nat ins_count) {
	
	printf("instructions: (%llu count) \n", ins_count);

	for (nat i = 0; i < ins_count; i++) {

		const char* name = "(user-defined-function)";
		if (ins[i].args[0] < isa_count) name = ins_spelling[ins[i].args[0]];

		printf("%llu: %s %lld %lld %lld\n", 
			i, name, 
			ins[i].args[1], ins[i].args[2], 
			ins[i].args[3]
		);
	}
	puts("done");
}


static void debug_dictionary(struct dictionary d) {
	printf("dictionary: (%llu count)\n", d.count);
	for (nat i = 0; i < d.count; i++) {
		printf("%llu: .name = %s, .value = %llu\n", i, d.names[i], d.values[i]);
	}
	puts("done");
}

static void debug_scopes(struct scope* scopes, nat scope_count) {
	printf("scope stack: (%llu count)\n", scope_count);
	for (nat i = 0; i < scope_count; i++) {
		printf("\tscope %llu: \n", i);
		for (nat t = 0; t < 2; t++) {
			printf("\t\t[%llu]: ", t);
			for (nat n = 0; n < scopes[i].count[t]; n++) 
				printf("%llu ", scopes[i].list[t][n]);
			puts("");
		}
		puts("");
	}
	puts("done");
}


static void parse(char* text, const nat text_length) {

	struct instruction* ins = NULL;
	nat ins_count = 0;

	struct function* functions = NULL;
	nat function_count = 0;

	nat scope_count = 1;
	struct scope* scopes = calloc(1, sizeof(struct scope));

	scopes[0].list = calloc(2, sizeof(nat*));
	scopes[0].count = calloc(2, sizeof(nat));

	struct dictionary d = {0};

	for (nat i = 0; i < isa_count; i++) {

		const nat a = arity(i);

		functions = realloc(functions, sizeof(struct function) * (function_count + 1));
		functions[function_count++] = (struct function) {
			.entry = d.count,
			.arity = a,
			.arguments = calloc(a, sizeof(nat)),
			.body = NULL
		};

		scopes[0].list[0] = realloc(scopes[0].list[0], sizeof(nat) * (scopes[0].count[0] + 1));
		scopes[0].list[0][scopes[0].count[0]++] = d.count;

		d.names = realloc(d.names, sizeof(char*) * (d.count + 1));
		d.values = realloc(d.values, sizeof(nat) * (d.count + 1));
		d.names[d.count] = strdup(ins_spelling[i]);
		d.values[d.count++] = 0;
	}

	for (nat i = 0; i < builtin_count; i++) {
		scopes[0].list[1] = realloc(scopes[0].list[1], sizeof(nat) * (scopes[0].count[1] + 1));
		scopes[0].list[1][scopes[0].count[1]++] = d.count;

		d.names = realloc(d.names, sizeof(char*) * (d.count + 1));
		d.values = realloc(d.values, sizeof(nat) * (d.count + 1));
		d.names[d.count] = strdup(builtin_spelling[i]);
		d.values[d.count++] = 1;
	}

	nat word_length = 0, word_start = 0, in_args = 0, arg_count = 0;

	for (nat index = 0; index < text_length; index++) {
		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++; 
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;
		char* word = strndup(text + word_start, word_length);
		
		printf("%llu:%llu: @ word: %s..\n", word_start, word_length, word);

		debug_instructions(ins, ins_count);
		debug_dictionary(d);
		debug_scopes(scopes, scope_count);

		nat name = -1;
		for (nat s = scope_count; s--;) {
			nat* list = scopes[s].list[in_args];
			nat count = scopes[s].count[in_args];
			for (nat i = 0; i < count; i++) {
				if (not strcmp(d.names[list[i]], word) and 
					in_args == d.values[list[i]]) {

					puts("found name!");
					puts(word);
					printf("in_args = %llu\n", in_args);
					name = list[i];
					goto found;
				}
			}
		}

		if (not in_args) {
			puts("unknown operation: ");
			puts(word);
			printf("in_args = %llu\n", in_args);

			debug_instructions(ins, ins_count);
			debug_dictionary(d);

			abort();

		} else {
			puts("defining a new name:");
			puts(word);
	
			if (ins[ins_count - 1].args[0] == def) goto found;
			const nat t = 1;
			scopes[scope_count - 1].list[t] = 
			realloc(scopes[scope_count - 1].list[t], 
			sizeof(nat) * (scopes[scope_count - 1].count[t] + 1));
			scopes[scope_count - 1].list[t][
			scopes[scope_count - 1].count[t]++] = d.count;

			d.names = realloc(d.names, sizeof(char*) * (d.count + 1));
			d.values = realloc(d.values, sizeof(nat) * (d.count + 1));
			d.names[d.count] = word;
			d.values[d.count++] = t;
			name = d.count - 1;
		}
	found:
		if (not in_args) {
			ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
			ins[ins_count++] = (struct instruction) {0};
			in_args = 1;
		}

		ins[ins_count - 1].args[arg_count++] = name;

		const nat op = ins[ins_count - 1].args[0];

		if (arg_count == functions[op].arity + 1) {

			if (op == ret) {
				puts("executing ret....");
				scope_count--;
			}

			/*if (op == ar) {
				puts("executing ar....");
			}*/

			if (op == def) {

				puts("EXECUTING DEF!!!");
				puts("defining a new name:");
				puts(word);
				
				functions = realloc(functions, sizeof(struct function) * (function_count + 1));
				functions[function_count++] = (struct function) {
					.entry = d.count
				};

				const nat t = 0;
				scopes[scope_count - 1].list[t] = 
				realloc(scopes[scope_count - 1].list[t], 
				sizeof(nat) * (scopes[scope_count - 1].count[t] + 1));
				scopes[scope_count - 1].list[t][
				scopes[scope_count - 1].count[t]++] = d.count;

				d.names = realloc(d.names, sizeof(char*) * (d.count + 1));
				d.values = realloc(d.values, sizeof(nat) * (d.count + 1));
				d.names[d.count] = word;
				d.values[d.count++] = t;
				ins[ins_count - 1].args[arg_count - 1] = d.count - 1;

				scopes = realloc(scopes, sizeof(struct scope) * (scope_count + 1));
				scopes[scope_count++] = (struct scope) {0};
				scopes[scope_count - 1].list = calloc(2, sizeof(nat*));
				scopes[scope_count - 1].count = calloc(2, sizeof(nat));

			}
			in_args = 0; 
			arg_count = 0; 
		}
		word_length = 0;
	}
}


int main(void) {

	int file = open("test.s", O_RDONLY);
	if (file < 0) { perror("open"); exit(1); }

	const nat text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(text_length + 1, 1);
	read(file, text, text_length);
	close(file);

	parse(text, text_length);
	puts("just parsed:");
	puts(text);
	printf("text_length = %llu\n", text_length);
}


// exit










