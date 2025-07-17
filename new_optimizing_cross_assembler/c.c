// optimizing assembler, revision of the compiler 
// written on 1202507174.162611 by dwrr


// current state:  we are looking at the parser, and realized that we need   rtat?  maybeee????

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>
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

static nat debug = 0;

#define max_variable_count 	(1 << 20)
#define max_instruction_count 	(1 << 20)
#define max_arg_count 		16

enum all_architectures { no_arch, arm64_arch, arm32_arch, rv64_arch, rv32_arch, msp430_arch, c_arch, };

enum all_output_formats {
	no_output,
	macho_executable, macho_object,
	elf_executable, elf_object,
	ti_txt_executable,
	uf2_executable,
	hex_array,
	c_source,
};
enum memory_mapped_addresses {
	compiler_return_address,
	compiler_format,
	compiler_should_overwrite,
	compiler_should_debug,
	compiler_stack_size,
};

enum isa {
noarch,
	zero, incr, nor, si, sd,
	set, add, sub, mul, div_,
	ld, st, emit, adr, 
	at, do_, lt, eq, 
	file, del, str,
riscv,
	rr, ri, rs, rb, ru, rj,

msp430,
	mo, mb,
arm64,
	nop, svc, mov, bfm,
	adc, addx, addi, addr, adr, 
	shv, clz, rev, jmp, bc, br, 
	cbz, tbz, ccmp, csel, 
	ori, orr, extr, ldrl, 
	memp, memia, memi, memr, 
	madd, divr, 

isa_count
};

static const char* operations[isa_count] = {
"",
	"zero", "incr", "nor", "si", "sd",
	"set", "add", "sub", "mul", "div",
	"ld", "st", "emit", "adr", 
	"at", "do", "lt", "eq", 
	"file", "del", "str",
"riscv",
	"rr", "ri", "rs", "rb", "ru", "rj",
"msp430",
	"mo", "mb",
"arm64",
	"nop", "svc", "mov", "bfm",
	"adc", "addx", "addi", "addr", "adr", 
	"shv", "clz", "rev", "jmp", "bc", "br", 
	"cbz", "tbz", "ccmp", "csel", 
	"ori", "orr", "extr", "ldrl", 
	"memp", "memia", "memi", "memr", 
	"madd", "divr", 
};

static const nat arity[isa_count] = {
0,	1, 1, 2, 2, 2,
	2, 2, 2, 2, 2,
	2, 2, 2, 1, 
	1, 1, 3, 3, 
	1, 1, 0,

0,	6, 5, 5, 5, 3, 3,

0,	8, 2,
				
0,	0, 0, 5, 7, 
	6, 8, 7, 8, 3,
	5, 4, 4, 2, 2, 3, 
	4, 4, 7, 7, 
	5, 8, 5, 3, 
	7, 6, 5, 6,
	8, 5,
};

