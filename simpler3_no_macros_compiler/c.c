// a compiler for my programming language
// written on 1202507034.195016 by dwrr




/*
1202507056.004138
	todo: riscv isel:

		. large immediates with operations
		. stores and loads
		. load label address

		. divide by constant
		. remaider/modulo by constant
		. multiply by a constant
















rv isel   
current state:  1202505235.133756



		set s r
		addi s k
		st d s 64


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

static nat debug = 1;

#define max_variable_count 	(1 << 14)
#define max_instruction_count 	(1 << 14)
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
	compiler_target,
	compiler_format,

	compiler_should_overwrite,
	compiler_should_debug,
	compiler_stack_size,

	compiler_get_length,
	compiler_is_compiletime,

	compiler_arg0,
	compiler_arg1,
	compiler_arg2,
	compiler_arg3,
	compiler_arg4,
	compiler_arg5,
	compiler_arg6,
	compiler_arg7,

	compiler_base,
};

enum language_system_calls {
	compiler_system_debug,
	compiler_system_exit,
	compiler_system_read,
	compiler_system_write,
	compiler_system_open,
	compiler_system_close,
};


// set add sub mul div rem and or eor si sd str reg bits
// ld st adr emit lt ge ne eq at do sc halt ct rt file del


enum core_language_isa {
	nullins,

	set, add, sub, mul, div_, rem, 
	and_, or_, eor, si, sd, str, reg, bits,
	ld, st, adr, emit, lt, ge, ne, eq, 
	at, do_, sc, halt, ct, rt, file, del,

	a6_nop, a6_svc, a6_mov, a6_bfm,
	a6_adc, a6_addx, a6_addi, a6_addr, a6_adr, 
	a6_shv, a6_clz, a6_rev, a6_jmp, a6_bc, a6_br, 
	a6_cbz, a6_tbz, a6_ccmp, a6_csel, 
	a6_ori, a6_orr, a6_extr, a6_ldrl, 
	a6_memp, a6_memia, a6_memi, a6_memr, 
	a6_madd, a6_divr, 

	m4_op, m4_br,
	r5_r, r5_i, r5_s, r5_b, r5_u, r5_j, 
	isa_count
};

static const char* operations[isa_count] = {
	"___nullins____",

	"set", "add", "sub", "mul", "div", "rem", 
	"and", "or", "eor", "si", "sd", "str", "reg", "bits",
	"ld", "st", "adr", "emit", "lt", "ge", "ne", "eq", 
	"at", "do", "sc", "halt", "ct", "rt", "file", "del",

	"a6_nop", "a6_svc", "a6_mov", "a6_bfm",
	"a6_adc", "a6_addx", "a6_addi", "a6_addr", "a6_adr", 
	"a6_shv", "a6_clz", "a6_rev", "a6_jmp", "a6_bc", "a6_br", 
	"a6_cbz", "a6_tbz", "a6_ccmp", "a6_csel", 
	"a6_ori", "a6_orr", "a6_extr", "a6_ldrl", 
	"a6_memp", "a6_memia", "a6_memi", "a6_memr", 
	"a6_madd", "a6_divr", 

	"m4_op", "m4_br",

	"r5_r", "r5_i", "r5_s", "r5_b", "r5_u", "r5_j", 
};

static const nat arity[isa_count] = {
	0,

	2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 0, 2, 2,
	3, 3, 1, 2, 3, 3, 3, 3, 
	1, 1, 0, 0, 0, 0, 1, 1,
	
			
	0, 0, 5, 7, 
	6, 8, 7, 8, 3,
	5, 4, 4, 2, 2, 3, 
	4, 4, 7, 7, 
	5, 8, 5, 3, 
	7, 6, 5, 6,
	8, 5, 

	8, 2,

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
	byte ct;

	uint8_t  unused0;
	uint16_t unused1;
	uint32_t unused2;
};

struct expected_instruction {
	nat op;
	nat imm;
	nat use;
	nat args[max_arg_count];	
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

static void print_dictionary(nat should_print_ct) {
	puts("variable dictionary: ");
	for (nat i = 0; i < var_count; i++) {
		should_print_ct = ((var_count - i) < 20);
		if (should_print_ct or (not is_constant[i] and not is_label[i])) 
		printf("   %c %c %c %c [%5llu]  \"%20s\"  :   { bc=%5lld, ri=%5lld }  :   0x%016llx (%llu decimal)\n",
			' ', 
			is_label[i] ? 'L' : ' ', 
			is_constant[i] ? 'C' : ' ', 
			is_undefined[i] ? 'U' : ' ', 
			i, variables[i], 
			bit_count[i], register_index[i], 
			values[i], values[i]
		);
	}
	puts("[end]");
}

static void print_instruction(struct instruction this) {

	//if (this.state) printf("\033[33m");

	int max_name_width = 0;
	for (nat i = 0; i < var_count; i++) {
		if (max_name_width < (int) strlen(variables[i])) {
			max_name_width = (int) strlen(variables[i]);
		}
	}

	printf("  %4s    ", operations[this.op]);

	for (nat a = 0; a < arity[this.op]; a++) {

		char string[4096] = {0};
		if (this.imm & (1 << a)) snprintf(string, sizeof string, "0x%llx(%llu)", this.args[a], this.args[a]);
		else if (this.args[a] < var_count) snprintf(string, sizeof string, "%s", variables[this.args[a]]);
		else snprintf(string, sizeof string, "(INTERNAL ERROR)");

		printf("%s", string);
		int left_to_print = max_name_width - (int) strlen(string);
		if (left_to_print < 0) left_to_print = 0;
		for (int i = 0; i < left_to_print; i++) putchar(' ');
		putchar(' ');
	}

	//if (this.state) printf("\033[0m");

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

static void print_index(const char* text, nat text_length, nat begin, nat end) {
	printf("\n\t@%llu..%llu: ", begin, end); 
	while (end and isspace(text[end])) end--;
	const nat max_width = 100;
	nat start_at = 
		begin < max_width ? 0 : begin - max_width;

	nat end_at = 
		end + max_width >= text_length 
		? text_length : end + max_width;

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
			gotos[2 * i + 1] = locations[ins[i].args[4]];

		} else if (op == r5_j) {
			gotos[2 * i + 0] = locations[ins[i].args[2]];
			gotos[2 * i + 1] = (nat) -1;

		} else if (	op == r5_i or 
				op == r5_u or 
				op == r5_s or 
				op == r5_r or 

				op == at or 
				op == adr or 
				op == emit
		) {
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
			gotos[2 * i + 1] = locations[ins[i].args[4]];

		} else if (op == r5_j) {
			gotos[2 * i + 0] = locations[ins[i].args[2]];
			gotos[2 * i + 1] = (nat) -1;

		} else if (	op == r5_i or 
				op == r5_u or 
				op == r5_s or 
				op == r5_r or 

				op == at or 
				op == adr or 
				op == emit
		) {
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

		} else if (op == adr or op == m4_op or op == at or op == emit) {
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

		} else if (op == adr or op == m4_op or op == at or op == emit) {
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

	const nat amount = 9;

	print_instruction_window_around(pc, 0, "PC");
	print_dictionary(0);

	printf("        "); 
	for (nat j = 0; j < var_count; j++) { 
		if (is_constant[j] or (var_count - j >= amount)) continue; 
		printf("%10s(%04llu) ", variables[j], j); 
	}
	puts("");
	printf("-----------"); 
	for (nat j = 0; j < var_count; j++) { 
		if (is_constant[j] or (var_count - j >= amount)) continue; 
		printf("-----------------"); 
	} 
	puts("");
	
	for (nat i = 0; i < ins_count; i++) {
		printf("ct %3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_constant[j] or (var_count - j >= amount)) continue; 
			if (not type[i * var_count + j]) printf("\033[90m");
			printf("%10s %4llu  ", "", value[i * var_count + j]);
			if (not type[i * var_count + j]) printf("\033[0m");
		}
		putchar(9);

		printf("cp %3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_constant[j] or (var_count - j >= amount)) continue; 
			if (not is_copy[i * var_count + j]) printf("\033[90m");
			printf("%3lld ", copy_of[i * var_count + j]);
			if (not is_copy[i * var_count + j]) printf("\033[0m");
		}


		print_instruction(ins[i]);
		
		if (i == pc) { 
			putchar(32); 
			print_nats(preds, pred_count); 
			putchar(32); 
			printf("\033[32;1m     <------- PC\033[0m"); } 
		putchar(10);
		printf("\033[38;5;235m");
		for (nat _ = 0; _ < 350; _++) printf("-");
		printf("\033[0m");
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
	printf("    "); 
	for (nat j = 0; j < var_count; j++) { 
		if (is_constant[j] or is_label[j]) continue; 
		printf("%20s(%04llu) ", variables[j], j); 
	} 
	puts("");
	printf("----"); 
	for (nat j = 0; j < var_count; j++) { 
		if (is_constant[j] or is_label[j]) continue; 
		printf("-----------------"); 
	} 
	puts("");

	for (nat i = 0; i < ins_count; i++) {
		printf("%2llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_constant[j] or is_label[j]) continue; 
			if (not alive[i * var_count + j]) printf("\033[38;5;235m");
			printf("%20s %4llu  ", "", alive[i * var_count + j]);
			if (not alive[i * var_count + j]) printf("\033[0m");
		}
		print_instruction(ins[i]);
		
		if (i == pc) { 
			putchar(32); 
			print_nats(gotos, goto_count); 
			print_nats(preds, pred_count); 
			putchar(32); 
			printf("\033[32;1m     <------- PC\033[0m"); } 
		putchar(10);
		printf("\033[38;5;235m");
		for (nat _ = 0; _ < 350; _++) printf("-");
		printf("\033[0m");
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

static nat load_nat_from_memory(uint8_t* memory, const nat address) {
	nat x = 0; 
	for (nat i = 0; i < 8; i++)  x |= (nat) ((nat) memory[8 * address + i] << (8LLU * i));
	return x;
}
static void store_nat_to_memory(uint8_t* memory, const nat address, const nat data) {
	for (nat i = 0; i < 8; i++) 
		memory[8 * address + i] = (data >> (8 * i)) & 0xFF;
}

int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("compiler: error: usage: ./run [file.s]"));
	
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
	byte is_compiletime = files[file_count - 1].ct;
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
			struct instruction new = { .op = str, .imm = 0xff, .state = is_compiletime };
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
		if (	(op == lt or op == ge or op == ne or op == eq) and arg_count == 2 or
			(op == set or op == ld or op == reg) and arg_count == 0 or
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
		if (op == set and arg_count == 1) {
			is_label[var_count] = 1;
			goto define_name;

		} else print_error(
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
		else if (op == ct) is_compiletime = 1;
		else if (op == rt) is_compiletime = 0;
		else if (op == del) {
			if (is_immediate) 
				print_error(
					"expected defined variable, found binary literal",
					filename, text, text_length, word_start, pc
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
			if ((op >= set and op <= ld) or op == reg or op == bits) {
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
			if (op == bits) is_ct = 1;
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


	nat at_count[max_variable_count] = {0};
	for (nat pc = 0; pc < ins_count; pc++) {
		const nat op = ins[pc].op;
		if (op == at) { values[ins[pc].args[0]] = pc; at_count[ins[pc].args[0]]++; }
	}
	for (nat pc = 0; pc < ins_count; pc++) {
	for (nat a = 0; a < arity[ins[pc].op]; a++) {
		const nat var = ins[pc].args[a];
		if (is_label[var] and at_count[var] != 1) {
			printf("error: label attribution error! expected exactly 1 label attribution for label %s", variables[var]); 
			print_instruction_window_around(pc, 1, "this label argument does not have exactly one at");
			abort();
		}
	}}

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1];
		const nat is_ct = ins[i].state;
		if (not is_ct and op == set and not is_constant[a0] and is_constant[a1] and is_label[a1]) {
			puts("error: cannot load a compiletime label into a runtime variable.");
			print_instruction_window_around(i, 1, "RT destination, CT source label");
			abort();
		}
	}


	if (debug) {
		print_dictionary(1);
		print_instructions(0);
		puts("parsing finished.");
		getchar();
	}

	{ struct instruction rt_ins[4096] = {0};
	nat rt_ins_count = 0;

	memset(bit_count, 255, sizeof bit_count);
	memset(register_index, 255, sizeof register_index);
	uint8_t* memory = calloc(65536, sizeof(nat));

	for (nat pc = 0; pc < ins_count; pc++) {

		nat op = ins[pc].op;
		nat imm = ins[pc].imm;
		nat is_compiletime = ins[pc].state;

		if (memory[compiler_should_debug]) {
			print_instruction_window_around(pc, 0, "");
			print_dictionary(0);
			dump_hex(memory, 128);

			printf("strings: (%llu count) : \n", string_list_count);
			for (nat i = 0; i < string_list_count; i++) {
				printf("#%llu string: .string = %p .length = %llu, .label = %llu, \n", 
					i, (void*) string_list[i], (nat) strlen(string_list[i]), string_label[i]
				);
			}

			puts("rt instructions: ");
			for (nat i = 0; i < rt_ins_count; i++) {
				putchar(9); print_instruction(rt_ins[i]); puts("");
			}
			puts("done");
			getchar();
		}

		nat arg0 = ins[pc].args[0];
		nat arg1 = ins[pc].args[1];
		nat arg2 = ins[pc].args[2];
		nat i0 = !!(imm & 1);
		nat i1 = !!(imm & 2);
		nat i2 = !!(imm & 4);

		

		const nat N = max_variable_count;
		nat val0 = 0, val1 = 0, val2 = 0;
		if (i0 or arg0 < N) val0 = not i0 ? values[arg0] : arg0;
		if (i1 or arg1 < N) val1 = not i1 ? values[arg1] : arg1;
		if (i2 or arg2 < N) val2 = not i2 ? values[arg2] : arg2;

		if (op == str and not is_compiletime) {
			for (nat s = 0; s < arg0; s++) {
				struct instruction new = { .op = emit, .imm = 3 };
				new.args[0] = 1;
				new.args[1] = (nat) string_list[arg1][s];
				rt_ins[rt_ins_count++] = new;
			}
		} 
		else if (op == bits) bit_count[arg0] = val1;
		else if (op == reg) register_index[arg0] = val1;

		else if (not is_compiletime) {
			struct instruction new = { .op = op, .imm = imm };
			memcpy(new.args, ins[pc].args, sizeof new.args);
			for (nat i = 0; i < arity[op]; i++) {
				const nat not_literal = not ((new.imm >> i) & 1);
				if (not_literal and is_label[new.args[i]]) continue;
				if (not_literal and is_constant[new.args[i]]) {
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

		else if (op == halt) break;
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

		} else if (op == do_) {
			store_nat_to_memory(memory, compiler_return_address, pc);
			pc = values[arg0];
		}
		else if (op == lt) { if (val0  < val1) pc = values[arg2]; }
		else if (op == ge) { if (val0 >= val1) pc = values[arg2]; }
		else if (op == eq) { if (val0 == val1) pc = values[arg2]; }
		else if (op == ne) { if (val0 != val1) pc = values[arg2]; }
		else if (op == sc) {
			nat x0 = load_nat_from_memory(memory, compiler_arg0);
			nat x1 = load_nat_from_memory(memory, compiler_arg1);
			nat x2 = load_nat_from_memory(memory, compiler_arg2);
			nat x3 = load_nat_from_memory(memory, compiler_arg3);

			if (x0 == compiler_system_debug) printf("debug: %llu (0x%llx)\n", x1, x1);
			else if (x0 == compiler_system_exit) exit((int) x1);
			else if (x0 == compiler_system_read) { 
				x1 = (nat) read((int) x1, (void*) x2, (size_t) x3); 
			} else if (x0 == compiler_system_write) { 
				x1 = (nat) write((int) x1, (void*) x2, (size_t) x3); 
			} else if (x0 == compiler_system_open) { 
				x1 = (nat) open((const char*) x1, (int) x2, (mode_t) x3); 
			} else if (x0 == compiler_system_close) { 
				x1 = (nat) close((int) x1); 
			} else { 
				printf("compiler: error: unknown system call number %llu\n", x0); 
				abort(); 
			} 
			store_nat_to_memory(memory, compiler_arg1, x1);
			store_nat_to_memory(memory, compiler_arg2, (nat) errno);
		} else { 
			printf("CTE: fatal internal error: "
				"unknown instruction executed: %s...\n", 
				operations[op]
			); 
			abort(); 
		}
	}
	memcpy(ins, rt_ins, ins_count * sizeof(struct instruction));
	ins_count = rt_ins_count; 
	target_arch = load_nat_from_memory(memory, compiler_target);
	output_format = load_nat_from_memory(memory, compiler_format);
	should_overwrite = load_nat_from_memory(memory, compiler_should_overwrite);
	stack_size = load_nat_from_memory(memory, compiler_stack_size);
	}


	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if ((op >= lt and op <= eq and values[ins[i].args[2]] == (nat) -1) or
		    
		    (op == do_ and values[ins[i].args[0]] == (nat) -1)) {
			puts("error: label does not have a defined position");
			print_instruction_window_around(i, 1, "this label is undefined");
			abort();
		}
	}

	if (target_arch == msp430_arch and stack_size) { 
		puts("fatal error: nonzero stack size for msp430 is not permitted"); 
		abort();

	} else if (target_arch == arm64_arch and stack_size < min_stack_size) {
		puts("warning: stack size less than the minimum size for arm64");
	}

	if (debug) {
		print_dictionary(1);
		print_instructions(0);
		puts("CT-PRUNED-EXECUTION finished.");
		getchar();
	}

	const char* output_filename = "output_file_from_compiler";
	if (output_format == uf2_executable) output_filename = "output_file_from_compiler.uf2";
	if (output_format == c_source) output_filename = "output_file_from_compiler.c";

	{ nat* type = calloc(ins_count * var_count, sizeof(nat));
	nat* value = calloc(ins_count * var_count, sizeof(nat));
	nat* is_copy = calloc(ins_count * var_count, sizeof(nat));
	nat* copy_of = calloc(ins_count * var_count, sizeof(nat));

	const nat traversal_count = 2; 
	nat stack[4096] = {0};
	nat stack_count = traversal_count;

	for (nat i = 0; i < ins_count; i++)  ins[i].state = 0;

	while (stack_count) {
		nat pc = stack[--stack_count];
		nat pred_count = 0;
		nat* preds = compute_predecessors(pc, &pred_count);
		nat* gotos = compute_successors(pc);
		ins[pc].state++;
		
		if (debug) {
			debug_data_flow_state(
				pc, preds, pred_count,
				stack, stack_count, value,
				type, is_copy, copy_of
			);
			getchar();
		}

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

			const nat dd = 0;//debug and pc == 4 and var == 169;      // temporary

		
			for (nat iterator_p = 0; iterator_p < pred_count; iterator_p++) {
				const nat pred = preds[iterator_p];

				if (not ins[pred].state) { 
					if (dd) { 
						printf("skipping predecessor, as its was never executed (pred = %llu)\n", pred); 
						getchar();
					}
					continue; 
				} 

				const nat ct_t = type[pred * var_count + var];
				const nat ct_v = value[pred * var_count + var];
				const nat cp_t = is_copy[pred * var_count + var];
				const nat cp_v = copy_of[pred * var_count + var];
	
				if (dd) { printf("info: [pred=%llu]: ct_t = %llu, ct_v = %llu\n", pred, ct_t, ct_v); getchar(); } 

				if (ct_t == 0) { if (dd) { puts("future set to 0! (rt!)"); getchar(); }  future_type = 0; first_ct = 0; } 
				else if (first_ct) { 
					if (dd) { puts("first! future set to 1! (ct!)"); getchar(); } 
					future_type = 1;
					future_value = ct_v; 
					first_ct = 0;

				} else if (future_value != ct_v) { 
					if (dd) { puts("value mismatch! (rt)"); getchar(); } 
					future_type = 0;
				}


				if (cp_t == 0) { future_is_copy = 0; first_cp = 0; } 
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
		else if (op == adr) { }
		else if (op == sc) { }
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
			else if (op == div_ and v1) out_v /= v1;
			else if (op == rem and v1)  out_v %= v1;
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

		} else if (op >= lt and op <= eq) {
			if (not ct0 or not ct1) {
				if (gt0 < ins_count and ins[gt0].state < traversal_count) stack[stack_count++] = gt0;
				if (gt1 < ins_count and ins[gt1].state < traversal_count) stack[stack_count++] = gt1; 
				continue;
			} else {
				bool cond = 0;
				     if (op == lt) cond = v0  < v1;
				else if (op == ge) cond = v0 >= v1;
				else if (op == eq) cond = v0 == v1;
				else if (op == ne) cond = v0 != v1;

				const nat target = cond ? gt1 : gt0;
				if (target < ins_count and ins[target].state < traversal_count) 
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
		if (gt0 < ins_count) stack[stack_count++] = gt0; 


		if (op >= set and op <= ld) {
			for (nat i = 0; i < var_count; i++) {
				if (is_copy[pc * var_count + i] and 
				    copy_of[pc * var_count + i] == a0) 
					is_copy[pc * var_count + i] = 0;
			}
		}
	}

	if (debug) {
		debug_data_flow_state(0, NULL, 0, stack, stack_count, value, type, is_copy, copy_of);
		puts("data flow: [FINAL VALUES]");
		print_instructions(0);
		puts("OPT2 finished.");
		getchar();
	}
	
	puts("pruning ctk instructions...");

	for (nat i = 0; i < ins_count; i++) {

		//if (not ins[i].state) ins[i].op = 0;

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("-----------PRUNING CTK INS:---------------");
			getchar();
		}

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		
		nat keep = 0;

		if (op == emit) { keep = 1; ins[i].state = 1; } 
		if (op == at) { keep = 1; ins[i].state = 1; } 

		if (ins[i].state and  
			(
			op == halt 	or op == sc or 
			op == set 	or op == do_ or
			op == at	or op == adr  or
			op == ld 	or op == st or
			op == emit	or op >= a6_nop
			)
		) keep = 1;

		if (not keep and ins[i].state)
		for (nat a = 0; a < arity[op]; a++) {

			if (is_label[ins[i].args[a]]) continue;
						
			if (((imm >> a) & 1)) {
				printf("found a compiletime immediate : %llu\n", 
					ins[i].args[a]
				);

			} else if (register_index[ins[i].args[a]] != (nat) -1) {

				printf("warning: found a register index variable "
					"as argument  :  %s\n",
					variables[ins[i].args[a]]
				); keep = 1; break;


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

		if ((not keep or not ins[i].state) and op != emit) {
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
				if (debug) { 
					puts("NOTE: found a compiletime-known instruction! "
						"deleting this instruction."
					);
					getchar(); 
				}
				ins[i].state = 0; 
			} 
			continue;
		}

		if (debug) {
			puts("found real RT isntruction!"); 
			putchar('\t');
			print_instruction(ins[i]); 
			puts("");
			getchar();
		}

		if (op >= set and op <= sd) {
			const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
			const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

			if (ct1 and not (imm & 2)) {
				puts("inlining compiletime argument...\n"); //getchar(); //abort();
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


				if (is_label[copy_of[i * var_count + ins[i].args[1]]]) {

					if (ins[i].op == set) {
						printf("WARNING: inlining a label into a set!!\n");
						getchar();
						ins[i].args[1] = copy_of[i * var_count + ins[i].args[1]];
					}

				} else {
				

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


		} else if (op == ld) {

			const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
			const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

			const nat ct2 = (imm & 4) or type[i * var_count + ins[i].args[2]];
			const nat v2 = (imm & 4) ? ins[i].args[2] : value[i * var_count + ins[i].args[2]];

			if (ct1 and not (imm & 2)) {
				puts("inlining compiletime ld argument...\n"); 
				ins[i].args[1] = v1;
				ins[i].imm |= 2;
			}

			if (ct2 and not (imm & 4)) {
				puts("inlining compiletime st argument 2 ...\n"); 
				ins[i].args[2] = v2;
				ins[i].imm |= 4;
			}

		} else if (op == st) {

			const nat ct0 = (imm & 1) or type[i * var_count + ins[i].args[0]];
			const nat v0 = (imm & 1) ? ins[i].args[0] : value[i * var_count + ins[i].args[0]];

			const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
			const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

			const nat ct2 = (imm & 4) or type[i * var_count + ins[i].args[2]];
			const nat v2 = (imm & 4) ? ins[i].args[2] : value[i * var_count + ins[i].args[2]];

			if (ct0 and not (imm & 1)) {
				puts("inlining compiletime st argument 0 ...\n"); 
				ins[i].args[0] = v0;
				ins[i].imm |= 1;
			}

			if (ct1 and not (imm & 2)) {
				puts("inlining compiletime st argument 1 ...\n"); 
				ins[i].args[1] = v1;
				ins[i].imm |= 2;
			}

			if (ct2 and not (imm & 4)) {
				puts("inlining compiletime st argument 2 ...\n"); 
				ins[i].args[2] = v2;
				ins[i].imm |= 4;
			}



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

	if (debug) {
		print_instructions(0);
		puts("CTK PRUNING finished.");
		getchar();
	}

	if (not (target_arch == rv32_arch or target_arch == rv64_arch)) goto skip_branch_imm_replace;

	puts("replacing branch and store immediates with branch and store register instructions, for rv32...");

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		const nat i0 = !!(imm & 1);
		const nat i1 = !!(imm & 2);
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1];

		if (not (op == lt or op == ge or op == ne or op == eq or op == st)) continue;
		if (op == lt or op == ge or op == ne or op == eq) {
		if ((i0 and a0) or (i1 and a1)) {
			if (i0) {
				nat t = a0;
				ins[i].args[0] = ins[i].args[1];
				ins[i].args[1] = t;
			}

			const nat n = ins[i].args[1];
			variables[var_count] = strdup("NEW");
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
		} else if (op == st) {

			if (i0) {
				const nat n = ins[i].args[0];
				variables[var_count] = strdup("NEW");
				var_count++;
				memmove(ins + i + 1, ins + i, sizeof(struct instruction) * (ins_count - i));
				ins[i] = (struct instruction) { set, 0x2, 0, { var_count - 1, n } };
				ins_count++;
				ins[i + 1].args[0] = var_count - 1;
				ins[i + 1].imm &= (nat) ~1;
				puts("rv32 replace st address imm: info: inserted a set statement!");
				//getchar();
				i++;
			}
			
			if (i1) {
				const nat n = ins[i].args[1];
				variables[var_count] = strdup("NEW");
				var_count++;
				memmove(ins + i + 1, ins + i, sizeof(struct instruction) * (ins_count - i));
				ins[i] = (struct instruction) { set, 0x2, 0, { var_count - 1, n } };
				ins_count++;
				ins[i + 1].args[1] = var_count - 1;
				ins[i + 1].imm &= (nat) ~2;
				puts("rv32 replace st data imm: info: inserted a set statement!");
				//getchar();
				i++;
			}
		}
	}

	if (debug) {
		print_instructions(0);
		puts("non imm branches for riscv done.");
		getchar();
	}

skip_branch_imm_replace:;
	

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
	if (target_arch == c_arch) 	goto c_instruction_selection;
	puts("instruction selection: error: unknown target"); abort();


rv32_instruction_selection:;
	puts("rv32: instruction selection starting...");
	{ struct instruction new = {0};
	const nat unrecognized = (nat) -1;

	for (nat i = 0; i < ins_count; i++) {

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("rv32 machine instructions:");
			for (nat e = 0; e < mi_count; e++) {
				printf("%llu: ", e); 
				print_instruction(mi[e]); 
				puts("");
			}
			puts("[mi done]");
			puts("[RISC-V ins sel]");
			getchar();
		}

		if (ins[i].state) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i);
			//getchar(); 
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
			op == at or op == emit or op == adr or op == halt
		) { 
			new = ins[i]; 
			goto r5_push_single_mi; 
		}








		//   set d m  OP_A d n   -->   OP_B d n m
		//   set d m  OP_A d k   -->   OP_B d n k

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
				if (j == unrecognized) goto skip_set_r_r;
				new = (struct instruction) { 
					r5_r, 0x25, 0,   
					{ 0x33, arg0, op_B1[this], arg1, ins[j].args[1], op_B2[this],    0,0 } 
				};
				ins[j].state = 1; 
				goto r5_push_single_mi;
				skip_set_r_r:;
			} 

			if (op == set and not imm) {
				const nat j = locate_instruction(
					(struct expected_instruction) {
						.op = op_A[this], .imm = 2,
						.use = 1,
						.args[0] = arg0
					}, i + 1
				);

				if (j == unrecognized) goto skip_set_r_i;
				if (ins[j].args[1] >= 2048) goto skip_set_r_i;
				if (op == mul or op == div_ or op == rem) goto skip_set_r_i;

				new = (struct instruction) { 
					r5_i, 0x15, 0, {
						0x13, 
						arg0, 
						op_B1[this], 
						arg1, 
						ins[j].args[1], 
						0, 0
					} 
				};
				ins[j].state = 1; 
				goto r5_push_single_mi;
				skip_set_r_i:;
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

			if (op == op_A[this] and imm) {

				if (arg1 >= 2048) goto skip_op_r_i;
				if (op == mul or op == div_ or op == rem) goto skip_op_r_i;

				new = (struct instruction) { 
					r5_i, 0x15, 0, {
						0x13, 
						arg0, 
						op_B1[this], 
						arg0, 
						arg1, 
						0, 0
					} 
				};
				goto r5_push_single_mi;
				skip_op_r_i:;
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

		if (op == sc) {
			new = (struct instruction) { r5_i, 0xff, 0,  { 0x73,0,0,0, 0,0,0,0 } };
			goto r5_push_single_mi;
		}

		else if (op == set and not imm and is_label[arg1]) {   

			// la d l ->   auipc d l[31:12] ;  addi d d l[11:0]

			new = (struct instruction) { r5_u, 0x5, 0,  { 0x17, arg0, arg1, 0x42,  0,0,0,0 } };
			mi[mi_count++] = new;
			new = (struct instruction) { r5_i, 0x15, 0, { 0x13, arg0, 0, arg0, arg1, 0x42, 0,0 } };
			goto r5_push_single_mi;
		} 

		else if (op == st) {
			if (imm & 3) { puts("store immediates are not supported yet lol"); abort(); } 
			const nat size = arg2; 

			if (size == 1) {
				new = (struct instruction) { r5_s, 0xf3, 0,   { 0x23, 0x000, arg0, arg1, 0x00000, 0,0,0 } };

			} else if (size == 2) {
				new = (struct instruction) { r5_s, 0xf3, 0,   { 0x23, 0x001, arg0, arg1, 0x00000, 0,0,0 } };

			} else if (size == 4) {
				new = (struct instruction) { r5_s, 0xf3, 0,   { 0x23, 0x010, arg0, arg1, 0x00000, 0,0,0 } };

			} else {
				puts("unknown size for store instruction on risc-v"); abort();
			}

			goto r5_push_single_mi;
		}




		else if (op == set and not imm) { // set d n -> addi d n 0 
			new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg1, 0, 0,0,0 } };
			goto r5_push_single_mi;
		}

		else if (op == set and imm and arg1 < 2048) { // set d #k -> addi d zr k
			new = (struct instruction) { r5_i, 0x1D, 0,   { 0x13, arg0, 0, 0, arg1, 0,0,0 } };
			goto r5_push_single_mi;
		}
	
		else if (op == set and imm and arg1 >= 2048 and arg1 < (1LLU << 32LLU)) {

 			const bool bit11_is_set = !!(arg1 & 0x800);
			const nat U20 = ((((uint32_t) arg1) >> 12) + bit11_is_set) & 0xfffff;
			const nat U12 = arg1 & 0xfff;

			new = (struct instruction) { r5_u, 0x5, 0,  { 0x37, arg0, U20,  0,0,0,0,0 } };
			if (not U12) goto r5_push_single_mi;
			mi[mi_count++] = new;
			new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg0, U12, 0,0,0 } };
			goto r5_push_single_mi;
		}

		else if (op == add and imm and arg1 < 2048) { // add d #k -> addi d d k
			new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg0, arg1,0,  0,0 } };
			goto r5_push_single_mi;
		}

		else if (op == sub and imm) { // sub d #k -> addi d d -k
			nat k = (-arg1) & 0xFFF;
			new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg0, k, 0,  0,0 } };
			goto r5_push_single_mi;
		}

		else if (op == do_) {			
			new = (struct instruction) { r5_j, 0x7, 0,  { 0x6f, 0, arg0, 0,0,0,0,0 } };
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
	//const nat unrecognized = (nat) -1;

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

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("msp430 machine instructions:");
			for (nat e = 0; e < mi_count; e++) {
				printf("%llu: ", e); 
				print_instruction(mi[e]); 
				puts("");
			}
			puts("[mi done]");
			puts("[MSP430 ins sel]");
			getchar();
		}

		if (ins[i].state) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i);
			//getchar(); 
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
			op == m4_op or op == adr or op == m4_br or
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



		if (op == set and not imm) { // mov d n
			new = (struct instruction) {
				m4_op, 0xFFFFF, 0, { msp_mov,
					reg_mode, arg0, 0,
					reg_mode, arg1, 0,
					0
				}
			};
			goto msp430_push_single_mi;
		}
		/*else if (op == set and imm) { // addi d zr k
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
		}*/
			
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

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("arm64 machine instructions:");
			for (nat e = 0; e < mi_count; e++) {
				printf("%llu: ", e); 
				print_instruction(mi[e]); 
				puts("");
			}
			puts("[mi done]");
			puts("[arm64 ins sel]");
			getchar();
		}

		if (ins[i].state) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i); 
			//getchar();
			continue;
		}

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1]; 


		if (op == halt or op == adr or op == at or op == emit or 
			(op >= a6_nop and op <= a6_divr)
		) { 
			mi[mi_count++] = ins[i]; 
			ins[i].state = 1; 
			continue;
		}


		if (op == set and is_label[arg1]) {

			puts("error unknown instruction selection pattern!");
			abort();
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

		if (op == sc) {
			struct instruction new = { .op = a6_svc, .imm = 0xff };
			mi[mi_count++] = new;
			ins[i].state = 1;
			continue;
		}
	}
	goto finish_instruction_selection;


