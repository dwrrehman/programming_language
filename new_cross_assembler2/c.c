// optimizing assembler, revision of the compiler 
// written on 1202507174.162611 by dwrr
// 1202507255.154607 rewritten to use pc-rel offset immediates instead of labels in rt ins.
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

#define max_variable_count	(1 << 20)
#define max_instruction_count	(1 << 20)
#define max_output_size		(1 << 16)
#define max_memory_size		(1 << 16)
#define max_string_count	(1 << 12)
#define max_file_count		(1 << 12)
#define max_section_count 	(1 << 7)
#define max_arg_count		(1 << 3)

enum all_architectures { 
	no_arch,
	rv64_arch,
	rv32_arch,
	arm64_arch,
	arm32_arch,
	msp430_arch
};

enum all_output_formats {
	no_output,
	macho_executable, 
	macho_object,
	elf_executable, 
	elf_object,
	ti_txt_executable,
	uf2_executable,
	hex_array
};
enum memory_mapped_addresses {
	compiler_return_address,
	compiler_target,
	compiler_format,
	compiler_should_overwrite,
	compiler_should_debug,
	compiler_stack_size,
	compiler_pass,
	compiler_putc
};

enum isa {
	_null___, 
	zero, incr, set, add, sub, mul, div_, 
	ld, st, lt, eq, at, emit, sect, 
	file, del, str, eoi, 
	rr, ri, rs, rb, ru, rj,
	mo, mb,
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
	"zero", "incr", "set", "add", "sub", "mul", "div", 
	"ld", "st", "lt", "eq", "at", "emit", "sect", 
	"file", "del", "str", "eoi",
	"rr", "ri", "rs", "rb", "ru", "rj",
	"mo", "mb",
	"nop", "svc", "mov", "bfm",
	"adc", "addx", "addi", "addr", "adr", 
	"shv", "clz", "rev", "jmp", "bc", "br", 
	"cbz", "tbz", "ccmp", "csel", 
	"ori", "orr", "extr", "ldrl", 
	"memp", "memia", "memi", "memr", 
	"madd", "divr", 
};

static const nat arity[isa_count] = {
	0,	
	1, 1, 2, 2, 2, 2, 2,
	2, 2, 3, 3, 1, 2, 1, 
	1, 1, 0, 0, 
	6, 5, 5, 5, 3, 3,
	8, 2,
	0, 0, 5, 7, 
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

static struct instruction ins[max_instruction_count] = {0};
static nat ins_count = 0;

static char* variables[max_variable_count] = {0};
static nat values[max_variable_count] = {0};
static nat is_ct_label[max_variable_count] = {0};
static nat is_undefined[max_variable_count] = {0};
static nat var_count = 0;

static char* string_list[max_string_count] = {0};
static nat string_label[max_string_count] = {0};
static nat string_list_count = 0;

static nat section_starts[max_section_count] = {0};
static nat section_addresses[max_section_count] = {0};
static nat section_count = 0;

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
		if (i % 2 == 0) puts("");
		printf("[0x%5llx/%5lld]:%lld:%24s:%016llx(%4lld)\t",
			i, i, is_undefined[i], variables[i], values[i], values[i]
		);
	}
	puts("");
}

static void print_instruction(struct instruction this) {
	int max_name_width = 0;
	printf(" %4s  ", operations[this.op]);
	if (this.op == str) {
		printf("\"%s\"", string_list[this.args[1]]);
	} else for (nat a = 0; a < arity[this.op]; a++) {
		char string[4096] = {0};
		if (this.imm & (1 << a)) 
			snprintf(string, sizeof string, "%llx(%llu)", this.args[a], this.args[a]);
		else if (this.args[a] < var_count) 
			snprintf(string, sizeof string, "%s\033[38;5;235m(%llu)\033[0m", 
				variables[this.args[a]], this.args[a]
			);
		else snprintf(string, sizeof string, "(INTERNAL ERROR)(%llu)", this.args[a]);
		printf("%s", string);
		int left_to_print = max_name_width - (int) strlen(string);
		if (left_to_print < 0) left_to_print = 0;
		for (int i = 0; i < left_to_print; i++) putchar(' ');
		putchar(' ');
	}
}

static void print_instructions(void) {
	printf("instructions: (count %llu) {\n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("%4llu: ", i);
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
		if (i % 16 == 0) puts("");
		if (i % 64 == 0) puts("");
		if (i % 4 == 0) putchar(32);
		if (memory[i]) printf("\033[32;1m");
		printf("%02hhx ", memory[i]);
		if (memory[i]) printf("\033[0m");
	}
	puts("\n");
}

static void print_nats(nat* array, nat count) {
	printf("(%llu) { ", count);
	for (nat i = 0; i < count; i++) {
		printf("%lld ", array[i]);
	}
	printf("}");
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


static void print_instruction_window_around(
	nat this,
	const bool should_just_print,
	const char* message
) {
	nat row_count = 0;
	if (not should_just_print) printf("\033[H\033[2J");
	const int64_t window_width = 8;
	const int64_t pc = (int64_t) this;
	for (int64_t i = -window_width; i < window_width; i++) {

		const int64_t here = pc + i;

		if (here >= 0 and here < (int64_t) ins_count and 
			ins[here].op == at) { putchar(10); row_count++; }

		if (not i) printf("\033[48;5;238m");

		if (	here < 0 or 
			here >= (int64_t) ins_count
		) { 
			puts("\033[0m"); 
			row_count++; continue; 
		}

		printf("%4llu  %s%4llu │ ", 
			ins[here].state, 
			not i and ins[here].state ? 
			"\033[32;1m•\033[0m\033[48;5;238m"
			: (ins[here].state ? "\033[32;1m•\033[0m" : " "), 
			here
		);
		if (not i and ins[here].state) printf("\033[48;5;238m");

		if (ins[here].op != at) putchar(9);
		print_instruction(ins[here]);
		if (should_just_print and not i) printf("    \033[0m   <----- \033[31;1m%s\033[0m", message);
		puts("\033[0m");
		row_count++; 
	}

	if (not should_just_print) {
		while (row_count < 2 * window_width + 6) { row_count++; putchar(10); } 
	}
}

static nat get_length(struct instruction new) {
	const nat op = new.op;
	const nat a0 = new.args[0];
	const nat a1 = new.args[1];
	const nat a4 = new.args[4];
	const nat a5 = new.args[5];

	nat length = 4;
	if (op == mo) {
		length = 2;
		if ((a4 == 1 and (a5 != 2 and a5 != 3)) or (a4 == 3 and not a5)) length += 2;
		if (a1 == 1) length += 2;
	}
	else if (op == mb) length = 2;
	else if (op == sect) length = 0;
	else if (op == emit) length = a0;
	else length = 4;
	return length;
}

int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("error: exactly one source file must be specified."));
	
	const nat min_stack_size = 16384 + 1;
	nat target_arch = no_arch;
	nat output_format = no_output;
	nat should_overwrite = false;
	nat stack_size = min_stack_size;

