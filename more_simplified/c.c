#include <stdio.h>
#include <stdbool.h>   // a programming language made by dwrr on 2306283.112548 
#include <iso646.h>    // simplified and revised on 202309063.134052
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
#include <errno.h>

typedef uint64_t nat;
static nat debug = 0;

enum thing_type { type_null, type_variable, type_label };
enum instruction_type { 
	null, 
	increment, zero,  store1, load1, store2, load2, store4, load4, store8, load8, 
	branch, discard, systemcall,  unname, executenow, 
	debugpause, debughex, debugdec,  debugarguments,    isa_count
};
static const nat arity[isa_count] = { 
	0, 
	1, 1,  2, 2, 2, 2, 2, 2, 2, 2, 
	3, 1, 1, 1, 0,

	0, 1, 1, 0
};

static const char* spelling[isa_count] = {
	"",
	"incr", "zero", "store1", "load1", "store2", "load2", "store4", "load4", "store8", "load8", 
	"branch", "discard", "system", "unname", "now",
	"debugpause", "debughex", "debugdec", "debugarguments"
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
	nat pt_value;
	nat def;
	nat file_location;
	nat address;
};

static const nat uninit = (nat) ~0;

static const char* spell_type(nat t) {
	if (t == type_null) return "{null_type}";
	if (t == type_label)  return "label";
	if (t == type_variable)  return "variable";
	return "unknown";
}

