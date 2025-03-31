#include <stdio.h>
#include <stdbool.h>   // a programming language made by dwrr on 2306283.112548 
#include <iso646.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>

typedef uint64_t nat;
static const nat debug = 0;

#define compiler  "unnamed: "

#define red   	"\x1B[31m"
#define green   "\x1B[32m"
#define yellow  "\x1B[33m"
#define blue   	"\x1B[34m"
#define magenta "\x1B[35m"
#define cyan   	"\x1B[36m"
#define bold    "\033[1m"
#define reset 	"\x1B[0m"

enum thing_type { type_null, type_variable, type_label };

enum instruction_type {
	null_ins,
	sll,srl,sra,add,_xor,_and,_or,sub,
	mul,mhs,mhsu,_div,rem,divs,rems,
	blt,bge,blts,bges,bne,beq,jalr,jal,
	store1,store2,store4,store8,
	load1,load2,load4,load8,
	load1s,load2s,load4s,loadi,
	ecall,
	debugprint, debughex, 

	isa_count
};

static const nat arity[] = {
	0,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,
	3,3,3,3,3,3,2,2,
	2,2,2,2,
	2,2,2,2,
	2,2,2,2,
	7,
	1, 1
};

static const char* spelling[] = {
	"{nulli}",
	"sll","srl","sra","add","xor","and","or","sub",
	"mul","mhs","mhsu","div","rem","divs","rems",
	"blt","bge","blts","bges","bne","beq","jalr","jal",
	"store1","store2","store4","store8",
	"load1","load2","load4","load8",
	"load1s","load2s","load4s","loadi",
	"ecall",
	"debugprint","debughex"
};

static const char* ins_color[] = {
	"",
	red,red,red,green, red,red,red,green,
	blue,blue,blue,blue,blue,blue,blue,
	cyan,cyan,cyan,cyan,cyan,cyan,cyan,cyan,
	yellow,yellow,yellow,yellow,
	magenta,magenta,magenta,magenta,
	magenta,magenta,magenta,green,
	"",
	"","",
};

struct instruction {
	nat op;
	nat in[7];
	nat ph;
	nat defs[7];
	nat begin;
	nat end;
	nat ct;
	nat start;
};

struct word { 
	char* name; 
	nat length;
	nat type;
	nat value;
	nat def;
	nat start;
};

static const char digits[96] = 
	"0123456789abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ,."
	";:-_=+/?!@#$%^&*()<>[]{}|\\~`\'\"";

static const nat arm64_register_count = 31;
static const nat uninit = (nat) ~0;

static bool is_branch(nat b)    { return b >= blt and b <= jal; }

static const char* spell_type(nat t) { 
	if (t == type_null) return "{null_type}";
	if (t == type_label)  return cyan "label" reset;
	if (t == type_variable)  return green "variable" reset;
	return "unknown";
}

static void print_name(struct word w) {
	printf("\033[38;5;%dm", 67);
	for (nat _ = 0; _ < w.length; _++) putchar(w.name[_]);
	printf(reset);
}

static void print_word(struct word w) {
	print_name(w);
	printf("   \t: (%llu) { .type = %s, .val = %lld .def = %lld } \n", 
		w.length, spell_type(w.type), w.value, w.def
	);
}

static void print_nats(nat* array, nat count) {
	printf("{ ");
	for (nat i = 0; i < count; i++) {
		printf("%llu ", array[i]);
	}
	printf("}\n");
}

static void print_instruction(struct instruction ins, struct word* dict) {
	putchar(9);

	if (arity[ins.op]) print_name(dict[ins.in[0]]);
	printf(" = ");
	printf("%s%s%s ", ins_color[ins.op], spelling[ins.op], reset);

	printf("{");
	for (nat i = 1; i < arity[ins.op]; i++) {
		putchar(32);
		print_name(dict[ins.in[i]]); 
	}
	printf(" }\n\t\t\t\t\t\t\t"
		".ph = %lld .life = [%lld,%lld] .ct = %lld .defs=", 
		ins.ph, ins.begin, ins.end, ins.ct);

	print_nats(ins.defs, arity[ins.op]);
}

static void print_dictionary(struct word* dictionary, nat dictionary_count) {
	printf("dictionary { \n");
	for (nat i = 0; i < dictionary_count; i++) {
		printf("\t%3llu  :  " green, i);
		print_word(dictionary[i]);
	}
	printf("}\n");
}

static void print_instructions(struct instruction* instructions, nat ins_count, struct word* dictionary) {
	printf("instructions { \n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t%3llu  :  " , i);
		print_instruction(instructions[i], dictionary);
	}
	printf("}\n");
}

static bool is(const char* thing, char* word, nat count) {
	return count == strlen(thing) and not strncmp(word, thing, count);
}

static nat string_to_number(char* string, nat* length) {
	nat radix = 0, value = 0;
	nat result = 0, index = 0, place = 1;
begin:	if (index >= *length) goto done;
	value = 0;
top:	if (value >= 96) goto found;
	if (digits[value] == string[index]) goto found;
	value++;
	goto top;
found:	if (index) goto check;
	radix = value;
	goto next;
check:	if (value >= radix) goto done;
	result += place * value;
	place *= radix;
next:	index++;
	goto begin;
done:	*length = index;
	return result;
}

static void ins(nat op, nat* arguments, struct word* dictionary, struct instruction** instructions, nat* ins_count, nat start) {

	if (is_branch(op)) {
		if (	dictionary[*arguments].type == type_label 	and 
			dictionary[*arguments].value == *ins_count) 
			dictionary[*arguments].value = uninit;
	} else for (nat i = 0; i < arity[op]; i++) dictionary[arguments[i]].type = type_variable;

	struct instruction new = {
		.op = op,
		.in = {0},
		.ph = uninit,
		.begin = uninit, 
		.end = uninit, 
		.defs = {0},
		.ct = op == loadi,
		.start = start
	};
	memset(new.in, 0xff, 7 * sizeof(nat));
	memcpy(new.in, arguments, arity[op] * sizeof(nat));

	*instructions = realloc(*instructions, sizeof(struct instruction) * (*ins_count + 1));
	(*instructions)[(*ins_count)++] = new;
}

static void push_argument(nat argument, nat* arguments) {
	for (nat a = 31; a; a--) arguments[a] = arguments[a - 1];
	*arguments = argument;
}

static void process_syscall(nat a0, nat a1, nat a2, nat a3, nat a4, nat a5, nat n, nat* variables) {
	nat r0 = variables[a0], r1 = variables[a1], r2 = variables[a2], r3 = variables[a3], r4 = variables[a4], r5 = variables[a5];
	printf(green "SYSCALL (NR=%llu): {%llu %llu %llu %llu %llu %llu}" reset "\n" , n, r0, r1, r2, r3, r4, r5);

	if (n == 1) exit((int) r0);
	if (n == 2) variables[a0] = (nat) fork();
	if (n == 3) variables[a0] = (nat) read((int) r0, (void*) r1, r2);
	if (n == 4) variables[a0] = (nat) write((int) r0, (void*) r1, r2);
	if (n == 5) variables[a0] = (nat) open((const char*) r0, (int) r1, r2);
	if (n == 6) variables[a0] = (nat) close((int) r0);
	if (n == 7) variables[a0] = (nat) (void*) mmap((void*) r0, r1, (int) r2, (int) r3, (int) r4, (long long) r5);
	if (n == 8) variables[a0] = (nat) munmap((void*) r0, r1);
}