{ 	struct file files[max_file_count] = {0};
	nat file_count = 1;
	const char* included_files[max_file_count] = {0};
	nat included_file_count = 0;

	{ nat text_length = 0;
	char* text = load_file(argv[1], &text_length);
	files[0].filename = argv[1];
	files[0].text = text;
	files[0].text_length = text_length;
	files[0].index = 0; }

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
					word_start, pc
				);
				pc = i;
				goto next_word;
			}

			for (op = 0; op < isa_count; op++) 
				if (not strcmp(word, operations[op])) goto process_op;

			for (var = var_count; var--;) {
				if (not is_ct_label[var]) continue;
				if (not strcmp(word, variables[var])) {
					ins[ins_count++] = (struct instruction) { 
						.op = eq, .imm = 3, .args = { 0, 0, var }
					};
					goto finish_operation;
				}
			}
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

		if ( (op == at or op == zero or op == set or op == ld) and arg_count == 0 or
			(op == lt or op == eq) and arg_count == 2
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
		if (	op == set and arg_count == 1 or
			op == st  and arg_count == 1 or
			(op > eoi and op < isa_count)
		) goto define_name;
		print_error(
			"undefined variable",
			filename, text, text_length, word_start, pc
		);
	define_name:
		var = var_count;
		variables[var] = word;
		values[var] = (nat) -1;
		var_count++;
	push_argument: 
		args[arg_count++] = var;
	process_op:
		if (op == eoi) break;
		if (op == str) { in_string = 1; goto next_word; }
		else if (op < isa_count and arg_count < arity[op]) goto next_word;
		else if (op == del) is_undefined[*args] = 1;
		else if (op == file) {
			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				print_error("file has already been included", 
					filename, text, text_length, word_start, pc
					);
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
			if (op == at) values[*args] = ins_count;
			else if (op == lt or op == eq) is_ct_label[args[2]] = 1;
			struct instruction new = { .op = op, .imm = is_immediate };
			is_immediate = 0;
			memcpy(new.args, args, sizeof args);
			memset(args, 0, sizeof args);
			ins[ins_count++] = new;
		}
		finish_operation: arg_count = 0; op = 0;
		next_word: word_length = 0;
	}
	file_count--;
	if (file_count) goto process_file; }

	if (debug) {
		print_instructions();
		print_dictionary();
		puts("parsing finished.");
		getchar();
	}

	{ nat memory[max_memory_size] = {0};
	struct instruction* rt_ins = calloc(max_instruction_count, sizeof(struct instruction));
	nat rt_ins_count = 0, total_byte_count = 0;

	for (nat pass = 0; pass < 2; pass++) {

	memory[compiler_pass] = pass;
	if (pass == 1) { rt_ins_count = 0; total_byte_count = 0; } 

	for (nat pc = 0; pc < ins_count; pc++) {
		nat op = ins[pc].op, imm = ins[pc].imm;

		if (memory[compiler_should_debug]) {
			print_instruction_window_around(pc, 0, "");
			printf("\033[32;1m [ PASS = %llu ] \033[0m \n", pass);			
			read_loop: 
			printf("[%llu]:ready: ", pass); fflush(stdout);
			char input[256] = {0}; 
			read(0, input, sizeof input);
			if (not strcmp(input, "\n")) goto done_read_loop;
			else if (not strcmp(input, ":this\n")) {
				print_instruction(ins[pc]); puts("");
			} else if (not strcmp(input, ":dictionary\n")) {
				print_dictionary(); puts("");
			} else if (not strcmp(input, ":list\n")) {
				for (nat i = 0; i < rt_ins_count; i++) {
					putchar(9); 
					print_instruction(rt_ins[i]); 
					puts("");
				}
			} else if (not strcmp(input, ":memory\n")) {
				dump_hex((uint8_t*) memory, 256);
			} else {
				if (strlen(input)) input[strlen(input) - 1] = 0;
				for (nat i = 0; i < var_count; i++) {
					if (strcmp(variables[i], input)) continue;
					printf("[0x%5llx/%5lld]:%lld:"
						"%24s:%016llx(%4lld)\n",
						i, i, is_undefined[i], 
						variables[i], values[i], 
						values[i]
					);
				}
			}
			goto read_loop; 
			done_read_loop:;
		}
		nat arg0 = ins[pc].args[0];
		nat arg1 = ins[pc].args[1];
		nat arg2 = ins[pc].args[2];
		nat val0 = imm & 1 ? arg0 : values[arg0];
		nat val1 = imm & 2 ? arg1 : values[arg1];
		nat val2 = imm & 4 ? arg2 : values[arg2];

		if ((op == lt or op == eq) and val2 >= ins_count) {
			printf("error: at pc %llu invalid jump address: 0x%016llx\n", pc, val2);
			print_instruction_window_around(pc, 1, "error");
			abort();
		}

		if (op == st and val0 >= max_memory_size) {
			printf("error: at pc %llu invalid address given to store to compiletime memory: 0x%016llx\n", pc, val0);
			print_instruction_window_around(pc, 1, "error");
			abort();
		}

		if (op == ld and val1 >= max_memory_size) {
			printf("error: at pc %llu invalid address given to load from compiletime memory: 0x%016llx\n", pc, val1);
			print_instruction_window_around(pc, 1, "error");
			abort();
		}
	
		if (op == str) {
			for (nat s = 0; s < arg0; s++) { 
				rt_ins[rt_ins_count++] = (struct instruction) { 
					.op = emit, .imm = 3,
					.args = { 1, (nat) string_list[arg1][s] }
				}; total_byte_count++;
			}

		} else if (op > eoi or op == emit or op == sect) {
			struct instruction new = ins[pc];
			new.imm = 0xffff;
			for (nat a = 0; a < arity[op]; a++) {
				nat this = ins[pc].args[a];
				if ((imm >> a) & 1) new.args[a] = this;
				else new.args[a] = values[this];
			}
			rt_ins[rt_ins_count++] = new;
			total_byte_count += get_length(new);
		}		
		else if (op == zero) values[arg0] = 0;
		else if (op == incr) values[arg0] += 1;
		else if (op == set)  values[arg0] = val1;
		else if (op == add)  values[arg0] += val1;
		else if (op == sub)  values[arg0] -= val1;
		else if (op == mul)  values[arg0] *= val1;
		else if (op == div_) values[arg0] /= val1;
		else if (op == ld)   values[arg0] = memory[val1];
		else if (op == st)   memory[val0] = val1;
		else if (op == at)   values[arg0] = is_ct_label[arg0] ? pc : total_byte_count;
		else if (op == lt) { if (val0  < val1) { *memory = pc; pc = val2; } }
		else if (op == eq) { if (val0 == val1) { *memory = pc; pc = val2; } }
		else { 
			printf("CTE: fatal internal error: "
				"unknown instruction executed: \"%s\", (opcode %llu).\n", 
				operations[op], op
			); 
			abort(); 
		}
		if (op == st and val0 == compiler_putc) { char c = (char) val1; write(1, &c, 1); } 
	}}

	memcpy(ins, rt_ins, rt_ins_count * sizeof(struct instruction));
	ins_count = rt_ins_count;
	for (nat i = 0; i < ins_count; i++) ins[i].imm = (nat) -1;

	target_arch = memory[compiler_target];
	output_format = memory[compiler_format];
	should_overwrite = memory[compiler_should_overwrite];
	stack_size = memory[compiler_stack_size]; }

	if (target_arch == no_arch and not ins_count) exit(0);
	if (target_arch == no_arch and ins_count) {
		puts("error: target architecture no_arch was specified, but encountered runtime instructions");
		abort();
	}

	if (target_arch == msp430_arch and stack_size) { 
		puts("error: nonzero stack size for msp430 is not permitted"); 
		abort();

	} else if (target_arch == arm64_arch and stack_size < min_stack_size) {
		puts("warning: stack size less than the minimum size for arm64");
	}

	if (debug) {
		print_dictionary();
		print_instructions();
		puts("CTE finished.");
	}

	const char* output_filename = "output_file_from_compiler";
	if (output_format == uf2_executable) output_filename = "output_file_from_compiler.uf2";

	print_dictionary();
	print_instructions();
	puts("CTE finished.");
	puts("generating final machine code binary...");

	uint8_t* my_bytes = NULL;
	nat my_count = 0;

	if (target_arch == rv32_arch) goto rv32_generate_machine_code;
	if (target_arch == rv64_arch) goto rv32_generate_machine_code;
	if (target_arch == arm64_arch) goto arm64_generate_machine_code;
	if (target_arch == msp430_arch) goto msp430_generate_machine_code;
	puts("unknown target architecture"); abort();

