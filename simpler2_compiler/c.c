// 1202504045.155147 a compiler / assembler for a simpler version of the language.
// 1202505106.125751 revised to be much simpler and have a simpler translation process.
// 1202505294.204607 revised the ct system! 

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

#define max_variable_count (1 << 14)
#define max_instruction_count (1 << 14)
#define max_arg_count 12

enum all_architectures { no_arch, arm64_arch, arm32_arch, rv64_arch, rv32_arch, msp430_arch, };
enum all_output_formats { 
	debug_output_only, 
	macho_executable, macho_object, 
	elf_executable, elf_object, 
	ti_txt_executable, 
	uf2_executable, 
	hex_array_output, 
};
enum compiler_system_calls { 
	compiler_abort, compiler_exit, 
	compiler_getchar, compiler_putchar, 
	compiler_printbin, compiler_printdec, 
	compiler_setdebug, compiler_print, 
	compiler_target, compiler_format, 
	compiler_overwrite, compiler_getlength, 
	compiler_gettarget, compiler_getformat, 
	compiler_stacksize, compiler_getstacksize,
};

enum core_language_isa {
	nullins,

	ct, rt, system_, emit, string,
	file, del, register_, bits,
	set, add, sub, mul, div_, rem, 
	and_, or_, eor, si, sd, la, 
	ld, st, lt, ge, ne, eq, do_, at, halt, 
	
	a6_nop, a6_svc, a6_mov, a6_bfm,
	a6_adc, a6_addx, a6_addi, a6_addr, a6_adr, 
	a6_shv, a6_clz, a6_rev, a6_jmp, a6_bc, a6_br, 
	a6_cbz, a6_tbz, a6_ccmp, a6_csel, 
	a6_ori, a6_orr, a6_extr, a6_ldrl, 
	a6_memp, a6_memia, a6_memi, a6_memr, 
	a6_madd, a6_divr, 

	m4_sect, m4_op, m4_br,

	r5_r, r5_i, r5_s, r5_b, r5_u, r5_j, 
	isa_count
};

static const char* operations[isa_count] = {
	"___nullins____",

	"ct", "rt", "system", "emit", "string", 
	"file", "del", "register", "bits",
	"set", "add", "sub", "mul", "div", "rem", 
	"and", "or", "eor", "si", "sd", "la", 
	"ld", "st", "lt", "ge", "ne", "eq", "do", "at", "halt", 

	"a6_nop", "a6_svc", "a6_mov", "a6_bfm",
	"a6_adc", "a6_addx", "a6_addi", "a6_addr", "a6_adr", 
	"a6_shv", "a6_clz", "a6_rev", "a6_jmp", "a6_bc", "a6_br", 
	"a6_cbz", "a6_tbz", "a6_ccmp", "a6_csel", 
	"a6_ori", "a6_orr", "a6_extr", "a6_ldrl", 
	"a6_memp", "a6_memia", "a6_memi", "a6_memr", 
	"a6_madd", "a6_divr", 

	"m4_sect", "m4_op", "m4_br",

	"r5_r", "r5_i", "r5_s", "r5_b", "r5_u", "r5_j", 
};

static const nat arity[isa_count] = {
	0,

	0, 0, 0, 2, 2, 
	1, 1, 2, 2,
	2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 
	3, 3, 3, 3, 3, 3, 1, 1, 0,
			
	0, 0, 5, 7, 
	6, 8, 7, 8, 3,
	5, 4, 4, 2, 2, 3, 
	4, 4, 7, 7, 
	5, 8, 5, 3, 
	7, 6, 5, 6,
	8, 5, 

	1, 8, 2,

	6, 5, 5, 5, 3, 3,
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
static nat bit_count[max_variable_count] = {0};
static nat register_index[max_variable_count] = {0};
static nat values[max_variable_count] = {0};
static byte is_constant[max_variable_count] = {0};
static byte is_label[max_variable_count] = {0};
static byte is_undefined[max_variable_count] = {0};
static nat var_count = 0;

static void print_nats(nat* array, nat count) {
	printf("(%llu) { ", count);
	for (nat i = 0; i < count; i++) {
		printf("%lld ", array[i]);
	}
	printf("}");
}

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

static nat read_single_char_from_stdin(void) {
	struct termios terminal = {0};
	tcgetattr(0, &terminal);
	struct termios copy = terminal; 
	copy.c_cc[VMIN] = 1; 
	copy.c_cc[VTIME] = 0;
	copy.c_lflag &= ~((size_t) ECHO | ICANON);
	tcsetattr(0, TCSANOW, &copy);

	char c = 0;
	ssize_t n = read(0, &c, 1);
	if (n <= 0) { 
		puts("compiler: fatal error: input/output error: "); 
		perror("read"); 
		abort(); 
	}

	tcsetattr(0, TCSANOW, &terminal);	
	return (nat) c;
}

static void print_dictionary(const nat should_print_ct) {
	puts("variable dictionary: ");
	for (nat i = 0; i < var_count; i++) {
		if (should_print_ct or (not is_constant[i] and not is_label[i])) 
		printf("   %c %c %c %c [%5llu]  \"%20s\"  :   { bc=%5lld, ri=%5lld }  :   0x%016llx\n",
			' ', 
			is_label[i] ? 'L' : ' ', 
			is_constant[i] ? 'C' : ' ', 
			is_undefined[i] ? 'U' : ' ', 
			i, variables[i], 
			bit_count[i], register_index[i], 
			values[i]
		);
	}
	puts("[end]");
}

static void print_instruction(struct instruction this) {

	//if (this.ct) printf("\033[33m");

	int max_name_width = 0;
	for (nat i = 0; i < var_count; i++) {
		if (max_name_width < (int) strlen(variables[i])) {
			max_name_width = (int) strlen(variables[i]);
		}
	}

	printf("  %4s    ", operations[this.op]);
	/*int left_to_print = max_name_width - (int) strlen(operations[this.op]);
	if (left_to_print < 0) left_to_print = 0;
	for (int i = 0; i < left_to_print; i++) putchar(' ');
	putchar(' '); }*/

	for (nat a = 0; a < arity[this.op]; a++) {

		char string[4096] = {0};
		if (this.imm & (1 << a)) snprintf(string, sizeof string, "0x%llx", this.args[a]);
		else snprintf(string, sizeof string, "%s", variables[this.args[a]]);

		printf("%s", string);
		int left_to_print = max_name_width - (int) strlen(string);
		if (left_to_print < 0) left_to_print = 0;
		for (int i = 0; i < left_to_print; i++) putchar(' ');
		putchar(' ');
	}

	//if (this.ct) printf("\033[0m");

}

static void print_instructions(const bool should_number_them) {

	printf("instructions: (%llu count) {\n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) puts("");
		if (should_number_them) printf("%4llu: ", i);
		if (ins[i].op != at) printf("      ");
		print_instruction(ins[i]);
		puts("");
	}
	puts("}");
}

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

		printf("  %s%4llu │ ", 
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


static nat* compute_successors(nat pc) {

	nat* gotos = calloc(2 * ins_count, sizeof(nat)); 
	nat locations[4096] = {0}; // var_count sized

	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) 
			locations[ins[i].args[0]] = i;
	}

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == lt or op == ge or op == ne or op == eq) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = locations[ins[i].args[2]];

		} else if (op == do_) {
			gotos[2 * i + 0] = locations[ins[i].args[0]];
			gotos[2 * i + 1] = (nat) -1;

		} else if (op == halt) {
			gotos[2 * i + 0] = (nat) -1;
			gotos[2 * i + 1] = (nat) -1;

		} else {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = (nat) -1;
		}
	}

	nat* result = calloc(2, sizeof(nat));
	if (pc < ins_count) result[0] = gotos[2 * pc + 0];
	if (pc < ins_count) result[1] = gotos[2 * pc + 1];
	free(gotos);
	return result;
}

static nat* compute_predecessors(nat pc, nat* pred_count) {

	nat* gotos = calloc(2 * ins_count, sizeof(nat)); 
	nat locations[4096] = {0}; // var_count sized

	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) 
			locations[ins[i].args[0]] = i;
	}

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == lt or op == ge or op == ne or op == eq) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = locations[ins[i].args[2]];

		} else if (op == do_) {
			gotos[2 * i + 0] = locations[ins[i].args[0]];
			gotos[2 * i + 1] = (nat) -1;

		} else if (op == halt) {
			gotos[2 * i + 0] = (nat) -1;
			gotos[2 * i + 1] = (nat) -1;

		} else {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = (nat) -1;
		}
	}

	nat* result = NULL;
	nat count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (gotos[2 * i + 0] == pc or gotos[2 * i + 1] == pc) {
			result = realloc(result, sizeof(nat) * (count + 1));
			result[count++] = i;
		}
	}

	free(gotos);
	*pred_count = count;
	return result;
}

static nat compute_label_location(nat label) {
	nat locations[4096] = {0}; // var_count sized
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) locations[ins[i].args[0]] = i;
	}
	return locations[label];
}

static nat* compute_riscv_successors(nat pc) {
	nat* gotos = calloc(2 * ins_count, sizeof(nat)); 
	nat locations[4096] = {0}; // var_count sized
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) locations[ins[i].args[0]] = i;
	}
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == halt) {
			gotos[2 * i + 0] = (nat) -1;
			gotos[2 * i + 1] = (nat) -1;
		} else if (op == r5_b) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = locations[ins[i].args[2]];
		} else if (op == r5_j) {
			gotos[2 * i + 0] = locations[ins[i].args[0]];
			gotos[2 * i + 1] = (nat) -1;
		} else if (op == r5_i or op == r5_u or op == r5_s or op == r5_r or op == at or op == emit) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = (nat) -1;
		} else {
			puts("error: compute_riscv_successors(): found a non RV MI...\n");
			print_instruction(ins[i]);
			abort();
		} 
	}

	nat* result = calloc(2, sizeof(nat));
	if (pc < ins_count) result[0] = gotos[2 * pc + 0];
	if (pc < ins_count) result[1] = gotos[2 * pc + 1];
	free(gotos);
	return result;
}

static nat* compute_riscv_predecessors(nat pc, nat* pred_count) {
	nat* gotos = calloc(2 * ins_count, sizeof(nat)); 
	nat locations[4096] = {0}; // var_count sized
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) locations[ins[i].args[0]] = i;
	}
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == halt) {
			gotos[2 * i + 0] = (nat) -1;
			gotos[2 * i + 1] = (nat) -1;
		} else if (op == r5_b) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = locations[ins[i].args[2]];
		} else if (op == r5_j) {
			gotos[2 * i + 0] = locations[ins[i].args[0]];
			gotos[2 * i + 1] = (nat) -1;
		} else if (op == r5_i or op == r5_u or op == r5_s or op == r5_r or op == at or op == emit) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = (nat) -1;
		} else {
			puts("error: compute_riscv_successors(): found a non RV MI...\n");
			print_instruction(ins[i]);
			abort();
		} 
	}

	nat* result = NULL;
	nat count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (gotos[2 * i + 0] == pc or gotos[2 * i + 1] == pc) {
			result = realloc(result, sizeof(nat) * (count + 1));
			result[count++] = i;
		}
	}

	free(gotos);
	*pred_count = count;
	return result;
}


static nat* compute_msp430_successors(nat pc) {
	nat* gotos = calloc(2 * ins_count, sizeof(nat)); 
	nat locations[4096] = {0}; // var_count sized
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) locations[ins[i].args[0]] = i;
	}
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == halt) {
			gotos[2 * i + 0] = (nat) -1;
			gotos[2 * i + 1] = (nat) -1;

		} else if (op == m4_br and ins[i].args[0] != 7) { // cond br
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = locations[ins[i].args[2]];

		} else if (op == m4_br and ins[i].args[0] == 7) { // uncond jump
			gotos[2 * i + 0] = locations[ins[i].args[0]];
			gotos[2 * i + 1] = (nat) -1;

		} else if (op == m4_sect or op == m4_op or op == at or op == emit) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = (nat) -1;

		} else {
			puts("error: compute_riscv_successors(): found a non RV MI...\n");
			print_instruction(ins[i]);
			abort();
		} 
	}

	nat* result = calloc(2, sizeof(nat));
	if (pc < ins_count) result[0] = gotos[2 * pc + 0];
	if (pc < ins_count) result[1] = gotos[2 * pc + 1];
	free(gotos);
	return result;
}

static nat* compute_msp430_predecessors(nat pc, nat* pred_count) {
	nat* gotos = calloc(2 * ins_count, sizeof(nat)); 
	nat locations[4096] = {0}; // var_count sized
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) locations[ins[i].args[0]] = i;
	}
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == halt) {
			gotos[2 * i + 0] = (nat) -1;
			gotos[2 * i + 1] = (nat) -1;

		} else if (op == m4_br and ins[i].args[0] != 7) { // cond br
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = locations[ins[i].args[2]];

		} else if (op == m4_br and ins[i].args[0] == 7) { // uncond jump
			gotos[2 * i + 0] = locations[ins[i].args[0]];
			gotos[2 * i + 1] = (nat) -1;

		} else if (op == m4_sect or op == m4_op or op == at or op == emit) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = (nat) -1;

		} else {
			puts("error: compute_riscv_successors(): found a non RV MI...\n");
			print_instruction(ins[i]);
			abort();
		} 
	}
	nat* result = NULL;
	nat count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (gotos[2 * i + 0] == pc or gotos[2 * i + 1] == pc) {
			result = realloc(result, sizeof(nat) * (count + 1));
			result[count++] = i;
		}
	}

	free(gotos);
	*pred_count = count;
	return result;
}


struct expected_instruction {
	nat op;
	nat imm;
	nat use;
	nat args[max_arg_count];	
};

static nat locate_instruction(struct expected_instruction expected, nat starting_from) {

	nat pc = starting_from;

	while (pc < ins_count) {

		nat pred_count = 0;
		compute_predecessors(pc, &pred_count);
		nat* gotos = compute_successors(pc);

		if (pred_count >= 2) break;

		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];
		const nat arg2 = ins[pc].args[2];

		const bool is_branch = op == lt or op == ge or op == ne or op == eq;

		const bool use_arg0 = !!(expected.use & 1);
		const bool use_arg1 = !!(expected.use & 2);
		const bool use_arg2 = !!(expected.use & 4);

		const bool arg0_matches = expected.args[0] == arg0;
		const bool arg1_matches = expected.args[1] == arg1;
		const bool arg2_matches = expected.args[2] == arg2;

		const bool valid_arg0 = not use_arg0 or arg0_matches;
		const bool valid_arg1 = not use_arg1 or arg1_matches;
		const bool valid_arg2 = not use_arg2 or arg2_matches;
		
		if (	op == expected.op and 
			imm == expected.imm and 
			valid_arg0 and 
			valid_arg1 and 
			valid_arg2) return pc; 

		if (is_branch) break;
		if (use_arg0 and arg1 == expected.args[0]) break;
		if (use_arg1 and arg0 == expected.args[1]) break;

		pc = gotos[0];
	}
	return (nat) -1;
}

static void dump_hex(uint8_t* memory, nat count) {
	printf("dumping bytes: (%llu)\n", count);
	for (nat i = 0; i < count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx(%c) ", memory[i], memory[i] >= 32 ? memory[i] : ' ');
	}
	puts("");

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

static void print_binary(nat x) {
	if (not x) { printf("0"); return; }

	for (nat i = 0; i < 64 and x; i++) {
		if (not (i % 8) and i) putchar('_');
		putchar((x & 1) + '0'); x >>= 1;
	}
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


static void debug_data_flow_state(
	nat pc,
	nat* preds, nat pred_count,
	nat* stack, nat stack_count,
	nat* value, nat* type, 
	nat* is_copy, nat* copy_of
) {

	print_instruction_window_around(pc, 0, "PC");
	print_dictionary(0);

	printf("        ");
	for (nat j = 0; j < var_count; j++) { if (is_constant[j]) continue; printf("%3lld ", j); }
	puts("\n-------------------------------------------------");
	for (nat i = 0; i < ins_count; i++) {
		printf("ct %3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_constant[j]) continue; 
			if (not type[i * var_count + j]) printf("\033[90m");
			printf("%3lld ", value[i * var_count + j]);
			if (not type[i * var_count + j]) printf("\033[0m");
		}
		putchar(9);

		printf("cp %3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_constant[j]) continue; 
			if (not is_copy[i * var_count + j]) printf("\033[90m");
			printf("%3lld ", copy_of[i * var_count + j]);
			if (not is_copy[i * var_count + j]) printf("\033[0m");
		}
		putchar(10);
	}
	puts("-------------------------------------------------");
	printf("[PC = %llu], pred:", pc);
	print_nats(preds, pred_count); putchar(32);
	printf("stack: "); 
	print_nats(stack, stack_count); 
	putchar(10);
}


static void debug_liveness(
	nat pc,
	nat* preds, nat pred_count,
	nat* gotos, nat goto_count,
	nat* stack, nat stack_count,
	nat* alive
) {
	print_instruction_window_around(pc, 0, "PC");
	print_dictionary(0);
	printf("    ");
	for (nat j = 0; j < var_count; j++) { if (is_constant[j]) continue; printf("%2lld ", j); } 
	puts("\n-------------------------------------------------");
	for (nat i = 0; i < ins_count; i++) {
		printf("%2llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_constant[j]) continue; 
			if (not alive[i * var_count + j]) printf("\033[90m");
			printf("%2lld ", alive[i * var_count + j]);
			if (not alive[i * var_count + j]) printf("\033[0m");
		}
		putchar(10);
	}
	puts("-------------------------------------------------");
	printf("[PC = %llu], pred:", pc);
	print_nats(preds, pred_count); putchar(32);
	printf(", goto:");
	print_nats(gotos, goto_count); putchar(32);
	printf(", stack: "); 
	print_nats(stack, stack_count); putchar(10);
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

int main(int argc, const char** argv) {

	if (argc != 2) exit(puts("compiler: error: usage: ./run [file.s]"));
	
	const nat min_stack_size = 16384 + 1;
	nat target_arch = no_arch;
	nat output_format = debug_output_only;
	nat should_overwrite = false;

	nat stack_size = min_stack_size;
	const char* output_filename = "output_file_from_compiler";

	const char* included_files[4096] = {0};
	nat included_file_count = 0;
	char* string_list[4096] = {0};
	nat string_list_count = 0;
	struct file files[4096] = {0};
	nat file_count = 1;
	
{ {
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

	nat word_length = 0, word_start = 0, arg_count = 0, is_immediate = 0;
	byte is_compiletime = 0, in_string = 0;

	nat args[max_arg_count] = {0};

	for (nat var = 0, op = 0, pc = starting_index; pc < text_length; pc++) {

		if (in_string) {
			op = 0; in_string = 0;
			while (isspace(text[pc])) pc++; 
			const char delim = text[pc];
			nat string_at = ++pc, string_length = 0;
			while (text[pc] != delim) { pc++; string_length++; }
			string_list[string_list_count++] = strndup(text + string_at, string_length);
			struct instruction new = { .op = string, .imm = 0xff, .state = is_compiletime };
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
				filename, text, text_length,
				word_start, pc
			);
		}
		else if (op == file) goto define_name;
		for (var = var_count; var--;) {
			if (not is_undefined[var] and 
			    not strcmp(word, variables[var])) {
				goto push_argument;
			}
		} 

		if (not(  
			(op == lt or op == ge or op == ne or op == eq) and arg_count == 2 or
			(op == set or op == ld or op == register_) and arg_count == 0 or
			op == do_ or op == at or op == la
		)) {
			nat r = 0, s = 1;
			for (nat i = 0; i < strlen(word); i++) {
				if (word[i] == '0') s <<= 1;
				else if (word[i] == '1') { r += s; s <<= 1; }
				else if (word[i] == '_') continue;
				else print_error(
					"undefined variable",
					filename, text, text_length,
					word_start, pc
				);
			}
			is_immediate |= 1 << arg_count;
			var = r;
			goto push_argument;
		}

	define_name:
		var = var_count;
		variables[var] = word; 
		is_constant[var] = is_compiletime;
		var_count++;

	push_argument: 
		args[arg_count++] = var;

	process_op: 
		if (op == string) { in_string = 1; goto next_word; } 
		else if (arg_count < arity[op]) goto next_word;
		else if (op == ct) is_compiletime = 1;
		else if (op == rt) is_compiletime = 0;
		else if (op == del) {
			if (is_immediate) 
				print_error(
					"expected defined variable, found binary literal",
					filename, text, text_length, 
					word_start, pc
				);
			is_undefined[args[0]] = 1;
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
			files[file_count].filename = word;
			files[file_count].text = string;
			files[file_count].text_length = len;
			files[file_count++].index = 0;
			var_count--;
			goto process_file;

		} else {
			if (op >= a6_nop and op < isa_count and is_compiletime) 
				print_error(
					"machine instruction cannot execute at compiletime",
					filename, text, text_length,
					word_start, pc
				);

			if (op == do_ or op == at) { 
				if (is_immediate & 1) { 
				error_label_immediate: 
					print_error(
						"expected label argument, found binary literal",
						filename, text, text_length,
						word_start, pc
					); 
				} else is_label[args[0]] = 1; 
			}
			if (op == la) { 
				if (is_immediate & 2) goto error_label_immediate; else is_label[args[1]] = 1;
			}
			if (op == lt or op == ge or op == ne or op == eq) {
				if (is_immediate & 4) goto error_label_immediate; else is_label[args[2]] = 1;
			}
			if ((op >= set and op <= ld) or op == register_ or op == bits) {
				if (is_immediate & 1) 
					print_error(
						"expected destination variable, found binary literal",
						filename, text, text_length,
						word_start, pc
					); 
			}

			struct instruction new = { 
				.op = op, 
				.imm = is_immediate,
				.state = is_compiletime,
			};
			memcpy(new.args, args, sizeof args);
			is_immediate = 0;
			ins[ins_count++] = new;
		}
		arg_count = 0; op = 0;
		next_word: word_length = 0;
	}
	file_count--;
	if (file_count) goto process_file; }

	if (not ins_count or ins[ins_count - 1].op != halt) 
		ins[ins_count++] = (struct instruction) { .op = halt, .imm = 0, .state = 0 };

	for (nat pc = 0; pc < ins_count; pc++) {
		const nat op = ins[pc].op;
		if (op == at) values[ins[pc].args[0]] = pc;
	}
		
	print_dictionary(1);
	print_instructions(0);
	puts("parsing finished.");
	//getchar();

	{ struct instruction rt_ins[4096] = {0};
	nat rt_ins_count = 0;

	memset(bit_count, 255, sizeof bit_count);
	memset(register_index, 255, sizeof register_index);
	uint8_t* memory = calloc(65536, sizeof(nat));
	bool should_debug_cte = 0;
	const nat compiletime_register_flag = (nat) (1LLU << 63LLU);

	for (nat pc = 0; pc < ins_count; pc++) {

		nat op = ins[pc].op;
		nat imm = ins[pc].imm;
		nat is_compiletime = ins[pc].state;

		nat arg0 = ins[pc].args[0];
		nat arg1 = ins[pc].args[1];
		nat arg2 = ins[pc].args[2];

		nat i0 = !!(imm & 1);
		nat i1 = !!(imm & 2);
		nat i2 = !!(imm & 4);

		nat val0 = not i0 ? values[arg0] : arg0;
		nat val1 = not i1 ? values[arg1] : arg1;
		nat val2 = not i2 ? values[arg2] : arg2;

		if (should_debug_cte) {
			print_instruction_window_around(pc, 0, "");
			print_dictionary(1);
			puts("rt instructions: ");
			for (nat i = 0; i < rt_ins_count; i++) {
				putchar(9); print_instruction(rt_ins[i]); puts("");
			}
			puts("done");
			getchar();
		}


		if (op == string and not is_compiletime) {
			for (nat s = 0; s < arg0; s++) { // for string length; 
				struct instruction new = { .op = emit, .imm = 3 };
				new.args[0] = 1;
				new.args[1] = (nat) string_list[arg1][s];
				rt_ins[rt_ins_count++] = new;
			}
		}

		else if (op == register_) register_index[arg0] = val1 | (is_compiletime * compiletime_register_flag);
		else if (op == bits)  bit_count[arg0] = val1;

		else if (not is_compiletime) {
			struct instruction new = { .op = op, .imm = imm };
			memcpy(new.args, ins[pc].args, sizeof new.args);
			for (nat i = 0; i < arity[op]; i++) {
				if (is_label[new.args[i]]) continue;
				if (not ((new.imm >> i) & 1) and is_constant[new.args[i]]) {
					new.args[i] = values[new.args[i]];
					new.imm |= 1 << i;
				}
			}

			imm = new.imm;
			i1 = !!(imm & 2);
			arg0 = new.args[0];
			arg1 = new.args[1];
			bool keep = 1; 

			if (op >= set and op <= sd and i1) {
				if (	(op == add or op == sub or op == si or op == sd or
					 op == eor or op == or_) and not arg1 or 
					(op == mul or op == div_) and arg1 == 1
				) keep = 0; 

				else if (op == and_ and not arg1) new.op = set;

				for (nat sh = 0; sh < 64; sh++) {
					if (op == mul and arg1 == (1LLU << sh)) {
						new.op = si; new.args[1] = sh; break;
					} else if (op == div_ and arg1 == (1LLU << sh)) {
						new.op = sd; new.args[1] = sh; break;
					} else if (op == rem and arg1 == (1LLU << sh)) {
						new.op = and_; new.args[1]--; break;
					}
				}

			} else if (op >= set and op <= sd and arg0 == arg1) {
				if (op == set or op == and_ or op == or_) keep = 0;
				else if (op == eor or op == sub) {
					new.op = set; new.imm |= 2; new.args[1] = 0;
				}
	
			} else if ((op == lt or op == ge or op == ne or 
				   op == eq) and arg0 == arg1 and not imm) {
				if (op == lt or op == ne) keep = 0;
				else if (op == eq or op == ge) {
					new.op = do_; new.args[0] = new.args[2];
				}
			}
			if (keep) rt_ins[rt_ins_count++] = new;

		}

		else if (op == at)   values[arg0]  = pc;
		else if (op == set)  values[arg0]  = val1;
		else if (op == add)  values[arg0] += val1;
		else if (op == sub)  values[arg0] -= val1;
		else if (op == mul)  values[arg0] *= val1;
		else if (op == div_) values[arg0] /= val1;
		else if (op == rem)  values[arg0] %= val1;
		else if (op == and_) values[arg0] &= val1;
		else if (op == or_)  values[arg0] |= val1;
		else if (op == eor)  values[arg0] ^= val1;
		else if (op == si)   values[arg0] <<= val1;
		else if (op == sd)   values[arg0] >>= val1;

		else if (op == ld) {
			values[arg0] = 0;
			for (nat i = 0; i < val2; i++) 
				values[arg0] |= (nat) ((nat) memory[val1 + i] << (8LLU * i));

		} else if (op == st) {
			for (nat i = 0; i < val2; i++) 
				memory[val0 + i] = (val1 >> (8 * i)) & 0xFF;

		} else if (op == do_)  pc = values[arg0];
		else if (op == lt) { if (val0  < val1) pc = values[arg2]; }
		else if (op == ge) { if (val0 >= val1) pc = values[arg2]; }
		else if (op == eq) { if (val0 == val1) pc = values[arg2]; }
		else if (op == ne) { if (val0 != val1) pc = values[arg2]; }
		else if (op == system_) {
			nat n = 0, output = 0;
			for (nat i = 0; i < var_count; i++) {
				if (register_index[i] == (0 | compiletime_register_flag)) n = values[i];
				if (register_index[i] == (1 | compiletime_register_flag)) output = i;
			}
			const nat data = values[output];

			if (n == compiler_abort) abort();
			else if (n == compiler_exit) exit(0);

			else if (n == compiler_getchar) values[output] = read_single_char_from_stdin();
			else if (n == compiler_putchar) { putchar((int) data); fflush(stdout); }

			else if (n == compiler_printbin) { print_binary(data); fflush(stdout); }
			else if (n == compiler_printdec) { printf("%llu", data); fflush(stdout); }

			else if (n == compiler_setdebug) should_debug_cte = data;
			else if (n == compiler_print) { printf("%s", string_list[data]); fflush(stdout); }

			else if (n == compiler_target) target_arch = data;
			else if (n == compiler_format) output_format = data;

			else if (n == compiler_overwrite) should_overwrite = data;
			else if (n == compiler_getlength) values[output] = strlen(string_list[data]);
			
			else if (n == compiler_gettarget) values[output] = target_arch;
			else if (n == compiler_getformat) values[output] = output_format;

			else if (n == compiler_stacksize) stack_size = data;
			else if (n == compiler_getstacksize) values[output] = stack_size;
			else { puts("error: unknown compiler CT system call"); abort(); } 
		} else { 
			printf("CTE: fatal internal error: "
				"unknown instruction executed: %s...\n", 
				operations[op]
			); 
			abort(); 
		} 
	}

	memcpy(ins, rt_ins, ins_count * sizeof(struct instruction));
	ins_count = rt_ins_count;   }

	if (target_arch == msp430_arch and stack_size) { 
		puts("fatal error: nonzero stack size for msp430 is not permitted"); 
		abort();

	} else if (target_arch == arm64_arch and stack_size < min_stack_size) {
		puts("warning: stack size less than the minimum size for arm64");
	}

	print_dictionary(1);
	print_instructions(0);
	puts("CT-PRUNED-EXECUTION finished.");

	{ nat* type = calloc(ins_count * var_count, sizeof(nat));
	nat* value = calloc(ins_count * var_count, sizeof(nat));
	nat* is_copy = calloc(ins_count * var_count, sizeof(nat));
	nat* copy_of = calloc(ins_count * var_count, sizeof(nat));

	nat stack[4096] = {0};
	nat stack_count = 1;

	for (nat i = 0; i < ins_count; i++)  ins[i].state = 0;

	while (stack_count) {
		nat pc = stack[--stack_count];

		nat pred_count = 0;
		nat* preds = compute_predecessors(pc, &pred_count);
		nat* gotos = compute_successors(pc);

		debug_data_flow_state(pc, preds, pred_count, stack, stack_count, value, type, is_copy, copy_of);
		//getchar();

		ins[pc].state++;

		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;
		const nat gt0 = gotos[0];
		const nat gt1 = gotos[1];
		const nat a0 = ins[pc].args[0];
		const nat a1 = ins[pc].args[1];
		const nat i0 = !!(imm & 1);
		const nat i1 = !!(imm & 2);

		for (nat var = 0; var < var_count; var++) {

			nat future_type = 0;
			nat future_value = 0;
			nat future_is_copy = 0;
			nat future_copy_of = 0;

			nat first_ct = 1;
			nat first_cp = 1;
		
			for (nat iterator_p = 0; iterator_p < pred_count; iterator_p++) {
				const nat pred = preds[iterator_p];
				if (not ins[pred].state) continue;

				const nat ct_t = type[pred * var_count + var];
				const nat ct_v = value[pred * var_count + var];
				const nat cp_t = is_copy[pred * var_count + var];
				const nat cp_v = copy_of[pred * var_count + var];

				if (ct_t == 0) future_type = 0;
				else if (first_ct) { 
					future_type = 1;
					future_value = ct_v; 
					first_ct = 0;
				} else if (future_value != ct_v) 
					future_type = 0;

				if (cp_t == 0) future_is_copy = 0;
				else if (first_cp) { 
					future_is_copy = 1; 
					future_copy_of = cp_v; 
					first_cp = 0;
				} else if (future_copy_of != cp_v) 
					future_is_copy = 0;
			}

			type[pc * var_count + var] = future_type;
			value[pc * var_count + var] = future_value;
			is_copy[pc * var_count + var] = future_is_copy;
			copy_of[pc * var_count + var] = future_copy_of;	
		}

	
		const nat ct0 = 0 < arity[op] ? (i0 or type[pc * var_count + a0]) : 0;
		const nat ct1 = 1 < arity[op] ? (i1 or type[pc * var_count + a1]) : 0;

		nat v0 = 0;
		if (i0) v0 = a0; else if (pc < ins_count and a0 < var_count) v0 = value[pc * var_count + a0];

		nat v1 = 0;
		if (i1) v1 = a1; else if (pc < ins_count and a1 < var_count) v1 = value[pc * var_count + a1];

		nat a0_is_copy = 0;
		if (pc < ins_count and a0 < var_count) a0_is_copy = is_copy[pc * var_count + a0];

		nat a0_copy_ref = 0;
		if (pc < ins_count and a0 < var_count) a0_copy_ref = copy_of[pc * var_count + a0];

		nat out_t = ct0, out_v = v0;
		nat out_is_copy = a0_is_copy, out_copy_ref = a0_copy_ref;

		if (op == halt) continue;
		else if (op == at) { }
		else if (op == m4_sect) { } 
		
		else if (op == system_) {

			/* nat n = (nat) -1;
			for (nat i = 0; i < var_count; i++) {
				if (register_index[i] == 17 and // generalize this! 
				// system_call_number_register(target) and 
				type[pc * var_count + i]) {
					n = value[pc * var_count + i]; 
					break;
				}
			}
			
			if (n == (nat) -1) {
				for (nat r = 10; r < 16; r++)
				for (nat i = 0; i < var_count; i++)
				if (register_index[i] == r)
				type[pc * var_count + i] = 0;

			} else if (n == 2) {
				for (nat i = 0; i < var_count; i++)
				if (register_index[i] == 10) // arg0 on riscv, outparam of   k = read();
							// todo, errno should also be returned as an outparam!
				type[pc * var_count + i] = 0;

			} else if (n == 3) {
				for (nat i = 0; i < var_count; i++)
				if (register_index[i] == 10) // arg0 on riscv, outparam of   k = read(); ...(+ errno?)
				type[pc * var_count + i] = 0;
			}*/

		}
		else if (op == emit) { } 
		else if (op == do_) { }
		else if (op == set) {
			if (register_index[a0] == (nat) -1) { out_t = ct1; out_v = v1; }
			if (not i1) {
				out_is_copy = 1;
				out_copy_ref = is_copy[pc * var_count + a1] ? copy_of[pc * var_count + a1] : a1;
			}
		}

		else if (op >= add and op <= sd) {
			out_t = ct0 and ct1;

			     if (op == add)  out_v += v1;
			else if (op == sub)  out_v -= v1;
			else if (op == mul)  out_v *= v1;
			else if (op == div_) out_v /= v1;
			else if (op == rem)  out_v %= v1;
			else if (op == and_) out_v &= v1;
			else if (op == or_)  out_v |= v1;
			else if (op == eor)  out_v ^= v1;
			else if (op == si)   out_v <<= v1;
			else if (op == sd)   out_v >>= v1;

			out_is_copy = 0;
			
		} else if (op == st) {

		} else if (op == ld) {
			out_t = 0;
			out_is_copy = 0;

		} else if (op == la) {
			out_t = 0;
			out_v = 0;
			out_is_copy = 0;

		} else if (op == lt or op == ge or op == ne or op == eq) {
			if (not ct0 or not ct1) {
				if (gt0 < ins_count and ins[gt0].state < 2) stack[stack_count++] = gt0;
				if (gt1 < ins_count and ins[gt1].state < 2) stack[stack_count++] = gt1; 
				continue;
			} else {
				bool cond = 0;
				     if (op == lt) cond = v0  < v1;
				else if (op == ge) cond = v0 >= v1;
				else if (op == eq) cond = v0 == v1;
				else if (op == ne) cond = v0 != v1;

				const nat target = cond ? gt1 : gt0;
				if (target < ins_count and ins[target].state < 2) 
					stack[stack_count++] = target; 
				continue;
			}		

		} else if (not (op >= a6_nop and op < isa_count)) {
			puts("WARNING: EXECUTING AN UNKNOWN INSTRUCTION WITHOUT AN IMPLEMENTATION!!!");
			puts(operations[op]);
			abort();
		}

		if (pc < ins_count and a0 < var_count) {
			type[pc * var_count + a0] = out_t;
			value[pc * var_count + a0] = out_v;
			is_copy[pc * var_count + a0] = out_is_copy;
			copy_of[pc * var_count + a0] = out_copy_ref;
		}
		if (gt0 < ins_count and ins[gt0].state < 2) stack[stack_count++] = gt0; 

		if (op == la) {
			const nat label = compute_label_location(a1);
			if (label < ins_count and ins[label].state < 2) stack[stack_count++] = label; 
			continue;
		}

		if (op >= set and op <= ld) {
			for (nat i = 0; i < var_count; i++) {
				if (is_copy[pc * var_count + i] and 
				    copy_of[pc * var_count + i] == a0) 
					is_copy[pc * var_count + i] = 0;
			}
		}
	}

	debug_data_flow_state(0, NULL, 0, stack, stack_count, value, type, is_copy, copy_of);
	puts("data flow: [FINAL VALUES]");
	// getchar();
	
	print_instructions(0);
	puts("OPT2 finished.");
	//getchar();

	puts("pruning ctk instructions...");

	for (nat i = 0; i < ins_count; i++) {

		//if (not ins[i].state) ins[i].op = 0;

		print_instruction_window_around(i, 0, "");
		puts("-----------PRUNING CTK INS:---------------");
		//getchar();

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		
		nat keep = 0;

		if (ins[i].state and  
			(
			op == halt 	or op == system_ or 
			op == set 	or op == do_ or
			op == at 	or op == la or 
			op == ld 	or op == st or
			op == emit	or op >= a6_nop
			)
		) keep = 1;

		if (not keep and ins[i].state)
		for (nat a = 0; a < arity[op]; a++) {

			if (op == at and a == 0) continue;
			if (op == do_ and a == 0) continue;
			if (op == lt and a == 2) continue;
			if (op == ge and a == 2) continue;
			if (op == ne and a == 2) continue;
			if (op == eq and a == 2) continue;
			if (op == la and a == 1) continue; 
						
			if (((imm >> a) & 1)) {
				printf("found a compiletime immediate : %llu\n", 
					ins[i].args[a]
				);

			} else if (register_index[ins[i].args[a]] != (nat) -1) {

				printf("warning: found a register index variable "
					"as argument  :  %s\n",
					variables[ins[i].args[a]]
				); abort();


			} else if (type[i * var_count + ins[i].args[a]]) {
				
				printf("found a compiletime variable "
					"as argument  :  %s = {type = %llu, value = %llu}\n",
					variables[ins[i].args[a]], 
					type[i * var_count + ins[i].args[a]],
					value[i * var_count + ins[i].args[a]]
				);

			} else {
				puts("found a runtime argument!");
				printf("found variable "
					":  %s = {type = %llu, value = %llu}\n",
					variables[ins[i].args[a]], 
					type[i * var_count + ins[i].args[a]],
					value[i * var_count + ins[i].args[a]]
				);
				keep = 1; break;
			}
		}

		if (not keep or not ins[i].state) {
			if (op == lt or op == ge or op == ne or op == eq) {
				const nat v0 = (imm & 1) ? ins[i].args[0] : value[i * var_count + ins[i].args[0]];
				const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];
				bool cond = 0;
				     if (op == lt) cond = v0  < v1;
				else if (op == ge) cond = v0 >= v1;
				else if (op == eq) cond = v0 == v1;
				else if (op == ne) cond = v0 != v1;
				if (cond) { ins[i].op = do_; ins[i].args[0] = ins[i].args[2]; }
				else ins[i].state = 0;

			} else { 
				puts("NOTE: found a compiletime-known instruction! deleting this instruction."); 
				//getchar();
				ins[i].state = 0; 
			} 
			continue;
		}

		puts("found real RT isntruction!"); 
		putchar('\t');
		print_instruction(ins[i]); 
		puts("");
		//getchar();

		if (op >= set and op <= sd) {
			const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
			const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

			if (ct1 and not (imm & 2)) {
				puts("inlining compiletime argument...\n"); //getchar();
				ins[i].args[1] = v1;
				ins[i].imm |= 2;
			}

			if (ins[i].imm & 2) {
				const nat c = ins[i].args[1];
				if (	(op == add or op == sub or
					 op == si or op == sd or
					 op == eor or op == or_)
					and not c or 
					(op == mul or op == div_)
					and c == 1	
				) ins[i].state = 0; 
				else if (op == and_ and not c) ins[i].op = set;

				for (nat sh = 0; sh < 64; sh++) {
					if (op == mul and c == (1LLU << sh)) {
						ins[i].op = si;
						ins[i].args[1] = sh;
						break;
					} else if (op == div_ and c == (1LLU << sh)) {
						ins[i].op = sd;
						ins[i].args[1] = sh;
						break;
					} 
				}
			}

			if (not (ins[i].imm & 2) and is_copy[i * var_count + ins[i].args[1]]) {

				printf("note: inlining copy reference: a1=%llu imm=%llu copy_of=%llu, i=%llu...\n", 
					ins[i].args[1], ins[i].imm, 
					copy_of[i * var_count + ins[i].args[1]],
					i
				);

				puts("original:");
				print_instruction(ins[i]); puts("");

				ins[i].args[1] = copy_of[i * var_count + ins[i].args[1]];

				puts("modified form:"); 
				print_instruction(ins[i]); 
				puts("");
			}

			if (ins[i].args[0] == ins[i].args[1]) {
				if (op == set or op == and_ or op == or_) { 
					puts("found a rt NOP! deleting this instruction."); 
					ins[i].state = 0; }
				else if (op == eor or op == sub) {
					ins[i].op = set;
					ins[i].imm |= 2;
					ins[i].args[1] = 0;
				}
			}


		} else if (op == ld or op == st) {
			puts("we still need to embed the immediates into the load and store instructions.");
			abort();


		} else if (op == lt or op == ge or op == ne or op == eq) {
			const nat ct0 = (imm & 1) or type[i * var_count + ins[i].args[0]];
			const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
			const nat v0 = (imm & 1) ? ins[i].args[0] : value[i * var_count + ins[i].args[0]];
			const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

			if (ct0 and not (imm & 1)) {
				ins[i].args[0] = v0;
				ins[i].imm |= 1;

			} else if (ct1 and not (imm & 2)) {
				ins[i].args[1] = v1;
				ins[i].imm |= 2;
			}

			if (not ins[i].imm and ins[i].args[0] == ins[i].args[1]) {
				if (op == lt or op == ne) ins[i].state = 0; 
				else if (op == eq or op == ge) {
					ins[i].op = do_;
					ins[i].args[0] = ins[i].args[2];
				}
			}
		}
	}

	{ nat final_ins_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (not ins[i].state) continue;
		ins[final_ins_count++] = ins[i];
	}
	ins_count = final_ins_count; } } 

	print_instructions(0);
	puts("CTK PRUNING finished.");
	//getchar();

	if (target_arch == rv32_arch or target_arch == rv64_arch) {
		puts("replacing branch immediates with branch register, on rv32...");

		for (nat i = 0; i < ins_count; i++) {
			const nat op = ins[i].op;
			const nat imm = ins[i].imm;
			const nat i0 = !!(imm & 1);
			const nat i1 = !!(imm & 2);
			const nat a0 = ins[i].args[0];
			const nat a1 = ins[i].args[1];

			if (not (op == lt or op == ge or op == ne or op == eq)) continue;
			if ((i0 and a0) or (i1 and a1)) {
				if (i0) {
					nat t = a0;
					ins[i].args[0] = ins[i].args[1];
					ins[i].args[1] = t;
				}
				const nat n = ins[i].args[1];
				
				variables[var_count] = strdup("MY_NEW"); //strdup(generate_new_variable_name());
				var_count++;

				memmove(ins + i + 1, ins + i, sizeof(struct instruction) * (ins_count - i));
				ins[i] = (struct instruction) { set, 0x2, 0, { var_count - 1, n } };
				ins_count++;

				ins[i + 1].args[1] = var_count - 1;
				ins[i + 1].imm = 0;

				puts("rv32 replace br imm: info: inserted a set statement!");
				//getchar();
				i++;
			}
		}
	}
	
	print_instructions(0);
	puts("non imm branches for riscv done.");
	//getchar();


	printf("info: compiling for [target_architecture = %llu, output_format = %llu (%s)]\n", 
		target_arch, output_format, 
		should_overwrite ? "overwrite" : "non-destructive"
	);

	if (not target_arch) exit(0);

	struct instruction mi[4096] = {0};
	nat mi_count = 0;

	for (nat i = 0; i < ins_count; i++)
		ins[i].state = 0;

	if (target_arch == rv32_arch) 	goto rv32_instruction_selection;
	if (target_arch == rv64_arch) 	goto rv32_instruction_selection;
	if (target_arch == msp430_arch) goto msp430_instruction_selection;
	if (target_arch == arm64_arch) 	goto arm64_instruction_selection;
	puts("instruction selection: error: unknown target"); abort();


rv32_instruction_selection:;
	puts("rv32: instruction selection starting...");
	{ struct instruction new = {0};
	const nat unrecognized = (nat) -1;

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("rv32 machine instructions:");
		for (nat e = 0; e < mi_count; e++) {
			printf("%llu: ", e); 
			print_instruction(mi[e]); 
			puts("");
		}
		puts("[mi done]");
		puts("[RISC-V ins sel]");
		//getchar();

		if (ins[i].state) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i);
			getchar(); 
			continue;
		}

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1]; 
		const nat arg2 = ins[i].args[2]; 
		const nat i0 = !!(imm & 1);
		const nat i1 = !!(imm & 2);
		//const nat i2 = !!(imm & 4);

		if (	
			op == r5_i or op == r5_r or 
			op == r5_s or op == r5_b or 
			op == r5_u or op == r5_j or 
			op == at or op == emit or op == halt
		) { 
			new = ins[i]; 
			goto r5_push_single_mi; 
		}

	