struct instruction {
	nat op;
	nat imm;
	nat state;
	nat args[max_arg_count];
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

static struct instruction[max_instruction_count] ins = {0};
static nat ins_count = 0;

static char* variables[max_variable_count] = {0};
static nat values[max_variable_count] = {0};
static nat is_undefined[max_variable_count] = {0};
static nat var_count = 0;

static char* load_file(const char* filename, nat* text_length) {
	int file = open(filename, O_RDONLY);
	if (file < 0) { 
		printf("compiler: \033[31;1merror:\033[0m could not open '%s': %s\n", 
			filename, strerror(errno)
		); 
		exit(1);
	}
	*text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(*text_length + 1, 1);
	read(file, text, *text_length);
	close(file);
	return text;
}

static void print_dictionary(void) {
	puts("dictionary: ");
	for (nat i = 0; i < var_count; i++) {
		if (i % 4 == 0 and i < var_count - 1) puts("");
		printf("[%5llu]:%c:%16s:%016llx(%llu)\t",
			i, is_undefined[i] ? 'U' : ' ',
			variables[i], values[i], values[i]
		);
	}
	puts("");
}

static void print_instruction(struct instruction this) {
	int max_name_width = 0;
	for (nat i = 0; i < var_count; i++) {
		if (max_name_width < (int) strlen(variables[i])) {
			max_name_width = (int) strlen(variables[i]);
		}
	}
	printf(" %4s  ", operations[this.op]);
	for (nat a = 0; a < arity[this.op]; a++) {
		char string[4096] = {0};
		if (this.imm & (1 << a)) 
			snprintf(string, sizeof string, "%llx(%llu)", this.args[a], this.args[a]);
		else if (this.args[a] < var_count) 
			snprintf(string, sizeof string, "%s\033[38;5;235m(%llu)\033[0m", 
				variables[this.args[a]], this.args[a]
			);
		else snprintf(string, sizeof string, "(INTERNAL ERROR)");
		printf("%s", string);
		int left_to_print = max_name_width - (int) strlen(string);
		if (left_to_print < 0) left_to_print = 0;
		for (int i = 0; i < left_to_print; i++) putchar(' ');
		putchar(' ');
	}
}

static void print_instructions(const bool should_number_them) {
	printf("instructions: (count %llu) {\n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		if (should_number_them) printf("%4llu: ", i);
		print_instruction(ins[i]);
		puts("");
	}
}

static void print_index(const char* text, nat text_length, nat begin, nat end) {
	printf("\n\t@%llu..%llu: ", begin, end); 
	while (end and isspace(text[end])) end--;
	const nat max_width = 100;
	nat start_at = begin < max_width ? 0 : begin - max_width;
	nat end_at = end + max_width >= text_length 
		? text_length : end + max_width;
	for (nat i = start_at; i < end_at; i++) {
		if (i == begin) printf("\033[32;1m[");
		putchar(text[i]);
		if (i == end) printf("]\033[0m");
	}	
	printf("\033[0m");
	puts("\n");
}

noreturn static void print_error(
	const char* message, 
	const char* filename, 
	char* text, nat text_length, 
	nat begin, nat end
) {
	printf("%s:%llu:%llu: error: %s\n", filename, begin, end, message);  
	print_index(text, text_length, begin, end);
	abort();
}

static void dump_hex(uint8_t* memory, nat count) {
	puts("second debug output:    debugging executable bytes:\n");
	for (nat i = 0; i < count; i++) {
		if (i % 32 == 0) puts("");
		if (i % 4 == 0) putchar(32);
		if (memory[i]) printf("\033[32;1m");
		printf("%02hhx ", memory[i]);
		if (memory[i]) printf("\033[0m");
	}
	puts("\n");
}

static nat calculate_offset(nat* length, nat here, nat target) {
	nat offset = 0;
	if (target > here) {
		for (nat i = here; i < target; i++) {
			if (i >= ins_count) {
				printf("fatal error: calculate_offset(lengths, %lld, %lld): "
					"[ins_count = %llu]: tried to index %lld into lengths, "
					"aborting..\n", here, target, ins_count, i
				);
				print_nats(length, ins_count);
				abort();			
			}
			offset += length[i];
		}
	} else {
		for (nat i = target; i < here; i++) {
			if (i >= ins_count) {
				printf("fatal error: calculate_offset(lengths, %lld, %lld): "
					"[ins_count = %llu]:  tried to index %lld into lengths, "
					"aborting..\n", here, target, ins_count, i
				);
				print_nats(length, ins_count);
				abort();			
			}		
			offset -= length[i];
		}
	}
	return offset;
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

#define MH_SUBSECTIONS_VIA_SYMBOLS 	0x2000
#define	N_SECT 			0xe
#define	N_EXT 			0x01
#define	MH_OBJECT 		0x1
#define REFERENCE_FLAG_DEFINED 	2



int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("error: exactly one source file must be specified."));
	
	const nat min_stack_size = 16384 + 1;
	nat target_arch = no_arch;
	nat output_format = no_output;
	nat should_overwrite = false;
	nat stack_size = min_stack_size;