rv32_generate_machine_code:;
	for (nat i = 0; i < ins_count; i++) {

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("");
			dump_hex(my_bytes, my_count);
			getchar();
		}

		const nat op = ins[i].op;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1]; 
		const nat a2 = ins[i].args[2]; 
		const nat a3 = ins[i].args[3]; 
		const nat a4 = ins[i].args[4]; 
		const nat a5 = ins[i].args[5]; 

		if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, a1);
			else if (a0 == 4) {
				if (ins[i].args[1] >= (1LLU << 32LLU)) { puts("error: invalid emit u32 data argument"); abort(); }
				insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			}

			else if (a0 == 2) {
				if (ins[i].args[1] >= (1LLU << 16LLU)) { puts("error: invalid emit u16 data argument"); abort(); }
				insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			}

			else if (a0 == 1) {
				if (ins[i].args[1] >= (1LLU << 8LLU)) { puts("error: invalid emit u8 data argument"); abort(); }
				insert_u8 (&my_bytes, &my_count, (uint8_t) a1);

			} else { puts("error: invalid emit size, must be 1, 2, 4, or 8."); abort(); } 

		} else if (op == sect) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;

		} else if (op == ri) {
			if (a0 >= (1LLU << 7LLU)) { 
				puts("risc-v: ri: arg0: invalid 7-bit major op code"); 
				print_instruction_window_around(i, 1, "invalid argument 0"); 
				abort(); 
			} 

			if (a1 >= (1LLU << 3LLU)) { 
				puts("risc-v: ri: arg1: invalid 3-bit minor op code"); 
				print_instruction_window_around(i, 1, "invalid argument 1"); 
				abort(); 
			} 

			if (a2 >= (1LLU << 5LLU)) { 
				puts("risc-v: ri: arg2: invalid destination register"); 
				print_instruction_window_around(i, 1, "invalid argument 2"); 
				abort();
			} 

			if (a3 >= (1LLU << 5LLU)) { 
				puts("risc-v: ri: arg3: invalid source register"); 
				print_instruction_window_around(i, 1, "invalid argument 3"); 
				abort(); 
			}

			if (a4 >= (1LLU << 12LLU)) { 
				puts("risc-v: ri: arg4: invalid 12-bit immediate"); 
				print_instruction_window_around(i, 1, "invalid argument 4"); 
				abort(); 
			}

			const nat word = 
				(a4  << 20U) | 
				(a3 << 15U) | 
				(a1 << 12U) | 
				(a2 <<  7U) | 
				(a0 <<  0U) ;

			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == rr) {	
			if (a0 >= (1LLU << 7LLU)) { puts("error"); abort(); } 
			if (a1 >= (1LLU << 3LLU)) { puts("error"); abort(); } 
			if (a2 >= (1LLU << 5LLU)) { puts("error"); abort(); } 
			if (a3 >= (1LLU << 5LLU)) { puts("error"); abort(); } 
			if (a4 >= (1LLU << 5LLU)) { puts("error"); abort(); } 
			if (a5 >= (1LLU << 7LLU)) { puts("error"); abort(); } 

			const nat word = 
				(a5 << 25U) | 
				(a4 << 20U) | 
				(a3 << 15U) | 
				(a1 << 12U) | 
				(a2 <<  7U) | 
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == rs) {

			if (a0 >= (1LLU << 7LLU)) { puts("error"); abort(); } 
			if (a1 >= (1LLU << 3LLU)) { puts("error"); abort(); } 
			if (a2 >= (1LLU << 5LLU)) { puts("error"); abort(); } 
			if (a3 >= (1LLU << 5LLU)) { puts("error"); abort(); } 
			if (a4 >= (1LLU << 12LLU)) { puts("error"); abort(); } 

			/*nat k = a4;
			if (k & is_label) {
				const nat im = calculate_offset(lengths, i - 1, k) & 0xffff; // TODO: error???
				k = im;
			} puts("fix me"); abort(); */

			const nat word = 
				(((a4 >> 5) & 0x3f) << 25U) | 
				(a3 << 20U) | 
				(a2 << 15U) | 
				(a1 << 12U) | 
				((a4 & 0x1f) <<  7U) | 
				(a0 << 0U) ;
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == ru) {

			if (a0 >= (1LLU << 7LLU)) { puts("error"); abort(); } 
			if (a1 >= (1LLU << 5LLU)) { puts("error"); abort(); } 
			if (a2 >= (1LLU << 20LLU)) { puts("error"); abort(); } 
			
			nat im = (a2 << 12) & 0xFFFFF000;
			//if (a0 == 0x17) im = calculate_offset(lengths, i, a2) & 0xFFFFF000;

			const nat word =
				(im) |
				(a1 <<  7U) |
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == rb) {

			if (a0 >= (1LLU << 7LLU)) { puts("error"); abort(); } 
			if (a1 >= (1LLU << 3LLU)) { puts("error"); abort(); } 
			if (a2 >= (1LLU << 5LLU)) { puts("error"); abort(); } 
			if (a3 >= (1LLU << 5LLU)) { puts("error"); abort(); } 
			if (a4 >= (1LLU << 12LLU)) { puts("error"); abort(); } 
			
			//nat im = (u32) calculate_offset(lengths, i, a4);
			//if ((int32_t) im >= (1 << 12)) abort();
			//if ((int32_t) im < -(1 << 12)) abort();
			//im &= 0x1FFF;
			nat im = (a4 - my_count) & 0x1fff;
			//if (a4 & 0x8000000000000000) im = a4;

			const nat bit4_1  = (im >> 1) & 0xF;
			const nat bit10_5 = (im >> 5) & 0x3F;
			const nat bit11   = (im >> 11) & 0x1;
			const nat bit12   = (im >> 12) & 0x1;
			const nat lo = (bit4_1 << 1) | bit11;
			const nat hi = (bit12 << 6) | bit10_5;
	
			const nat word =
				(hi << 25U) |
				(a3 << 20U) |
				(a2 << 15U) |
				(a1 << 12U) |
				(lo <<  7U) |
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == rj) {

			if (a0 >= (1LLU << 7LLU)) { puts("error"); abort(); } 
			if (a1 >= (1LLU << 5LLU)) { puts("error"); abort(); } 
			if (a2 >= (1LLU << 21LLU)) { puts("error"); abort(); } // TODO: this check is not right... it needs to be relative....

			//nat im = (u32) calculate_offset(lengths, i, a2);
			//nat im = a2 - i ;

			nat im = (a2 - my_count) & 0x1fffff;

			//if ((int32_t) im >= (1 << 21)) abort();
			//if ((int32_t) im < -(1 << 21)) abort();
			//im &= 0x1FFFFF;

			const nat bit10_1  = (im >> 1U) & 0x3FF;
			const nat bit19_12 = (im >> 12U) & 0xFF;
			const nat bit11   = (im >> 11U) & 0x1;
			const nat bit20   = (im >> 20U) & 0x1;
			const nat offset = (bit20 << 31U) | (bit10_1 << 21U) | (bit11 << 20U) | (bit19_12 << 12U);

			const nat word =
				(offset) |
				(a1 <<  7U) |
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else {
			printf("could not generate machine code for instruction: %llu\n", op);
			abort();
		}
	}
	goto finished_generation;

msp430_generate_machine_code:;

	{
	for (nat i = 0; i < ins_count; i++) {

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("");
			dump_hex(my_bytes, my_count);
			getchar();	
		}

		const nat op = ins[i].op;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1];
		const nat a2 = ins[i].args[2];
		const nat a3 = ins[i].args[3];
		const nat a4 = ins[i].args[4];
		const nat a5 = ins[i].args[5];
		const nat a6 = ins[i].args[6];
		const nat a7 = ins[i].args[7];

		if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, a1);
			else if (a0 == 4) {
				if (ins[i].args[1] >= (1LLU << 32LLU)) { puts("error: invalid emit u32 data argument"); abort(); }
				insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			}

			else if (a0 == 2) {
				if (ins[i].args[1] >= (1LLU << 16LLU)) { puts("error: invalid emit u16 data argument"); abort(); }
				insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			}

			else if (a0 == 1) {
				if (ins[i].args[1] >= (1LLU << 8LLU)) { puts("error: invalid emit u8 data argument"); abort(); }
				insert_u8 (&my_bytes, &my_count, (uint8_t) a1);

			} else { puts("error: invalid emit size, must be 1, 2, 4, or 8."); abort(); } 

		} else if (op == sect) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;

		} else if (op == mb) { 
			const nat offset = 0x3FF & a1;//((calculate_offset(lengths, i + 1, a1) >> 1));
			const nat word =  ((1LLU << 13LLU) | (a0 << 10LLU) | (offset));
			insert_u16(&my_bytes, &my_count, (u16) word);
		}
		else if (op == mo) {  
			nat word = (
				(a0 << 12LLU) | (a5 << 8LLU) | (a1 << 7LLU) | 
				(a7 << 6LLU) | (a4 << 4LLU) | (a2)
			);
			insert_u16(&my_bytes, &my_count, (u16) word);
			if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
				or (a4 == 3 and not a5)) insert_u16(&my_bytes, &my_count, (u16) a6);						
			if (a1 == 1) insert_u16(&my_bytes, &my_count, (u16) a3);
		} else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}}

	goto finished_generation;