c_instruction_selection:;
	puts("c: instruction selection starting...");
	{ struct instruction new = {0};
	for (nat i = 0; i < ins_count; i++) {

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("C machine instructions:");
			for (nat e = 0; e < mi_count; e++) {
				printf("%llu: ", e); 
				print_instruction(mi[e]); puts("");
			}
			puts("[mi done]\n[C ins sel]"); 
			getchar();
		}
		if (ins[i].state) {
			printf("warning: [i = %llu]: skipping, part of a pattern.\n", i);
			//getchar(); 
			continue;
		}
		const nat op = ins[i].op;
		if (op < a6_nop) { new = ins[i]; goto c_push_single_mi; } 
		puts("error: unknown instruction selection pattern");
		abort();
	c_push_single_mi:
		mi[mi_count++] = new;
		ins[i].state = 1;
	}}
	goto finish_instruction_selection;



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

	if (debug) {
		puts("finished instruction selection!");
		printf("info: preliminary machine code prior to RA: for target = %llu\n", target_arch);
		print_instructions(1);
		getchar();
	}


	puts("RA: starting register allocation!");
	nat hardware_register_count = 0;
	if (target_arch == rv32_arch) hardware_register_count = 31;
	else if (target_arch == msp430_arch) hardware_register_count = 12;
	else if (target_arch == c_arch) hardware_register_count = 4096;
	else {
		puts("cannot perform RA for this target, unimplemented.");
		abort();
	}

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

		} else if (target_arch == c_arch) {
			preds = compute_predecessors(pc, &pred_count);
			gotos = compute_successors(pc);
		}

		if (gotos[0] != (nat) -1) goto_count++;
		if (gotos[1] != (nat) -1) goto_count++;

		if (debug) {
			debug_liveness(pc, preds, pred_count, gotos, goto_count, stack, stack_count, alive);
			printf("executing: [pc = %llu]: ", pc); 
			print_instruction(ins[pc]);
			puts("");
			getchar();
		}

		ins[pc].state++;

		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;

		const nat a0 = ins[pc].args[0];
		const nat a1 = ins[pc].args[1];
		const nat a2 = ins[pc].args[2];
		const nat a3 = ins[pc].args[3];
		const nat a4 = ins[pc].args[4];
		const nat a5 = ins[pc].args[5];
		
		const nat i0 = !!(imm & (1 << 0));
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
		else if (op == adr) {}

		else if (target_arch == rv32_arch) { 

			if (	op == r5_i and a0 == 0x73 and 
				a1 == 0x0 and a2 == 0x0 and 
				a3 == 0x0 and a4 == 0x0
			) {
				for (nat e = 0; e < var_count; e++) {
					if (register_index[e] >= 10 and register_index[e] <= 17) 
						alive[pc * var_count + e] = 1;
				}			

			} else if (op == r5_r) { // addr D A A
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

			} else goto unknown_liveness_error;

		} else if (target_arch == c_arch) {
			if (op == set or op == ld) {
				if (not i0) alive[pc * var_count + a0] = 0;
				if (not i1 and not is_label[a1]) alive[pc * var_count + a1] = 1;

			} else if ((op >= add and op <= sd) or op == st or (op >= lt and op <= eq)) {
				if (not i0) alive[pc * var_count + a0] = 1;
				if (not i1) alive[pc * var_count + a1] = 1;

			} else if (op == sc) { // system calls and liveness is a bit icky right now... :((((
				for (nat e = 0; e < var_count; e++) {
					if (register_index[e] < 8) alive[pc * var_count + e] = 1;
				}

			} else if (op == do_) {
				// do nothing

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

	puts("liveness analysis finished.");


	nat range_begin[4096] = {0};
	nat range_end[4096] = {0};
	nat range_var[4096] = {0};
	nat range_count = 0;

	puts("now we have to form the live-range-node listing!");

	for (nat var = 0; var < var_count; var++) {

		if (is_constant[var] or is_label[var]) continue; // optional...

		nat pc = 0;
		while (pc < ins_count and alive[pc * var_count + var]) pc++;
		while (pc < ins_count and not alive[pc * var_count + var]) pc++;

		while (pc < ins_count) {

			if (debug) {
			printf("[begin]: live range var = %s: { ", variables[var]);
			for (nat i = 0; i < ins_count; i++) {
				if (i != pc) printf(" %llu ", alive[i * var_count + var]);
				if (i == pc) printf(" [%llu] ", alive[i * var_count + var]);
			}
			puts("} ");
			getchar(); } 

			nat begin = pc;

			while (pc < ins_count and alive[pc * var_count + var]) pc++;

			if (debug) { printf("[end]: live range var = %s: { ", variables[var]);
			for (nat i = 0; i < ins_count; i++) {
				if (i != pc) printf(" %llu ", alive[i * var_count + var]);
				if (i == pc) printf(" [%llu] ", alive[i * var_count + var]);
			}
			puts("} ");
			getchar(); }

			range_begin[range_count] = begin;
			range_end[range_count] = pc;
			range_var[range_count++] = var;

			while (pc < ins_count and not alive[pc * var_count + var]) pc++;
		} 
		if (debug) { printf("computed all live ranges for variable %s...\n", variables[var]);
		getchar(); } 
	}

	if (debug) {

	puts("RA: done with node generation, computed these nodes!:");
	for (nat i = 0; i < range_count; i++) {
		printf("range[%llu] = {.begin = %llu, .end = %llu, .var = %s(%llu) }\n", 
			i, range_begin[i], range_end[i], variables[range_var[i]], range_var[i]
		);
	}
	puts("all nodes computed.");

	puts("live ranges which need RA performed on them (or already have a register index!)");
	for (nat i = 0; i < range_count; i++) {
		printf("#%llu:  var %s (%llu)   was live from {.begin= %llu, .end= %llu}    ", 
			i, variables[range_var[i]], range_var[i], range_begin[i], range_end[i]
		);
		if (register_index[range_var[i]] != (nat) -1) {
			printf("\t ----> must be stored in hardware register %llu\n", 
				register_index[range_var[i]]
			);
		} else puts("");
	}
	puts("\n");

	}



	nat rig[4096] = {0};
	nat rig_count = 0;

	puts("RA: constructing register interference graph...");


	for (nat pc = 0 ; pc < ins_count; pc++) {

		for (nat i = 0; i < range_count; i++) {
			for (nat j = 0; j < i; j++) {

				if (pc < range_begin[i] or pc >= range_end[i]) continue;
				if (pc < range_begin[j] or pc >= range_end[j]) continue;

				rig[2 * rig_count + 0] = i;
				rig[2 * rig_count + 1] = j;
				rig_count++;

				for (nat r = 0; r < rig_count - 1; r++) {
					const nat first = rig[2 * r + 0];
					const nat second = rig[2 * r + 1];
					if (first == i and second == j) { rig_count--; break; }
					if (first == j and second == i) { rig_count--; break; }
				}
			}
		}
	}

	if (debug) {

	printf("constructed the following RIG: (%llu interferences)\n", rig_count);
	for (nat i = 0; i < rig_count; i++) {
		const nat first = rig[2 * i + 0];
		const nat second = rig[2 * i + 1];
		printf("    . live-range %s{%llu,%llu} interferes with %s{%llu,%llu}.\n", 
			variables[range_var[first]], range_begin[first], range_end[first],
			variables[range_var[second]], range_begin[second], range_end[second]
		);
	}

	puts("info: current state is the above input to RA!");
	getchar();

	}


	nat* node_selected = calloc(range_count, sizeof(nat));
	stack_count = 0;

	while (1) {
		for (nat i = 0; i < range_count; i++) {
			if (not node_selected[i]) goto find_virtual_register; 
		}
		puts("RA: pushed all nodes!");
		break;

	find_virtual_register:
		for (nat var = 0; var < range_count; var++) {
			if (node_selected[var]) continue;

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
		printf("%5llu: %s (range_var[stack[i]]=%llu), (stack[i]=%llu) \n", 
			i, variables[range_var[stack[i]]], range_var[stack[i]], stack[i]
		);
	}
	puts("[ordering done]");

	nat* occupied = calloc(hardware_register_count, sizeof(nat));

	nat* allocation = calloc(range_count, sizeof(nat));
	memset(allocation, 255, sizeof(nat) * range_count);

	for (nat i = 0; i < range_count; i++) {

		if (register_index[range_var[i]] == (nat) -1) continue;

		if (target_arch == rv32_arch) {
			allocation[i] = register_index[range_var[i]] - 1;

		} else if (target_arch == msp430_arch) {
			allocation[i] = register_index[range_var[i]] - 4;

		} else if (target_arch == c_arch) {
			allocation[i] = register_index[range_var[i]];

		} else abort();
	}

	printf("occupied: "); print_nats(occupied, 10); puts("");
	
	for (nat s = stack_count; s--;) {
		const nat top = stack[s];
		node_selected[top] = 0;
		printf("[s=%llu]: trying to allocate live-range:  {%s:%llu,%llu}...\n", 
			s, variables[range_var[top]], range_begin[top], range_end[top]
		);

		if (debug) {
			puts("info: current allocation scheme:");
			for (nat i = 0; i < range_count; i++) {
				if (allocation[i] == (nat) -1) continue;
				printf("    . range[%llu]{%s,%llu,%llu} is stored in hardware register x[%lld]\n", 
					i, variables[range_var[i]], range_begin[i], range_end[i], allocation[i]
				);
			}
			puts("\n[continue?]");
			getchar();
		}


		if (allocation[top] != (nat) -1) continue;

		memset(occupied, 0, sizeof(nat) * hardware_register_count);

		for (nat e = 0; e < rig_count; e++) {
			const nat a = rig[2 * e + 0];
			const nat b = rig[2 * e + 1];
			if (node_selected[a] or node_selected[b]) continue;
			     if (a == top) occupied[allocation[b]] = 1;
			else if (b == top) occupied[allocation[a]] = 1;
		}
		
		if (debug) { printf("current occupied: "); print_nats(occupied, 10); puts(""); } 



		// TODO: BUG:   we need to prioritize picking the same register as a hardware reg  used in a    set
		//	   such that we actaully      elide the       set        instruction    because its a set to itself.        set x[1] x[1]  would get deleted!
		//        but this only happens      IFFF we pick the right register for the variable. hmmmm. 




		for (nat pick = 0; pick < hardware_register_count; pick++) {

			nat interference_count = 0;
			for (nat e = 0; e < range_count; e++) {
				if (allocation[e] == (nat) -1) continue;
				for (nat r = 0; r < rig_count; r++) {
					const nat a = rig[2 * r + 0];
					const nat b = rig[2 * r + 1];
					if (((a == top and b == e) or (a == e and b == top)) and pick == allocation[e]) {
						printf("we cannot pick register [%llu] for variable %s, "
							"because there is a RIG conflict between %s and %s, "
							"and %s must live in regsiter %llu.\n",

							pick, variables[range_var[top]], 
							variables[a], variables[b], 
							variables[e], allocation[e]
						);
						interference_count++;
					}
				}
			}

			if (not occupied[pick] and interference_count == 0) {
				allocation[top] = pick;
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

	for (nat i = 0; i < range_count; i++) {
		if (allocation[i] == (nat) -1) continue;
		if (target_arch == rv32_arch) {
			allocation[i] += 1;
		} else if (target_arch == msp430_arch) {
			allocation[i] += 4;
		} else if (target_arch == c_arch) {
			//allocation[i] += 0;
		} else abort();
	}

	if (debug) {
		puts("RA: FINAL REGISTER ALLOCATION:");
		for (nat i = 0; i < range_count; i++) {
			if (allocation[i] == (nat) -1) continue;
			printf("    . live-range %llu: {%s:%llu,%llu} is stored in hardware register x[%lld]\n", 
				i, variables[range_var[i]], range_begin[i], range_end[i], allocation[i]
			);
		}
		puts("\n[done with graph coloring in RA]");
		getchar();
	}
		
	for (nat i = 0; i < ins_count; i++) ins[i].state = 0;

	puts("filling in RA assignments into the machine code...");

	for (nat pc = 0; pc < ins_count; pc++) {

		if (debug) {
			print_instruction_window_around(pc, 0, "");
			puts("[RA: filling in allocation scheme, [dead store elmination]]");
			getchar();
		}

		const nat op = ins[pc].op;
		const nat imm = ins[pc].imm;
	
		for (nat a = 0; a < arity[op]; a++) {
			if (op == at and a == 0) continue;
			const nat var = ins[pc].args[a];

			if (imm & (1 << a)) {
				printf("on argument: [a = %llu]: is_immediate!  (immediate value is %llu)\n", a, var);
				continue;
			} 

			else if (is_label[var]) {
				printf("on argument: [a = %llu]: is_label! \"%s\"\n", a, variables[var]);
				continue;
			} 

			printf("on argument: [a = %llu]: NOT is_immediate and NOT label!  (variable = %s) \n", 
				a, variables[var]
			);

			puts("filling in the register index we found for this operation!");
			puts("finding associated live range for this variable...");

			nat range = range_count; 

			for (nat i = 0; i < range_count; i++) {
				if (var != range_var[i]) continue;

				if (pc < range_begin[i] - 1 or pc >= range_end[i]) continue;
				range = i; break;
			}


			if (range == range_count) {
				if (debug) {
					puts("warning: this variable does not have an associated live range, "
						"and thus this instruction has been deleted."
					);
					getchar();
				}
				ins[pc].state = 1;
				continue;
			}

			if (allocation[range] == (nat) -1) {
				if (debug) {
					printf("warning: no hardware register index was "
						"not found for variable %s in the below instruction. "
						"deleting this insttruction via dead-store elimination.\n", 
						variables[var]
					);
					printf("DELETED:   --->   "); print_instruction(ins[pc]); puts(""); 
					getchar();
				}
				ins[pc].state = 1;
				continue;
			}

			ins[pc].args[a] = allocation[range];
			if (target_arch != c_arch) ins[pc].imm |= 1LLU << a;

			printf("info: filled in register index %lld for variable %s into this instruction. ", 
				allocation[range], variables[var]
			);
		}
	}

	{ nat final_ins_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].state) continue;
		ins[final_ins_count++] = ins[i];
	}
	ins_count = final_ins_count; } }


	if (debug) {
		print_instructions(0);
		puts("RA DEAD-STORE PRUNING finished.");
		puts("[done with RA");
		printf("info: finished final machine code for target = %llu\n", target_arch);
		print_instructions(1);
		getchar();
	}

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
	if (target_arch == c_arch) goto c_generate_source_code;
	puts("unknown target"); abort();

rv32_generate_machine_code:;

	{ nat* lengths = calloc(ins_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (op == halt or op == at or op == adr) continue;
		nat k = 4;
		if (op == emit) k = ins[i].args[0];
		lengths[i] = k;
	}

	print_nats(lengths, ins_count); puts("");

	for (nat i = 0; i < ins_count; i++) {

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("");
			dump_hex(my_bytes, my_count);
			getchar();
		}

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

		} else if (op == adr) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;


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
				(((a4 >> 5) & 0x3f) << 25U) | 
				(a3 << 20U) | 
				(a2 << 15U) | 
				(a1 << 12U) | 
				((a4 & 0x1f) <<  7U) | 
				(a0 << 0U) ;
			insert_u32(&my_bytes, &my_count, word);

		} else if (op == r5_u) {
			u32 im = (a2 << 12) & 0xFFFFF000;
			if (a0 == 0x17) {
				const nat n = compute_label_location(a2);
				im = calculate_offset(lengths, i, n) & 0xFFFFF000;
			}
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

		} else if (op == r5_j) {

			const nat n = compute_label_location(a2);
			const u32 im = (u32) calculate_offset(lengths, i, n) & 0x1FFFFF;

			printf("decimal: im = %d\n", im | ((im & (1 << 21)) ? 0xFFE00000 : 0));
			printf("binary: "); print_binary(im | ((im & (1 << 21)) ? 0xFFE00000 : 0)); puts("");

			const u32 bit10_1  = (im >> 1U) & 0x3FF;
			const u32 bit19_12 = (im >> 12U) & 0xFF;
			const u32 bit11   = (im >> 11U) & 0x1;
			const u32 bit20   = (im >> 20U) & 0x1;
			const u32 offset = (bit20 << 31U) | (bit10_1 << 21U) | (bit11 << 20U) | (bit19_12 << 12U);
	
			printf("j_type:  offset = 0x%08x\n", offset);

			printf("offset = "); print_binary(offset); puts("");
			//getchar();

			const u32 word =
				(offset) |
				(a1 <<  7U) |
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
		if (op == adr) len = 0;
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

		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("");
			dump_hex(my_bytes, my_count);
			getchar();	
		}

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

		} else if (op == adr) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;


		} else if (op == m4_br) { // br4 cond:[3 bits] label:[pc-rel offset]
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


		if (debug) {
			print_instruction_window_around(i, 0, "");
			puts("");
			dump_hex(my_bytes, my_count);
			getchar();
		}

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

		} else if (op == adr) {
			section_addresses[section_count] = a0;
			section_starts[section_count++] = my_count;

		} else if (op == a6_clz) { puts("clz is unimplemented currently, lol"); abort(); }
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
	goto finished_generation;

