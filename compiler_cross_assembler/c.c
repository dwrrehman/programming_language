// 1202504045.155147 a compiler / assembler for a simpler version of the language.

/* 
	32 instructions:

	eoi halt ct ud do at
	lf ro set add sub 
	mul div rem and or 
	eor si sd ri bc ld 
	st lt ge ne eq sc

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

	eoi, halt, ct, ud, do_, at, lf, ro,
	set, add, sub, mul, div_, rem, 
	and_, or_, eor, si, sd, ri, bc,
	ld, st, lt, ge, ne, eq,
	sc,
	

	a6_nop, a6_svc, a6_mov, a6_bfm,
	a6_adc, a6_addx, a6_addi, a6_addr, a6_adr, 
	a6_shv, a6_clz, a6_rev, a6_jmp, a6_bc, a6_br, 
	a6_cbz, a6_tbz, a6_ccmp, a6_csel, 
	a6_ori, a6_orr, a6_extr, a6_ldrl, 
	a6_memp, a6_memia, a6_memi, a6_memr, 
	a6_madd, a6_divr, 

	m4_section, m4_gen, m4_br,

	isa_count
};

static const char* operations[isa_count] = {
	"nullins",

	"eoi", "halt", "ct", "ud", "do", "at", "lf", "ro",
	"set", "add", "sub", "mul", "div", "rem", 
	"and", "or", "eor", "si", "sd", "ri", "bc",
	"ld", "st", "lt", "ge", "ne", "eq",
	"sc", 

	"a6_nop", "a6_svc", "a6_mov", "a6_bfm",
	"a6_adc", "a6_addx", "a6_addi", "a6_addr", "a6_adr", 
	"a6_shv", "a6_clz", "a6_rev", "a6_jmp", "a6_bc", "a6_br", 
	"a6_cbz", "a6_tbz", "a6_ccmp", "a6_csel", 
	"a6_ori", "a6_orr", "a6_extr", "a6_ldrl", 
	"a6_memp", "a6_memia", "a6_memi", "a6_memr", 
	"a6_madd", "a6_divr", 

	"m4_section", "m4_gen", "m4_br",
};

static const nat arity[isa_count] = {
	0,

	0, 0, 0, 0, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 
	3, 3, 3, 3, 3, 3, 
	7, 
	
	
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
	nat ct;
	nat imm;
	nat args[16];
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

static char* load_file(const char* filename, nat* text_length) {
	int file = open(filename, O_RDONLY);
	if (file < 0) { 
		printf("error: could not open '%s': %s\n", filename, strerror(errno)); 
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
	char** variables, 
	nat* is_undefined,
	nat* is_readonly,
	nat* values, 
	nat var_count
) {
	puts("variable dictionary: ");
	for (nat i = 0; i < var_count; i++) {
		printf(" %c  %c [%llu]  \"%s\" = 0x%016llx\n", 
			is_undefined[i] ? 'U' : ' ', 
			is_readonly[i] ? 'R' : ' ', i + 1, 
			variables[i], values[i]
		);
	}
	puts("[end]");
}

static void print_instruction(
	struct instruction this, 
	char** variables
) {

	printf("  %s  %10s { ", 
		this.ct ? "CT" : "  ", 
		operations[this.op]
	);

	for (nat a = 0; a < arity[this.op]; a++) {
		if (this.imm & (1 << a))
			printf("#%llu", this.args[a]);
		else
			printf("%s", variables[this.args[a]]);
		printf(" ");
	}

	printf("}");
}


static void print_instructions(
	struct instruction* ins, 
	nat ins_count,
	char** variables
) {
	printf("instructions: (%llu count) {\n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], variables);
		puts("");
	}
	puts("}");
}


static void print_instruction_window_around(
	nat this, 
	struct instruction* ins, 
	nat ins_count, 
	char** variables
) {

	printf("\033[H\033[2J");
	const int64_t window_width = 8;
	const int64_t pc = (int64_t) this;

	for (int64_t i = -window_width; i < window_width; i++) {

		const int64_t here = pc + i;

		if (	here < 0 or 
			here >= (int64_t) ins_count
		) { puts(""); continue; }

		printf(" %5llu |    ", here);
		print_instruction(ins[here], variables);

		if (not i) puts("   <------ pc"); else puts("");
	}
}

static void print_index(const char* text, nat text_length, nat begin, nat end) {
	printf("\n\t@%llu..%llu: ", begin, end); 

	while (end and isspace(text[end])) end--;

	const nat max_width = 100;

	nat start_at = 
		begin < max_width 
		? 0 
		: begin - max_width;

	nat end_at = 
		end + max_width >= text_length 
		? text_length 
		: end + max_width;

	for (nat i = start_at; i < end_at; i++) {
		if (i == begin) printf("\033[32;1m[");
		putchar(text[i]);
		if (i == end) printf("]\033[0m");
	}
	
	printf("\033[0m");
	puts("\n");
}

/*


static void print_nats(nat* array, nat count) {
	printf("(%llu) { ", count);
	for (nat i = 0; i < count; i++) {
		printf("%llu ", array[i]);
	}
	puts("}");
}

static void print_binary(nat x) {
	for (nat i = 0; i < 64 and x; i++) {
		if (not (i % 8) and i) putchar('.');
		putchar((x & 1) + '0'); x >>= 1;
	}
	puts("");
}

static void insert_byte(
	uint8_t** output_data, 
	nat* output_data_count, 
	uint8_t x
) {
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
	if (target > here) {
		for (nat i = here; i < target; i++) 
			offset += length[i];
	} else {
		for (nat i = target; i < here; i++) 
			offset -= length[i];
	}
	return offset;
}

*/