arm64_generate_machine_code:;

	for (nat i = 0; i < ins_count; i++) {

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("");
			dump_hex(my_bytes, my_count);
			getchar();
		}

		const nat op = ins[i].op;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1];
		const nat a2 = ins[i].args[2];
		const nat a3 = ins[i].args[3];
		const nat a4 = ins[i].args[4];
		const nat a5 = ins[i].args[5];
		const nat a6 = ins[i].args[6];
		const nat a7 = ins[i].args[7];

		if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, a1);
			else if (a0 == 4) {
				if (ins[i].args[1] >= (1LLU << 32LLU)) { puts("error: invalid emit u32 data argument"); abort(); }
				insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			}

			else if (a0 == 2) {
				if (ins[i].args[1] >= (1LLU << 16LLU)) { puts("error: invalid emit u16 data argument"); abort(); }
				insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			}

			else if (a0 == 1) {
				if (ins[i].args[1] >= (1LLU << 8LLU)) { puts("error: invalid emit u8 data argument"); abort(); }
				insert_u8 (&my_bytes, &my_count, (uint8_t) a1);

			} else { puts("error: invalid emit size, must be 1, 2, 4, or 8."); abort(); } 

		} else if (op == sect) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;

		} else if (op == clz) { puts("clz is unimplemented currently, lol"); abort(); }
		else if (op == rev) { puts("rev is unimplemented currently, lol"); abort(); }
		else if (op == extr) { puts("extr is unimplemented currently, lol"); abort(); }
		else if (op == ldrl) { puts("ldrl is unimplemented currently, lol"); abort(); }

		else if (op == nop) insert_u32(&my_bytes, &my_count, 0xD503201F);
		else if (op == svc) insert_u32(&my_bytes, &my_count, 0xD4000001);

		else if (op == br) {
			nat l = a2 ? 2 : a1 ? 1 : 0;
			const nat word = 
				(0x6BU << 25U) | (l << 21U) | 
				(0x1FU << 16U) | (a0 << 5U);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == adc) {
			const nat word = 
				(a5 << 31U) | (a4 << 30U) | (a3 << 29U) | 
				(0xD0 << 21U) | (a2 << 16U) | (0 << 19U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == shv) {
			uint32_t op2 = 8;
			if (a3 == 0) op2 = 8;
			if (a3 == 1) op2 = 9;
			if (a3 == 2) op2 = 10;
			if (a3 == 3) op2 = 11;
			const nat word = 
				(a4 << 31U) | (0 << 30U) | 
				(0 << 29U) | (0xD6 << 21U) | 
				(a2 << 16U) | (op2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == mov) {
			const nat word = 
				(a4 << 31U) | (a3 << 29U) | (0x25U << 23U) |
				(a2 << 21U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == bc) {
			const nat offset = 0x7ffff & (a1 - my_count);
			const nat word = (0x54U << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == jmp) {
			const nat offset = 0x3ffffff & (a1 - my_count);
			const nat word = (a0 << 31U) | (0x5U << 26U) | (offset);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == adr) {
			nat o1 = a2;
			nat count = a1 - my_count;
			if (a2) count /= 4096;
			const nat offset = 0x1fffff & count;
			const nat lo = offset & 3, hi = offset >> 2;
			const nat word = 
				(o1 << 31U) | (lo << 29U) | (0x10U << 24U) |
				(hi << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == cbz) {
			const nat offset = 0x7ffff & (a1 - my_count);
			const nat word = 
				(a3 << 31U) | (0x1AU << 25U) | 
				(a2 << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == tbz) {
			const nat b40 = a1 & 0x1F;
			const nat b5 = a1 >> 5;
			const nat offset = 0x3fff & (a2 - my_count);
			const nat word = 
				(b5 << 31U) | (0x1BU << 25U) | (a3 << 24U) |
				(b40 << 19U) |(offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == ccmp) {
			const nat word = 
				(a6 << 31U) | (a4 << 30U) | (0x1D2 << 21U) | 
				(a3 << 16U) | (a0 << 12U) | (a2 << 11U) | 
				(a1 << 5U) | (a5); 
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == addi) {
			const nat word = 
				(a6 << 31U) | (a5 << 30U) | (a4 << 29U) | 
				(0x22 << 23U) | (a3 << 22U) | (a2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == addr) {
			const nat word = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0xB << 24U) | (a3 << 22U) | (a2 << 16U) |
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == addx) {
			const nat word = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0x59 << 21U) | (a2 << 16U) | (a3 << 13U) | 
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == divr) {
			const nat word = 
				(a4 << 31U) | (0xD6 << 21U) | (a2 << 16U) |
				(1 << 11U) | (a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == csel) {
			const nat word = 
				(a6 << 31U) | (a5 << 30U) | (0xD4 << 21U) | 
				(a2 << 16U) | (a3 << 12U) | (a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == madd) {
			const nat word = 
				(a7 << 31U) | (0x1B << 24U) | (a5 << 23U) | 
				(a4 << 21U) | (a2 << 16U) | (a6 << 15U) | 
				(a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == bfm) {
			nat imms = 0, immr = 0;
			if (not a2) { imms = a3 + a4 - 1; immr = a3; } 
			else { imms = a4 - 1; immr = (a6 ? 64 : 32) - a3; }
			const nat word = (a6 << 31U) | (a5 << 29U) | 	
				(0x26U << 23U) | (a6 << 22U) | (immr << 16U) |
				(imms << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == ori) { // TODO: implement this instruction

			puts("TODO: please implemented the ori instruction: "
				"this is the last instruction we need to implement "
				"and then we are done with iplemementing the arm64 backend!"
			);

			abort();

		} else if (op == orr) {
			const nat word = 
				(a7 << 31U) | (a0 << 29U) | (10 << 24U) | 
				(a4 << 22U) | (a6 << 21U) | (a3 << 16U) | 
				(a5 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == memp) {
			const nat word = 
				(a1 << 30U) | (0x14 << 25U) | (a6 << 23U) | (a0 << 22U) | 
				(a5 << 15U) | (a3 << 10U) | (a4 << 5U) | (a2);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == memi) {
			const nat is_load = (a0 >> 2) & 1;
			const nat is_signed = (a0 >> 1) & 1;
			const nat is_64_dest = (a0 >> 0) & 1;
			nat opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const nat word = 
				(a4 << 30U) | (0x39 << 24U) | (opc << 22U) |
				(a3 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == memia) { 			
			const nat is_load = (a0 >> 2) & 1;
			const nat is_signed = (a0 >> 1) & 1;
			const nat is_64_dest = (a0 >> 0) & 1;
			nat opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const nat word = 
				(a4 << 30U) | (0x38 << 24U) | (opc << 22U) | (a3 << 12U) | 
				(a5 << 11U) | (1 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, (u32) word);

		} else if (op == memr) { 
			const nat S = (a4 >> 2) & 1, option = a4 & 3;
			nat opt = 0;
			if (option == 0) opt = 2;
			else if (option == 1) opt = 3;
			else if (option == 2) opt = 6;
			else if (option == 3) opt = 7;
			else abort();
			const nat is_load = (a0 >> 2) & 1;
			const nat is_signed = (a0 >> 1) & 1;
			const nat is_64_dest = (a0 >> 0) & 1;
			nat opc = 0;
			if (not is_load) opc = 0;
			else if (a5 == 3) opc = 1;
			else if (a5 == 2 and is_signed) opc = 2;
			else if (a5 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const nat word = 
				(a5 << 30U) | (0x38 << 24U) | (opc << 22U) |
				(1 << 21U) | (a3 << 16U) | (opt << 13U) |
				(S << 12U) | (2 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, (u32) word);
		}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}

finished_generation:;

	char debug_string[4096] = {0};
	puts("done: byte generation successful.");
	puts("final_bytes:");
	dump_hex(my_bytes, my_count);
	printf("info: generating output file with format #%llu...\n", output_format);

	if (output_format == no_output) goto print_debug_output;
	if (output_format == hex_array) goto generate_hex_array_output;
	if (output_format == ti_txt_executable) goto generate_ti_txt_executable;
	if (output_format == uf2_executable) goto generate_uf2_executable;
	if (output_format == macho_executable) goto generate_macho_executable;
	if (output_format == macho_object) goto generate_macho_object;
	if (output_format == elf_executable) abort();
	if (output_format == elf_object) abort();
	puts("unknown outputformat"); abort();

print_debug_output:;
	printf("debug: executable bytes: (%llu bytes)\n", my_count);
	for (nat i = 0; i < my_count; i++) {
		if (i % 16 == 0) printf(" ");
		if (i % 32 == 0) puts("");
		if (my_bytes[i]) printf("\033[32;1m");
		printf("%02hhx ", my_bytes[i]);
		if (my_bytes[i]) printf("\033[0m");
	}
	puts("");
	goto finished_outputting;

generate_hex_array_output:;

	{ nat len = 0; // TODO: rewrite this using insert_byte().
	char out[max_output_size] = {0};

	len += (nat) snprintf(out + len, sizeof out, "// executable autogenerated by my compiler\n");
	len += (nat) snprintf(out + len, sizeof out, "export const executable = [\n\t");
	for (nat i = 0; i < my_count; i++) {
		if (i and (i % 8 == 0)) len += (nat) snprintf(out + len, sizeof out, "\n\t");
		len += (nat) snprintf(out + len, sizeof out, "0x%02hhX,", my_bytes[i]);
	}	
	len += (nat) snprintf(out + len, sizeof out, "\n];\n");

	printf("about to write out: \n-------------------\n<<<%.*s>>>\n----------------\n", (int) len, out);
	printf("writing hex txt executable...\n");
	fflush(stdout);

	if (not access(output_filename, F_OK)) {
		printf("file exists. do you wish to remove the previous one? (y/n) ");
		fflush(stdout);
		if (should_overwrite or getchar() == 'y') {
			printf("file %s was removed.\n", output_filename);
			int r = remove(output_filename);
			if (r < 0) { perror("remove"); exit(1); }
		} else {
			puts("not removed, compilation aborted.");
		}
	}

	{ int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (file < 0) { perror("open: could not create executable file"); exit(1); }
	write(file, out, len);
	close(file); }

	printf("hex-txt: wrote %llu bytes to file %s.\n", len, output_filename);
	snprintf(debug_string, sizeof debug_string, "./rv_dis/run print %s", output_filename);
	system(debug_string); }
	goto finished_outputting;

generate_macho_executable:;

	{while (my_count % 16) insert_byte(&my_bytes, &my_count, 0);
	uint8_t* data = NULL;
	nat count = 0;	

	insert_u32(&data, &count, MH_MAGIC_64);
	insert_u32(&data, &count, CPU_TYPE_ARM | (int)CPU_ARCH_ABI64);
	insert_u32(&data, &count, CPU_SUBTYPE_ARM64_ALL);
	insert_u32(&data, &count, MH_EXECUTE);
	insert_u32(&data, &count, 11);
	insert_u32(&data, &count, 72 + (72 + 80) + 72 + 24 + 80 + 32 + 24 + 32 + 16 + 24 +  (24 + 32) ); 
	insert_u32(&data, &count, MH_NOUNDEFS | MH_PIE | MH_DYLDLINK | MH_TWOLEVEL);
	insert_u32(&data, &count, 0);

	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'P', 'A', 'G', 'E', 'Z', 'E', 
		'R', 'O',  0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0x0000000100000000);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0);
	for (nat _ = 0; _ < 4; _++)  insert_u32(&data, &count, 0);

	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72 + 80);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'T', 'E', 'X', 'T',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0x0000000100000000);
	insert_u64(&data, &count, 0x0000000000004000);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 16384);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_EXECUTE);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_EXECUTE);
	insert_u32(&data, &count, 1);
	insert_u32(&data, &count, 0);

	insert_bytes(&data, &count, (char[]){
		'_', '_', 't', 'e', 'x', 't',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'T', 'E', 'X', 'T',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);

	insert_u64(&data, &count, 0x0000000100000000 + 16384 - my_count);
	insert_u64(&data, &count, my_count); 
	insert_u32(&data, &count, 16384 - (uint32_t) my_count);
	insert_u32(&data, &count, 4); 
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0); 
	insert_u32(&data, &count, S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS);
	for (nat _ = 0; _ < 3; _++)  insert_u32(&data, &count, 0);

	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'L', 'I', 'N', 'K', 'E', 'D', 
		'I', 'T',  0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0x0000000100004000);
	insert_u64(&data, &count, 0x0000000000004000);
	insert_u64(&data, &count, 16384);
	insert_u64(&data, &count, 800);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_WRITE);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_WRITE);
	for (nat _ = 0; _ < 2; _++)  insert_u32(&data, &count, 0);

	insert_u32(&data, &count, LC_SYMTAB);
	insert_u32(&data, &count, 24); 
	for (nat _ = 0; _ < 4; _++) insert_u32(&data, &count, 0);

	insert_u32(&data, &count, LC_DYSYMTAB);
	insert_u32(&data, &count, 80); 
	for (nat _ = 0; _ < 18; _++)  insert_u32(&data, &count, 0);

	insert_u32(&data, &count, LC_LOAD_DYLINKER);
	insert_u32(&data, &count, 32);
	insert_u32(&data, &count, 12);
	insert_bytes(&data, &count, (char[]){
		'/', 'u', 's', 'r', '/', 'l', 'i', 'b', 
		'/', 'd', 'y',  'l', 'd',  0,   0,   0
	}, 16);
	insert_u32(&data, &count, 0);

	insert_u32(&data, &count, LC_UUID);
	insert_u32(&data, &count, 24); 
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());

	insert_u32(&data, &count, LC_BUILD_VERSION);
	insert_u32(&data, &count, 32);
	insert_u32(&data, &count, PLATFORM_MACOS);
	insert_u32(&data, &count, 13 << 16);
	insert_u32(&data, &count, (13 << 16) | (3 << 8));
	insert_u32(&data, &count, 1);
	insert_u32(&data, &count, TOOL_LD);
	insert_u32(&data, &count, (857 << 16) | (1 << 8));

	insert_u32(&data, &count, LC_SOURCE_VERSION);
	insert_u32(&data, &count, 16);
	insert_u64(&data, &count, 0);

	insert_u32(&data, &count, LC_MAIN);
	insert_u32(&data, &count, 24);
	insert_u64(&data, &count, 16384 - my_count);
	insert_u64(&data, &count, stack_size); // put stack size here

	insert_u32(&data, &count, LC_LOAD_DYLIB);
	insert_u32(&data, &count, 24 + 32);
	insert_u32(&data, &count, 24);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, (1319 << 16) | (100 << 8) | 3);
	insert_u32(&data, &count, 1 << 16);
	insert_bytes(&data, &count, (char[]){
		'/', 'u', 's', 'r', '/', 'l', 'i', 'b', 
		'/', 'l', 'i', 'b', 'S', 'y', 's', 't',
		'e', 'm', '.', 'B', '.', 'd', 'y', 'l', 
		'i', 'b',  0,   0,   0,   0,   0,   0
	}, 32);

	while (count < 16384 - my_count) insert_byte(&data, &count, 0);
	for (nat i = 0; i < my_count; i++) insert_byte(&data, &count, my_bytes[i]);
	for (nat i = 0; i < 800; i++) insert_byte(&data, &count, 0);

	puts("");
	puts("bytes: ");
	for (nat i = 0; i < count; i++) {
		if (i % 32 == 0) puts("");
		if (data[i]) printf("\033[32;1m");
		printf("%02hhx ", data[i]);
		if (data[i]) printf("\033[0m");
	}
	puts("");

	if (not access(output_filename, F_OK)) {
		printf("file exists. do you wish to remove the previous one? (y/n) ");
		fflush(stdout);
		if (should_overwrite or getchar() == 'y') {
			printf("file %s was removed.\n", output_filename);
			int r = remove(output_filename);
			if (r < 0) { perror("remove"); exit(1); }
		} else {
			puts("not removed, compilation aborted.");
		}
	}

	{ int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("could not create executable file"); exit(1); }
	int r = fchmod(file, 0777);
	if (r < 0) { perror("could not make the output file executable"); exit(1); }
	write(file, data, count);
	close(file); }
	printf("mach-o: wrote %llu bytes to file %s.\n", count, output_filename);

	char codesign_string[4096] = {0};
	snprintf(codesign_string, sizeof codesign_string, "codesign -s - %s", output_filename);
	system(codesign_string);


	// debugging:
	snprintf(codesign_string, sizeof codesign_string, "otool -htvxVlL %s", output_filename);
	system(codesign_string);
	snprintf(codesign_string, sizeof codesign_string, "objdump -D %s", output_filename);
	system(codesign_string);

	printf("info: successsfully generated executable: %s\n", output_filename); } 
	goto finished_outputting;

