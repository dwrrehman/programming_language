// 1202504045.155147 a compiler / assembler for a simpler version of the language. 


/*
	NOTE:

		THE REASON WHYYYYY        having runtime variables simply be   ct regindex  constants      

								doesnt work 



	the reason for why that doesnt work        is that 


				when you say                    add  x  blah


				now, you don't know what that meanssss


						if x is a runtime regindex constant    then we know  that we want to do a   runtime add   with blah


						if x is a compiltime  general purp;ose constant   then we know we want to do a  ct_add   at tompiletime. 




								BUTTTT      A GEN PURPOSE CT CONSTANT   AND A RT REGINDEX CONSTNAT  LOOK AND FEEL COMPLETELY IDENTICAL TO THE COMPILER

								THE ARE THE SAME THINGSS



						AND SO   we don't know which       rt_add    or ct_add     to gneerate.   when we see "add x blah"





						thatsssssssss why we need to pick.   either or.    XOR.   we need either to have regindexes be rt constants


						orrrrrr we have seperate rt and ct variables,   with ct being constatns, and rt being variables. 

										...and then we just provide a    register-constraint  instruction

										to assign a  regindex to a rt variable.   simple as that. 



							WE CANT HAVE BOTH.   










				honestly, the reality is...   using actual   regindexes      verbatim     ie, raw   hardware registers 



						is actually rather rare.  



									like,   on all archs  and all targets, anad all intended usecases. 






							EXCEPTTTTTT for system calls.   that uses them like crazy. 




		now, interestingly   we are not going to actually expose the system call insturction   with   argumentssssss i think??



		maybee..


			hmm actually maybe we have to?       because otherwise our code will have different     hardware register indexes for each target/arch

				so i think we needddd        the sc  instruction      which takes 7 arguments?... hmmm




						hmmmm idk.... 











	make comments only allowed   between instructions!
	also, make them use "(" and ")", 

			....ie, the moment you see a "(", start skipping until a ")".

							(this is a comment!)	( so is this )     ()   



	labels are simply ct variables! but they are defined in a weird way, defined on use, for "at", and "ne", "lt", "ge" etc. 

		ne a b skip

			(...code here...)

		at skip udf skip





	version 2: ideal rt code in this language    for adding two numbers, into a particular hardware register!


		rt c 1111          (register 15)               (c is defined here, and 

		set c 101
		add c 001





	version 2: ideal rt code in this language    for adding two numbers

		init c 101 
		add c 001


	version 2: ideal ct code in this lauguage    for adding two numbers

		ct c 101 
		add c 001

		ro c


	version 1: ideal rt code in this language    for adding two numbers

		def a 5
		def b 4
		def c a
		add c b


	version 1: ideal ct code in this lauguage    for adding two numbers

		bn a 01
		bn b 11
		ct c a 
		add c b


	examples of rt and ct code:


		ct y 001		<----   this defines and initializes a compiletime register, 
						with a binary literal, or another compiletime value.
		
		def x y			<---- this defines a runtime register, 
					      initializing it at runtime with a runtime or compiletime value. 

		add x 1     	        <---- this is a runtime incr statement. (this syntax applies both to compiletime and runtime instructions.)

		mul x y			<---- this is a runtime multiply_immediate statement, because y is compiletime known. 
		set x y			<---- this is a set_immediate runtime instruction. 

		set y 0			<---- this is a compiletime zero statement. 


	

	immediates are assumed to be only given if a given variable used in an instruction is not defined!

	this way this does not conflict with other variable names, at all. 

		additionally, binary literals are only assumed to be valid if they contain only zeros and ones, and maybe underscores too lol. 



	so yeah!





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
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t byte;

#define max_variable_count (1 << 14)
#define max_instruction_count (1 << 14)

enum all_architectures { no_arch, arm64_arch, arm32_arch, rv64_arch, rv32_arch, msp430_arch, };
enum all_output_formats { debug_output_only, macho_executable, elf_executable, ti_txt_executable };

enum core_language_isa {
	nullins,

	df, udf, lf,

	zero, incr, decr, not_, bn, 
	set, add, sub, mul, div_, rem,
	and_, or_, eor, si, sd, ld, at, cat, 
	ro, st, lt, ge, eq, ne, do_, 
	ctdebug, ctabort, ctpause,

	halt, emit, stringliteral, 
	nop, svc, mov, bfm,
	adc, addx, addi, addr, adr, 
	shv, clz, rev, jmp, bc, br, 
	cbz, tbz, ccmp, csel, 
	ori, orr, extr, ldrl, 
	memp, memia, memi, memr, 
	madd, divr,

	section_start, general4, branch4,

	isa_count
};

static const char* operations[isa_count] = {
	"--",
	"df", "udf", "lf",

	"df0", "df1", "df2", "df3",
	"df4", "df5", "df6", "df7",
	"df8", "df9", "df10", "df11",
	"df12", "df13", "df14", "df15",

	"zero", "incr", "decr", "not", "bn", 
	"set", "add", "sub", "mul", "div", "rem",
	"and", "or", "eor", "si", "sd", "ld", "at", "cat", 
	"ro", "st", "lt", "ge", "eq", "ne", "do", 
	"ctdebug", "ctabort", "ctpause",
	
	"halt", "emit", "string",
	"nop", "svc", "mov", "bfm",
	"adc", "addx", "addi", "addr", "adr", 
	"shv", "clz", "rev", "jmp", "bc", "br", 
	"cbz", "tbz", "ccmp", "csel", 
	"ori", "orr", "extr", "ldrl", 
	"memp", "memia", "memi", "memr", 
	"madd", "divr",

	"section", "gen4", "br4",
};

static const nat arity[isa_count] = {
	0,
	1, 1, 1,

	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1,

	1, 1, 1, 1, 2, 
	2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 1, 1, 
	1, 2, 3, 3, 3, 3, 1, 
	1, 0, 0,
	
	0, 2, 0, 
	0, 0, 5, 7, 
	6, 8, 7, 8, 3,
	5, 4, 4, 2, 2, 3, 
	4, 4, 7, 7, 
	5, 8, 5, 3, 
	7, 6, 5, 6,
	8, 5, 

	1, 8, 2,
};

struct instruction {
	nat op;
	nat args[max_arg_count];
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

char* load_file(const char* filename, nat* text_length) {
	int file = open(filename, O_RDONLY);
	if (file < 0) { 
		printf("error: could not open '%s': %s\n", filename, sterror(errno)); 
		exit(1);
	}
	*text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(*text_length + 1, 1);
	read(file, text, *text_length);
	close(file);
	return text;
}

static void print_dictionary(
	char** variables, nat* values, 
	nat* is_readonly, nat* undefined, nat max_count
) {
	puts("variables: ");
	const nat count = values[0];
	nat start = 0;
	if (count >= max_count) start = count - max_count;
	for (nat i = start; i < count; i++) {
		printf(" %c  %c [%llu]  \"%s\" = 0x%016llx\n", 
			undefined[i + 1] ? 'U' : ' ', 
			is_readonly[i + 1] ? 'R' : ' ', i + 1, 
			variables[i + 1], values[i + 1]
		);
	}
	puts("[end]");
}

static void print_instruction(struct instruction this) {
	printf("  %10s { ", operations[this.op]);
	for (nat a = 0; a < arity[this.op]; a++) { 
		if (this.args[a] < 512) printf("%llu", this.args[a]); 
		else if (this.args[a] == (nat) -1) printf("%lld", this.args[a]);
		else printf("0x%016llx", this.args[a]);
		printf(" ");
	}
	printf("}");
}

static void print_instructions(struct instruction* ins, nat ins_count) {
	printf("instructions: (%llu count) {\n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_instruction(ins[i]);
		puts("");
	}
	puts("}");
}

static void print_instruction_window_around(nat this, struct instruction* ins, nat ins_count, char** variables) {
	printf("\033[H\033[2J");
	const int64_t window_width = 8;
	const int64_t pc = (int64_t) this;
	for (int64_t i = -window_width; i < window_width; i++) {
		const int64_t here = pc + i;
		if (here < 0 or here >= (int64_t) ins_count) { puts(""); continue; }
		printf(" %5llu |   %s  ", here, operations[ins[here].op]);
		for (nat a = 0; a < arity[ins[here].op]; a++) {
			if (ins[here].op == bn and a == 1) 
				printf(" #%llx", ins[here].args[a]);
			else printf(" %s", variables[ins[here].args[a]]);
		}
		if (not i) puts("   <------ pc"); else puts("");
	}
}

static void print_index(const char* text, nat text_length, nat begin, nat end) {
	printf("\n\t@%llu..%llu: ", begin, end); 

	while (end and isspace(text[end])) end--;

	const nat max_width = 100;
	nat start_at = begin < max_width ? 0 : begin - max_width;
	nat end_at = end + max_width >= text_length ? text_length : end + max_width;

	for (nat i = start_at; i < end_at; i++) {
		if (i == begin) printf("\033[32;1m[");
		putchar(text[i]);
		if (i == end) printf("]\033[0m");
	}
	
	printf("\033[0m");
	puts("\n");
}

static void print_nats(nat* array, nat count) {
	printf("(%llu) { ", count);
	for (nat i = 0; i < count; i++) {
		printf("%llu ", array[i]);
	}
	puts("}");
}

static void print_binary(nat x) {
	for (nat i = 0; i < 64 and x; i++) {
		if (not (i % 8) and i) putchar(' ');
		putchar((x & 1) + '0'); x >>= 1;
	}
	puts("");
}



static void insert_byte(uint8_t** output_data, nat* output_data_count, uint8_t x) {
	*output_data = realloc(*output_data, *output_data_count + 1);
	(*output_data)[(*output_data_count)++] = x;
}

static void insert_u8(uint8_t** d, nat* c, uint8_t x) {
	insert_byte(d, c, x);
}

static void insert_bytes(uint8_t** d, nat* c, char* s, nat len) {
	for (nat i = 0; i < len; i++) insert_byte(d, c, (uint8_t) s[i]);
}

static void insert_u16(uint8_t** d, nat* c, uint16_t x) {
	insert_byte(d, c, (x >> 0) & 0xFF);
	insert_byte(d, c, (x >> 8) & 0xFF);
}

static void insert_u32(uint8_t** d, nat* c, uint32_t x) {
	insert_u16(d, c, (x >> 0) & 0xFFFF);
	insert_u16(d, c, (x >> 16) & 0xFFFF);
}

static void insert_u64(uint8_t** d, nat* c, uint64_t x) {
	insert_u32(d, c, (x >> 0) & 0xFFFFFFFF);
	insert_u32(d, c, (x >> 32) & 0xFFFFFFFF);
}

#define MH_MAGIC_64             0xfeedfacf
#define MH_EXECUTE              2
#define	MH_NOUNDEFS		1
#define	MH_PIE			0x200000
#define MH_DYLDLINK 		0x4
#define MH_TWOLEVEL		0x80
#define	LC_SEGMENT_64		0x19
#define LC_DYSYMTAB		0xb
#define LC_SYMTAB		0x2
#define LC_LOAD_DYLINKER	0xe
#define LC_LOAD_DYLIB		0xc
#define LC_REQ_DYLD		0x80000000
#define LC_MAIN			(0x28 | LC_REQ_DYLD)
#define LC_BUILD_VERSION 	0x32
#define LC_SOURCE_VERSION 	0x2A
#define LC_UUID            	0x1B
#define S_ATTR_PURE_INSTRUCTIONS 0x80000000
#define S_ATTR_SOME_INSTRUCTIONS 0x00000400
#define CPU_SUBTYPE_ARM64_ALL   0
#define CPU_TYPE_ARM            12
#define CPU_ARCH_ABI64          0x01000000 
#define VM_PROT_READ       	1
#define VM_PROT_WRITE   	2
#define VM_PROT_EXECUTE 	4
#define TOOL_LD			3
#define PLATFORM_MACOS 		1

static nat calculate_offset(nat* length, nat here, nat target) {
	nat offset = 0;
	if (target > here) for (nat i = here; i < target; i++) offset += length[i];
	else  		   for (nat i = target; i < here; i++) offset -= length[i];
	return offset;
}





int main(void) {
	if (argc != 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));

	const nat min_stack_size = 16384 + 1;
	nat stack_size = min_stack_size;
	const char* output_filename = "output_executable_new";

	struct instruction ins[max_instruction_count] = {0};
	nat ins_count = 0;

	char* variables[max_variable_count] = {0};
	nat values[max_variable_count] = {0};
	nat variable_count = 0;

	const char* included_files[4096] = {0};
	nat included_file_count = 0;

	char* string_list[4096] = {0};
	nat string_list_count = 0;

	struct file files[4096] = {0};
	nat file_count = 1;

	{ 
	nat text_length = 0;
	char* text = load_file(argv[1], &text_length);
	files[0].filename = argv[1];
	files[0].text = text;
	files[0].text_length = text_length;
	files[0].index = 0;
	}

process_file:;

	const nat starting_index = files[files_count - 1].index;
	const nat text_length = files[files_count - 1].text_length;
	char* text = files[files_count - 1].text;
	const char* filename = files[files_count - 1].filename;

	nat word_length = 0, word_start = 0, arg_count = 0;
	nat args[4] = {0}; 

	for (nat var = 0, op = 0, pc = starting_index; pc < text_length; pc++) {

		if (not isspace(text[pc])) {
			if (not word_length) word_start = pc;
			word_length++; 
			if (pc + 1 < text_length) continue;
		} else if (not word_length) continue;

		char* word = strndup(text + word_start, word_length);

		printf("[pc=%llu][skip=%llu]: w=\"%s\" w_st=%llu\n", pc, skip, word, word_start);
		print_dictionary(variables, values, variable_count);
		print_instructions(ins, ins_count);

		if (not op) {
			if (not strcmp(word, "eoi")) break;
			for (op = 0; op < isa_count; op++) 
				if (not strcmp(word, operations[op])) goto process_op;
			print_error: 
			printf("%s:%llu:%llu:"
				" error: undefined %s \"%s\"\n",
				filename, word_start, pc, 
				op == isa_count ? "operation" : "variable", word
			); 
			print_index(text, text_length, word_start, pc);
			if (op != isa_count) 
				printf( "note: calling operation: "
					"%s (arity %llu)\n", 
					operations[op], arity[op]
				);
			abort();
		} 
		else if (op == lf or op == df) goto define_name;
		for (var = *values + 1; var-- > 1;) {
			if (not undefined[var] and not strcmp(word, variables[var]))
				goto push_argument;
		} 
		goto print_error;

		define_name: var = *values + 1;
		values[var] = register_index++;
		variables[var] = word; ++*values;
		push_argument: args[arg_count++] = var;
		process_op: 

		if (op & (1LLU << 62LLU)) {
			const nat m = (nat) ((uint32_t) op);
			if (arg_count < macro_arity[m]) goto next_word;
			for (nat i = 0; i < arg_count; i++) {
				struct instruction new = { 
					.op = set, 
					.args[0] = i + 6, 
					.args[1] = args[i] 
				};
				ins[ins_count++] = new;
			}
			struct instruction new0 = { .op = cat, .args[0] = 5 }; 
			struct instruction new1 = { .op = do_, .args[0] = macro_label[m] }; 
			ins[ins_count++] = new0;
			ins[ins_count++] = new1;
		} 		
		else if (op == stringliteral) { in_string = 1; goto next_word; } 
		else if (arg_count < arity[op]) goto next_word;
		else if (op == df) {}
		else if (op >= df0 and op <= df15) {
			macro_label[macro_count] = var;
			macro_arity[macro_count] = op - df0;
			macros[macro_count++] = word;
		} else if (op == udf) undefined[args[0]] = 1;

		else if (op == lf) {

			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				printf("warning: %s: already included\n", word);
				goto next_word;
			}
			included_files[included_file_count++] = word;

			nat new_text_length = 0;
			char* new_text = load_file(word, &new_text_length);

			filestack[filestack_count - 1].index = pc;
			filestack[filestack_count].filename = word;
			filestack[filestack_count].text = new_text;
			filestack[filestack_count].text_length = new_text_length;
			filestack[filestack_count++].index = 0;
			variable_count--; 
			goto process_file;

		} else if (op < isa_count) {
			if (not op) { puts(" error"); abort(); }
			struct instruction new = { .op = op };
			memcpy(new.args, args, sizeof args);
			ins[ins_count++] = new;

		} else { puts("internal error: parsing unknown operation"); abort(); }

		arg_count = 0; op = 0;

		next_word: word_length = 0;
	}
	if (comment) { puts("error: unterminated comment"); abort(); }
	file_count--;
	if (file_count) goto process_file; 

	print_dictionary(variables, values, variable_count);
	print_instructions(ins, ins_count);
	puts("done!");
	exit(0);
}