/*


current state:  1202505235.133756



		set s r
		addi s k
		ld d s 64


		set s r
		addi s k
		ld d s 64


				question:  what about 


					addi s k
					ld d s 64                   ie, it modifies the source?   
								how do we represent this semantics?...


		


			if (op == add and not imm) {
				const nat j = locate_instruction(
					(struct expected_instruction) {
						.op = op_A[this],
						.use = 1,
						.args[0] = arg0
					}, i + 1
				);
				if (j == unrecognized) goto skip_set_r;
				new = (struct instruction) { 
					r5_r, 0x25, 0,   
					{ 0x33, arg0, op_B1[this], arg1, ins[j].args[1], op_B2[this],    0,0 } 
				};
				ins[j].state = 1; 
				goto r5_push_single_mi;
				skip_set_r:;
			} 


		*/



		//   set d m  OP_A d n   -->   OP_B d n m
		{
		nat op_A [] = {add,  sub,  mul,  div_, rem,  and_,  or_,  eor,  si,   sd,  };
		nat op_B1[] = {0,    0,    0,    5,    7,    7,     6,    4,    1,    5,   };
		nat op_B2[] = {0,    0x20, 1,    1,    1,    0,     0,    0,    0,    0,   };

		for (nat this = 0; this < 10; this++) {
			if (op == set and not imm) {
				const nat j = locate_instruction(
					(struct expected_instruction) {
						.op = op_A[this],
						.use = 1,
						.args[0] = arg0
					}, i + 1
				);
				if (j == unrecognized) goto skip_set_r;
				new = (struct instruction) { 
					r5_r, 0x25, 0,   
					{ 0x33, arg0, op_B1[this], arg1, ins[j].args[1], op_B2[this],    0,0 } 
				};
				ins[j].state = 1; 
				goto r5_push_single_mi;
				skip_set_r:;
			} 
		}}

		//   OP_A d n   -->   OP_B d d n
		{
		nat op_A [] = {add,  sub,  mul,  div_, rem,  and_,  or_,  eor,  si,   sd,  };
		nat op_B1[] = {0,    0,    0,    5,    7,    7,     6,    4,    1,    5,   };
		nat op_B2[] = {0,    0x20, 1,    1,    1,    0,     0,    0,    0,    0,   };

		for (nat this = 0; this < 10; this++) {
			if (op == op_A[this] and not imm) {
				new = (struct instruction) { 
					r5_r, 0x25, 0,   
					{ 0x33, arg0, op_B1[this], arg0, arg1, op_B2[this],    0,0 } 
				};
				goto r5_push_single_mi;
			} 
		}}


		{
		nat op_A [] = { lt, ge, ne, eq, };
		nat op_B1[] = { 6,  7,  1,  0,  };

		for (nat this = 0; this < 4; this++) {
			if (op == op_A[this] and i1 and not arg1) {
				new = (struct instruction) { r5_b, 0xB, 0,  { 0x63, op_B1[this], arg0, 0, arg2,   0,0,0 } };
				goto r5_push_single_mi;
			}
			if (op == op_A[this] and i0 and not arg0) {
				new = (struct instruction) { r5_b, 0x7, 0,  { 0x63, op_B1[this], 0, arg1, arg2,   0,0,0 } };
				goto r5_push_single_mi;
			}
			if (op == op_A[this]) {
				if (imm) {
					puts("rv32 ins sel: internal error: no branch immediates should be possible."); 
					abort();
				}
				new = (struct instruction) { r5_b, 0x3, 0, {  0x63, op_B1[this], arg0, arg1, arg2,   0,0,0 } };
				goto r5_push_single_mi;	
			} 
		}}

		if (op == system_) {
			new = (struct instruction) { r5_i, 0xff, 0,  { 0x73,0,0,0, 0,0,0,0 } };
			goto r5_push_single_mi;
		}

		else if (op == set and not imm) { // addi d n 0 
			new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg1, 0, 0,0,0 } };
			goto r5_push_single_mi;
		}
		else if (op == set and imm) { // addi d zr k
			new = (struct instruction) { r5_i, 0x1D, 0,   { 0x13, arg0, 0, 0, arg1, 0,0,0 } };
			goto r5_push_single_mi;
		}
		else if (op == add and imm) { // addi d d k
			new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg0, arg1,0,  0,0 } };
			goto r5_push_single_mi;
		}
		else if (op == sub and imm) { // addi d d -k
			nat k = (-arg1) & 0xFFF;
			new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg0, k, 0,  0,0 } };
			goto r5_push_single_mi;
		}
		else if (op == la) {
			new = (struct instruction) { r5_u, 0x5, 0,  { 0x17, arg0, arg1, 0x42,  0,0,0,0 } };
			mi[mi_count++] = new;
			new = (struct instruction) { r5_i, 0x15, 0, { 0x13, arg0, 0, arg0, arg1, 0x42, 0,0 } };
			goto r5_push_single_mi;
		}
			
		puts("error: unknown instruction selection pattern");
		abort();

	r5_push_single_mi:
		mi[mi_count++] = new;
		ins[i].state = 1;
	}}

	goto finish_instruction_selection;



msp430_instruction_selection:

	puts("msp430: instruction selection starting...");
	{ struct instruction new = {0};
	const nat unrecognized = (nat) -1;

	const nat msp_mov = 4;
/*	const nat msp_add = 5;
	const nat msp_addc = 6;
	const nat msp_sub = 7;
	const nat msp_subc = 8;
	const nat msp_cmp = 9;
	const nat msp_dadd = 10;
	const nat msp_bit = 11;
	const nat msp_bic = 12;
	const nat msp_bis = 13;
	const nat msp_xor = 14;
	const nat msp_and = 15;
*/
	const nat reg_mode = 0;
/*	const nat index_mode = 1;
	const nat deref_mode = 2;
 	const nat incr_mode = 3;
*/

/* 

set pc 0
set sp 1
set sr 01
set cg 11
set r4 001
...
set r15 1111


set condjnz 0
set condjz 1
set condjnc 01
set condjc 11
set condjn 001
set condjge 101
set condjl 011
set condjmp 111

set size_byte 1
set size_word 0

set reg_mode 0
set index_mode 1
set deref_mode 01
set incr_mode 11

set imm_mode incr_mode
set imm_reg pc

set literal_mode index_mode
set constant_1 cg

set fixed_reg sr
set fixed_mode index_mode

set nat8 1
set nat16 01

*/

	// gen4  op(0)  dm(1) dr(2) di(3)  sm(4) sr(5) si(6)   size(7)
	// eg:
	//  m4_op  msp_mov reg_mode r6 0   reg_mode r7  0      size_word

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("msp430 machine instructions:");
		for (nat e = 0; e < mi_count; e++) {
			printf("%llu: ", e); 
			print_instruction(mi[e]); 
			puts("");
		}
		puts("[mi done]");
		puts("[MSP430 ins sel]");
		//getchar();

		if (ins[i].state) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i);
			getchar(); 
			continue;
		}

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1]; 
		//const nat arg2 = ins[i].args[2]; 
		//const nat i0 = !!(imm & 1);
		//const nat i1 = !!(imm & 2);
		//const nat i2 = !!(imm & 4);

		if (	
			op == m4_op or op == m4_sect or op == m4_br or
			op == at or op == emit or op == halt
		) { 
			new = ins[i]; 
			goto msp430_push_single_mi; 
		}

	
		/*
		//   set d m  OP_A d n   -->   OP_B d n m
		{
		nat op_A [] = {add,  sub,  mul,  div_, rem,  and_,  or_,  eor,  si,   sd,  };
		nat op_B1[] = {0,    0,    0,    5,    7,    7,     6,    4,    1,    5,   };
		nat op_B2[] = {0,    0x20, 1,    1,    1,    0,     0,    0,    0,    0,   };

		for (nat this = 0; this < 10; this++) {
			if (op == set and not imm) {
				const nat j = locate_instruction(
					(struct expected_instruction) {
						.op = op_A[this],
						.use = 1,
						.args[0] = arg0
					}, i + 1
				);
				if (j == unrecognized) goto skip_set_r;
				new = (struct instruction) { 
					r5_r, 0x25, 0,   
					{ 0x33, arg0, op_B1[this], arg1, ins[j].args[1], op_B2[this],    0,0 } 
				};
				ins[j].state = 1; 
				goto msp430_push_single_mi;
				skip_set_r:;
			} 
		}}*/



		if (op == set and not imm) { // addi d n 0
			new = (struct instruction) {
				m4_op, 0xFFFFF, 0, { msp_mov,
					reg_mode, arg0, 0,
					reg_mode, arg1, 0,
					0
				}
			};
			goto msp430_push_single_mi;
		}
		else if (op == set and imm) { // addi d zr k
			new = (struct instruction) { r5_i, 0x1D, 0,   { 0x13, arg0, 0, 0, arg1, 0,0,0 } };
			goto msp430_push_single_mi;
		}
		else if (op == add and imm) { // addi d d k
			new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg0, arg1,0,  0,0 } };
			goto msp430_push_single_mi;
		}
		else if (op == sub and imm) { // addi d d -k
			nat k = (-arg1) & 0xFFF;
			new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg0, k, 0,  0,0 } };
			goto msp430_push_single_mi;
		}
		else if (op == la) {
			new = (struct instruction) { r5_u, 0x5, 0,  { 0x17, arg0, arg1, 0x42,  0,0,0,0 } };
			mi[mi_count++] = new;
			new = (struct instruction) { r5_i, 0x15, 0, { 0x13, arg0, 0, arg0, arg1, 0x42, 0,0 } };
			goto msp430_push_single_mi;
		}
			
		puts("error: unknown instruction selection pattern");
		abort();

	msp430_push_single_mi:
		mi[mi_count++] = new;
		ins[i].state = 1;
	}}

	goto finish_instruction_selection;




arm64_instruction_selection:;

	puts("arm64: instruction selection starting...");

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("[ARM64 ins sel]");
		//getchar();

		if (ins[i].state) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i); 
			//getchar();
			continue;
		}

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1]; 


		if (op == halt or op == at or op == emit or (op >= a6_nop and op <= a6_divr)) { 
			mi[mi_count++] = ins[i]; 
			ins[i].state = 1; 
			continue;
		}

		if (op == set) {
			const nat b = locate_instruction(
				(struct expected_instruction){ .op = si, .imm = 2, .use = 1, .args[0] = arg0 },
				i + 1
			);
			if (b == (nat) -1) goto addsrlsl_bail;

			const nat c = locate_instruction(
				(struct expected_instruction){ .op = add, .use = 1, .args[0] = arg0 },
				b + 1
			);
			if (c == (nat) -1) goto addsrlsl_bail;

			struct instruction new = { .op = a6_addr, .imm = 0xf8 };
			new.args[0] = arg0; 		// d
			new.args[1] = ins[c].args[1]; 	// n
			new.args[2] = arg1; 		// m 
			new.args[3] = ins[b].args[1]; 	// k
			new.args[4] = 0; //???sb?
			new.args[5] = 0; // ????sf???
			mi[mi_count++] = new;
			ins[i].state = 1; ins[b].state = 1; ins[c].state = 1;
			continue;
		} addsrlsl_bail:

		if (op == set and not imm) {
			struct instruction new = { .op = a6_orr, .imm = 0xff };
			new.args[0] = arg0;
			new.args[1] = 0;
			new.args[2] = arg1;			
			new.args[3] = 0;
			mi[mi_count++] = new;
			ins[i].state = 1;
			continue;
		}

		else if (op == set and imm) {
			struct instruction new = { .op = a6_mov, .imm = 0xfe };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = 0;
			new.args[3] = 0;
			new.args[4] = 0;
			mi[mi_count++] = new;
			ins[i].state = 1;
			continue;
		}

		if (op == lt and not imm) {
			struct instruction new = { .op = a6_addr, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = lt;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			ins[i].state = 1; 
			continue;
		}

		if (op == eq and not imm) {
			struct instruction new = { .op = a6_orr, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = eq;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			ins[i].state = 1; 
			continue;
 		}

		if (op == lt and imm) {			
			struct instruction new = { .op = a6_addi, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = lt;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			ins[i].state = 1; 

			puts("ins sel: unimplemented: we need to detrmine the the label still!");
			abort();
		}

		if (op == eq and imm) {
			struct instruction new = { .op = a6_ori, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = eq;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			ins[i].state = 1; 
			continue;
		}

		if (op == system_) {
			struct instruction new = { .op = a6_svc, .imm = 0xff };
			mi[mi_count++] = new;
			ins[i].state = 1;
			continue;
		}
	}



finish_instruction_selection:;

	for (nat i = 0; i < ins_count; i++) {
		if (not ins[i].state) {
			puts("error: instruction unprocessed by ins sel: internal error");
			puts("error: this instruction failed to be lowered:\n");
			print_instruction_window_around(i, 1, "not selected instruction!");
			puts("");
			abort();
		}
	}
	
	for (nat i = 0; i < mi_count; i++) ins[i] = mi[i];
	ins_count = mi_count;

	puts("finished instruction selection!");
	printf("info: preliminary machine code prior to RA: for target = %llu\n", target_arch);
	print_instructions(1);
	//getchar();

	puts("RA: starting register allocation!");           







				// BUG IN RA:   we need to be looking at the number of "disjoint live ranges" for a variable. and treating those as seperate variables!!!!










	nat hardware_register_count = 0; 
	if (target_arch == rv32_arch) hardware_register_count = 31;
	else if (target_arch == msp430_arch) hardware_register_count = 12;
	else {
		puts("cannot perform RA for this target, unimplemented.");
		abort();
	}

	nat allocation[max_variable_count] = {0};
	{ nat* alive = calloc(ins_count * var_count, sizeof(nat)); 
	nat stack[4096] = {0};
	nat stack_count = 0;	
	for (nat i = 0; i < ins_count; i++) 
		if (ins[i].op == halt) stack[stack_count++] = i;
	for (nat i = 0; i < ins_count; i++)  ins[i].state = 0;

	while (stack_count) {
		nat pc = stack[--stack_count];

		nat pred_count = 0;  nat* preds = NULL;
		nat goto_count = 0;  nat* gotos = NULL;

		if (target_arch == rv32_arch) {
			preds = compute_riscv_predecessors(pc, &pred_count);
			gotos = compute_riscv_successors(pc);

		} else if (target_arch == msp430_arch) {
			preds = compute_msp430_predecessors(pc, &pred_count);
			gotos = compute_msp430_successors(pc);
		}

		if (gotos[0] != (nat) -1) goto_count++;
		if (gotos[1] != (nat) -1) goto_count++;

		debug_liveness(pc, preds, pred_count, gotos, goto_count, stack, stack_count, alive);
		printf("executing: [pc = %llu]: ", pc); 
		print_instruction(ins[pc]);
		puts("");
		//getchar();

		ins[pc].state++;

		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;

		const nat a0 = ins[pc].args[0];
		const nat a1 = ins[pc].args[1];
		const nat a2 = ins[pc].args[2];
		const nat a3 = ins[pc].args[3];
		const nat a4 = ins[pc].args[4];
		const nat a5 = ins[pc].args[5];
		
		//const nat i0 = !!(imm & (1 << 0));
		const nat i1 = !!(imm & (1 << 1));
		const nat i2 = !!(imm & (1 << 2));
		const nat i3 = !!(imm & (1 << 3));
		const nat i4 = !!(imm & (1 << 4));
		const nat i5 = !!(imm & (1 << 5));

		for (nat var = 0; var < var_count; var++) {
			nat future_alive = 0;		
			for (nat i = 0; i < goto_count; i++) {
				const nat gt = gotos[i];
				if (not ins[gt].state) continue;
				if (alive[gt * var_count + var]) future_alive = 1;
			}
			alive[pc * var_count + var] = future_alive;
		}

		if (op == halt) {}
		else if (op == at) {}
		else if (op == emit) {}

		else if (target_arch == rv32_arch) {
			if (op == r5_r) { // addr D A A
				if (not i1) alive[pc * var_count + a1] = 0;
				if (not i3) alive[pc * var_count + a3] = 1;
				if (not i4) alive[pc * var_count + a4] = 1;
			} else if (op == r5_i) { // addi D A
				if (not i1) alive[pc * var_count + a1] = 0;
				if (not i3) alive[pc * var_count + a3] = 1;
			} else if (op == r5_s or op == r5_b) { // BLT X Y   or // STW A A
				if (not i2) alive[pc * var_count + a2] = 1;
				if (not i3) alive[pc * var_count + a3] = 1;
			} else if (op == r5_u or op == r5_j) { // JAL D   or  // LUI D
				if (not i1) alive[pc * var_count + a1] = 0;
			} else goto unknown_liveness_error;

		} else if (target_arch == msp430_arch) {
			const nat msp_mov = 4, reg_mode = 0;
			if (op == m4_op) { 
				if (not i2) alive[pc * var_count + a2] = not (a0 == msp_mov and a1 == reg_mode);
				if (not i5) alive[pc * var_count + a5] = 1;

			} else if (op == m4_br) {
				// nothing.

			} else if (op == m4_sect) {
				// nothing.

			} else goto unknown_liveness_error;
		}

		for (nat i = 0; i < pred_count; i++) {
			if (preds[i] < ins_count and ins[preds[i]].state < 2) 
				stack[stack_count++] = preds[i]; 
		}
		continue;

	unknown_liveness_error:
		puts("liveness: error: processing an unknown instruction without an implementation!!!");
		puts(operations[op]);
		abort();
	}
	print_instructions(0);
	puts("liveness analysis finished.");

	nat* needs_ra = calloc(var_count, sizeof(nat));

	for (nat i = 0; i < ins_count; i++) {
		for (nat j = 0; j < var_count; j++) {
			if (alive[i * var_count + j]) needs_ra[j] = 1;
		}
	}

	nat rig[4096] = {0};
	nat rig_count = 0;

	puts("RA: constructing register interference graph...");

	for (nat pc = 0 ; pc < ins_count; pc++) {
		for (nat i = 0; i < var_count; i++) {
			for (nat j = 0; j < i; j++) {
				if (not alive[pc * var_count + i]) continue;
				if (not alive[pc * var_count + j]) continue;
				rig[2 * rig_count + 0] = i;
				rig[2 * rig_count + 1] = j;
				rig_count++;
			}
		}
	}

	puts("registers which need RA performed on them (or already have a register index!)");
	for (nat i = 0; i < var_count; i++) {
		if (not needs_ra[i]) continue;
		printf("  . register %s (%llu)\n", variables[i], i);
	}
	puts("\n");

	printf("constructed the following RIG: (%llu interferences)\n", rig_count);
	for (nat i = 0; i < rig_count; i++) {
		const nat first = rig[2 * i + 0];
		const nat second = rig[2 * i + 1];
		printf("    . register %s (%llu) interferes with %s (%llu).\n", 
			variables[first], first, 
			variables[second], second
		);
	}

	puts("\n\nadditionally, we know that: \n");
	for (nat i = 0; i < var_count; i++) {
		if (register_index[i] != (nat) -1) {
			printf("   .  %s (%llu) must be stored in hardware register %llu.\n", 
				variables[i], i, register_index[i]
			);
		}
	}
	
	nat node_selected[max_variable_count] = {0};
	stack_count = 0;

	while (1) {		
		for (nat i = 0; i < var_count; i++) {
			if (not needs_ra[i]) continue; 
			if (not node_selected[i]) goto find_virtual_register; 
		}
		puts("RA: pushed all nodes!");
		break;

	find_virtual_register:
		for (nat var = 0; var < var_count; var++) {
			if (not needs_ra[var] or node_selected[var]) continue;

			nat neighbor_count = 0;
			for (nat e = 0; e < rig_count; e++) {
				const nat a = rig[2 * e + 0];
				const nat b = rig[2 * e + 1];
				if (node_selected[a] or node_selected[b]) continue;
				if (a == var or b == var) neighbor_count++;
			}
			if (neighbor_count < hardware_register_count) {
				stack[stack_count++] = var;
				node_selected[var] = 1;
				goto next_iteration;
			}

		}

		printf("compiler: register allocation: error: failed to allocate "
			"variables to %llu hardware registers.\n", hardware_register_count
		);
		abort();

		next_iteration:;
	}

	puts("created the following ordering on the virtual registers: ");
	for (nat i = 0; i < stack_count; i++) {
		printf("%5llu: %s (%llu) \n", i, variables[stack[i]], stack[i]);
	}
	puts("[ordering done]");

	nat* occupied = calloc(hardware_register_count, sizeof(nat));
	memset(allocation, 255, sizeof(nat) * var_count);
	for (nat i = 0; i < var_count; i++) {
		if (register_index[i] == (nat) -1) continue;

		if (target_arch == rv32_arch) {
			allocation[i] = register_index[i] - 1;
			// this line: this translation is specific to risc-v. 
			// (zero register is unused, and non-allocatable.)
		} else if (target_arch == msp430_arch) {
			allocation[i] = register_index[i] - 4;
	/*
msp430 registers are the following:

set pc 0	: unallocatable: special purpose: program counter
set sp 1	: unallocatable: special purpose: stack pointer  (ie, edited via the push and pop instructions..)
set sr 01	: unallocatable: special purpose: status register
set cg 11	: unallocatable: special purpose: constant generater 2

set r4 001	: general purpose, allocatable
set r5 101	: general purpose, allocatable
set r6 011	: general purpose, allocatable
set r7 111	: general purpose, allocatable
set r8 0001	: general purpose, allocatable
set r9 1001	: general purpose, allocatable
set r10 0101	: general purpose, allocatable
set r11 1101	: general purpose, allocatable
set r12 0011	: general purpose, allocatable
set r13 1011	: general purpose, allocatable
set r14 0111	: general purpose, allocatable
set r15 1111	: general purpose, allocatable
	*/
		} else abort();
	}

	printf("occupied: "); print_nats(occupied, hardware_register_count); puts("");
	
	for (nat s = stack_count; s--;) {
		const nat var = stack[s];
		node_selected[var] = 0;
		printf("[s=%llu]: trying to allocate %s\n", s, variables[var]);
		if (allocation[var] != (nat) -1) continue;
		memset(occupied, 0, sizeof(nat) * hardware_register_count);
		for (nat e = 0; e < rig_count; e++) {
			const nat a = rig[2 * e + 0];
			const nat b = rig[2 * e + 1];
			if (node_selected[a] or node_selected[b]) continue;
			     if (a == var) occupied[allocation[b]] = 1;
			else if (b == var) occupied[allocation[a]] = 1;
		}
		
		printf("current occupied: "); print_nats(occupied, hardware_register_count); puts("");
		for (nat i = 0; i < hardware_register_count; i++) {
			if (not occupied[i]) {
				allocation[var] = i;
				goto allocation_found;
			}
		}
		puts("internal error in RA: could not find a HW reg.");
		abort();
		allocation_found:;
	}

	for (nat e = 0; e < rig_count; e++) {
		const nat a = rig[2 * e + 0];
		const nat b = rig[2 * e + 1];
		if (allocation[a] == allocation[b]) {
			printf("compiler: register allocation: error: "
				"unresolved register interference between variables %s and %s\n",
				variables[a], variables[b]
			);
			abort();
		}
	} 

	for (nat i = 0; i < var_count; i++) {
		if (not needs_ra[i] and allocation[i] == (nat) -1) continue;
		if (target_arch == rv32_arch) {
			allocation[i] += 1;
			// this line is specific to risc-v! 
			// we'd translate the RI's back to hardware RI space. 

		} else if (target_arch == msp430_arch) {
			allocation[i] += 4; // translate language RI space back into msp430 register RI space. 
		} else abort();

	}

	puts("RA: FINAL REGISTER ALLOCATION:");
	for (nat i = 0; i < var_count; i++) {
		if (not needs_ra[i] and allocation[i] == (nat) -1) continue;
		printf("    . %s (%llu) is stored in hardware register x[%lld]\n", variables[i], i, allocation[i]);
	}
	puts("\n[done with graph coloring in RA]");
	}
		
	for (nat i = 0; i < ins_count; i++) {
		ins[i].state = 0;
	}

	puts("filling in RA assignments into the machine code...");
	for (nat i = 0; i < ins_count; i++) {
		print_instruction_window_around(i, 0, "");
		puts("[RA: filling in allocation scheme, [dead store elmination]]");
		//getchar();

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
	
		for (nat a = 0; a < arity[op]; a++) {
			if (op == at and a == 0) continue;
			const nat this_arg = ins[i].args[a];

			if (imm & (1 << a)) {
				printf("on argument: [a = %llu]: is_immediate!  (immediate value is %llu)\n", a, this_arg);
			} else if (not is_label[this_arg]) {				
				printf("on argument: [a = %llu]: NOT is_immediate and NOT label!  (variable = %s) \n", 
					a, variables[this_arg]
				);

				puts("filling in the register index we found for this operation!");
				if (allocation[this_arg] == (nat) -1) {
					printf("FATAL ERROR: no hardware register index was "
						"not found for variable %s in the below instruction. "
						"aborting...\n", variables[this_arg]
					);
					print_instruction(ins[i]); puts(""); 
					puts("skipping this instruction, instead of aborting...");
					getchar();
					ins[i].state = 1;
					//abort();
				}

				ins[i].args[a] = allocation[this_arg];
				ins[i].imm |= 1LLU << a;

				printf("info: filled in register index %llu for variable %s into this instruction. ", 
					ins[i].args[a], variables[this_arg]
				);				
			}
		}
	}

	{ nat final_ins_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].state) continue;
		ins[final_ins_count++] = ins[i];
	}
	ins_count = final_ins_count; } 

	print_instructions(0);
	puts("RA DEAD-STORE PRUNING finished.");
	//getchar();
	puts("[done with RA");

	printf("info: finished final machine code for target = %llu\n", target_arch);
	print_instructions(1);
	//getchar();

	puts("generating final machine code binary...");

	uint8_t* my_bytes = NULL;
	nat my_count = 0;

#define max_section_count 128
	nat section_count = 0;
	nat section_starts[max_section_count] = {0};
	nat section_addresses[max_section_count] = {0};

	if (target_arch == rv32_arch) goto rv32_generate_machine_code;
	if (target_arch == rv64_arch) goto rv32_generate_machine_code;
	if (target_arch == arm64_arch) goto arm64_generate_machine_code;
	if (target_arch == msp430_arch) goto msp430_generate_machine_code;
	puts("unknown target"); abort();

rv32_generate_machine_code:;

	{ nat* lengths = calloc(ins_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == halt or op == at) continue;
		nat k = 4;
		if (op == emit) k = ins[i].args[0];
		lengths[i] = k;
	}

	print_nats(lengths, ins_count); puts("");


/*
	r_type: 	
		a0: opcode(0-6)  
		a1: rd(7-11)   
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: rs2(20-24)
		a5: funct7(25-31)

	i_type: 
		a0: opcode(0-6)
		a1: rd(7-11)
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: imm_11_0(20-31)

	s_type: 	
		a0: opcode(0-6)  
		a1: imm_4_0(7-11)
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: rs2(20-24)
		a5: imm_5_11(25-31)

	b_type: 
		a0: opcode(0-6)
		a1: imm_4_1_11(7-11)
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: rs2(20-24)
		a5: imm_12_10_5(25-31)

	u_type: 	
		a0: opcode(0-6)
		a1: rd(7-11)
		a2: imm_31_12(12-31)

	j_type: 	
		a0: opcode(0-6)
		a1: rd(7-11)
		a2: imm_20_10_1_11_19_12(12-31)		
*/

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("");
		dump_hex(my_bytes, my_count);
		//getchar();

		const nat op = ins[i].op;
		const u32 a0 = (u32) ins[i].args[0];
		const u32 a1 = (u32) ins[i].args[1]; 
		const u32 a2 = (u32) ins[i].args[2]; 
		const u32 a3 = (u32) ins[i].args[3]; 
		const u32 a4 = (u32) ins[i].args[4]; 
		const u32 a5 = (u32) ins[i].args[5]; 

		if (op == at or op == halt) { 	
			// do nothing


		} else if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (uint64_t) ins[i].args[1]);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (uint8_t) a1);


		} else if (op == r5_i) {

			u32 k = a4; 
			if (a5 == 0x42) {
				printf("info: found a compiler generated r5_i!!\n");
				const nat n = compute_label_location(a4);
				const u32 im = calculate_offset(lengths, i - 1, n) & 0x00000FFF; // new value of k
				k = im;
			} else k = a4;

			const u32 word = 
				(k  << 20U) | 
				(a3 << 15U) | 
				(a2 << 12U) | 
				(a1 <<  7U) | 
				(a0 <<  0U) ;

			if (not k and a5 == 0x42) {}
			else insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_r) {

			const u32 word = 
				(a5 << 25U) | 
				(a4 << 20U) | 
				(a3 << 15U) | 
				(a2 << 12U) | 
				(a1 <<  7U) | 
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_s) {

			const u32 word = 
				(((a1 >> 5) & 0x3f) << 25U) | 
				(a4 << 20U) | 
				(a3 << 15U) | 
				(a2 << 12U) | 
				((a1 & 0x1f) <<  7U) | 
				(a0 << 0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_u) {

			const nat n = compute_label_location(a2);
			const u32 im = calculate_offset(lengths, i, n) & 0xFFFFF000;
			const u32 word =
				(im <<  0U) |
				(a1 <<  7U) |
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, word);


		} else if (op == r5_b) {

			const nat n = compute_label_location(a4);
			const u32 im = (u32) calculate_offset(lengths, i, n) & 0x1FFF;

			printf("decimal: im = %d\n", im | ((im & (1 << 12)) ? 0xFFFFE000 : 0));
			printf("decimal: "); print_binary(im | ((im & (1 << 12)) ? 0xFFFFE000 : 0)); puts("");

			const u32 bit4_1  = (im >> 1) & 0xF;
			const u32 bit10_5 = (im >> 5) & 0x3F;
			const u32 bit11   = (im >> 11) & 0x1;
			const u32 bit12   = (im >> 12) & 0x1;
			const u32 lo = (bit4_1 << 1) | bit11;
			const u32 hi = (bit12 << 6) | bit10_5;
	
			printf("b_type:  im = 0x%08x, lo = 0x%08x, hi = 0x%08x\n", im, lo, hi);

			printf("im = "); print_binary(im); puts("");
			printf("lo = "); print_binary(lo); puts("");
			printf("hi = "); print_binary(hi); puts("");
			//getchar();

			const u32 word =
				(hi << 25U) |
				(a3 << 20U) |
				(a2 << 15U) |
				(a1 << 12U) |
				(lo <<  7U) |
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else {
			printf("could not generate machine code for instruction: %llu\n", op);
			abort();
		}
	}} 
	goto finished_generation;