int main(int argc, const char** argv) {

	if (argc != 2) 
		exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));


	const nat min_stack_size = 16384 + 1;
	nat stack_size = min_stack_size;
	const char* output_filename = "output_executable_new";

	struct instruction ins[max_instruction_count] = {0};
	nat ins_count = 0;

	char* variables[max_variable_count] = {0};
	nat is_undefined[max_variable_count] = {0};
	nat is_readonly[max_variable_count] = {0};
	nat values[max_variable_count] = {0};
	nat var_count = 0;

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

	const nat starting_index = files[file_count - 1].index;
	const nat text_length = files[file_count - 1].text_length;
	char* text = files[file_count - 1].text;
	const char* filename = files[file_count - 1].filename;

	nat 
		word_length = 0, word_start = 0, 
		arg_count = 0, last_used = 0, 
		is_compiletime = 0, is_immediate = 0;

	nat args[16] = {0};

	for (nat var = 0, op = 0, pc = starting_index; pc < text_length; pc++) {

		if (not isspace(text[pc])) {
			if (not word_length) word_start = pc;
			word_length++; 
			if (pc + 1 < text_length) continue;
		} else if (not word_length) continue;

		char* word = strndup(text + word_start, word_length);

		printf("[pc=%llu]:"
			" w=\"%s\" w_st=%llu\n", 
			pc, word, word_start
		);

		print_dictionary(
			variables, is_undefined, 
			is_readonly, values, 
			var_count
		);

		print_instructions(ins, ins_count, variables);

		if (not op) {
			if (*word == '(') { 				
				nat i = word_start + 1, comment = 1;
				while (comment and i < text_length) {
					printf("skipping over: '%c'  "
						"(comment = %llu)\n", 
						text[i], comment
					);
					if (text[i] == '(') comment++;
					if (text[i] == ')') comment--;
					i++;
				}
				if (comment) goto print_error;
				pc = i;
				goto next_word; 
			}


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
		else if (op == lf) goto define_name;
		for (var = var_count; var--;) {
			if (not is_undefined[var] and 
			    not strcmp(word, variables[var])) {
				last_used = var;
				goto push_argument;
			}
		} 

		if (	op == set and arg_count == 0 or 
			op == ri  and arg_count == 0 or 
			op == bc  and arg_count == 0 or 

			op == do_ or op == at or

			(op == lt or
			 op == ge or 
			 op == ne or 
			 op == eq) and arg_count == 2
		) {
			// do nothing

		} else { // treat as a binary literal:

			nat r = 0, s = 1;

			for (nat i = 0; i < strlen(word); i++) {

				if (word[i] == '0') s <<= 1;
				else if (word[i] == '1') { r += s; s <<= 1; }
				else if (word[i] == '_') continue;
				else goto print_error;
			}
			is_immediate |= 1 << arg_count;
			var = r;
			goto push_argument;
		}

	define_name:
		var = var_count;
		values[var] = 0;
		variables[var] = word; 
		var_count++;

	push_argument: 
		args[arg_count++] = var;

	process_op: 

		if (arg_count < arity[op]) goto next_word;

		else if (op == ud) is_undefined[last_used] = 1;
		else if (op == ct) is_compiletime = 1;
		else if (op == lf) {

			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				printf("warning: %s: already included\n", word);
				goto next_word;
			}
			included_files[included_file_count++] = word;

			nat len = 0;
			char* string = load_file(word, &len);

			files[file_count - 1].index = pc;
			files[file_count].filename = word;
			files[file_count].text = string;
			files[file_count].text_length = len;
			files[file_count++].index = 0;

			var_count--; 

			goto process_file;



		} else {

			if (not op) { puts("null operation parsed???"); abort(); }

			struct instruction new = { 
				.op = op,
				.ct = is_compiletime,
				.imm = is_immediate,
			};

			is_compiletime = 0;
			is_immediate = 0;

			memcpy(new.args, args, sizeof args);

			ins[ins_count++] = new;
		}
		arg_count = 0; op = 0;
		next_word: word_length = 0;
	}
	file_count--;
	if (file_count) goto process_file; 


	print_dictionary(
		variables, 
		is_undefined, 
		is_readonly, 
		values, 
		var_count
	);

	print_instructions(ins, ins_count, variables);

	puts("done!");


	exit(0);
}
















































// printf("0x%016llx", this.args[a]);
// printf("%lld", this.args[a]);



// bc, ri, set,   can define variables
// ud 		  can undefine them
/*
		if (comment) {
			if (word[strlen(word) - 1] == ')') { 
				comment = 0; continue; 
			}
			continue;
		}


*/