c_generate_source_code:;

	{ const char* header = 
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
		const nat imm = ins[i].imm;
		const nat a0 = ins[i].args[0];
		const nat a1 = ins[i].args[1];
		const nat a2 = ins[i].args[2];

		if (op == halt or op == adr) {}

		else if (op == sc) {
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


		} else if (op == set and not imm and is_label[a1]) {
			char str[4096] = {0}; nat len = 0;
			len = (nat) snprintf(str, sizeof str, "\tx[%llu] = ((uint64_t)(void*)d) + 0x%llx;\n", a0, label_data_locations[a1]);
			insert_bytes(&my_bytes, &my_count, str, len);



		} else if (op == set) {

			if (a0 == a1 and not imm) continue;

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
	}
	char str[4096] = {0};
	const nat len = (nat) snprintf(str, sizeof str, "%s", footer);
	insert_bytes(&my_bytes, &my_count, str, len); }

	goto finished_generation;

finished_generation:;
	puts("done: byte generation successful.");
	puts("final_bytes:");
	dump_hex(my_bytes, my_count);
	printf("info: generating output file with format #%llu...\n", output_format);

	if (output_format == no_output) goto print_debug_output;
	if (output_format == hex_array) goto generate_hex_array_output;
	if (output_format == c_source) goto generate_c_source_output;
	if (output_format == ti_txt_executable) goto generate_ti_txt_executable;
	if (output_format == uf2_executable) goto generate_uf2_executable;
	if (output_format == macho_executable) goto generate_macho_executable;
	if (output_format == macho_object) goto generate_macho_object;
	if (output_format == elf_executable) abort();
	if (output_format == elf_object) abort();
	puts("unknown target"); abort();