msp430_generate_machine_code:;

	{nat* lengths = calloc(ins_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		const u32 a0 = (u32) ins[i].args[0];
		const u32 a1 = (u32) ins[i].args[1];
		const u32 a4 = (u32) ins[i].args[4];
		const u32 a5 = (u32) ins[i].args[5];

		nat len = 0;
		if (op == m4_sect) len = 0;
		else if (op == halt) len = 0;
		else if (op == emit) len = a0;
		else if (op == m4_br) len = 2;
		else if (op == m4_op) {
			len = 2;
			if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
				or (a4 == 3 and not a5)) len += 2;						
			if (a1 == 1) len += 2;
		}
		lengths[i] = len;
	}

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("");
		dump_hex(my_bytes, my_count);
		//getchar();

		const nat op = ins[i].op;
		const u16 a0 = (u16) ins[i].args[0];
		const u16 a1 = (u16) ins[i].args[1];
		const u16 a2 = (u16) ins[i].args[2];
		const u16 a3 = (u16) ins[i].args[3];
		const u16 a4 = (u16) ins[i].args[4];
		const u16 a5 = (u16) ins[i].args[5];
		const u16 a6 = (u16) ins[i].args[6];
		const u16 a7 = (u16) ins[i].args[7];


		if (op == at or op == halt) { 	
			// do nothing


		} else if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (uint64_t) ins[i].args[1]);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (uint8_t) a1);


		} else if (op == m4_sect) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;


		} else if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (uint64_t) ins[i].args[1]);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (uint8_t) a1);
		}

		else if (op == m4_br) { // br4 cond:[3 bits] label:[pc-rel offset]
			const nat n = compute_label_location(a1);
			const u16 offset = 0x3FF & ((calculate_offset(lengths, i + 1, n) >> 1));
			const u16 word = (u16) ((1U << 13U) | (u16)(a0 << 10U) | (offset));
			insert_u16(&my_bytes, &my_count, word);
		}
		else if (op == m4_op) {  
			// gen4  op(0)  dm(1) dr(2) di(3)  sm(4) sr(5) si(6)   size(7)

			// 	op : 4 bits
			// 	dm : 1 bit
			// 	sm : 2 bits
			// 	dr,sr : 4 bits
			// 	di,si : 16 bits, only required with particular modes
			// 	size : 1 bit

			u16 word = (u16) (
				(a0 << 12U) | (a5 << 8U) | (a1 << 7U) | 
				(a7 << 6U) | (a4 << 4U) | (a2)
			);
			insert_u16(&my_bytes, &my_count, word);

			if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
				or (a4 == 3 and not a5)) insert_u16(&my_bytes, &my_count, a6);
						
			if (a1 == 1) insert_u16(&my_bytes, &my_count, a3);
		}				
		else if (op == halt) {}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}}

	goto finished_generation;


arm64_generate_machine_code:;

	nat* lengths = calloc(ins_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == halt) continue;
		lengths[i] = ins[i].op == emit ? ins[i].args[0] : 4;
	}

	print_nats(lengths, ins_count); puts("");

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("");
		dump_hex(my_bytes, my_count);
		//getchar();

		const nat op = ins[i].op;
		const u32 a0 = (u32) ins[i].args[0];
		const u32 a1 = (u32) ins[i].args[1];
		const u32 a2 = (u32) ins[i].args[2];
		const u32 a3 = (u32) ins[i].args[3];
		const u32 a4 = (u32) ins[i].args[4];
		const u32 a5 = (u32) ins[i].args[5];
		const u32 a6 = (u32) ins[i].args[6];
		const u32 a7 = (u32) ins[i].args[7];


		if (op == at) {}
		else if (op == halt) {}

		else if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (uint64_t) ins[i].args[1]);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (uint8_t) a1);
		}

		else if (op == a6_clz) { puts("clz is unimplemented currently, lol"); abort(); }
		else if (op == a6_rev) { puts("rev is unimplemented currently, lol"); abort(); }
		else if (op == a6_extr) { puts("extr is unimplemented currently, lol"); abort(); }
		else if (op == a6_ldrl) { puts("ldrl is unimplemented currently, lol"); abort(); }

		else if (op == a6_nop) insert_u32(&my_bytes, &my_count, 0xD503201F);
		else if (op == a6_svc) insert_u32(&my_bytes, &my_count, 0xD4000001);

		else if (op == a6_br) {
			uint32_t l = a2?2:a1?1:0;
			const uint32_t to_emit = 
				(0x6BU << 25U) | (l << 21U) | 
				(0x1FU << 16U) | (a0 << 5U);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_adc) {
			const uint32_t to_emit = 
				(a5 << 31U) | (a4 << 30U) | (a3 << 29U) | 
				(0xD0 << 21U) | (a2 << 16U) | (0 << 19U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_shv) {
			uint32_t op2 = 8;
			if (a3 == 0) op2 = 8;
			if (a3 == 1) op2 = 9;
			if (a3 == 2) op2 = 10;
			if (a3 == 3) op2 = 11;
			const uint32_t to_emit = 
				(a4 << 31U) | (0 << 30U) | 
				(0 << 29U) | (0xD6 << 21U) | 
				(a2 << 16U) | (op2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_mov) {
			const uint32_t to_emit = 
				(a4 << 31U) | (a3 << 29U) | (0x25U << 23U) | 
				(a2 << 21U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_bc) {
			const uint32_t offset = 0x7ffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = 
				(0x54U << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_jmp) {
			const uint32_t offset = 0x3ffffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = (a0 << 31U) | (0x5U << 26U) | (offset);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_adr) {
			uint32_t o1 = a2;
			nat count = calculate_offset(lengths, i, a1);
			if (a2) count /= 4096;
			const uint32_t offset = 0x1fffff & count;
			const uint32_t lo = offset & 3, hi = offset >> 2;
			const uint32_t to_emit = 
				(o1 << 31U) | (lo << 29U) | (0x10U << 24U) |
				(hi << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_cbz) {
			const uint32_t offset = 0x7ffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = 
				(a3 << 31U) | (0x1AU << 25U) | 
				(a2 << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_tbz) {
			const uint32_t b40 = a1 & 0x1F;
			const uint32_t b5 = a1 >> 5;
			const uint32_t offset = 0x3fff & (calculate_offset(lengths, i, a2) >> 2);
			const uint32_t to_emit = 
				(b5 << 31U) | (0x1BU << 25U) | (a3 << 24U) |
				(b40 << 19U) |(offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_ccmp) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a4 << 30U) | (0x1D2 << 21U) | 
				(a3 << 16U) | (a0 << 12U) | (a2 << 11U) | 
				(a1 << 5U) | (a5); 
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_addi) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a5 << 30U) | (a4 << 29U) | 
				(0x22 << 23U) | (a3 << 22U) | (a2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_addr) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0xB << 24U) | (a3 << 22U) | (a2 << 16U) |
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_addx) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0x59 << 21U) | (a2 << 16U) | (a3 << 13U) | 
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_divr) {
			const uint32_t to_emit = 
				(a4 << 31U) | (0xD6 << 21U) | (a2 << 16U) |
				(1 << 11U) | (a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_csel) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a5 << 30U) | (0xD4 << 21U) | 
				(a2 << 16U) | (a3 << 12U) | (a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_madd) {
			const uint32_t to_emit = 
				(a7 << 31U) | (0x1B << 24U) | (a5 << 23U) | 
				(a4 << 21U) | (a2 << 16U) | (a6 << 15U) | 
				(a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_bfm) {
			u32 imms = 0, immr = 0;
			if (not a2) { imms = a3 + a4 - 1; immr = a3; } 
			else { imms = a4 - 1; immr = (a6 ? 64 : 32) - a3; }
			const uint32_t to_emit = (a6 << 31U) | (a5 << 29U) | 	
				(0x26U << 23U) | (a6 << 22U) | (immr << 16U) |
				(imms << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_ori) {


			puts("TODO: please implemented the ori instruction: "
				"this is the last instruction we need to implement "
				"and then we are done with iplemementing the arm64 backend!"
			);

			abort();


		} else if (op == a6_orr) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a0 << 29U) | (10 << 24U) | 
				(a4 << 22U) | (a6 << 21U) | (a3 << 16U) | 
				(a5 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_memp) {
			const uint32_t to_emit = 
				(a1 << 30U) | (0x14 << 25U) | (a6 << 23U) | (a0 << 22U) | 
				(a5 << 15U) | (a3 << 10U) | (a4 << 5U) | (a2);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_memi) {
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a4 << 30U) | (0x39 << 24U) | (opc << 22U) |
				(a3 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == a6_memia) { 			
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a4 << 30U) | (0x38 << 24U) | (opc << 22U) | (a3 << 12U) | 
				(a5 << 11U) | (1 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);


		} else if (op == a6_memr) { 
			const u32 S = (a4 >> 2) & 1, option = a4 & 3;
			u32 opt = 0;
			if (option == 0) opt = 2;
			else if (option == 1) opt = 3;
			else if (option == 2) opt = 6;
			else if (option == 3) opt = 7;
			else abort();
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a5 == 3) opc = 1;
			else if (a5 == 2 and is_signed) opc = 2;
			else if (a5 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a5 << 30U) | (0x38 << 24U) | (opc << 22U) |
				(1 << 21U) | (a3 << 16U) | (opt << 13U) |
				(S << 12U) | (2 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}


finished_generation:;
	puts("done: byte generation successful.");
	puts("final_bytes:");
	dump_hex(my_bytes, my_count);
	printf("info: generating output file with format #%llu...\n", output_format);

	if (output_format == debug_output_only) goto print_debug_output_only;
	if (output_format == hex_array_output) goto generate_hex_array_output;
	if (output_format == ti_txt_executable) goto generate_ti_txt_executable;
	if (output_format == uf2_executable) goto generate_uf2_executable;
	if (output_format == macho_executable) goto generate_macho_executable;
	if (output_format == macho_object) goto generate_macho_object;
	if (output_format == elf_executable) abort();
	if (output_format == elf_object) abort();
	puts("unknown target"); abort();

print_debug_output_only:;

	printf("debug: executable bytes: (%llu bytes)\n", my_count);
	for (nat i = 0; i < my_count; i++) {
		if (i % 32 == 0) puts("");
		if (my_bytes[i]) printf("\033[32;1m");
		printf("%02hhx ", my_bytes[i]);
		if (my_bytes[i]) printf("\033[0m");
	}
	puts("");
	goto finished_outputting;
	
generate_hex_array_output:;

	{ nat len = 0;
	char out[14000] = {0};

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

	{ int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("open: could not create executable file"); exit(1); }
	write(file, out, len);
	close(file); }

	printf("hex-txt: wrote %llu bytes to file %s.\n", len, output_filename);

	{ char debug_string[4096] = {0};
	snprintf(debug_string, sizeof debug_string, 
		"./useful/riscv_disassembler/run print %s", 
		output_filename
	);
	system(debug_string); } } 

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

struct nlist_64 {
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


generate_ti_txt_executable:;

	{ char out[14000] = {0};
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

	char debug_string[4096] = {0};
	snprintf(debug_string, sizeof debug_string, "../../led_display/embedded_assembler/msp430_disassembler/run %s", output_filename);
	system(debug_string); }

	goto finished_outputting;

generate_uf2_executable:;

	{ const nat starting_address = 0x00000000;    // user sets this using the "section_address k" instruction. 

	while (my_count % 256) 
		insert_byte(&my_bytes, &my_count, 0); 		// pad to 256 byte chunks

	const nat block_count = my_count / 256;

	uint8_t* data = NULL;
	nat count = 0;	
	nat current_offset = 0;
	for (nat block = 0; block < block_count; block++) {
		insert_u32(&data, &count, 0x0A324655);
		insert_u32(&data, &count, 0x9E5D5157);
		insert_u32(&data, &count, 0);
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

	dump_hex(data, count);	
	puts("generating uf2 output file...");

	// next step:   write these blocks to   a new file  with filename       "output_from_compiler.uf2"

	// and also generate a "hex array" version of the output of these bytes, so that we can see the dissassembly,
	//  or just make the dissassembler able to read in  a UF2 file directly, and dissassemble it. 

	// 		(probably this option, actually. lol. shouldnt be that hard.?)
	
	}


finished_outputting: 
	exit(0);


} // main 


























































// 			gen4 op(0)   dm(1) dr(2) di(3)    sm(4) sr(5) si(6)   size(7)
//     			mov   [reg_mode: D]   [any: ...A...]


/*	r_type: 	
		a0: opcode(0-6) 
		a1: rd(7-11)   
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: rs2(20-24)
		a5: funct7(25-31)

	i_type: 
		a0: opcode(0-6)
		a1: rd(7-11)
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: imm_11_0(20-31)

	s_type: 	
		a0: opcode(0-6)  
		a1: imm_4_0(7-11)
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: rs2(20-24)
		a5: imm_5_11(25-31)

	b_type: 
		a0: opcode(0-6)
		a1: imm_4_1_11(7-11)
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: rs2(20-24)
		a5: imm_12_10_5(25-31)

	u_type: 	
		a0: opcode(0-6)
		a1: rd(7-11)
		a2: imm_31_12(12-31)

	j_type: 	
		a0: opcode(0-6)
		a1: rd(7-11)
		a2: imm_20_10_1_11_19_12(12-31)
		
*/
































































































































































































































































































































	// the alive[] values      represent   the aliveness                    BEFORE    the instruction executes.  
	// this is because we are going backwards. 
	// after the cf merge (going backwards) the alive[] val represents after instruction, 
	// but then after we update the information, we actually make the alive values represent BEFORE the 
	// instruction executes. which is exactly what we need to know when looking at our successors's alive val states. 
	// target dependent, we'll configure this automatically per target!!
	// once this pass completes, these values represent b:BEFORE; the instruction executes. 










//printf( "note: calling operation: "
//	"%s (arity %llu)\n", 
//	operations[op], arity[op]
//);







/*

copy propagation has a bug:

  •   7 │ 	   set    rv_sc_arg0               0x1                      
  •   8 │ 	    la    rv_sc_arg1               string                   
  •   9 │ 	   set    rv_sc_arg2               0xa                      
  •  10 │ 	   set    rv_sc_number             0x3                      
  •  11 │ 	  system    
  •  12 │ 	   set    input_length             rv_sc_arg0               <---- def
  •  13 │ 	   set    rv_sc_arg0               0x1                      <---- modification to copy
  •  14 │ 	    la    rv_sc_arg1               inputarea                
  •  15 │ 	   set    rv_sc_arg2               input_length             <---- propagation fail!
  •  16 │ 	   set    rv_sc_number             0x3                      
  •  17 │ 	  system    
  •  18 │ 	   set    rv_sc_arg0               0x1                      
  •  19 │ 	    la    rv_sc_arg1               newline                  
  •  20 │ 	   set    rv_sc_arg2               0x1                      
  •  21 │ 	   set    rv_sc_number             0x3                      
  •  22 │ 	  system    






-----------PRUNING CTK INS:---------------

found real RT isntruction!
	   set    rv_sc_arg2               input_length             
note: inlining copy reference: a1=42 imm=0 copy_of=0, i=15...
original:
   set    rv_sc_arg2               input_length             
modified form:
   set    rv_sc_arg2               rv_sc_arg0               
was this right to do?...






the fix to the bug is the following:


	when we see a given variable "X"    is modified   
		as a result of an insrtuction we are on (ie, changed, in any way!)




		then we need to loop over all variables,

			and then look for which are currently known to be a copy of another variable. 

			if, for a given is_copy[] variable, "G"

				if we see that G's   copy_of[] variable index,      IS the edited/modfied variable X   

						then we need to   set   is_copy[]   for that variable G   to 0!

		ie, we will have set is_copy[] = 0   for several variables, maybe   becuase we saw that that variable's value no longer reflects the up to date version of that copy_of[]'s variable's value!!!



	this is a case    within copy prop   that we arent currently handling, and we need to handle it. 


		its pretty simple,  the critcal thing is that we need to do this if we see that the variable X   

			"changed"  in some way 


			the ways it could change   is being the destination of   add, sub, mul, div, rem, si, sd, and, or, eor, la, ld,   

				and also               set  


			those instructions are capable of changing their destinations. so yeah! nice. 



we need to implement this. 

written   as of 1202505235.203820


*/













/*	1202505294.204718 todo: we need to actually make the ct system work like this:

		instead of attributing whether a variable is compiletime or not, 

		we instead attribute whether an instruction is compiletime or not, 

		and if a variable is defined in a rt or ct context, and this bit setting is defined using the 

		"rt" and "ct" instructions.   by default, the context is set to "rt". 

		saying "rt" alone (no args) changes this bit to runtime code,

		saying "ct" alone (no args) changes this bit to compile time code.

		this allows for code to be more seemlessly transfered between runtime and compiletime,
		and also for the compiletime system call    "compiler" interface to simply be "system" itself,
			but where you execute this instruction within a runtime context lol.

		for example, you might see something like:

					ct system ctsc_set_target rv32_arch
					rt (some other instructions here)



		the problem with this of course, is that     "system" takes zero arguments......


		so i think we do need to have some builtin CT registers, or like, something..

			ORRRRR   we could just use the   register   instruction to actually make certain variables forced to be in particular COMPILETIME "compiler-hardware" registers!!!


				thatttt could work. nice. wow lol. thats awesome!!

instead of the large existing isa:


	system_, compiler, emit, string, file, del, 
	constant, register_, bitcount,
	set, add, sub, mul, div_, rem, 
	and_, or_, eor, si, sd, la, ld, st, 
	lt, ge, ne, eq, do_, at, halt, 


we could have:     (30 instructions total!)

	ct, rt, system, emit, string, 
	file, del, register, bits,
	set, add, sub, mul, div, rem, 
	and, or, eor, si, sd, la, 
	ld, st, lt, ge, ne, eq, do, at, halt, 


this simplifies a tonnn of the language's ct/rt semantics. 
		



*/










/*

1202505224.160953
	adding more instructions to the msp430 backend:



1202505224.204759
	actually i ended up doing regsiter allocation instead LOLL
	

		





struct instruction new = { .op = la, .imm = 0x5 }; // not r5_u, like you would think!!! we expose the auipc and addi at the end, after we determined the pc-rel offset. THIS IS REQUIRED!!!!!!
			new.args[0] = 0x17;
			new.args[1] = arg0;
			new.args[2] = arg1; // we need to determine how we are going to refer to a label here..... probably via name. which means we need to generate "at" statements too, in ins sel. 
			mi[mi_count++] = new;
			ins[i].state = 1;
			continue;


			alternatively, we can just generate the   AUIPC / ADDI  pair   and then just know that they always come in pairs, always lol. this is a bit risky though. to help distinguish this, i think we should like, set an additioal bit outside the argument list, just so we know that the compiler generated these ourselves. and they werent from the user. lets do that. 

			
			*/


/* notes i realized whlie reading the risc-v spec:


	we will use ins sel patterns   of   la's:


			when the programmer says 

				la x label 
				ld data x size_u32


			that will actually translate into a pattern NOTTT involving an ADDI. 


			instead, it will translate to:       


				auipc x label[31:12]
				ldw data (x + label[11:0])          (the risc-v load includes an addi.)


			thus, only an auipc corresponds to an la in source. 

				it depends whats afterrrr the la.   thats why we need multiple 
								ins-sel patterns detecting la in various patterns.


	//halt, sc, sl, ud, def, do_, at, lf,
	//set, add, sub, mul, div_, rem, 
	//and_, or_, eor, si, sd, la, rt, emit, 
	//ld, st, lt, ge, ne, eq, 
		
*/



		// todo:   we need to be doing this for the risc-v machine instructions,
		//          ...not the language isa instructions. 

		//						crap lol 

		/*
		else if (op == set) {
			alive[pc * var_count + a0] = 0;
			if (not i1) alive[pc * var_count + a1] = 1;

		} else if (op >= add and op <= sd) {
			if (not i1) alive[pc * var_count + a1] = 1;
			
		} else if (op == st) {
			if (not i0) alive[pc * var_count + a0] = 1;
			if (not i1) alive[pc * var_count + a1] = 1;

		} else if (op == ld) {
			alive[pc * var_count + a0] = 0;
			if (not i1) alive[pc * var_count + a1] = 1;

		} else if (op == la) {
			alive[pc * var_count + a0] = 0;
			if (not i1) alive[pc * var_count + a1] = 1;			

		} else if (op == lt or op == ge or op == ne or op == eq) {

			if (not i0) alive[pc * var_count + a0] = 1;
			if (not i1) alive[pc * var_count + a1] = 1;			

		} */









	/*


	// 	our previous RA implementation, which just uses the user's register attributes:


	for (nat i = 0; i < mi_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("[doing something like RA]");
		//getchar();

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
	
		for (nat a = 0; a < arity[op]; a++) {

			if (op == at and a == 0) continue;
			if(op == do_ and a == 0) continue;
			if (op == lt and a == 2) continue;
			if (op == ge and a == 2) continue;
			if (op == ne and a == 2) continue;
			if (op == eq and a == 2) continue;
			if (op == la and a == 1) continue;

			const nat this_arg = ins[i].args[a];

			if (imm & (1 << a)) {
				printf("on argument: [a = %llu]: is_immediate!  (immediate value is %llu)\n", a, this_arg);
			} else if (not is_label[this_arg]) {
				
				printf("on argument: [a = %llu]: NOT is_immediate and NOT label!  (variable = %s) \n", 
					a, variables[this_arg]
				);

				puts("filling in the register index we found for this operation!");

				if (register_index[this_arg] == (nat) -1) {
					printf("FATAL ERROR: no hardware register index was "
						"not found for variable %s in the below instruction. "
						"aborting...\n", variables[this_arg]
					);
					print_instruction(ins[i]); puts(""); 
					abort();
				}
				ins[i].args[a] = register_index[this_arg];
				ins[i].imm |= 1LLU << a;

				printf("info: filled in register_index %llu for variable %s into this instruction. ", 
					ins[i].args[a], variables[this_arg]
				);				
			}
		}
		puts("hello");
	}

	puts("[done with RA");

	printf("info: final machine code: for target = %llu\n", target_arch);
	print_instructions(1);

	*/


/*
riscv branches: (spelt using little endian binary):
	beq 000
	bne 100
	blt 001
	bge 101
	bltu 011
	bgeu 111
*/



/*static void push_mi(
	struct instruction** mi, 
	nat* mi_count, 

	nat op, nat imm, 
	
	nat arg0, nat arg1, nat arg2, 
	nat arg3, nat arg4, nat arg5,
	nat arg6, nat arg7
) {
	struct instruction new = { 
		.op = op, 
		.imm = imm 
	};

	new.args[0] = arg0;
	new.args[1] = arg1;
	new.args[2] = arg2;
	new.args[3] = arg3;
	new.args[4] = arg4;
	new.args[5] = arg5;
	new.args[6] = arg6;
	new.args[7] = arg7;

	(*mi)[(*mi_count)++] = new;
}*/













































































































































/*

	// TODO: copy_prop:    we need to be tracing not only rt variables through copies,  
	//                     but also runtime immediate set statements. ie,    set x 5 set y x  (y is 5!)

	const nat not_a_copy = (nat) -1;
	memset(values, 255, sizeof values);

	for (nat pc = 0; pc < ins_count; pc++) {
		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];
		const nat i1 = !!(imm & 2);

		print_instruction_window_around(pc, 0, "");
		print_dictionary(0);
		puts("----------- COPY PROP --------------");
		//getchar();
		
		if (op == halt or op == at or op == do_ or op == lt or op == ge or op == ne or op == eq) {
			memset(values, 255, sizeof values);

		} else if (op == set) {
			if (i1) continue;
			if (values[arg1] != not_a_copy) ins[pc].args[1] = values[arg1];
			values[arg0] = ins[pc].args[1];

		} else if (op >= add and op <= sd) {
			if (i1) continue;
			if (values[arg1] != not_a_copy) ins[pc].args[1] = values[arg1];
			values[arg0] = not_a_copy;

		} else { puts("CP: fatal internal error: unknown instruction executed...\n"); abort(); } 
	}




*/



	//// HERE:       CURRENT STATE:              1202505121.211911    we need to do this:


	/*

			we need to bring in the previous way we were doing  CTE   and CP    AT THIS POINT! 


			the goal here, is to KEEPPP the existing amazing CTE stage listed above, 

			ANDDD to put in  an  RT-var  specific     CTE stage,  which doesnt do execution  AT ALL!!!


			ie, we seperate out the actual RUNNING of code, vs just propgation of constants!


				AMAZING!!      see, the cool part is that   running of the program (ie, the values changing over the course of the program, 

								is not applicable to   CP!!      ANDDD not applicable  to CTK/RTK analysis   ie constant prop/folding

									becuase of the fact that we are trying to find a stable representation, which doesnt evolve over time! 


									and so the problem becomes MUCHHH more easier, becuase now we won't run into the same problem that we were having before!   AMAZINGGG


utterly geniusss

YAYYYY




	*/













































































































































































































































































































































































































/*

		//else if (op == ld)   values[arg0] = memory[val1];
		//else if (op == st)   memory[val0] = val1;

if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (uint64_t) a1);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (uint8_t) a1);
		}



static void debug_cte(
	nat pc, nat last_pc,
	nat* preds, nat pred_count,
	nat* stack, nat stack_count,
	nat* def,
	nat* value, nat* type, 
	nat* copy_of, nat* copy_type,
	nat* is_copy
) {

	print_instruction_window_around(pc, 0, "PC");
	print_dictionary(0);

	printf("        ");
	for (nat j = 0; j < var_count; j++) 
		printf("%3lld ", j);
	puts("\n-------------------------------------------------");
	for (nat i = 0; i < ins_count; i++) {
		printf("ct %3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (type[i * var_count + j])
				printf("%3lld ", 
					value[i * var_count + j]);
			else 	printf("\033[90m%3lld\033[0m ", 
					value[i * var_count + j]);
		}
		putchar(9);

		printf("cp %3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_copy[i * var_count + j])
				printf("%3lld(%llu) ", 
					copy_of[i * var_count + j], 
					copy_type[i * var_count + j]
				);
			else 	printf("\033[90m%3lld(%llu)\033[0m ", 
					copy_of[i * var_count + j], 
					copy_type[i * var_count + j]
				);
		}
		putchar(9);

		printf("def %3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if ((int64_t) def[i * var_count + j] >= 0)
				printf("%3lld ", 
					def[i * var_count + j]
				);
			else 	printf("\033[90m%3lld\033[0m ", 
					def[i * var_count + j]
				);
		}
		puts("");
	}
	puts("-------------------------------------------------");
	printf("[PC = %llu, last_PC = %llu], pred:", pc, last_pc);
	print_nats(preds, pred_count); putchar(32);
	printf("stack: "); 
	print_nats(stack, stack_count); 
	putchar(10);
}
*/















































































































































































































































































/*

	const nat target_arch = rv64_arch;
	const nat output_format = debug_output_only;
	const nat should_overwrite = true;

	printf("info: assemblying for [target = %llu, output = %llu (%s)]\n", 
		target_arch, output_format, 
		should_overwrite ? "overwrite" : "non-destructive"
	);

	if (not target_arch) exit(0);

	uint8_t* my_bytes = NULL;
	nat my_count = 0;




#define max_section_count 128
	nat section_count = 0;
	nat section_starts[max_section_count] = {0};
	nat section_addresses[max_section_count] = {0};




if (target_arch == rv32_arch) {


	nat* lengths = calloc(ins_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		if (rt_ins[i].op == halt) continue;
		lengths[i] = ins[i].op == emit ? rt_ins[i].args[0] : 4;
	}

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("");
		dump_hex(my_bytes, my_count);
		getchar();


		const nat op = ins[i].op;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1]; 
		const nat a2 = ins[i].args[2]; 
		const nat a3 = ins[i].args[3]; 
		const nat a4 = ins[i].args[4]; 
		const nat a5 = ins[i].args[5]; 

		if (op == at or op == halt) { 	
			// do nothing

		} else if (op == r5_i) {
			const u32 word = 
				(a4 << 20U) | 
				(a3 << 15U) | 
				(a2 << 12U) | 
				(a1 <<  7U) | 
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_r) {
			const u32 word = 
				(a5 << 25U) | 
				(a4 << 20U) | 
				(a3 << 15U) | 
				(a2 << 12U) | 
				(a1 <<  7U) | 
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_s) {


		} else if (op == r5_b) {

		} else if (op == r5_b) {


		} else if (op == r5_b) {


		} else if (op == r5_b) {

		} else {
			printf("could not generate machine code for instruction: %llu\n", op);
			abort();
		}
	}

	goto finished_generation;








} else if (target_arch == msp430_arch) {

	nat* lengths = calloc(rt_ins_count, sizeof(nat));
	for (nat i = 0; i < rt_ins_count; i++) {
		const nat op = rt_ins[i].op;
		const u32 a0 = (u32) rt_ins[i].args[0];
		const u32 a1 = (u32) rt_ins[i].args[1];
		const u32 a4 = (u32) rt_ins[i].args[4];
		const u32 a5 = (u32) rt_ins[i].args[5];

		nat len = 0;
		if (op == section_start) len = 0;
		else if (op == halt) len = 0;
		else if (op == emit and a0 == 1) len = 1;
		else if (op == emit and a0 == 2) len = 2;
		else if (op == emit and a0 == 4) len = 4;
		else if (op == emit and a0 == 8) len = 8;
		else if (op == branch4) len = 2;
		else if (op == general4) {
			len = 2;
			if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
				or (a4 == 3 and not a5)) len += 2;						
			if (a1 == 1) len += 2;
		}
		lengths[i] = len;
	}

	print_nats(lengths, rt_ins_count);

	for (nat i = 0; i < rt_ins_count; i++) {
		const nat op = rt_ins[i].op;
		const u16 a0 = (u16) rt_ins[i].args[0];
		const u16 a1 = (u16) rt_ins[i].args[1];
		const u16 a2 = (u16) rt_ins[i].args[2];
		const u16 a3 = (u16) rt_ins[i].args[3];
		const u16 a4 = (u16) rt_ins[i].args[4];
		const u16 a5 = (u16) rt_ins[i].args[5];
		const u16 a6 = (u16) rt_ins[i].args[6];
		const u16 a7 = (u16) rt_ins[i].args[7];

		if (op == section_start) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;
		}
		else if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (nat) a1);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (u32) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (u16) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (byte) a1);
		}
		else if (op == branch4) {
			const u16 offset = 0x3FF & ((calculate_offset(lengths, i + 1, a1) >> 1));
			const u16 word = (u16) ((1U << 13U) | (u16)(a0 << 10U) | (offset));
			insert_u16(&my_bytes, &my_count, word);
		}
		else if (op == general4) {  
			// gen4  op(0)  dm(1) dr(2) di(3)  sm(4) sr(5) si(6)   size(7)
			u16 word = (u16) (
				(a0 << 12U) | (a5 << 8U) | (a1 << 7U) | 
				(a7 << 6U) | (a4 << 4U) | (a2)
			);
			insert_u16(&my_bytes, &my_count, word);

			if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
				or (a4 == 3 and not a5)) insert_u16(&my_bytes, &my_count, a6);
						
			if (a1 == 1) insert_u16(&my_bytes, &my_count, a3);
		}				
		else if (op == halt) {}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}	
	
} else if (target_arch == arm64_arch) {

	nat* lengths = calloc(rt_ins_count, sizeof(nat));
	for (nat i = 0; i < rt_ins_count; i++) {
		if (rt_ins[i].op == halt) continue;
		lengths[i] = rt_ins[i].op == emit ? rt_ins[i].args[0] : 4;
	}

	print_nats(lengths, rt_ins_count);

	for (nat i = 0; i < rt_ins_count; i++) {
		const nat op = rt_ins[i].op;
		const u32 a0 = (u32) rt_ins[i].args[0];
		const u32 a1 = (u32) rt_ins[i].args[1];
		const u32 a2 = (u32) rt_ins[i].args[2];
		const u32 a3 = (u32) rt_ins[i].args[3];
		const u32 a4 = (u32) rt_ins[i].args[4];
		const u32 a5 = (u32) rt_ins[i].args[5];
		const u32 a6 = (u32) rt_ins[i].args[6];
		const u32 a7 = (u32) rt_ins[i].args[7];

		if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (uint64_t) a1);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (uint8_t) a1);
		}
		else if (op == nop) insert_u32(&my_bytes, &my_count, 0xD503201F);
		else if (op == svc) insert_u32(&my_bytes, &my_count, 0xD4000001);
		else if (op == br) { // 
			uint32_t l = a2?2:a1?1:0;
			const uint32_t to_emit = 
				(0x6BU << 25U) | (l << 21U) | 
				(0x1FU << 16U) | (a0 << 5U);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == adc) { // 
			const uint32_t to_emit = 
				(a5 << 31U) | (a4 << 30U) | (a3 << 29U) | 
				(0xD0 << 21U) | (a2 << 16U) | (0 << 19U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == shv) {
			uint32_t op2 = 8;
			if (a3 == 0) op2 = 8;
			if (a3 == 1) op2 = 9;
			if (a3 == 2) op2 = 10;
			if (a3 == 3) op2 = 11;
			const uint32_t to_emit = 
				(a4 << 31U) | (0 << 30U) | 
				(0 << 29U) | (0xD6 << 21U) | 
				(a2 << 16U) | (op2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == mov) {
			const uint32_t to_emit = 
				(a4 << 31U) | (a3 << 29U) | (0x25U << 23U) | 
				(a2 << 21U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == bc) {
			const uint32_t offset = 0x7ffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = 
				(0x54U << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == jmp) {
			const uint32_t offset = 0x3ffffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = (a0 << 31U) | (0x5U << 26U) | (offset);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == adr) {
			uint32_t o1 = a2;
			nat count = calculate_offset(lengths, i, a1);
			if (a2) count /= 4096;
			const uint32_t offset = 0x1fffff & count;
			const uint32_t lo = offset & 3, hi = offset >> 2;
			const uint32_t to_emit = 
				(o1 << 31U) | (lo << 29U) | (0x10U << 24U) |
				(hi << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == cbz) {
			const uint32_t offset = 0x7ffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = 
				(a3 << 31U) | (0x1AU << 25U) | 
				(a2 << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == tbz) {
			const uint32_t b40 = a1 & 0x1F;
			const uint32_t b5 = a1 >> 5;
			const uint32_t offset = 0x3fff & (calculate_offset(lengths, i, a2) >> 2);
			const uint32_t to_emit = 
				(b5 << 31U) | (0x1BU << 25U) | (a3 << 24U) |
				(b40 << 19U) |(offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == ccmp) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a4 << 30U) | (0x1D2 << 21U) | 
				(a3 << 16U) | (a0 << 12U) | (a2 << 11U) | 
				(a1 << 5U) | (a5); 
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == addi) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a5 << 30U) | (a4 << 29U) | 
				(0x22 << 23U) | (a3 << 22U) | (a2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == addr) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0xB << 24U) | (a3 << 22U) | (a2 << 16U) |
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == addx) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0x59 << 21U) | (a2 << 16U) | (a3 << 13U) | 
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == divr) {
			const uint32_t to_emit = 
				(a4 << 31U) | (0xD6 << 21U) | (a2 << 16U) |
				(1 << 11U) | (a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);	
		} else if (op == csel) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a5 << 30U) | (0xD4 << 21U) | 
				(a2 << 16U) | (a3 << 12U) | (a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == madd) {
			const uint32_t to_emit = 
				(a7 << 31U) | (0x1B << 24U) | (a5 << 23U) | 
				(a4 << 21U) | (a2 << 16U) | (a6 << 15U) | 
				(a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == bfm) {
			u32 imms = 0, immr = 0;
			if (not a2) { imms = a3 + a4 - 1; immr = a3; } 
			else { imms = a4 - 1; immr = (a6 ? 64 : 32) - a3; }
			const uint32_t to_emit = (a6 << 31U) | (a5 << 29U) | 	
				(0x26U << 23U) | (a6 << 22U) | (immr << 16U) |
				(imms << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == ori) {


			puts("TODO: please implemented the ori instruction: "
				"this is the last instruction we need to implement "
				"and then we are done with iplemementing the arm64 backend!"
			);

			abort();


		} else if (op == orr) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a0 << 29U) | (10 << 24U) | 
				(a4 << 22U) | (a6 << 21U) | (a3 << 16U) | 
				(a5 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == memp) {
			const uint32_t to_emit = 
				(a1 << 30U) | (0x14 << 25U) | (a6 << 23U) | (a0 << 22U) | 
				(a5 << 15U) | (a3 << 10U) | (a4 << 5U) | (a2);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == memi) {
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a4 << 30U) | (0x39 << 24U) | (opc << 22U) |
				(a3 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == memia) { 			
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a4 << 30U) | (0x38 << 24U) | (opc << 22U) | (a3 << 12U) | 
				(a5 << 11U) | (1 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == memr) { 
			const u32 S = (a4 >> 2) & 1, option = a4 & 3;
			u32 opt = 0;
			if (option == 0) opt = 2;
			else if (option == 1) opt = 3;
			else if (option == 2) opt = 6;
			else if (option == 3) opt = 7;
			else abort();				
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a5 == 3) opc = 1;
			else if (a5 == 2 and is_signed) opc = 2;
			else if (a5 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a5 << 30U) | (0x38 << 24U) | (opc << 22U) |
				(1 << 21U) | (a3 << 16U) | (opt << 13U) |
				(S << 12U) | (2 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		} 
		else if (op == clz) { puts("clz is unimplemented currently, lol"); abort(); }
		else if (op == rev) { puts("rev is unimplemented currently, lol"); abort(); }
		else if (op == extr) { puts("extr is unimplemented currently, lol"); abort(); }
		else if (op == ldrl) { puts("ldrl is unimplemented currently, lol"); abort(); }
		else if (op == halt) {}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}
} else {
	printf("unknown target architecture: %llu\n", target_arch);
	abort();
}

	puts("info: machine code assembled. generating output file...");

if (output_format == debug_output_only) {

	puts("debugging executable bytes:\n");
	for (nat i = 0; i < my_count; i++) {
		if (i % 32 == 0) puts("");
		if (my_bytes[i]) printf("\033[32;1m");
		printf("%02hhx ", my_bytes[i]);
		if (my_bytes[i]) printf("\033[0m");
	}
	puts("");
	exit(0);

} else if (output_format == ti_txt_executable) {

	char out[14000] = {0};
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

	int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("open: could not create executable file"); exit(1); }
	write(file, out, len);
	close(file);
	printf("ti-txt: wrote %llu bytes to file %s.\n", len, output_filename);

	char debug_string[4096] = {0};
	snprintf(debug_string, sizeof debug_string, "../../led_display/embedded_assembler/msp430_disassembler/run %s", output_filename);
	system(debug_string);


} else if (output_format == macho_executable) {

	while (my_count % 16) insert_byte(&my_bytes, &my_count, 0);

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

	int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("could not create executable file"); exit(1); }
	int r = fchmod(file, 0777);
	if (r < 0) { perror("could not make the output file executable"); exit(1); }
	write(file, data, count);
	close(file);
	printf("mach-o: wrote %llu bytes to file %s.\n", count, output_filename);

	char codesign_string[4096] = {0};
	snprintf(codesign_string, sizeof codesign_string, "codesign -s - %s", output_filename);
	system(codesign_string);

	// debugging:
	snprintf(codesign_string, sizeof codesign_string, "otool -htvxVlL %s", output_filename);
	system(codesign_string);
	snprintf(codesign_string, sizeof codesign_string, "objdump -D %s", output_filename);
	system(codesign_string);

} else {
	printf("unknown target architecture: %llu\n", target_arch);
	abort();
}

	printf("info: successsfully generated executable: %s\n", output_filename);
}



}








	puts("generating final machine code binary...");

	uint8_t* my_bytes = NULL;
	nat my_count = 0;



#define max_section_count 128
	nat section_count = 0;
	nat section_starts[max_section_count] = {0};
	nat section_addresses[max_section_count] = {0};





	if (target_arch == rv32_arch) goto rv32_generate_machine_code;
	if (target_arch == arm64_arch) goto arm64_generate_machine_code;
	if (target_arch == msp430_arch) goto msp430_generate_machine_code;
	puts("unknown target"); abort();


rv32_generate_machine_code:

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("");
		dump_hex(my_bytes, my_count);
		getchar();


		const nat op = ins[i].op;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1]; 
		const nat a2 = ins[i].args[2]; 
		const nat a3 = ins[i].args[3]; 
		const nat a4 = ins[i].args[4]; 
		const nat a5 = ins[i].args[5]; 

		if (op == at or op == halt) { 	
			// do nothing

		} else if (op == r5_i) {
			const u32 word = 
				(a4 << 20U) | 
				(a3 << 15U) | 
				(a2 << 12U) | 
				(a1 <<  7U) | 
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_r) {
			const u32 word = 
				(a5 << 25U) | 
				(a4 << 20U) | 
				(a3 << 15U) | 
				(a2 << 12U) | 
				(a1 <<  7U) | 
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_s) {


		} else if (op == r5_b) {

		} else if (op == r5_b) {


		} else if (op == r5_b) {


		} else if (op == r5_b) {

		} else {
			printf("could not generate machine code for instruction: %llu\n", op);
			abort();
		}
	}

	goto finished_generation;

msp430_generate_machine_code:
	goto finished_generation;

arm64_generate_machine_code:

finished_generation:;

	puts("done: compliation successful.");

	puts("final_bytes:");
	dump_hex(my_bytes, my_count);
	
	if (output_format == debug_output_only) {

		printf("// risc-v executable autogenerated by my languages compiler\n");
		printf("export const executable = [\n\t");
		for (nat i = 0; i < my_count; i++) {
			if (i and (i % 8 == 0)) printf("\n\t");
			printf("0x%02hhX,", my_bytes[i]);
		}	
		printf("\n];\n");

	} else {
		puts("error: unknown output format.");
		abort();
	}

	exit(0);

























	print_dictionary(variables, values, is_readonly, undefined, (nat) -1);
	print_instructions(rt_ins, rt_ins_count);

	const nat target_arch = values[1];
	const nat output_format = values[2];
	const nat should_overwrite = values[3];

	printf("info: assemblying for [target = %llu, output = %llu (%s)]\n", 
		target_arch, output_format, 
		should_overwrite ? "overwrite" : "non-destructive"
	);

	if (not target_arch) exit(0);

	uint8_t* my_bytes = NULL;
	nat my_count = 0;

#define max_section_count 128
	nat section_count = 0;
	nat section_starts[max_section_count] = {0};
	nat section_addresses[max_section_count] = {0};

if (target_arch == msp430_arch) {

	nat* lengths = calloc(rt_ins_count, sizeof(nat));
	for (nat i = 0; i < rt_ins_count; i++) {
		const nat op = rt_ins[i].op;
		const u32 a0 = (u32) rt_ins[i].args[0];
		const u32 a1 = (u32) rt_ins[i].args[1];
		const u32 a4 = (u32) rt_ins[i].args[4];
		const u32 a5 = (u32) rt_ins[i].args[5];

		nat len = 0;
		if (op == section_start) len = 0;
		else if (op == halt) len = 0;
		else if (op == emit and a0 == 1) len = 1;
		else if (op == emit and a0 == 2) len = 2;
		else if (op == emit and a0 == 4) len = 4;
		else if (op == emit and a0 == 8) len = 8;
		else if (op == branch4) len = 2;
		else if (op == general4) {
			len = 2;
			if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
				or (a4 == 3 and not a5)) len += 2;						
			if (a1 == 1) len += 2;
		}
		lengths[i] = len;
	}

	print_nats(lengths, rt_ins_count);

	for (nat i = 0; i < rt_ins_count; i++) {
		const nat op = rt_ins[i].op;
		const u16 a0 = (u16) rt_ins[i].args[0];
		const u16 a1 = (u16) rt_ins[i].args[1];
		const u16 a2 = (u16) rt_ins[i].args[2];
		const u16 a3 = (u16) rt_ins[i].args[3];
		const u16 a4 = (u16) rt_ins[i].args[4];
		const u16 a5 = (u16) rt_ins[i].args[5];
		const u16 a6 = (u16) rt_ins[i].args[6];
		const u16 a7 = (u16) rt_ins[i].args[7];

		if (op == section_start) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;
		}
		else if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (nat) a1);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (u32) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (u16) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (byte) a1);
		}
		else if (op == branch4) {
			const u16 offset = 0x3FF & ((calculate_offset(lengths, i + 1, a1) >> 1));
			const u16 word = (u16) ((1U << 13U) | (u16)(a0 << 10U) | (offset));
			insert_u16(&my_bytes, &my_count, word);
		}
		else if (op == general4) {  
			// gen4  op(0)  dm(1) dr(2) di(3)  sm(4) sr(5) si(6)   size(7)
			u16 word = (u16) (
				(a0 << 12U) | (a5 << 8U) | (a1 << 7U) | 
				(a7 << 6U) | (a4 << 4U) | (a2)
			);
			insert_u16(&my_bytes, &my_count, word);

			if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
				or (a4 == 3 and not a5)) insert_u16(&my_bytes, &my_count, a6);
						
			if (a1 == 1) insert_u16(&my_bytes, &my_count, a3);
		}				
		else if (op == halt) {}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}	
	
} else if (target_arch == arm64_arch) {

	nat* lengths = calloc(rt_ins_count, sizeof(nat));
	for (nat i = 0; i < rt_ins_count; i++) {
		if (rt_ins[i].op == halt) continue;
		lengths[i] = rt_ins[i].op == emit ? rt_ins[i].args[0] : 4;
	}

	print_nats(lengths, rt_ins_count);

	for (nat i = 0; i < rt_ins_count; i++) {
		const nat op = rt_ins[i].op;
		const u32 a0 = (u32) rt_ins[i].args[0];
		const u32 a1 = (u32) rt_ins[i].args[1];
		const u32 a2 = (u32) rt_ins[i].args[2];
		const u32 a3 = (u32) rt_ins[i].args[3];
		const u32 a4 = (u32) rt_ins[i].args[4];
		const u32 a5 = (u32) rt_ins[i].args[5];
		const u32 a6 = (u32) rt_ins[i].args[6];
		const u32 a7 = (u32) rt_ins[i].args[7];

		if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (uint64_t) a1);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (uint8_t) a1);
		}
		else if (op == nop) insert_u32(&my_bytes, &my_count, 0xD503201F);
		else if (op == svc) insert_u32(&my_bytes, &my_count, 0xD4000001);
		else if (op == br) { // 
			uint32_t l = a2?2:a1?1:0;
			const uint32_t to_emit = 
				(0x6BU << 25U) | (l << 21U) | 
				(0x1FU << 16U) | (a0 << 5U);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == adc) { // 
			const uint32_t to_emit = 
				(a5 << 31U) | (a4 << 30U) | (a3 << 29U) | 
				(0xD0 << 21U) | (a2 << 16U) | (0 << 19U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == shv) {
			uint32_t op2 = 8;
			if (a3 == 0) op2 = 8;
			if (a3 == 1) op2 = 9;
			if (a3 == 2) op2 = 10;
			if (a3 == 3) op2 = 11;
			const uint32_t to_emit = 
				(a4 << 31U) | (0 << 30U) | 
				(0 << 29U) | (0xD6 << 21U) | 
				(a2 << 16U) | (op2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == mov) {
			const uint32_t to_emit = 
				(a4 << 31U) | (a3 << 29U) | (0x25U << 23U) | 
				(a2 << 21U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == bc) {
			const uint32_t offset = 0x7ffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = 
				(0x54U << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == jmp) {
			const uint32_t offset = 0x3ffffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = (a0 << 31U) | (0x5U << 26U) | (offset);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == adr) {
			uint32_t o1 = a2;
			nat count = calculate_offset(lengths, i, a1);
			if (a2) count /= 4096;
			const uint32_t offset = 0x1fffff & count;
			const uint32_t lo = offset & 3, hi = offset >> 2;
			const uint32_t to_emit = 
				(o1 << 31U) | (lo << 29U) | (0x10U << 24U) |
				(hi << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == cbz) {
			const uint32_t offset = 0x7ffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = 
				(a3 << 31U) | (0x1AU << 25U) | 
				(a2 << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == tbz) {
			const uint32_t b40 = a1 & 0x1F;
			const uint32_t b5 = a1 >> 5;
			const uint32_t offset = 0x3fff & (calculate_offset(lengths, i, a2) >> 2);
			const uint32_t to_emit = 
				(b5 << 31U) | (0x1BU << 25U) | (a3 << 24U) |
				(b40 << 19U) |(offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == ccmp) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a4 << 30U) | (0x1D2 << 21U) | 
				(a3 << 16U) | (a0 << 12U) | (a2 << 11U) | 
				(a1 << 5U) | (a5); 
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == addi) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a5 << 30U) | (a4 << 29U) | 
				(0x22 << 23U) | (a3 << 22U) | (a2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == addr) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0xB << 24U) | (a3 << 22U) | (a2 << 16U) |
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == addx) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0x59 << 21U) | (a2 << 16U) | (a3 << 13U) | 
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == divr) {
			const uint32_t to_emit = 
				(a4 << 31U) | (0xD6 << 21U) | (a2 << 16U) |
				(1 << 11U) | (a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);	
		} else if (op == csel) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a5 << 30U) | (0xD4 << 21U) | 
				(a2 << 16U) | (a3 << 12U) | (a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == madd) {
			const uint32_t to_emit = 
				(a7 << 31U) | (0x1B << 24U) | (a5 << 23U) | 
				(a4 << 21U) | (a2 << 16U) | (a6 << 15U) | 
				(a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == bfm) {
			u32 imms = 0, immr = 0;
			if (not a2) { imms = a3 + a4 - 1; immr = a3; } 
			else { imms = a4 - 1; immr = (a6 ? 64 : 32) - a3; }
			const uint32_t to_emit = (a6 << 31U) | (a5 << 29U) | 	
				(0x26U << 23U) | (a6 << 22U) | (immr << 16U) |
				(imms << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == ori) {


			puts("TODO: please implemented the ori instruction: "
				"this is the last instruction we need to implement "
				"and then we are done with iplemementing the arm64 backend!"
			);

			abort();


		} else if (op == orr) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a0 << 29U) | (10 << 24U) | 
				(a4 << 22U) | (a6 << 21U) | (a3 << 16U) | 
				(a5 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == memp) {
			const uint32_t to_emit = 
				(a1 << 30U) | (0x14 << 25U) | (a6 << 23U) | (a0 << 22U) | 
				(a5 << 15U) | (a3 << 10U) | (a4 << 5U) | (a2);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == memi) {
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a4 << 30U) | (0x39 << 24U) | (opc << 22U) |
				(a3 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == memia) { 			
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a4 << 30U) | (0x38 << 24U) | (opc << 22U) | (a3 << 12U) | 
				(a5 << 11U) | (1 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == memr) { 
			const u32 S = (a4 >> 2) & 1, option = a4 & 3;
			u32 opt = 0;
			if (option == 0) opt = 2;
			else if (option == 1) opt = 3;
			else if (option == 2) opt = 6;
			else if (option == 3) opt = 7;
			else abort();				
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a5 == 3) opc = 1;
			else if (a5 == 2 and is_signed) opc = 2;
			else if (a5 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a5 << 30U) | (0x38 << 24U) | (opc << 22U) |
				(1 << 21U) | (a3 << 16U) | (opt << 13U) |
				(S << 12U) | (2 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		} 
		else if (op == clz) { puts("clz is unimplemented currently, lol"); abort(); }
		else if (op == rev) { puts("rev is unimplemented currently, lol"); abort(); }
		else if (op == extr) { puts("extr is unimplemented currently, lol"); abort(); }
		else if (op == ldrl) { puts("ldrl is unimplemented currently, lol"); abort(); }
		else if (op == halt) {}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}
} else {
	printf("unknown target architecture: %llu\n", target_arch);
	abort();
}

	puts("info: machine code assembled. generating output file...");

if (output_format == debug_output_only) {

	puts("debugging executable bytes:\n");
	for (nat i = 0; i < my_count; i++) {
		if (i % 32 == 0) puts("");
		if (my_bytes[i]) printf("\033[32;1m");
		printf("%02hhx ", my_bytes[i]);
		if (my_bytes[i]) printf("\033[0m");
	}
	puts("");
	exit(0);

} else if (output_format == ti_txt_executable) {

	char out[14000] = {0};
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

	int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("open: could not create executable file"); exit(1); }
	write(file, out, len);
	close(file);
	printf("ti-txt: wrote %llu bytes to file %s.\n", len, output_filename);

	char debug_string[4096] = {0};
	snprintf(debug_string, sizeof debug_string, "../../led_display/embedded_assembler/msp430_disassembler/run %s", output_filename);
	system(debug_string);


} else if (output_format == macho_executable) {

	while (my_count % 16) insert_byte(&my_bytes, &my_count, 0);

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

	int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("could not create executable file"); exit(1); }
	int r = fchmod(file, 0777);
	if (r < 0) { perror("could not make the output file executable"); exit(1); }
	write(file, data, count);
	close(file);
	printf("mach-o: wrote %llu bytes to file %s.\n", count, output_filename);

	char codesign_string[4096] = {0};
	snprintf(codesign_string, sizeof codesign_string, "codesign -s - %s", output_filename);
	system(codesign_string);

	// debugging:
	snprintf(codesign_string, sizeof codesign_string, "otool -htvxVlL %s", output_filename);
	system(codesign_string);
	snprintf(codesign_string, sizeof codesign_string, "objdump -D %s", output_filename);
	system(codesign_string);

} else {
	printf("unknown target architecture: %llu\n", target_arch);
	abort();
}

	printf("info: successsfully generated executable: %s\n", output_filename);
}











}






































































































































































































































































































redo_cfs:
	for (nat pc = 0; pc < ins_count; pc++) {
		if (not ins[pc].ct) continue;

		//print_instruction_window_around(pc, 1, "CF SIMPLIFY: on this ins");
		print_instruction_window_around(pc, 0, "");
		print_dictionary();
		puts("-----------CF SIMPLIFY:---------------");
		getchar();

		const nat op = ins[pc].op;

		nat pred_count = 0;
		nat* preds = compute_predecessors(pc, &pred_count);
		nat* gotos = compute_successors(pc);

		nat visited_pred_count = 0;
		for (nat i = 0; i < pred_count; i++) 
			if (ins[preds[i]].ct) visited_pred_count++;
		
		if (not pc) visited_pred_count++;
		if (not visited_pred_count) { 
			ins[pc].ct = 0; 
			goto redo_cfs; 
		}

		if (op == do_) {
			nat count = 0;
			compute_predecessors(gotos[0], &count);
			if (gotos[0] == pc + 1) { 
				ins[pc].ct = 0; 
				if (count < 2) ins[pc + 1].ct = 0; 
				goto redo_cfs; 
			}
		}

		if (op == at and pred_count == 1) {
			puts("we need to inline this block!");
			abort();
		}
	}

	nat* label_use_count = calloc(var_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == do_) label_use_count[ins[i].args[0]]++;
		if (op == la) label_use_count[ins[i].args[1]]++;
		if (op == lt or op == ge or op == ne or op == eq) label_use_count[ins[i].args[2]]++;
	}

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == at and not label_use_count[ins[i].args[0]]) ins[i].ct = 0;
	}

	{ nat final_ins_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (not ins[i].ct) continue;
		ins[final_ins_count++] = ins[i];
	}
	ins_count = final_ins_count; }


	for (nat i = 0; i < ins_count; i++) {
		ins[i].ct = 0;
	}






























			struct instruction new = { .op = op };
			memcpy(new.args, ins[pc].args, sizeof new.args);
			new.imm = ins[pc].imm;
			for (nat i = 0; i < arity[op]; i++) {

				if (op == at and i == 0) continue;
				if(op == do_ and i == 0) continue;
				if (op == lt and i == 2) continue;
				if (op == ge and i == 2) continue;
				if (op == ne and i == 2) continue;
				if (op == eq and i == 2) continue;
				if (op == la and i == 1) continue;
				
				if (not ((new.imm >> i) & 1) and not is_runtime[new.args[i]]) {
					new.args[i] = values[new.args[i]];
					new.imm |= 1 << i;
				}
			}
			rt_ins[rt_ins_count++] = new;
			continue;




			else if (op == stringliteral) {
				for (nat i = 0; i < arg0; i++) {
					struct instruction new = { .op = emit };
					new.args[0] = 1; new.args[1] = (nat) string_list[arg1][i];
					rt_ins[rt_ins_count++] = new;
				}}







	
	nat stack[4096] = {0};
	nat stack_count = 1;

	memset(bit_count, 255, sizeof bit_count);
	memset(register_index, 255, sizeof register_index);

	while (stack_count) {
		nat pc = stack[--stack_count];

		if (pc >= ins_count) continue;

		nat pred_count = 0;
		nat* preds = compute_predecessors(pc, &pred_count);
		nat* gotos = compute_successors(pc);

		ins[pc].ct++;

		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;
		const nat gt0 = gotos[0];
		const nat gt1 = gotos[1];
		const nat a0 = 0 < arity[op] ? ins[pc].args[0] : 0;
		const nat a1 = 1 < arity[op] ? ins[pc].args[1] : 0;
		const nat a2 = 2 < arity[op] ? ins[pc].args[2] : 0;
		const nat i0 = !!(imm & 1);
		const nat i1 = !!(imm & 2);
		const nat i2 = !!(imm & 4);

		if (op == halt) continue; 
		else if (op == at) { }
		else if (op == sc) { }
		else if (op == do_) { pc = gt0; goto execute_ins; }
		else if (op == set) {
			if (not is_runtime[a0]) {
				out_t = ct1; out_v = v1;
			}
			out_def = pc;
			out_is_copy = 1;
			if (not i1 and is_copy[pc * var_count + a1]) {
				out_copy_ref = copy_of[pc * var_count + a1];
				out_copy_type = copy_type[pc * var_count + a1];
			} else { 
				out_copy_ref = a1;
				out_copy_type = i1;
			}
		}
		else if (op >= add and op <= sd) {
			if (not i1) out_t = ct0 and ct1;

			     if (op == add)  out_v += v1;
			else if (op == sub)  out_v -= v1;
			else if (op == mul)  out_v *= v1;
			else if (op == div_) out_v /= v1;
			else if (op == rem)  out_v %= v1;
			else if (op == and_) out_v &= v1;
			else if (op == or_)  out_v |= v1;
			else if (op == eor)  out_v ^= v1;
			else if (op == si)   out_v <<= v1;
			else if (op == sd)   out_v >>= v1;
			out_is_copy = 0;
			
		} else if (op == st) {	
			if (not ct2) { puts("error: size of store must be ctk."); abort(); }
			out_is_copy = 0;

		} else if (op == ld) {
			if (not ct2) { puts("error: size of load must be ctk."); abort(); }
			out_t = 0;
			out_is_copy = 0;

		} else if (op == la) {
			out_t = 0;
			out_v = 0;
			out_is_copy = 0;
						
		} else if (op == lt or op == ge or op == ne or op == eq) {
			if (not ct0 or not ct1) {
				last_pc = (nat) -1;
				if (gt0 < ins_count and ins[gt0].visited < 3) { 
					stack[stack_count++] = gt0; 
				}
				if (gt1 < ins_count and ins[gt1].visited < 3) { 
					stack[stack_count++] = gt1; 
				}
				continue;
			} else {
				bool cond = 0;
				     if (op == lt) cond = v0  < v1;
				else if (op == ge) cond = v0 >= v1;
				else if (op == eq) cond = v0 == v1;
				else if (op == ne) cond = v0 != v1;
				pc = cond ? gt1 : gt0; goto execute_ins;
			}

		} else if (op == rt) {
			if (not ct1) { puts("error: source argument to RT must be compiletime known."); abort(); }

			if (v1 & (1LLU << 63LLU)) register_index[a0] = v1 ^ (1LLU << 63LLU);
			else bit_count[a0] = v1;
			is_runtime[a0] = 1;
			out_t = 0;

		} else if (op < a6_nop) {
			puts("WARNING: EXECUTING AN UNKNOWN INSTRUCTION WITHOUT AN IMPLEMENTATION!!!");
			puts(operations[op]); 
			abort();
		}
		pc++; goto execute_ins;

	} // while stack count 


	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		print_dictionary();
		puts("-----------PRUNING CTK INS:---------------");
		getchar();

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		
		nat ignore = 1;

		if (	op == halt or op == sc or 
			op == do_  or op == at or op == la or 
			op == ld   or op == st
		) ignore = 0;

		//puts("looking at: "); 
		//print_instruction(ins[i], variables, var_count); puts("");
		
		if (ignore) for (nat a = 0; a < arity[op]; a++) {

			if (op == at and a == 0) continue;
			if (op == do_ and a == 0) continue;
			if (op == lt and a == 2) continue;
			if (op == ge and a == 2) continue;
			if (op == ne and a == 2) continue;
			if (op == eq and a == 2) continue;
			if (op == la and a == 1) continue; 
						
			if (((imm >> a) & 1)) {
				printf("found a compiletime immediate : %llu\n", 
					ins[i].args[a]
				);

			} else if (type[i * var_count + ins[i].args[a]]) {
				
				printf("found a compiletime variable "
					"as argument  :  %s = {type = %llu, value = %llu}\n",
					variables[ins[i].args[a]], 
					type[i * var_count + ins[i].args[a]],
					value[i * var_count + ins[i].args[a]]
				);

			} else {
				puts("found a runtime argument!");
				printf("found variable "
					":  %s = {type = %llu, value = %llu}\n",
					variables[ins[i].args[a]], 
					type[i * var_count + ins[i].args[a]],
					value[i * var_count + ins[i].args[a]]
				);
				ignore = 0; break;
			}
		}

		if (ignore or not ins[i].visited or op == rt) {
			if (op == lt or op == ge or op == ne or op == eq) {
				const nat v0 = (imm & 1) ? ins[i].args[0] : value[i * var_count + ins[i].args[0]];
				const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];
				bool cond = 0;
				     if (op == lt) cond = v0  < v1;
				else if (op == ge) cond = v0 >= v1;
				else if (op == eq) cond = v0 == v1;
				else if (op == ne) cond = v0 != v1;
				if (cond) { ins[i].op = do_; ins[i].args[0] = ins[i].args[2]; }
				else ins[i].visited = 0;

			} else { 
				puts("NOTE: found a compiletime-known instruction! deleting this instruction."); 
				getchar();
				ins[i].visited = 0; 
			} 
			continue;
		}

		puts("found real RT isntruction!"); 
		putchar('\t');
		print_instruction(ins[i]); 
		puts("");
		getchar();

		if (op >= set and op <= sd) {
			const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
			const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

			if (ct1 and not (imm & 2)) {
				puts("inlining compiletime argument...\n"); getchar();
				ins[i].args[1] = v1;
				ins[i].imm |= 2;
			}

			if (ins[i].imm & 2) {
				const nat c = ins[i].args[1];
				if (	(op == add or op == sub or
					 op == si or op == sd or
					 op == eor or op == or_)
					and not c or 
					(op == mul or op == div_)
					and c == 1	
				) ins[i].visited = 0; 
				else if (op == and_ and not c) ins[i].op = set;

				for (nat sh = 0; sh < 64; sh++) {
					if (op == mul and c == (1LLU << sh)) {
						ins[i].op = si;
						ins[i].args[1] = sh;
						break;
					} else if (op == div_ and c == (1LLU << sh)) {
						ins[i].op = sd;
						ins[i].args[1] = sh;
						break;
					} 
				}

			} else if (ins[i].args[0] == ins[i].args[1]) {
				if (op == set or op == and_ or op == or_) { 
					puts("found a rt NOP! deleting this instruction."); 
					ins[i].visited = 0; }
				else if (op == eor or op == sub) {
					ins[i].op = set;
					ins[i].imm |= 2;
					ins[i].args[1] = 0;
				}
			}

			if (not (ins[i].imm & 2) and is_copy[i * var_count + ins[i].args[1]]) {

				printf("note: inlining copy reference: a1=%llu imm=%llu copy_of=%llu, copy_type=%llu, i=%llu...\n", 
					ins[i].args[1], ins[i].imm, 
					copy_of[i * var_count + ins[i].args[1]], 
					copy_type[i * var_count + ins[i].args[1]],
					i
				);

				puts("original:");
				print_instruction(ins[i]); puts("");

				if (copy_type[i * var_count + ins[i].args[1]]) ins[i].imm |= 2;
				ins[i].args[1] = copy_of[i * var_count + ins[i].args[1]];

				puts("modified form:"); 
				print_instruction(ins[i]); 
				puts("");
				getchar();
			}

		} else if (op == ld or op == st) {
			puts("we still need to embed the immediates into the load and store instructions.");
			abort();


		} else if (op == lt or op == ge or op == ne or op == eq) {
			const nat ct0 = (imm & 1) or type[i * var_count + ins[i].args[0]];
			const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
			const nat v0 = (imm & 1) ? ins[i].args[0] : value[i * var_count + ins[i].args[0]];
			const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

			if (ct0 and not (imm & 1)) {
				ins[i].args[0] = v0;
				ins[i].imm |= 1;

			} else if (ct1 and not (imm & 2)) {
				ins[i].args[1] = v1;
				ins[i].imm |= 2;
			}

			if (not ins[i].imm and ins[i].args[0] == ins[i].args[1]) {
				if (op == lt or op == ne) ins[i].visited = 0; 
				else if (op == eq or op == ge) {
					ins[i].op = do_;
					ins[i].args[0] = ins[i].args[2];
				}
			}
		}
	}



redo_cfs:

	{ nat final_ins_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (not ins[i].visited) continue;
		ins[final_ins_count++] = ins[i];
	}
	ins_count = final_ins_count; }

	for (nat pc = 0; pc < ins_count; pc++) {
		if (not ins[pc].visited) continue;

		//print_instruction_window_around(pc, 1, "CF SIMPLIFY: on this ins");

		print_instruction_window_around(pc, 0, "");
		print_dictionary();
		puts("-----------CF SIMPLIFY:---------------");
		getchar();

		const nat op = ins[pc].op;

		nat pred_count = 0;
		nat* preds = compute_predecessors(pc, &pred_count);
		nat* gotos = compute_successors(pc);

		nat visited_pred_count = 0;
		for (nat i = 0; i < pred_count; i++) 
			if (ins[preds[i]].visited) visited_pred_count++;
		
		if (not pc) visited_pred_count++;
		if (not visited_pred_count) { 
			ins[pc].visited = 0; 
			goto redo_cfs; 
		}

		if (op == do_) {
			nat count = 0;
			compute_predecessors(gotos[0], &count);
			if (gotos[0] == pc + 1) { 
				ins[pc].visited = 0; 
				if (count < 2) ins[pc + 1].visited = 0; 
				goto redo_cfs; 
			}
		}

		if (op == at and pred_count == 1) {
			puts("we need to inline this block!");
			abort();
		}
	}



	nat* label_use_count = calloc(var_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == do_) label_use_count[ins[i].args[0]]++;
		if (op == la) label_use_count[ins[i].args[1]]++;
		if (op == lt or op == ge or op == ne or op == eq) label_use_count[ins[i].args[2]]++;
	}
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == at and not label_use_count[ins[i].args[0]]) ins[i].visited = 0;
	}

	{ nat final_ins_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (not ins[i].visited) continue;
		ins[final_ins_count++] = ins[i];
	}
	ins_count = final_ins_count; }


	for (nat i = 0; i < ins_count; i++) {
		ins[i].visited = 0;
	}





	struct instruction* new_ins = calloc(ins_count, sizeof(struct instruction));
	nat new_ins_count = 0;

	stack_count = 1;
	stack[0] = 0;

	while (stack_count) {

		const nat pc = stack[--stack_count];

		//print_instruction_window_around(pc, 1, "CF SIMPLIFY: on this ins");

		print_instruction_window_around(pc, 0, "");

		//print_instructions(1);
		print_dictionary();
		puts("-----------CF SIMPLIFY:---------------");

		puts("current new instruction listing: ");
		for (nat i = 0; i < new_ins_count; i++) {
			printf("\t#%llu: ", i); print_instruction(new_ins[i]); puts("");
		}
		getchar();

		const nat op = ins[pc].op;
	
		ins[pc].visited++;

		nat pred_count = 0;
		nat* preds = compute_predecessors(pc, &pred_count);
		nat* gotos = compute_successors(pc);

		new_ins[new_ins_count++] = ins[pc];

		if (op == halt) {
			// do nothing.
		} else if (op == lt or op == ge or op == ne or op == eq) {
			if (gotos[1] < ins_count and not ins[gotos[1]].visited) stack[stack_count++] = gotos[1];
			if (gotos[0] < ins_count and not ins[gotos[0]].visited) stack[stack_count++] = gotos[0];		
		} else {
			if (gotos[0] < ins_count and not ins[gotos[0]].visited) stack[stack_count++] = gotos[0];
		}

	}



	{ nat final_ins_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (not ins[i].visited) continue;
		ins[final_ins_count++] = ins[i];
	}
	ins_count = final_ins_count; }



	puts("final optimized instruction listing:");
	print_dictionary();
	print_instructions(false);
	puts("[done with opt phase]");

	exit(0);


// ------------------------------------------------- backend ------------------------------------------------------------


//enum all_architectures { no_arch, arm64_arch, arm32_arch, rv64_arch, rv32_arch, msp430_arch, };
//enum all_output_formats { debug_output_only, macho_executable, elf_executable, ti_txt_executable };

	nat target_arch = rv32_arch;
	nat output_format = debug_output_only;
	nat should_overwrite = true;


	printf("info: assemblying for [target = %llu, output = %llu (%s)]\n", 
		target_arch, output_format, 
		should_overwrite ? "overwrite" : "non-destructive"
	);


	if (not target_arch) exit(0);

	struct instruction mi[4096] = {0};
	nat mi_count = 0;

	for (nat i = 0; i < ins_count; i++)
		ins[i].visited = 0;

	if (target_arch == rv32_arch) 	goto rv32_instruction_selection;
	if (target_arch == msp430_arch) 	goto msp430_instruction_selection;
	if (target_arch == arm64_arch) 	goto arm64_instruction_selection;
	puts("instruction selection: error: unknown target"); abort();


rv32_instruction_selection:;

	puts("rv32: instruction selection starting...");


	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		print_dictionary();
		getchar();

		if (ins[i].visited) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i); 
			getchar();
			continue;
		}

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1]; 

		if (	
			op == r5_i or 
			op == r5_r or 
			op == r5_s or 
			op == r5_b or 
			op == r5_u or 
			op == r5_j or 
			op == at or 
			op == halt
		) { ins[i].visited = 1; mi[mi_count++] = ins[i]; continue; }

		if (op == set and not imm) { // addi d n 0 
			struct instruction new = { .op = r5_i, .imm = 0x15 };
			new.args[0] = 0x13;
			new.args[1] = arg0;
			new.args[2] = 0;
			new.args[3] = arg1;
			new.args[4] = 0;
			mi[mi_count++] = new;
			ins[i].visited = 1;
			continue;
		} 

		if (op == set and imm) { // addi d zr k
			struct instruction new = { .op = r5_i, .imm = 0x1D };
			new.args[0] = 0x13;
			new.args[1] = arg0;
			new.args[2] = 0;
			new.args[3] = 0;
			new.args[4] = arg1;
			mi[mi_count++] = new;
			ins[i].visited = 1;
			continue;
		}

		if (op == add and imm) { // addi d d k
			struct instruction new = { .op = r5_i, .imm = 0x15 };
			new.args[0] = 0x13;
			new.args[1] = arg0;
			new.args[2] = 0;
			new.args[3] = arg0;
			new.args[4] = arg1;
			mi[mi_count++] = new;
			ins[i].visited = 1;
			continue;
		}

		if (op == add and not imm) { // addr d d n
			struct instruction new = { .op = r5_r, .imm = 0x25 };
			new.args[0] = 0x33;
			new.args[1] = arg0;
			new.args[2] = 0;
			new.args[3] = arg0;
			new.args[4] = arg1;
			new.args[5] = 0;
			mi[mi_count++] = new;
			ins[i].visited = 1;
			continue;
		}
	}

	goto finish_instruction_selection;


msp430_instruction_selection:

	// do stuff for msp430 here lol

	goto finish_instruction_selection;


arm64_instruction_selection:;

	puts("arm64: instruction selection starting...");

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		getchar();

		if (selected[i]) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i); 
			getchar();
			continue;
		}

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1]; 

		if (op == halt) { mi[mi_count++] = ins[i]; selected[i] = 1; continue; }
		if (op == at) { mi[mi_count++] = ins[i]; selected[i] = 1; continue; }

		if (op == set) {
			const nat b = locate_instruction(
				(struct expected_instruction){ .op = si, .imm = 2, .use = 1, .args[0] = arg0 },
				i + 1, ins, ins_count
			);
			if (b == (nat) -1) goto addsrlsl_bail;

			const nat c = locate_instruction(
				(struct expected_instruction){ .op = add, .use = 1, .args[0] = arg0 },
				b + 1, ins, ins_count
			);
			if (c == (nat) -1) goto addsrlsl_bail;

			struct instruction new = { .op = a6_addr, .imm = 0xf8 };
			new.args[0] = arg0; 		// d
			new.args[1] = ins[c].args[1]; 	// n
			new.args[2] = arg1; 		// m 
			new.args[3] = ins[b].args[1]; 	// k
			new.args[4] = 0; //???sb?
			new.args[5] = 0; // ????sf???
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1; selected[c] = 1;
			goto finish_mi_instruction;
		} addsrlsl_bail:

		if (op == set and not imm) {
			struct instruction new = { .op = a6_orr, .imm = 0xff };
			new.args[0] = arg0;
			new.args[1] = 0;
			new.args[2] = arg1;			
			new.args[3] = 0;
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

		else if (op == set and imm) {
			struct instruction new = { .op = a6_mov, .imm = 0xfe };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = 0;
			new.args[3] = 0;
			new.args[4] = 0;
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

		if (op == lt and not imm) {
			struct instruction new = { .op = a6_addr, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = lt;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == eq and not imm) {
			struct instruction new = { .op = a6_orr, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = eq;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == lt and imm) {			
			struct instruction new = { .op = a6_addi, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = lt;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == eq and imm) {
			struct instruction new = { .op = a6_ori, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = eq;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == sc) {
			struct instruction new = { .op = a6_svc, .imm = 0xff };
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

		
	finish_mi_instruction:
		puts("so far:");
		print_instructions(mi, mi_count, variables, var_count, 0);
		getchar();

	}


finish_instruction_selection:;

	for (nat i = 0; i < ins_count; i++) {
		if (not ins[i].visited) {
			puts("error: instruction unprocessed by ins sel: internal error");
			puts("error: this instruction failed to be lowered:\n");
			print_instruction_window_around(i, 1, "not selected instruction!");
			puts("");
			abort();
		}
	}

	puts("finished instruction selection!");
	for (nat i = 0; i < mi_count; i++) ins[i] = mi[i];
	ins_count = mi_count;

	printf("info: final machine code: for target = %llu\n", target_arch);
	print_instructions(1);

	puts("starting register allocation!");



	for (nat i = 0; i < mi_count; i++) {

		printf("%llu: ", i);
		print_instruction(ins[i]); puts("");

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
	

		for (nat a = 0; a < arity[op]; a++) {

			const nat this_arg = ins[i].args[a];

			if (imm & (1 << a)) {
				printf("on argument: [a = %llu]: is_immediate!  (immediate value is %llu)\n", a, this_arg);
			} else {
				
				printf("on argument: [a = %llu]: NOT is_immediate!  (variable = %s) \n", a, variables[this_arg]);
				puts("filling in the register index we found for this operation!");

				if (register_index[this_arg] == (nat) -1) {
					printf("FATAL ERROR: no hardware register index was "
						"not found for variable %s in the below instruction. "
						"aborting...\n", variables[this_arg]
					);
					print_instruction(ins[i]); puts(""); 
					abort();
				}
				ins[i].args[a] = register_index[this_arg];
				ins[i].imm |= 1LLU << a;

				printf("info: filled in register_index %llu for variable %s into this instruction. ", 
					ins[i].args[a], variables[this_arg]
				);				
			}
		}
		puts("hello");
	}

	puts("[done with RA");

	printf("info: final machine code: for target = %llu\n", target_arch);
	print_dictionary();
	print_instructions(1);





	puts("generating final machine code binary...");

	uint8_t* my_bytes = NULL;
	nat my_count = 0;



#define max_section_count 128
	nat section_count = 0;
	nat section_starts[max_section_count] = {0};
	nat section_addresses[max_section_count] = {0};





	if (target_arch == rv32_arch) goto rv32_generate_machine_code;
	if (target_arch == arm64_arch) goto arm64_generate_machine_code;
	if (target_arch == msp430_arch) goto msp430_generate_machine_code;
	puts("unknown target"); abort();


rv32_generate_machine_code:

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, 0, "");
		puts("");
		dump_hex(my_bytes, my_count);
		getchar();


		const nat op = ins[i].op;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1]; 
		const nat a2 = ins[i].args[2]; 
		const nat a3 = ins[i].args[3]; 
		const nat a4 = ins[i].args[4]; 
		const nat a5 = ins[i].args[5]; 

		if (op == at or op == halt) { 	
			// do nothing

		} else if (op == r5_i) {
			const u32 word = 
				(a4 << 20U) | 
				(a3 << 15U) | 
				(a2 << 12U) | 
				(a1 <<  7U) | 
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_r) {
			const u32 word = 
				(a5 << 25U) | 
				(a4 << 20U) | 
				(a3 << 15U) | 
				(a2 << 12U) | 
				(a1 <<  7U) | 
				(a0 <<  0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_s) {


		} else if (op == r5_b) {

		} else if (op == r5_b) {


		} else if (op == r5_b) {


		} else if (op == r5_b) {

		} else {
			printf("could not generate machine code for instruction: %llu\n", op);
			abort();
		}
	}

	goto finished_generation;

msp430_generate_machine_code:
	goto finished_generation;

arm64_generate_machine_code:

finished_generation:;

	puts("done: compliation successful.");

	puts("final_bytes:");
	dump_hex(my_bytes, my_count);
	
	if (output_format == debug_output_only) {

		printf("// risc-v executable autogenerated by my languages compiler\n");
		printf("export const executable = [\n\t");
		for (nat i = 0; i < my_count; i++) {
			if (i and (i % 8 == 0)) printf("\n\t");
			printf("0x%02hhX,", my_bytes[i]);
		}	
		printf("\n];\n");

	} else {
		puts("error: unknown output format.");
		abort();
	}

	exit(0);
}


*/







































/*

static const char* type_string[] = {
	"(none):", 
	"\033[1;31merror:\033[0m", 
	"\033[1;35mwarning:\033[0m", 
	"\033[1;36minfo:\033[0m", 
	"\033[1;32mdata:\033[0m", 
	"\033[1;33mdebug:\033[0m"
};




static void dump_hex(uint8_t* memory, nat count) {
	printf("dumping bytes: (%llu)\n", count);
	for (nat i = 0; i < count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx(%c) ", memory[i], memory[i] >= 32 ? memory[i] : ' ');
	}
	puts("");
}


static void check(nat value, nat limit, nat a, struct instruction this) {
	if (value >= limit) {
		puts("check error");
		snprintf(reason, sizeof reason, "check: value %llu >= limit %llu check did not succeed for instruction", value, limit);
		print_message(error, reason, this.source[a + 1]);
		exit(1);
	}
}

static void check_offset(__attribute__((unused)) nat value, __attribute__((unused)) nat limit, __attribute__((unused)) nat arg, struct instruction this) {
	if ((0)) {
		puts("check_offset error");
		print_message(error, "sorry bad logic or something", this.source[0]);
		exit(1);
	}
}

static void r_type(nat* a, nat o, nat f, nat g, struct instruction this) {
	check(a[0], 32, 0, this);
	check(a[1], 32, 1, this);
	check(a[2], 32, 2, this);
	emitw( (g << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o);
}

static void i_type(nat* a, nat o, nat f, struct instruction this) { 
	check(a[0], 32, 0, this);
	check(a[1], 32, 1, this);
	check(a[2], 1 << 12, 2, this);
	emitw( (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o);
}

static void s_type(nat* a, nat o, nat f, struct instruction this) {
	check(a[0], 32, 0, this);
	check(a[1], 32, 1, this);
	check(a[2], 1 << 12, 2, this);
	emitw( ((a[0] >> 5U) << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | ((a[0] & 0x1F) << 7U) | o);
}

static void u_type(nat* a, nat o, struct instruction this) { 
	check(a[0], 32, 0, this);
	check(a[1], 1 << 20, 1, this);
	emitw( (a[1] << 12U) | (a[0] << 7U) | o);
}

static nat ins_size(nat op) {

	if (architecture == arm64) {
		if (
			op == lt or  op == ge or 
			op == ne or  op == eq or
			op == slt
		) return 8; 
		return 4;
	} else {
		puts("that target is not supported yet...");
		abort();
	}
}


static nat calculate_offset(nat here, nat label, struct instruction this) {
	printf("calculate_offset: called using here=%llu, label=%llu...\n", here, label);
	nat offset = 0;
	if (label < here) {
		printf("backwards branch...\n");
		for (nat i = label; i < here; i++) {
			if (i >= ins_count) {
				print_message(error, "invalid label given to a branching instruction", 
						this.source[0]);
				exit(1);
			}
			offset -= ins_size(ins[i].args[0]);
		}
	} else {
		printf("forwards branch...\n");
		for (nat i = here; i < label; i++) {
			if (i >= ins_count) {
				print_message(error, "invalid label given to a branching instruction", 
						this.source[0]);
				exit(1);
			}
			offset += ins_size(ins[i].args[0]);
		}
	}
	printf("output: found an offset of %llu bytes.\n", offset);
	return offset;
}

static void j_type(nat here, nat* a, nat o, struct instruction this) {
	
	const nat e = calculate_offset(here, a[1], this);

	check(a[0], 32, 0, this);
	check_offset(e, 1 << 0, 1, this);

	const nat imm19_12 = (e & 0x000FF000);
	const nat imm11    = (e & 0x00000800) << 9;
	const nat imm10_1  = (e & 0x000007FE) << 20;
	const nat imm20    = (e & 0x00100000) << 11;
	const nat imm = imm20 | imm10_1 | imm11 | imm19_12;
	emitw( (imm << 12U) | (a[0] << 7U) | o);
}

static void generate_riscv_machine_code(void) {
	abort();

	
	for (nat i = 0; i < ins_count; i++) {

		struct instruction this = ins[i];
		nat op = this.args[0];
		nat* a = this.args + 1;

		if (op == ec)   emitw(0x00000073);

		else if (op == add)     r_type(a, 0x33, 0x0, 0x00, this);
		else if (op == sub)     r_type(a, 0x33, 0x0, 0x20, this);
		else if (op == slt)     r_type(a, 0x33, 0x3, 0x00, this);
		
		
	
		else if (op == and_)    r_type(a, 0x33, 0x7, 0x00, this);
		else if (op == or_)     r_type(a, 0x33, 0x6, 0x00, this);
		else if (op == eor)     r_type(a, 0x33, 0x4, 0x00, this);
		else if (op == sl)      r_type(a, 0x33, 0x1, 0x00, this);
		else if (op == sr)      r_type(a, 0x33, 0x5, 0x00, this);
		
		
		

		else if (op == sl and is_signed)     r_type(a, 0x33, 0x2, 0x00, this);
		else if (op == sr and is_signed)     r_type(a, 0x33, 0x5, 0x20, this);

		else if (op == lb)     i_type(a, 0x03, 0x0, this);
		else if (op == lh)     i_type(a, 0x03, 0x1, this);
		else if (op == lw)     i_type(a, 0x03, 0x2, this);
		else if (op == ld)     i_type(a, 0x03, 0x3, this);

		//else if (op == addi)    i_type(a, 0x13, 0x0, this);
		//else if (op == sltis)   i_type(a, 0x13, 0x2, this);
		//else if (op == slti)    i_type(a, 0x13, 0x3, this);
		//else if (op == eori)    i_type(a, 0x13, 0x4, this);
		//else if (op == iori)    i_type(a, 0x13, 0x6, this);
		//else if (op == andi)    i_type(a, 0x13, 0x7, this);
		//else if (op == slli)    i_type(a, 0x13, 0x1, this);
		//else if (op == srli)    i_type(a, 0x13, 0x5, this);
		//else if (op == srai)    i_type(a, 0x13, 0x5, this);

		else if (op == cr)    i_type(a, 0x67, 0x0, this);

		else if (op == sb)      s_type(a, 0x23, 0x0, this);
		else if (op == sh)      s_type(a, 0x23, 0x1, this);
		else if (op == sw)      s_type(a, 0x23, 0x2, this);
		else if (op == sd)      s_type(a, 0x23, 0x3, this);

		// else if (op == lui)  u_type(a, 0x37, this);
		// else if (op == lpa)   u_type(a, 0x17, this);

		else if (op == cl)     j_type(i, a, 0x6F, this);

		else {
			snprintf(reason, sizeof reason, "riscv: unknown runtime instruction: \"%s\" (%llu)\n", spelling[op], op);
			print_message(error, reason, this.start[0], this.count[0]);
			exit(1);
		}
	}
	
}




*/







































































/*












	r_type: 	
		a0: opcode(0-6)  
		a1: rd(7-11)   
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: rs2(20-24)
		a5: funct7(25-31)

	i_type: 
		a0: opcode(0-6)
		a1: rd(7-11)
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: imm_11_0(20-31)

	s_type: 	
		a0: opcode(0-6)  
		a1: imm_4_0(7-11)
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: rs2(20-24)
		a5: imm_5_11(25-31)

	b_type: 
		a0: opcode(0-6)
		a1: imm_4_1_11(7-11)
		a2: funct3(12-14)
		a3: rs1(15-19)
		a4: rs2(20-24)
		a5: imm_12_10_5(25-31)

	u_type: 	
		a0: opcode(0-6)
		a1: rd(7-11)
		a2: imm_31_12(12-31)

	j_type: 	
		a0: opcode(0-6)
		a1: rd(7-11)
		a2: imm_20_10_1_11_19_12(12-31)
		
*/




































/*












static void push_mi(
	struct instruction* mi, 
	nat* mi_count, 

	nat op, nat imm, 
	
	nat arg0, nat arg1, nat arg2, 
	nat arg3, nat arg4, nat arg5
) {












	puts("preliminary:");
	print_dictionary(variables, is_undefined, var_count);
	print_instructions(mi, mi_count, variables, var_count, 0);

	for (nat i = 0; i < ins_count; i++) {
		if (not selected[i]) {

			puts("error: instruction unprocessed by ins sel: internal error");
			puts("error: this instruction failed to be lowered:\n");
			print_instruction_window_around(i, ins, ins_count, variables, var_count, selected, 1, "not selected instruction!");
			puts("");
			abort();
		}
	}

	print_dictionary(variables, is_undefined, var_count);
	print_instructions(mi, mi_count, variables, var_count, 0);
	puts("finished instruction selection!");


*/


















































































































/*

static void print_binary(nat x) {
	if (not x) { puts("0"); return; }

	for (nat i = 0; i < 64 and x; i++) {
		if (not (i % 8) and i) putchar('.');
		putchar((x & 1) + '0'); x >>= 1;
	}
	puts("");
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















/*










------------------------------------------------------------------------------------------------------------------------
current state 1202504196.234547
------------------------------------------------------------------------------------------------------------------------

	we should flush out the assembler outputting of machine instructions 
	first, and then work on the optimizations! so we can use the language
	to code up the display firmware!! 


	we are currently on loop unrolling, and trying to fit that into the 
	ct-eval stage, and also combine several other optimizations with it. 
	these all should in theory be able to be done at the same time. 

	also, we want to try to make the data stored in the value/type matrix 
	sparse, so that we are only storing changes. this will compress a tonn
	better, and make it so that we can store instruction operand chnages
	across ct execution. 

	another way to do loop unrolling is to generate a new ins list
	as a result of execution, but this has problems because of 
	the rt graph traversal machinery going over things twice. 

------------------------------------------------------------------------------------------------------------------------





















--------------------------------------------------------
we are currently doing these optimizations:
--------------------------------------------------------

	- copy propagation
	- FULL constant propgation/folding (with control flow)
	- dead code elimination
	- pointless goto's
	- no-operation elimination
	- strength reduction


--------------------------------------------------------
remaining optimizations to perform:
--------------------------------------------------------

	- algebraic simplification     [ medium ]




	- jump threading cf simplification     [ medium ]

	- common subexpression elimination   [ hard ] 

	- dead store elimination   [ easy ]



	- unused variable     [ easy ]

	- set but not used     [ easy ]

	- loop unrolling  [ easy ish ]


------------------------------------------------



***	- loop unrolling.......

		wait, CRAP..  hmmm





--------------------------------------------------------















	halt : termination point in the control flow graph. control flow does not continue past this instruction. 
	sc : system call, target specific, and triggers a context switch on targets with an operating system, to perform a specialized task. 
	do k : unconditional graph to label k. 
	at k : attribute label k to this position in the code. k is used as the destination of branches, or with the source of an la instruction.
	lf f : load file f from the filesystem, and include its parsed contents here.
	rt x y : force the variable x to be runtime known. 
		if y > 0, this sets the number of bits allocated to x, and 
		if y == 0, x is forced to be runtime known, with no further constraints, and
		if y < 0, this denotes the hardware register x should be allocated in. 
	set x y : assignment to destination register x, using the value present in source y.
	add x y : assigns the value x + y to the destination register x.
	sub x y : assigns the value x - y to the destination register x.
	mul x y : assigns the value x * y to the destination register x.
	div x y : assigns the value x / y to the destination register x.
	rem x y : assigns the value x modulo y to the destination register x.
	and x y : assigns the value x bitwise-AND y to the destination register x.
	or x y : assigns the value x bitwise-OR y to the destination register x.
	eor x y : assigns the value x bitwise-XOR y to the destination register x.
	si x y : shifts the bits in x up by y bits. 
	sd x y : shifts the bits in x down by y bits. (always an unsigned shift)
	la x k : loads a program-counter relative address given by a label k into a destination register x.
	ld x y z : load z bytes from memory address y into destination register x. 
	st x y z : store z bytes from the soruce register y into the memory at address x.
	lt x y k : if x is less than y, control flow branches to label k. 
	ge x y k : if x is not less than y, control flow branches to label k. 
	ne x y k : if x is not equal to y, control flow branches to label k. 
	eq x y k : if x is equal to y, control flow branches to label k. 

	udf0 : undefine source0 register variable used by previous instruction
	udf1 : undefine source1 register variable used by previous instruction
	udf2 : undefine source2 register variable used by previous instruction



static const char* type_string[] = {
	"(none):", 
	"\033[1;31merror:\033[0m", 
	"\033[1;35mwarning:\033[0m", 
	"\033[1;36minfo:\033[0m", 
	"\033[1;32mdata:\033[0m", 
	"\033[1;33mdebug:\033[0m"
};




static void dump_hex(uint8_t* memory, nat count) {
	printf("dumping bytes: (%llu)\n", count);
	for (nat i = 0; i < count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx(%c) ", memory[i], memory[i] >= 32 ? memory[i] : ' ');
	}
	puts("");
}


static void check(nat value, nat limit, nat a, struct instruction this) {
	if (value >= limit) {
		puts("check error");
		snprintf(reason, sizeof reason, "check: value %llu >= limit %llu check did not succeed for instruction", value, limit);
		print_message(error, reason, this.source[a + 1]);
		exit(1);
	}
}

static void check_offset(__attribute__((unused)) nat value, __attribute__((unused)) nat limit, __attribute__((unused)) nat arg, struct instruction this) {
	if ((0)) {
		puts("check_offset error");
		print_message(error, "sorry bad logic or something", this.source[0]);
		exit(1);
	}
}

static void r_type(nat* a, nat o, nat f, nat g, struct instruction this) {
	check(a[0], 32, 0, this);
	check(a[1], 32, 1, this);
	check(a[2], 32, 2, this);
	emitw( (g << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o);
}

static void i_type(nat* a, nat o, nat f, struct instruction this) { 
	check(a[0], 32, 0, this);
	check(a[1], 32, 1, this);
	check(a[2], 1 << 12, 2, this);
	emitw( (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o);
}

static void s_type(nat* a, nat o, nat f, struct instruction this) {
	check(a[0], 32, 0, this);
	check(a[1], 32, 1, this);
	check(a[2], 1 << 12, 2, this);
	emitw( ((a[0] >> 5U) << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | ((a[0] & 0x1F) << 7U) | o);
}

static void u_type(nat* a, nat o, struct instruction this) { 
	check(a[0], 32, 0, this);
	check(a[1], 1 << 20, 1, this);
	emitw( (a[1] << 12U) | (a[0] << 7U) | o);
}

static nat ins_size(nat op) {

	if (architecture == arm64) {
		if (
			op == lt or  op == ge or 
			op == ne or  op == eq or
			op == slt
		) return 8; 
		return 4;
	} else {
		puts("that target is not supported yet...");
		abort();
	}
}


static nat calculate_offset(nat here, nat label, struct instruction this) {
	printf("calculate_offset: called using here=%llu, label=%llu...\n", here, label);
	nat offset = 0;
	if (label < here) {
		printf("backwards branch...\n");
		for (nat i = label; i < here; i++) {
			if (i >= ins_count) {
				print_message(error, "invalid label given to a branching instruction", 
						this.source[0]);
				exit(1);
			}
			offset -= ins_size(ins[i].args[0]);
		}
	} else {
		printf("forwards branch...\n");
		for (nat i = here; i < label; i++) {
			if (i >= ins_count) {
				print_message(error, "invalid label given to a branching instruction", 
						this.source[0]);
				exit(1);
			}
			offset += ins_size(ins[i].args[0]);
		}
	}
	printf("output: found an offset of %llu bytes.\n", offset);
	return offset;
}

static void j_type(nat here, nat* a, nat o, struct instruction this) {
	
	const nat e = calculate_offset(here, a[1], this);

	check(a[0], 32, 0, this);
	check_offset(e, 1 << 0, 1, this);

	const nat imm19_12 = (e & 0x000FF000);
	const nat imm11    = (e & 0x00000800) << 9;
	const nat imm10_1  = (e & 0x000007FE) << 20;
	const nat imm20    = (e & 0x00100000) << 11;
	const nat imm = imm20 | imm10_1 | imm11 | imm19_12;
	emitw( (imm << 12U) | (a[0] << 7U) | o);
}

static void generate_riscv_machine_code(void) {
	abort();

	
	for (nat i = 0; i < ins_count; i++) {

		struct instruction this = ins[i];
		nat op = this.args[0];
		nat* a = this.args + 1;

		if (op == ec)   emitw(0x00000073);

		else if (op == add)     r_type(a, 0x33, 0x0, 0x00, this);
		else if (op == sub)     r_type(a, 0x33, 0x0, 0x20, this);
		else if (op == slt)     r_type(a, 0x33, 0x3, 0x00, this);
		
		
	
		else if (op == and_)    r_type(a, 0x33, 0x7, 0x00, this);
		else if (op == or_)     r_type(a, 0x33, 0x6, 0x00, this);
		else if (op == eor)     r_type(a, 0x33, 0x4, 0x00, this);
		else if (op == sl)      r_type(a, 0x33, 0x1, 0x00, this);
		else if (op == sr)      r_type(a, 0x33, 0x5, 0x00, this);
		
		
		

		else if (op == sl and is_signed)     r_type(a, 0x33, 0x2, 0x00, this);
		else if (op == sr and is_signed)     r_type(a, 0x33, 0x5, 0x20, this);

		else if (op == lb)     i_type(a, 0x03, 0x0, this);
		else if (op == lh)     i_type(a, 0x03, 0x1, this);
		else if (op == lw)     i_type(a, 0x03, 0x2, this);
		else if (op == ld)     i_type(a, 0x03, 0x3, this);

		//else if (op == addi)    i_type(a, 0x13, 0x0, this);
		//else if (op == sltis)   i_type(a, 0x13, 0x2, this);
		//else if (op == slti)    i_type(a, 0x13, 0x3, this);
		//else if (op == eori)    i_type(a, 0x13, 0x4, this);
		//else if (op == iori)    i_type(a, 0x13, 0x6, this);
		//else if (op == andi)    i_type(a, 0x13, 0x7, this);
		//else if (op == slli)    i_type(a, 0x13, 0x1, this);
		//else if (op == srli)    i_type(a, 0x13, 0x5, this);
		//else if (op == srai)    i_type(a, 0x13, 0x5, this);

		else if (op == cr)    i_type(a, 0x67, 0x0, this);

		else if (op == sb)      s_type(a, 0x23, 0x0, this);
		else if (op == sh)      s_type(a, 0x23, 0x1, this);
		else if (op == sw)      s_type(a, 0x23, 0x2, this);
		else if (op == sd)      s_type(a, 0x23, 0x3, this);

		// else if (op == lui)  u_type(a, 0x37, this);
		// else if (op == lpa)   u_type(a, 0x17, this);

		else if (op == cl)     j_type(i, a, 0x6F, this);

		else {
			snprintf(reason, sizeof reason, "riscv: unknown runtime instruction: \"%s\" (%llu)\n", spelling[op], op);
			print_message(error, reason, this.start[0], this.count[0]);
			exit(1);
		}
	}
	
}



#define CPU_SUBTYPE_ARM64_ALL 0
#define CPU_TYPE_ARM  12
#define CPU_ARCH_ABI64  0x01000000 
#define MH_MAGIC_64 0xfeedfacf
#define MH_SUBSECTIONS_VIA_SYMBOLS 0x2000
#define S_ATTR_PURE_INSTRUCTIONS 0x80000000
#define	N_SECT 0xe
#define	N_EXT 0x01
#define	LC_SYMTAB 0x2
#define	MH_OBJECT 0x1
#define REFERENCE_FLAG_DEFINED 2
#define VM_PROT_READ    0x01
#define VM_PROT_EXECUTE 0x04
#define	MH_NOUNDEFS	0x1
#define	LC_SEGMENT_64	0x19

struct nlist_64 {
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


static void make_macho_object_file(void) {

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
	segment.vmsize = byte_count;
	segment.filesize = byte_count;

	segment.fileoff = 	sizeof(struct mach_header_64) + 
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	struct section_64 section = {0};
	strncpy(section.sectname, "__text", 16);
	strncpy(section.segname, "__TEXT", 16);
	section.addr = 0;
	section.size = byte_count;	
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
				byte_count
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

	if (preserve_existing_object and not access(object_filename, F_OK)) {
		puts("asm: object_file: file exists"); 
		puts(object_filename);
		exit(1);
	}

	const int flags = O_WRONLY | O_CREAT | O_TRUNC | (preserve_existing_object ? O_EXCL : 0);
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(object_filename, flags, mode);
	if (file < 0) { perror("obj:open"); exit(1); }

	write(file, &header, sizeof(struct mach_header_64));
	write(file, &segment, sizeof (struct segment_command_64));
	write(file, &section, sizeof(struct section_64));
	write(file, &table, sizeof table);
	write(file, bytes, byte_count);
	write(file, symbols, sizeof(struct nlist_64));
	write(file, strings, sizeof strings);
	close(file);
}









	nat* def = calloc(ins_count * var_count, sizeof(nat));      memset(def, 255, sizeof(nat) * var_count * ins_count); 
	nat* type = calloc(ins_count * var_count, sizeof(nat));
	nat* value = calloc(ins_count * var_count, sizeof(nat)); 
	nat* is_copy = calloc(ins_count * var_count, sizeof(nat)); 
	nat* copy_of = calloc(ins_count * var_count, sizeof(nat));  
	nat* copy_type = calloc(ins_count * var_count, sizeof(nat));  

	nat stack[4096] = {0};
	nat stack_count = 1;
	nat visited[4096] = {0};
	nat last_pc = (nat) -1;

	nat bit_count[4096] = {0};
	nat register_index[4096] = {0};
	nat is_runtime[4096] = {0};

	memset(bit_count, 0xff, sizeof bit_count);
	memset(register_index, 0xff, sizeof register_index);


	while (stack_count) {
		nat pc = stack[--stack_count];

	execute_ins:
		if (pc >= ins_count) continue;
		nat pred_count = 0;
		nat* preds = compute_predecessors(ins, ins_count, pc, &pred_count);
		nat* gotos = compute_successors(ins, ins_count, pc);

		if ((0)) goto skip_printing;

		print_instruction_window_around(pc, ins, ins_count, variables, var_count, visited, 0, "");
		print_dictionary(variables, is_undefined, var_count);
		printf("        ");
		for (nat j = 0; j < var_count; j++) {
			printf("%3lld ", j);
		}
		puts("\n-------------------------------------------------");
		for (nat i = 0; i < ins_count; i++) {
			printf("ct %3llu: ", i);
			for (nat j = 0; j < var_count; j++) {
				if (type[i * var_count + j])
					printf("%3lld ", value[i * var_count + j]);
				else 	printf("\033[90m%3llx\033[0m ", value[i * var_count + j]);
			}
			putchar(9);

			printf("cp %3llu: ", i);
			for (nat j = 0; j < var_count; j++) {
				if (is_copy[i * var_count + j])
					printf("%3lld(%llu) ", copy_of[i * var_count + j], copy_type[i * var_count + j]);
				else 	printf("\033[90m%3llx(%llu)\033[0m ", copy_of[i * var_count + j], copy_type[i * var_count + j]);
			}
			putchar(9);

			printf("def %3llu: ", i);
			for (nat j = 0; j < var_count; j++) {
				if (def[i * var_count + j] != (nat) -1)
					printf("%3lld ", def[i * var_count + j]);
				else 	printf("\033[90m%3lld\033[0m ", def[i * var_count + j]);
			}
			puts("");
		}
		puts("-------------------------------------------------");
		printf("[PC = %llu, last_PC = %llu], pred:", pc, last_pc);
		print_nats(preds, pred_count); putchar(32);
		printf("stack: "); 
		print_nats(stack, stack_count); 
		putchar(10);
		getchar();

		skip_printing:;

		visited[pc]++;

		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;
		const nat gt0 = gotos[0];
		const nat gt1 = gotos[1];
		const nat a0 = 0 < arity[op] ? ins[pc].args[0] : 0;
		const nat a1 = 1 < arity[op] ? ins[pc].args[1] : 0;
		const nat a2 = 2 < arity[op] ? ins[pc].args[2] : 0;
		const nat i0 = !!(imm & 1);
		const nat i1 = !!(imm & 2);
		const nat i2 = !!(imm & 4);

		for (nat e = 0; e < var_count; e++) {

		const nat this_var = e; 
		nat future_type = 0;
		nat future_value = 0;
		nat value_mismatch = 0;
		
		bool at_least_one_pred = 0;
		for (nat iterator_p = 0; iterator_p < pred_count; iterator_p++) {
			const nat pred = preds[iterator_p];
			if (not visited[pred]) continue;

			if (ins[pred].op == lt or ins[pred].op == ge or ins[pred].op == ne or ins[pred].op == eq) {
				if (	 (not (ins[pred].imm & 1) and not type[pred * var_count + ins[pred].args[0]]) or 
					 (not (ins[pred].imm & 2) and not type[pred * var_count + ins[pred].args[1]])
				) at_least_one_pred = 1;
			}

			const nat t = type[pred * var_count + this_var];
			const nat v = value[pred * var_count + this_var];
			if (t == 0) { future_type = 0; future_value = v; break; }
			if (not future_type) { future_type = 1; future_value = v; } 
			else if (future_value != v) { value_mismatch = 1; } 
		}

		if (at_least_one_pred and value_mismatch) {
			if (pc < ins_count and last_pc != (nat) -1) { printf("last_pc = %llu\n", last_pc); puts("invalid internal state"); abort(); }
			future_type = 0;
		} else if (at_least_one_pred) {
			if (pc < ins_count and last_pc != (nat) -1) { printf("last_pc = %llu\n", last_pc); puts("invalid internal state"); abort(); } 
		}

		if (value_mismatch and future_type) {
			
			nat found = 0;
			for (nat i = 0; i < pred_count; i++) {
				if (preds[i] == last_pc) { found = 1; break; }
			}

			if (not found) {
				printf("for variable %llu:\"%s\", in instruction: ", this_var, variables[this_var]); 
				print_instruction(ins[pc], variables, var_count); putchar(10);
				printf("fatal error: in value mismatch: last_pc was not a predecessor"
					" (last_pc = %llu, lastpc not found in predecessors)... aborting.\n", 
					last_pc
				); 
				getchar();
				future_type = 0;
				abort();
				goto done_with_mismatch;
			}
			if (last_pc == (nat) -1) { puts("error: valuemismatch: last_pc == -1"); abort(); }
			const nat pred = last_pc;
			if (not visited[pred]) continue; 
			const nat t = type[pred * var_count + this_var];
			const nat v = value[pred * var_count + this_var];
			future_type = t;
			future_value = v;
			done_with_mismatch:;
		}

		type[pc * var_count + this_var] = future_type;
		value[pc * var_count + this_var] = future_value;

		nat future_is_copy = 0;
		nat future_copy_ref = 0;
		nat future_copy_type = 0;
		nat copy_ref_mismatch = 0;

		for (nat iterator_p = 0; iterator_p < pred_count; iterator_p++) {
			const nat pred = preds[iterator_p];
			if (not visited[pred]) continue;
			const nat t = is_copy[pred * var_count + this_var];
			const nat v = copy_of[pred * var_count + this_var];
			const nat ct = copy_type[pred * var_count + this_var];
			if (t == 0) { future_is_copy = 0; future_copy_ref = 0; future_copy_type = 0; break; }
			if (not future_is_copy) { 
				future_is_copy = 1; 
				future_copy_ref = v;
				future_copy_type = ct;
			} else if (future_copy_ref != v or future_copy_type != ct) copy_ref_mismatch = 1; 
		}


		if (copy_ref_mismatch) {
			printf("copy_ref MISMATCH!\n");
			abort();



			comment begin    nat found = 0;
			for (nat i = 0; i < pred_count; i++) {
				if (preds[i] == last_pc) { found = 1; break; }
			}
			if ((true) or found) {
				printf("for variable %llu:\"%s\", in instruction: ", this_var, variables[this_var]); 
				print_instruction(ins[pc], variables, var_count); putchar(10);
				printf("fatal error: in value mismatch: last_pc was not a predecessor"
					" (last_pc = %llu, lastpc not found in predecessors)... aborting.\n", 
					last_pc
				);
				abort();
			} 
			//if (last_pc == (nat) -1) abort();
			const nat pred = last_pc;
			if (not visited[pred]) continue; 
			const nat t = type[pred * var_count + this_var];
			const nat v = value[pred * var_count + this_var];
			future_type = t;
			future_value = v;
			done_with_copy_mismatch:;   comment end
		}

		is_copy[pc * var_count + this_var] = future_is_copy;
		copy_of[pc * var_count + this_var] = future_copy_ref;
		copy_type[pc * var_count + this_var] = future_copy_type;

		nat future_def = (nat) -1;

		for (nat iterator_p = 0; iterator_p < pred_count; iterator_p++) {
			const nat pred = preds[iterator_p];
			if (not visited[pred]) continue;
			const nat d = def[pred * var_count + this_var];
			if (future_def == (nat) -1) future_def = d;
			else if (future_def == d) { }
			else { puts("fatal error: mismatch in definition index: "
				"unable to track location of defintion for this variable... aborting..."); abort(); }
		}

		def[pc * var_count + this_var] = future_def;
		} // for e 

		const nat a0_def = def[pc * var_count + a0];

		const nat ct0 = i0 or type[pc * var_count + a0];
		const nat ct1 = i1 or type[pc * var_count + a1];
		const nat ct2 = i2 or type[pc * var_count + a2];
		const nat v0 = i0 ? a0 : value[pc * var_count + a0];
		const nat v1 = i1 ? a1 : value[pc * var_count + a1];
		//const nat v2 = i2 ? a2 : value[pc * var_count + a2];

		const nat a0_is_copy = is_copy[pc * var_count + a0];
		//const nat a1_is_copy = is_copy[pc * var_count + a1];
		const nat a0_copy_ref = copy_of[pc * var_count + a0];
		//const nat a1_copy_ref = copy_of[pc * var_count + a1];
		const nat a0_copy_type = copy_type[pc * var_count + a0];
		//const nat a1_copy_type = copy_type[pc * var_count + a1];

		nat out_t = ct0, out_v = v0;

		nat 	out_is_copy = a0_is_copy, out_def = a0_def,
			out_copy_ref = a0_copy_ref, 
			out_copy_type = a0_copy_type
		;
		last_pc = pc;

		if (op == halt) { last_pc = (nat) -1; continue; }
		else if (op == at) { }
		else if (op == sc) { }
		else if (op == do_) { pc = gt0; goto execute_ins; }
		else if (op == set) {
			if (not is_runtime[a0]) {
				out_t = ct1; out_v = v1;
			}
			out_def = pc;
			out_is_copy = 1;
			if (not i1 and is_copy[pc * var_count + a1]) {
				out_copy_ref = copy_of[pc * var_count + a1];
				out_copy_type = copy_type[pc * var_count + a1];
			} else { 
				out_copy_ref = a1;
				out_copy_type = i1;
			}
		}
		else if (op >= add and op <= sd) {
			if (not i1) out_t = ct0 and ct1;

			     if (op == add)  out_v += v1;
			else if (op == sub)  out_v -= v1;
			else if (op == mul)  out_v *= v1;
			else if (op == div_) out_v /= v1;
			else if (op == rem)  out_v %= v1;
			else if (op == and_) out_v &= v1;
			else if (op == or_)  out_v |= v1;
			else if (op == eor)  out_v ^= v1;
			else if (op == si)   out_v <<= v1;
			else if (op == sd)   out_v >>= v1;
			out_is_copy = 0;
			
		} else if (op == st) {	
			if (not ct2) { puts("error: size of store must be ctk."); abort(); }
			out_is_copy = 0;

		} else if (op == ld) {
			if (not ct2) { puts("error: size of load must be ctk."); abort(); }
			out_t = 0;
			out_is_copy = 0;

		} else if (op == la) {
			out_t = 0;
			out_v = 0;
			out_is_copy = 0;
						
		} else if (op == lt or op == ge or op == ne or op == eq) {
			if (not ct0 or not ct1) {
				last_pc = (nat) -1;
				if (gt0 < ins_count and visited[gt0] < 2) { 
					stack[stack_count++] = gt0; 
				}
				if (gt1 < ins_count and visited[gt1] < 2) { 
					stack[stack_count++] = gt1; 
				}
				printf("executing top of stack...\n"); getchar();
				continue;
			} else {
				bool cond = 0;
				     if (op == lt) cond = v0  < v1;
				else if (op == ge) cond = v0 >= v1;
				else if (op == eq) cond = v0 == v1;
				else if (op == ne) cond = v0 != v1;
				pc = cond ? gt1 : gt0; goto execute_ins;
			}

		} else if (op == rt) {
			if (not ct1) { puts("error: source argument to RT must be compiletime known."); abort(); }

			printf("note: storing: RT x (%lld)\n", v1);
			if ((int64_t) v1 < 0) { puts("STORING REGSITER INDEX"); register_index[a0]  = -v1; }
			else { puts("STORING BIT COUNT"); bit_count[a0]  = v1; }
			getchar();

			is_runtime[a0] = 1;
			out_t = 0;

		} else {
			puts("WARNING: EXECUTING AN UNKNOWN INSTRUCTION WITHOUT AN IMPLEMENTATION!!!");
			puts(operations[op]);
			puts("note this!!!"); getchar();
			abort();
		}

		def[pc * var_count + a0] = out_def;
		type[pc * var_count + a0] = out_t;
		value[pc * var_count + a0] = out_v;
		is_copy[pc * var_count + a0] = out_is_copy;
		copy_of[pc * var_count + a0] = out_copy_ref;
		copy_type[pc * var_count + a0] = out_copy_type;
		pc++; goto execute_ins;
	}


	puts("...done executing code. visited: ");
	print_nats(visited, ins_count);
	puts("");
	getchar();

	print_instruction_window_around(0, ins, ins_count, variables, var_count, visited, 0, "");
	print_instructions(ins, ins_count, variables, var_count, 1);
	print_dictionary(variables, is_undefined, var_count);
	printf("     ");
	for (nat j = 0; j < var_count; j++) {
		printf("%3lld ", j);
	}
	puts("\n-------------------------------------------------");
	for (nat i = 0; i < ins_count; i++) {
			printf("ct %3llu: ", i);
			for (nat j = 0; j < var_count; j++) {
				if (type[i * var_count + j])
					printf("%3lld ", value[i * var_count + j]);
				else 	printf("\033[90m%3llx\033[0m ", value[i * var_count + j]);
			}
			putchar(9);

			printf("cp %3llu: ", i);
			for (nat j = 0; j < var_count; j++) {
				if (is_copy[i * var_count + j])
					printf("%3lld(%llu) ", copy_of[i * var_count + j], copy_type[i * var_count + j]);
				else 	printf("\033[90m%3llx(%llu)\033[0m ", copy_of[i * var_count + j], copy_type[i * var_count + j]);
			}
			putchar(9);

			printf("def %3llu: ", i);
			for (nat j = 0; j < var_count; j++) {
				if (def[i * var_count + j] != (nat) -1)
					printf("%3lld ", def[i * var_count + j]);
				else 	printf("\033[90m%3lld\033[0m ", def[i * var_count + j]);
			}
			puts("");
	}
	puts("-------------------------------------------------");

	printf("[PC = %llu, last_PC = %llu], pred:", 0LLU, 0LLU);
	putchar(32);
	printf("stack: "); 
	print_nats(stack, stack_count); 
	putchar(10);

	puts("...beginning to simplify instructions now!");
	getchar();


	for (nat i = 0; i < ins_count; i++) {

		//print_instruction_window_around(i, ins, ins_count, variables, var_count, visited, 1, "PRUNING CTK INS: on this ins");
		//getchar();


		print_instruction_window_around(i, ins, ins_count, variables, var_count, visited, 0, "");
		print_dictionary(variables, is_undefined, var_count);
		puts("-----------PRUNING CTK INS:---------------");
		getchar();


		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		
		nat ignore = 1;

		if (	op == halt or op == sc or 
			op == do_  or op == at or op == la or 
			op == ld   or op == st
		) ignore = 0;

		//puts("looking at: "); 
		//print_instruction(ins[i], variables, var_count); puts("");
		
		if (ignore and op != rt) 
		for (nat a = 0; a < arity[op]; a++) {
			if (op == at and a == 0) continue;
			if (op == do_ and a == 0) continue;
			if (op == lt and a == 2) continue;
			if (op == ge and a == 2) continue;
			if (op == ne and a == 2) continue;
			if (op == eq and a == 2) continue;
			if (op == la and a == 1) continue; 
						

			if (((imm >> a) & 1)) {

				printf("found a compiletime immediate : %llu\n", 
					ins[i].args[a]
				);

			} else if (type[i * var_count + ins[i].args[a]]) {
				
				printf("found a compiletime variable "
					"as argument  :  %s = {type = %llu, value = %llu}\n",
					variables[ins[i].args[a]], 
					type[i * var_count + ins[i].args[a]],
					value[i * var_count + ins[i].args[a]]
				);

			} else {
				puts("found a runtime argument!");	
				printf("found variable "
					":  %s = {type = %llu, value = %llu}\n",
					variables[ins[i].args[a]], 
					type[i * var_count + ins[i].args[a]],
					value[i * var_count + ins[i].args[a]]
				);
				ignore = 0; break;
			}
		}

		if (ignore or not visited[i]) {

			if (op == lt or op == ge or op == ne or op == eq) {
				const nat v0 = (imm & 1) ? ins[i].args[0] : value[i * var_count + ins[i].args[0]];
				const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];
				bool cond = 0;
				     if (op == lt) cond = v0  < v1;
				else if (op == ge) cond = v0 >= v1;
				else if (op == eq) cond = v0 == v1;
				else if (op == ne) cond = v0 != v1;
				if (cond) { ins[i].op = do_; ins[i].args[0] = ins[i].args[2]; }
				else visited[i] = 0;

			} else { puts("NOTE: found a compiletime-known instruction! deleting this instruction."); getchar(); visited[i] = 0; } 

		} else {
			puts("found real RT isntruction!"); putchar('\t');
			print_instruction(ins[i], variables, var_count); puts("");
			
			if ((op >= set and op <= sd) or op == rt) {
				const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
				const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

				if (ct1 and not (imm & 2)) {
					//puts("found an instruction which has a ct argument!");
					ins[i].args[1] = v1;
					ins[i].imm |= 2;
				}

				if (ins[i].imm & 2) {
					const nat c = ins[i].args[1];
					if (	(op == add or op == sub or
						 op == si or op == sd or
						 op == eor or op == or_)
						and not c or 
						(op == mul or op == div_)
						and c == 1	
					) { puts("found a NOP! deleting this instruction."); getchar(); visited[i] = 0; } 

					for (nat sh = 0; sh < 64; sh++) {
						if (op == mul and c == (1LLU << sh)) {
							ins[i].op = si;
							ins[i].args[1] = sh;
							break;

						} else if (op == div_ and c == (1LLU << sh)) {
							ins[i].op = sd;
							ins[i].args[1] = sh;
							break;
						} 
					}

				} else {
					if (ins[i].args[0] == ins[i].args[1]) {
						if (op == set or op == and_ or op == or_) { 
							puts("found a rt NOP! deleting this instruction."); 
							getchar(); visited[i] = 0; }
						else if (op == eor or op == sub) {
							ins[i].op = set;
							ins[i].imm |= 2;
							ins[i].args[1] = 0;
						}
					}
				}

				if (not (ins[i].imm & 2) and is_copy[i * var_count + ins[i].args[1]]) { 

					printf("note: inlining copy reference: a1=%llu imm=%llu copy_of=%llu, copy_type=%llu, i=%llu...\n", 
						ins[i].args[1], ins[i].imm, 
						copy_of[i * var_count + ins[i].args[1]], 
						copy_type[i * var_count + ins[i].args[1]],
						i
					);

					puts("original:");
					print_instruction(ins[i], variables, var_count); puts("");
					getchar();

					if (copy_type[i * var_count + ins[i].args[1]]) ins[i].imm |= 2;
					ins[i].args[1] = copy_of[i * var_count + ins[i].args[1]];

					puts("modified form:"); print_instruction(ins[i], variables, var_count); puts("");
					getchar();
				}

			} else if (op == ld or op == st) {
				puts("we still need to embed the immediates into the load and store instructions.");
				abort();	

			} else if (op == lt or op == ge or op == ne or op == eq) {
				//puts("we still need to embed the immediates into the conditioal branch instructions.");

				const nat ct0 = (imm & 1) or type[i * var_count + ins[i].args[0]];
				const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
				const nat v0 = (imm & 1) ? ins[i].args[0] : value[i * var_count + ins[i].args[0]];
				const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

				if (ct0 and not (imm & 1)) {
					ins[i].args[0] = v0;
					ins[i].imm |= 1;

				} else if (ct1 and not (imm & 2)) {
					ins[i].args[1] = v1;
					ins[i].imm |= 2;
				}
			}
		}

		getchar();
	}


	puts("");
	puts(" ---------------------------------- ON REMATERIALIZATION NOW ------------------------------");
	puts("");

	// rematerialization: 

	for (nat pc = 0; pc < ins_count; pc++) {

		if (not visited[pc]) continue;

		//print_instruction_window_around(pc, ins, ins_count, variables, var_count, visited, 1, "REMAT: on this ins");

		print_instruction_window_around(pc, ins, ins_count, variables, var_count, visited, 0, "");
		print_dictionary(variables, is_undefined, var_count);
		puts("-----------REMAT:---------------");
		getchar();

		const nat op = ins[pc].op;
		//const nat imm = ins[pc].imm;

		if (op == set) { puts("skipping all sets...\n"); continue; }

		nat pred_count = 0;
		nat* preds = compute_predecessors(ins, ins_count, pc, &pred_count);
		//nat* gotos = compute_successors(ins, ins_count, pc);
		
		for (nat e = 0; e < var_count; e++) {
			const nat this_var = e;
			printf("ON VARIABLE %s\n", variables[this_var]);

			for (nat iterator_p = 0; iterator_p < pred_count; iterator_p++) {
				const nat pred = preds[iterator_p];
				const nat t = type[pred * var_count + this_var];
				const nat v = value[pred * var_count + this_var];
	
				printf("RESULTS pred %llu  ", pred);
				printf("	ct_ness = %llu   ", t);
				printf("	ct_value = %llu   ", v);
				printf("	value_mismatch = %llu  ", 0LLU);
				puts("");
				if (t == 1) { 
					printf("note: found CTK predecessor, with value %llu... setting def to setimm ctkv\n", v); 
					if (not type[pc * var_count + e]) { 

						const nat setimm_defined_on = def[pred * var_count + this_var];
						printf("setimm_defined_on = %llu\n", setimm_defined_on);
						//const nat ctk_value = 0;
						puts("on one of the pred exec paths, the variable was compiletime, "
						"but on this instruction, it is now RTK, but not via a set..");
						puts("this means that we need to materialize the variable:");
						puts(variables[this_var]);
						
						visited[setimm_defined_on] = 1;
						struct instruction this_def = ins[setimm_defined_on];
						if (this_def.op != set) abort();

						ins[setimm_defined_on].args[1] = v;
						printf("reassigned the initial value for ctk setimm def at ins[%llu] to value #%llu\n", 
							setimm_defined_on, v
						); 
						puts("was:"); print_instruction(this_def, variables, var_count); puts("");
						puts("now is:"); print_instruction(ins[setimm_defined_on], variables, var_count); puts("");
						getchar();
					} else puts("NA, not RTK dest");
				}
			}
			puts("done with var");
			//getchar();
		}
		puts("[done]");
		getchar();
	}




	{ nat final_ins_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		//const nat op = ins[i].op;
		if (not visited[i]) continue;
		ins[final_ins_count++] = ins[i];
	}
	ins_count = final_ins_count;
	memset(visited, 255, sizeof visited); }









	for (nat pc = 0; pc < ins_count; pc++) {
		if (not visited[pc]) continue;
		//print_instruction_window_around(pc, ins, ins_count, variables, var_count, visited, 1, "CF SIMPLIFY: on this ins");

		print_instruction_window_around(pc, ins, ins_count, variables, var_count, visited, 0, "");
		print_dictionary(variables, is_undefined, var_count);
		puts("-----------CF SIMPLIFY:---------------");
		getchar();

		const nat op = ins[pc].op;

		nat pred_count = 0;
		nat* preds = compute_predecessors(ins, ins_count, pc, &pred_count);
		nat* gotos = compute_successors(ins, ins_count, pc);

		if (pred_count < 2 and op == at) { visited[pc] = 0; continue; } else { puts("KEEPING AT INSTRUCTION!!!");  getchar(); }

		if (op == do_) {
			nat count = 0;
			compute_predecessors(ins, ins_count, gotos[0], &count);
			if (count < 2 and gotos[0] == pc + 1) {
				visited[pc] = 0; continue;
			}
		}

		puts("[done]");
		getchar();
	}











           // first attempt at algebraic simplification: 


	for (nat pc = 0; pc < ins_count; pc++) {

		if (not visited[pc]) continue;
		
		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;

		if ((op == add or op == sub) and imm) {

			if (pc + 1 < ins_count and 
				(ins[pc + 1].op == add or 
				 ins[pc + 1].op == sub) 
				and ins[pc + 1].imm and 
				ins[pc + 1].args[0] == ins[pc].args[0]
			) {

				const nat second_op = ins[pc + 1].op;

				visited[pc + 1] = 0;

				const nat a = ins[pc + 0].args[1];
				const nat b = ins[pc + 1].args[1];

				printf("note: combining addi instructions!\n");
				printf("a = %llu, b = %llu\n", a, b);

				if ((0)) {}
				else if (op == sub and second_op == sub) ins[pc].args[1] = a + b;
				else if (op == add and second_op == add) ins[pc].args[1] = a + b;

				else if (op == add and second_op == sub) { 

					if (a < b) { 
						ins[pc].op = sub; 
						ins[pc].args[1] = b - a; 

					} else if (a > b) { 
						ins[pc].op = sub; 
						ins[pc].args[1] = a - b;
					} else {
						visited[pc] = 0;
					}

				} else if (op == sub and second_op == add) { 

					if (a < b) { 
						ins[pc].op = add; 
						ins[pc].args[1] = b - a; 

					} else if (a > b) {
						ins[pc].op = sub; 
						ins[pc].args[1] = a - b;
					} else {
						visited[pc] = 0;
					}

				else { abort(); }
			}

		}

	}











	{ nat final_ins_count = 0;

	for (nat i = 0; i < ins_count; i++) {
		//const nat op = ins[i].op;
		if (not visited[i]) continue;
		ins[final_ins_count++] = ins[i];
	}

	ins_count = final_ins_count;

	memset(visited, 255, sizeof visited);
	}


	for (nat i = 0; i < ins_count; i++) {
		if (not visited[i]) continue;
		//const nat op = ins[i].op;


	}













	print_dictionary(variables, is_undefined, var_count);
	print_instructions(ins, ins_count, variables, var_count, 0);
	puts("[done]");

	print_nats(is_runtime, var_count); puts("");
	print_nats(bit_count, var_count); puts("");
	print_nats(register_index, var_count); puts("");

	getchar();






















	// --------------------------------------------------------------------------------------
	// once we finish with copy prop and it works well, we should do ins sel for msp430!!!
	// --------------------------------------------------------------------------------------

	a6_nop, a6_svc, a6_mov, a6_bfm,
	a6_adc, a6_addx, a6_addi, a6_addr, a6_adr, 
	a6_shv, a6_clz, a6_rev, a6_jmp, a6_bc, a6_br, 
	a6_cbz, a6_tbz, a6_ccmp, a6_csel, 
	a6_ori, a6_orr, a6_extr, a6_ldrl, 
	a6_memp, a6_memia, a6_memi, a6_memr, 
	a6_madd, a6_divr, 


	
	struct instruction mi[4096] = {0};
	nat mi_count = 0;
	nat selected[4096] = {0};

	puts("ins sel: starting instruction selection now!");

	for (nat i = 0; i < ins_count; i++) {

		print_instruction_window_around(i, ins, ins_count, variables, var_count, selected, 0, "");
		getchar();

		if (selected[i]) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i); 
			getchar();
			continue;
		}

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1]; 

		if (op == halt) { mi[mi_count++] = ins[i]; selected[i] = 1; continue; }
		if (op == at) { mi[mi_count++] = ins[i]; selected[i] = 1; continue; }

		if (op == set) {
			const nat b = locate_instruction(
				(struct expected_instruction){ .op = si, .imm = 2, .use = 1, .args[0] = arg0 },
				i + 1, ins, ins_count
			);
			if (b == (nat) -1) goto addsrlsl_bail;

			const nat c = locate_instruction(
				(struct expected_instruction){ .op = add, .use = 1, .args[0] = arg0 },
				b + 1, ins, ins_count
			);
			if (c == (nat) -1) goto addsrlsl_bail;

			struct instruction new = { .op = a6_addr, .imm = 0xf8 };
			new.args[0] = arg0; 		// d
			new.args[1] = ins[c].args[1]; 	// n
			new.args[2] = arg1; 		// m 
			new.args[3] = ins[b].args[1]; 	// k
			new.args[4] = 0; //???sb?
			new.args[5] = 0; // ????sf???
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1; selected[c] = 1;
			goto finish_mi_instruction;
		} addsrlsl_bail:

		if (op == set and not imm) {
			struct instruction new = { .op = a6_orr, .imm = 0xff };
			new.args[0] = arg0;
			new.args[1] = 0;
			new.args[2] = arg1;			
			new.args[3] = 0;
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

		else if (op == set and imm) {
			struct instruction new = { .op = a6_mov, .imm = 0xfe };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = 0;
			new.args[3] = 0;
			new.args[4] = 0;
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

		if (op == lt and not imm) {
			struct instruction new = { .op = a6_addr, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = lt;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == eq and not imm) {
			struct instruction new = { .op = a6_orr, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = eq;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == lt and imm) {			
			struct instruction new = { .op = a6_addi, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = lt;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == eq and imm) {
			struct instruction new = { .op = a6_ori, .imm = 0xff };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = a6_bc, .imm = 0xff };
			new2.args[0] = eq;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == sc) {
			struct instruction new = { .op = a6_svc, .imm = 0xff };
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

		
	finish_mi_instruction:
		puts("so far:");
		print_instructions(mi, mi_count, variables, var_count, 0);
		getchar();

	}



	puts("preliminary:");
	print_dictionary(variables, is_undefined, var_count);
	print_instructions(mi, mi_count, variables, var_count, 0);

	for (nat i = 0; i < ins_count; i++) {
		if (not selected[i]) {

			puts("error: instruction unprocessed by ins sel: internal error");
			puts("error: this instruction failed to be lowered:\n");
			print_instruction_window_around(i, ins, ins_count, variables, var_count, selected, 1, "not selected instruction!");
			puts("");
			abort();
		}
	}

	print_dictionary(variables, is_undefined, var_count);
	print_instructions(mi, mi_count, variables, var_count, 0);
	puts("finished instruction selection!");











// ---------------------------------- machine code generation ---------------------------------


//enum all_architectures { no_arch, arm64_arch, arm32_arch, rv64_arch, rv32_arch, msp430_arch, };
//enum all_output_formats { debug_output_only, macho_executable, elf_executable, ti_txt_executable };


	const nat target_arch = rv64_arch;
	const nat output_format = debug_output_only;
	const nat should_overwrite = true;

	printf("info: assemblying for [target = %llu, output = %llu (%s)]\n", 
		target_arch, output_format, 
		should_overwrite ? "overwrite" : "non-destructive"
	);

	if (not target_arch) exit(0);

	uint8_t* my_bytes = NULL;
	nat my_count = 0;




#define max_section_count 128
	nat section_count = 0;
	nat section_starts[max_section_count] = {0};
	nat section_addresses[max_section_count] = {0};




if (target_arch == rv32_arch) {


	nat* lengths = calloc(ins_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		if (rt_ins[i].op == halt) continue;
		lengths[i] = ins[i].op == emit ? rt_ins[i].args[0] : 4;
	}

	print_nats(lengths, ins_count);


	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		const u16 a0 = (u16) rt_ins[i].args[0];
		const u16 a1 = (u16) ins[i].args[1];
		const u16 a2 = (u16) ins[i].args[2];
		const u16 a3 = (u16) ins[i].args[3];
		const u16 a4 = (u16) ins[i].args[4];
		const u16 a5 = (u16) ins[i].args[5];
		const u16 a6 = (u16) ins[i].args[6];
		const u16 a7 = (u16) ins[i].args[7];

	
		if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (nat) a1);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (u32) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (u16) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (byte) a1);
		}

		else if (op == r5_i) {
			//const u16 offset = 0x3FF & ((calculate_offset(lengths, i + 1, a1) >> 1));
			//const u16 word = (u16) ((1U << 13U) | (u16)(a0 << 10U) | (offset));
			//insert_u16(&my_bytes, &my_count, word);			
		}

		else if (op == r5_r) {

		}

		else if (op == r5_r) {

		}

		else if (op == r5_r) {

		}

		else if (op == r5_b) {  

			// gen4  op(0)  dm(1) dr(2) di(3)  sm(4) sr(5) si(6)   size(7)
			//u16 word = (u16) (
			//	(a0 << 12U) | (a5 << 8U) | (a1 << 7U) |
			//	(a7 << 6U) | (a4 << 4U) | (a2)
			//);
			//insert_u16(&my_bytes, &my_count, word);
			//if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
			//	or (a4 == 3 and not a5)) insert_u16(&my_bytes, &my_count, a6);
			//if (a1 == 1) insert_u16(&my_bytes, &my_count, a3);
		}
		else if (op == halt) {}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}	






} else if (target_arch == msp430_arch) {

	nat* lengths = calloc(rt_ins_count, sizeof(nat));
	for (nat i = 0; i < rt_ins_count; i++) {
		const nat op = rt_ins[i].op;
		const u32 a0 = (u32) rt_ins[i].args[0];
		const u32 a1 = (u32) rt_ins[i].args[1];
		const u32 a4 = (u32) rt_ins[i].args[4];
		const u32 a5 = (u32) rt_ins[i].args[5];

		nat len = 0;
		if (op == section_start) len = 0;
		else if (op == halt) len = 0;
		else if (op == emit and a0 == 1) len = 1;
		else if (op == emit and a0 == 2) len = 2;
		else if (op == emit and a0 == 4) len = 4;
		else if (op == emit and a0 == 8) len = 8;
		else if (op == branch4) len = 2;
		else if (op == general4) {
			len = 2;
			if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
				or (a4 == 3 and not a5)) len += 2;						
			if (a1 == 1) len += 2;
		}
		lengths[i] = len;
	}

	print_nats(lengths, rt_ins_count);

	for (nat i = 0; i < rt_ins_count; i++) {
		const nat op = rt_ins[i].op;
		const u16 a0 = (u16) rt_ins[i].args[0];
		const u16 a1 = (u16) rt_ins[i].args[1];
		const u16 a2 = (u16) rt_ins[i].args[2];
		const u16 a3 = (u16) rt_ins[i].args[3];
		const u16 a4 = (u16) rt_ins[i].args[4];
		const u16 a5 = (u16) rt_ins[i].args[5];
		const u16 a6 = (u16) rt_ins[i].args[6];
		const u16 a7 = (u16) rt_ins[i].args[7];

		if (op == section_start) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;
		}
		else if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (nat) a1);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (u32) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (u16) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (byte) a1);
		}
		else if (op == branch4) {
			const u16 offset = 0x3FF & ((calculate_offset(lengths, i + 1, a1) >> 1));
			const u16 word = (u16) ((1U << 13U) | (u16)(a0 << 10U) | (offset));
			insert_u16(&my_bytes, &my_count, word);
		}
		else if (op == general4) {  
			// gen4  op(0)  dm(1) dr(2) di(3)  sm(4) sr(5) si(6)   size(7)
			u16 word = (u16) (
				(a0 << 12U) | (a5 << 8U) | (a1 << 7U) | 
				(a7 << 6U) | (a4 << 4U) | (a2)
			);
			insert_u16(&my_bytes, &my_count, word);

			if ((a4 == 1 and (a5 != 2 and a5 != 3)) 
				or (a4 == 3 and not a5)) insert_u16(&my_bytes, &my_count, a6);
						
			if (a1 == 1) insert_u16(&my_bytes, &my_count, a3);
		}				
		else if (op == halt) {}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}	
	
} else if (target_arch == arm64_arch) {

	nat* lengths = calloc(rt_ins_count, sizeof(nat));
	for (nat i = 0; i < rt_ins_count; i++) {
		if (rt_ins[i].op == halt) continue;
		lengths[i] = rt_ins[i].op == emit ? rt_ins[i].args[0] : 4;
	}

	print_nats(lengths, rt_ins_count);

	for (nat i = 0; i < rt_ins_count; i++) {
		const nat op = rt_ins[i].op;
		const u32 a0 = (u32) rt_ins[i].args[0];
		const u32 a1 = (u32) rt_ins[i].args[1];
		const u32 a2 = (u32) rt_ins[i].args[2];
		const u32 a3 = (u32) rt_ins[i].args[3];
		const u32 a4 = (u32) rt_ins[i].args[4];
		const u32 a5 = (u32) rt_ins[i].args[5];
		const u32 a6 = (u32) rt_ins[i].args[6];
		const u32 a7 = (u32) rt_ins[i].args[7];

		if (op == emit) {
			if (a0 == 8) insert_u64(&my_bytes, &my_count, (uint64_t) a1);
			if (a0 == 4) insert_u32(&my_bytes, &my_count, (uint32_t) a1);
			if (a0 == 2) insert_u16(&my_bytes, &my_count, (uint16_t) a1);
			if (a0 == 1) insert_u8 (&my_bytes, &my_count, (uint8_t) a1);
		}
		else if (op == nop) insert_u32(&my_bytes, &my_count, 0xD503201F);
		else if (op == svc) insert_u32(&my_bytes, &my_count, 0xD4000001);
		else if (op == br) { // 
			uint32_t l = a2?2:a1?1:0;
			const uint32_t to_emit = 
				(0x6BU << 25U) | (l << 21U) | 
				(0x1FU << 16U) | (a0 << 5U);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == adc) { // 
			const uint32_t to_emit = 
				(a5 << 31U) | (a4 << 30U) | (a3 << 29U) | 
				(0xD0 << 21U) | (a2 << 16U) | (0 << 19U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == shv) {
			uint32_t op2 = 8;
			if (a3 == 0) op2 = 8;
			if (a3 == 1) op2 = 9;
			if (a3 == 2) op2 = 10;
			if (a3 == 3) op2 = 11;
			const uint32_t to_emit = 
				(a4 << 31U) | (0 << 30U) | 
				(0 << 29U) | (0xD6 << 21U) | 
				(a2 << 16U) | (op2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == mov) {
			const uint32_t to_emit = 
				(a4 << 31U) | (a3 << 29U) | (0x25U << 23U) | 
				(a2 << 21U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == bc) {
			const uint32_t offset = 0x7ffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = 
				(0x54U << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == jmp) {
			const uint32_t offset = 0x3ffffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = (a0 << 31U) | (0x5U << 26U) | (offset);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == adr) {
			uint32_t o1 = a2;
			nat count = calculate_offset(lengths, i, a1);
			if (a2) count /= 4096;
			const uint32_t offset = 0x1fffff & count;
			const uint32_t lo = offset & 3, hi = offset >> 2;
			const uint32_t to_emit = 
				(o1 << 31U) | (lo << 29U) | (0x10U << 24U) |
				(hi << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == cbz) {
			const uint32_t offset = 0x7ffff & (calculate_offset(lengths, i, a1) >> 2);
			const uint32_t to_emit = 
				(a3 << 31U) | (0x1AU << 25U) | 
				(a2 << 24U) | (offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == tbz) {
			const uint32_t b40 = a1 & 0x1F;
			const uint32_t b5 = a1 >> 5;
			const uint32_t offset = 0x3fff & (calculate_offset(lengths, i, a2) >> 2);
			const uint32_t to_emit = 
				(b5 << 31U) | (0x1BU << 25U) | (a3 << 24U) |
				(b40 << 19U) |(offset << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == ccmp) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a4 << 30U) | (0x1D2 << 21U) | 
				(a3 << 16U) | (a0 << 12U) | (a2 << 11U) | 
				(a1 << 5U) | (a5); 
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == addi) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a5 << 30U) | (a4 << 29U) | 
				(0x22 << 23U) | (a3 << 22U) | (a2 << 10U) |
				(a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == addr) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0xB << 24U) | (a3 << 22U) | (a2 << 16U) |
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == addx) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a6 << 30U) | (a5 << 29U) | 
				(0x59 << 21U) | (a2 << 16U) | (a3 << 13U) | 
				(a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == divr) {
			const uint32_t to_emit = 
				(a4 << 31U) | (0xD6 << 21U) | (a2 << 16U) |
				(1 << 11U) | (a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);	
		} else if (op == csel) {
			const uint32_t to_emit = 
				(a6 << 31U) | (a5 << 30U) | (0xD4 << 21U) | 
				(a2 << 16U) | (a3 << 12U) | (a4 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == madd) {
			const uint32_t to_emit = 
				(a7 << 31U) | (0x1B << 24U) | (a5 << 23U) | 
				(a4 << 21U) | (a2 << 16U) | (a6 << 15U) | 
				(a3 << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == bfm) {
			u32 imms = 0, immr = 0;
			if (not a2) { imms = a3 + a4 - 1; immr = a3; } 
			else { imms = a4 - 1; immr = (a6 ? 64 : 32) - a3; }
			const uint32_t to_emit = (a6 << 31U) | (a5 << 29U) | 	
				(0x26U << 23U) | (a6 << 22U) | (immr << 16U) |
				(imms << 10U) | (a1 << 5U) | (a0);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == ori) {


			puts("TODO: please implemented the ori instruction: "
				"this is the last instruction we need to implement "
				"and then we are done with iplemementing the arm64 backend!"
			);

			abort();


		} else if (op == orr) {
			const uint32_t to_emit = 
				(a7 << 31U) | (a0 << 29U) | (10 << 24U) | 
				(a4 << 22U) | (a6 << 21U) | (a3 << 16U) | 
				(a5 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);

		} else if (op == memp) {
			const uint32_t to_emit = 
				(a1 << 30U) | (0x14 << 25U) | (a6 << 23U) | (a0 << 22U) | 
				(a5 << 15U) | (a3 << 10U) | (a4 << 5U) | (a2);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == memi) {
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a4 << 30U) | (0x39 << 24U) | (opc << 22U) |
				(a3 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == memia) { 			
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a4 == 3) opc = 1;
			else if (a4 == 2 and is_signed) opc = 2;
			else if (a4 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a4 << 30U) | (0x38 << 24U) | (opc << 22U) | (a3 << 12U) | 
				(a5 << 11U) | (1 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		} else if (op == memr) { 
			const u32 S = (a4 >> 2) & 1, option = a4 & 3;
			u32 opt = 0;
			if (option == 0) opt = 2;
			else if (option == 1) opt = 3;
			else if (option == 2) opt = 6;
			else if (option == 3) opt = 7;
			else abort();				
			const u32 is_load = (a0 >> 2) & 1;
			const u32 is_signed = (a0 >> 1) & 1;
			const u32 is_64_dest = (a0 >> 0) & 1;
			u32 opc = 0;
			if (not is_load) opc = 0;
			else if (a5 == 3) opc = 1;
			else if (a5 == 2 and is_signed) opc = 2;
			else if (a5 == 2 and not is_signed) opc = 1;
			else if (not is_signed) opc = 1;
			else if (not is_64_dest) opc = 3; 
			else opc = 2;
			const u32 to_emit = 
				(a5 << 30U) | (0x38 << 24U) | (opc << 22U) |
				(1 << 21U) | (a3 << 16U) | (opt << 13U) |
				(S << 12U) | (2 << 10U) | (a2 << 5U) | (a1);
			insert_u32(&my_bytes, &my_count, to_emit);
		} 
		else if (op == clz) { puts("clz is unimplemented currently, lol"); abort(); }
		else if (op == rev) { puts("rev is unimplemented currently, lol"); abort(); }
		else if (op == extr) { puts("extr is unimplemented currently, lol"); abort(); }
		else if (op == ldrl) { puts("ldrl is unimplemented currently, lol"); abort(); }
		else if (op == halt) {}
		else {
			printf("error: unknown mi op=\"%s\"\n", operations[op]);
			abort();
		}
	}
} else {
	printf("unknown target architecture: %llu\n", target_arch);
	abort();
}

	puts("info: machine code assembled. generating output file...");

if (output_format == debug_output_only) {

	puts("debugging executable bytes:\n");
	for (nat i = 0; i < my_count; i++) {
		if (i % 32 == 0) puts("");
		if (my_bytes[i]) printf("\033[32;1m");
		printf("%02hhx ", my_bytes[i]);
		if (my_bytes[i]) printf("\033[0m");
	}
	puts("");
	exit(0);

} else if (output_format == ti_txt_executable) {

	char out[14000] = {0};
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

	int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("open: could not create executable file"); exit(1); }
	write(file, out, len);
	close(file);
	printf("ti-txt: wrote %llu bytes to file %s.\n", len, output_filename);

	char debug_string[4096] = {0};
	snprintf(debug_string, sizeof debug_string, "../../led_display/embedded_assembler/msp430_disassembler/run %s", output_filename);
	system(debug_string);


} else if (output_format == macho_executable) {

	while (my_count % 16) insert_byte(&my_bytes, &my_count, 0);

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

	int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("could not create executable file"); exit(1); }
	int r = fchmod(file, 0777);
	if (r < 0) { perror("could not make the output file executable"); exit(1); }
	write(file, data, count);
	close(file);
	printf("mach-o: wrote %llu bytes to file %s.\n", count, output_filename);

	char codesign_string[4096] = {0};
	snprintf(codesign_string, sizeof codesign_string, "codesign -s - %s", output_filename);
	system(codesign_string);

	// debugging:
	snprintf(codesign_string, sizeof codesign_string, "otool -htvxVlL %s", output_filename);
	system(codesign_string);
	snprintf(codesign_string, sizeof codesign_string, "objdump -D %s", output_filename);
	system(codesign_string);

} else {
	printf("unknown target architecture: %llu\n", target_arch);
	abort();
}

	printf("info: successsfully generated executable: %s\n", output_filename);
}



}








*/






































































































































































































