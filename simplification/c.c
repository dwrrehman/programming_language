#include <stdio.h>
#include <stdbool.h>   // a programming language made by dwrr on 2306283.112548 
#include <iso646.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>    /// candidate names for the language {   n3zqx2l,     2dcw9o0u4ny7ekz1,   }
#include <stdint.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
/*	ISA:	
	w increment              sdju0t9ah8vz5r26
	w setzero                2dcw9o0u4ny7ekz1
	w w l branch             kb8g4tl0fwu1dco3
	w w l store              er23nlvghmkzcpiw
	w w l load               z7m9hjko172w5n0f
	l systemcall             dzmto4xrku8n306i
*/
typedef uint64_t nat;
static const nat debug = 1;

#define compiler  "unnamed: "

#define lightblue "\033[38;5;67m"
#define red   	"\x1B[31m"
#define green   "\x1B[32m"
#define yellow  "\x1B[33m"
#define blue   	"\x1B[34m"
#define magenta "\x1B[35m"
#define cyan   	"\x1B[36m"
#define bold    "\033[1m"
#define reset 	"\x1B[0m"

enum thing_type { type_null, type_variable, type_label, type_macro };
enum instruction_type { null, increment, setzero, branch, store, load, systemcall, debughex, isa_count };
static const nat arity[isa_count] = { 0, 1, 1, 3, 3, 3, 1, 1 };
static const char* ins_color[isa_count] = { "", green, yellow, cyan, red, magenta, bold, "" };
static const char* spelling[isa_count] = {
"", 
	"sdju0t9ah8vz5r26", 
	"2dcw9o0u4ny7ekz1", 
	"kb8g4tl0fwu1dco3", 
	"er23nlvghmkzcpiw", 
	"z7m9hjko172w5n0f", 
	"dzmto4xrku8n306i", 
	"temporary-debug-print-hex"
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
	nat address;
};

// static const nat arm64_register_count = 31;
static const nat uninit = (nat) ~0;

static const char* spell_type(nat t) { 
	if (t == type_null) return "{null_type}";
	if (t == type_label)  return cyan "label" reset;
	if (t == type_variable)  return green "variable" reset;
	if (t == type_macro)  return yellow "macro" reset;
	return "unknown";
}

static void print_name(struct word w) {
	printf("\033[38;5;%dm", 67);
	for (nat _ = 0; _ < w.length; _++) putchar(w.name[_]);
	printf(reset);
}