print_debug_output:;
	printf("debug: executable bytes: (%llu bytes)\n", my_count);
	for (nat i = 0; i < my_count; i++) {
		if (i % 32 == 0) puts("");
		if (my_bytes[i]) printf("\033[32;1m");
		printf("%02hhx ", my_bytes[i]);
		if (my_bytes[i]) printf("\033[0m");
	}
	puts("");
	goto finished_outputting;



generate_c_source_output:;

	{
	char* out = malloc(my_count);
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

	{ int file = open(output_filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
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

	{
	printf("section_starts: "); print_nats(section_starts, section_count); puts("");
	printf("section_addresses: "); print_nats(section_addresses, section_count); puts("");
	const nat starting_address = section_addresses[0]; 

	printf("info: starting UF2 file at address: %08llx\n", starting_address);

	while (my_count % 256)
		insert_byte(&my_bytes, &my_count, 0); 		// pad to 256 byte chunks

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


	printf("writing out uf2 executable...\n");

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

	{ char debug_string[4096] = {0};
	snprintf(debug_string, sizeof debug_string, 
		"./useful/riscv_disassembler/run print %s", 
		output_filename
	);
	system(debug_string); }  

	goto finished_outputting;
	}

finished_outputting: 
	exit(0);

} // main 














































































































































// XXX






   // user sets this using the "section 0101010011" instruction. 

						// all other sections are ignored except for the first one, for uf2 files. 





	//we need to make it so that we pick the already allocated vars last!????
	// please do this lol

	//   we need to have the edges in the rig refer to nodes which we computed! 
	// which means we need to find interferences and root those indexes 

//(op == la and values[ins[i].args[1]] == (nat) -1) or






/*

dissasm version:

		u32 imm10_1 = (word >> 21) & 0x3FF;
		u32 imm20 = (word >> 31) & 0x1;
		u32 imm11 = (word >> 10) & 0x1;
		u32 imm19_12 = (word >> 12) & 0xFF;
		u32 imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
		if (imm20 == 1) imm |= 0xFFE00000;

*/








	// the general idea behind live ranges, is to    NOT give an allocation for each variable,  but rather, for each variable, at each instruction.
	//  or, really, just for each live range, which starts at a particular instruction, and has a length, and has a variable which its associated with. 
	// so, we are just going to have a list of range-start's   for each variable which needs allocation performed 
	// we are going to create a seperate list  (seperate from the dictionary)  which we are going to keep track of the variables which we want to perform register allocation on.. 

	// its a super set of var_count, thus we cannot use var_count. 

	// so, instead, we are going to try to store a new realloc'd   dedicated list   which includes the    ins-start index,  and the ins-end index, denoting the extend of the liverange.

	// we are going to alsoooo include the actual variable  that we are going to use along that path. that live range should include both a set/def and also several uses! so yeah. super important.  yay. we need to see these both, in order to push a new variable to the   "to_allocate[]" array lol. so yeah. 




	/*for (nat e = 0; e < var_count; e++) {
		if (alive[e]) {
			printf("error: variable %s was alive at the first statement of the program, which means it was never defined.\n", variables[e]);

			printf("warning: variable %s is used while uninitialized in the program\n", variables[e]);
			puts("instead, of keeping this var, we will delete it from the program...");
			for (nat pc = 0; pc < ins_count; pc++) alive[pc * var_count + e] = 0;
			printf("removed variable %s from the program.\n", variables[e]);
		}
	}*/


	//nat* needs_ra = calloc(var_count, sizeof(nat));

	/*for (nat i = 0; i < ins_count; i++) {
		for (nat j = 0; j < var_count; j++) {
			if (alive[i * var_count + j]) needs_ra[j] = 1;
		}
	}*/





/*else if (op == set and not i1 and is_label[a1]) {
			out_t = 0;
			out_v = 0;
			out_is_copy = 0; // FALSE! this is techniaclly a copy... FIX THISSS 
		}*/











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





/*for (nat i = 0; i < var_count; i++) {     
			// TODO: this is just a weird hack.. does this work at all!?!?
			if (allocation[i] != (nat) -1) occupied[allocation[i]] = 1;
		}*/




//if (not_literal) { while (replace[new.args[i]]) new.args[i] = values[new.args[i]]; }



			//if (op == la) { 
			//if (is_immediate & 2) goto error_label_immediate; else is_label[args[1]] = 1;
			//}

/*if (op == at and a == 0) continue;
			if (op == do_ and a == 0) continue;
			if (op == lt and a == 2) continue;
			if (op == ge and a == 2) continue;
			if (op == ne and a == 2) continue;
			if (op == eq and a == 2) continue;*/



/*if (is_compiletime) {
			if (not i0) while (replace[arg0]) arg0 = values[arg0];
			if (not i1) while (replace[arg1]) arg1 = values[arg1];
			if (not i2) while (replace[arg2]) arg2 = values[arg2];
		}*/



		//if (gt0 < ins_count and ins[gt0].state < traversal_count) stack[stack_count++] = gt0; 

		/*if (op == set and not i1 and is_label[a1]) {
			const nat label = compute_label_location(a1);
			if (label < ins_count and ins[label].state < traversal_count) stack[stack_count++] = label; 
			continue;
		}*/






			//if (not is_ct and op == la and has_ct_arg0 and has_ct_arg1) is_ct = 1;
			//if (is_ct and op == la and not has_ct_arg0 and not has_ct_arg1) is_ct = 0;
			//if (op == la and has_ct_arg0 != has_ct_arg1) 
			//	print_error(
			//		"instruction requires destination and source to be both compiletime or both runtime",
			//		filename, text, text_length, word_start, pc
			//	);
								








/*

n	c_system_number(0063) 
n	c_system_arg0(0064) 
n	c_system_arg1(0065)

	c_system_arg2(0066) 
	c_system_arg3(0067) 
	c_system_arg4(0068) 
	c_system_arg5(0069) 
	c_system_arg6(0070) 
	rv_sc_arg0(0133) 
	rv_sc_arg1(0134) 
	rv_sc_arg2(0135) 
	rv_sc_number(0136)

n	a0(0140)
	a1(0141)  
n	data(0164)      
n	bit(0166)       
n	i(0172)       
n	j(0174)      
n	r(0177)  


63
64
65
140
141
164
166
172
174
177

*/











	//abort(); 			
				// solved:
 				// currently a huge bug in the CTE2 optimization stage:

				// the CTE2 pass is caling particular variables compiletime known when it shouldnt, as it isnt seeing the right control flow merge points with different data values at the right time, 
				//  in the prime number written for the c arch and output format,    the variable j is being deleted, deduced to be 0 CTK. this is wrong, obviously, because it should be seeing the merge of CF and different CTK values of j (0 vs 1) at "inner", and thus keeping j. this isnt happening, and thus is a huge bug lol. 


						// solved: oh also, RA needs an overhaul kinda, we arent handling   RA constraints right i think,   and we also need to split up variable live ranges into seperate disjoint ranges when possible. 

				













			











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




















/*
				//printf("is_real = %llu\n", is_real);
				//printf("new.args[i] = %llu\n", new.args[i]);
				//printf("replace[new.args[i]] = %llu\n", replace[new.args[i]]);


			printf("generating a call to %llu:%llu: macro %s (arity %llu) (label %llu(%s)) with arguments: {  ", 
					op, op - isa_count, macros[op - isa_count], macro_arity[op - isa_count], macro_label[op - isa_count], variables[macro_label[op - isa_count]]
			);
			for (nat a = 0; a < arg_count; a++) {
				printf("%llu ", args[a]);
			}
			printf(" }\n");




ct 

def after

st compiler_ctsc_number #&after nat 
st compiler_ctsc_arg0   #&u nat 
st compiler_ctsc_arg1   #&u nat 
do my_macro 
at after

del after

*/









		/* 	previous rep implmentation: 

			nat here = pc; while (here < ins_count and ins[here].op == rep) here++;
			if (here < ins_count) for (nat a = 0; a < arity[ins[here].op]; a++) 
				if (ins[here].args[a] == arg0) ins[here].args[a] = values[arg0];
		





		//printf("for your information, the immediate bits are : %llu: { %llu  %llu  %llu }\n", imm, i0, i1, i2);
		//printf("for your information, the arg vals : op=%llu: { %llu  %llu  %llu }\n", op, arg0, arg1, arg2);


else { printf("0 uh oh! ...something is going wrong.. :(\n");  abort(); }
} else { printf("1 uh oh! ...something is going wrong.. tried to index %llu into %llu-sized array. the immediate value is currently %llu. we are on instruction %s. \n", arg1, N, imm, operations[op]);  abort(); }
		else { printf("2 uh oh! ...something is going wrong.. :(\n"); abort(); }

*/







