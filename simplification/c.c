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
#include <stdint.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
/*	ISA:	

	w increment
	w setzero
	l systemcall
	w w l store
	w w l load
	w w l branch
	w w l jump
*/
typedef uint64_t nat;
static const nat debug = 1;

// #define compiler  "unnamed: "

#define lightblue "\033[38;5;67m"
#define red   	"\x1B[31m"
#define green   "\x1B[32m"
#define yellow  "\x1B[33m"
#define blue   	"\x1B[34m"
#define magenta "\x1B[35m"
#define cyan   	"\x1B[36m"
#define bold    "\033[1m"
#define reset 	"\x1B[0m"


enum thing_type { type_null, type_variable, type_label };

enum instruction_type { null, increment, setzero, branch, jump, store, load, systemcall, debughex, isa_count };
static const nat arity[] = { 0, 1, 1, 3, 3, 3, 3, 1, 1 };
static const char* ins_color[] = { "", green, red, cyan, blue, yellow, magenta, bold, "" };
static const char* spelling[] = {
	"null", "increment", "setzero", "branch", "jump", "store", "load", "systemcall", "debughex"
};

struct instruction {
	nat op;
	nat in[3];
	nat ph;
	nat defs[3];
	nat begin;
	nat end;
	nat ct;
	nat file_location;
};

struct word { 
	char* name; 
	nat length;
	nat type;
	nat value;
	nat def;
	nat file_location;
};

static const nat arm64_register_count = 31;
static const nat uninit = (nat) ~0;

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