	memset(values, 255, sizeof values);

	char* string_list[4096] = {0};
	nat string_label[4096] = {0};
	nat string_list_count = 0;	

{ 	struct file files[4096] = {0};
	nat file_count = 1;
	const char* included_files[4096] = {0};
	nat included_file_count = 0;

	{ nat text_length = 0;
	char* text = load_file(argv[1], &text_length);
	files[0].filename = argv[1];
	files[0].text = text;
	files[0].text_length = text_length;
	files[0].index = 0;
	files[0].ct = 0; }

process_file:;
	const nat starting_index = files[file_count - 1].index;
	const nat text_length = files[file_count - 1].text_length;
	char* text = files[file_count - 1].text;
	const char* filename = files[file_count - 1].filename;

	nat word_length = 0, word_start = 0, arg_count = 0, is_immediate = 0;
	byte in_string = 0;

	nat args[max_arg_count] = {0}; 

	for (nat var = 0, op = 0, pc = starting_index; pc < text_length; pc++) {

		if (in_string) {
			op = 0; in_string = 0;
			while (isspace(text[pc])) pc++; 
			const char delim = text[pc];
			nat string_at = ++pc, string_length = 0;
			while (text[pc] != delim) { pc++; string_length++; }
			nat label = (nat) -1;
			if (ins_count and ins[ins_count - 1].op == at) 
				label = ins[ins_count - 1].args[0];
			string_label[string_list_count] = label;
			string_list[string_list_count++] = strndup(text + string_at, string_length);
			struct instruction new = { .op = str, .imm = 0xff };
			new.args[0] = string_length;
			new.args[1] = string_list_count - 1;
			ins[ins_count++] = new;
			goto next_word;
		}

		if (not isspace(text[pc])) {
			if (not word_length) word_start = pc;
			word_length++; 
			if (pc + 1 < text_length) continue;
		} else if (not word_length) continue;

		char* word = strndup(text + word_start, word_length);

		if (not op) {
			if (*word == '(') {
				nat i = word_start + 1, comment = 1;
				while (comment and i < text_length) {
					if (text[i] == '(') comment++;
					if (text[i] == ')') comment--;
					i++;
				}
				if (comment) print_error(
					"unterminated comment",
					filename, text, text_length, 
					word_start, pc);

				pc = i;
				goto next_word;
			}
			
			for (op = 0; op < isa_count; op++) 
				if (not strcmp(word, operations[op])) goto process_op;

			print_error(
				"nonexistent operation",
				filename, text, text_length, word_start, pc
			);
		}
		else if (op == file) goto define_name;
		for (var = var_count; var--;) {
			if (not is_undefined[var] and 
			    not strcmp(word, variables[var])) goto push_argument;
		} 
		if (	(op == lt or op == eq) and arg_count == 2 or
			(op == set) and arg_count == 0 or
			op == do_ or op == at
		) goto define_name;

		nat r = 0, s = 1;
		for (nat i = 0; i < strlen(word); i++) {
			if (word[i] == '0') s <<= 1;
			else if (word[i] == '1') { r += s; s <<= 1; }
			else if (word[i] == '_') continue;
			else goto undefined_var;
			
		}
		is_immediate |= 1 << arg_count;
		var = r;
		goto push_argument;
	undefined_var:
		print_error(
			"undefined variable",
			filename, text, text_length,
			word_start, pc
		);	
	define_name:
		var = var_count;
		variables[var] = word; 
		is_constant[var] = is_compiletime;
		var_count++;
	push_argument: 
		args[arg_count++] = var;
	process_op: 
		if (op == str) { in_string = 1; goto next_word; } 
		else if (op < isa_count and arg_count < arity[op]) goto next_word;
		else if (op == del) {
			if (is_immediate) 
				print_error(
					"expected defined variable, found binary literal",
					filename, text, text_length, word_start, pc
				);
			is_undefined[args[0]] = 1;
			if (is_label[args[0]] and not is_constant[args[0]]) {
				struct instruction new = { .op = op, .state = 1 };
				memcpy(new.args, args, sizeof args);
				memset(args, 0, sizeof args);
				ins[ins_count++] = new;
			}
		} else if (op == file) {
			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				print_error("file has already been included", 
					filename, text, text_length, 
					word_start, pc
				);
			}
			included_files[included_file_count++] = word;
			nat len = 0;
			char* string = load_file(word, &len);
			files[file_count - 1].index = pc;
			files[file_count - 1].ct = is_compiletime;
			files[file_count].filename = word;
			files[file_count].text = string;
			files[file_count].text_length = len;			
			files[file_count++].index = 0;
			var_count--;
			goto process_file;
		} else {

			const nat has_ct_arg0 = (is_immediate & 1) or is_constant[args[0]];
			const nat has_ct_arg1 = (is_immediate & 2) or is_constant[args[1]];
			const nat has_ct_arg2 = (is_immediate & 4) or is_constant[args[2]];

			if (op == do_ or op == at) { 
				if (is_immediate & 1) { 
				error_label_immediate: 
					print_error(
						"expected label argument, found binary literal",
						filename, text, text_length, word_start, pc
					); 
				} else is_label[args[0]] = 1; 
			}
			
			if (op == lt or op == ge or op == ne or op == eq) {
				if (is_immediate & 4) goto error_label_immediate; else is_label[args[2]] = 1;
			}
			if ((op >= set and op <= ld) or op == reg) {
				if (is_immediate & 1) 
					print_error(
						"expected destination variable, found binary literal",
						filename, text, text_length, word_start, pc
					); 
			}

			if ((op == ld or op == st) and not has_ct_arg2) {
				print_error(
					"expected compiletime-known variable or binary literal as load/store width",
					filename, text, text_length, word_start, pc
				); 
			}

			nat is_ct = is_compiletime;
			if (op == emit) is_ct = 0;
			if (op == adr) is_ct = 0;
			if (op == halt) is_ct = 0;
			if (op >= a6_nop and op < isa_count) is_ct = 0;
			if (op == reg) { is_ct = 1; is_constant[args[0]] = 0; } 
			if (not is_ct and op == do_ and has_ct_arg0) is_ct = 1;
			if (is_ct and op == do_ and not has_ct_arg0) is_ct = 0;
			if (not is_ct and op == at and has_ct_arg0) is_ct = 1;
			if (is_ct and op == at and not has_ct_arg0) is_ct = 0;

			if (not is_ct and op >= lt and op <= eq and has_ct_arg0 and has_ct_arg1 and has_ct_arg2) is_ct = 1;
			if (is_ct and op >= lt and op <= eq and not has_ct_arg2) is_ct = 0;
			if (op >= lt and op <= eq and (not has_ct_arg0 or not has_ct_arg1) and has_ct_arg2)
				print_error(
					"compiletime branch instruction must have a compiletime-known condition",
					filename, text, text_length, word_start, pc
				);

			if (not is_ct and (op >= set and op <= sd or op == ld) and has_ct_arg0 and has_ct_arg1) is_ct = 1;
			if (is_ct and (op >= set and op <= sd or op == ld) and not has_ct_arg0) is_ct = 0;
			if (op >= set and op <= sd and has_ct_arg0 and not has_ct_arg1) 
				print_error(
					"instruction cannot store to a compiletime destination with a runtime source",
					filename, text, text_length, word_start, pc
				);
		
			struct instruction new = {
				.op = op, 
				.imm = is_immediate,
				.state = is_ct,
			};
			memcpy(new.args, args, sizeof args);
			is_immediate = 0;
			memset(args, 0, sizeof args);
			ins[ins_count++] = new;
		}
		arg_count = 0; op = 0;
		next_word: word_length = 0;
	}
	file_count--;
	if (file_count) goto process_file; }

	if (not ins_count or ins[ins_count - 1].op != halt) 
		ins[ins_count++] = (struct instruction) { .op = halt, .imm = 0, .state = 0 };






