generate_macho_object:;

{  

struct nlist_64 {  // TODO: rewrite this to not use structs. 
    union { uint32_t  n_strx; } n_un;
    uint8_t n_type;
    uint8_t n_sect; 
    uint16_t n_desc; 
    uint64_t n_value;  
};

struct mach_header_64 {
	uint32_t	magic;
	int32_t		cputype;
	int32_t		cpusubtype;
	uint32_t	filetype;
	uint32_t	ncmds;
	uint32_t	sizeofcmds;
	uint32_t	flags;
	uint32_t	reserved;
};

struct segment_command_64 {
	uint32_t	cmd;
	uint32_t	cmdsize;
	char		segname[16];
	uint64_t	vmaddr;
	uint64_t	vmsize;
	uint64_t	fileoff;
	uint64_t	filesize;
	int32_t		maxprot;
	int32_t		initprot;
	uint32_t	nsects;
	uint32_t	flags;
};

struct section_64 {
	char		sectname[16];
	char		segname[16];
	uint64_t	addr;
	uint64_t	size;
	uint32_t	offset;
	uint32_t	align;
	uint32_t	reloff;
	uint32_t	nreloc;
	uint32_t	flags;
	uint32_t	reserved1;
	uint32_t	reserved2;
	uint32_t	reserved3;
};

struct symtab_command {
	uint32_t	cmd;
	uint32_t	cmdsize;
	uint32_t	symoff;
	uint32_t	nsyms;
	uint32_t	stroff;
	uint32_t	strsize;
};

	struct mach_header_64 header = {0};
	header.magic = MH_MAGIC_64;
	header.cputype = (int)CPU_TYPE_ARM | (int)CPU_ARCH_ABI64;
	header.cpusubtype = (int) CPU_SUBTYPE_ARM64_ALL;
	header.filetype = MH_OBJECT;
	header.ncmds = 2;
	header.sizeofcmds = 0;
	header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;

	header.sizeofcmds = 	sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	struct segment_command_64 segment = {0};
	strncpy(segment.segname, "__TEXT", 16);
	segment.cmd = LC_SEGMENT_64;
	segment.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64);
	segment.maxprot =  (VM_PROT_READ | VM_PROT_EXECUTE);
	segment.initprot = (VM_PROT_READ | VM_PROT_EXECUTE);
	segment.nsects = 1;
	segment.vmaddr = 0;
	segment.vmsize = my_count;
	segment.filesize = my_count;

	segment.fileoff = 	sizeof(struct mach_header_64) + 
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	struct section_64 section = {0};
	strncpy(section.sectname, "__text", 16);
	strncpy(section.segname, "__TEXT", 16);
	section.addr = 0;
	section.size = my_count;	
	section.align = 3;
	section.reloff = 0;
	section.nreloc = 0;
	section.flags = S_ATTR_PURE_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS;

	section.offset = 	sizeof(struct mach_header_64) + 
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	const char strings[] = "\0_start\0";

	struct symtab_command table  = {0};
	table.cmd = LC_SYMTAB;
	table.cmdsize = sizeof(struct symtab_command);
	table.strsize = sizeof(strings);
	table.nsyms = 1;
	table.stroff = 0;
	
	table.symoff = (uint32_t) (
				sizeof(struct mach_header_64) +
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command) + 
				my_count
			);

	table.stroff = table.symoff + sizeof(struct nlist_64);

	struct nlist_64 symbols[] = {
	        (struct nlist_64) {
	            .n_un.n_strx = 1,
	            .n_type = N_SECT | N_EXT,
	            .n_sect = 1,
	            .n_desc = REFERENCE_FLAG_DEFINED,
	            .n_value = 0x0000000000,
	        }
	};

	if (not should_overwrite and not access(output_filename, F_OK)) {
		puts("macho object file: file exists"); 
		puts(output_filename);
		exit(1);
	}

	{ const int flags = O_WRONLY | O_CREAT | O_TRUNC | (not should_overwrite ? O_EXCL : 0);
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(output_filename, flags, mode);
	if (file < 0) { perror("obj:open"); exit(1); }

	write(file, &header, sizeof(struct mach_header_64));
	write(file, &segment, sizeof (struct segment_command_64));
	write(file, &section, sizeof(struct section_64));
	write(file, &table, sizeof table);
	write(file, my_bytes, my_count);
	write(file, symbols, sizeof(struct nlist_64));
	write(file, strings, sizeof strings);
	close(file); } 

	printf("info: success: generated object file: %s\n", output_filename);
	goto finished_outputting;
}