static void print_name(struct word w) {
	for (nat _ = 0; _ < w.length; _++) putchar(w.name[_]);
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

static void print_instruction(struct instruction ins, struct word* dict, nat dictionary_count) {
	putchar(9);
	printf("%s ", spelling[ins.op]);

	printf("{");
	for (nat i = 0; i < arity[ins.op]; i++) {
		putchar(32);
		if (ins.in[i] < dictionary_count) print_name(dict[ins.in[i]]); 
		else printf("{name null}");
	}
	printf(" }\n\t\t\t\t\t\t\t"
		".ph = %lld .life = [%lld,%lld] .ct = %lld .defs=", 
		ins.ph, ins.begin, ins.end, ins.ct);

	print_nats(ins.defs, arity[ins.op]);
}

static void print_dictionary(struct word* dictionary, nat dictionary_count) {
	printf("dictionary { \n");
	for (nat i = 0; i < dictionary_count; i++) {
		printf("\t%3llu  :  ", i);
		print_word(dictionary[i]);
	}
	printf("}\n");
}

static void print_instructions(struct instruction* instructions, nat ins_count, struct word* dictionary, nat dictionary_count) {
	printf("instructions { \n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t%3llu  :  " , i);
		print_instruction(instructions[i], dictionary, dictionary_count);
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
	nat file_location, nat dictionary_count) {

	const nat arg0 = argument_count > 0 ? arguments[argument_count - 1] : uninit;
	const nat arg1 = argument_count > 1 ? arguments[argument_count - 2] : uninit;
	const nat arg2 = argument_count > 2 ? arguments[argument_count - 3] : uninit;

	if (op == branch) {
		if (arg0 >= dictionary_count or arg1 >= dictionary_count or arg2 >= dictionary_count) {
			printf("error: branch argument error\n"); 
			return;
		}

		if (dictionary[arg0].type == type_label and dictionary[arg0].value == *ins_count) {
			dictionary[arg0].value = uninit;
			dictionary[arg0].pt_value = uninit;
		}

		if (dictionary[arg0].type != type_label) { 
			printf("error: bad branch arg0 label %llu:%llu :: "
				"expected label argument of type label.\n", 
				file_location, dictionary[arg0].file_location
			); 
			return; 
		}

		dictionary[arg1].type = type_variable;
		dictionary[arg2].type = type_variable;
	} 


	else if (op == load1 or op == store1 or
		 op == load2 or op == store2 or
		 op == load4 or op == store4 or
		 op == load8 or op == store8) {

		if (not argument_count) { 
			printf("error: argstack was empty, no arguments can be given for load or store instruction.\n"); 
			return; 
		}

		if (arg0 >= dictionary_count or arg1 >= dictionary_count) {
			printf("error: load/store argument error\n"); 
			return;
		}

		dictionary[arg0].type = type_variable;
		dictionary[arg1].type = type_variable;
	}


	else if (op == zero or op == increment) {

		if (not argument_count) { 
			printf("error: argstack was empty, no arguments can be given for increment or zero instruction.\n"); 
			return; 
		}

		if (arg0 >= dictionary_count) {
			printf("error: zero argument error\n"); 
			return;
		}

		dictionary[arg0].type = type_variable;
	}

	else if (op == systemcall) {  
		if (arg0 >= dictionary_count) {
			printf("error: systemcall argument error\n"); 
			return;
		}
		if (dictionary[arg0].type != type_label) { 
			printf("error: bad systemcall arg0 var %llu:%llu\n", file_location, dictionary[arg0].file_location); 
			return;
		}

		if (not argument_count) { 
			printf("error: argstack was empty, no arguments can be given for systemcall instruction.\n"); 
			return; 
		}
	} 

	else if (op == discard) {
		if (arg0 >= dictionary_count) {
			printf("error: discard argument error\n"); 
			return;
		}

		if (dictionary[arg0].type != type_variable) { 
			printf("error: bad discard arg0 var %llu:%llu\n", file_location, dictionary[arg0].file_location); 
			return;
		}

		if (not argument_count) { 
			printf("error: argstack was empty, no arguments can be given for discard instruction.\n"); 
			return; 
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


static void process_syscall(nat n, nat* r) {

	nat r0 = r[1], r1 = r[2], r2 = r[3], r3 = r[4], r4 = r[5], r5 = r[6];

	printf("SYSCALL (NR=%llu): {%llu %llu %llu %llu %llu %llu}\n", 
		n, r0, r1, r2, r3, r4, r5
	);

	if (n == 7) {
		printf("calling exit(%llu);\n", r0);
		exit((int) r0);
	}

	else if (n == 8) {
		printf("calling read(%llu, %p, %llu);\n", r0, (void*) r1, r2);
		r[1] = (nat) read((int) r0, (void*) r1, r2);
		r[2] = (nat) (long long) errno;
	}

	else if (n == 9) {
		printf("calling write(%llu, %p, %llu);\n", r0, (void*) r1, r2);
		r[1] = (nat) write((int) r0, (void*) r1, r2);
		r[2] = (nat) (long long) errno;
	}

	else if (n == 10) {
		printf("calling open(%p, %llu, %llu);\n", (void*) r0, r1, r2);
		r[1] = (nat) open((const char*) r0, (int) r1, r2);
		r[2] = (nat) (long long) errno;
	}

	else if (n == 11) {
		printf("calling close(%llu);\n", r0);
		r[1] = (nat) close((int) r0);
		r[2] = (nat) (long long) errno;
	} else {
		printf("unknown syscall: %llu\n", n);
	}


if (n == 1000) r[1] = (nat) fork();
if (n == 1000) r[1] = (nat)(void*)mmap((void*)r0,r1,(int)r2,(int)r3,(int)r4,(long long) r5);
if (n == 1000) r[1] = (nat)munmap((void*)r0, r1);

}





static void execute_directly(
	const nat starting_ip, struct instruction* instructions, 
	nat ins_count, struct word* dictionary, nat dictionary_count,
	nat* r
) {

	if (debug) {
		printf("executing these instructions: \n");
		print_instructions(instructions, ins_count, dictionary, dictionary_count);
	}

	for (*r = starting_ip; *r < ins_count; (*r)++) {

		if (debug) printf("executing @%llu : ", *r);
		if (debug) print_instruction(instructions[*r], dictionary, dictionary_count);

		const nat op  = instructions[*r].op;

		nat in[3] = {0};
		memcpy(in, instructions[*r].in, 3 * sizeof(nat));

		if (false) {}
		else if (op == increment) r[in[0]]++;
		else if (op == zero) r[in[0]] = 0;
		
		else if (op == branch) { if (r[in[1]] < r[in[2]]) *r = dictionary[in[0]].value - 1; } 

		else if (op == load8) r[in[0]] = (nat) *(uint64_t*) r[in[1]];
		else if (op == store8) *(uint64_t*)r[in[0]] = (uint64_t) r[in[1]];

		else if (op == load4) r[in[0]] = (nat) *(uint32_t*) r[in[1]];
		else if (op == store4) *(uint32_t*)r[in[0]] = (uint32_t) r[in[1]];

		else if (op == load2) r[in[0]] = (nat) *(uint16_t*) r[in[1]];
		else if (op == store2) *(uint16_t*)r[in[0]] = (uint16_t) r[in[1]];

		else if (op == load1) r[in[0]] = (nat) *(uint8_t*) r[in[1]];
		else if (op == store1) *(uint8_t*)r[in[0]] = (uint8_t) r[in[1]];

		else if (op == discard) { printf("value discarded: %s\n", dictionary[in[0]].name); }
		else if (op == systemcall) { process_syscall(in[0], r); }

		else if (op == debugpause) getchar();
		else if (op == debughex) printf("debug: 0x%llx\n", r[in[0]]);
		else if (op == debugdec) printf("debug: %lld\n", r[in[0]]);
		else if (op == debugarguments) printf("debug: error: no arguments exist during runtime\n");
		else {
			printf("internal error: execute: unexpected instruction: %llu\n", op);
			abort();
		}
	}

 	if (debug) puts("[finished execution]");
}


static nat count = 0;
static nat start = 0;

static void parse(
	char* string, nat length, nat starting_index,
	struct instruction** out_instructions, nat *out_ins_count, 
	struct word** out_dictionary, nat* out_dictionary_count,
	nat* r
) {

	// r[2] = (nat)(void*) malloc(65536);
	// r[3] = 0;

	struct word* dictionary = *out_dictionary;
	nat dictionary_count = *out_dictionary_count;
	struct instruction* instructions = *out_instructions;
	nat ins_count = *out_ins_count;
	nat d = 0;

	for (*r = starting_index; *r < length; (*r)++) {
		if (not isspace(string[*r])) { 
			if (not count) start = *r;
			count++; continue;
		} else if (not count) continue;

		process_word:;
		char* const word = string + start;


		if (is(spelling[debugarguments], word, count)) print_nats((nat*) r[2], r[3]);

		if (is(spelling[executenow], word, count)) {
	
			const struct instruction this = instructions[--ins_count];
			const nat op  = this.op;

			nat in[3] = {0};
			memcpy(in, this.in, 3 * sizeof(nat));

			if (false) {}
			else if (op == increment) r[in[0]]++;
			else if (op == zero) r[in[0]] = 0;
			
			else if (op == branch) { if (r[in[1]] < r[in[2]]) *r = dictionary[in[0]].pt_value - 1; } 

			else if (op == load8) r[in[0]] = (nat) *(uint64_t*) r[in[1]];
			else if (op == store8) *(uint64_t*)r[in[0]] = (uint64_t) r[in[1]];

			else if (op == load4) r[in[0]] = (nat) *(uint32_t*) r[in[1]];
			else if (op == store4) *(uint32_t*)r[in[0]] = (uint32_t) r[in[1]];

			else if (op == load2) r[in[0]] = (nat) *(uint16_t*) r[in[1]];
			else if (op == store2) *(uint16_t*)r[in[0]] = (uint16_t) r[in[1]];

			else if (op == load1) r[in[0]] = (nat) *(uint8_t*) r[in[1]];
			else if (op == store1) *(uint8_t*)r[in[0]] = (uint8_t) r[in[1]];

			else if (op == discard) { printf("value discarded: %s\n", dictionary[in[0]].name); }
			else if (op == systemcall) { process_syscall(in[0], r); }

			else if (op == debugpause) getchar();
			else if (op == debughex) printf("pt_debug: 0x%llx\n", r[in[0]]);
			else if (op == debugdec) printf("pt_debug: %lld\n", r[in[0]]);
			else if (op == debugarguments) print_nats((nat*) r[2], r[3]);

			else {
				printf("internal error: execute: unexpected instruction: %llu\n", op);
				abort();
			}

			goto next;
		}

		if (is(spelling[unname], word, count)) { 
		
			if (debug) puts("UNNAME EXECUTED"); 
			if (not r[3]) { 
				printf("error: argstack was empty, no arguments can be given for unname instruction.\n"); 
				goto next; 

			}

			if (((nat*)r[2])[r[3] - 1] >= dictionary_count) {
				printf("error: dictionary was empty, no arguments can be given for unname instruction.\n"); 
				goto next;
			}

			dictionary[((nat*)r[2])[r[3] - 1]].length = 0; goto next; 
		}

		for (nat i = null; i < isa_count; i++) {    // todo: refactor this to be a simple if-elseif statement chain.

			if (is(spelling[i], word, count)) {
				ins(i, (nat*)r[2], r[3], dictionary, &instructions, &ins_count, *r, dictionary_count);
				if (debug and ins_count) {
					print_instruction(
						instructions[ins_count - 1], 
						dictionary, dictionary_count
					); 
				}
				goto next;
			}
		}

		for (d = 0; d < dictionary_count; d++) {
			if (dictionary[d].length != count or strncmp(dictionary[d].name, word, count)) continue;
			if (debug) printf("[DEFINED]    ");
			if (debug) print_word(dictionary[d]);
		
			((nat*)r[2])[r[3]++] = d;

			if (dictionary[d].type == type_label and dictionary[d].value == uninit) {
				dictionary[d].value = ins_count;
				dictionary[d].pt_value = *r;
			}

			goto next;
		}

		((nat*)r[2])[r[3]++] = dictionary_count;
		dictionary = realloc(dictionary, sizeof(struct word) * (dictionary_count + 1));
		dictionary[dictionary_count++] = (struct word) {
			.name = strndup(word, count), 
			.length = count, 
			.type = type_label,
			.value = ins_count,
			.pt_value = *r,
			.def = uninit,
			.file_location = start + count
		};

		if (debug) printf("[not defined]  -->  assuming  ");
		if (debug) print_word(dictionary[dictionary_count - 1]);

	next: 	count = 0;
	}
	if (count) goto process_word;

	*out_dictionary = dictionary;
	*out_dictionary_count = dictionary_count;
	*out_instructions = instructions;
	*out_ins_count = ins_count;
}

static char* read_file(const char* filename, size_t* length) {
	FILE* file = fopen(filename, "r");
	if (not file) {
		fprintf(stderr, "compiler-name: error: ");
		perror(filename);
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

static void create_dictionary(struct word** dictionary, nat* dictionary_count) {

	(*dictionary_count) = 0;
	*dictionary = calloc(15, sizeof(struct word));
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "pc",  .length = 2, .type = type_variable, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "sp",  .length = 2, .type = type_variable, .def = uninit };

	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "astack",  .length = 6, .type = type_variable, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "acount",  .length = 6, .type = type_variable, .def = uninit };

	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "arg0", .length = 4, .type = type_variable, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "arg1", .length = 4, .type = type_variable, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "arg2", .length = 4, .type = type_variable, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "arg3", .length = 4, .type = type_variable, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "arg4", .length = 4, .type = type_variable, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "arg5", .length = 4, .type = type_variable, .def = uninit };

	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "sys_exit", .length = 8, .type = type_label, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "sys_read", .length = 8, .type = type_label, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "sys_write", .length = 9, .type = type_label, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "sys_open", .length = 8, .type = type_label, .def = uninit };
	(*dictionary)[(*dictionary_count)++] = (struct word) { .name = "sys_close", .length = 9, .type = type_label, .def = uninit };
}

static _Noreturn void repl(void) {
	
	const char* welcome_string = 
		"{Unnamed Language} 0.0.1 202307086.162752 interpreter"   "\n"
		"   Type \"help\" for more information.";

	const char* help_string = 
		"Help menu: " "\n"
		"\t. help : 		(you are here.)\n"
		"\t. quit : 		quit the interpreter.\n"
		"\t. undo : 		remove the last instruction.\n"
		"\t. dictionary : 	display the current dictionary.\n"
		"\t. instructions : 	display the current instructions.\n"
		"\t. resetall : 	reset the instructions and dictionary to be empty.\n"
		"\t. toggledebug : 	toggle whether debug output is given.\n"
		"\t. {expression} : 	a series of instructions in the language ISA.\n"
	;
	
	puts(welcome_string);

	char input[4096] = {0};

	char* program = NULL;
	nat program_length = 0;

	nat ins_count = 0;
	struct instruction* instructions = NULL;
	
	nat dictionary_count = 0;
	struct word* dictionary = NULL;
	create_dictionary(&dictionary, &dictionary_count);

	nat* registers = malloc(65536 * sizeof(nat));
	registers[1] = (nat)(void*) malloc(65536);
	nat* pt_registers = malloc(65536 * sizeof(nat));
	pt_registers[1] = (nat)(void*) malloc(65536);
	pt_registers[2] = (nat)(void*) malloc(65536);
	pt_registers[3] = 0;

loop:
	printf(":%llu:%llu: ", dictionary_count, ins_count);
	fgets(input, sizeof input, stdin);
	nat len = strlen(input);
	input[--len] = 0;

	if (not strcmp(input, "q") or not strcmp(input, "quit")) exit(0);
	else if (not strcmp(input, "o") or not strcmp(input, "clear")) printf("\033[H\033[2J");
	else if (not strcmp(input, "help")) {
		puts(help_string);
		puts("ISA:");
		for (nat i = 0; i < isa_count; i++) {
			printf("\t  (%llu) %s \n", arity[i], spelling[i]);
		}
		puts("[done]");
	}
	else if (not strcmp(input, "undo")) {if (ins_count) ins_count--;}
	else if (not strcmp(input, "arguments")) print_nats((nat*) pt_registers[2], pt_registers[3]);
	else if (not strcmp(input, "dictionary")) print_dictionary(dictionary, dictionary_count);
	else if (not strcmp(input, "instructions")) print_instructions(instructions, ins_count, dictionary, dictionary_count);
	else if (not strcmp(input, "resetall")) { dictionary_count = 0; ins_count = 0; create_dictionary(&dictionary, &dictionary_count); }
	else if (not strcmp(input, "toggledebug")) debug = not debug;
	
	else {
		input[len++] = 32;
		program = realloc(program, program_length + len);
		memcpy(program + program_length, input, len);
		program_length += len;
		const nat origin = ins_count;
		parse(program, program_length, program_length - len,
			&instructions, &ins_count, 
			&dictionary, &dictionary_count,  pt_registers
		);
		execute_directly(origin, instructions, ins_count, dictionary, dictionary_count, registers); 
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
	struct word* dictionary = NULL;
	create_dictionary(&dictionary, &dictionary_count);

	nat* registers = malloc(65536 * sizeof(nat));
	registers[1] = (nat)(void*) malloc(65536);
	nat* pt_registers = malloc(65536 * sizeof(nat));
	pt_registers[1] = (nat)(void*) malloc(65536);
	pt_registers[2] = (nat)(void*) malloc(65536);
	pt_registers[3] = 0;

	size_t length = 0;
	char* text = read_file(argv[1], &length);
	parse(text, length, 0, &instructions, &ins_count, &dictionary, &dictionary_count, pt_registers);
	execute_directly(0, instructions, ins_count, dictionary, dictionary_count, registers); 

	if (debug) {
		print_nats(arguments, argument_count);
		print_dictionary(dictionary, dictionary_count);
		print_instructions(instructions, ins_count, dictionary, dictionary_count);
	}

	free(arguments);
	free(instructions);
	free(dictionary);
}



















































































// instead of executing instructions directly, we need to do instruction selection for  the     "C virtual machine ISA", ie, an interpreter in C. 
	// ...for the actual compiler, we will use the machine ISA, of course. the usage of each instruction has an associated number of cycles. for C, everything is 1 cycle. 































/*




else if () {   
		if (arg0 >= dictionary_count) {
			printf("error: increment argument error\n"); 
			return;
		}

		if (not argument_count) { 
			printf("error: argstack was empty, no arguments can be given for incr instruction.\n"); 
			return; 
		}

		dictionary[arg0].type = type_variable;
	} 








//		if (dictionary[arg0].type != type_variable) { 
//			printf("error: increment arg0 %llu:%llu :: argument to increment is not a variable or is not initialized.\n",
//				 file_location, dictionary[arg0].file_location); 
//			return;
//		} 





else if () {

		if (arg0 >= dictionary_count or arg1 >= dictionary_count or arg2 >= dictionary_count) {
			printf("error: load/store argument error\n"); 
			return;
		}
		
		if (dictionary[arg0].type != type_label) { 
			printf("error: ldst arg0 label %llu:%llu :: expected label argument of type label.\n", 
				file_location, dictionary[arg0].file_location); 
			return; 
		}

		if (dictionary[arg1].type != type_variable) { 
			printf("error: ldst arg1 var %llu:%llu :: expected arg1 lhs to be a variable.\n",  
				 file_location, dictionary[arg1].file_location); 
			return; 
		}

		if (dictionary[arg2].type != type_variable) { 
			printf("error: ldst arg2 var %llu:%llu :: expected arg2 rhs to be a variable.\n",   
				file_location, dictionary[arg2].file_location); 
			return; 
		}


		if (not argument_count) { 
			printf("error: argstack was empty, no arguments can be given for nor instruction.\n"); 
			return; 
		}
	}




*/

//		if (dictionary[arg1].type != type_variable) { 
//			printf("error: bad branch arg1 var %llu:%llu :: "
//				"expected arg1 lhs to be a variable.\n",   
//				file_location, dictionary[arg1].file_location
//			); 
//			return; 
//		}
//
//		if (dictionary[arg2].type != type_variable) { 
//			printf("error: bad branch arg2 var %llu:%llu :: "
//				"expected arg2 rhs to be a variable.\n",   
//				file_location, dictionary[arg2].file_location
//			); 
//			return; 
//		}






























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






















/*
			if (dictionary[in[0]].value == (size_t) -1) { 
				puts(red "internal error: unspecified label in branch" reset); 
				goto halt; 
			}



1202307274.164349: 
	i want to add strings to the language simply by making macros be turned into a string, via a language primitive.

	essentially the equivalent of #macroname    
	in a macro in C preprocessing,  

	but it turns the contents of the macro into a string! 

	of course, this allows for any delimiter, not just quotes. 

	yay!!! cool beans. even new lines would be allow, and all whitespace too! 
	lets add a debugprintstring  routine to the language too! 

	thats important lol. just for debugging, 
	
	i think. yay. lets do that. we need to make the space after the macro name,
	 not included though. thats important. 
	lol. ie, one ws char after the first macro name.    





lldb notes:
	run
	breakpoint set -f c.c -l 390


			*/






//push_new: 


// if (not thing) { puts("string was found null in is()."); }


// typedef long long integer;





/*




	current language ISA:
===============================================




variables:
============================

	programcounter
	stackpointer
	arg0
	arg1
	arg2
	arg3
	arg4
	arg5



labels:
==============================

	byte
	doublebyte
	quadbyte
	octalbyte

	exit
	open
	read
	write
	close


ISA: (8 ins)
============================

	a dis
	a anon
	c sc
	a incr
	a zero
	a b c st
	a b c ld
	a b c blt






















deleted:

x	a b nor

x	a b s1
x	a b s2
x	a b s4
x	a b s8
x	a b l1
x	a b l2
x	a b l4
x	a b l8























IHHHH

FUST ASCESS THE PROGRAMM COUNTER


	PC PC PC PC PC 


add var    pc    

x     jalr


yay



	a sc
	a incr
	a zero
	a b nor
	a b s1
	a b s2
	a b s4
	a b s8
	a b l1
	a b l2
	a b l4
	a b l8

	a b c blt

	a b c jalr              <-------- we just need to figure out this instruction, now! then we are done. 


	setcallbit

	makeparsetime


	a anon        still important.

	gen           still important. 

	a dis	   probably going to add this back in later?..




							19 instructions now, so far?


								we might be able to get rid another one, maybe. idk. hm. 
								maybe setcallbit? or    gen?   or anon?    or dis?

										hm

				






x	a del0       <--------- not neccessary anymore:   just a stack pop operation, derived using language isa at pt.

x	a str       no more strings in the language?...

x	debugarguments
x	debugpause
x	a debughex
x	a debugdec

x	dup0     these can be replaced with memory accesses using loads and stores!!
x	dup1
x	dup2
x	dup3
x	dup4
x	dup5









writing bitwise  and   or     using only += and control flow with blt's!    nice. love it. 




// AND (&)
c = 0;
for (x = 0; x <= 15; ++x) {
    c += c;
    if (a < 0) {
        if (b < 0) {
            c += 1;
        }
    }
    a += a;
    b += b;
}

// OR (|)
c = 0;
for (x = 0; x <= 15; ++x) {
    c += c;
    if (a < 0) {
        c += 1;
    } else if (b < 0) {
        c += 1;
    }
    a += a;
    b += b;
}






*/







// static const nat arm64_register_count = 31;


/*if (ins.op < isa_count or 
	ins.in[0] >= dictionary_count) {
		printf(red "error ins in 0 not in dict" reset);
	} else 
		if (arity[ins.op]) print_name(dict[ins.in[0]]);*/





	/*else if (op == nor) {
		if (arg0 >= dictionary_count or arg1 >= dictionary_count) {
			printf(red "nor argument error\n" reset); return;
		}

		if (dictionary[arg0].type != type_variable) { 
			printf(red "error: bad nor arg0 var %llu:%llu\n" reset, file_location, dictionary[arg0].file_location); 
			return;
		} 

		if (dictionary[arg1].type != type_variable) { 
			printf(red "error: bad nor arg1 var %llu:%llu\n" reset, file_location, dictionary[arg1].file_location); 
			return;
		} 

		if (not argument_count) { printf(red "error: argstack was empty, no arguments can be given for nor instruction." reset "\n"); return; }
	}*/




//dictionary[arg0].type = type_variable;





// else if (op == compl) r[in[0]] = ~r[in[0]];
		//else if (op == l1) r[in[0]] = (nat) *(uint8_t*)  r[in[1]];
		//else if (op == l2) r[in[0]] = (nat) *(uint16_t*) r[in[1]];
		//else if (op == l4) r[in[0]] = (nat) *(uint32_t*) r[in[1]];
		//else if (op == s1) *(uint8_t*) r[in[0]] = (uint8_t)  r[in[1]];
		//else if (op == s2) *(uint16_t*)r[in[0]] = (uint16_t) r[in[1]];
		//else if (op == s4) *(uint32_t*)r[in[0]] = (uint32_t) r[in[1]];





/*
static bool macro = false;
static nat previous_start = 0;
static nat delimiter_start = 0;

static nat previous_count = 0;
static nat delimiter_count = 0;

static nat stack_pointer = 0;
static nat stack[4096] = {0};
static nat return_start[4096] = {0};
static nat return_count[4096] = {0};



static void debug_stack(void) {
	puts("debug: [STACK POINTER NONZERO]");

	puts("return_start"); print_nats(return_start, stack_pointer);
	puts("return_count"); print_nats(return_count, stack_pointer);

	puts("stack"); print_nats(stack, stack_pointer);

	printf(	"values: \n"
		"\t is_in_macro=%d    stack_pointer=%llu\n"

		"\t previous=[.start=%llu .count=%llu]\n"
		"\t delimiter=[.start=%llu .count=%llu]\n"
		"\t {word}=[.start=%llu .count=%llu]\n",

		macro, stack_pointer,
		previous_start, previous_count,
		delimiter_start, delimiter_count, 
		start, count
	);
}*/





		//char* const delimiter  = string + delimiter_start;
		//char* const previous   = string + previous_start;
		//char* const returnw    = stack_pointer ? string + return_start[stack_pointer - 1] : NULL;
	
		/*if (macro) {
			char* t = strndup(word, count);
			if (debug) printf("{%s}, ", t); 
			free(t);
			if (count == delimiter_count and not memcmp(word, delimiter, count)) {
				char* s = strndup(delimiter, count);
				if (debug) printf("\nMACRO: inside macro definition, found end of defintion! " yellow "%s" reset "\n", s);
				macro = 0; count = 0; delimiter_count = 0;
				free(s);
			} goto next;
		} */

		//if (is(spelling[del0], word, count)) { if (debug) puts("DEL0 EXECUTED"); if (not argument_count) { puts(red "error: argument count is zero in del0\n" reset); } else argument_count--; goto next; }
		//else if (is(spelling[gen], word, count)) { if (debug) puts("GEN EXECUTED"); count = 0; goto push_new; }






	/*if (stack_pointer and count == return_count[stack_pointer - 1] and not memcmp(word, returnw, count)) { 
			if (debug) printf("MACRO: executing runtime return statement for macro! " magenta "%s" reset 
					"  @ %llu\n", strndup(returnw, count), start + count);

			index = stack[--stack_pointer]; 
			count = 0; 
			if (debug and stack_pointer) debug_stack(); 
			goto next;
		}*/


		/* if (count == previous_count and not memcmp(word, previous, count)) {
			if (debug) printf("MACRO: encountered macro definition! " red "%s" reset "\n", strndup(previous, count));
			dictionary[arguments[argument_count - 1]].type = type_macro;
			dictionary[arguments[argument_count - 1]].address = index;
			macro = 1;
			delimiter_start = start;
			delimiter_count = count;
			goto next;
		} */




		//else if (is(spelling[dup0], word, count)) { if (debug) puts("DUP0 EXECUTED"); d = arguments[argument_count - 1]; goto call_macro; }
		//else if (is(spelling[dup1], word, count)) { if (debug) puts("DUP1 EXECUTED"); d = arguments[argument_count - 2]; goto push_existing; }
		//else if (is(spelling[dup2_], word, count)) { d = arguments[argument_count - 3]; goto push_existing; }
		//else if (is(spelling[dup3], word, count)) { d = arguments[argument_count - 4]; goto push_existing; }
		//else if (is(spelling[dup4], word, count)) { d = arguments[argument_count - 5]; goto push_existing; }
		//else if (is(spelling[dup5], word, count)) { d = arguments[argument_count - 6]; goto push_existing; }
		//else if (is(spelling[debugarguments], word, count)) { print_nats(arguments, argument_count); goto next; }
		






/*call_macro: 
			if (dictionary[d].address and dictionary[d].type == type_macro) {
				if (debug) printf("MACRO: calling macro! " cyan "%s" reset " @ %llu\n", dictionary[d].name, index);
				if (stack_pointer >= 4096) { puts("stack overflow"); abort(); } 
				stack[stack_pointer] = index;
				return_start[stack_pointer] = dictionary[d].address - dictionary[d].length;
				return_count[stack_pointer++] = dictionary[d].length;
				index = dictionary[d].address;
				count = 0;
				if (debug and stack_pointer) debug_stack(); 
				goto next;
			}



static const char* ins_color[isa_count] = { 
	"", 
	green, green, yellow, yellow, cyan, blue, red, red,
	magenta, magenta, magenta, magenta, 
};



#define lightblue ""
#define red   	""
#define green   ""
#define yellow  ""
#define blue   	""
#define magenta ""
#define cyan   	""
#define bold    ""
#define reset 	""























0:       5 zero executenow increment executenow increment executenow increment executenow
:16:0:  i zero executenow
:17:0:  label
:18:0:   a zero
:19:1:   i increment executenow
:19:1:   5 i label branch executenow



a zero
5 zero now incr now incr now incr now incr now incr now
i zero now
label
a incr
i incr now
5 i label branch now







*/
			//push_existing: 