		//printf("RESULTS  ");
		//printf("	is_copy = %llu   ", future_is_copy);
		//printf("	copy_ref = %llu   ", future_copy_ref);
		//printf("	copy_type = %llu   ", future_copy_type);
		//printf("	mismatch = %llu", copy_ref_mismatch);
		//puts("");
		
		//printf("CP: VARIABLE: %10s   ", variables[this_var]);

		//puts("note: on sythesizing of preds, we found this.  publishing the values:");
		//printf("info:    future_type = %llu, future_value = %llu\n", future_type, future_value);

		//puts("checking for CTK value mismatch without runtime component...\n");




		//puts("");
		//printf("CT: VARIABLE: %10s   ", variables[this_var]);





//puts("found a runtime branch as a predecessor!");
					//puts("within the branch, the condition contains RTK variables: ");
					//if (not (ins[pred].imm & 1) and not type[pred * var_count + ins[pred].args[0]]) printf("%s was runtime known.\n", variables[ins[pred].args[0]]);
					//if (not (ins[pred].imm & 2) and not type[pred * var_count + ins[pred].args[1]]) printf("%s was runtime known.\n", variables[ins[pred].args[1]]);
					//puts("thus, the branch is rtk, and we will mark the boolean as such.");
					//getchar();


// puts("err, found a rtk value on this pred!"); 
				//printf("assigned: ft=%llu, fv=%llu\n", future_type, future_value); 