static void print_word(struct word w) {
	print_name(w);
	printf("   \t: (%llu) { .type = %s, .val = %lld .def = %lld .fl = %lld .a = %lld } \n", 
		w.length, spell_type(w.type), w.value, w.def, w.file_location, w.address
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

static void ins(nat op, 
	nat* arguments, nat argument_count, 
	struct word* dictionary, 
	struct instruction** instructions, nat* ins_count, 
	nat file_location) {

	nat arg0 = argument_count > 0 ? arguments[argument_count - 1] : uninit;
	nat arg1 = argument_count > 1 ? arguments[argument_count - 2] : uninit;
	nat arg2 = argument_count > 2 ? arguments[argument_count - 3] : uninit;

	if (op == branch or op == load or op == store) {
		if (dictionary[arg0].type == type_label and dictionary[arg0].value == *ins_count) 
			dictionary[arg0].value = uninit;

		if (dictionary[arg0].type != type_label) {
			printf("bad br arg0 label :%llu\n", dictionary[arg0].file_location); getchar(); }
		if (dictionary[arg1].type != type_variable) { 
			printf("bad br arg1 var :%llu\n", dictionary[arg1].file_location); getchar(); }
		if (dictionary[arg2].type != type_variable) { 
			printf("bad br arg2 var :%llu\n", dictionary[arg2].file_location); getchar(); }

	} else if (op == setzero) dictionary[arg0].type = type_variable;
	else if (op == increment) {   
		if (dictionary[arg0].type != type_variable) { 
			printf("bad incr arg0 var :%llu\n", dictionary[arg0].file_location); getchar(); 
		} 
	} else if (op == systemcall) {  
		if (dictionary[arg0].type != type_label) { 
			printf("bad syscall arg0 label :%llu\n", dictionary[arg0].file_location); getchar(); 
		} 
	} 

	*instructions = realloc(*instructions, sizeof(struct instruction) * (*ins_count + 1));
	(*instructions)[(*ins_count)++] = (struct instruction) {
		.op = op,
		.in = {arg0, arg1, arg2},
		.ph = uninit,
		.begin = uninit, 
		.end = uninit, 
		.defs = {0},
		.file_location = file_location
	};
}

static void process_syscall(nat n, nat* variables) {
	nat r0 = variables[0], r1 = variables[1], r2 = variables[2], r3 = variables[3], r4 = variables[4], r5 = variables[5];
	printf(green "SYSCALL (NR=%llu): {%llu %llu %llu %llu %llu %llu}" reset "\n" , n, r0, r1, r2, r3, r4, r5);

	if (n == 1) exit((int) r0);
	if (n == 2) *variables = (nat) fork();
	if (n == 3) *variables = (nat) read((int) r0, (void*) r1, r2);
	if (n == 4) *variables = (nat) write((int) r0, (void*) r1, r2);
	if (n == 5) *variables = (nat) open((const char*) r0, (int) r1, r2);
	if (n == 6) *variables = (nat) close((int) r0);
	if (n == 7) *variables = (nat) (void*) mmap((void*) r0, r1, (int) r2, (int) r3, (int) r4, (long long) r5);
	if (n == 8) *variables = (nat) munmap((void*) r0, r1);
}

static void execute_directly(struct instruction* instructions, nat ins_count, struct word* dictionary) {
	if (debug) {
		printf("executing these instructions: \n");
		print_instructions(instructions, ins_count, dictionary);
	}

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
			process_syscall(in[0], variables);

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


static bool macro = false;

static nat previous_start = 0;
static nat delimiter_start = 0;
static nat start = 0;

static nat previous_count = 0;
static nat delimiter_count = 0;
static nat count = 0;

static nat stack_pointer = 0;

static nat stack[4096] = {0};
static nat return_start[4096] = {0};
static nat return_count[4096] = {0};

static void parse(
	char* string, nat length, nat starting_index,
	struct instruction** out_instructions, nat *out_ins_count, 
	struct word** out_dictionary, nat* out_dictionary_count,
	nat** out_arguments, nat* out_argument_count
) {
	struct word* dictionary = *out_dictionary;
	nat dictionary_count = *out_dictionary_count;
	struct instruction* instructions = *out_instructions;
	nat ins_count = *out_ins_count;
	nat* arguments = *out_arguments;
	nat argument_count = *out_argument_count;

	for (nat index = starting_index; index < length; index++) {
		if (not isspace(string[index])) { 
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;

		process_word:; 

		char* const word       = string + start;
		char* const delimiter  = string + delimiter_start;
		char* const previous   = string + previous_start;
		char* const returnw    = stack_pointer ? string + return_start[stack_pointer - 1] : NULL;

		if (macro) {
			if (count == delimiter_count and not memcmp(word, delimiter, count)) {
				if (debug) printf("MACRO: inside macro definition, found end of defintion! %s\n", strndup(delimiter, count));
				macro = 0; count = 0; delimiter_count = 0;
			} goto next;

		} else if (is("fmgx6srl95ywtuan", word, count)) { argument_count--; goto next; }
		  else if (is("6drwb5t2epv1ax4k", word, count)) { count = 0; goto push_new; }

		  else if (is("hnav2gtf4bixkyuc", word, count)) { arguments[argument_count] = arguments[argument_count - 1]; argument_count++; goto next; }
		  else if (is("nze1942qpht7dmcg", word, count)) { arguments[argument_count] = arguments[argument_count - 2]; argument_count++; goto next; }
		  else if (is("3ba4te1ulnh26g0v", word, count)) { arguments[argument_count] = arguments[argument_count - 3]; argument_count++; goto next; }
		  else if (is("rfph6jaw3diels2m", word, count)) { arguments[argument_count] = arguments[argument_count - 4]; argument_count++; goto next; }
		  else if (is("n3oehasx4rv5iz06", word, count)) { arguments[argument_count] = arguments[argument_count - 5]; argument_count++; goto next; }
		  else if (is("wgxzcp5o81yinebd", word, count)) { arguments[argument_count] = arguments[argument_count - 6]; argument_count++; goto next; }
		
		for (nat i = null; i < isa_count; i++) {
			if (is(spelling[i], word, count)) {
				ins(i, arguments, argument_count, dictionary, &instructions, &ins_count, index);
				if (debug) { if (ins_count) print_instruction(instructions[ins_count - 1], dictionary); } 
				goto next;
			}
		}

		if (stack_pointer and count == return_count[stack_pointer - 1] and not memcmp(word, returnw, count)) { 
			if (debug) printf("MACRO: executing return statement for macro! %s\n", strndup(returnw, count));
			index = stack[--stack_pointer]; 
			count = 0; 
			goto next;
		}

		else if (count == previous_count and not memcmp(word, previous, count)) {
			if (debug) printf("MACRO: encountered macro definition! %s\n", strndup(previous, count));
			dictionary[arguments[argument_count - 1]].type = type_macro;
			dictionary[arguments[argument_count - 1]].address = index;
			macro = 1;
			delimiter_start = start;
			delimiter_count = count; 
			goto next;

		} 

		for (nat d = 0; d < dictionary_count; d++) {
			if (dictionary[d].length != count or strncmp(dictionary[d].name, word, count)) continue;
			if (debug) printf("[DEFINED]    ");
			if (debug) print_word(dictionary[d]);
			if (dictionary[d].address and dictionary[d].type == type_macro) {
				if (debug) printf("MACRO: calling macro! %s\n", dictionary[d].name);
				if (stack_pointer >= 4096) { puts("stack overflow"); abort(); } 
				stack[stack_pointer] = index;
				return_start[stack_pointer] = start;
				return_count[stack_pointer++] = count;
				index = dictionary[d].address;
				count = 0;
				goto next;
			}
			arguments[argument_count++] = d;
			if (dictionary[d].type == type_label and dictionary[d].value == uninit)
				dictionary[d].value = ins_count;
			goto next;
		}
		push_new: arguments[argument_count++] = dictionary_count;
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

	next: 	previous_start = start;
		previous_count = count;
		count = 0;
	}
	if (count) goto process_word;

	*out_dictionary = dictionary;
	*out_dictionary_count = dictionary_count;
	*out_instructions = instructions;
	*out_ins_count = ins_count;
	*out_arguments = arguments;
	*out_argument_count = argument_count;
}

static char* read_file(const char* filename, size_t* length) {
	FILE* file = fopen(filename, "r");
	if (not file) {
		fprintf(stderr, compiler bold red "error:" reset bold " ");
		perror(filename);
		fprintf(stderr, reset);
		exit(1);
	}

	fseek(file, 0, SEEK_END);
        *length = (size_t) ftell(file); 
	char* text = calloc(*length + 1, 1);
        fseek(file, 0, SEEK_SET); 
	fread(text, 1, *length, file);
	fclose(file); 

	if (debug) printf("info: file \"%s\": read %lu bytes\n", filename, *length);
	return text;
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
		"\t     " lightblue "w " bold yellow "setzero" reset "\n"
		"\t     " lightblue "w w l " bold cyan "branch" reset "\n"
		"\t     " lightblue "w w l " bold red "store" reset "\n"
		"\t     " lightblue "w w l " bold magenta "load" reset "\n"
		"\t     " lightblue "l " reset bold "systemcall" reset "\n"
		;
	
	puts(welcome_string);

	char input[4096] = {0};

	char* program = NULL;
	nat program_length = 0;

	nat* arguments = calloc(4096, sizeof(nat));
	nat argument_count = 0;

	nat ins_count = 0;
	struct instruction* instructions = NULL;

	nat dictionary_count = 0;
	struct word* dictionary = calloc(4, sizeof(struct word));
	dictionary[dictionary_count++] = (struct word) { .name = "pc00", .length = 4, .type = type_variable, .def = uninit };
	dictionary[dictionary_count++] = (struct word) { .name = "s000", .length = 4, .type = type_variable, .def = uninit };
	dictionary[dictionary_count++] = (struct word) { .name = "arg1", .length = 4, .type = type_variable, .def = uninit };
	dictionary[dictionary_count++] = (struct word) { .name = "arg2", .length = 4, .type = type_variable, .def = uninit };

loop:
	printf(":%llu:%llu: ", dictionary_count, ins_count);
	fgets(input, sizeof input, stdin);
	nat len = strlen(input);
	input[--len] = 0;

	if (not strcmp(input, "q") or not strcmp(input, "quit")) exit(0);
	else if (not strcmp(input, "o") or not strcmp(input, "clear")) printf("\033[H\033[2J");
	else if (not strcmp(input, "help")) puts(help_string);
	else if (not strcmp(input, "undo")) {if (ins_count) ins_count--;}
	else if (not strcmp(input, "arguments")) print_nats(arguments, argument_count);
	else if (not strcmp(input, "dictionary")) print_dictionary(dictionary, dictionary_count);
	else if (not strcmp(input, "instructions")) print_instructions(instructions, ins_count, dictionary);
	else if (not strcmp(input, "resetall")) { dictionary_count = 0; ins_count = 0; }
	else {
		input[len++] = 32;
		program = realloc(program, program_length + len);
		memcpy(program + program_length, input, len);
		program_length += len;
		parse(program, program_length, program_length - len,
			&instructions, &ins_count, 
			&dictionary, &dictionary_count, 
			&arguments, &argument_count
		);
		execute_directly(instructions, ins_count, dictionary); 
	}
	goto loop;
}

int main(int argc, const char** argv) {
	if (argc < 2) repl(); 

	nat* arguments = calloc(4096, sizeof(nat));
	nat argument_count = 0;

	nat ins_count = 0;
	struct instruction* instructions = NULL;
	nat dictionary_count = 0;
	struct word* dictionary = calloc(4, sizeof(struct word));
	dictionary[dictionary_count++] = (struct word) { .name = "pc00", .length = 4, .type = type_variable, .def = uninit };
	dictionary[dictionary_count++] = (struct word) { .name = "s000", .length = 4, .type = type_variable, .def = uninit };
	dictionary[dictionary_count++] = (struct word) { .name = "arg1", .length = 4, .type = type_variable, .def = uninit };
	dictionary[dictionary_count++] = (struct word) { .name = "arg2", .length = 4, .type = type_variable, .def = uninit };

	size_t length = 0;
	char* text = read_file(argv[1], &length);
	parse(text, length, 0, &instructions, &ins_count, &dictionary, &dictionary_count, &arguments, &argument_count);

	execute_directly(instructions, ins_count, dictionary); 

	print_nats(arguments, argument_count);
	print_dictionary(dictionary, dictionary_count);
	print_instructions(instructions, ins_count, dictionary);
	
	


	// instead of executing instructions directly, we need to do instruction selection for  the     "C virtual machine ISA", ie, an interpreter in C. 
	// ...for the actual compiler, we will use the machine ISA, of course. the usage of each instruction has an associated number of cycles. for C, everything is 1 cycle. 

}
































/*

note 
	first code with macros ever!
note

bubbles setzero 
mr define increment increment increment end
dr define mr mr mr mr end
tr define increment end
bubbles dr dr dr dr

	i setzero 

loop 
	i tr
	debughex
	bubbles i loop branch


*/












/*
	else if (include) {
		size_t file_length = 0;
		char* file = read_file(w, &file_length);
		if (not file) abort();
		file_stack[file_stack_count++] = (struct file_frame) {
			.file = file, 
			.file_length = file_length, 
			.F_index = 0, 
			.F_begin = 0
		};
		include = false;
		goto advance;



	} else if (macro) {
		if (equal(w, "define")) macro++; 
		if (equal(w, "endmacro")) macro--; 
		goto advance;


	} else if (equal(w, "endmacro")) {
		if (not stack_pointer) { printf("cannot ret! endmacro not in macro!\n"); abort(); }
		stack_pointer = base_pointer;
		word_pc = stack[--stack_pointer];
		base_pointer = stack[--stack_pointer];
		goto advance;
	} 


	else if (equal(w, "callsave")) { save[15] = _[0]; }
	else if (equal(w, "include")) include = 1;
	else if (equal(w, "define")) { 
		addresses[*_] = word_pc; 
		macro++;
	}

	else if (equal(w, "call")) {
		stack[stack_pointer++] = base_pointer;
		stack[stack_pointer++] = word_pc;
		base_pointer = stack_pointer;
		word_pc = addresses[save[15]];
		goto advance;
	}















	else if (equal(w, "gensym")) { 

		nat name = 0; 
		nat open = name_count;

		while (name < name_count) {
			if (names[name]) {
			} else if (open == name_count) open = name;
			name++;
		}

		if (open == name_count) name_count++;
		names[open] = strdup("");
		name = open;

		nat i = sizeof _ / sizeof(nat) - 1;
		while (i) { _[i] = _[i - 1]; i--; } *_ = name;
	}





*/













/* 

   ------------ ret ----------

	sp = bp
	wpc = stack[--sp]
	bp = sp[--bp]


------------- call -----------

	stack[sp++] = bp
	stack[sp++] = word_pc;
	bp = sp















static void push_argument(nat argument, nat* arguments) {
	for (nat a = 31; a; a--) arguments[a] = arguments[a - 1];
	*arguments = argument;
}



*/



























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




























} else if (op == jump) {
			if (debug) printf("executing jalr (%llu) -> %llu]\n", in[0], in[1]);
			if (debug) printf("executing jalr -> @%llu (%llu) \n", in[0], dictionary[in[1]].value);
			if (dictionary[in[0]].value == (size_t) -1) { 
				puts(red "internal error: unspecified label in branch" reset); 
				goto halt; 
			}
			variables[in[1]] = ip;
			ip = dictionary[in[0]].value - 1;
			ip = variables[in[0]];
			abort();


	//	if (is("note", word, count)) { comment = not comment; goto next; }
	//	if (comment) goto next;


*/












// dzmto4xrku8n306i
// er23nlvghmkzcpiw
// 9rvduo4xh1wpy7m0
// sdjuwt9ah8vz5r26
// zdm9hjko172w5n0f
// 2dcw9o0u4ny7ekzh
// kb8gptl0fwu1dco3






