generate_ti_txt_executable:;

	{ char out[max_output_size] = {0}; // TODO: rewrite this using insert_byte().
	nat len = 0, this_section = 0, section_byte_count = 0;

	print_nats(section_starts, section_count);
	print_nats(section_addresses, section_count);

	for (nat i = 0; i < my_count; i++) {
		if (this_section < section_count and i == section_starts[this_section]) {
			if (this_section) len += (nat) snprintf(out + len, sizeof out, "\n");
			len += (nat) snprintf(out + len, sizeof out, "@%04llx", section_addresses[this_section]);
			this_section++; section_byte_count = 0;
		}
		if (section_byte_count % 16 == 0) len += (nat) snprintf(out + len, sizeof out, "\n");
		len += (nat) snprintf(out + len, sizeof out, "%02hhX ", my_bytes[i]);
		section_byte_count++;
	}

	len += (nat) snprintf(out + len, sizeof out, "\nq\n");

	printf("about to write out: \n-------------------\n<<<%.*s>>>\n----------------\n", (int) len, out);
	printf("writing out ti txt executable...\n");
	fflush(stdout);

	if (not access(output_filename, F_OK)) {
		printf("file exists. do you wish to remove the previous one? (y/n) ");
		fflush(stdout);
		if (should_overwrite or getchar() == 'y') {
			printf("file %s was removed.\n", output_filename);
			int r = remove(output_filename);
			if (r < 0) { perror("remove"); exit(1); }
		} else {
			puts("not removed, compilation aborted.");
		}
	}

	{ int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("open: could not create executable file"); exit(1); }
	write(file, out, len);
	close(file); }
	printf("ti-txt: wrote %llu bytes to file %s.\n", len, output_filename);
	snprintf(debug_string, sizeof debug_string, "../../led_display/embedded_assembler/msp430_disassembler/run %s", output_filename);
	system(debug_string); }
	goto finished_outputting;

generate_uf2_executable:;

	{ printf("section_starts: "); print_nats(section_starts, section_count); puts("");
	printf("section_addresses: "); print_nats(section_addresses, section_count); puts("");
	const nat starting_address = section_addresses[0]; 
	printf("info: starting UF2 file at address: %08llx\n", starting_address);
	while (my_count % 256) insert_byte(&my_bytes, &my_count, 0);
	const nat block_count = my_count / 256;
	uint8_t* data = NULL;
	nat count = 0;	
	nat current_offset = 0;

	for (nat block = 0; block < block_count; block++) {
		insert_u32(&data, &count, 0x0A324655);
		insert_u32(&data, &count, 0x9E5D5157);
		insert_u32(&data, &count, 0x00002000);
		insert_u32(&data, &count, (uint32_t) (starting_address + current_offset));
		insert_u32(&data, &count, 256); 
		insert_u32(&data, &count, (uint32_t) block); 
		insert_u32(&data, &count, (uint32_t) block_count); 
		insert_u32(&data, &count, 0xE48BFF5A); // rp2350-riscv family ID. 
		for (nat i = 0; i < 256; i++) 
			insert_byte(&data, &count, my_bytes[current_offset + i]);
		for (nat i = 0; i < 476 - 256; i++) insert_byte(&data, &count, 0);
		insert_u32(&data, &count, 0x0AB16F30);
		current_offset += 256;
	}

	if (not access(output_filename, F_OK)) {
		printf("file exists. do you wish to remove the previous one? (y/n) ");
		fflush(stdout);
		if (should_overwrite or getchar() == 'y') {
			printf("file %s was removed.\n", output_filename);
			int r = remove(output_filename);
			if (r < 0) { perror("remove"); exit(1); }
		} else {
			puts("not removed, compilation aborted.");
		}
	}

	{ int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (file < 0) { perror("open: could not create executable file"); exit(1); }
	write(file, data, count);
	close(file); }

	dump_hex(data, count);
	printf("uf2: wrote %llu bytes to file %s.\n", count, output_filename);	
	snprintf(debug_string, sizeof debug_string, "./rv_dis/run print %s", output_filename);
	system(debug_string); }
finished_outputting: 
	exit(0);
} // main 


















































































































































































































































































































































































































































































































































































































































































































































































































/*if (op == st and val0 == compiler_length) {
			for (nat i = 0; i < string_list_count; i++) {
				if (string_label[i] == val1) { 
					memory[compiler_length] = strlen(string_list[i]); 
					goto found_string;
				}
			}
			memory[compiler_length] = (nat) -1; found_string:;
		}*/

/*

TRASH:
---------------


//static const nat uninitialized_variable = 	(nat) -1;
//static const nat is_label = 			0x8000000000000000;
//static const nat is_ct_label = 			0x4000000000000000;

*/
/*static nat compute_label_location(nat label) {
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at and ins[i].args[0] == label) return i;
	}
	printf("error: could not compute label location for label: %s(0x%016llx)\n", 
		label < var_count ? variables[label] : "(OUTSIDE VAR RANGE)", 
		label
	);
	abort();
}*/

/*static nat calculate_offset(nat* length, nat here, nat label) {

	if (not (label & is_label)) { puts("error: calculate_offset(): was passed a nonlabel: "); printf("error: could not compute label location for label: %s(0x%016llx)\n", 
		label < var_count ? variables[label] : "(OUTSIDE VAR RANGE)", 
		label
	); abort(); } 
	
	const nat target = compute_label_location(label & ~is_label);

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
}*/



			//op == ru and arg_count == 2
			//op == rs and arg_count == 4 or
			//op == ri and arg_count == 4

	//TODO: figure out a way to delete this variable lol.
	//bool should_check[192390] = {0}; // TODO: make this sized max_variable_count, global. 


	/*for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op, e = ins[i].args[0];
		if (op == at) values[e] = i;
		//(values[e] & is_ct_label) ? i : e | is_label;
	}*/
			

	/*nat label_count[4096] = {0};
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) label_count[ins[i].args[0]]++;
	}

	for (nat i = 0; i < ins_count; i++) {
		for (nat a = 0; a < arity[ins[i].op]; a++) {
			const nat this = ins[i].args[a];
			if (not ((ins[i].imm >> a) & 1) and label_count[this] != 1) {
				printf("error: instruction %s, argument #%llu, expected exactly one label attribution, "
					"but found label count = %llu for variable %s\n", 
					operations[ins[i].op], a, 
					label_count[this], variables[this]
				);
				print_instruction_window_around(i, 1, "incorrect label attribution for this argument");
				abort();
			}
		}
	}*/


		/*if (op == at) for (nat i = 0; i < fixup_count; i++) {
			if (fixup_var[i] == arg0) {
				rt_ins[fixup_ins[i]].args[fixup_arg[i]] = total_byte_count;
				fixup_var[i] = (nat) -1;
			}
		}*/

		//if (op == del) {			
			//is_undefined[arg0] = var_count;
			//is_undefined[var_count] = (nat) -1;
			//variables[var_count++] = strdup("_generated_");
			//abort();






/*if (values[this] == (nat) -1) { 
						printf("WARNING: FOUND A FORWARD BRANCH RUNTIME INSTRUCTION. "
							"BRANCHING FORWARDS TO \"%s\"\n", 
							variables[this]
						);
						printf("uhh...not sure what to do here...?");
						abort();
						
						//fixup_ins[fixup_count] = rt_ins_count;
						//fixup_arg[fixup_count] = a;
						//fixup_var[fixup_count++] = this;

						//printf("info: just pushed a new fixup: (fixupcount %llu) {.arg = %llu, .ins = %llu}\n", 
							//fixup_count, a, rt_ins_count
						//);
						//getchar();
					}*/



		//} else if () {
			//struct instruction new = ins[pc];
			//if (is_undefined[arg0] != (nat) -1) new.args[0] = is_undefined[arg0];
			//values[rt_label]



	/*nat* lengths = calloc(ins_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;

		if (op == at) lengths[i] = 0;
		else if (op == sect) lengths[i] = 0;
		else if (op == emit) lengths[i] = ins[i].args[0];
		else lengths[i] = 4;
	}*/

	//print_nats(lengths, ins_count); puts("");





			//if (not k and (a4 & is_label)) {} else 


//nat k = a4;

			/*if (k & is_label) {
				const nat im = calculate_offset(lengths, i - 1, k) & 0x00000FFF;
				k = im;
			}*/