		//printf("RESULTS  ");
		//printf("	is_ct = %llu   ", future_type);
		//printf("	ct_value = %llu   ", future_value);
		//printf("	value_mismatch = %llu  ", value_mismatch);
		//puts("");



//puts("[no mismatch / consistent]: CONTINUING: warning: found at least one runtime branch! ");
			//puts("thus, we know that any value_mismatches on CTK variables, must be made RTK instead. ");
			//puts("we must now look for value_mismatches in CTK variables!");
			//getchar();

			//puts("do we abort...? uhhhh"); abort();




//puts("[mismatch]: ABORTING: warning: found at least one runtime branch! ");
			//puts("thus, we know that any value_mismatches on CTK variables, must be made RTK instead. ");
			//puts("we must now look for value_mismatches in CTK variables!");


//puts("marked this variable as runtime known.");

			//printf("value MISMATCH!\n"); getchar();



		//puts("publishing the copy data:");
		//printf("info:    future_is_copy = %llu, future_copy_ref = %llu, future_copy_type = %llu\n", future_is_copy, future_copy_ref, future_copy_type);



		//fflush(stdout);
		//getchar();
		//printf("DEF: VARIABLE: %10s   ", variables[this_var]);

		//printf("RESULTS  ");
		//printf("	def = %llu   ", future_def);
		//puts("");
		
		//if (future_def == (nat) -1) { puts("future_def == -1"); abort(); }


		//printf("pc = %llu, a0 = %llu, a1 = %llu, a1 = %llu\n", pc, a0, a1, a2); fflush(stdout);






		//printf("[pc = %llu] FINISHED EXECUTING INSTRUCTION!: publishing: ", pc);
		//printf("type=%llu, value=%llu, is_copy=%llu, copy_of=%llu, copy_type=%llu\n", 
		//	out_t, out_v, out_is_copy, out_copy_ref, out_copy_type
		//);
		//getchar();


//puts("failed to push false side!");
					//printf("gt0 < ins_count = %u\n", gt0 < ins_count);
					//printf("visited[gt0] < 2 = %u\n", visited[gt0] < 2);
					//getchar();




//puts("failed to push true side!");
					//printf("gt1 < ins_count = %u\n", gt1 < ins_count);
					//printf("visited[gt1] < 2 = %u\n", visited[gt1] < 2);
					//getchar();


				//printf("info: taking %s side of CTK branch!\n", cond ? "true" : "false"); getchar();

				// TODO: work this out:     are la's CT OR NOT!?!?!?!    .....they arent. lol. i think. 










	// next opt: 

	// todo:  "DEAD STORE ELIMINATION"!!   remove the set statements which don't do anything now. 
	// go through each set statement, and look at the number of downstream uses of that set actaully happened!  
	// for this, we need to trace which active value for a set is currently being used lol.
/*
------------------------------------------------------------------------------------------------------------------------------------------------------------------------
current state: 1202504163.024837


		// to materialize the variable,  we are just going to utilize the fact that    all ctk variables have a   set_imm statement that defines them, somewhere prior to this instruction. we will simply keep track of where this instruction lives, for each compiletime variable, during CT-EVAl. 

		// to keep track of this, we just simply reassign it when we do a "set", but not when we do a "add/sub/si" etc etc. 

		// then,  in this code, here, we will read that value, (called "definition_at") and then we will overwrite this instruction with the REAL value we expect. hypothetically, it should be a statement along the execution path of the CTK pred which caused this in the first place, and thus we don't need to add/insert any new instructions. we are just using the existing one. 

		// oh also, we need to remember to set the visted[pc] = 1;  for that instruction lol.  to turn it to be runtime known lol. yay. 




------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/



	
	// todo opt:
	//  . simplify control flow! 
	//  . simplify labels
	//  . copy propagation! 
	//  . loop invariant code motion

















/*if (t == 1) { future_type = 1; future_value = v; puts("err, found a rtk value on this pred!"); 
			printf("assigned: ft=%llu, fv=%llu\n", future_type, future_value); break; }

			if (future_type) { 
				puts("found a first pred, with ctk!");
				future_type = 0; 
				future_value = v; 
			} else if (future_value != v) { value_mismatch = 1; puts("mismatch ctk value!"); } */







/*static void print_machine_instruction(struct instruction this, char** names, nat name_count) {
	printf("  %13s { ", mi_spelling[this.op]);
	for (nat a = 0; a < 5; a++) {
		if (this.op == csel and a == 3) { printf("        #{%s}   ", ins_spelling[this.args[a]]); continue; }
		if (this.args[a] < 256) printf("%3llu", this.args[a]); 
		else printf("0x%016llx[%llu]", this.args[a], this.args[a]);
		printf("('%8s') ", this.args[a] < name_count ? names[this.args[a]] : "");
	}

	printf("} : {.f=#");
	if (this.gotos[0] == ~is_label or this.gotos[0] == (nat) -1) {} 
	else if (this.gotos[0] < 256) printf("%3llu", this.gotos[0]); 
	else printf("0x%016llx", this.gotos[0]);
	printf(", .t=#");
	if (this.gotos[1] == ~is_label or this.gotos[1] == (nat) -1) {} 
	else if (this.gotos[1] < 256) printf("%3llu", this.gotos[1]); 
	else printf("0x%016llx", this.gotos[1]);
	printf("}");
}*/





//////////////////opt////////////////
/* 	optimizations left to perform:

		- copy propagation / simplification of dataflow
		- control flow simplification, eliminate chained jumps
		- simplify conditions?...

		-   TODO: look up some more lol.


opt     future passes!
1202504115.221656
notes to me:
----------

x	1. add opt    mul_imm ---> si_imm 


	2. add opt   copy prop


	3. add opt   commutitive/accumulative math consolidation   eg, 
						add x 01    add x 001   becomes    add x 011


	4. add opt   jump control flow simplification :   jumps to labels, right after the jump, 
	
	5. sink common logic into basic blocks, deduplicating code


	for (nat i = 0; i < ins_count; i++) {
		if (not visited[i]) continue;
		const nat op = ins[i].op;

		// lets try to do copy propagation now?
	}
*/
//////////////////////////////////



