static void execute(struct instruction* instructions, nat ins_count, struct word* dictionary) {
	if (debug) print_instructions(instructions, ins_count, dictionary);

	static nat variables[4096] = {0};
	*variables = (nat)(void*) malloc(65536);

	for (nat ip = 0; ip < ins_count; ip++) {

		if (debug) printf("executing @%llu : ", ip);
		if (debug) print_instruction(instructions[ip], dictionary);
	
		const nat op  = instructions[ip].op;

		nat in[7] = {0};
		memcpy(in, instructions[ip].in, 7 * sizeof(nat));

		if (op == sll) {
			if (debug) printf("executed sll: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] << variables[in[2]];

		} else if (op == srl) {
			if (debug) printf("executed srl: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] >> variables[in[2]];

		} else if (op == sra) {
			if (debug) printf("executed sra: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] >> variables[in[2]];

		} else if (op == add) {
			if (debug) printf("executed add: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] + variables[in[2]];

		} else if (op == _xor) {
			if (debug) printf("executed xor: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] ^ variables[in[2]];

		} else if (op == _and) {
			if (debug) printf("executed and: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] & variables[in[2]];

		} else if (op == _or) {
			if (debug) printf("executed or: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] | variables[in[2]];

		} else if (op == sub) {
			if (debug) printf("executed sub: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] - variables[in[2]];

		} else if (op == mul) {
			if (debug) printf("executed mul: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] * variables[in[2]];

		} else if (op == mhs) {
			if (debug) printf("internal error: executed mhs: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] * variables[in[2]]; 
			abort();

		} else if (op == mhsu) {
			if (debug) printf("internal error: executed mhsu: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] * variables[in[2]]; 
			abort();

		} else if (op == _div) {
			if (debug) printf("executed div: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] / variables[in[2]]; 

		} else if (op == rem) {
			if (debug) printf("executed rem: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] % variables[in[2]]; 

		} else if (op == divs) {
			if (debug) printf("executed divs: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] / variables[in[2]]; 

		} else if (op == rems) {
			if (debug) printf("executed rems: %llu = %llu %llu\n", in[0], in[1], in[2]);
			variables[in[0]] = variables[in[1]] % variables[in[2]]; 
		}

		else if (op == blt) {
			if (debug) printf("executing blt -> @%llu [%llu %llu]\n", dictionary[in[0]].value, in[1], in[2]);
			if (dictionary[in[0]].value == (size_t) -1) { puts(red "internal error: unspecified label in branch" reset); goto halt; }
			if (variables[in[1]] < variables[in[2]]) ip = dictionary[in[0]].value - 1;

		} else if (op == bge) {
			if (debug) printf("executing bge -> @%llu [%llu %llu]\n", dictionary[in[0]].value, in[1], in[2]);
			if (dictionary[in[0]].value == (size_t) -1) { puts(red "internal error: unspecified label in branch" reset); goto halt; }
			if (variables[in[1]] >= variables[in[2]]) ip = dictionary[in[0]].value - 1;

		} else if (op == blts) {
			if (debug) printf("executing blts -> @%llu [%llu %llu]\n", dictionary[in[0]].value, in[1], in[2]);
			if (dictionary[in[0]].value == (size_t) -1) { puts(red "internal error: unspecified label in branch" reset); goto halt; }
			if (variables[in[1]] < variables[in[2]]) ip = dictionary[in[0]].value - 1;

		} else if (op == bges) {
			if (debug) printf("executing bges -> @%llu [%llu %llu]\n", dictionary[in[0]].value, in[1], in[2]);
			if (dictionary[in[0]].value == (size_t) -1) { puts(red "internal error: unspecified label in branch" reset); goto halt; }
			if (variables[in[1]] >= variables[in[2]]) ip = dictionary[in[0]].value - 1;

		} else if (op == bne) {
			if (debug) printf("executing bne -> @%llu [%llu %llu]\n", dictionary[in[0]].value, in[1], in[2]);
			if (dictionary[in[0]].value == (size_t) -1) { puts(red "internal error: unspecified label in branch" reset); goto halt; }
			if (variables[in[1]] != variables[in[2]]) ip = dictionary[in[0]].value - 1;

		} else if (op == beq) {
			if (debug) printf("executing beq -> @%llu [%llu %llu]\n", dictionary[in[0]].value, in[1], in[2]);
			if (dictionary[in[0]].value == (size_t) -1) { puts(red "internal error: unspecified label in branch" reset); goto halt; }
			if (variables[in[1]] == variables[in[2]]) ip = dictionary[in[0]].value - 1;
		
		} else if (op == jal) {
			if (debug) printf("executing jal -> @%llu (%llu) \n", in[0], dictionary[in[1]].value);
			if (dictionary[in[0]].value == (size_t) -1) { puts(red "internal error: unspecified label in branch" reset); goto halt; }
			variables[in[1]] = ip; ip = dictionary[in[0]].value - 1;
		
		} else if (op == jalr) {
			if (debug) printf("executing jalr (%llu) -> %llu]\n", in[0], in[1]);
			variables[in[0]] = ip; ip = variables[in[1]];
		}

		else if (op == store1) {
			if (debug) printf("executed store1: *(%llu) = %llu\n", in[0], in[1]);
			*(uint8_t*)variables[in[0]] = (uint8_t) variables[in[1]];

		} else if (op == store2) {
			if (debug) printf("executed store2: *(%llu) = %llu\n", in[0], in[1]);
			*(uint16_t*)variables[in[0]] = (uint16_t) variables[in[1]];

		} else if (op == store4) {
			if (debug) printf("executed store4: *(%llu) = %llu\n", in[0], in[1]);
			*(uint32_t*)variables[in[0]] = (uint32_t) variables[in[1]];

		} else if (op == store8) {
			if (debug) printf("executed store8: *(%llu) = %llu\n", in[0], in[1]);
			*(uint64_t*)variables[in[0]] = (uint64_t) variables[in[1]];

		} else if (op == load1) {
			if (debug) printf("executed load1: %llu = *%llu\n", in[0], in[1]);
			variables[in[0]] = (nat) *(uint8_t*)variables[in[1]];

		} else if (op == load2) {
			if (debug) printf("executed load2: %llu = *%llu\n", in[0], in[1]);
			variables[in[0]] = (nat) *(uint16_t*)variables[in[1]];

		} else if (op == load4) {
			if (debug) printf("executed load4: %llu = *%llu\n", in[0], in[1]);
			variables[in[0]] = (nat) *(uint32_t*)variables[in[1]];

		} else if (op == load8) {
			if (debug) printf("executed load8: %llu = *%llu\n", in[0], in[1]);
			variables[in[0]] = (nat) *(uint64_t*)variables[in[1]];

		} else if (op == load1s) {
			if (debug) printf("executed load1s: %llu = *%llu\n", in[0], in[1]);
			variables[in[0]] = (nat) *(int8_t*)variables[in[1]];

		} else if (op == load2s) {
			if (debug) printf("executed load2s: %llu = *%llu\n", in[0], in[1]);
			variables[in[0]] = (nat) *(int16_t*)variables[in[1]];

		} else if (op == load4s) {
			if (debug) printf("executed load4s: %llu = *%llu\n", in[0], in[1]);
			variables[in[0]] = (nat) *(int32_t*)variables[in[1]];

		} else if (op == loadi) {
			if (debug) printf("executing loadi %llu %llu]\n", in[0], in[1]);
			nat m = dictionary[in[1]].length;
			const nat n = string_to_number(dictionary[in[1]].name, &m);
			if (debug) { printf("in[1] constant = %llu (length = %llu)\n", n, m); }
			variables[in[0]] = n;

		} else if (op == ecall) {
			if (debug) printf("executed ecall: { [%llu, %llu, %llu, %llu, %llu, %llu, :: #%llu] }\n", 
					in[0], in[1], in[2], in[3], in[4], in[5], in[6]);

			process_syscall(in[0], in[1], in[2], in[3], in[4], in[5], in[6], variables);

		} else if (op == debugprint) {
			if (debug) printf("executed debugprint: %llu\n", in[0]);
			printf(green "debug: %llu" reset "\n" , variables[in[0]]);

		} else if (op == debughex) {
			if (debug) printf("executed hex: %llu\n", in[0]);
			printf(green "debug: %llx" reset "\n", variables[in[0]]);
			
		} else {
			printf("internal error: execute: unexpected instruction: %llu\n", op);
			abort();
		}
	}
halt: 	if (debug) puts(green "[finished execution]" reset);
}

static void parse(
	char* string, nat length, 
	struct instruction** out_instructions, nat *out_ins_count, 
	struct word** out_dictionary, nat* out_dictionary_count
) {

	struct word* dictionary = *out_dictionary;
	nat dictionary_count = *out_dictionary_count;

	struct instruction* instructions = NULL;
	nat ins_count = 0;
	nat arguments[32] = {0};
	nat count = 0, start = 0;
	bool comment = false;

	for (nat index = 0; index < length; index++) {
		if (not isspace(string[index])) { 
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;

		process_word:; char* word = string + start;
		if (is("note", word, count)) { comment = not comment; goto done; }
		if (comment) goto done;

		bool found = false;
		for (nat i = sll; i < isa_count; i++) {
			if (is(spelling[i], word, count)) {
				ins(i, arguments, dictionary, &instructions, &ins_count, index);
				found = true;
			}
		}

		if (not found) {
			for (nat d = 0; d < dictionary_count; d++) {
				if (dictionary[d].length != count or strncmp(dictionary[d].name, word, count)) continue;

				if (debug) printf("[DEFINED]    ");
				if (debug) print_word(dictionary[d]);

				push_argument(d, arguments);
				if (	dictionary[*arguments].type == type_label 	and
					dictionary[*arguments].value == uninit
				) 	dictionary[*arguments].value = ins_count;

				goto done;
			}

			push_argument(dictionary_count, arguments);
			dictionary = realloc(dictionary, sizeof(struct word) * (dictionary_count + 1));

			dictionary[dictionary_count++] = (struct word) {
				.name = strndup(word, count), 
				.length = count, 
				.type = type_label,
				.value = ins_count,
				.def = uninit,
				.start = start + count
			};

			if (debug) printf("[not defined]  -->  assuming  ");
			if (debug) print_word(dictionary[dictionary_count - 1]);
			goto done;
		}
		if (debug) { if (ins_count) print_instruction(instructions[ins_count - 1], dictionary); } 
		done: count = 0;
	}
	if (count) goto process_word;

	*out_dictionary = dictionary;
	*out_dictionary_count = dictionary_count;
	*out_instructions = instructions;
	*out_ins_count = ins_count;
}

static char* read_file(const char* filename, size_t* count) {
	FILE* file = fopen(filename, "r");
	if (not file) {
		fprintf(stderr, compiler bold red "error:" reset bold " ");
		perror(filename);
		fprintf(stderr, reset);
		exit(1);
	}

	fseek(file, 0, SEEK_END);
        *count = (size_t) ftell(file); 
	char* text = calloc(*count + 1, 1);
        fseek(file, 0, SEEK_SET); 
	fread(text, 1, *count, file);
	fclose(file); 

	if (debug) printf("info: file \"%s\": read %lu bytes\n", filename, *count);
	return text;
}

static void print_labels(nat* labels, nat label_count, struct word* dictionary) {
	printf("found %llu labels: {", label_count);
	for (nat i = 0; i < label_count; i++) {
		printf("#%llu: %s, ", i, dictionary[labels[i]].name);
	}
	puts("}");
}

static nat* find_labels(struct word* dictionary, nat dictionary_count, nat* out_label_count) {
	
	nat label_count = 0;
	nat* labels = malloc(4096 * sizeof(nat));

	for (nat d = 0; d < dictionary_count; d++) {
		if (dictionary[d].type == type_label) {
			if (debug) printf("found label: %s\n", dictionary[d].name);
			labels[label_count++] = d;
		}
	}
	*out_label_count = label_count;
	return labels;
}

static nat* find_ecalls(struct instruction* instructions, nat ins_count, nat* out_ecall_count) {
	
	nat ecall_count = 0;
	nat* ecalls = malloc(4096 * sizeof(nat));

	for (nat i = 0; i < ins_count; i++) {
		if (instructions[i].op == ecall) {
			if (debug) printf("found ecall: %llu\n", i);
			ecalls[ecall_count++] = i;
		}
	}
	*out_ecall_count = ecall_count;
	return ecalls;
}

static void find_lifetimes(struct instruction* instructions, nat ins_count, struct word* dictionary, nat dictionary_count) {
	if (debug) printf("RA: obtaining lifetime information...\n");

	for (nat i = 0; i < ins_count; i++) {

		if (debug) puts("");
		if (debug) printf("looking at: \n\t#%llu:", i);
		if (debug) print_instruction(instructions[i], dictionary);

		const nat op = instructions[i].op;

		if (not arity[op]) continue;

		for (nat j = 0; j < arity[op]; j++) {
			if (op == loadi and j == 1) break;

			const nat this = instructions[i].in[j];
			const nat def = dictionary[this].def;
			instructions[i].defs[j] = def;

			if (not j and def == uninit) instructions[i].defs[j] = i;
			if (not j) continue;

			if (def == uninit) { 
				printf(bold "%s:%llu:%llu: " red "error: " reset bold 
					"use of uninitialized variable \"%s\" in instruction: " reset "\n", 				

					"filename.txt",
					instructions[i].start,
					dictionary[this].start,
					dictionary[this].name
				);
				print_instruction(instructions[i], dictionary);

			} else {
				instructions[def].end = i;
				if (debug) printf("encountered use of " magenta "%s" reset 
					", \n using definition at ins #%llu: ", dictionary[this].name, def);
				if (debug) print_instruction(instructions[def], dictionary);
			}
		}

		if (not is_branch(op)) { 
			const nat dest = instructions[i].in[0];
			instructions[i].begin = i;
			dictionary[dest].def = i;
			if (debug) printf("found definition! " magenta "%s" reset " is being defined at ins #%llu...\n", dictionary[dest].name, i);
		} 

		if (debug) printf("generated lifetime of: \n\t#%llu:", i);
		if (debug) print_instruction(instructions[i], dictionary);

		if (debug) printf("debugging dict: \n");
		if (debug) print_dictionary(dictionary, dictionary_count);
		if (debug) printf("-------------------\n");
	}

	if (debug) printf("printing lifetimes...\n");
	if (debug) print_instructions(instructions, ins_count, dictionary);

	for (nat i = 0; i < ins_count; i++) {
		if (dictionary[instructions[i].in[0]].type == type_label) continue;
		if (instructions[i].end == uninit and instructions[i].begin != uninit) {

			printf(bold "%s:%llu:%llu: " magenta "warning: " reset bold 
				"result \"%s\" unused in instruction: " reset "\n", 

				"filename.txt",
				instructions[i].start, 
				dictionary[instructions[i].in[0]].start, 
				dictionary[instructions[i].in[0]].name
			);
			print_instruction(instructions[i], dictionary);
		}
	}
}

static nat find_available(nat* array, nat count) {
	for (nat i = 0; i < count; i++) {
		if (not array[i]) return i;
	}
	return count;
}

static void assign_registers(struct instruction* instructions, nat ins_count, struct word* dictionary) {
	if (debug) printf("RA: performing register allocation...\n");

	const nat live_count = arm64_register_count;
	nat* live = calloc(live_count, sizeof(nat));
	
	for (nat i = 0; i < ins_count; i++) {
		if (instructions[i].ph != uninit) continue;
		memset(live, 0, live_count * sizeof(nat));
		for (nat i1 = 0; i1 < ins_count; i1++) {
			bool conflict = 0;
			if (instructions[i].end > instructions[i1].begin and instructions[i1].end > instructions[i].begin) {
				conflict = 1;
			}
			if (instructions[i1].ph != uninit) live[instructions[i1].ph] = conflict;
		}
		const nat spot = find_available(live, live_count);
		if (spot == live_count) { puts("internal error: ran out of regs! (fix me!)"); abort(); }
		instructions[i].ph = spot;
	}
	if (debug) printf("printing assignments...\n");
	if (debug) print_instructions(instructions, ins_count, dictionary);
}

static void generate_operation(
	const char* op_string, 
	FILE* file,
	struct instruction ins,
	struct instruction* instructions, 
	struct word* dictionary
) {
	if (debug) print_instruction(ins, dictionary);
	fprintf(file, "\t%s ", op_string);

	fprintf(file, "x%llu", ins.ph); fprintf(file, ", ");
	fprintf(file, "x%llu", instructions[ins.defs[1]].ph); fprintf(file, ", ");
	fprintf(file, "x%llu", instructions[ins.defs[2]].ph); fprintf(file, "\n");	
}

static void generate_branch(
	const char* condition_string, 
	FILE* file,
	struct instruction ins,
	struct instruction* instructions, 
	struct word* dictionary
) {
	const char* label = dictionary[ins.in[0]].name;
	if (dictionary[ins.in[0]].value == uninit) {
		puts("internal error: branch value was uninitialized: unknown branch...?");
		abort();
	}
	const nat x = instructions[ins.defs[1]].ph;
	const nat y = instructions[ins.defs[2]].ph;
	fprintf(file, "\tcmp x%llu, x%llu\n", x, y);
	fprintf(file, "\tb.%s L_%s\n", condition_string, label);
}

static void generate_loadi(
	FILE* file, 
	struct instruction ins,
	struct word* dictionary
) {
	const nat r = ins.ph;
	if (r > 31) {
		printf("internal error: generate_loadi: bad destination register\n");
		abort();
	}
	nat m = dictionary[ins.in[1]].length;
	const nat n = string_to_number(dictionary[ins.in[1]].name, &m);
	if (debug) { printf("in[1] constant = %llu (length = %llu)\n", n, m); }

	uint64_t raw = n;
	uint16_t imm0 = (uint16_t) raw;
	uint16_t imm16 = (uint16_t) (raw >> 16);
	uint16_t imm32 = (uint16_t) (raw >> 32);
	uint16_t imm48 = (uint16_t) (raw >> 48);

	// if (not imm0) return;                   ///TODO: use the zero register instead of the constant zero, when possible.

	fprintf(file, "\tmovz ");
	fprintf(file, "x%llu", r);
	fprintf(file, ", 0x%hx\n", (uint16_t) imm0);

	if (not imm16) return;
	fprintf(file, "\tmovk ");
	fprintf(file, "x%llu", r);
	fprintf(file, ", 0x%hx, lsl 16\n", imm16);

	if (not imm32) return;
	fprintf(file, "\tmovk ");
	fprintf(file, "x%llu", r);
	fprintf(file, ", 0x%hx, lsl 32\n", imm32);

	if (not imm48) return;
	fprintf(file, "\tmovk ");
	fprintf(file, "x%llu", r);
	fprintf(file, ", 0x%hx, lsl 48\n", imm48);
}

static void generate_assembly(
	struct instruction* instructions, nat ins_count, 
	struct word* dictionary, nat dictionary_count, 
	nat* labels, nat label_count
) {
	if (debug) printf("generating asm file...\n");
	
	if (not dictionary_count) {
		printf(compiler bold red "error: " reset bold "missing entry point label" reset "\n");
		exit(1);
	}

	FILE* file = fopen("asm_output.s", "w");
	fprintf(file, "\
	.section __TEXT,__text,regular,pure_instructions\n\
	.build_version macos, 13, 0 sdk_version 13, 3\n\
	.globl %s\n\
	.p2align 2\n", dictionary->name);
	
	for (nat i = 0; i < ins_count; i++) {
		
		if (debug) printf("generate_assembly: generating this \n");
		if (debug) print_instruction(instructions[i], dictionary);

		for (nat l = 0; l < label_count; l++) {
			if (dictionary[labels[l]].value == i) { 
				if (debug) printf("generating label for %s at %llu...\n", dictionary[labels[l]].name, i);
				if (labels[l]) fprintf(file, "L_%s:\n", dictionary[labels[l]].name);
				else fprintf(file, "%s:\n", dictionary[labels[l]].name);
			}
		}

		const nat op  = instructions[i].op;
		nat in[7] = {0};
		memcpy(in, instructions[i].in, 7 * sizeof(nat));

		if (not op) abort();
		else if (op == ecall) fprintf(file, "\tsvc 0x80\n");
		else if (op == loadi) generate_loadi(file, instructions[i], dictionary);

		else if (op == add)   generate_operation("add", file, instructions[i], instructions, dictionary);
		else if (op == _or)   generate_operation("orr", file, instructions[i], instructions, dictionary);
		else if (op == _xor)  generate_operation("eor", file, instructions[i], instructions, dictionary);
		else if (op == _and)  generate_operation("and", file, instructions[i], instructions, dictionary);
		else if (op == sub)   generate_operation("sub", file, instructions[i], instructions, dictionary);
		else if (op == mul)   generate_operation("mul", file, instructions[i], instructions, dictionary);		

		else if (op == bne)   generate_branch(    "ne", file, instructions[i], instructions, dictionary);
		else if (op == beq)   generate_branch(    "eq", file, instructions[i], instructions, dictionary);
		else if (op == blt)   generate_branch(    "lo", file, instructions[i], instructions, dictionary);
		else if (op == bge)   generate_branch(    "hs", file, instructions[i], instructions, dictionary);
		else if (op == blts)  generate_branch(    "lt", file, instructions[i], instructions, dictionary);
		else if (op == bges)  generate_branch(    "ge", file, instructions[i], instructions, dictionary);
	
		else {
			printf("internal error: unknown instruction to generate: %s : %llu\n", spelling[op], op);
			abort();
		}
	}

	fprintf(file, "\tmov x0, #37\n");
	fprintf(file, "\tmov x16, #%u\n", SYS_exit);
	fprintf(file, "\tsvc 0x80 ; temporary auto-generated exit syscall\n");
	fprintf(file, ".subsections_via_symbols\n");
	fclose(file);
}

static void assign_ecall_registers(
	struct instruction* instructions, nat ins_count, 
	struct word* dictionary,
	nat* ecalls, nat ecall_count
) {
	const nat system_call_registers[] = {0, 1, 2, 3, 4, 5, 16};

	for (nat i = 0; i < ecall_count; i++) {
		for (nat j = 0; j < arity[ecall]; j++) {
			if (instructions[instructions[ecalls[i]].defs[j]].ph != uninit) {
				puts("internal error: conflict in assigning registers! already assigned reg must be moved...\n");
				abort();

			} else  instructions[instructions[ecalls[i]].defs[j]].ph = system_call_registers[j];
		}
	}
	if (debug) printf("printing results from ecall reg assignments: \n");
	if (debug) print_instructions(instructions, ins_count, dictionary);
}

static void print_ecalls(nat* ecalls, nat ecall_count, struct instruction* instructions, struct word* dictionary) {
	printf("printing list of ecalls found: \n");
	for (nat i = 0; i < ecall_count; i++) {
		printf("ecall #%llu   :  ", i);
		print_instruction(instructions[ecalls[i]], dictionary);
	}
	printf("[end of ecall list]\n");
}

static void compile(char* text, const size_t count, const char* executable_name) {

	if (debug) printf("compile: text = \"%s\"\n", text);
	nat ins_count = 0;
	struct instruction* instructions = NULL;
	nat dictionary_count = 0;
	struct word* dictionary = NULL;

	parse(text, count, &instructions, &ins_count, &dictionary, &dictionary_count);

	if (debug) print_instructions(instructions, ins_count, dictionary);
	if (debug) print_dictionary(dictionary, dictionary_count);

	nat label_count = 0;
	nat* labels = find_labels(dictionary, dictionary_count, &label_count);
	if (debug) print_labels(labels, label_count, dictionary);

	find_lifetimes(instructions, ins_count, dictionary, dictionary_count);

	nat ecall_count = 0;
	nat* ecalls = find_ecalls(instructions, ins_count, &ecall_count);
	if (debug) print_ecalls(ecalls, ecall_count, instructions, dictionary);
	
	assign_ecall_registers(instructions, ins_count, dictionary, ecalls, ecall_count);
	assign_registers(instructions, ins_count, dictionary);

	generate_assembly(instructions, ins_count, dictionary, dictionary_count, labels, label_count);
	if (debug) system("cat asm_output.s");

	const char* assembler_string = "as -v asm_output.s -o object_output.o";
	char linker_string[4096] = {0};
	snprintf(linker_string, sizeof linker_string, 
		"/Library/Developer/CommandLineTools/usr/bin/ld -v "
		"-demangle "
		"-lto_library /Library/Developer/CommandLineTools/usr/lib/libLTO.dylib "
		"-dynamic -arch arm64 -platform_version macos 13.0.0 13.3 "
		"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk "
		"-e %s "
		"-o %s "
		"-L/usr/local/lib "
		"object_output.o "
		"-lSystem /Library/Developer/CommandLineTools/usr/lib/clang/14.0.3/lib/darwin/libclang_rt.osx.a"
		, dictionary->name, executable_name
	);

	if (debug) puts(assembler_string);
	system(assembler_string);
	if (debug) system("objdump -D object_output.o");
	if (debug) puts(linker_string);
	system(linker_string);
}

static void configure_terminal(void) {
	struct termios terminal = {0}; 
	tcgetattr(0, &terminal);
	struct termios copy = terminal; 
	copy.c_lflag &= ~((size_t)ICANON | ECHO);
	tcsetattr(0, TCSAFLUSH, &copy);
}

static char* get_string(const char* prompt, nat* out_length) {

	struct winsize window = {0};

	char* input = NULL;
	uint8_t* tabs = NULL;
	uint16_t* newlines = NULL;

	size_t len = 0, capacity = 0, tab_count = 0, newline_count = 0;
	uint16_t column = (uint16_t) strlen(prompt);
	char w = 0, p = 0, d = 0;

	ioctl(0, TIOCGWINSZ, &window);

	write(1, prompt, column);

read:	read(0, &w, 1);
	if (w == 27) goto next;
	if (w == 127) goto delete;

	if (w != 't' or p != 'r' or d != 'd') goto push;
	if (len >= 2) len -= 2;
	input[len] = 0;
	*out_length = len;
	return input;

push:	if (w == 10) {
		newlines = realloc(newlines, sizeof(uint16_t) * (newline_count + 1));
		newlines[newline_count++] = column;
		column = 0;
		write(1, &w, 1);

	} else if (w == 9) {
		const uint8_t amount = 8 - column % 8;
		column += amount; column %= window.ws_col;
		write(1, "        ", amount);
		tabs = realloc(tabs, tab_count + 1);
		tabs[tab_count++] = amount;
	} else {
		if (column >= window.ws_col) column = 0;
		if((unsigned char) w >> 6 != 2) column++;
		write(1, &w, 1);
	}
	if (len + 1 >= capacity) {
		capacity = 4 * (capacity + 1);
		input = realloc(input, capacity);	
	}
	input[len++] = w;
	goto next;

delete: if (not len) goto next;
	len--;
	if (input[len] == 10) {
		column = newlines[--newline_count];
		printf("\033[A");
		if (column) printf("\033[%huC", column);
		fflush(stdout);

	} else if (input[len] == 9) {
		uint8_t amount = tabs[--tab_count];
		column -= amount;
		write(1, "\b\b\b\b\b\b\b\b", amount);

	} else {
		while ((unsigned char) input[len] >> 6 == 2) len--;
		if (not column) {
			column = window.ws_col - 1;
			write(1, "\b", 1);
		} else {
			column--;
			write(1, "\b \b", 3);
		}
	}

next:	d = p; 
	p = w; 
	goto read;
}

static _Noreturn void repl(void) {
	
	const char* welcome_string = 

		bold 
		"{Unnamed Language} 0.0.1 2306283.211605 interpreter"   "\n"
		"   Type \"helpdrt\" for more information." 
		reset;

	const char* help_string = 

		bold 
		"Help menu: " "\n"
		"\t  " "(terminate all commands with \"drt\")" "\n"
		"\t. " magenta "help: " reset "(you are here.)" "\n"
		"\t. " cyan "quit: " reset "quit the interpreter." "\n"
		"\t. " blue "dictionary: " reset "display the previous execution's dictionary." "\n"
		"\t. " green "instructions: " reset "display the previous execution's instructions." "\n"
		"\t. " red "reset: " reset "reset the instructions and dictionary to be empty." "\n"
		"\t. " yellow "{expression}: " reset "self-contained code written in the language that will be run." "\n"
		reset;

	puts(welcome_string);
	configure_terminal();

	nat ins_count = 0;
	struct instruction* instructions = NULL;
	nat dictionary_count = 0;
	struct word* dictionary = NULL;

loop:;
	nat len = 0;
	char* input = get_string(":ready: ", &len);
	puts("");
	if (debug) printf("\n\trecieved input(%llu): \n\n\t\t\"%s\"\n", len, input);

	if (not strcmp(input, "q") or not strcmp(input, "quit")) exit(0);
	else if (not strcmp(input, "o") or not strcmp(input, "clear")) printf("\033[H\033[2J");
	else if (not strcmp(input, "help")) puts(help_string);
	else if (not strcmp(input, "dictionary")) print_dictionary(dictionary, dictionary_count);
	else if (not strcmp(input, "instructions")) print_instructions(instructions, ins_count, dictionary);
	else if (not strcmp(input, "reset")) {dictionary_count = 0; ins_count = 0;}
	else {
		parse(input, len, &instructions, &ins_count, &dictionary, &dictionary_count);
		execute(instructions, ins_count, dictionary); 
	}
	goto loop; 
}

int main(int argc, const char** argv) {
	if (argc <= 1) repl();
	const char* filename = argv[1];
	size_t count = 0;
	char* contents = read_file(filename, &count);
	compile(contents, count, "executable_program.out");
}



























































































































/*
movz x0, 0x7788
movk x0, 0x5566, lsl 16
movk x0, 0x3344, lsl 32
movk x0, 0x1122, lsl 48
*/













/*struct decision {
	nat instruction;
	nat physical_register;
	nat virtual_register;
};*/
/*struct machine_instruction {
	nat opcode;
	nat arg0;
	nat arg1;
	nat arg2;
	nat last_use;
};*/













/*
	for (nat ip = 0, pi = ins_count - 1; ip < ins_count; ip++, pi--) {
		
		if (	instructions[ip].in[1] and arity[instructions[ip].op] >= 2 and 
			instructions[ip].op != loadi
		) {
			struct word* r = dictionary + instructions[ip].in[1];
			if (r->begin == uninit) {
				printf(red "ERROR: usage of uninitialized variable named \"%s\" is undefined behavior." reset "\n", r->name);
				printf("at instruction: ");
				print_instruction(instructions[ip], dictionary);
				puts("");
				abort(); 
			}	
		}

		if (instructions[ip].in[2] and arity[instructions[ip].op] >= 3) {
			struct word* r = dictionary + instructions[ip].in[2];
			if (r->begin == uninit) {
				printf(red "ERROR: usage of uninitialized variable named \"%s\" is undefined behavior." reset "\n", r->name);
				printf("at instruction: ");
				print_instruction(instructions[ip], dictionary);
				puts("");
				abort(); 
			}	
		}

		if (instructions[ip].in[0] and arity[instructions[ip].op] >= 1) {
			struct word* r = dictionary + instructions[ip].in[0];
			if (r->begin == uninit) { r->begin = ip; }
		}

		if (instructions[pi].in[1] and arity[instructions[ip].op] >= 2) {
			struct word* r = dictionary + instructions[pi].in[1];
			if (r->end == uninit) { r->end = pi; }
		} 

		if (instructions[pi].in[2] and arity[instructions[ip].op] >= 3) {
			struct word* r = dictionary + instructions[pi].in[2];
			if (r->end == uninit) { r->end = pi; }
		}
	}



	for (nat i = 1; i < dictionary_count; i++) {
		if (dictionary[i].type != type_variable) continue;
		if (dictionary[i].end == uninit) 
			printf(magenta "WARNING: found variable which was set but never used: \"%s\"" reset "\n", dictionary[i].name);
	}






// dictionary[(*dictionary_count)++] = (struct word) {.name = strndup("debug_memory", 12), .length = 12, .value = (size_t) type_variable};

	//.ph = uninit, 
	//	.begin = uninit, 
	//	.end = uninit





*/











/*
		for (nat d = 1; d < dictionary_count; d++) {
			if (dictionary[d].type != type_variable) {
				printf("skipping over %s...\n", dictionary[d].name);
				continue;
			}
			
			struct word* r = dictionary + d;	

			printf("processing word:  \"%s\"\n", r->name);

			if (r->begin == ip) {
				if (next >= 31) printf("next = %lld\n", next);
				if ((ssize_t) next == -1) {
					printf(red "ERROR: found negative   next available register   for %s" reset "\n", r->name);
					abort();
				} else {
					live[next] = d;
					r->ph = next;
				}

				printf("FOUND START OF LIFETIME: r%lld now holds the value for \"%s\"\n", next, r->name);
				next++;
			}

			
			if (r->end == ip and instructions[ip].op != loadi) {
				next = r->ph;
				printf("FOUND END OF LIFETIME: r%lld is now open.\n", next);
			}
		}

*/























/*
	nat number = dictionary[ins.in[0]].ph;
	nat arg0 = dictionary[ins.in[1]].ph;
	nat arg1 = dictionary[ins.in[2]].ph;
	nat arg2 = dictionary[ins.in[3]].ph;

	nat free16 = find_first_free(live);
	nat free0  = find_first_free(live);
	nat free1  = find_first_free(live);
	nat free2  = find_first_free(live);

	printf("INFO: found frees[f16=%llu, f0=%llu, f1=%llu, f2=%llu]\n", free16, free0, free1, free2);
	printf("INFO: found arguments[num=%llu, a0=%llu, a1=%llu, a2=%llu]\n", number, arg0, arg1, arg2);

// save:
	if (live[16]) fprintf(file, "\tadd x%llu, x%llu, xzr\n", free16, 16LLU); 
	if (live[0])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", free0, 0LLU); 
	if (live[1])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", free1, 1LLU);
	if (live[2])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", free2, 2LLU);

// fill in inputs
	if (number != 16) fprintf(file, "\tadd x%llu, x%llu, xzr\n", 16LLU, number); 
	if (arg0 != 0)    fprintf(file, "\tadd x%llu, x%llu, xzr\n", 0LLU, arg0);
	if (arg1 != 1)    fprintf(file, "\tadd x%llu, x%llu, xzr\n", 1LLU, arg1);
	if (arg2 != 2)    fprintf(file, "\tadd x%llu, x%llu, xzr\n", 2LLU, arg2);

	fprintf(file, "\tsvc 0x80"); fprintf(file, "\n");

// fill in outputs
	if (arg0 != 0) fprintf(file, "\tadd x%llu, x%llu, xzr\n", arg0, 0LLU); 
	if (arg1 != 1) fprintf(file, "\tadd x%llu, x%llu, xzr\n", arg1, 1LLU); 

// restore:
	if (live[16]) fprintf(file, "\tadd x%llu, x%llu, xzr\n", 16LLU, free16);
	if (live[0])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", 0LLU, free0);
	if (live[1])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", 1LLU, free1);
	if (live[2])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", 2LLU, free2);

	live[free16] = 0;
	live[free0] = 0;
	live[free1] = 0;
	live[free2] = 0;







//	nat def_count = 0;
//	nat* defs = malloc(4096 * sizeof(nat));

//	nat last_count = 0;
//	nat* last_usages = malloc(4096 * sizeof(nat));
	
	puts("----------------------------forw------------------------------------------------------------------------");






	print_nats(defs, def_count);


	puts("---------------------------back-------------------------------------------------------------------------");
	
	for (nat j = ins_count; j--;) {
		puts("\n\n");
		puts("looking at: [backwards]");
		print_instruction(instructions[j], dictionary);
		const nat op = instructions[j].op;
	}

	puts("------------------------------end----------------------------------------------------------------------");
		nat found = is_in(j, last_usages, last_count);
		if (found == last_count) {

			printf("[backwards]  found last usage! pushing %llu...\n", j);
			last_usages[last_count++] = j;

			printf("[backwards]  --> lastusages:  ");
			print_nats(last_usages, last_count);
		} else {
			printf("[backwards]  not the last usage! last one is at: %llu...\n", last_usages[found]);
		}





		if (instructions[i].op == loadi) goto dpunrt;

		if (i == dictionary[in[1]].end) {
			const struct word w = dictionary[in[1]];
			if (w.type != type_variable) abort();

			printf("found the end of %s's lifetime! at %llu.\n", w.name, i);
			
			live[w.ph] = 0;
		}

		if (i == dictionary[in[2]].end) {

			const struct word w = dictionary[in[2]];

			if (w.type != type_variable) abort();

			printf("found the end of %s's lifetime! at %llu.\n", w.name, i);
			live[w.ph] = 0;
		}

	dpunrt: 
		if (i == dictionary[in[0]].begin) {
			const struct word w = dictionary[in[0]];

			if (w.type != type_variable) abort();

			printf("found the beginning of %s's lifetime! at %llu.\n", w.name, i);

			if (w.ph == uninit) abort();

			live[w.ph] = in[0];
		}



} else if (op == set) {
			if (debug) printf("executing loadm %llu]\n", in[0]);
			variables[in[0]] = stack_pointer;

		} else if (op == loadm) {
			if (debug) printf("executing storem %llu]\n", in[0]);
			stack_pointer = variables[in[0]];







static struct word* create_dictionary(nat* dictionary_count) {
	*dictionary_count = 0;
	struct word* dictionary = malloc(sizeof(struct word));
	dictionary[(*dictionary_count)++] = (struct word) {
		.name = strndup("sp", 2), 
		.length = 2, 
		.type = type_variable,
		.value = uninit, 
		.def = uninit
	};
	return dictionary;
}





*/


//  else fprintf(file, "xzr");














/*
	nat free16 = find_first_free(live);
	nat free0  = find_first_free(live);
	

	printf("INFO: found frees[f16=%llu, f0=%llu]\n", free16, free0);
	printf("INFO: found arguments[num=%llu, a0=%llu]\n", number, arg0);

// save:
	if (live[16]) fprintf(file, "\tadd x%llu, x%llu, xzr\n", free16, 16LLU); 
	if (live[0])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", free0, 0LLU); 

// fill in inputs
	if (number != 16) fprintf(file, "\tadd x%llu, x%llu, xzr\n", 16LLU, number); 
	if (arg0 != 0)    fprintf(file, "\tadd x%llu, x%llu, xzr\n", 0LLU, arg0);
	if (arg1 != 1)    fprintf(file, "\tadd x%llu, x%llu, xzr\n", 1LLU, arg1);
	if (arg2 != 2)    fprintf(file, "\tadd x%llu, x%llu, xzr\n", 2LLU, arg2);

	

// fill in outputs
	if (arg0 != 0) fprintf(file, "\tadd x%llu, x%llu, xzr\n", arg0, 0LLU); 
	if (arg1 != 1) fprintf(file, "\tadd x%llu, x%llu, xzr\n", arg1, 1LLU); 

// restore:
	if (live[16]) fprintf(file, "\tadd x%llu, x%llu, xzr\n", 16LLU, free16);
	if (live[0])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", 0LLU, free0);
	if (live[1])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", 1LLU, free1);
	if (live[2])  fprintf(file, "\tadd x%llu, x%llu, xzr\n", 2LLU, free2);

	live[free16] = 0;
	live[free0] = 0;
	live[free1] = 0;
	live[free2] = 0;

*/




/*
static nat find_first_free(nat* live) {
	for (nat i = 0; i < arm64_register_count; i++) {
		if (i < 6)   continue;
		if (i == 16) continue;
		if (live[i] == 0) {
			live[i] = uninit;
			return i;
		}
	}
	printf("ERROR: ran out of hardware registers to use find_first_free();\n");
	print_live(live, arm64_register_count);
	abort();
}




	printf("INFO: found named arguments[num=%s, a0=%s, a1=%s, a2=%s]\n", dictionary[ins.in[0]].name, dictionary[ins.in[1]].name);

	nat num =  instructions[dictionary[ins.in[0]].def].ph;
	nat arg0 = instructions[dictionary[ins.in[1]].def].ph;
*/











/*

, nat dictionary_count


			if (dictionary[d].type != type_variable) {
				printf("skipping over %s...\n", dictionary[d].name);
				continue;
			}
			
			struct word* r = dictionary + d;	

			printf("processing word:  \"%s\"\n", r->name);

			if (r->begin == ip) {
				if (next >= 31) printf("next = %lld\n", next);
				if ((ssize_t) next == -1) {
					printf(red "ERROR: found negative   next available register   for %s" reset "\n", r->name);
					abort();
				} else {
					live[next] = d;
					r->ph = next;
				}

				printf("FOUND START OF LIFETIME: r%lld now holds the value for \"%s\"\n", next, r->name);
				next++;
			}

			
			if (r->end == ip and instructions[ip].op != loadi) {
				next = r->ph;
				printf("FOUND END OF LIFETIME: r%lld is now open.\n", next);
			}
*/










/*
static void print_nats(nat* array, nat count) {
	printf("{ ");
	for (nat i = 0; i < count; i++) {
		printf("%llu ", array[i]);
	}
	printf("}\n");
}

static nat is_in(nat element, nat* array, nat count) {
	for (nat i = 0; i < count; i++) {
		if (array[i] == element) return i;
	}
	return count;
}
*/








/*
static void print_live(nat* live, nat count) {
	printf("printing live registers { ");
	for (nat i = 0; i < count; i++) {
		printf("%llu, ", live[i]);
	}
	puts("}");
}
*/







//printf("printing live registers...\n");
		//print_live(live, arm64_register_count);





///if (nr == 7) variables[a0] = (nat) (int) brk((void*) r0);





// nat next = 0;
	// nat* live = calloc(arm64_register_count, sizeof(nat));



/*
static const char* spell_ins(nat t) {         // delete me!!!!

	if (t == null_ins) return "{nulli}";
	if (t == debugprint) return "debugprint";
	if (t == debughex) return "debughex";

	if (t == sll)  return red "sll" reset;
	if (t == srl)  return red "srl" reset;
	if (t == sra)  return red "sra" reset;
	if (t == _xor) return red "xor" reset;
	if (t == _and) return red "and" reset;
	if (t == _or)  return red "or" reset;
	if (t == add)  return green "add" reset;
	if (t == sub)  return green "sub" reset;

	if (t == mul)  return blue "mul" reset;
	if (t == mhs)  return blue "mhs" reset;
	if (t == mhsu) return blue "mhsu" reset;
	if (t == _div) return blue "div" reset;
	if (t == rem)  return blue "rem" reset;
	if (t == divs) return blue "divs" reset;
	if (t == rems) return blue "rems" reset;

	if (t == blt)  return cyan "blt" reset;
	if (t == bge)  return cyan "bge" reset;
	if (t == blts) return cyan "blts" reset;
	if (t == bges) return cyan "bges" reset;
	if (t == bne)  return cyan "bne" reset;
	if (t == beq)  return cyan "beq" reset;
	if (t == jal)  return cyan "jal" reset;
	if (t == jalr) return cyan "jalr" reset;

	if (t == store1) return yellow "store1" reset;
	if (t == store2) return yellow "store2" reset;
	if (t == store4) return yellow "store4" reset;
	if (t == store8) return yellow "store8" reset;

	if (t == load1)  return magenta "load1" reset;
	if (t == load2)  return magenta "load2" reset;
	if (t == load4)  return magenta "load4" reset;
	if (t == load8)  return magenta "load8" reset;

	if (t == load1s) return magenta "load1s" reset;
	if (t == load2s) return magenta "load2s" reset;
	if (t == load4s) return magenta "load4s" reset;
	if (t == loadi)  return magenta "loadi" reset;

	if (t == ecall)  return "ecall" reset;
	if (t == ebreak) return "ebreak" reset;

	return "unknown";
}


*/









/*
for (nat j = 0; j < arity[op]; j++) {

				const nat this = instructions[i].in[j];
				const nat def = dictionary[this].def;
				instructions[i].defs[j] = def;


			//	if (not j and def == uninit) instructions[i].defs[j] = i;
			//	if (not j) continue;

				if (def == uninit) { 
					printf(yellow "warning: use of uninitialized variable \"%s\" in instruction: " 
						reset "\n", dictionary[this].name);
					print_instruction(instructions[i], dictionary);
					abort();

				} else {
					instructions[def].end = i;
					printf("encountered use of " magenta "%s" reset 
						", \n using definition at ins #%llu: ", dictionary[this].name, def);
					print_instruction(instructions[def], dictionary);
				}
			}

			const nat dest = instructions[i].in[1];
			instructions[i].begin = i;
			dictionary[dest].def = i;
			printf("found definition! " magenta "%s" reset " is being defined at ins #%llu...\n", dictionary[dest].name, i);


*/








/*

const nat ar = ;

		printf("looking at ecall call# reg : \n");
		print_instruction(instructions[instructions[ecalls[i]].defs[6]], dictionary);
		puts("");

		if (instructions[instructions[ecalls[i]].defs[6]].ct) puts("the ecall # is known at compiletime!!!");
		
		// eval(instructions[instructions[ecalls[i]].defs[6]]);



*/










// instructions[instructions[ecalls[i]].defs[0]]     evaluate_ct(instructions, ins_count);










	/*
		this code doesnt respect the existing   premade colorings/assignments to nodes   that have already been done. 

		beacause they occur later,  and thus, we don't see them, until its too late.  and theres already a conlict. 

				we need to loop over all the nodes and check if there would be a conflict with another lifetime, before generating one as a particular target. if that register doesnt work, we should try another one instead. repeat until we find one that doesnt conflict with ANY lifetime of any variable in the execution state. 


									...this is a      n^2 algorithm in the instruction count... 
													bad.

						hm



	

	
			lets try it, until we find something better lol. 

						why not. 

			




*/


/*
for (nat t = 0; t < live_count; t++) 
			if (live[t] == i) live[t] = 0;

		if (instructions[i].begin == i) {

			const nat spot = find_available(live, live_count);
			if (spot == live_count) { puts("ran out of regs!"); abort(); }

			live[spot] = instructions[i].end;
			if (instructions[i].ph == uninit) instructions[i].ph = spot; 
			else {
				puts("found an already filled instruction ph!!!");
				print_instruction(instructions[i], dictionary);
				sleep(1);
			}
		}
*/









/*
static void format_register(
	FILE* file, 
	nat def, 
	struct instruction* instructions, 
	struct word* dictionary
) {
	printf("format_register: generating register with def_ins_index of %lld...\n", def);
	print_instruction(instructions[def], dictionary);

	const nat r = instructions[def].ph;
	if (r == uninit) {
		puts(red "internal error: format_register: register index is uninitialized." reset);
		abort();
	}

	fprintf(file, "x%llu", r);
}

*/









//puts("");
			//printf("A: ");
			//print_instruction(instructions[i], dictionary);
			//printf("B: ");
			//print_instruction(instructions[i1], dictionary);








// static bool is_operation(nat b) { return b >= sll and b <= rems; }
// static bool is_loadstore(nat b) { return b >= store1 and b <= load4s; }




/*
static void evaluate_ct(struct instruction* instructions, nat ins_count) {
	for (nat i = 0; i < ins_count; i++) {
		const nat op = instructions[i].op;
		if (op == ecall) continue;
		for (nat j = 1; j < arity[op]; j++) {
			if (not instructions[instructions[i].defs[j]].ct) {
				goto not_ct;
			}
		}
		instructions[i].ct = 1;
		not_ct: continue;
	}
}
*/