/*

nat* lengths = calloc(ins_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at or ins[i].op == sect) continue;
		lengths[i] = ins[i].op == emit ? ins[i].args[0] : 4;
	}

	print_nats(lengths, ins_count); puts("");
*/





















































				//if (not (values[this] & is_label)) 
				//else if (is_undefined[this] != (nat) -1) new.args[a] = is_undefined[this];
				//else new.args[a] = values[this];

	/*for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == lt or op == eq) values[ins[i].args[2]] &= execute_mask;
	}	
	for (nat i = 0; i < var_count; i++) if (values[i] & is_label) values[i] = i | is_label;
	*/
	



/*

if (	op == zero or op == incr or 
				op == set or op == st or 
				op == sect) is_constant[args[0]] = 1;
			else if (op == emit or (op >= add and <= ld) {
				is_constant[args[0]] = 1;
				is_constant[args[1]] = 1;
			} else if (op == lt or op == eq) {
				is_constant[args[0]] = 1;
				is_constant[args[1]] = 1;
				is_constant[args[2]] = 1;
			}





*/



































































/*

generate_c_source_output:;

	{ char* out = malloc(my_count);
	memcpy(out, my_bytes, my_count);
	nat len = my_count;

	printf("about to write out: \n-------------------\n<<<%.*s>>>\n----------------\n", (int) len, out);
	printf("writing c source code file...\n");
	fflush(stdout);

	if (not access(output_filename, F_OK)) {
		printf("file exists. do you wish to remove the previous one? (y/n) ");
		fflush(stdout);
		if (should_overwrite or getchar() == 'y') {
			printf("file %s was removed.\n", output_filename);
			int r = remove(output_filename);
			if (r < 0) { perror("remove"); exit(1); }
		} else {
			puts("not removed, compilation aborted.");
		}
	}

	{ int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (file < 0) { perror("open: could not create executable file"); exit(1); }
	write(file, out, len);
	close(file); }

	printf("c source output: wrote %llu bytes to file %s.\n", len, output_filename);

	{ char debug_string[4096] = {0};
	snprintf(debug_string, sizeof debug_string, 
		"clang -Weverything -Wno-unused-label -Wno-poison-system-directories -O3 %s",
		output_filename
	);
	system(debug_string); } } 

	goto finished_outputting;
	













c_generate_source_code:;

	{ 

	const char* header = 
	"// c source file auto-generated by my compiler.\n"
	"#include <stdlib.h>\n"
	"#include <unistd.h>\n"
	"#include <fcntl.h>\n"
	"#include <sys/mman.h>\n"
	"#include <stdio.h>\n"
	"#include <errno.h>\n"
	"#include <stdint.h>\n"
	"\n"
	"static uint64_t x[4096];\n"
	"\n"
	"static void ecall(void) {\n"
	"\tif (x[0] == 0) printf(\"debug: hello: %llu (0x%llx)\\n\", x[1], x[1]);\n"
	"\telse if (x[0] == 1) exit((int) x[1]);\n"
	"\telse if (x[0] == 2) { x[1] = (uint64_t) read((int) x[1], (void*) x[2], (size_t) x[3]); x[2] = (uint64_t) errno; }\n"
	"\telse if (x[0] == 3) { x[1] = (uint64_t) write((int) x[1], (void*) x[2], (size_t) x[3]); x[2] = (uint64_t) errno; }\n"
	"\telse if (x[0] == 4) { x[1] = (uint64_t) open((const char*) x[1], (int) x[2], (mode_t) x[3]); x[2] = (uint64_t) errno; }\n"
	"\telse if (x[0] == 5) { x[1] = (uint64_t) close((int) x[1]); x[2] = (uint64_t) errno; }\n"
	"\telse if (x[0] == 6) { x[1] = (uint64_t) (void*) mmap((void*) x[1], (size_t) x[2], (int) x[3],"
				" (int) x[4], (int) x[5], (off_t) x[6]); x[2] = (uint64_t) errno; }\n"
	"\telse if (x[0] == 7) { x[1] = (uint64_t) munmap((void*) x[1], (size_t) x[2]); x[2] = (uint64_t) errno; }\n"
	"\telse abort();\n"
	"}\n\n";

	const char* footer = 
		"}\n// (end of file)\n\n"
	;

	{ char str[4096] = {0};
	const nat len = (nat) snprintf(str, sizeof str, "%s", header);
	insert_bytes(&my_bytes, &my_count, str, len); } 

	nat* label_data_locations = calloc(var_count, sizeof(nat));

	uint8_t data_bytes[64000] = {0};
	nat data_byte_count = 0;

	for (nat i = 0; i < ins_count; i++) {

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("");
			dump_hex(data_bytes, data_byte_count);
			getchar();
		}

		const nat op = ins[i].op;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1];

		if (op == at) {
			label_data_locations[a0] = data_byte_count;

		} else if (op == emit) {
			if (a0 == 8) {
				data_bytes[data_byte_count++] = (a1 >> 0) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 8) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 16) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 24) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 32) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 40) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 48) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 56) & 0xff;
			}

			if (a0 == 4) {
				data_bytes[data_byte_count++] = (a1 >> 0) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 8) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 16) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 24) & 0xff;

			}

			if (a0 == 2) {
				data_bytes[data_byte_count++] = (a1 >> 0) & 0xff;
				data_bytes[data_byte_count++] = (a1 >> 8) & 0xff;
			}

			if (a0 == 1) data_bytes[data_byte_count++] = (a1 >> 0) & 0xff;;
		}
	}

	{ char str[4096] = {0};
	const nat len = (nat) snprintf(str, sizeof str, "static uint8_t d[%llu] = {\n\t", data_byte_count);
	insert_bytes(&my_bytes, &my_count, str, len); } 

	for (nat i = 0; i < data_byte_count; i++) {
		char str[4096] = {0};
		const nat len = (nat) snprintf(str, sizeof str, "0x%02x,%s", data_bytes[i], i % 8 == 7 ? "\n\t" : " ");
		insert_bytes(&my_bytes, &my_count, str, len);
	}
	
	{ char str[4096] = {0};
	const nat len = (nat) snprintf(str, sizeof str, "\n};\n\nint main(void) {\n");
	insert_bytes(&my_bytes, &my_count, str, len); } 

	for (nat i = 0; i < ins_count; i++) {

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("");
			printf("C source code: \n<<<%.*s>>>\n", (int) my_count, (char*) my_bytes);
			getchar();
		}

		const nat op = ins[i].op;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1];
		//const nat a2 = ins[i].args[2];

		if (op == sect) {}

		else if (op == ri and a0 == 0x73) {
			char str[4096] = {0};
			const nat len = (nat) snprintf(str, sizeof str, "\tecall();\n");
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == emit) {}

		else if (op == at) {
			char str[4096] = {0};
			const nat len = (nat) snprintf(str, sizeof str, "_%llu:;\n", a0);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == do_) {
			char str[4096] = {0};
			const nat len = (nat) snprintf(str, sizeof str, "\tgoto _%llu;\n", a0);
			insert_bytes(&my_bytes, &my_count, str, len);


		} else if (op == set) { //not imm and is_label[a1]
			char str[4096] = {0}; nat len = 0;
			len = (nat) snprintf(str, sizeof str, "\tx[%llu] = ((uint64_t)(void*)d) + 0x%llx;\n", a0, label_data_locations[a1]);
			insert_bytes(&my_bytes, &my_count, str, len);

		}

	 

		else if (op == set) {

			//if (a0 == a1 and not imm) continue;

			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] = 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] = x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == add) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] += 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] += x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == sub) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] -= 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] -= x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == mul) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] *= 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] *= x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == div_) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] /= 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] /= x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == rem) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] %%= 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] %%= x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == and_) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] &= 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] &= x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == or_) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] |= 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] |= x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == eor) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] ^= 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] ^= x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == si) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] <<= 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] <<= x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == sd) {
			char str[4096] = {0}; nat len = 0;
			if (imm) len = (nat) snprintf(str, sizeof str, "\tx[%llu] >>= 0x%llx;\n", a0, a1);
			else len = (nat) snprintf(str, sizeof str, "\tx[%llu] >>= x[%llu];\n", a0, a1);
			insert_bytes(&my_bytes, &my_count, str, len);


		} else if (op == st) {
			
			if (a2 == 8) {
				char str[4096] = {0}; nat len = 0;
				if (imm & 1) { puts("error: cannot store to an immediate address in c\n"); abort(); }
				else if (imm) len = (nat) snprintf(str, sizeof str, "\t*(uint64_t*)(x[%llu]) = 0x%llx;\n", a0, a1);
				else len = (nat) snprintf(str, sizeof str, "\t*(uint64_t*)(x[%llu]) = x[%llu];\n", a0, a1);
				insert_bytes(&my_bytes, &my_count, str, len);

			} else if (a2 == 1) {
				char str[4096] = {0}; nat len = 0;
				if (imm & 1) { puts("error: cannot store to an immediate address in c\n"); abort(); }
				else if (imm) len = (nat) snprintf(str, sizeof str, "\t*(uint8_t*)(x[%llu]) = (uint8_t) 0x%llx;\n", a0, a1);
				else len = (nat) snprintf(str, sizeof str, "\t*(uint8_t*)(x[%llu]) = (uint8_t) x[%llu];\n", a0, a1);
				insert_bytes(&my_bytes, &my_count, str, len);

			} else if (a2 == 2) {
				char str[4096] = {0}; nat len = 0;
				if (imm & 1) { puts("error: cannot store to an immediate address in c\n"); abort(); }
				else if (imm) len = (nat) snprintf(str, sizeof str, "\t*(uint16_t*)(x[%llu]) = (uint16_t) 0x%llx;\n", a0, a1);
				else len = (nat) snprintf(str, sizeof str, "\t*(uint16_t*)(x[%llu]) = (uint16_t) x[%llu];\n", a0, a1);
				insert_bytes(&my_bytes, &my_count, str, len);

			} else if (a2 == 4) {
				char str[4096] = {0}; nat len = 0;
				if (imm & 1) { puts("error: cannot store to an immediate address in c\n"); abort(); }
				else if (imm) len = (nat) snprintf(str, sizeof str, "\t*(uint32_t*)(x[%llu]) = (uint32_t) 0x%llx;\n", a0, a1);
				else len = (nat) snprintf(str, sizeof str, "\t*(uint32_t*)(x[%llu]) = (uint32_t) x[%llu];\n", a0, a1);
				insert_bytes(&my_bytes, &my_count, str, len);

			} else { puts("unimplemented"); abort(); } 


		} else if (op == ld) {

			if (a2 == 8) {
				char str[4096] = {0}; nat len = 0;
				if (imm & 3) { puts("error: cannot load from an immediate address in c\n"); abort(); }
				else len = (nat) snprintf(str, sizeof str, "\tx[%llu] = *(uint64_t*)(x[%llu]);\n", a0, a1);
				insert_bytes(&my_bytes, &my_count, str, len);

			} else if (a2 == 1) {
				char str[4096] = {0}; nat len = 0;
				if (imm & 3) { puts("error: cannot load from an immediate address in c\n"); abort(); }
				else len = (nat) snprintf(str, sizeof str, "\tx[%llu] = (uint64_t) (*(uint8_t*)(x[%llu]));\n", a0, a1);
				insert_bytes(&my_bytes, &my_count, str, len);

			} else if (a2 == 2) {
				char str[4096] = {0}; nat len = 0;
				if (imm & 3) { puts("error: cannot load from an immediate address in c\n"); abort(); }
				else len = (nat) snprintf(str, sizeof str, "\tx[%llu] = (uint64_t) (*(uint16_t*)(x[%llu]));\n", a0, a1);
				insert_bytes(&my_bytes, &my_count, str, len);

			} else if (a2 == 4) {
				char str[4096] = {0}; nat len = 0;
				if (imm & 3) { puts("error: cannot load from an immediate address in c\n"); abort(); }
				else len = (nat) snprintf(str, sizeof str, "\tx[%llu] = (uint64_t) (*(uint32_t*)(x[%llu]));\n", a0, a1);
				insert_bytes(&my_bytes, &my_count, str, len);

			} else { puts("unimplemented"); abort(); } 


		} else if (op == lt) {
			char str[4096] = {0}; nat len = 0;
			if (imm & 1) len = (nat) snprintf(str, sizeof str, "\tif (0x%llx < x[%llu]) goto _%llu;\n", a0, a1, a2);
			else if (imm & 2) len = (nat) snprintf(str, sizeof str, "\tif (x[%llu] < 0x%llx) goto _%llu;\n", a0, a1, a2);
			else len = (nat) snprintf(str, sizeof str, "\tif (x[%llu] < x[%llu]) goto _%llu;\n", a0, a1, a2);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == ge) {
			char str[4096] = {0}; nat len = 0;
			if (imm & 1) len = (nat) snprintf(str, sizeof str, "\tif (0x%llx >= x[%llu]) goto _%llu;\n", a0, a1, a2);
			else if (imm & 2) len = (nat) snprintf(str, sizeof str, "\tif (x[%llu] >= 0x%llx) goto _%llu;\n", a0, a1, a2);
			else len = (nat) snprintf(str, sizeof str, "\tif (x[%llu] >= x[%llu]) goto _%llu;\n", a0, a1, a2);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == ne) {
			char str[4096] = {0}; nat len = 0;
			if (imm & 1) len = (nat) snprintf(str, sizeof str, "\tif (0x%llx != x[%llu]) goto _%llu;\n", a0, a1, a2);
			else if (imm & 2) len = (nat) snprintf(str, sizeof str, "\tif (x[%llu] != 0x%llx) goto _%llu;\n", a0, a1, a2);
			else len = (nat) snprintf(str, sizeof str, "\tif (x[%llu] != x[%llu]) goto _%llu;\n", a0, a1, a2);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else if (op == eq) {
			char str[4096] = {0}; nat len = 0;
			if (imm & 1) len = (nat) snprintf(str, sizeof str, "\tif (0x%llx == x[%llu]) goto _%llu;\n", a0, a1, a2);
			else if (imm & 2) len = (nat) snprintf(str, sizeof str, "\tif (x[%llu] == 0x%llx) goto _%llu;\n", a0, a1, a2);
			else len = (nat) snprintf(str, sizeof str, "\tif (x[%llu] == x[%llu]) goto _%llu;\n", a0, a1, a2);
			insert_bytes(&my_bytes, &my_count, str, len);

		} else {
			printf("error: unknown C machine instruction op=\"%s\"\n", operations[op]);
			abort();
		}

		abort();
	}

	char str[4096] = {0};
	const nat len = (nat) snprintf(str, sizeof str, "%s", footer);
	insert_bytes(&my_bytes, &my_count, str, len); 

	}

	goto finished_generation;








*/



























	/*for (nat i = 0; i < var_count; i++) {
		if (max_name_width < (int) strlen(variables[i])) {
			max_name_width = (int) strlen(variables[i]);
		}
	}*/










			//printf("decimal: im = %d\n", im | ((im & (1 << 21)) ? 0xFFE00000 : 0));
			//printf("binary: "); print_binary(im | ((im & (1 << 21)) ? 0xFFE00000 : 0)); puts("");
			//printf("j_type:  offset = 0x%08x\n", offset);
			//printf("offset = "); print_binary(offset); puts("");
			//getchar();
	

			//printf("decimal: im = %d\n", im | ((im & (1 << 12)) ? 0xFFFFE000 : 0));
			//printf("decimal: "); print_binary(im | ((im & (1 << 12)) ? 0xFFFFE000 : 0)); puts("");
			//printf("b_type:  im = 0x%08x, lo = 0x%08x, hi = 0x%08x\n", im, lo, hi);
			//printf("im = "); print_binary(im); puts("");
			//printf("lo = "); print_binary(lo); puts("");
			//printf("hi = "); print_binary(hi); puts("");
			//getchar();