	// copy prop: 

	/*

		the basic idea:

			you just do the ct-eval algortihm/stage  that we already coded up ,  but instead of storing ctk/rtk ness  

			you store   if a variable is currently the result of an assignment. ie, the computation currently performed is simply   that which is given by another variable!
				ie,      you have seen          set x y           and x has never been used with any other instruction since seeing that set!


				then, when you see the set, you store   a 1 in the type    (saying it is actually the result of an assignment, and a candidate for copy prop, 

						and then, for the value, you set it to the variable reference y !!!!!      


						but note, you coulddd actuallyyyy have 





		ANDD!!!

			the control flow merging abilities of the ct-eval stage   ARE RELEVANT  HERE        FOR COPY PROP!!! 

				ITS THE SAME PROBLEMMMM     we are tracing data flow throughhhh RT loopss!!!!

				if a set  is given in the same way    coming from two different pred's     then we can literally just keep the fact that x is represented by y!!!
					butttt if we see DIFFERENTTTT   "value[]"'s for x    coming from different preds,   then we know we must keep x, 

						and we cannot actually replace THIS INSTRUCTIONSS use of x. with y. 



		now, 
	note, 
			we never actually delete the      set x y      !!!!


		we just    attempt   to make it pointless lol. 

			and then, later on, if we have literally managed to remove all uses of x,   and successfullyyyy replace them with y, 


					then we know!   that the set is not used, because now,    we are actually setting a variable to a value, and never using it, 


							ie,   dead store   elmination!   this is a trivial optimization to do! niceeee



								that happens afterwards. after we finish the whole     "while(stack_count)"   stage of the this copy prop algorithm! 

	so yeah 


	thats how copy prop works! 













wait. 



	no 
					instead, 
							lets just merge   copy prop,    and ct-eval. 




							 		THEY     ARE      THE SAME   ALGORITHMMM



					its just we need to add more arrays to the matrix   type/value thingy 

						like, we need to add   equiv_var[]        and then also    is_copy[]


							we can just do the merge logic   in with the ct-eval's merge logic! shouldnt be that hard lol



		

		honesty, the crazy part is that.. 

			i feel like    theres a more general form of this to, 


				which doesnt just deal with  variables being equivalent, or equal, at a certain poitn, 

						but actually represents arbitrary computatinal semantic information on the variables... 

				like, i feel like we can genuinely generalize this algortihm,   from ct-eval   amid   copy-prop   to finally 


							a more generallll   opmimization  pass




			one which, in theory could even do     common sub expression elimination?? hmmm 
									ie, when the comptuaion on a variable is equivlaent to another one kinda


							hmmmmm


	i'll thinkk about it 



i'm quite certain about merging  copy prop and ct-eval though


that feels like a no brainer nowww




	*/
































/*














main idea behind solution for instruction selection:
----------------------------------------------------------

it all revolves around the notion of computational equivalence. there are only so many ways to write the IR sequences that equate to doing an arm64 "addsr" instruction, and thus, we can dig deeper and look at exactly what transformations would cause two given patterns to be different. turns out, for this example, only additive commutativity (exchanging the order of the arguments to an add) and also inserting extraneous copies/moves, can make two IR patterns distinct, yet still have both functionally still perform an addsr. and addsr is not a special case, almost all the instruction on arm64 have this property, that there are only a finite and specific set of transformations which can occur on the IR pattern to transform it into a functionally equivalent but distinct IR pattern.

so now, to start off with, we must first take the radically simplifying approach of simply expecting a certain predefined pattern (or maybe a few patterns) of IR instructions which we know efficiently represent an addsr. this is what i have coded up so far. it uses locate data instruction to find a particular IR instruction thats part of the pattern, and requires arguments, and some relative sequence constraints to be satisfied to classify as a match.

then, second, we now solve the inevitable problems which arises of this simple pattern-matching approach being too brittle and restrictive (ie, we may accidentally not classify some IR data flow performing an addsr, as an addsr, resulting in slower, less efficient code). the way that we solve this problem is by tackling one by one the possible ways that IR patterns can be functionally equivalent, but distinct. for addsr (and most other data instructions), these include commutativity of operands, and adding extraneous copies/assignments. to solve the first issue, we simply expect two patterns, one where the operands are in one order, and one where they are the other alternative order. secondly, to solve the second problem of assignments, we simply run a copy propagation optimization pass over the IR prior to instruction selection! this will make all instructions sequences have the minimal number of copies/moves, which means there is much less (possibly ZERO) possibility for a valid "addsr" pattern of IR instructions to be different from the one we are expecting, yet still validly perform the semantics for an addsr!!!

thats the key idea. its leveraging the fact that there are a finite number of ways that two computations can be equivalent, in this context, and thus we can enumerate and resolve/restrict all methods of causing those equivalences, and thus the end result is that a simplistic pattern-matching approach can work extremely well and reliably recognize the IR data flow semantics required for generating a given machine instruction, WITHOUT EVER forming a tree of IR instructions, or DAG of IR instructions!!! isnt that so cool????





arm64:
	addsr_lsl   d, n, (m LSL #k)



lang isa:

	


	set d m

	si_imm d k

	add d n









	set s m 
	
	si_imm s k

	set d n

	add d s




*/

































































































































































































































/*





if (op == set) {
			const nat b = locate_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, variables, variable_count);
			if (b == (nat) -1) goto subsrlsl_bail;
			const nat c = locate_instruction(sub, arg0, 0, 1, 0, b + 1, ins, ins_count, variables, variable_count);
			if (c == (nat) -1) goto subsrlsl_bail;
			struct instruction new = { .op = addr };
			new.args[0] = arg0;
			new.args[1] = ins[c].args[1];
			new.args[2] = arg1;
			new.args[3] = ins[b].args[1];
			new.args[4] = 1;
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1; selected[c] = 1;
			goto finish_mi_instruction;
		} subsrlsl_bail:





*/








		// sub x x  -->  set x 0
		// eor x x  -->  set x 0

		// div x x  -->  set x 1      ???

		// and x x  nop
		// or  x x  nop
		// set x x  nop






/*

1202502252.005724

	just writing some goals of the language for me:

target ISAs:
---------------------------------------------
	- ARM64
	- ARM32
	- RV64
	- RV32
	- MSP430

performance advantages over C / LLVM:
---------------------------------------------

	- full program ins-level optimizations (no basic blocks)

	- no SSA representation: stateful optimizations

	- conservative memory aliasing semantics

	- no implicit memory allocations, register level semantics instead

	- no unneccessary ABI requirements on functions, no link-time optimization

	- no unneccessary overhead: no abstractions present
	  which don't easily map onto hardware instructions 

	- bit-width for variables providing more use of registers

	- more powerful compiletime evaluation semantics
	
	- vectorization primitives in the language 


*

// a new parser and ct system written on 1202502016.131044 dwrr
// 1202501116.181306 new parser   dwrr



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
typedef uint64_t nat;

enum {
	nullins, 
	
	set, add, sub, mul, div_, 
	and_, or_, eor, si, sd,
	lt, eq, ge, ne, do_, at, 
	ld, st, sc, rt, lf, eoi,

	isa_count,
	set_imm,  add_imm,  sub_imm,
	mul_imm,  div_imm,
	and_imm,  or_imm,   eor_imm,
	si_imm,   sd_imm,
	lt_imm,   gt_imm,   eq_imm,
};

static const char* ins_spelling[] = {
	"__ERROR_null_ins_unused__", 
	
	"set", "add", "sub", "mul", "div", 
	"and", "or", "eor", "si", "sd",
	"lt", "eq", "ge", "ne", "do", "at", 
	"ld", "st", "sc", "rt", "lf", "eoi",

	"__ISA_COUNT__ins_unused__",
	"set_imm", "add_imm", "sub_imm",
	"mul_imm", "div_imm",
	"and_imm", "or_imm",  "eor_imm",
	"si_imm",  "sd_imm",
	"lt_imm",  "gt_imm", "eq_imm",
};


//     in c, this would translate to:
//
//        = + - * / & | ^ << >> < == >= != goto : ____ ____ syscall() int/long  #include ____
//
// roughly speaking. 


struct instruction {
	nat op;
	nat gotos[2];
	nat args[7];
};

enum language_systemcalls {
	system_exit,
	system_read, system_write, 
	system_open, system_close,
	systemcall_count
};

static const char* systemcall_spelling[systemcall_count] = {
	"system_exit",
	"system_read", "system_write", 
	"system_open", "system_close",
};

enum arm64_ins_set {
	addsrlsl, addsrlsr,
	subsrlsl, subsrlsr,
	andsrlsl, andsrlsr,
	orrsrlsl, orrsrlsr,
	ornsrlsl, ornsrlsr,
	eorsrlsl, eorsrlsr,
	eonsrlsl, eonsrlsr,
	movz, addi, subi, andi, orri, eori, 
	madd, msub, udiv, lslv, lsrv, 
	addssrlsl, addssrlsr,
	subssrlsl, subssrlsr,
	andssrlsl, andssrlsr,
	addsi, subsi, andsi,
	csel, cbz, cbnz, bcond,
	svc,
	arm_isa_count,
};

static const char* mi_spelling[arm_isa_count] = {
	"addsrlsl", "addsrlsr",
	"subsrlsl", "subsrlsr",
	"andsrlsl", "andsrlsr",
	"orrsrlsl", "orrsrlsr",
	"ornsrlsl", "ornsrlsr",
	"eorsrlsl", "eorsrlsr",
	"eonsrlsl", "eonsrlsr",
	"movz", "addi", "subi", "andi", "orri", "eori", 
	"madd", "msub", "udiv", "lslv", "lsrv", 
	"addssrlsl", "addssrlsr",
	"subssrlsl", "subssrlsr",
	"andssrlsl", "andssrlsr",
	"addsi", "subsi", "andsi",
	"csel", "cbz", "cbnz", "bcond",
	"svc",
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

static const nat is_label = 1LLU << 63LLU;

static nat get_call_output_count(nat n) {
	if (n == system_exit) return 0;
	if (n == system_read) return 2;
	if (n == system_write) return 2;
	if (n == system_close) return 1;
	if (n == system_open) return 2;
	abort();
}

static void print_index(const char* text, nat text_length, nat begin, nat end) {
	printf("\n\t@%llu..%llu: ", begin, end); 
	for (nat i = 0; i < text_length; i++) {
		if (i == begin) printf("\033[32;1m[");
		putchar(text[i]);
		if (i == end) printf("]\033[0m");
	}
	puts("\n");
}

static void print_dictionary(char** names, nat* locations, nat name_count) {
	puts("found dictionary: { \n");
	for (nat i = 0; i < name_count; i++) {
		printf("\t%3llu: name = \"%-10s\", location = %3lld, \n", 
			i, names[i], locations[i]
		);
	}
	puts("}");
}

static void print_instruction(struct instruction this, char** names, nat name_count) {
	printf("  %8s { ", ins_spelling[this.op]);
	for (nat a = 0; a < 3; a++) {
		if (this.args[a] < 256) printf("%3llu", this.args[a]); 
		else printf("0x%016llx", this.args[a]);
		printf("('%8s') ", this.args[a] < name_count ? names[this.args[a]] : "");
	}

	printf("} : {.f=#");
	if (this.gotos[0] == ~is_label or this.gotos[0] == (nat) -1) {} 
	else if (this.gotos[0] < 256) printf("%3llu", this.gotos[0]); 
	else printf("0x%016llx", this.gotos[0]);
	printf(", .t=#");
	if (this.gotos[1] == ~is_label or this.gotos[1] == (nat) -1) {} 
	else if (this.gotos[1] < 256) printf("%3llu", this.gotos[1]); 
	else printf("0x%016llx", this.gotos[1]);
	printf("}");
}

static void print_machine_instruction(struct instruction this, char** names, nat name_count) {
	printf("  %13s { ", mi_spelling[this.op]);
	for (nat a = 0; a < 5; a++) {
		if (this.op == csel and a == 3) { printf("        #{%s}   ", ins_spelling[this.args[a]]); continue; }
		if (this.args[a] < 256) printf("%3llu", this.args[a]); 
		else printf("0x%016llx[%llu]", this.args[a], this.args[a]);
		printf("('%8s') ", this.args[a] < name_count ? names[this.args[a]] : "");
	}

	printf("} : {.f=#");
	if (this.gotos[0] == ~is_label or this.gotos[0] == (nat) -1) {} 
	else if (this.gotos[0] < 256) printf("%3llu", this.gotos[0]); 
	else printf("0x%016llx", this.gotos[0]);
	printf(", .t=#");
	if (this.gotos[1] == ~is_label or this.gotos[1] == (nat) -1) {} 
	else if (this.gotos[1] < 256) printf("%3llu", this.gotos[1]); 
	else printf("0x%016llx", this.gotos[1]);
	printf("}");
}

static void print_instructions(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count,
	nat* ignore
) {
	puts("found instructions: {");
	for (nat i = 0; i < ins_count; i++) {
		if (ignore[i]) printf("\033[38;5;239m");
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		puts("");
		if (ignore[i]) printf("\033[0m");
	}
	puts("}");
}

static void print_stack(nat* stack, nat stack_count) {
	printf("stack: %llu { \n", stack_count);
	for (nat i = 0; i < stack_count; i++) {
		printf("stack[%llu] =  %llu\n", 
			i, stack[i]
		);
	}
	puts("}");
}

static void print_machine_instructions(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count
) {
	printf("MACHINE INSTRUCTIONS: (%llu count) {\n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_machine_instruction(ins[i], names, name_count);
		puts("");
	}
	puts("}");
}

static void print_instruction_index(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count, nat* ignore,
	nat here, const char* message
) {
	printf("%s: at index: %llu: { \n", message, here);
	for (nat i = 0; i < ins_count; i++) {
		if (ignore[i]) printf("\033[38;5;239m");
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		if (i == here)  printf("  <------ %s\n", message); else puts("");
		if (ignore[i]) printf("\033[0m");
	}
	puts("}");
}

static void print_instruction_index_selected(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count, nat* ignore,
	nat here, const char* message, nat* selected
) {
	printf("%s: at index: %llu: { \n", message, here);
	for (nat i = 0; i < ins_count; i++) {
		if (ignore[i]) printf("\033[38;5;239m");
		printf("%c\t#%04llu: ", selected[i] ? 'S' : ' ', i);
		print_instruction(ins[i], names, name_count);
		if (i == here)  printf("  <------ %s\n", message); else puts("");
		if (ignore[i]) printf("\033[0m");
	}
	puts("}");
}



*static void print_instructions_ct_values_index(
	struct instruction* ins, const nat ins_count,
	char** names, const nat name_count, nat* locations,
	nat* execution_state_ctk, nat* execution_state_values,
	nat pc, const char* message
) {
	printf("@%lld: %s: (%llu instructions)\n", pc, message, ins_count);
	for (nat i = 0; i < ins_count; i++) {
		bool found = false;
		for (nat l = 0; l < name_count; l++) {
			if (i == locations[l]) { printf("LABEL{%s}: ", names[l]); found = true; } 
		}
		if (found) puts("");
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		nat* values = execution_state_values + name_count * i;
		nat* ctk = execution_state_ctk + name_count * i;
		printf(" -- ct { ");
		for (nat n = 0; n < name_count; n++) {
			if (not ctk[n]) continue;
			printf("%s:%lld ", names[n], values[n]);
		}
		printf("}  ");
		if (i == pc) printf("    <--- %s\n", message); else puts("");
	}
	printf("done printing index \"%s\"\n", message);
}*

static nat* compute_predecessors(struct instruction* ins, const nat ins_count, const nat pc, nat* pred_count, nat* ignore) {
	nat* result = NULL;
	nat count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (ignore[i]) continue;
		if (ins[i].gotos[0] == pc or ins[i].gotos[1] == pc) {
			result = realloc(result, sizeof(nat) * (count + 1));	
			result[count++] = i;
		}
	}
	*pred_count = count;
	return result;
}

static void print_ct_values(char** names, nat name_count, nat* is_runtime, nat* values) {
	printf("ct values: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (is_runtime[i]) continue;
		printf("\tvalues[%s] = 0x%016llx\n", names[i], values[i]);
	}
	puts("}");
}


static nat use_count(struct instruction* ins, const nat ins_count, nat this) {
	nat count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].args[0] == this) count++;
		if (ins[i].args[1] == this) count++;
		if (ins[i].args[2] == this) count++;
	}
	return count; 
}

static nat locate_instruction(
	nat expected_op, nat expected_arg0, nat expected_arg1,
	nat use_arg0, nat use_arg1, nat starting_from,
	struct instruction* ins, const nat ins_count,
	char** names, nat name_count, nat* ignore
) {
	nat pc = starting_from;
	while (pc < ins_count) {
		if (ignore[pc]) { puts("locate data instruction . ignore pc encountered . abort"); abort(); } 
		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];
		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);
		nat pred_count = 0;
		compute_predecessors(ins, ins_count, pc, &pred_count, ignore);
		if (	pred_count == 1 and 
			op == expected_op and 
			(not use_arg0 or expected_arg0 == arg0) and 
			(not use_arg1 or expected_arg1 == arg1)
		) return pc; 
		if (is_branch) break;
		if (use_arg0 and arg1 == expected_arg0) break;
		if (use_arg1 and arg0 == expected_arg1) break;
		pc = ins[pc].gotos[0];
	}
	return (nat) -1;
}

int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));

	struct instruction ins[4096] = {0};
	nat ins_count = 0;
	char* names[4096] = {0};
	nat name_count = 0;
	nat locations[4096] = {0};
	nat is_runtime[4096] = {0};
	nat bit_count[4096] = {0};
	memset(locations, 255, sizeof locations);

	struct file filestack[4096] = {0};
	nat filestack_count = 1;
	const char* included_files[4096] = {0};
	nat included_file_count = 0;

{
	int file = open(argv[1], O_RDONLY);
	if (file < 0) { puts(argv[1]); perror("open"); exit(1); }
	const nat text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(text_length + 1, 1);
	read(file, text, text_length);
	close(file);
	filestack[0].filename = argv[1];
	filestack[0].text = text;
	filestack[0].text_length = text_length;
	filestack[0].index = 0;
	printf("file: (%llu chars)\n<<<%s>>>\n", text_length, text);
}

process_file:;
	{nat 	word_length = 0, 
		word_start = 0,
		state = 0,
		comment = 0,
		arg_count = 0;
	nat args[7] = {0};

	const nat starting_index = 	filestack[filestack_count - 1].index;
	const nat text_length = 	filestack[filestack_count - 1].text_length;
	char* text = 			filestack[filestack_count - 1].text;
	const char* filename = 		filestack[filestack_count - 1].filename;

	for (nat index = starting_index; index < text_length; index++) {
		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++; 
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;

		char* word = strndup(text + word_start, word_length);
		printf("file:%llu: at \"%s\" @ %llu\n", index, word, word_start);

		if (not strcmp(word, ".") and not comment) { comment = 1; goto next_word; }
		if (comment) { if (not strcmp(word, ".")) { comment = 0; goto next_word; } else goto next_word; }

		if (not state) {
			arg_count = 0;
			if (not strcmp(word, "eoi")) break;
			for (; state < isa_count; state++) {
				if (not strcmp(word, ins_spelling[state])) goto next_word;
			}
			print_error:
			printf("%s:%llu:%llu: error: undefined %s \"%s\"\n",
				filename, word_start, index, 
				state == isa_count ? "operation" : "variable", word
			); 
			print_index(text, text_length, word_start, index);
			abort();

		} 
		nat variable = 0;
		for (; variable < name_count; variable++) {
			if (not strcmp(word, names[variable])) goto variable_name_found;
		}

		bool valid = 0;
		if (state == lf) valid = 1;
		if (state == lt and arg_count == 2) valid = 1;
		if (state == eq and arg_count == 2) valid = 1;
		if (state == ge and arg_count == 2) valid = 1;
		if (state == ne and arg_count == 2) valid = 1;
		if (state == rt and arg_count == 0) valid = 1;
		if (state == at and arg_count == 0) valid = 1;
		if (state == do_ and arg_count == 0) valid = 1;
		if (state == eor and arg_count == 0) valid = 1;
		if (state == set and arg_count == 0) valid = 1;
		if (not valid) goto print_error;
		names[name_count++] = word; 

	variable_name_found:
		args[arg_count++] = variable;

		if (state == lf) {
			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				printf("warning: %s: file already included\n", word);
				goto next_word;
			}
			included_files[included_file_count++] = word;
			int file = open(word, O_RDONLY);
			if (file < 0) { puts(word); perror("open"); exit(1); }
			const nat new_text_length = (nat) lseek(file, 0, SEEK_END);
			lseek(file, 0, SEEK_SET);
			char* new_text = calloc(new_text_length + 1, 1);
			read(file, new_text, new_text_length);
			close(file);
			filestack[filestack_count - 1].index = index;
			filestack[filestack_count].filename = word;
			filestack[filestack_count].text = new_text;
			filestack[filestack_count].text_length = new_text_length;
			filestack[filestack_count++].index = 0;
			name_count--;
			goto process_file;

		} else if (state == do_) {
			state = 0;
			struct instruction new = {
				.op = do_,
				.args = {variable},
				.gotos = {0, variable | is_label},
			};
			ins[ins_count++] = new;
		
		} else if (state == at) {
			state = 0;
			locations[variable] = ins_count;
			goto next_word;
			} else if (state == rt and arg_count == 2) {
			state = 0;
			is_runtime[args[0]] = 1;
			bit_count[args[0]] = args[1];
			goto next_word;

		} else if (((state == lt or state == eq or 
			state == ge or state == ne or 
			state == ld or state == st) and 
			arg_count == 3)
			or

			((state == add or state == sub or 
			state == mul or state == div_ or 
			state == and_ or state == or_ or 
			state == eor or state == set or
			state == si or state == sd) and 
			arg_count == 2) 
			or

			(state == sc and arg_count == 7)
		) {
			const nat op = state;
			struct instruction new = {
				.op = op,
				.gotos = {ins_count + 1, 
					op == lt or op == ge or 
					op == ne or op == eq ? 
					variable | is_label : (nat) ~is_label
				},
			};
			memcpy(new.args, args, sizeof args);
			if (op == ge or op == ne) { 
				new.gotos[0] = new.gotos[1]; 
				new.gotos[1] = ins_count + 1;
				if (op == ge) new.op = lt;
				if (op == ne) new.op = eq;
			}
			ins[ins_count++] = new;
			state = 0;
		}	
		next_word: word_length = 0;
	}

	filestack_count--;
	if (filestack_count) goto process_file;
	}

	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].gotos[0] & is_label) ins[i].gotos[0] = locations[ins[i].gotos[0] & ~is_label];
		if (ins[i].gotos[1] & is_label) ins[i].gotos[1] = locations[ins[i].gotos[1] & ~is_label];
	}

	nat stack_count = 1;
	nat* stack = calloc(ins_count, sizeof(nat));
	nat* visited = calloc(ins_count + 1, sizeof(nat));
	nat values[4096] = {0};
	memset(values, 255, sizeof values);
	nat ignore[4096] = {0};

	print_dictionary(names, locations, name_count);
	print_instructions(ins, ins_count, names, name_count, ignore);

	while (stack_count) {
		//print_stack(stack, stack_count);
		const nat pc = stack[--stack_count];

		//print_ct_values(names, name_count, is_runtime, values);
		//print_instruction_index(ins, ins_count, names, name_count, ignore, pc, "PC");
		//printf("executing pc #%llu\n", pc);
		//print_instruction(ins[pc], names, name_count); puts("");
		//getchar();

		visited[pc] = 1;
		const nat op = ins[pc].op;

		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];
		const nat goto0 = ins[pc].gotos[0];
		const nat goto1 = ins[pc].gotos[1];
		const nat rt0 = arg0 < name_count ? is_runtime[arg0] : 0;
		const nat rt1 = arg1 < name_count ? is_runtime[arg1] : 0;
		const nat ct0 = not rt0;
		const nat ct1 = not rt1;
		const nat val0 = arg0 < name_count ? values[arg0] : 0;
		const nat val1 = arg1 < name_count ? values[arg1] : 0;
		
		if (op == lt or op == eq) {
			if (ct0 and ct1) {
				nat c = 0;
				ignore[pc] = 1;
				if (op == eq and val0 == val1 or op == lt and val0 < val1) c = 1;
				if (ins[pc].gotos[c] < ins_count) stack[stack_count++] = ins[pc].gotos[c];
				continue;

			} else if (ct0 or ct1) {
				if (ct0) ins[pc].args[0] = ins[pc].args[1];
				ins[pc].args[1] = val0;
				ins[pc].op = op == lt ? gt_imm : eq_imm;
			}

			if (goto0 < ins_count and not visited[goto0]) stack[stack_count++] = goto0;
			if (goto1 < ins_count and not visited[goto1]) stack[stack_count++] = goto1;
			continue;		

		} else if (op == sc) {
			if (rt0) { 
				puts("error: all system calls must be compile time known."); 
				abort();
			}

			const nat n = val0;
			if (n == 9) goto do_debug_system_call;
			const nat output_count = get_call_output_count(n);
			for (nat i = 0; i < output_count; i++) {
				if (not is_runtime[ins[pc].args[1 + i]]) { puts("system call ct rt out"); abort(); }
			}

			ins[pc].args[0] = values[arg0];

			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(
					ins, ins_count, 
					names, name_count, ignore,
					pc, "CFG termination point here"
				);
				ins[pc].gotos[0] = (nat) -1;
				ins[pc].gotos[1] = (nat) -1;
				continue;

			} else if (n == 9) {
				do_debug_system_call: printf("DEBUG: %llu\n", val1); ignore[pc] = 1;
			} else {
				printf("info: found %s sc!\n", systemcall_spelling[n]);
				print_instruction_index(
					ins, ins_count, 
					names, name_count, ignore,
					pc, systemcall_spelling[n]
				);
			}
			if (goto0 < ins_count) stack[stack_count++] = goto0;			
			continue;

		} else if (op >= isa_count or rt0 and rt1) goto next_ins;
		else if (ct0 and rt1) { 

			if (op == set) { is_runtime[arg0] = 1; goto next_ins; }
			else {
				puts("error: instruction with compiletime destination must have compiletime source."); 
				print_instruction_index(
					ins, ins_count, 
					names, name_count, ignore,
					pc, "source is runtime known, but destination is compiletime known"
				);			
				abort();
			}
		}
		else if (ct0 and ct1) {
			ignore[pc] = 1;
			     if (op == set) values[arg0] = val1;
			else if (op == add) values[arg0] += val1;
			else if (op == sub) values[arg0] -= val1;
			else if (op == mul) values[arg0] *= val1;
			else if (op == div_)values[arg0] /= val1;
			else if (op == and_)values[arg0] &= val1;
			else if (op == or_) values[arg0] |= val1;
			else if (op == eor) values[arg0] ^= val1;
			else if (op == si)  values[arg0] <<= val1;
			else if (op == sd)  values[arg0] >>= val1;
			else {
				puts("internal error: op execution not specified");
				printf("op = %llu, op = %s\n", op, ins_spelling[op]);
				abort();
			}
			next_ins: if (goto0 < ins_count) stack[stack_count++] = goto0;
			continue;
		}

		ins[pc].args[1] = val1;
		if (op == set) ins[pc].op = set_imm;
		if (op == add) ins[pc].op = add_imm;
		if (op == sub) ins[pc].op = sub_imm;
		if (op == mul) ins[pc].op = mul_imm;
		if (op == div_)ins[pc].op = div_imm;
		if (op == and_)ins[pc].op = and_imm;
		if (op == or_) ins[pc].op = or_imm;
		if (op == eor) ins[pc].op = eor_imm;
		if (op == si)  ins[pc].op = si_imm;
		if (op == sd)  ins[pc].op = sd_imm;

		if (	ins[pc].op ==  set and arg0 == arg1 or
			ins[pc].op ==  or_ and arg0 == arg1 or 
			ins[pc].op == and_ and arg0 == arg1 or 
			ins[pc].op == add_imm and values[arg1] == 0 or 
			ins[pc].op == sub_imm and values[arg1] == 0 or 
			ins[pc].op == mul_imm and values[arg1] == 1 or 
			ins[pc].op == div_imm and values[arg1] == 1 or 
			ins[pc].op ==  or_imm and values[arg1] == 0 or
			ins[pc].op == eor_imm and values[arg1] == 0 or
			ins[pc].op ==  si_imm and values[arg1] == 0 or
			ins[pc].op ==  sd_imm and values[arg1] == 0) 
			ignore[pc] = 1;

		if (goto0 < ins_count) stack[stack_count++] = goto0;
	}

	for (nat i = 0; i < ins_count; i++) if (not visited[i]) ignore[i] = 1;

	print_ct_values(names, name_count, is_runtime, values);
	print_dictionary(names, locations, name_count);
	print_instructions(ins, ins_count, names, name_count, ignore);

	printf("not ignore: { \n");
	for (nat i = 0; i < ins_count; i++) {
		if (not ignore[i]) {
			printf("\t%llu : ", i);
			print_instruction(ins[i], names, name_count); puts("");
		}
	}
	printf("}\n");

	printf("is_runtime variables: { ");
	for (nat i = 0; i < name_count; i++) if (is_runtime[i]) printf("%s ", names[i]);
	printf("}\n");

	getchar();

	puts("starting ins sel..");
	struct instruction mi[4096] = {0};
	nat mi_count = 0;
	nat selected[4096] = {0};

	for (nat i = 0; i < ins_count; i++) {

		if (ignore[i]) continue;

		if (selected[i]) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i);
			continue;
		}

		const nat op = ins[i].op;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1];

		print_instruction_index_selected(ins, ins_count, names, name_count, ignore, i, "SELECTION ORIGIN", selected);
		printf("selecting from i #%llu\n", i);
		print_instruction(ins[i], names, name_count); puts("");
		//getchar();

		if (op == set) { 
			const nat i1 = locate_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count, ignore);
			if (i1 == (nat) -1) goto csellt_bail;
			const nat i2 = locate_instruction(set, 0, 0, 0, 0, i1 + 1, ins, ins_count, names, name_count, ignore); 
			if (i2 == (nat) -1) goto csellt_bail;
			const nat i3 = locate_instruction(lt, 0, arg0, 0, 1, i2 + 1, ins, ins_count, names, name_count, ignore);
			if (i3 == (nat) -1) goto csellt_bail;
			if (use_count(ins, ins_count, arg0) != 3) goto csellt_bail;
			if (not ((ins[i3].gotos[1] == i3 + 1 and ins[i3].gotos[0] == i3 + 2) or 
				 (ins[i3].gotos[0] == i3 + 1 and ins[i3].gotos[1] == i3 + 2))
			) goto csellt_bail;
			if (not (ins[i3 + 1].op == set and ins[i3 + 1].args[0] == ins[i2].args[0])) goto csellt_bail;
			
			struct instruction new = { .op = subssrlsl };
			new.args[0] = 0;
			new.args[1] = ins[i3].args[0];
			new.args[2] = arg1;
			new.args[3] = ins[i1].args[1];
			mi[mi_count++] = new;
			struct instruction new2 = { .op = csel };
			new2.args[0] = ins[i2].args[0];
			new2.args[1] = ins[i3 + 1].args[1];
			new2.args[2] = ins[i2].args[1];
			new2.args[3] = ins[i3].gotos[0] == i3 + 1 ? lt : ge;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			selected[i1] = 1; 
			selected[i2] = 1; 
			selected[i3] = 1; 
			selected[i3 + 1] = 1;
			goto finish_mi_instruction;
		} csellt_bail:

		if (op == set) { 
			const nat i1 = locate_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count, ignore);
			if (i1 == (nat) -1) goto cseleq_bail;
			const nat i2 = locate_instruction(set, 0, 0, 0, 0, i1 + 1, ins, ins_count, names, name_count, ignore); 
			if (i2 == (nat) -1) goto cseleq_bail;
			const nat i3 = locate_instruction(eq, 0, arg0, 0, 1, i2 + 1, ins, ins_count, names, name_count, ignore);
			if (i3 == (nat) -1) goto cseleq_bail;
			if (use_count(ins, ins_count, arg0) != 3) goto cseleq_bail;
			if (not ((ins[i3].gotos[1] == i3 + 1 and ins[i3].gotos[0] == i3 + 2) or 
				 (ins[i3].gotos[0] == i3 + 1 and ins[i3].gotos[1] == i3 + 2))
			) goto cseleq_bail;
			if (not (ins[i3 + 1].op == set and ins[i3 + 1].args[0] == ins[i2].args[0])) goto cseleq_bail;
			
			struct instruction new = { .op = subssrlsl };
			new.args[0] = 0;
			new.args[1] = ins[i3].args[0];
			new.args[2] = arg1;
			new.args[3] = ins[i1].args[1];
			mi[mi_count++] = new;
			struct instruction new2 = { .op = csel };
			new2.args[0] = ins[i2].args[0];
			new2.args[1] = ins[i3 + 1].args[1];
			new2.args[2] = ins[i2].args[1];
			new2.args[3] = ins[i3].gotos[0] == i3 + 1 ? eq : ne;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			selected[i1] = 1; 
			selected[i2] = 1; 
			selected[i3] = 1; 
			selected[i3 + 1] = 1;
			goto finish_mi_instruction;
		} cseleq_bail:

		if (op == set) { 
			const nat i1 = locate_instruction(mul, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count, ignore);
			if (i1 == (nat) -1) goto msub_bail;
			const nat i2 = locate_instruction(set, 0, 0, 0, 0, i1 + 1, ins, ins_count, names, name_count, ignore); // TODO: BUG:   s must be != to d.
			if (i2 == (nat) -1) goto msub_bail;
			const nat i3 = locate_instruction(sub, ins[i2].args[0], arg0, 1, 1, i2 + 1, ins, ins_count, names, name_count, ignore);
			if (i3 == (nat) -1) goto msub_bail;
			if (use_count(ins, ins_count, arg0) != 3) goto msub_bail;
			
			struct instruction new = { .op = msub };
			new.args[0] = ins[i2].args[0];
			new.args[1] = ins[i1].args[1];
			new.args[2] = arg1;
			new.args[3] = ins[i2].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[i1] = 1; selected[i2] = 1; selected[i3] = 1;
			goto finish_mi_instruction;
		} msub_bail:

		if (op == set) {
			const nat b = locate_instruction(mul, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto madd_bail;		
			const nat c = locate_instruction(add, arg0, 0, 1, 0, b + 1, ins, ins_count, names, name_count, ignore);
			if (c == (nat) -1) goto madd_bail;
						
			struct instruction new = {.op = madd};
			new.args[0] = arg0;
			new.args[1] = ins[b].args[1];
			new.args[2] = arg1;
			new.args[3] = ins[c].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1; selected[c] = 1;
			goto finish_mi_instruction;
		} madd_bail:

		if (op == set) {
			const nat b = locate_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto addsrlsl_bail;		
			const nat c = locate_instruction(add, arg0, 0, 1, 0, b + 1, ins, ins_count, names, name_count, ignore);
			if (c == (nat) -1) goto addsrlsl_bail;
			struct instruction new = { .op = addsrlsl };
			new.args[0] = arg0;
			new.args[1] = ins[c].args[1];
			new.args[2] = arg1;
			new.args[3] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1; selected[c] = 1;
			goto finish_mi_instruction;
		} addsrlsl_bail:

		if (op == set) {
			const nat b = locate_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto subsrlsl_bail;
			const nat c = locate_instruction(sub, arg0, 0, 1, 0, b + 1, ins, ins_count, names, name_count, ignore);
			if (c == (nat) -1) goto subsrlsl_bail;
			struct instruction new = { .op = subsrlsl };
			new.args[0] = arg0;
			new.args[1] = ins[c].args[1];
			new.args[2] = arg1;
			new.args[3] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1; selected[c] = 1;
			goto finish_mi_instruction;
		} subsrlsl_bail:

		if (op == set) {
			const nat b = locate_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto orrsrlsl_bail;
			const nat c = locate_instruction(or_, arg0, 0, 1, 0, b + 1, ins, ins_count, names, name_count, ignore);
			if (c == (nat) -1) goto orrsrlsl_bail;
			struct instruction new = { .op = orrsrlsl };
			new.args[0] = arg0;
			new.args[1] = ins[c].args[1];
			new.args[2] = arg1;
			new.args[3] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1; selected[c] = 1;
			goto finish_mi_instruction;
		} orrsrlsl_bail:

		if (op == set) {
			const nat b = locate_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto eorsrlsl_bail;
			const nat c = locate_instruction(eor, arg0, 0, 1, 0, b + 1, ins, ins_count, names, name_count, ignore);
			if (c == (nat) -1) goto eorsrlsl_bail;
			struct instruction new = { .op = eorsrlsl };
			new.args[0] = arg0;
			new.args[1] = ins[c].args[1];
			new.args[2] = arg1;
			new.args[3] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1; selected[c] = 1;
			goto finish_mi_instruction;
		} eorsrlsl_bail:

		if (op == set) {
			const nat b = locate_instruction(div_, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto udiv_bail;
			struct instruction new = { .op = udiv };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1;
			goto finish_mi_instruction;
		} udiv_bail:

		if (op == set) {
			const nat b = locate_instruction(si, arg0, 0, 1, 0, i + 1, 
					ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto lslv_bail;
			struct instruction new = { .op = lslv };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1;
			goto finish_mi_instruction;
		} lslv_bail:

		if (op == set) {
			const nat b = locate_instruction(add_imm, arg0, 0, 1, 0, i + 1, 
					ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto addi_bail;
			struct instruction new = { .op = addi };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1;
			goto finish_mi_instruction;
		} addi_bail:

		if (op == set) {
			const nat b = locate_instruction(sub_imm, arg0, 0, 1, 0, i + 1, 
					ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto subi_bail;
			struct instruction new = { .op = subi };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1;
			goto finish_mi_instruction;
		} subi_bail:

		if (op == set) {
			const nat b = locate_instruction(eor_imm, arg0, 0, 1, 0, i + 1, 
					ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto eori_bail;
			struct instruction new = { .op = eori };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1;
			goto finish_mi_instruction;
		} eori_bail:

		if (op == set) {
			const nat b = locate_instruction(or_imm, arg0, 0, 1, 0, i + 1, 
					ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto orri_bail;
			struct instruction new = { .op = orri };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1;
			goto finish_mi_instruction;
		} orri_bail:

		if (op == set) {
			const nat b = locate_instruction(and_imm, arg0, 0, 1, 0, i + 1, 
					ins, ins_count, names, name_count, ignore);
			if (b == (nat) -1) goto andi_bail;
			struct instruction new = { .op = andi };
			new.args[0] = arg0;
			new.args[1] = arg1;
			new.args[2] = ins[b].args[1];
			mi[mi_count++] = new;
			selected[i] = 1; selected[b] = 1;
			goto finish_mi_instruction;
		} andi_bail:

		if (op == eq_imm and arg1 == 0) {
			struct instruction new = { .op = cbnz };
			new.args[0] = arg0;
			if (ins[i].gotos[0] != i + 1) {
				new.op = cbz;
				new.args[1] = ins[i].gotos[1];
			} else {
				new.args[1] = ins[i].gotos[0];
			}
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;

		} 
		else if (op == set) {
			struct instruction new = { .op = orrsrlsl };
			new.args[0] = arg0;
			new.args[1] = 0;
			new.args[2] = arg1;			
			new.args[3] = 0;
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

		else if (op == set_imm) {
			struct instruction new = { .op = movz };
			new.args[0] = arg0;
			new.args[1] = arg1;
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

		if (op == lt) {			
			struct instruction new = { .op = subssrlsl };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = bcond };
			new2.args[0] = lt;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == eq) {			
			struct instruction new = { .op = andssrlsl };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = bcond };
			new2.args[0] = eq;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == lt_imm) {			
			struct instruction new = { .op = subsi };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = bcond };
			new2.args[0] = lt;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		if (op == eq_imm) {			
			struct instruction new = { .op = andsi };
			new.args[0] = 0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			struct instruction new2 = { .op = bcond };
			new2.args[0] = eq;
			new2.args[1] = (nat) -1;
			mi[mi_count++] = new2;
			selected[i] = 1; 
			goto finish_mi_instruction;
		}

		else {
			nat n = (nat) -1;
			if (false) {}
			else if (op == add) n = addsrlsl;
			else if (op == sub) n = subsrlsl;
			else if (op == and_)n = andsrlsl;
			else if (op == or_) n = orrsrlsl;
			else if (op == eor) n = eorsrlsl;
			else if (op == si)  n = lslv;
			else if (op == sd)  n = lsrv;
			else if (op == div_)n = udiv;
			else if (op == add_imm) n = addi;
			else if (op == sub_imm) n = subi;
			else if (op == and_imm) n = andi;
			else if (op == or_imm)  n = orri;
			else if (op == eor_imm) n = eori;
			else  { puts("ins sel error"); abort(); }

			struct instruction new = { .op = n };
			new.args[0] = arg0;
			new.args[1] = arg0;
			new.args[2] = arg1;
			new.args[3] = 0;
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

		/else if (op == sc) {
			struct instruction new = { .op = svc };
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}/


	finish_mi_instruction:;
		puts("so far:");
		print_machine_instructions(mi, mi_count, names, name_count);
		getchar();
	}

	for (nat i = 0; i < ins_count; i++) {
		if (ignore[i]) continue;
		if (not selected[i]) {
			puts("error: instruction unprocessed by ins sel: internal error");
			puts("not selected instruction: ");
			print_instruction_index(
				ins, ins_count, 
				names, name_count, ignore,
				i, "this failed to be lowered during ins sel."
			); abort();
		}
	}

	print_dictionary(names, locations, name_count);
	print_instructions(ins, ins_count, names, name_count, ignore);
	print_machine_instructions(mi, mi_count, names, name_count);
	puts("stopped after ins sel.");
	exit(0);
}


*/


















































