static void ins(nat op, nat* arguments, struct word* dictionary, struct instruction** instructions, nat* ins_count, nat start) {

	if (op == branch or op == jump) {
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
		.file_location = start
	};
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

		if (op == increment) {
			if (debug) printf("executed increment: %llu\n", in[0]);
			variables[in[0]]++;

		} else if (op == setzero) {
			if (debug) printf("executed setzero: %llu \n", in[0]);
			variables[in[0]] = 0;

		} else if (op == branch) {
			if (debug) printf("executing blt -> @%llu [%llu %llu]\n", dictionary[in[0]].value, in[1], in[2]);
			if (dictionary[in[0]].value == (size_t) -1) { 
				puts(red "internal error: unspecified label in branch" reset); 
				goto halt; 
			}
			if (variables[in[1]] < variables[in[2]]) 
				ip = dictionary[in[0]].value - 1;


		} else if (op == jump) {
			if (debug) printf("executing jalr (%llu) -> %llu]\n", in[0], in[1]);
			if (debug) printf("executing jalr -> @%llu (%llu) \n", in[0], dictionary[in[1]].value);
			if (dictionary[in[0]].value == (size_t) -1) { 
				puts(red "internal error: unspecified label in branch" reset); 
				goto halt; 
			}
			variables[in[1]] = ip; 


			ip = dictionary[in[0]].value - 1;
		//or
			ip = variables[in[0]];


		} else if (op == store) {
			if (debug) printf("executed store: *(%llu) = %llu\n", in[0], in[1]);

			*(uint8_t*) variables[in[0]] = (uint8_t)  variables[in[1]];
			*(uint16_t*)variables[in[0]] = (uint16_t) variables[in[1]];
			*(uint32_t*)variables[in[0]] = (uint32_t) variables[in[1]];
			*(uint64_t*)variables[in[0]] = (uint64_t) variables[in[1]];


		} else if (op == load) {
			if (debug) printf("executed load: %llu = *%llu\n", in[0], in[1]);
			variables[in[0]] = (nat) *(uint8_t*)variables[in[1]];
			variables[in[0]] = (nat) *(uint16_t*)variables[in[1]];
			variables[in[0]] = (nat) *(uint32_t*)variables[in[1]];
			variables[in[0]] = (nat) *(uint64_t*)variables[in[1]];

		} else if (op == systemcall) {
			if (debug) printf("executed ecall: { #%llu] }\n", in[0]);
			process_syscall(in[0], 0,0,0,0,0,0, variables);

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
	struct word** out_dictionary, nat* out_dictionary_count,
	nat* arguments
) {

	struct word* dictionary = *out_dictionary;
	nat dictionary_count = *out_dictionary_count;
	struct instruction* instructions = *out_instructions;
	nat ins_count = *out_ins_count;

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
		for (nat i = null; i < isa_count; i++) {
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
				.file_location = start + count
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

static _Noreturn void repl(void) {
	
	const char* welcome_string = 
		"{Unnamed Language} 0.0.1 202307086.162752 interpreter"   "\n"
		"   Type \"help\" for more information.";

	const char* help_string = 
		"Help menu: " "\n"
		"\t. " bold magenta "help: " reset "(you are here.)" "\n"
		"\t. " bold cyan "quit: " reset "quit the interpreter." "\n"
		"\t. " bold yellow "undo: " reset "remove the last instruction." "\n"
		"\t. " bold blue "dictionary: " reset "display the current dictionary." "\n"
		"\t. " bold green "instructions: " reset "display the current instructions." "\n"
		"\t. " bold red "resetall: " reset "reset the instructions and dictionary to be empty." "\n"
		"\t. " bold "{expression}: " reset "a series of instructions in the language ISA." "\n"
		"ISA:" "\n"
		"\t     " lightblue "w " bold green "increment" reset "\n"
		"\t     " lightblue "w " bold red "setzero" reset "\n"
		"\t     " lightblue "l " reset bold "systemcall" reset "\n"
		"\t     " lightblue "w w l " bold yellow "store" reset "\n"
		"\t     " lightblue "w w l " bold magenta "load" reset "\n"
		"\t     " lightblue "w w l " bold cyan "branch" reset "\n"
		"\t     " lightblue "w w l " bold blue "jump" reset "\n"
		;
		

	puts(welcome_string);

	nat ins_count = 0;
	struct instruction* instructions = NULL;
	nat dictionary_count = 0;
	struct word* dictionary = NULL;
	nat arguments[32] = {0};

	char input[1024] = {0};

loop:
	printf(":%llu:%llu: ", dictionary_count, ins_count);
	fgets(input, sizeof input, stdin);
	nat len = strlen(input);
	input[--len] = 0;

	if (not strcmp(input, "q") or not strcmp(input, "quit")) exit(0);
	else if (not strcmp(input, "o") or not strcmp(input, "clear")) printf("\033[H\033[2J");
	else if (not strcmp(input, "help")) puts(help_string);
	else if (not strcmp(input, "undo")) {if (ins_count) ins_count--;}
	else if (not strcmp(input, "dictionary")) print_dictionary(dictionary, dictionary_count);
	else if (not strcmp(input, "instructions")) print_instructions(instructions, ins_count, dictionary);
	else if (not strcmp(input, "resetall")) { dictionary_count = 0; ins_count = 0; }
	else {
		parse(input, len, &instructions, &ins_count, &dictionary, &dictionary_count, arguments);
		execute(instructions, ins_count, dictionary); 
	}
	goto loop;
}

int main(int argc, const char** argv) {
	if (argc <= 1) repl(); else exit(1);
	
}




















/*

static void configure_terminal(void) {
	struct termios terminal = {0}; 
	tcgetattr(0, &terminal);
	struct termios copy = terminal; 
	copy.c_lflag &= ~((size_t)ICANON | ECHO);
	tcsetattr(0, TCSAFLUSH, &copy);
}




//configure_terminal();


	//nat len = 0;
	//char* input = get_string(, &len);
	//puts("");
	if (debug) printf("\n\trecieved input(%llu): \n\n\t\t\"%s\"\n", len, input);


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




*/









/*


	l systemcall

	w increment

	w settozero

	w w l store      

	w w l load       

	w w l branch 

	w w l jump   










	
	w increment
	w setzero
	l systemcall
	w w l store
	w w l load
	w w l branch
	w w l jump




*/






