/* 1202507174.220252

		we are currently i the middle of sorting out how execution passing through a del causes the regeneration of a label, 

				and we are trying to figure out how to make it so that we don't need to add another

										at label 


							instruction 


					i think its possible lol. 


*/






/*



			



else if (op == sc) {

			nat x0 = *(((nat*) bridge) + compiler_arg0);
			nat x1 = *(((nat*) bridge) + compiler_arg1);
			nat x2 = *(((nat*) bridge) + compiler_arg2);
			nat x3 = *(((nat*) bridge) + compiler_arg3);
			nat x4 = *(((nat*) bridge) + compiler_arg4);
			nat x5 = *(((nat*) bridge) + compiler_arg5);
			nat x6 = *(((nat*) bridge) + compiler_arg6);

			if (x0 == compiler_system_debug) {
				printf("debug: %llu (0x%llx)\n", x1, x1);

			} else if (x0 == compiler_system_exit) {
				exit((int) x1);

			} else if (x0 == compiler_system_read) {
				x1 = (nat) read((int) x1, (void*) x2, (size_t) x3);

			} else if (x0 == compiler_system_write) { 
				x1 = (nat) write((int) x1, (void*) x2, (size_t) x3); 

			} else if (x0 == compiler_system_open) { 
				x1 = (nat) open((const char*) x1, (int) x2, (mode_t) x3); 

			} else if (x0 == compiler_system_close) { 
				x1 = (nat) close((int) x1); 

			} else if (x0 == compiler_system_mmap) {
				x1 = (uint64_t) (void*) mmap(
					(void*) x1, 
					(size_t) x2, 
					(int) x3, (int) x4, 
					(int) x5, (off_t) x6
				);

			} else if (x0 == compiler_system_munmap) {
				x1 = (uint64_t) munmap(
					(void*) x1, 
					(size_t) x2
				);

			} else { 
				printf("compiler: error: unknown system call number %llu\n", x0); 
				abort(); 
			} 

			*(((nat*) bridge) + compiler_arg1) = x1;
			*(((nat*) bridge) + compiler_arg2) = (nat) errno;
		} 




if (not_literal and is_label[this] and is_undefined[this]) {
					new.args[i] = is_undefined[this];
				}

*/

			/*printf("strings: (%llu count) : \n", string_list_count);
			for (nat i = 0; i < string_list_count; i++) {
				printf("#%llu string: .string = %p .length = %llu, .label = %llu, \n", 
					i, (void*) string_list[i], (nat) strlen(string_list[i]), string_label[i]
				);
			}*/