					//printf("skipping over: '%c'  "
					//	"(comment = %llu)\n", 
					//	text[i], comment
					//);




	//const nat label_bit = 1LLU << 63LLU;

	//byte memory[65536] = {0};






		//(false) ? "\033[38;5;101mct" : "  ", 
	//if (this.gotos[0] != (nat) -1) printf(" .0 = %-4lld", this.gotos[0]); else printf("          ");
	//if (this.gotos[1] != (nat) -1) printf(" .1 = %-4lld", this.gotos[1]); else printf("          ");
	//if ((false)) printf("\033[0m");
/*	for (nat a = arity[this.op]; a < 7; a++) {
		if (this.imm & (1 << a)) printf("         ");
		else printf("         ");
		printf(" ");
	}
*/





	/*printf("debugging memory state\n");
	for (nat i = 0; i < 1024; i++) {
		if (i % 16 == 0) puts("");
		if (memory[i]) printf("\033[32;1m");
		printf(" %02hhx ", memory[i]);
		if (memory[i]) printf("\033[0m");
	} 
	puts("\n[done]");*/



		/*printf("debugging memory state\n");
		for (nat i = 0; i < 1024; i++) {
			nat empty = 1;
			for (nat e = 0; e < 16; e++) { if (memory[i * 16 + e]) empty = 0; }
			if (empty) continue;

			printf("%08llx : ", i * 16);
			for (nat e = 0; e < 16; e++) {
				if (memory[i * 16 + e]) printf("\033[32;1m");
				printf(" %02hhx ", memory[i * 16 + e]);
				if (memory[i * 16 + e]) printf("\033[0m");
			}
			puts("");
		} 
		puts("[done]");*/


		//printf("----- AFTER PREDCESSOR CT ANALYSIS: ----\n");
		//printf("analayzed %llu predecessors,\n", pred_count);
		//printf("future_type = %llu\n", future_type);
		//printf("future_value = %llu\n", future_value);
		//getchar();

		//printf("about to assign {ins=%llu:var=%llu} = [type=%llu,value=%llu]\n", 
		//		pc, this_var, future_type, future_value);			
		//puts("continue?");
		//getchar();




/*if (not ct0 and (label_bit & v0)) {
				//puts("STORING TO THE .TEXT SECTION OF THE EXECUTABLE!!!!"); getchar();

				if (not ct1) {
					puts("FATAL ERROR: the data being stored to the .text section of the executable "
						"is not compiletime-known, and thus this store is impossible to perform."
					);
					abort();
				}

				const nat address = v0 ^ label_bit;

				//printf("address = %llu, data = %llu, size = %llu\n", address, v1, v2); getchar();

				for (nat i = 0; i < v2; i++) memory[address + i] = (v1 >> (8 * i)) & 0xFF;

			} else {
				puts("found a runtime store!");
				//getchar();
			}*/


			//printf("found a compiletime known value == %llu\n", v);


			//printf("%llu: observed {type=%llu,value=%llu} pair "
			//	"for predecessor = %llu\n", 
			//	this_var, t, v, pred
			//);
			//getchar();

			/// at this point, we could in theory do partial-constant-propgation-analysis, if we wanted to.
			/// we would simply keep track of the set of possible values from the preds, instead of just one ctk value.
			//puts("MISMATCH OCCURED!!!! we will prefer which values came from last_pc!!!");
			//puts("WAIT!!! LAST_PC == PRED!!!!");
			//puts("does this mean control came from last_pc??...");
			//puts("we should probably prefer the value state from this pred over the others....");
			//printf("setting future type and value to t%llu and v%llu, and breaking!\n", t, v);
			//getchar();








			//puts("executing a LD instruction!"); getchar();

			/*if (not ct1 and (label_bit & v1)) {
				//puts("(CT or RT) LOADING FROM THE .TEXT SECTION OF THE EXECUTABLE!?!!?!?"); getchar();
			}*/




		/*printf("[pc=%llu]:"
			" w=\"%s\" w_st=%llu\n", 
			pc, word, word_start
		);
		print_dictionary(variables, is_undefined, var_count);
		print_instructions(ins, ins_count, variables, 1);
		*/






/* 


	new todo: 1202504115.002432
---------------------------------------------------



		. figure out how to add back in    the   udf   and eoi    functionality...?


		. we should remove the entire    label-based stores are ct   thingy??? 


		. and just make ld and st ALWAYS runtime. always. 
			i think we actually do still need the notion of label based computations, though. 
			because computating an offset is quite useful actually. 
			its just.. i don't think that storing to a label should be allowed? idk. at least, it shouldnt neccessarily be done at compiletime. it should just trigger a segementation fault instead, or be valid if we are on msp430 and using FRAM lol. 

		. to get ct loads and stores, lets just leave that feature out for now? hmmmm


		





	missing features still:
---------------------------------------------------

		. we need some way of allocating some read-only bytes in the executable!      eg,         allocate 4        to allocate 4 bytes

		. we then need to find some way of making  loads  compiletime executed.   
			becuase currently they are only ever runtime. 

		. we need to find some way of seperating out the index space   of the various labels and the memory regions they represent. 
			because currently they all share the exact same memory array. and this isnt quite sound to do, i think... 
				we should be doing bounds checks on all loads and stores to the executable, 
				to make sure they that they lie within the allocated region.  this should be pretty easy. 


		. 




isa:
	halt sc 

	do at lf 

	set add sub mul div rem

	and or eor si sd 

	la rt

	ld st 

	lt ge ne eq





meaning/usage of each instruction:
----------------------------------------

	halt : termination point in the control flow graph. control flow does not continue past this instruction. 
	sc : system call, target specific, and triggers a context switch on targets with an operating system, to perform a specialized task. 
	do k : unconditional graph to label k. 
	at k : attribute label k to this position in the code. k is used as the destination of branches, or with the source of an la instruction.
	lf f : load file f from the filesystem, and include its parsed contents here.
	rt x y : force the variable x to be runtime known. 
		if y > 0, this sets the number of bits allocated to x, and 
		if y == 0, x is forced to be runtime known, with no further constraints, and
		if y < 0, this denotes the hardware register x should be allocated in. 
	set x y : assignment to destination register x, using the value present in source y.
	add x y : assigns the value x + y to the destination register x.
	sub x y : assigns the value x - y to the destination register x.
	mul x y : assigns the value x * y to the destination register x.
	div x y : assigns the value x / y to the destination register x.
	rem x y : assigns the value x modulo y to the destination register x.
	and x y : assigns the value x bitwise-AND y to the destination register x.
	or x y : assigns the value x bitwise-OR y to the destination register x.
	eor x y : assigns the value x bitwise-XOR y to the destination register x.
	si x y : shifts the bits in x up by y bits. 
	sd x y : shifts the bits in x down by y bits. (always an unsigned shift)
	la x k : loads a program-counter relative address given by a label k into a destination register x.
	ld x y z : load z bytes from memory address y into destination register x. 
	st x y z : store z bytes from the soruce register y into the memory at address x.
	lt x y k : if x is less than y, control flow branches to label k. 
	ge x y k : if x is not less than y, control flow branches to label k. 
	ne x y k : if x is not equal to y, control flow branches to label k. 
	eq x y k : if x is equal to y, control flow branches to label k. 
	


current:

	24 instructions:

	halt sc   do   at   lf   rt
	set  add  sub  mul  div  rem
	and  or   eor  si   sd   la
	ld   st   lt   ge   ne   eq




old:
	25 instructions:

	halt do   at   lf   set 
	add  sub  mul  div  rem 
	and  or   eor  si   sd 
	ri   bc   ld   st   lt 
	ge   ne   eq   la   sc



notes:


	1. we are still midding "udf" functionality. 


x	2. bc <varname> 0 is used to define a ct variable.                 <------- DELETE THISSSS
			....although, usually this can be deduced i think?... 

				via constant propagation. basically. yeah. and TRUE ct analysis. 


		this ct attribution method should only be required when we are dealing with loops / control flow, 

				and we want to force  the compiler to execute something rt at compiletime anyways lol. 

	
x	3. "do label" is always runtime. 

		(the user never writes this like this, if they wanted a ct label instead lol.)

			...if you want a CT unconditional branch, use "eq 0 0 label" instead. 



	4. ri, bc, set, ld, at, la, do, lt, eq, ge, ne   all can define-on-use some of their arguments. 

				la(1) at(0) do(0) lt(2) ge(2) ne(2) eq(2)    are all related to labels. so these stay. 

				set(0) ld(0) la(0)    these are about defining a value, via an assignment. so these stay. 



		butttt	i'm debating on removing       ri   and bc   from that list though lol... 

									technically not neccessary to be here... hmmm
				


*/











// current state: 1202504104.000033

/*

	we were just about to add the notion of control flow into this const prop!

	the way we are giong to do this is:

			we are going to tell the cfg node we are going to    how we got there. whether it was a result of a ct or rt decision. 


				(note: going via an implicit ip++ is ctk cf)

			and then, the cool part is that we select either   a given  var state   types/values  array    from a given pred

												or select another, 


					based on which pred we came from!!!


							ifffffff our  ct/rt    bit decision  is       ct 


							then we use this   came-from-pred   information  
								to help select the   type/values   array  we want to use, for our prior values, to update what we think the state of the variables is. 






	this is how we are goign to add cf to the const prop alg.


*/
















































/*
		if (gt0 < ins_count and visited[gt0] < 3) stack[stack_count++] = gt0; // specialize this per ins. 
		if (gt1 < ins_count and visited[gt1] < 3) stack[stack_count++] = gt1;


		///  THESEEEE (the above lines) AREEEE THE CONTROL FLOW hAPPENIONG. 

			we need to push    ALSO      to the stack.       a bit     of whether we pushed this because of a rt decsion, (ie, pushing both gt0 and gt1 , because we didnt know which would really happen)

					orrrr          because of a ct decision, where only pushed one branch side,  say, gt1, or something, 


										because we KNOWWW control flow will traverse to that side based on the conditin! 

					and so, 

						when we pop,  at the top of the while (stack_count) loop,  we pop the rt/ct decision bit 

								as well 


						and based on this, we know whether or not we came here   from a    ct control flow edge!!


									THIS IS CRUCIAL PRIOR INFORMATION.


				

		now, the problem, is, i don't really know how to synthesize this with other information lol...  like,  to propgate this rt bit further down into the instruction stream... hmmm 

	i don't know that yet lol 


	bu tyeah we are getting closerrr


yayy


*/






		/*	r += r;     { 0 }
			r += c;     { 0 }
			c += r;     { 0 }
			c += c;     { 1, c+c }
		*/

/*



----------------------------------------------------------------------------------------------------------------------

	visted[I] == 0       : means that this statement never executes.  takes priority over other data. 

----------------------------------------------------------------------------------------------------------------------

	visted[I] != 0 and 
	type[I * var_count + V] == 0  : means that the variable V is runtime known before the statement I exectues.

----------------------------------------------------------------------------------------------------------------------

	visted[I] != 0 and 
	type[I * var_count + V] != 0 and
	value[I * var_count + V] == K   : means that the variable V is compiletime known, 
							and has value K at this point.	

----------------------------------------------------------------------------------------------------------------------



*/












/*
------------------------------------------------------------------------------------------
	FACTS:
------------------------------------------------------------------------------------------

	1. if there is at least one pred which has a runtime-only known value for the variabel X, 
		then we must assume X is runtime-only known. 

	2. if there are NO preds which have a runtime-only attribute on X,  
		then we can and must assume X is compiletime known. 

	3. if there are NO preds with RTK, AND all preds have CTK attribute on X, 
		BUTTT   the values are not all the same, on all preds,
			THENNNN we know that, either:

			[
				3.1. there is compiletime looping happening here, if the conditions involved in this control flow
					are CTK as well, 
	
				3.2. or, if there conditions are not CTK, (ie, they are RTK only) then we know that 

					we must PROMOTE X   (technically "demote" lolll)    
						from CTK (with multi-value)   to RTK. (completely unknown value)

						note: if we really want to try-hard   then we can keep track of the possible 
						CTK values that a variable could have. this doesnt work with loops at all though. note that. 	

			]


	4. if a given pred is not executed at all,  it does not have a say in the voting process. 
			it can be ignored, as if it isnt even a pred at all. 

*/

























/*
	nat register_constraint[4096] = {0};
	nat bit_width[4096] = {0};



	byte memory[65536] = {0};



	struct instruction machine_instructions[4096] = {0};
	nat machine_instruction_count = 0;
	puts("starting instruction selection now.....");

*/
















/*

	//for (nat i = 0; i < var_count; i++) is_compiletime[i] = 1;

	while (stack_count) {
		nat pc = stack[--stack_count];

	execute_ins:
		if (pc >= ins_count and not stack_count) break;
		visited[pc] = 1;

		if (*values or true) {
			print_instruction_window_around(pc, ins, ins_count, variables, visited);
			print_dictionary(variables, is_undefined, var_count);
			printf("stack: "); print_nats(stack, stack_count);
			printf("[PC = %llu]\n", pc);
			getchar();
		}

		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;

		const nat gt0 = ins[pc].gotos[0];
		const nat gt1 = ins[pc].gotos[1];

		const nat a0 = ins[pc].args[0];
		const nat a1 = ins[pc].args[1];
		const nat a2 = ins[pc].args[2];

		const nat ct0 = (imm & 1) ? 1 : is_compiletime[a0];
		const nat ct1 = (imm & 2) ? 1 : is_compiletime[a1];
		const nat ct2 = (imm & 4) ? 1 : is_compiletime[a2];

		const nat lb1 = (imm & 2) ? 0 : is_label[a1];

		const nat val0 = (imm & 1) ? a0 : values[a0];
		const nat val1 = (imm & 2) ? a1 : values[a1];
		const nat val2 = (imm & 4) ? a2 : values[a2];

		if (op == halt) {}
		else if (op == sc) {}
		else if (op >= a6_nop and op < isa_count) { }

		else if (op == at) { values[a0] = pc; pc++; goto execute_ins; } 
		else if (op == do_) { pc = values[a0]; goto execute_ins; }

		else if (op == la) {
			is_label[a0] = 1;
			values[a0] = 4 * values[1];
		}

		else if (
			op == lt or op == ge or
			op == ne or op == eq
		) {
			if (ct0 and ct1) {
				bool cond = 0;
				if (op == lt)      cond = val0 < val1;
				else if (op == ge) cond = val0 >= val1;
				else if (op == eq) cond = val0 == val1;
				else if (op == ne) cond = val0 != val1;
				if (cond) pc = values[a2]; else pc++;
				goto execute_ins;

			} else if (ct0) {
				ins[pc].args[0] = val0;
				ins[pc].imm |= 1;

			} else if (ct1) {					
				ins[pc].args[1] = val1;
				ins[pc].imm |= 2;
			}

			if (not visited[gt0]) stack[stack_count++] = gt0;
			if (not visited[gt1]) stack[stack_count++] = gt1;

		} else if (
			op == set or op == add or 
			op == sub or op == mul or 
			op == div_ or op == rem or
			op == and_ or op == or_ or
			op == eor or op == si or op == sd
		) {
			is_label[a0] = lb1;

			if (op == set) 
				is_compiletime[a0] = ct1;
			else 
				is_compiletime[a0] = ct0 and ct1;

			if (op == set)       values[a0]  = val1;
			else if (op == add)  values[a0] += val1;
			else if (op == sub)  values[a0] -= val1;
			else if (op == mul)  values[a0] *= val1;
			else if (op == div_) values[a0] /= val1;
			else if (op == rem)  values[a0] %= val1;
			else if (op == and_) values[a0] &= val1;
			else if (op == or_)  values[a0] |= val1;
			else if (op == eor)  values[a0] ^= val1;
			else if (op == si)   values[a0] <<= val1;
			else if (op == sd)   values[a0] >>= val1;
			pc++; goto execute_ins;			

		} else if (op == ld) {
			if (not ct2) goto ct_error;
			if (ct0) {
				values[a0] = 0;
				for (nat i = 0; i < val2; i++) 
					values[a0] |= (nat) ((nat) memory[val1 + i] << (8LLU * i));
			}
			pc++; goto execute_ins;

		} else if (op == st) {
			if (not ct2) goto ct_error;
			if (is_label[a0]) {
				for (nat i = 0; i < val2; i++) 
					memory[val0 + i] = (val1 >> (8 * i)) & 0xFF;
			}
			pc++; goto execute_ins;
			
		} else if (op == rt) {
			if (not ct1 or not ct2 or ct0) goto ct_error;
			is_compiletime[a0] = not val1;
			bit_width[a0] = val1;
			register_constraint[a0] = val2;
			pc++; goto execute_ins;

		} else {
			printf("ERROR: unknown instsruction executing/analyzing...\n");
			abort();
		}

		continue;

	ct_error: 
		puts("error: unknown ct exec semantics"); 
		abort();
	}






	print_dictionary(
		variables, is_undefined,
		is_compiletime, values, var_count
	);

	print_instructions(ins, ins_count, variables);
	print_nats(visited, ins_count);
	struct instruction machine_instructions[4096] = {0};
	nat machine_instruction_count = 0;
	puts("starting instruction selection now.....");

	puts("done!");
	exit(0);
}


*/







































































/*


if (ct) {
				const nat n = val0;
				const nat x0 = val1;

				if (n == 0) { 
					print_binary(x0); 
					fflush(stdout); 
					if (*values) getchar(); 

				} else if (n == 1) { 
					printf("[CTPAUSE]"); 
					fflush(stdout); 
					getchar(); 

				} else if (n == 2) {
					printf("debugging memory state\n");
					for (nat i = 0; i < 1024; i++) {
						if (i % 16 == 0) puts("");
						if (memory[i]) printf("\033[32;1m");
						printf(" %02hhx ", memory[i]);
						if (memory[i]) printf("\033[0m");
					} 
					puts("[done]");	
					fflush(stdout);
					if (*values) getchar();

				} else goto ct_error;

				pc++; goto execute_ins;
			} else {
				for (nat i = 0; i < 7; i++) {
					if (not (imm & (1 << i)) and is_compiletime[ins[pc].args[i]]) {
						ins[pc].args[i] = values[ins[pc].args[i]];
						ins[pc].imm |= (1 << i);
					}
				}
			}





	for (nat pc = 0; pc < ins_count; pc++) {

		const nat op = ins[pc].op;
		const nat ct = ins[pc].ct;
		//const nat imm = ins[pc].imm;
		//const nat gt0 = ins[pc].gotos[0];
		//const nat gt1 = ins[pc].gotos[1];
		//const nat a0 = ins[pc].args[0];
		//const nat a1 = ins[pc].args[1];
		//const nat a2 = ins[pc].args[2];

		if (ct) { puts("skipping over CT instruction...\n"); continue; }

		if (op == set) {

		}		
	}


else {
				if (ct0) {
					ins[pc].args[0] = val0;
					ins[pc].imm |= 1;
				} if (ct1) {
					ins[pc].args[1] = val1;
					ins[pc].imm |= 2;
				} if (ct2) {
					ins[pc].args[2] = val2;
					ins[pc].imm |= 4;
				}
			}

else {
				if (ct1) {
					ins[pc].args[1] = val1;
					ins[pc].imm |= 2;
				}
			}


else {
				if (ct1) {
					ins[pc].args[1] = val1;
					ins[pc].imm |= 2;
				} if (ct2) {
					ins[pc].args[2] = val2;
					ins[pc].imm |= 4;
				}
			}


else {
				if (ct1) {
					ins[pc].args[1] = val1;
					ins[pc].imm |= 2;
				} if (ct0) {
					ins[pc].args[0] = val0;
					ins[pc].imm |= 1;
				} if (ct2) {
					ins[pc].args[2] = val2;
					ins[pc].imm |= 4;
				}
			}



else {
				for (nat i = 0; i < 7; i++) {
					if (not (imm & (1 << i)) and is_compiletime[ins[pc].args[i]]) {
						ins[pc].args[i] = values[ins[pc].args[i]];
						ins[pc].imm |= (1 << i);
					}
				}
			}



else {
				if (ct1) {					
					ins[pc].args[1] = val1;
					ins[pc].imm |= 2;
				} if (ct0) {
					ins[pc].args[0] = val0;
					ins[pc].imm |= 1;
				}
			}




*/


// TODO:  WE NEED TO STORE THE IMMEDIATE INTO THE RT_INSTRUCTION!!!

				//       setting the   imm  bit  accordingly!!    ie, its as if we gave a binary literal. 
				// 					but we do this on every single argument.

				
				// we don't need to change the op code, i think...?  hmmmm

/*

1202504082.012807 dwrr

ins sel thoughts:



	basic approach to ins sel:

		. don't deal with branches at all yet, just deal with straight line code!

		. wea re basicallyy forming SSA representation, just without any of the control flow horribleness of ssa. only data flow alone. 

		. make the representation keep track of the data dependancies. if there is 
			a use later on of a temporary, this aborts the pattern technically speaking. 

		. we need to be keeping track of a tree/dag representation of the entire  feed forward  controlflownless program. 
			we then do tree/dag pattern matching on this monolithic dag.

			note, due to the ssa-like nature of this representation, there is no reusing or resetting of any variables. single assigement, 
				ie, data does not neccessary live in any registers. rather it lives in the abstract, as a value result from a particular operation. 
				this represetnation also allows for easier time finding common subexpression elimintation. 

							note: this rep is only good when theres no control flow. very important to note that. 

								ssa is not good when control flow is involved, which it always will be in practice. lol 

		. operations which are commutitive, ie, ins which can be exchanged will always be put into a sorted caonicalized order. 
			not sure how.. but this neeeds to happen.. i think we should probably have some "sequence" modifier, which states that certain operations can and cannot happen in sequence or not..?

					oh wait no!! ssa means we don't need to take into account sequence at all, because the data dependancy actually represents that completel for us. there is no notion of seuqence of things, when dealing with this ssa like respresentation. nice!!!  its just dag connections. data dependancies impose the minimal ordering over the ins listing. NICEEEE





		. in fact, in the process of forming the ssa rep, using a sort of "global value numbering",   we can actually preform a value numbering, which keeps track of.. essentially like a hash value for every possible computation, and if two registers have the same hash value, ie, are redundant expressions, then we can merge them. 

			i'll probably leave this out though, because common sub expression elimination isnt really a super important optimization really. so yeah. 

	
		. note: the ssa used here really doesnt actually have virtual registers, rather, we can just make some way of referencing points in a tree/dag.
			which i think will end up looking like some notion of registers i think lol. but yeah. they are technically speaking distinct. 



		. note, for optimization, we use our stateful language/ir,   but for ins sel, we use this   ssa-like  rep.

			the idea is, we need to find an abstract data representation which showcases what computation has been done on a given variable, 
				but without actaully having any notion of order/sequence, and no notion of registers, and statefulness, as this is also not relevant to ins sel. 
						ssa is truly the best rep for ins sel      but cruddy for every other aspect of compilers lolll

							so yeah 



						oh, and actualy we can't even actually just use ssa in ins sel because it doesnt deal with control flow lollll

					so yeah


						cool yay

			oh wait! constant propagation alsoooo is easy once we have this ssa like rep! wow niceee


				


	
	in fact we can do a lot of optimization from this type of appoach i think!

		OH WOW


			wait two things



				1. use loads        ld     and   st         to get trulyyyy rtk variables to play with during tetisng for ins sel and const prop

				2.  treat adds     as         k ary       adds      ie,   with k inputs       not     NOTTTTT    two inputs.      k!!!

			


		for 1, the idea is  loads from memory are volatile/rtk, (for now lol) and thus   will force rt data to not collapse via const prop.

		for 2,  the idea is we don't want to inhibit  the merging of commutitive adds together,  

			howeverrrr we shouldd actually probably seperate out    ct adds from rt adds? hmmm not sure.... they are quite different... hmmm

		




		wait!   turns out i think this dag ssa rep needs to be computatoinally defined!    ie, not stored. 



				it should be sometithng we computeeee and thats it   we only ever store the ins   the actual ir ins    notttt the ssa dag itself



					and then, we just need some way of making the compiler generate like... a data state, which represetns the tree!!!


						i feel like that genuinely could be on the right track!!!1


						its like a hash. or something. 

					like, something which basically has the     hmm      the full semantics     of a ssa-like   dag           all in a single number. 


						then we just need to check for this number in the ins sel! pattern recognition is as simple as that, then 

					if we can generate this       "ssa" number    that represents the dag lol


									hmmmmm interestingggg wowwww




					like, i feel like, it critically relies   firstttt   on making every single data variable, and data point in the program have its own number 

						and thennnn  we make every single operation have its own number too 

						and then we just use those nmubers to    transform a given prior input number     in various ways   to get a new number 

						like????


									i feel like this is actually the way!?!?!



		we need to nail down the ins sel number formation though. thats the key. 

			we need to turn a dag into a single number.  or like.. a sequence of bits lol. hmmm


			and for that, we need to know how many possible data points in the program there are. 

			so maybe we should start there? lol.. hmmm




	hmmm.. i kinda want it to not.. use the constants to affect the number though.. idk.. hmmmmm
	
			although i mean it does make sense to do that though lol.. 

	i just.. i feel like we need to abstract ovre the actual immediates use kinda   at least a little bit 
		although that does definitely influence instruction selection lol... because like, some patterns are actualy specific to particular constants.. thats the thing.. hmmmm



	intterestingggggggg hmmm






					












*/






/*

else if (
			op == set or op == add or 
			op == sub or op == mul or 
			op == div_ or op == rem or
			op == and_ or op == or_ or
			op == eor or op == si or op == sd
		) 


*/































/*	while (stack_count) {

		//const nat pc = stack[--stack_count];
		nat pc =0 ;

		if (values[0]) {
			print_instruction_window_around(
				pc, ins, ins_count, variables, visited
			);
			print_dictionary(variables, is_undefined, 
				is_compiletime, values, var_count
			);
			getchar();
		}

		const nat op = ins[pc].op;
		nat ct = ins[pc].ct;
		const nat imm = ins[pc].imm;

		const nat gt0 = ins[pc].gotos[0];
		const nat gt1 = ins[pc].gotos[1];

		const nat a0 = ins[pc].args[0];
		const nat a1 = ins[pc].args[1];
		const nat a2 = ins[pc].args[2];

		const nat ct0 = (imm & 1) ? 1 : is_compiletime[a0];
		const nat ct1 = (imm & 2) ? 1 : is_compiletime[a1];
		const nat ct2 = (imm & 4) ? 1 : is_compiletime[a2];
		
		const nat val0 = (imm & 1) ? a0 : values[a0];
		const nat val1 = (imm & 2) ? a1 : values[a1];
		const nat val2 = (imm & 4) ? a2 : values[a2];

		if (op == halt) {

		} else if (op == at) {
			values[a0] = pc;

		} else if (op == do_) {
			if (ct) { pc = values[a0]; continue; }
			
		} else if (
			op == lt or op == ge or
			op == ne or op == eq
		) {
			if (ct0 and ct1) ct = 1;
			if (ct) {
				bool cond = 0;
				if (op == lt)      cond = val0 < val1;
				else if (op == ge) cond = val0 >= val1;
				else if (op == eq) cond = val0 == val1;
				else if (op == ne) cond = val0 != val1;
				if (cond) { pc = values[a2]; continue; }

			}
			

		} else if (
			op == set or op == add or 
			op == sub or op == mul or 
			op == div_ or op == rem or
			op == and_ or op == or_ or
			op == eor or op == si or op == sd
		) {
			if (ct0 and ct1) ct = 1;
			else if (ct0) goto ct_error;
			if (ct) {
				if (op == set) is_compiletime[a0] = 1;
				const nat s = val1;

				if (op == set)       values[a0] = s;
				else if (op == add)  values[a0] += s;
				else if (op == sub)  values[a0] -= s;
				else if (op == mul)  values[a0] *= s;
				else if (op == div_) values[a0] /= s;
				else if (op == rem)  values[a0] %= s;
				else if (op == and_) values[a0] &= s;
				else if (op == or_)  values[a0] |= s;
				else if (op == eor)  values[a0] ^= s;
				else if (op == si)   values[a0] <<= s;
				else if (op == sd)   values[a0] >>= s;				
			}

		} else if (op == ld) {
			if (not ct2) goto ct_error;
			values[a0] = 0;
			for (nat i = 0; i < val2; i++)
				values[a0] |= (nat) ((nat) memory[val1 + i] << (8LLU * i));

		} else if (op == st) {
			if (not ct2) goto ct_error;
			for (nat i = 0; i < val2; i++) 
				memory[val0 + i] = (val1 >> (8 * i)) & 0xFF;

		} else if (op == ri) {
			if (ct0) goto ct_error;
			register_constraint[a0] = val0;

		} else if (op == bc) {
			if (ct0) goto ct_error;
			bit_width[a0] = val0;

		} else if (op == sc) {

			if (ct) {
				const nat n = val0;
				const nat x0 = val1;

				if (n == 0) { 
					print_binary(x0); 
					fflush(stdout); 
					if (*values) getchar(); 

				} else if (n == 1) { 
					printf("[CTPAUSE]"); 
					fflush(stdout); 
					getchar(); 

				} else if (n == 2) {
					printf("debugging memory state\n");
					for (nat i = 0; i < 1024; i++) {
						if (i % 16 == 0) puts("");
						if (memory[i]) printf("\033[32;1m");
						printf(" %02hhx ", memory[i]);
						if (memory[i]) printf("\033[0m");
					} 
					puts("[done]");	
					fflush(stdout);
					if (*values) getchar();

				} else goto ct_error;
			}
		} else goto ct_error;

		


		pc++;	
		continue;

		ct_error: puts("error: unknown ct exec semantics"); 
		abort();
	}

	print_dictionary(
		variables, is_undefined,
		is_compiletime, values, var_count
	);

	print_instructions(ins, ins_count, variables);

	// here, we need to form the control flow graph over these rt instructions, and do data flow analysis.  





	// then we can do instruction selection. 

	// then register allocation, 



	// ...		and then special case stuff like  bit widths, register constraints, strings, and other stuff like that lol.



*/































































/*

		} else if (op == set) {

			if (ct0 and ct1) ct = 1;                  // set ct ct    (ct set)
			else if (ct0) goto ct_error;              // set ct RT    (invalid!)
			else if (ct1) {}                     	  // set RT ct    (set_imm)
			else {}					  // set RT RT    (rt set)

			if (ct) values[a0] = values[a1];
			else rt_ins[rt_count++] = ins[pc];


*/











/*

		} else if (op == add) {

			if (ct0 and ct1) ct = 1;                  // add ct ct    (ct add)
			else if (ct0) goto ct_error;              // add ct RT    (invalid!)
			else if (ct1) {}                     	  // add RT ct    (add_imm)
			else {}					  // add RT RT    (rt add)

			if (ct) values[a0] += values[a1];
			else rt_ins[rt_count++] = ins[pc];


*/






































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













			/*if (val2 == 1) {
				values[a0] = 0;
				values[a0] |= (nat) (memory[val1 + 0] << (8 * 0));

			} else if (val2 == 2) {
				values[a0] = 0;
				values[a0] |= (nat) (memory[val1 + 0] << (8 * 0));
				values[a0] |= (nat) (memory[val1 + 1] << (8 * 1));

			} else if (val2 == 4) {
				values[a0] = 0;
				values[a0] |= (nat) (memory[val1 + 0] << (8 * 0));
				values[a0] |= (nat) (memory[val1 + 1] << (8 * 1));
				values[a0] |= (nat) (memory[val1 + 2] << (8 * 2));
				values[a0] |= (nat) (memory[val1 + 3] << (8 * 3));

			} else if (val2 == 8) {
				values[a0] = 0;
				values[a0] |= (nat) ((nat) memory[val1 + 0] << (8LLU * 0LLU));
				values[a0] |= (nat) ((nat) memory[val1 + 1] << (8LLU * 1LLU));
				values[a0] |= (nat) ((nat) memory[val1 + 2] << (8LLU * 2LLU));
				values[a0] |= (nat) ((nat) memory[val1 + 3] << (8LLU * 3LLU));
				values[a0] |= (nat) ((nat) memory[val1 + 4] << (8LLU * 4LLU));
				values[a0] |= (nat) ((nat) memory[val1 + 5] << (8LLU * 5LLU));
				values[a0] |= (nat) ((nat) memory[val1 + 6] << (8LLU * 6LLU));
				values[a0] |= (nat) ((nat) memory[val1 + 7] << (8LLU * 7LLU));
			}*/


			/*if (val2 == 1) {
				memory[val0 + 0] = (val1 >> (8 * 0)) & 0xFF;

			} else if (val2 == 2) {
				memory[val0 + 0] = (val1 >> (8 * 0)) & 0xFF;
				memory[val0 + 1] = (val1 >> (8 * 1)) & 0xFF;

			} else if (val2 == 4) {
				memory[val0 + 0] = (val1 >> (8 * 0)) & 0xFF;
				memory[val0 + 1] = (val1 >> (8 * 1)) & 0xFF;
				memory[val0 + 2] = (val1 >> (8 * 2)) & 0xFF;
				memory[val0 + 3] = (val1 >> (8 * 3)) & 0xFF;

			} else if (val2 == 8) {
				memory[val0 + 0] = (val1 >> (8 * 0)) & 0xFF;
				memory[val0 + 1] = (val1 >> (8 * 1)) & 0xFF;
				memory[val0 + 2] = (val1 >> (8 * 2)) & 0xFF;
				memory[val0 + 3] = (val1 >> (8 * 3)) & 0xFF;
				memory[val0 + 4] = (val1 >> (8 * 4)) & 0xFF;
				memory[val0 + 5] = (val1 >> (8 * 5)) & 0xFF;
				memory[val0 + 6] = (val1 >> (8 * 6)) & 0xFF;
				memory[val0 + 7] = (val1 >> (8 * 7)) & 0xFF;
			}*/












