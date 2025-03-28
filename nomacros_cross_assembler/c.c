/*

1202503171.193144:

we need to add  macro support, via 


      func a b 


turns into: 


      set _arg0 a  set _arg1 b  cat _lr do func


and func definitions are:

	...
	df _lr
	df _arg0
	df _arg1
	df _arg2
	df _arg3
	df _arg4
	...
	
	df2 func
	df func cat func
	
		... use _arg0 and _arg1
	
		incr _lr do _lr




*/




// cross-arch macro assembler, 1202502263.200223 dwrr
// removed macros and simplified labels on 1202503086.020218
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
#define max_macro_count (1 << 13)
#define max_arg_count 16

enum all_architectures { no_arch, arm64_arch, arm32_arch, rv64_arch, rv32_arch, msp430_arch, };
enum all_output_formats { debug_output_only, macho_executable, elf_executable, ti_txt_executable };
enum core_language_isa {
	nullins,
	df, udf, lf,

	df0, df1, df2, df3,
	df4, df5, df6, df7,
	df8, df9, df10, df11,
	df12, df13, df14, df15,

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

	printf("here = %llu\ntarget = %llu\n", here, target);

	nat offset = 0;
	if (target > here) for (nat i = here; i < target; i++) offset += length[i];
	else  		   for (nat i = target; i < here; i++) offset -= length[i];
	return offset;
}

int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));

	const nat min_stack_size = 16384 + 1;
	nat stack_size = min_stack_size;
	const char* output_filename = "output_executable_new";

	struct instruction ins[max_instruction_count] = {0};
	nat ins_count = 0;
	struct instruction rt_ins[max_instruction_count] = {0};
	nat rt_ins_count = 0;

	nat macro_count = 0;
	char* macros[max_macro_count] = {0};
	nat macro_arity[max_macro_count] = {0};
	nat macro_label[max_macro_count] = {0};

	char* variables[max_variable_count] = {0};
	nat is_readonly[max_variable_count] = {0};
	nat values[max_variable_count] = {0};
	nat undefined[max_variable_count] = {0};
	nat register_index = 1LLU << 62LLU;

	const char* included_files[4096] = {0};
	nat included_file_count = 0;
	struct file filestack[4096] = {0};
	nat filestack_count = 1;
	char* string_list[4096] = {0};
	nat string_list_count = 0;
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
}

process_file:;		
	const nat starting_index = filestack[filestack_count - 1].index;
	const nat text_length = filestack[filestack_count - 1].text_length;
	char* text = filestack[filestack_count - 1].text;
	const char* filename = filestack[filestack_count - 1].filename;
	nat word_length = 0, word_start = 0, skip = 0, 
	in_string = 0, arg_count = 0;
	nat args[max_arg_count] = {0}; 

	for (nat var = 0, op = 0, pc = starting_index; pc < text_length; pc++) {

		if (in_string) {
			op = 0; in_string = 0;
			while (isspace(text[pc])) pc++; 
			const char delim = text[pc];
			nat string_at = ++pc, string_length = 0;
			while (text[pc] != delim) { pc++; string_length++; }
			string_list[string_list_count++] = strndup(text + string_at, string_length);
			struct instruction new = { .op = stringliteral };
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
		//printf("[pc=%llu][skip=%llu]: w=\"%s\" w_st=%llu\n", pc, skip, word, word_start);
		//print_dictionary(variables, values, undefined, -1);
		//print_instructions(ins, ins_count);

		if (not strcmp(word, ".") and not skip) { 
			skip = 1; goto next_word; 
		} else if (skip) { 
			if (not strcmp(word, ".")) { skip = 0; goto next_word; 
			} else goto next_word; 
		} else if (not op) {
			if (not strcmp(word, "eoi")) break;
			for (nat m = 0; m < macro_count; m++) 
				if (not strcmp(word, macros[m])) { 
					op = m | (1LLU << 62LLU); goto process_op; 
				} 
			for (op = 0; op < isa_count; op++) 
				if (not strcmp(word, operations[op])) goto process_op;
			print_error: printf("%s:%llu:%llu:"
				" error: undefined %s \"%s\"\n",
				filename, word_start, pc, 
				op == isa_count ? "operation" : "variable", word
			); 
			print_index(text, text_length, word_start, pc);
			if (op != isa_count) 
				printf(
					"calling operation: "
					"%s (arity %llu)\n", 
					operations[op], arity[op]
				);
			abort();
		} 
		else if (op == bn and arg_count == 1) {
			nat s = 1, r = 0;
			for (nat i = 0; i < strlen(word); i++) {
				if (word[i] == '0') s <<= 1;
				if (word[i] == '1') { r += s; s <<= 1; } 
				if (word[i] == '_') continue;
			}
			var = r;
			goto push_argument;
		}
		else if (op >= df0 and op <= df15) goto define_name;
		else if (op == lf or op == df) goto define_name;
		for (var = *values + 1; var-- > 1;) {
			if (not undefined[var] and not strcmp(word, variables[var]))
				goto push_argument;
		} goto print_error;
		define_name: var = *values + 1;
		values[var] = register_index++;
		variables[var] = word; ++*values;
		push_argument: args[arg_count++] = var;
		process_op: 

		if (op & (1LLU << 62LLU)) {
			const nat m = (nat) ((uint32_t) op);
			if (arg_count < macro_arity[m]) goto next_word;
			for (nat i = 0; i < arg_count; i++) {
				struct instruction new = { .op = set, .args[0] = i + 6, .args[1] = args[i] };
				ins[ins_count++] = new;
			}
			struct instruction new0 = { .op = cat, .args[0] = 5 }; ins[ins_count++] = new0;
			struct instruction new1 = { .op = do_, .args[0] = macro_label[m] }; ins[ins_count++] = new1;
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
			int file = open(word, O_RDONLY);
			if (file < 0) { puts(word); perror("open"); exit(1); }
			const nat new_text_length = (nat) lseek(file, 0, SEEK_END);
			lseek(file, 0, SEEK_SET);
			char* new_text = calloc(new_text_length + 1, 1);
			read(file, new_text, new_text_length);
			close(file);
			filestack[filestack_count - 1].index = pc;
			filestack[filestack_count].filename = word;
			filestack[filestack_count].text = new_text;
			filestack[filestack_count].text_length = new_text_length;
			filestack[filestack_count++].index = 0;
			--*values; goto process_file;
		} else if (op < isa_count) {
			if (not op) { puts(" error"); abort(); }
			struct instruction new = { .op = op };
			memcpy(new.args, args, sizeof args);
			ins[ins_count++] = new;
		} else { puts("fatal error: internal error, unknown operation parsed"); abort(); }
		arg_count = 0; op = 0;
		next_word: word_length = 0;
	}
	if (skip) { puts("error: unterminated comment"); abort(); }
	filestack_count--;
	if (filestack_count) goto process_file; 

	for (nat pc = 0; pc < ins_count; pc++) 
		if (ins[pc].op == cat) values[ins[pc].args[0]] = pc;

	print_dictionary(variables, values, is_readonly, undefined, (nat) -1);
	print_instructions(ins, ins_count);

	for (nat pass = 0; pass < 2; pass++) {
		if (pass == 1) rt_ins_count = 0;

		for (nat pc = 0; pc < ins_count; pc++) {

			const nat op = ins[pc].op;
			const nat arg0 = ins[pc].args[0];
			const nat arg1 = ins[pc].args[1];
			const nat arg2 = ins[pc].args[2];

			if (values[4] == 1 and pass == 1) {
				print_instruction_window_around(pc, ins, ins_count, variables);
				printf("\n[pass %llu][pc = %llu]: executing (%llu){ %s : %s %s %s }\n\n\n", 
					pass, pc, arity[op],
					operations[op], variables[arg0], 
					variables[arg1], variables[arg2]
				); 
				print_dictionary(variables, values, is_readonly, undefined, 12);
				getchar();
			}
			if (op <= ro and is_readonly[arg0] and pass == 1) {
				printf("error: cannot modify read-only variable \"%s\"\n", variables[arg0]);
				print_instruction_window_around(pc, ins, ins_count, variables);
				printf("error: cannot modify read-only variable \"%s\"\n", variables[arg0]);
				abort();
			} 
			else if (op == ro) { if (pass == 1) is_readonly[arg0] = 1; } 
			else if (op == zero) values[arg0] = 0;
			else if (op == incr) values[arg0]++;
			else if (op == decr) values[arg0]--;
			else if (op == not_) values[arg0] = ~values[arg0]; 
			else if (op == bn)   values[arg0] = arg1;
			else if (op == set)  values[arg0] = values[arg1];
			else if (op == add)  values[arg0] += values[arg1];
			else if (op == sub)  values[arg0] -= values[arg1];
			else if (op == mul)  values[arg0] *= values[arg1];
			else if (op == div_) values[arg0] /= values[arg1];
			else if (op == rem)  values[arg0] %= values[arg1];
			else if (op == and_) values[arg0] &= values[arg1];
			else if (op == or_)  values[arg0] |= values[arg1];
			else if (op == eor)  values[arg0] ^= values[arg1];
			else if (op == si)   values[arg0] <<= values[arg1];
			else if (op == sd)   values[arg0] >>= values[arg1];
			else if (op == ld)   values[arg0] = values[values[arg1]];
			else if (op == st)   values[values[arg0]] = values[arg1];
			else if (op == cat)  values[arg0] = pc; 
			else if (op == at)   { values[arg0] = rt_ins_count; } //reset_values[arg0] = rt_ins_count; }
			else if (op == do_)  pc = values[arg0]; 
			else if (op == lt) { if (values[arg0]  < values[arg1]) pc = values[arg2]; }
			else if (op == ge) { if (values[arg0] >= values[arg1]) pc = values[arg2]; }
			else if (op == eq) { if (values[arg0] == values[arg1]) pc = values[arg2]; }
			else if (op == ne) { if (values[arg0] != values[arg1]) pc = values[arg2]; }
			else if (op == ctdebug) { if (pass == 1) print_binary(values[arg0]); }
			else if (op == ctabort) { if (pass == 1) abort(); }
			else if (op == ctpause) { if (pass == 1) getchar(); }
			else if (op == stringliteral) {
				for (nat i = 0; i < arg0; i++) {
					struct instruction new = { .op = emit };
					new.args[0] = 1; new.args[1] = (nat) string_list[arg1][i];
					rt_ins[rt_ins_count++] = new;
				}
			} else {
				if (op == 0) { puts("internal error"); abort(); }
				struct instruction new = { .op = op };
				memcpy(new.args, ins[pc].args, sizeof new.args);
				for (nat i = 0; i < arity[op]; i++) new.args[i] = values[new.args[i]];
				rt_ins[rt_ins_count++] = new;
			}
		}
	}

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










































































	/*else if (op == bn) {
			nat s = 1, r = 0;
			for (nat i = 0; i < strlen(word); i++) {
				if (word[i] == '0') s <<= 1;
				if (word[i] == '1') { r += s; s <<= 1; } 
				if (word[i] == '_') continue;
			}
			//printf("bn: found binary constant reg:%s: %s ---> %llu\n", variables[args[0]], word, r);
			//abort();
			struct instruction new0 = { .op = bn, .args[0] = args[0], .args[1] = r };
			ins[ins_count++] = new0;
		}*/


































//nat reset_values[max_variable_count] = {0};
	//memcpy(reset_values, values, sizeof reset_values);
//memcpy(values, reset_values, sizeof values);	










			/*for (nat i = 0; i < string_length; i++) {
				struct instruction new = { .op = emit };
				new.args[0] = 1; new.args[1] = (nat) text[string_at + i];
				ins[ins_count++] = new;
			}*/




/*

how the load and store instructions work:

		memi type d n imm12 size
		memr type d n m opt size
		memia type d n imm9 size mode
		memp L type_size d t n imm7		

bwr
		bwr opc d n m sh imm6 n sf

*/





















































































































































/*static void write_string(const char* filename, char* w_string, nat w_length) {
        char name[4096] = {0};
        srand((unsigned)time(0)); rand();
        char datetime[32] = {0};
        struct timeval t = {0};
        gettimeofday(&t, NULL);
        struct tm* tm = localtime(&t.tv_sec);
        strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm);
        snprintf(name, sizeof name, "%s%s_%08x%08x.txt", directory, datetime, rand(), rand());
        int flags = O_WRONLY | O_TRUNC | O_CREAT | (should_set_output_name ? 0 : O_EXCL);
        mode_t permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int file = open(should_set_output_name ? "output_machine_code.txt" : name, flags, permission);
        if (file < 0) { perror("save: open file"); puts(name); getchar(); }
        write(file, w_string, w_length);
        close(file);
	printf("write_string: successfully wrote out %llu bytes to file %s.\n", 
		w_length, should_set_output_name ? "output_machine_code.txt" : name
	);
}
*/


/*		

		LS == 0    ---> store32
		LS == 1    ---> store64

		LS == 2    ---> load_signed_32
		LS == 2    ---> load_signed_64
		LS == 2    ---> load_signed_32
		LS == 2    ---> load_signed_32
		LS == 2    ---> load_signed_32
		LS == 2    ---> load_signed_32
		LS == 2    ---> load_signed_32
		LS == 2    ---> load_signed_32
		LS == 2    ---> load_signed_32


*/



///// memia LS(0) d(1) n(2) imm9(3) dsize(4) nsize(5) mode(6)

/////   memp L(0) type_size(1) d(2) t(3) n(4) imm7(5)


 // memi LS(0) d(1) n(2) imm12(3) dsize(4) nsize(5)			
 //   (store = 0, load = 1, load_signed = 2)


 //   memr LS(0) Rd(1) Rn(2) Rm(3) option(4) dsize(5) nsize(6) 
 //      (store = 0, load = 1, load_signed = 2)
















//	pc, sp, sr, cg, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, 
//	r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31,
//	size_word, size_byte, size_address, 
//	nonzero, zero, nocarry, carry, negative, greaterequal, less, always,
//	direct, indexed, indirect, autoincr,
//	isa_count
























			// nsize == 0  -->  8-bit
			// nsize == 1  -->  16-bit
			// nsize == 2  -->  32-bit
			// nsize == 3  -->  64-bit
			// dsize == 0 --> 32-bit
			// dsize == 1 --> 64-bit

/*			64<--s8 64/32<--u8 32<--s8

			64<--s16 64/32<--u16 32<--s16

			64/32<--u32 64<--s32

			64<--64
*/




















 //   bfm d n dir pos count type width      (keep = 1, unsigned_zero = 2, signed_zero = 0)



/*
		else if (op >= mov and op <= and_) {

			u16 word = (u16) (
				((op - mov + 4) << 12) | 
				(this.source_reg << 8) | 
				(this.dest_mode << 7) | 
				((nat)(type == size_byte) << 6) | 
				(this.source_mode << 4) | 
				(this.dest_reg)
			);

			section->data[section->length++] = (word >> 0) & 0xFF;
			section->data[section->length++] = (word >> 8) & 0xFF;
			printf("generating word = 0x%04hx\n", word);

			if (
				(this.source_mode == 1 and this.source_reg != 2 and this.source_reg != 3) or
				(this.source_mode == 3 and not this.source_reg)
			) {
				word = (u16) this.source_imm;
				section->data[section->length++] = (word >> 0) & 0xFF;
				section->data[section->length++] = (word >> 8) & 0xFF;
				printf("generating source imm word = 0x%04hx\n", word);
			}

			if (this.dest_mode == 1) {
				word = (u16) this.dest_imm;
				section->data[section->length++] = (word >> 0) & 0xFF;
				section->data[section->length++] = (word >> 8) & 0xFF;
				printf("generating dest imm word = 0x%04hx\n", word);
			}

		} 

*/





























/*
	--------------------------------------------------------------------------------------- 

		this is the msp430 assembler code that we will work into this assembler soon.

	--------------------------------------------------------------------------------------- 
/
	an assembler for the msp430 arch. 
	used for programming msp430frxxxx 
	chips that i have.
	written on 1202409043.013935 by dwrr.
/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <iso646.h>
#include <time.h>
#include <sys/time.h>

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t byte;

struct section {
	byte* data;
	nat length;
	nat address;
};

struct instruction {
	nat opcode;
	nat type;

	nat dest_reg;
	nat source_reg;

	nat dest_mode;
	nat source_mode;

	nat dest_imm;
	nat source_imm;
};

enum language_instructions {
	undefined_ins,
	eof, setoutputname, def, 
	section_start, literal_byte, literal_word,
	mov, add, addc, sub, subc, cmp, dadd, bit, bic, bis, xor_, and_, branch, at, 
	pc, sp, sr, cg, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, 
	r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31,
	size_word, size_byte, size_address, 
	nonzero, zero, nocarry, carry, negative, greaterequal, less, always,
	direct, indexed, indirect, autoincr,
	isa_count
};

static const char* spelling[isa_count] = {
	"__[undefined]__",
	"eof", "setoutputname", "def", 
	"section", "literalbyte", "literalword", 
	"mov", "add", "addc", "sub", "subc", "cmp", "dadd", "bit", "bic", "bis", "xor", "and", "branch", "at", 
	"pc", "sp", "sr", "cg", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", 
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
	"word", "byte", "address", 
	"nonzero", "zero", "nocarry", "carry", "negative", "greaterequal", "less", "always",
	"direct", "index", "deref", "incr",
};

static void debug_sections(struct section* sections, const nat section_count) {
	printf("printing %llu sections: \n", section_count);
	for (nat s = 0; s < section_count; s++) {
		printf("section #%llu: .address = 0x%04llx, .length = %llu :: ", s, sections[s].address, sections[s].length);
		for (nat n = 0; n < sections[s].length; n++) {
			if (n % 16 == 0) printf("\n\t");
			printf("[%02hhx] ", sections[s].data[n]);
		}
		puts("[end of section]");
	}
	puts("[done]");
}

static void print_instructions(struct instruction* ins, nat ins_count) {
	puts("printing instructions");
	for (nat i = 0; i < ins_count; i++) {
		printf("%llu: { .opcode = %s : .dest=%llu, .src=%llu     .destm=%llu, .srcm=%llu     .desti=%llu, .srci=%llu  }\n", 
			i, spelling[ins[i].opcode], 
			ins[i].dest_reg, ins[i].source_reg, 
			ins[i].dest_mode, ins[i].source_mode, 
			ins[i].dest_imm, ins[i].source_imm
		);
	}
	puts("done");
}

static void write_string(const char* directory, char* w_string, nat w_length, nat should_set_output_name) {
        char name[4096] = {0};
        srand((unsigned)time(0)); rand();
        char datetime[32] = {0};
        struct timeval t = {0};
        gettimeofday(&t, NULL);
        struct tm* tm = localtime(&t.tv_sec);
        strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm);
        snprintf(name, sizeof name, "%s%s_%08x%08x.txt", directory, datetime, rand(), rand());
        int flags = O_WRONLY | O_TRUNC | O_CREAT | (should_set_output_name ? 0 : O_EXCL);
        mode_t permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int file = open(should_set_output_name ? "output_machine_code.txt" : name, flags, permission);
        if (file < 0) { perror("save: open file"); puts(name); getchar(); }
        write(file, w_string, w_length);
        close(file);
	printf("write_string: successfully wrote out %llu bytes to file %s.\n", 
		w_length, should_set_output_name ? "output_machine_code.txt" : name
	);
}

int main(int argc, const char** argv) {
	if (argc <= 1) return puts("usage: ./assemble <file.s>");
	const char* filename = argv[1];
	int file = open(filename, O_RDONLY);
	if (file < 0) { perror("open"); printf("error: could not open: %s\n", filename); exit(1); }
	const nat text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(text_length + 1, 1);
	read(file, text, text_length);
	close(file);

	puts("printing the text: ");
	printf("text %p @ %llu chars...\n", (void*) text, text_length);

	puts("file contents:");
	puts("------------------------");
	puts(text);
	puts("------------------------");

	struct instruction ins[4096] = {0};
	nat ins_count = 0;
	nat word_length = 0, word_start = 0, arg = 0, ignoring = 0, should_set_output_name = 0;

	for (nat index = 0; index < text_length; index++) {
		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++;
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;
		char* word = strndup(text + word_start, word_length);
		printf("start: %llu, count: %llu  |   word = %s\n", word_start, word_length, word);
		if (not strcmp(word, "comment")) {
			ignoring = not ignoring;
			if (not ignoring) goto next;
		} 
		if (ignoring) goto next;
		nat n = 0;
		for (nat i = 0; i < isa_count; i++) {
			if (not strcmp(word, spelling[i])) { n = i; goto found; }
		}

		if (*word == '0' or *word == '1') {
			nat r = 0; 
			nat s = 1;
			for (nat i = 0; i < word_length; i++) {
				if (word[i] == '0') s <<= 1;
				else if (word[i] == '1') { r += s; s <<= 1; }
				else if (word[i] == '_') {}
				else goto binary_error;
			}

			printf("info: found binary constant literal = %08llx\n", r);
			if (not ins_count) { puts("usage error"); abort(); }
			if (arg < 2) ins[ins_count - 1].dest_imm = r;
			else ins[ins_count - 1].source_imm = r;
			goto next;
		binary_error:
			puts("binary number error");
			puts("error: unknown binary number encountered");
			printf("index = %llu\n", index);
			puts(word);
			abort();
		}
		puts("error: unknown word encountered!!...");
		printf("index = %llu\n", index);
		puts(word);
		abort();
	found:
		if (n == eof) { puts("found eof"); break; }
		else if (n == setoutputname) should_set_output_name = 1;
		else if (n >= section_start and n <= at) {
			ins[ins_count++] = (struct instruction) {.opcode = n};
			arg = 0;

		} else if (n >= pc and n <= r31) {
			const nat r = n - pc;
			if (not ins_count) { puts("usage error"); abort(); }
			if (arg == 0) ins[ins_count - 1].dest_reg = r;
			else if (arg == 1) ins[ins_count - 1].source_reg = r;
			else { puts("usage error"); abort(); }
			arg++;

		} else if (n >= direct and n <= autoincr) {
			const nat r = n - direct;
			if (not ins_count) { puts("usage error"); abort(); }
			if (arg < 2) ins[ins_count - 1].dest_mode = r;
			else ins[ins_count - 1].source_mode = r;

		} else if (n >= size_word and n <= always) {
			if (not ins_count) { puts("usage error"); abort(); }
			ins[ins_count - 1].type = n;

		} else {
			puts("error: unimpl ins");
			puts(word);
			abort();	
		}
	next:	word_length = 0;
	}


	print_instructions(ins, ins_count);
	struct section sections[128] = {0};
	nat section_count = 0;
	nat labels[32] = {0};


	for (nat pass = 0; pass < 2; pass++) {
		if (pass) {
			for (nat s = 0; s < section_count; s++) 
				free(sections[s].data);
			section_count = 0;
		}


	for (nat i = 0; i < ins_count; i++) {
		printf("ins: %llu: \n", i);
		struct instruction this = ins[i];
		const nat op = this.opcode;
		const nat type = this.type;
		struct section* section = sections + section_count - 1;

		if (op == section_start) {
			printf("new section starts at: 0x%08llx\n", this.dest_imm);
			sections[section_count++] = (struct section) { .data = calloc(65536, 1), .address = this.dest_imm, .length = 0 };

		} else if (op == literal_byte) {
			section->data[section->length++] = (this.dest_imm) & 0xFF;

		} else if (op == literal_word) {
			const u16 word = (u16) this.dest_imm;
			section->data[section->length++] = (word >> 0) & 0xFF;
			section->data[section->length++] = (word >> 8) & 0xFF;

		} else if (op >= mov and op <= and_) {

			if (not section_count) { puts("error: no section given for instruction"); abort(); }
			printf("generating double operand instruction : %s\n", spelling[op]);

			if (type == size_address) abort();
			if (this.dest_reg >= 16) abort();
			if (this.source_reg >= 16) abort();
			if (this.dest_mode >= 2) abort();
			if (this.source_mode >= 4) abort();

			u16 word = (u16) (
				((op - mov + 4) << 12) | 
				(this.source_reg << 8) | 
				(this.dest_mode << 7) | 
				((nat)(type == size_byte) << 6) | 
				(this.source_mode << 4) | 
				(this.dest_reg)
			);

			section->data[section->length++] = (word >> 0) & 0xFF;
			section->data[section->length++] = (word >> 8) & 0xFF;
			printf("generating word = 0x%04hx\n", word);

			if (
				(this.source_mode == 1 and this.source_reg != 2 and this.source_reg != 3) or
				(this.source_mode == 3 and not this.source_reg)
			) {
				word = (u16) this.source_imm;
				section->data[section->length++] = (word >> 0) & 0xFF;
				section->data[section->length++] = (word >> 8) & 0xFF;
				printf("generating source imm word = 0x%04hx\n", word);
			}

			if (this.dest_mode == 1) {
				word = (u16) this.dest_imm;
				section->data[section->length++] = (word >> 0) & 0xFF;
				section->data[section->length++] = (word >> 8) & 0xFF;
				printf("generating dest imm word = 0x%04hx\n", word);
			}

		} else if (op == branch) {

			printf("generating branch...\n");
			const nat offset = ((labels[this.dest_reg] - (section->length + 2)) / 2);
			if ((int) offset <= -500 or (int) offset >= 500) { printf("error: offset too large in branch: %lld\n", offset); abort(); }
			const u16 word = (u16) ((0x1 << 13) | ((type - nonzero) << 10) | (offset & 0x3FF));
			section->data[section->length++] = (word >> 0) & 0xFF;
			section->data[section->length++] = (word >> 8) & 0xFF;

		} else if (op == at) {
			puts("executing at directive...");
			labels[this.dest_reg] = section->length;
		} else {
			printf("unknown instruction to generate: op = %s\n", spelling[op]);
			getchar();
		}
	}

	}

	char out[4096] = {0};
	int len = 0;
	debug_sections(sections, section_count);

	for (nat s = 0; s < section_count; s++) {
		len += snprintf(out + len, sizeof out, "@%04llx", sections[s].address);
		for (nat n = 0; n < sections[s].length; n++) {
			if (n % 16 == 0) len += snprintf(out + len, sizeof out, "\n");
			len += snprintf(out + len, sizeof out, "%02hhX ", sections[s].data[n]);
		}
		len += snprintf(out + len, sizeof out, "\n");
	}
	len += snprintf(out + len, sizeof out, "q\n");
	printf("about to write out: \n-------------------\n<<<%.*s>>>\n----------------\n", len, out);
	printf("write out machine code to file? (y/n) ");
	fflush(stdout);
	write_string("./", out, (nat) len, should_set_output_name);
}


*/






















/*	for (nat rt = 0, pc = 0; pc < ins_count; pc++) {
		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		if (op == cat) values[arg0] = pc;
		else if (op == at) values[arg0] = rt;
		else if (op >= halt) rt++;
	}
*/



/*
if (arg_stack_count) {
			printf("printing arg stack...\n");
			for (nat i = 0; i < arg_stack_count; i++) {	
				printf("if_seen[%llu] = ", i); print_nats(if_seen + i * max_arg_count, if_seen_arity[i]);
				printf("replace_with[%llu] = ", i); print_nats(replace_with + i * max_arg_count, if_seen_arity[i]);
				puts("");
			}
			puts("done");
			getchar();
		}







static nat* get_preds(
	nat* out_pred_count, nat pc, 
	del struct instruction* ins, 
	nat ins_count,
	nat* goto0,
	nat* goto1
) {
	nat count = 0;
	nat* preds = calloc(ins_count, sizeof(nat));
	for (nat i = 0; i < ins_count; i++) {
		if (goto0[i] == pc) preds[count++] = i;
		if (goto1[i] == pc) preds[count++] = i;
	}
	*out_pred_count = count;
	return preds;
}





static void print_index(const char* text, nat text_length, nat begin, nat end) {
	printf("\n\t@%llu..%llu: ", begin, end); 
	for (nat i = 0; i < text_length; i++) {
		if (i == begin) printf("\033[32;1m[");
		putchar(text[i]);
		if (i == end) printf("]\033[0m");
	}
	
	printf("\033[0m");
	puts("\n");
}

static void print_operations(
	nat* op_stack, nat op_stack_count, 
	char** operations, nat* arity, 
	nat* parameters, nat* observable, 
	nat operation_count, char** variables,
	nat max_count
) {
	nat start = 0;
	if (operation_count >= max_count) start = operation_count - max_count;
	puts("operation dictionary: ");
	for (nat i = start; i < operation_count; i++) {
		bool found = 0;
		for (nat j = 0; j < op_stack_count; j++) {
			if (op_stack[j] == i) { found = 1; break; }
		}
		printf(" %c [%llu]  \"%s\" : <%llu>(", not found ? 'U' : ' ', i, operations[i], arity[i]);
		for (nat a = 0; a < arity[i]; a++) {
			nat arg = parameters[max_arg_count * i + a];
			printf(" {(%lld):%s%s}", 
				arg, variables[arg], 
				((observable[i] >> a) & 1) ? ":OBS" : ""
			);
		}
		printf(")\n");
	}
	puts(".");
}

static void print_variables(
	nat* var_stack, nat var_stack_count, 
	char** variables, nat* values, 
	nat* locations, nat* rt_locations,
	nat variable_count, nat max_count
) {
	puts("variable dictionary: ");

	nat start = 0;
	if (variable_count >= max_count) start = variable_count - max_count;
	for (nat i = start; i < variable_count; i++) {
		bool found = 0;
		for (nat j = 0; j < var_stack_count; j++) {
			if (var_stack[j] == i) { found = 1; break; }
		}
		printf(" %c [%llu]  \"%s\" : value=0x%016llx, "
			"location=%lld, rt_location=%lld\n", 
			not found ? 'U' : ' ', i, variables[i], 
			values[i], locations[i], rt_locations[i]);
	}
	puts(".");
}





static nat is_label_arg(nat op, nat arg) {
	if (op == emit) return 0;
	if (op < ct_isa_count) { 
		printf("error: is_runtime_arg: op < ct_isa_count: op=%llu\n", op); 
		abort(); 
	}

	if (op >= arm64_isa_count) {
		printf("error: is_runtime_arg: op >= arm64_isa_count: op=%llu\n", op); 
		abort(); 
	}

	if (op == jmp  and arg == 1) return 1;
	if (op == bc   and arg == 1) return 1;
	if (op == cbz  and arg == 1) return 1;
	if (op == adr  and arg == 1) return 1;
	if (op == ldrl and arg == 1) return 1;
	if (op == tbz  and arg == 2) return 1;
	return 0;
}




def0 nop ret
def0 svc ret
def5 mov d k sh op sf obs d ret
def5 bfm d n k op sf obs d ret
def6 adc d n m s sf sb obs d ret
def8 addx d n m opt i3 s sf sb obs d ret
def7 addi d n i12 sh s sf sb obs d ret
def8 addr d n m sh i6 s sf sb obs d ret
def3 adr d a p obs d obs a ret
def5 shv d n m op sf obs d ret
def4 clz d n sf op obs d ret
def4 rev d n sf op obs d ret
def2 jmp l a obs a ret
def2 bc cond a obs a ret
def3 br n l r ret

def4 cbz n a sf iv obs a ret
def4 tbz n b a iv obs a ret
def7 ccmp n m cond nzcv sf iv im ret
def7 csel d n m cond ic iv sf obs d ret

def5 ori d n k sf op obs d ret
def8 orr d n m sh i6 iv sf op obs d ret
def5 extr d n m i7 sf obs d ret
def3 ldrl d a sz obs d ret
def7 memp d t n i7 l e sz ret
def6 memia d n i9 l eo sz ret
def5 memi d n i12 l sz ret
def6 memr d n m opt l sz ret
def8 madd d n m a op u sf sb obs d ret
def5 divr d n m u sf obs d ret
*/





































































































/*
	uint8_t* my_bytes = NULL;
	nat my_count = 0;

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		const nat imm0 = ins[i].immediates[0];
		const nat imm1 = ins[i].immediates[1];
		const uint32_t Im0 = (uint32_t) imm0;
		const uint32_t Im1 = (uint32_t) imm1;
		const uint32_t sf = ins[i].size == size8;
		const uint32_t Rd = (uint32_t) (ins[i].registers[0] - r0);
		const uint32_t Rn = (uint32_t) (ins[i].registers[1] - r0);
		const uint32_t Rm = (uint32_t) (ins[i].registers[2] - r0);
		const uint32_t Ra = (uint32_t) (ins[i].registers[3] - r0);

		if (op == emit) {
			if (ins[i].size == size8) {
				for (nat t = 0; t < ins[i].immediate_count; t++) 
					insert_u64(&my_bytes, &my_count, (uint64_t) ins[i].immediates[t]); 
			} else if (ins[i].size == size4) {
				for (nat t = 0; t < ins[i].immediate_count; t++) 
					insert_u32(&my_bytes, &my_count, (uint32_t) ins[i].immediates[t]); 
			} else if (ins[i].size == size2) { 
				for (nat t = 0; t < ins[i].immediate_count; t++) 
					insert_u16(&my_bytes, &my_count, (uint16_t) ins[i].immediates[t]); 
			} else if (ins[i].size == size1) { 
				for (nat t = 0; t < ins[i].immediate_count; t++) 
					insert_byte(&my_bytes, &my_count, (uint8_t) ins[i].immediates[t]); 
			}

		} else if (op == stringliteral) {
			for (nat t = imm0; t < imm1; t++) {
				//printf("emitting: %c (%d)\n", text[t], text[t]);
				insert_byte(&my_bytes, &my_count, (uint8_t) text[t]);
			}
			//abort();
		}

		else if (op == nop) insert_u32(&my_bytes, &my_count, 0xD503201F);
		else if (op == svc) insert_u32(&my_bytes, &my_count, 0xD4000001);

		else if (op == br) {
			uint32_t l = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == link_) l = 1;
				if (k == return_) l = 2;
			}

			const uint32_t to_emit = 
				(0x6BU << 25U) | 
				(l << 21U) | 
				(0x1FU << 16U) | 
				(Rd << 5U);

			insert_u32(&my_bytes, &my_count, to_emit);
		}		

		else if (op == adc) {

			uint32_t op1 = 0xD0, op2 = 0, o1 = 0, s = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == flags) s = 1;
				if (k == inv) o1 = 1;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(s << 29U) | 
				(op1 << 21U) | 
				(Rm << 16U) | 
				(op2 << 10U) |
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == sh_) {

			uint32_t op1 = 0xD6, op2 = 8, o1 = 0, s = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == up) 		op2 = 8;
				if (k == down) 		op2 = 9;
				if (k == signed_) 	op2 = 10;
				if (k == rotate) 	op2 = 11;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(s << 29U) | 
				(op1 << 21U) | 
				(Rm << 16U) | 
				(op2 << 10U) |
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == if_) {
			const uint32_t cond = parse_condition(ins[i].modifiers);
			const nat target = values[ins[i].label];
			const nat count = calculate_offset(ins, i, target);
			const uint32_t offset = 0x7ffff & (count >> 2);

			const uint32_t to_emit = 
				(0x54U << 24U) | 
				(offset << 5U) | 
				(cond);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == adr) {

			uint32_t o1 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == page) o1 = 1;
			}

			const nat target = values[ins[i].label];
			nat count = calculate_offset(ins, i, target);
			if (o1) count /= 4096;
			const uint32_t offset = 0x1fffff & count;
			const uint32_t lo = offset & 3, hi = offset >> 2;

			const uint32_t to_emit = 
				(o1 << 31U) | 
				(lo << 29U) |
				(0x10U << 24U) | 
				(hi << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == cbz) {

			uint32_t o1 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == not_) o1 = 1;
			}

			const nat target = values[ins[i].label];
			const nat count = calculate_offset(ins, i, target);
			const uint32_t offset = 0x7ffff & (count >> 2);

			const uint32_t to_emit = 
				(sf << 31U) | 
				(0x1AU << 25U) | 
				(o1 << 24U) |
				(offset << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == tbz) {
			if (imm0 >= (1 << 6)) { puts("bad literal"); abort(); } 

			const uint32_t b40 = Im0 & 0x1F;
			const uint32_t b5 = Im0 >> 5;

			uint32_t o1 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == not_) o1 = 1;
			}

			const nat target = values[ins[i].label];
			const nat count = calculate_offset(ins, i, target);
			const uint32_t offset = 0x3fff & (count >> 2);

			const uint32_t to_emit = 
				(b5 << 31U) | 
				(0x1BU << 25U) | 
				(o1 << 24U) |
				(b40 << 19U) |
				(offset << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == ccmp) {

			uint32_t o1 = 0, r = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == inv) o1 = 1;
				if (k == regimm) r = 1;
			}

			const uint32_t cond = parse_condition(ins[i].modifiers);

			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(0x1D2 << 21U) | 
				(Rn << 16U) | 
				(cond << 12U) |
				(r << 11U) | 
				(Rd << 5U) | 
				(Rm & 0xF); 
			insert_u32(&my_bytes, &my_count, to_emit);
		}


		else if (op == addi) {
			if (imm0 >= (1 << 12)) { puts("bad literal"); abort(); } 

			uint32_t o1 = 0, s = 0, sh = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == flags) 	s = 1;
				if (k == inv) 		o1 = 1;
				if (k == shift12) 	sh = 1;
			}
			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(s << 29U) | 
				(0x22 << 23U) | 
				(sh << 22U) |
				(Im0 << 10U) |
				(Rn << 5U) | 
				(Rd);
			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == add) {
			if (imm0 >= (sf ? 64 : 32)) { puts("bad literal"); abort(); } 

			uint32_t o1 = 0, s = 0, sh = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == flags) 	s = 1;
				if (k == inv) 		o1 = 1;
				if (k == up)		sh = 0;
				if (k == down) 		sh = 1;
				if (k == signed_) 	sh = 2;
			}
			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(s << 29U) | 
				(0xB << 24U) | 
				(sh << 22U) | 
				(Rm << 16U) |
				(Im0 << 10U) | 
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == do_) {
			uint32_t l = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == link_) l = 1;
			}

			const nat target = values[ins[i].label];
			const nat count = calculate_offset(ins, i, target);
			const uint32_t offset = 0x3ffffff & (count >> 2);

			const uint32_t to_emit = 
				(l << 31U) | 
				(0x5U << 26U) | 
				(offset);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == mov) {
			if (imm0 >= (1 << 16)) { puts("bad literal"); abort(); } 

			uint32_t opc = 2, shift = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == keep) 		opc = 3;
				if (k == inv) 		opc = 0;
				if (k == shift16) 	shift = 1;
				if (k == shift32) 	shift = 2;
				if (k == shift48) 	shift = 3;
			}
			const uint32_t to_emit = 
				(sf << 31U) | 
				(opc << 29U) | 
				(0x25U << 23U) | 
				(shift << 21U) | 
				(Im0 << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		} 

		else if (op == bfm) {

			const uint32_t bit_count = Im0;
			const uint32_t position = Im1;
			const uint32_t max = (sf ? 64 : 32);

			if (not bit_count) { puts("bad field bit count"); abort(); }
			if (not position) { puts("???? bfm position zero??"); abort(); }
			if (position + bit_count > max or position >= max) {
				puts("bfm: bit field outside register width");
				abort();
			}

			uint32_t opc = 1, msb = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == signed_) 	opc = 0;
				if (k == unsigned_) 	opc = 2;
				if (k == not_) 		msb = 1;
			}

			const uint32_t N = sf;
			uint32_t imms = 0, immr = 0;
			if (not msb) {
				imms = position + bit_count - 1;
				immr = position;
			} else {
				imms = bit_count - 1;
				immr = max - position;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(opc << 29U) | 	
				(0x26U << 23U) | 
				(N << 22U) | 
				(immr << 16U) |
				(imms << 10U) |
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		} 

		else if (op == csel) {

			uint32_t o1 = 2, o2 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == incr) 		o2 = 1;
				if (k == inv) 		o1 = 1;
			}

			const uint32_t cond = parse_condition(ins[i].modifiers);

			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(0xD4 << 21U) | 
				(Rm << 16U) | 
				(cond << 12U) | 
				(o2 << 10U) | 
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		} 

		else if (op == mul) {

			uint32_t o0 = 0, o1 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == inv) 		o0 = 1;
				if (k == signed_) 	o1 = 1;
				if (k == unsigned_) 	o1 = 5;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(0x1B << 24U) | 
				(o1 << 21U) |
				(Rm << 16U) |
				(o0 << 15U) | 
				(Ra << 10U) | 
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == div_) {

			uint32_t op1 = 0xD6, s = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == unsigned_) s = 0;
				if (k == signed_) s = 1;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(op1 << 21U) | 
				(Rm << 16U) | 
				(1 << 11U) |
				(s << 10U) |
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}
	}

*/







/*
	nat goto0[4096] = {0}, goto1[4096] = {0};

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		const nat arg1 = ins[i].args[1];
		if (op == jmp) {
			goto0[i] = (nat) -1;
			goto1[i] = rt_locations[arg1];
		} else if (op == br) {
			goto0[i] = (nat) -1;
			goto1[i] = (nat) -1;
		} else if (op == halt) {
			goto0[i] = (nat) -1;
			goto1[i] = (nat) -1;
		} else if (op == bc) {
			goto0[i] = i + 1;
			goto1[i] = rt_locations[arg1];
		} else if (op == cbz) {
			goto0[i] = i + 1;
			goto1[i] = rt_locations[arg1];
		} else if (op == tbz) {
			goto0[i] = i + 1;
			goto1[i] = rt_locations[arg1];
		} else {
			goto0[i] = i + 1;
			goto1[i] = (nat) -1;
		}
	}

	puts("printing control flow graph:");
	for (nat i = 0; i < ins_count; i++) {
		printf("[%3llu]: %8s --> { f=%3lld, t=%3lld }\n", 
			i, operations[ins[i].op], goto0[i], goto1[i]
		);
	}
	puts("[done]");

	printf("finding preds..\n");
	for (nat i = 0; i < ins_count; i++) {
		nat pred_count = 0;
		nat* preds = get_preds(
			&pred_count, i, 
			ins, ins_count, 
			goto0, goto1
		);

		printf("[pc=%llu]: %8s  --> ", 
			i, operations[ins[i].op]
		);
		printf(" : preds ");
		print_nats(preds, pred_count);
	}

	puts("done");	

	nat stack_count = 0;
	nat stack[4096] = {0};
	for (nat i = 0; i < ins_count; i++) 
		if (ins[i].op == halt) stack[stack_count++] = i; 

	puts("starting stack for backwards traversal:");
	print_nats(stack, stack_count);	
	nat visited[4096] = {0};
	nat* live_in = calloc(ins_count * 64 * variable_count, sizeof(nat));

	while (stack_count) {

		const nat pc = stack[--stack_count];
		printf("currently at TOP = %llu\n", pc);

		visited[pc] = 1;

		const nat op = ins[pc].op;
		nat args[max_arg_count];
		memcpy(args, ins[pc].args, sizeof args);
		
		nat pred_count = 0;
		nat* preds = get_preds(
			&pred_count, pc, 
			ins, ins_count, 
			goto0, goto1
		);	

		if (op == mov) {
			if (not args[2] and not args[3] and not args[4]) {

				// args[0] bits(0:15) are overwritten

				puts("doing liveness analysis on movz instruction...");
				abort();

			} else { puts("other mov unimplemented"); abort(); }
		} else if (op == nop) {// nothing
		} else if (op == svc) {// nothing

		} else if (op == bfm) {
				// args[0] is overwritten			
		} else if (op == adc) {
				// args[0] is overwritten
		} else if (op == addx) {
		} else if (op == addi) {
		} else if (op == addr) {
		} else if (op == adr) {
		} else if (op == shv) {
		} else if (op == clz) {
		} else if (op == rev) {
		} else if (op == jmp) { // nothing, unless we are linking
		} else if (op == bc) {//nothing
		} else if (op == br) {
			// reads arg0,   writes the link register if we are linking 
		} else if (op == cbz) {
			// reads arg0
		} else if (op == tbz) {
			// reads arg0

		} else if (op == ccmpi) {
			// reads arg0
		} else if (op == ccmpr) {
			// reads arg0 and arg1

		} else if (op == csel) {
		} else if (op == ori) {
		} else if (op == orr) {

		} else if (op == extr) {
		} else if (op == ldrl) {

		} else if (op == memp) {
		} else if (op == memia) {
		} else if (op == memi) {
		} else if (op == memr) {
		} else if (op == madd) {
		} else if (op == maddl) {
	
		} else if (op == halt) { // do nothing 

		} else if (op == divr) {

			puts("doing data flow analysis on divr ins");
			abort();


		} else { printf("unknown arm64 op code %s\n", operations[op]); abort(); }


		for (nat i = 0; i < pred_count; i++) {
			if (not visited[preds[i]]) stack[stack_count++] = preds[i];
		}
	}	
}

*/



















/*

// do we include register allocation?


	// yes        but not in the way you think.. 

	//     we make it so that    if you define-on-use a variable     without a compiletime value, 

				(note: all variables are CT. yes, i mean this.   ALL VARAIBLES ARE CT. there are ONLY EVER CONSTANTS, NO RUNTIME VARIABLES)

				(the CT value of a "runtime variable" is actually its rt hw reg index.)

		when you define on use a variable,  we simply assign its rt hw reg index (RHRI) value        to be -1  or   some crazy specific value, like 0xA5A5A5A5A5A5A5A5A5 etc idk,

		and then, when we see that THIS particular RHRI is being used   as the register index for a given mi, we perform register allocation, to fix this up. 

		now, we don't need "ri x n"  anymore.  we just use set. ie,    set   myvar 14        allocates myvar to physical register 14,  if we use myvar in the correct spot in a given arm64 mi. 


		so, yeah, we don't need  ri  anymore,  anddd we don't have any distinction between   rt variables, and ct variables!   we doooo however, have a notion of what spots in a given ct or rt instruction are LABELS or not. because we choose a different value based on this distinction of labelness vs ct-variable-ness.   (of course, if it wasnt for this, then labels would BE just ct variables as well lol. )


		so yeah,     RA         truly            i mean this      truly                is just      like         type inference  in other compilers,  

				its an automatic     system that kicks in, when the assembler sees that a given CT-variable was not given a value,   BUTTT it was used in a rt mi. 

				when this happens, we DONTT trigger an error, but rather, assume that the user wanted the assembler to determine the register index. 

									ie,                RA. 





	this is why and how and exactly when  RA will kick in,    and be useful.   


	if you always define the values of your variables,   (ct, mind you!)       then you never need this code path        in the assembler to ever kick in, becuase when it sees that a given instruction has everythng it needs, it just generates it. no RA required. 


	we will actually start with this case, and treat  the case of    ACTUALLY HAVING TO DO   RA          as a special case. that we can handle seperately. (albeit, its quite computationally expensive to support this small little ommision, but whatever, we'll do it lol. )


	so yeah! thats how this works. wow. cool. 




	oh!
	note:
			we will actually be doing    data-flow-analysis   like we currently are      REGARDLESSSSS of whether all registers are allocated or some are or none are 


			we do data flow analysis, just as part of the middle stage of the assembler, before emmiting the binary code. 



hopefully this makes sense.

this is the optimal solution. 





REMEMBER THESE FACTS:

	1. ALL variables EVER are CT constants. 

	2. RA is a special case, where the user doesnt give the value for a define-on-use variable.

	3. the data flow analysis (which happenssss to be useful for RA)  is done regardless.    REGARDLESS.  of the register indexs (or lackthereof) used. 

	4. the compiler does not distinguish between rt and ct inputs.   it only distinguishes between   
				CT inputs, and labels,     becasue of the fact that every ct-variable is both a label and a data ct-variable. 

	5. that is all.   please reread all 5 of these bullet points, and memorize them lol 


















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
//
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
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
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
//
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
	insert_u32(&data, &count, 0); 
	insert_u32(&data, &count, 0); 
	insert_u32(&data, &count, 0); 
//
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
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_SYMTAB);
	insert_u32(&data, &count, 24); 
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_DYSYMTAB);
	insert_u32(&data, &count, 80); 
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_LOAD_DYLINKER);
	insert_u32(&data, &count, 32);
	insert_u32(&data, &count, 12);
	insert_bytes(&data, &count, (char[]){
		'/', 'u', 's', 'r', '/', 'l', 'i', 'b', 
		'/', 'd', 'y',  'l', 'd',  0,   0,   0
	}, 16);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_UUID);
	insert_u32(&data, &count, 24); 
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
//
	insert_u32(&data, &count, LC_BUILD_VERSION);
	insert_u32(&data, &count, 32);
	insert_u32(&data, &count, PLATFORM_MACOS);
	insert_u32(&data, &count, 13 << 16);
	insert_u32(&data, &count, (13 << 16) | (3 << 8));
	insert_u32(&data, &count, 1);
	insert_u32(&data, &count, TOOL_LD);
	insert_u32(&data, &count, (857 << 16) | (1 << 8));
//
	insert_u32(&data, &count, LC_SOURCE_VERSION);
	insert_u32(&data, &count, 16);
	insert_u64(&data, &count, 0);
//
	insert_u32(&data, &count, LC_MAIN);
	insert_u32(&data, &count, 24);
	insert_u64(&data, &count, 16384 - my_count);
	insert_u64(&data, &count, stack_size); // put stack size here
//
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

//	puts("");
//	puts("bytes: ");
//	for (nat i = 0; i < count; i++) {
//		if (i % 32 == 0) puts("");
//		if (data[i]) printf("\033[32;1m");
//		printf("%02hhx ", data[i]);
//		if (data[i]) printf("\033[0m");
//	}
//	puts("");

	//puts("preparing for writing out the data.");

	const bool overwrite_executable_always = true;

	if (not access("output_executable_new", F_OK)) {
		//printf("file exists. do you wish to remove the previous one? ");
		//fflush(stdout);
		if (overwrite_executable_always or getchar() == 'y') {
			puts("file was removed.");
			int r = remove("output_executable_new");
			if (r < 0) { perror("remove"); exit(1); }
		} else {
			puts("not removed");
		}
	}
	int file = open("output_executable_new", O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("could not create executable file"); exit(1); }
	int r = fchmod(file, 0777);
	if (r < 0) { perror("could not make the output file executable"); exit(1); }

	write(file, data, count);
	close(file);
	printf("wrote %llu bytes to file %s.\n", count, "output_executable_new");
	system("codesign -s - output_executable_new");


// debugging:

	//system("otool -htvxVlL output_executable_new");
	system("objdump -D output_executable_new");

}

*/

































//print_instruction(ins[i], variables, operations, arity, variable_count, operation_count);





	/*for (nat i = 0; i < variable_count; i++) {
		printf("\t RA CONSTRAINT: %s has register index r[%llu].\n", variables[i], ra_constraint[i]);
	}*/

	// begin register allocation here!!!!
	// [DONE] step 0. form the control flow graph which represents the machine code instructions.
	// step 1. form liveness analysis
	// step 2. form register interference graph
	// step 3. use graph coloring algorithm to find an ordering in the rig. 
	// step 4. done! now generate machine code. 

	
	// step 1: liveness analysis. 

	// we first need to start at the last instruction, and work our way backwards through the control flow graph, 
	// slowly setting the live in and live out of particular instructions, based on which registers are input registers, and which are output registers. 
	// for every program, point, we have a boolean of whether or not it is live at that point. 
	// we continue following the control flow graph backwards, until nothing changes on an iteration, or we have traversed all nodes, i think. 
	

	// we first have to start by tracing through the control flow graph   in the backwards order. we can use the standard DFS traversal algortihm for this, but we use predecessors instead of .goto's (successors)  for our child pointers lol. 

	// lets first walk through the cfg backwards, starting from the last instruction.









/*if (a == 0 and this.op >= def0 and this.op < ret) {
			printf("(\"%7s\") ", this.args[a] < operation_count ? operations[this.args[a]] : "");			
		}
		else printf("'%s' ", this.args[a] < name_count ? names[this.args[a]] : "");*/


/*static const nat arm64_rt_arity[arm64_isa_count - ct_isa_count] = {
	0, 0, 1, 2,
	3, 3, 2, 3, 1, 
	3, 2, 2, 0, 0, 1, 
	1, 1, 1, 2, 3, 
	2, 3, 3, 1, 
	3, 2, 2, 3, 
	4, 4, 3, 
};*/



/*static nat is_runtime_arg(nat op, nat arg) {
	if (op < ct_isa_count) { 
		printf("error: is_runtime_arg: op < ct_isa_count: op=%llu\n", op); 
		abort(); 
	}

	if (op >= arm64_isa_count) {
		printf("error: is_runtime_arg: op >= arm64_isa_count: op=%llu\n", op); 
		abort(); 
	}

	return arg < arm64_rt_arity[op - ct_isa_count];
}*/


















	// thought:        please make it so that we generate    "at <label>" instructions   as runtime instructions     so that we don't rely on absolute position of a given instruction in the sequence, but rather, we can form the control flow graph that is invariant to how things move around! this is important, i think. 



	// 






































































					//printf("WARNING: performed global_replacement: found %llu(%s), replaced with %llu(%s).\n",
					//	args[a], variables[args[a]], 
					//	replace_with[r], variables[replace_with[r]]
					//); 
					//getchar();

		/*if (filestack_count == 1) {
			puts("\n\n\n\n");
			print_variables(var_stack, var_stack_count, variables, values, locations, ra_constraint, variable_count);
			print_operations(op_stack, op_stack_count, operations, arity, parameters, observable, operation_count, variables);
			print_index(text, text_length, word_start, pc);

			
			printf("[pc=%llu][com=%llu][def=%llu][skip=%llu]: w=\"%s\" w_st=%llu\n", 
				pc, comment, def_stack_count, skipping, word, word_start
			);
			printf("var = %llu, op = %llu, args = { %llu %llu %llu %llu }, arg_count = %llu\n", 
				var, temp_op, 
				temp_args[0], temp_args[1], 
				temp_args[2], temp_args[3], 
				temp_arg_count 
			);
			//getchar();
		}*/





			//abort();
			//puts("executing defN: creating a ct function definition!");
			//getchar();
			//printf("arg0 = %llu\n", arg0);
			//printf("arity[arg0] = %llu\n", arity[arg0]);
			//printf("args[] = (){ ");
			//for (nat i = 0; i < arg_count; i++) {
			//	printf("%llu ", args[i]);
			//}
			//puts("}");
			//getchar();

			//getchar(); getchar(); 
			//print_operations(op_stack, op_stack_count, operations, arity, 
			//parameters, observable, operation_count, variables);
			//getchar();



/*static void print_dictionary(char** names, nat* array1, nat* array2, nat* array3, nat count, bool mode) {

	printf(" %s dictionary (%llu entries) {\n", mode ? "operation" : "variable", count);
	for (nat i = 0; i < count; i++) {
		printf("\t%3llu: \"%-10s\", %s = %3lld, %s = %3lld, %s = %3lld\n", 
			i, names[i], 
			mode ? "arity" : "value", array1[i], 
			mode ? "observable" : "location", array2[i], 
			mode ? "operation_defined_in" : "variable_defined_in", array3[i]
		);
	}
	puts("}");
}

static void print_instruction(struct instruction this, char** names, char** operations, nat* arity, nat name_count, nat operation_count) {
	printf("  %10s { ", operations[this.op]);
	for (nat a = 0; a < arity[this.op]; a++) { 
		if (this.args[a] < 256) printf("%3llu", this.args[a]); 
		else if (this.args[a] == (nat) -1) printf("%3lld", this.args[a]);
		else printf("0x%016llx", this.args[a]);		
		if (a == 0 and this.op >= def0 and this.op < ret) {
			printf("(\"%7s\") ", this.args[a] < operation_count ? operations[this.args[a]] : "");			
		}
		else printf("('%7s') ", this.args[a] < name_count ? names[this.args[a]] : "");
	}
	printf("}");
}

static void print_instructions(
	struct instruction* ins, nat ins_count, 
	char** names, char** operations, nat* arity, nat name_count, nat operation_count
) {
	puts("found instructions: {");
	for (nat i = 0; i < ins_count; i++) {
		if ((0)) printf("\033[38;5;239m");
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, operations, arity, name_count, operation_count);
		puts("");
		if ((0)) printf("\033[0m");
	}
	puts("}");
}*/

/*static void print_instruction_index(
	struct instruction* ins, nat ins_count, 
	char** names, char** operations, nat* arity, 
	nat name_count, nat operation_count, 
	nat here, const char* message
) {
	printf("%s: at index: %llu: { \n", message, here);
	for (nat i = 0; i < ins_count; i++) {
		if ((0)) printf("\033[38;5;239m");
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, operations, arity, name_count, operation_count);
		if (i == here)  printf("  <------ %s\n", message); else puts("");
		if ((0)) printf("\033[0m");
	}
	puts("}");
}*/




// instead of using values[]        we need to have  each macro call     (ie, each call frame)    have its own local values array. 
//	like, stack variables   for a rt function. 
// this will allow us to define local labels, which are truly local, allowing for local control flow generation. (which is a first class use case of course.)





	//print_dictionary(operations, arity, observable, operation_defined_in_scope, operation_count, 1);
	//print_dictionary(variables, values, locations, variable_defined_in_scope, variable_count, 0);
	//print_instructions(ins, ins_count, variables, operations, arity, variable_count, operation_count);



//"def",            TODO: 				should we have "def"...... hmmmmmmm


/*



	nat if_seen[max_arg_count * 4096] = {0};
	nat replace_with[max_arg_count * 4096] = {0};
	nat replacement_count[4096] = {0};




						//printf("%s vs %s : operation name mismatch\n", 
						//	word, operations[state]
						//); 
						continue; 
					}
					//printf("comparing scopes: operation_defined_in_scope"
					//	"[state](%llu) ==? scopes[i](%llu)\n", 
					//	operation_defined_in_scope[state], 
					//	scopes[i]
					//);
					//if (operation_defined_in_scope[state] != scopes[i]) { 
					//	//puts("scope mismatch"); 
					//	continue; 
					//}
					//puts("SUCESSFUL OPERATION RECOGNIZED");

				//printf("searching for operations in scope i=%llu: #%llu..\n", i, scopes[i]);




			//printf("searching for operations in scope i=%llu: #%llu..\n", i, scopes[i]);
			for (variable = 0; variable < variable_count; variable++) {

		for (nat i = op_stack_count; i--;) {
			op = op_stack[i];
			if (not strcmp(word, operations[op])) goto process_op;
		}
		op = (nat) -1;


			printf("%s vs %s : variable name mismatch\n", 
					word, variables[variable]); 
					continue; 
				}
				if (variable_defined_in_scope[variable] != scopes[i]) { 
					puts("scope mismatch"); 
					continue; 
				} 
				puts("SUCESSFUL VARIABLE RECOGNIZED");
				goto variable_name_found;
			}			

variable_defined_in_scope[variable_count] = scopes[scope_count - 1];


	nat variable_defined_in_scope[4096] = {0};

	nat operation_defined_in_scope[4096] = {0};

	nat variable_inactive[4096] = {0};

	nat operation_inactive[4096] = {0};



			//puts("calling OBS!");
			//printf("def_location = %llu\n", def_location);
			//printf("ins[def_location].op = %llu (\"%s\")\n", ins[def_location].op, operations[ins[def_location].op]);

			const nat calling = operation_count - 1;

			//printf("calling = %llu\n", calling);
			//printf("arity[calling] = %llu\n", arity[calling]);

			for (nat i = 0; i < arity[calling]; i++) {
				printf("trying argument #%llu / %llu... comparing .args[%llu]:%llu and obs:%llu\n", 
					i, arity[calling], i + 1, ins[def_location].args[i + 1], variable
				);
				getchar();
				if (ins[def_location].args[i + 1] == variable) {
					observable[calling] |= 1 << i;
					//printf("info: FOUND OBSERVABLE!!\n");//getchar();
				} //else printf("error: mismatch in finding observable!!\n");//getchar();
			}

*/



			/*

obs imple:


			const nat calling = operation_count - 1;
			for (nat i = 0; i < arity[calling]; i++) 
				if (ins[(((((0)))))].args[i + 1] == var) observable[calling] |= 1 << i;


*/

		//print_dictionary(operations, arity, observable, operation_defined_in_scope, operation_count, 1);
		//print_dictionary(variables, values, locations, variable_defined_in_scope, variable_count, 0);
		//print_instructions(ins, ins_count, variables, operations, arity, variable_count, operation_count);





	//nat def_stack_count = 0;

	//nat global_if_seen[4096] = {0};
	//nat global_replace_with[4096] = {0};
	//nat global_replacement_count = 0;

	//struct instruction rt_ins[4096] = {0};
	//nat rt_ins_count = 0;

/*

	//nat locations[4096] = {0}; memset(locations, 255, sizeof locations);
	//nat values[4096] = {0};
	nat definitions[4096] = {0};
	nat stack[4096] = {0};
	nat stack_count = 0;
	nat def_stack_count = 0;
	nat register_constraint[4096] = {0}; memset(register_constraint, 255, sizeof register_constraint);
	nat target_architecture = no_arch;
	nat if_seen[max_arg_count * 4096] = {0};
	nat replace_with[max_arg_count * 4096] = {0};
	nat replacement_count[4096] = {0};
	nat global_if_seen[4096] = {0};
	nat global_replace_with[4096] = {0};
	nat global_replacement_count = 0;

	struct instruction rt_ins[4096] = {0};
	nat rt_ins_count = 0;

	for (nat i = 0; i < ins_count; i++) if (ins[i].op == at) locations[ins[i].args[0]] = i;






	//nat in_current_function = 0;

	nat pc = 0;
	while (pc < ins_count) {
		//printf("executing this instruction: [pc = %llu] : ", pc);
		//print_dictionary(operations, arity, observable, operation_defined_in_scope, operation_count);
		//print_instruction_index(ins, ins_count, variables, operations, arity, variable_count, operation_count, ignore, pc, "PC"); puts("");
		//print_dictionary(variables, locations, values, variable_defined_in_scope, variable_count);
		//print_dictionary(variables + (variable_count - 10), locations + (variable_count - 10), values + (variable_count - 10), variable_defined_in_scope + (variable_count - 10), 10);
	
		//print_stack(stack, replacement_count, if_seen, replace_with, stack_count);
		const nat op = ins[pc].op;
		//printf("op = %llu (\"%s\")\n", op, operations[op]);

		nat args[max_arg_count] = {0};
		memcpy(args, ins[pc].args, sizeof args);
		
		for (nat a = 0; a < arity[op]; a++) {
			for (nat r = 0; r < global_replacement_count; r++) {
				if (args[a] != global_if_seen[r]) continue;
				printf("WARNING: performed global_replacement: found %llu(%s), replaced with %llu(%s).\n",
					args[a], variables[args[a]], 
					global_replace_with[r],
					variables[global_replace_with[r]]);//getchar();
				args[a] = global_replace_with[r];
			}
		}

		if (not stack_count) goto skip_replacement;
		for (nat a = 0; a < arity[op]; a++) {
			for (nat r = 0; r < replacement_count[stack_count - 1]; r++) {
				if (args[a] != if_seen[ max_arg_count * (stack_count - 1) + r]) continue;
				printf("WARNING: performed argument replacement: found %llu(%s), replaced with %llu(%s).\n",
					args[a], variables[args[a]], 
					replace_with[max_arg_count * (stack_count - 1) + r],
					variables[replace_with[max_arg_count * (stack_count - 1) + r]]);
				args[a] = replace_with[max_arg_count * (stack_count - 1) + r];
			}
		}
	
		skip_replacement:; 
		const nat arg0 = args[0];
		const nat arg1 = args[1];
		const nat arg2 = args[2];

		if (op >= def0 and op < ret) {
			//puts("found def!");
			const nat n = ins[pc].args[0];
			//printf("ins[pc].args[0] = %llu\n", n);
			definitions[n] = pc;
			def_stack_count++;
			goto next_instruction;
		} else if (op == ret) {
			if (def_stack_count) { def_stack_count--; goto next_instruction; }
			if (not stack_count) { puts("NOT STACK COUNT IN RET"); abort(); }
			pc = stack[--stack_count];
			//puts("returning...");
			goto next_instruction;

		} else if (def_stack_count) goto next_instruction;

		if (op == def) {
			printf("executing a def statement... adding a "
				"replacement of [if_seen=%llu:%s, replace_with=%llu:%s]\n", 
				arg0, variables[arg0], arg1, variables[arg1]
			);

			global_if_seen[global_replacement_count] = arg0;
			global_replace_with[global_replacement_count++] = arg1;
			//abort();
			//getchar();
		}
		else if (op == settarget) target_architecture = values[arg0];
		//else if (op == rt) bit_count[arg0] = values[arg1];
		else if (op == ri) register_constraint[arg0] = values[arg1];
		else if (op == zero) values[arg0] = 0;
		else if (op == incr) values[arg0]++;
		else if (op == decr) values[arg0]--;
		else if (op == not_) values[arg0] = ~values[arg0]; 
		else if (op == set)  values[arg0] = values[arg1];
		else if (op == add)  values[arg0] += values[arg1];
		else if (op == sub)  values[arg0] -= values[arg1];
		else if (op == mul)  values[arg0] *= values[arg1];
		else if (op == div_) values[arg0] /= values[arg1];
		else if (op == rem)  values[arg0] %= values[arg1];
		else if (op == and_) values[arg0] &= values[arg1];
		else if (op == or_)  values[arg0] |= values[arg1];
		else if (op == eor)  values[arg0] ^= values[arg1];
		else if (op == si)   values[arg0] <<= values[arg1];
		else if (op == sd)   values[arg0] >>= values[arg1];
		else if (op == ld)  { printf("executing LD"); values[arg0] = values[arg1]; abort(); }
		else if (op == st)  { printf("executing ST"); values[arg0] = values[arg1]; abort(); }
		else if (op == at) locations[arg0] = pc;
		else if (op == do_) pc = locations[arg0];
		else if (op == lt) { if (values[arg0]  < values[arg1]) pc = locations[arg2]; }
		else if (op == ge) { if (values[arg0] >= values[arg1]) pc = locations[arg2]; }
		else if (op == eq) { if (values[arg0] == values[arg1]) pc = locations[arg2]; }
		else if (op == ne) { if (values[arg0] != values[arg1]) pc = locations[arg2]; }
		else if (op == ctdebug)  { printf("ctdebug: %llu\n", values[arg0]); }//getchar(); } 
		else {
			if (target_architecture == no_arch) {
				// treat it like a macro function call!
			call_macro:
				for (nat r = 0; r < arity[op]; r++) {
					if_seen[max_arg_count * stack_count + r] = ins[definitions[op]].args[1 + r];
					replace_with[max_arg_count * stack_count + r] = ins[pc].args[r];
				}
				
				replacement_count[stack_count] = arity[op];
				stack[stack_count++] = pc;
				pc = definitions[op];
				//puts("calling macro...");
				goto next_instruction;
			} else if (target_architecture) {
				if (op >= arm64_isa_count) goto call_macro;
				struct instruction new = { .op = op };
				memcpy(new.args, args, sizeof args);
				for (nat i = 0; i < arity[op]; i++) 
					if (not is_runtime_arg(op, i)) 
						new.args[i] = values[args[i]]; 
					//else if (((observable[op] >> i) & 1) ) 
					//	bit_count[args[i]] = 64;
				rt_ins[rt_ins_count++] = new;
			} else { puts("unknown arch so far..."); abort(); }
			//puts("unknown execution for instruction");
			//printf("op = %llu (\"%s\")\n", op, operations[op]);
			//abort();
		}
	next_instruction:
		//getchar();
		pc++;
	}
	
	puts("finished executing CT instruction");	
	print_dictionary(operations, arity, observable, operation_defined_in_scope, operation_count, 1);
	print_dictionary(variables, locations, values, variable_defined_in_scope, variable_count, 0);
	print_instruction_index(ins, ins_count, variables, operations, arity, variable_count, operation_count, ignore, ins_count, ""); 
	puts("");
	printf("[INFO]: assembling for target_arch = %llu\n", target_architecture);
	print_instructions(rt_ins, rt_ins_count, variables, operations, arity, variable_count, operation_count, ignore);

	for (nat i = 0; i < variable_count; i++) {
		if (register_constraint[i] != (nat) -1) 
			printf("\t RA CONSTRAINT: %s has register index r[%llu].\n", variables[i], register_constraint[i]);
	}

}


*/























/*


	for (nat i = 0; i < variable_count; i++) {
		if (bit_count[i]) 
			printf("\t BC CONSTRAINT: %s is %llu bits wide.\n", variables[i], bit_count[i]);
	}

*/































/*static void print_stack(nat* stack, nat* replacement_count, nat* if_seen, nat* replace_with, nat stack_count) {
	printf("stack: %llu { \n", stack_count);
	for (nat i = 0; i < stack_count; i++) {
		printf("stack[%llu] =  { pc = %llu , replacement_count=%llu, { ", 
			i, stack[i], replacement_count[i]
		);
		for (nat r = 0; r < replacement_count[i]; r++) {
			printf("(%llu:%llu) ", if_seen[max_arg_count * i + r], replace_with[max_arg_count * i + r]);
		}
		puts(" } ");
	}
	puts("}");
}*/





// previous list of arm64 machine instructions:

//	addx, addi, addr, adr,



/*	adcs,
	addx, addi, addr, addxs, addis, addrs, 
	adr, adrp,
	andi, andr, andis, andrs,
	asrv, jmp, bc, 
	bfm, bicr, bicrs,
	jmpl, brl, br, cbnz, cbz, 
	ccmni, ccmnr, ccmpi, ccmpr, 
	clz, cls, csel, csinc, csinv, csneg, 
	eonr, eori, eorr, extr, 
	ldnp, ldp, ldpe, ldpo, 
	ldri, ldrie, ldrio, ldrl, ldrr, ldur, 
	lslv, lsrv, madd, msub, 
	movk, movz, movn, nop,
	ornr, ori, orr, 
	rbit, return_, rev, rorv, 
	sbc, sbcs, 
	sbfm, sdiv, smaddl, smsubl, smulh, 
	stnp, stp, stpe, stpo,
	stri, strie, strio, strr, stur, 
	subx, subi, subr, subxs, subis, subrs,
	svc, tbnz, tbz, 
	ubfm, udiv, umaddl, umsubl, umulh,
*/



/*


	if ((op == ccmni or op == ccmpi) and (arg == 1 or arg == 2 or arg == 3)) return 0;


	if ((	op == ccmnr or 
		op == ccmpr or
		op == ldri or
		op == ldrie or
		op == ldrio or
		op == ldur or
		op == stri or
		op == strie or
		op == strio or
		op == stur or
		op == 
	) and (arg == 2 or arg == 3)) return 0;


	if ((	op == addx or op == addr or op == addxs or op == addrs or 
		op == andr or op == andrs or 
		op == eonr or op == eorr or 
		op == ldnp or op == ldp or op == ldpe or op == ldpo or op == ldrr or
		op == ornr or op == orr or
		op == stnp or op == stp or op == stpe or op == stpo or op == strr or
		op == subx or op == subr or op == subxs or op == subrs or 
		
	) and (arg == 3 or arg == 4)) return 0;


	if ((	op == bc or
		op == 
	) and (arg == 0 or arg == 1)) return 0;

	if ((	op == ldrl or
		op == 
	) and (arg == 1 or arg == 2)) return 0;



	if ((	op == bicr or 
		op == bicrs or
		op == csel or 
		op == csinc or
		op == csinv or
		op == csneg or
		op == extr or 
		op == 
	) and arg == 3) return 0;


	if ((	op == addi or 
		op == addis or
		op == andi or 
		op == andis or 
		op == bfm or
		op == eori or
		op == ori or
		op == sbfm or
		op == 
	) and arg == 2) return 0;


	if ((	op == adr or 
		op == adrp or
		op == cbnz or 
		op == cbz or
		op == movk or
		op == movz or
		op == movn or
		op == 
	) and arg == 1) return 0;


	if ((	op == jmp or
		op == jmpl or
		op == 
	) and arg == 0) return 0;


*/
















/*enum arm64_ins_set {
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

*/












/*	printf("} : {.f=#");
	if (this.gotos[0] == ~is_label or this.gotos[0] == (nat) -1) {} 
	else if (this.gotos[0] < 256) printf("%3llu", this.gotos[0]); 
	else printf("0x%016llx", this.gotos[0]);
	printf(", .t=#");
	if (this.gotos[1] == ~is_label or this.gotos[1] == (nat) -1) {} 
	else if (this.gotos[1] < 256) printf("%3llu", this.gotos[1]); 
	else printf("0x%016llx", this.gotos[1]);
	printf("}");
*/

/*

static void print_ct_values(char** names, nat name_count, nat* is_runtime, nat* values) {
	printf("ct values: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (is_runtime[i]) continue;
		printf("\tvalues[%s] = 0x%016llx\n", names[i], values[i]);
	}
	puts("}");
}

static void print_machine_instruction(struct instruction this, char** names, nat name_count) {
	printf("  %13s { ", mi_spelling[this.op]);
	for (nat a = 0; a < 5; a++) {
		if (this.op == csel and a == 3) { printf("        #{%s}   ", ins_spelling[this.args[a]]); continue; }
		if (this.args[a] < 256) printf("%3llu", this.args[a]); 
		else printf("0x%016llx[%llu]", this.args[a], this.args[a]);
		printf("('%8s') ", this.args[a] < name_count ? names[this.args[a]] : "");
	}

	printf("}");


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
	char** names, char** operations, nat* arity, nat name_count, nat operation_count, nat* ignore,
	nat here, const char* message
) {
	printf("%s: at index: %llu: { \n", message, here);
	for (nat i = 0; i < ins_count; i++) {
		if (ignore[i]) printf("\033[38;5;239m");
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, operations, arity, name_count);

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
		print_instruction(ins[i], names, operations, name_count);
		if (i == here)  printf("  <------ %s\n", message); else puts("");
		if (ignore[i]) printf("\033[0m");
	}
	puts("}");
}

static void print_ct_values(char** names, nat name_count, nat* is_runtime, nat* values) {
	printf("ct values: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (is_runtime[i]) continue;
		printf("\tvalues[%s] = 0x%016llx\n", names[i], values[i]);
	}
	puts("}");
}
*/










/*


def1 create x obs x 
	ret
create a
create b


def4 csel x y z cond
	obs x
	...body....
	ret


*/






/*	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].gotos[0] & is_label) ins[i].gotos[0] = locations[ins[i].gotos[0] & ~is_label];
		if (ins[i].gotos[1] & is_label) ins[i].gotos[1] = locations[ins[i].gotos[1] & ~is_label];
	}






fixed arity:
------------

def4 csel x y z cond
	. this is a function which conditionally selects x or y .
	...code here...
	ret


comments == macros:
------------

. csel x y z cond .
	. this is a function which conditionally selects x or y . . . 
	...code here...
	ret


IDEAL:
------------

def csel x y z cond 
	. this is a function which conditionally selects x or y .
	...code here...
	ret




IDEAL, comments == macros:
------------

. csel x y z cond . 

	...code here...

. .                       <--------- this is a return statement, kinda. 

	





now, technically speaking we could also use parens:




	(csel x y cond) 

		...code for csel...

	()




ehhh, i kinda like this better:




. csel x y z cond . 

	...code here...

. .                       <--------- this is a return statement, kinda. 











the above solution only uses     "."  as a special word,  for both   def, ar, ret, and comments!
	so i think in terms of making the simplest possible language, and syntax, this is the best solution. 


		i think... idk. lol. but yeah, probably lol. yay






...hmmm yeah, i think just  "dot" is the right solution, ie, "."   because like its the one character that is kinda special, and should mean something special, i think. and like, we need to pick somethinggg to have this role lol. so yeah. cool yayy




this should workkkk 


1202502263.204921












*/



















































































/*



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



static void print_instructions_ct_values_index(
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
}


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

static const nat is_label = 1LLU << 63LLU;


static nat get_call_output_count(nat n) {
	if (n == system_exit) return 0;
	if (n == system_read) return 2;
	if (n == system_write) return 2;
	if (n == system_close) return 1;
	if (n == system_open) return 2;
	abort();
}














	nat stack_count = 1;
	nat* stack = calloc(ins_count, sizeof(nat));
	nat* visited = calloc(ins_count + 1, sizeof(nat));
	nat values[4096] = {0};
	memset(values, 255, sizeof values);

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
	//getchar();

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

		else if (op == sc) {
			struct instruction new = { .op = svc };
			mi[mi_count++] = new;
			selected[i] = 1;
			goto finish_mi_instruction;
		}

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






















































































//     in c, this would translate to:
//
//        = + - * / & | ^ << >> < == >= != goto : ____ ____ syscall() int/long  #include ____
//
// roughly speaking. 






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


*/

// a new parser and ct system written on 1202502016.131044 dwrr
// 1202501116.181306 new parser   dwrr
























/*

	set s b
	si_imm s k
	set x z
	ge a s skip
		set x y
	at skip


becomes:

	subs XZR, a, b << k
	csel x, y, z, lt



*/























	/*for (nat pc = 0; pc < ins_count; pc++) {
		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];		
		if ((op == lt or op == eq) and not is_runtime[arg0] and not is_runtime[arg1]) {
			const bool condition = op == lt
				? (values[arg0] < values[arg1])
				: (values[arg0] == values[arg1]);
			const nat g = ins[pc].gotos[condition];
			ins[pc].op = eq;
			ins[pc].gotos[0] = (nat) -1;
			ins[pc].gotos[1] = g;
			ins[pc].args[0] = 0;
			ins[pc].args[1] = 0;
		}
	}

	for (nat pc = 0; pc < ins_count; pc++) {
		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];
		if (op == eq and not arg0 and not arg1 and ins[pc].gotos[1] == pc + 1) ignore[pc] = 1;
	}*/










		/*else {
			print_instruction(ins[pc], names, name_count); printf("   : mismatch :   %u   %u(%s != %s)  %u %u \n", 
				pred_count == 1,
				op == expected_op, ins_spelling[op], ins_spelling[expected_op],
				not use_arg0 or expected_arg0 == arg0,
				not use_arg1 or expected_arg1 == arg1
			);
		}*/











/*static void print_machine_instructions(struct instruction* mi, nat mi_count) {
	printf("printing machine instructions: (%llu)\n", mi_count);
	for (nat i = 0; i < mi_count; i++) {
		printf("#%llu: MACHINE INSTRUCTION:  "
			" %s  :  %llu %llu %llu %llu %llu \n",
			i, mi_spelling[mi[i].op],
			mi[i].args[0], mi[i].args[1],
			mi[i].args[2], mi[i].args[3], 
			mi[i].args[4]
		);
	}
}*/




//printf("SELECTION: FOUND A PC OF: %llu\n", pc); //getchar();
// printf("FOUND RT BRANCH @ %llu\n", pc); 

		//print_instruction_index(ins, ins_count, names, name_count, ignore, pc, "FOLLOWING");
		//printf("following pc #%llu\n", pc);
		//print_instruction(ins[pc], names, name_count); puts("");
		//printf("SUCCESS: FOUND: WE FOUND WHAT WE WERE LOOKING FOR!!!\n");



/*printf(
				"failed: we didnt find a match here: \n"
				"\t expected op: %s      found op: %s  --->  [%s]\n"
				"\t expected arg0: %lld     found arg0: %lld   ---> [%s]\n"
				"\t expected arg1: %lld     found arg1: %lld  ---> [%s]\n"
				"\n\n", 
				ins_spelling[expected_op], ins_spelling[op],   op == expected_op ? "MATCHES" : "mismatch",
				expected_arg0, arg0, (not use_arg0 or expected_arg0 == arg0) ? "MATCHES" : "mismatch",
				expected_arg1, arg1, (not use_arg1 or expected_arg1 == arg1) ? "MATCHES" : "mismatch"
			);*/












		/*	


// we need to check that the number of reads from "s" is only contained in this pattern. thats key. hmm. okay. 




					msub	set s m   mul s n   set d a   sub d s


				there are exactly 3 reads of s...  thats what we should expect...?  or maybe we just require that  if there is a subsequent read, we do a write first?... hmmmmmm...
				 i feel like thats more general and more correct, but harder to implement, idk.. hmmm  the count of the number of uses is easier lololl..idk.. hmmm we'll probably just do that. thats all we need for us i think. 



















set s m
si_imm s k
lt n s label

	-->	subs_srlsl ZXR, N, M << K
		bc.lt LABEL













--------------------------- ARM64 INS SEL PATTERNS -----------------------------

movz	setimm x k 

lslv	set d m si d n
lsrv	set d m sd d n

udiv	set d n div d m

madd	set d m mul d n add d a
msub	set s m mul s n set d a sub d s



addsr	set d m siimm d k add d n 
subsr	set s m siimm s k set d n sub d s

andsr	set d m siimm d k and d n

orrsr	set d m siimm d k or d n
ornsr	set d m siimm d k not d or d n

eorsr	set d m siimm d k eor d n
eonsr	set d m siimm d k not d eor d n



addi	set d n addimm d k

subi	set d n subimm d k

andi	set d n andimm d k

orri	set d n orimm d k

eori	set d n eorimm d k



--------------------------- END OF ARM64 INS SEL PATTERNS -----------------------------

*/









































































































































		/*if (op == add_imm) {
			const nat d = arg0;
			const nat n = d;
			const nat k = arg1;
			
			struct instruction new = {0};
			new.op = addi;
			new.args[0] = d;
			new.args[1] = n;
			new.args[2] = k;
			mi[mi_count++] = new;

			selected[i] = 1;
			goto finish_mi_instruction;
		}*/







/*



	if (argc != 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));
	printf("isa_count = %u\n", isa_count);
	
	char* names[4096] = {0};
	nat name_count = 0;

	struct instruction* ins = NULL;
	nat ins_count = 0;

	struct file filestack[4096] = {0};
	nat filestack_count = 1;

	const char* included_files[4096] = {0};
	nat included_file_count = 0;

	{int file = open(argv[1], O_RDONLY);
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
	printf("file: (%llu chars)\n<<<%s>>>\n", text_length, text);}

	for (nat i = 0; i < builtin_count; i++) names[name_count++] = strdup(builtin_spelling[i]);

process_file:;

	nat word_length = 0, word_start = 0, first = 1, comment = 0, arg_count = 0;

	const nat starting_index = 	filestack[filestack_count - 1].index;
	const nat text_length = 	filestack[filestack_count - 1].text_length;
	char* text = 			filestack[filestack_count - 1].text;
	const char* filename = 		filestack[filestack_count - 1].filename;

	for (nat index = starting_index; index < text_length; index++) {




		if (arg_count != isa_arity(ins[ins_count - 1].op)) goto next_word;
		if (ins[ins_count - 1].op == lf) {
			ins_count--;
			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				printf("warning: %s: file already included\n", word);
				goto finish_instruction;
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
			goto process_file;
		} 
		finish_instruction: first = 1;
		next_word: word_length = 0;
	}
	filestack_count--;
	if (filestack_count) goto process_file;

*/
	//const char* filename = argv[1];
	//char* text = NULL;
	//nat text_length = 0;







/*{
	int f = open(filename, O_RDONLY);
	text_length = (nat) lseek(f, 0, SEEK_END);
	text = calloc(text_length + 1, 1);
	lseek(f, 0, SEEK_SET);
	read(f, text, text_length);
	close(f);
}


	printf("parsing this text: (%llu) \n", text_length);
	puts(text);
	puts("");

*/









			//printf("arg0 = 0x%016llx\n", arg0);
			//printf("arg1 = 0x%016llx\n", arg1);
			//fflush(stdout);



			/*} else if (state == incr or state == decr or state == not_) {
				if (variable == last) goto print_error;
				goto push_ins;*/


	/*

		//print_ct_values(names, name_count, ctk, values);
		//print_instruction_index(ins, ins_count, names, name_count, pc, "PC");


		printf("executing pc #%llu    :    ", pc);
		print_instruction(ins[pc], names, name_count); puts("");


	nat* stack = calloc(ins_count, sizeof(nat));
	nat stack_count = 0;
	stack[stack_count++] = 0; 

	nat* visited = calloc(ins_count + 1, sizeof(nat));






	while (stack_count) {
		print_stack(stack, stack_count);
		const nat pc = stack[--stack_count];
		print_ct_values(names, name_count, is_runtime, values);
		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");

		getchar();

		visited[pc] = 1;
		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];

		if (op == lt or op == eq) {

			if (ins[pc].gotos[0] == ins[pc].gotos[1]) {
				ins[pc].ct = (1 << 1) | 1;
				ins[pc].op = eq;
				ins[pc].args[0] = 0;
				ins[pc].args[1] = 0;
				ins[pc].gotos[0] = (nat) -1;
				if (ins[pc].gotos[1] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[1];
				continue;

			} else if (ins[pc].args[0] == ins[pc].args[1]) {

				const nat condition = op == eq;
				ins[pc].ct = (condition << 1) | 1;
				ins[pc].op = eq;
				ins[pc].args[0] = 0;
				ins[pc].args[1] = 0;
				ins[pc].gotos[1] = ins[pc].gotos[condition];
				ins[pc].gotos[0] = (nat) -1;
				if (ins[pc].gotos[1] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[1];
				continue;


			} else if (ctk[arg0] and ctk[arg1]) {
				nat condition = 0;
				if (op == eq and values[arg0] == values[arg1]) condition = 1;
				if (op == lt and values[arg0] <  values[arg1]) condition = 1;
				ins[pc].ct = (condition << 1) | 1;
				if (ins[pc].gotos[condition] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[condition];
				continue;
			}

			if (ctk[arg0]) {
				nat t = ins[pc].args[0]; 
				ins[pc].args[0] = ins[pc].args[1]; 
				ins[pc].args[1] = t;
				if (op == lt) ins[pc].op = gt_imm;
				else ins[pc].op = eq_imm;
				ins[pc].args[1] = values[ins[pc].args[1]];

			} else if (ctk[arg1]) {
				if (op == lt) ins[pc].op = lt_imm;
				else ins[pc].op = eq_imm;
				ins[pc].args[1] = values[ins[pc].args[1]];
			}
			if (ins[pc].gotos[0] < ins_count and 
				not visited[ins[pc].gotos[0]]) 
				stack[stack_count++] = ins[pc].gotos[0];
			if (ins[pc].gotos[1] < ins_count and 
				not visited[ins[pc].gotos[1]]) 
				stack[stack_count++] = ins[pc].gotos[1];

			continue;

		} else if (op == sc) {
			if (not ctk[arg0]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg0];
			const nat output_count = get_call_output_count(n);
			for (nat i = 0; i < output_count; i++) {
				if (ctk[ins[pc].args[1 + i]]) { puts("system call ct rt out"); abort(); }
			}
			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, "CFG termination point here"
				);
				getchar();
				ins[pc].gotos[0] = (nat) -1;
				ins[pc].gotos[1] = (nat) -1;
				goto skip_next;
			} else {
				printf("info: found %s sc!\n", systemcall_spelling[n]);
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, systemcall_spelling[n]
				);
			}

			ins[pc].args[0] = values[arg0];

		} else if (op == zero) { 
			if (ctk[arg0]) { 
				values[arg0] = 0; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = set_imm; 
				ins[pc].args[1] = 0; 
			}

		} else if (op == incr) { 
			if (ctk[arg0]) { 
				values[arg0]++; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = add_imm; 
				ins[pc].args[1] = 1; 
			}
		} else if (op == decr) { 
			if (ctk[arg0]) { 
				values[arg0]--; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = sub_imm; 
				ins[pc].args[1] = 1; 
			}
		} else if (op == not_) { 
			if (ctk[arg0]) { 
				values[arg0] = ~values[arg0]; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = eor_imm; 
				ins[pc].args[1] = (nat) -1; 
			}
		} else {
			if (ctk[arg0] and ctk[arg1]) {
				ins[pc].ct = 1; 
				     if (op == set) values[arg0] = values[arg1];
				else if (op == add) values[arg0] += values[arg1];
				else if (op == sub) values[arg0] -= values[arg1];
				else if (op == mul) values[arg0] *= values[arg1];
				else if (op == div_)values[arg0] /= values[arg1];
				else if (op == and_)values[arg0] &= values[arg1];
				else if (op == or_) values[arg0] |= values[arg1];
				else if (op == eor) values[arg0] ^= values[arg1];
				else if (op == si)  values[arg0] <<= values[arg1];
				else if (op == sd)  values[arg0] >>= values[arg1];
				else {
					puts("internal error: op execution not specified");
					printf("op = %llu, op = %s\n", op, ins_spelling[op]);
					abort();
				}
				goto next_ins;
			}

			//printf("arg0 = 0x%016llx\n", arg0);
			//printf("arg1 = 0x%016llx\n", arg1);
			//fflush(stdout);

			if (op >= isa_count or not ctk[arg0] and not ctk[arg1]) goto next_ins;
			if (    ctk[arg0] and not ctk[arg1]) {
				puts("error: ct destination must have ct source."); 
				abort();
			}
			ins[pc].args[1] = values[arg1];
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
				ins[pc].op ==  sd_imm and values[arg1] == 0) ins[pc].ct = 1;
		} 
		next_ins: if (ins[pc].gotos[0] < ins_count) 
			stack[stack_count++] = ins[pc].gotos[0]; skip_next:;
	}

	for (nat i = 0; i < ins_count; i++) if (not visited[i]) ins[i].ct = 4;

	print_ct_values(names, name_count, ctk, values);
	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);

	getchar();


	exit(1);



	*/



























/*{const nat ct = ins[i].ct;
		const bool unreachable = ct & ct_is_unreachable;
		const bool compiletime = ct & ct_is_compiletime;
		const bool generated_do = ct & ct_is_generated_do;
		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);

		if (unreachable or compiletime and (not is_branch or is_branch and not generated_do)) continue;}
		*/




		//const nat arg0 = ins[i].args[0];
		//const nat arg1 = ins[i].args[1];


/*		const nat ct = ins[i].ct;
		const bool unreachable = ct & ct_is_unreachable;
		const bool compiletime = ct & ct_is_compiletime;
		const bool generated_do = ct & ct_is_generated_do;
		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);

		if (unreachable or compiletime and (not is_branch or is_branch and not generated_do)) continue;}
*/










































/*








movz	setimm x k 
addsr	set d m siimm d k add d n 
lslv	set d m si d n
madd	set d m mul d n add d a

msub	set s m mul s n set d a sub d s
eori	set d n eorimm d k
eorsr	set d m siimm d k eor d n
eonsr	set d m siimm d k not d eor d n

addi	set d n addimm d k
andi	set d n andimm d k
orrsr	set d m siimm d k or d n
orri	set d n orimm d k

ornsr	set d m siimm d k not d or d n
subsr	set s m siimm s k set d n sub d s
subi	set d n subimm d k
udiv	set d n div d m







--------------------------- ARM64 INS SEL PATTERNS -----------------------------

movz	setimm x k 

lslv	set d m si d n
lsrv	set d m sd d n

udiv	set d n div d m

madd	set d m mul d n add d a
msub	set s m mul s n set d a sub d s



addsr	set d m siimm d k add d n 
subsr	set s m siimm s k set d n sub d s

andsr	set d m siimm d k and d n

orrsr	set d m siimm d k or d n
ornsr	set d m siimm d k not d or d n

eorsr	set d m siimm d k eor d n
eonsr	set d m siimm d k not d eor d n



addi	set d n addimm d k

subi	set d n subimm d k

andi	set d n andimm d k

orri	set d n orimm d k

eori	set d n eorimm d k



--------------------------- END OF ARM64 INS SEL PATTERNS -----------------------------









movz . setimm x k

addsrlsl . set d m siimm d k add d n 
addsrlsr . set d m sdimm d k add d n

eorsrlsl . set d m siimm d k eor d n 
eorsrlsr . set d m sdimm d k eor d n 
eonsrlsl . set d m siimm d k not d eor d n
eonsrlsr . set d m sdimm d k not d eor d n

orrsrlsl . set d m siimm d k or d n 
orrsrlsr . set d m sdimm d k or d n 
ornsrlsl . set d m siimm d k not d or d n
ornsrlsr . set d m sdimm d k not d or d n




lslv . set d m si d n

madd . set d m mul d n add d a 
msub . set s m mul s n set d a sub d s

addi . set d n addimm d k
andi . set d n andimm d k 
orri . set d n orimm d k
eori . set d n eorimm d k



*/



		//const nat ct = ins[pc].ct;

		//if (compiletime and generated_do) { printf("FOUND MACHINE DO @ %llu\n", pc); getchar(); break; }

		//const bool compiletime = ct & ct_is_compiletime;
		//const bool generated_do = ct & ct_is_generated_do;
		//if (not compiletime) 


























		//}
		//if (is_branch) pc = ins[pc].gotos[(ins[pc] >> 1) & 1];
		//else 




// things to layer on to the front end still: 
//   - including of multiple files
//   - comments

/*

things in the compiler to do:
	
	- ct-eval : recognize when a variable is compiletime known, automatically,
			by looking at the predeccessors, and keeping track of a variable/ctkness at each point in the program, and going back and looking at previous decisions, because of loops! push a new branch node to the branch stack, when we encounter a br, and check the decision that was made on all ctkness of all variables. store the values and ctk array on the branch stack. i think this is required. 


after that:


	- ins sel : add more patterns, for arm64
	- ins sel :    recognize control flow patterns:   implement  csel!!!!!!


	- RA : find the lifetime of variables   and the reads and writes to variables, in the mi listing!
	- code gen: generate the mi into machine code lol
		- work on the msp430 backend!

*/





/*

} else if (op == zero) { new.op = set_imm; new.args[1] = 0;
} else if (op == incr) { new.op = add_imm; new.args[1] = 1;
} else if (op == decr) { new.op = sub_imm; new.args[1] = 1;
} else if (op == not_) { new.op = eor_imm; new.args[1] = (nat) -1; }

*/



/*	
				compiletime / runtime identification pass:

-------------apca-------------apca-------------apca-------------apca-------------apca-------------apca-------------apca


	nat previous_pc = (nat) -1, stack_count = 1;
	nat* stack = calloc(ins_count, sizeof(nat));
	nat* visited = calloc(ins_count + 1, sizeof(nat));
	nat* execution_state_values = calloc(ins_count * name_count, sizeof(nat));
	nat* execution_state_ctk = calloc(ins_count * name_count, sizeof(nat));
	memset(execution_state_values, 0xA5, sizeof(nat) * ins_count * name_count); // debug

	while (stack_count) {

		print_stack(stack, stack_count);
		const nat pc = stack[--stack_count];
		nat* values = execution_state_values + name_count * pc;
		nat* ctk = execution_state_ctk + name_count * pc;
		if (not pc) goto process;

		nat pred_count = 0;
		nat* preds = compute_predecessors(ins, ins_count, pc, &pred_count);
		for (nat n = 0; n < name_count; n++) {
			for (nat i = 0; i < pred_count; i++) {
				if (not visited[preds[i]]) continue;
				if (not execution_state_ctk[name_count * preds[i] + n]) goto one_rtk;
			}
			nat spot = 0;
			for (; spot < pred_count; spot++) {
				if (not visited[preds[spot]]) continue;
				if (preds[spot] == previous_pc) goto pred_found;
			}
			puts("fatal error: could not find previous_pc in predecessor list.");
			printf("previous_pc = %llu  :  { ", previous_pc);
			for (nat i = 0; i < pred_count; i++) {
				printf("%llu ", preds[i]);
			} 
			puts(" }");
			abort();
		pred_found:;
			printf("selecting pred[%llu] == previous_pc, which was %llu\n", spot, previous_pc);
			if (not execution_state_ctk[name_count * previous_pc + n]) abort();
			ctk[n] = 1;
			values[n] = execution_state_values[name_count * previous_pc + n];			
			goto next_name;
		one_rtk:;
			ctk[n] = 0;
			values[n] = 0x999999999;
			next_name:;
		}
		process:;
		visited[pc]++;

		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];

		if (op == lt or op == eq) {
			if (not ctk[arg0] or not ctk[arg1]) {

				if (ins[pc].gotos[0] < ins_count and 
					visited[ins[pc].gotos[0]] < 2) 
					stack[stack_count++] = ins[pc].gotos[0];
				if (ins[pc].gotos[1] < ins_count and 
					visited[ins[pc].gotos[1]] < 2) 
					stack[stack_count++] = ins[pc].gotos[1];
			} else {
				nat condition = 0;
				if (op == eq and values[arg0] == values[arg1]) condition = 1;
				if (op == lt and values[arg0] <  values[arg1]) condition = 1;
				if (ins[pc].gotos[condition] < ins_count) stack[stack_count++] = ins[pc].gotos[condition];
			}
			goto next_instruction;
		}
		
		if (op == 0) abort();
		else if (op == zero) { ctk[arg0] = 1; values[arg0] = 0; }
		else if (op == incr) { if (ctk[arg0]) values[arg0]++; } 
		else if (op == add) { 
			if (ctk[arg0] and ctk[arg1]) values[arg0] += values[arg1]; 
			else if (not ctk[arg0] and not ctk[arg1]) {}
			else if (not ctk[arg0]) {  set add_imm op code }
			else {
				puts("i think this is the point where we make the dest CT now, instead of RT.");
				abort();
			}
		} 
		else if (op == sc) {

			if (not ctk[arg0]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg0];
			const nat output_count = get_call_output_count(n);
			for (nat i = 0; i < output_count; i++) {
				const nat this = ins[pc].args[1 + i];
				ctk[this] = 0;
				values[this] = 0x999999999999333;
			}
			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instructions_ct_values_index(
					ins, ins_count, 
					names, name_count, locations, 
					execution_state_ctk, execution_state_values,
					pc, "CFG termination point here"
				);
				getchar();
				ins[pc].gotos[0] = (nat) -1;
				ins[pc].gotos[1] = (nat) -1;
				goto next_instruction;

			} else if (n == system_write) {			
				printf("warning: system_write syscall encountered\n");
				print_instructions_ct_values_index(
					ins, ins_count, 
					names, name_count, locations, 
					execution_state_ctk, execution_state_values,
					pc, "system_write"
				);
				getchar();

			} else {
				printf("info: found %s sc!\n", systemcall_spelling[n]);
				printf("ERROR: unknown syscall encountered\n");
				print_instructions_ct_values_index(
					ins, ins_count, 
					names, name_count, locations, 
					execution_state_ctk, execution_state_values,
					pc, "??????"
				);
				getchar();
			}
		} else {
			puts("FATAL_ERROR: unknown operation encountered, aborting.."); 
			abort();
		}
		
		if (ins[pc].gotos[0] < ins_count) 
			stack[stack_count++] = ins[pc].gotos[0];

	next_instruction:;
		previous_pc = pc;
		print_instructions_ct_values_index(
			ins, ins_count, 
			names, name_count, locations, 
			execution_state_ctk, execution_state_values,
			pc, "PC"
		);
		getchar();
	}


	print_instructions_ct_values_index(
		ins, ins_count, 
		names, name_count, locations, 
		execution_state_ctk, execution_state_values, 
		(nat) -1, ""
	);


-------------apca-------------apca-------------apca-------------apca-------------apca-------------apca-------------apca

	*/


































		//print_ct_values(names, name_count, ctk, values);
		//print_instruction_index(ins, ins_count, names, name_count, pc, "PC");


	/*
		printf("executing pc #%llu    :    ", pc);
		print_instruction(ins[pc], names, name_count); puts("");






	nat* stack = calloc(ins_count, sizeof(nat));
	nat stack_count = 0;
	stack[stack_count++] = 0; 

	nat* visited = calloc(ins_count + 1, sizeof(nat));




	while (stack_count) {
		print_stack(stack, stack_count);
		const nat pc = stack[--stack_count];
		print_ct_values(names, name_count, ctk, values);
		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");

		getchar();

		visited[pc] = 1;
		const nat op = ins[pc].op;
		const nat arg0 = ins[pc].args[0];
		const nat arg1 = ins[pc].args[1];

		if (op == lt or op == eq) {

			if (ins[pc].gotos[0] == ins[pc].gotos[1]) {
				ins[pc].ct = (1 << 1) | 1;
				ins[pc].op = eq;
				ins[pc].args[0] = 0;
				ins[pc].args[1] = 0;
				ins[pc].gotos[0] = (nat) -1;
				if (ins[pc].gotos[1] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[1];
				continue;

			} else if (ins[pc].args[0] == ins[pc].args[1]) {

				const nat condition = op == eq;
				ins[pc].ct = (condition << 1) | 1;
				ins[pc].op = eq;
				ins[pc].args[0] = 0;
				ins[pc].args[1] = 0;
				ins[pc].gotos[1] = ins[pc].gotos[condition];
				ins[pc].gotos[0] = (nat) -1;
				if (ins[pc].gotos[1] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[1];
				continue;


			} else if (ctk[arg0] and ctk[arg1]) {
				nat condition = 0;
				if (op == eq and values[arg0] == values[arg1]) condition = 1;
				if (op == lt and values[arg0] <  values[arg1]) condition = 1;
				ins[pc].ct = (condition << 1) | 1;
				if (ins[pc].gotos[condition] < ins_count) 
					stack[stack_count++] = ins[pc].gotos[condition];
				continue;
			}

			if (ctk[arg0]) {
				nat t = ins[pc].args[0]; 
				ins[pc].args[0] = ins[pc].args[1]; 
				ins[pc].args[1] = t;
				if (op == lt) ins[pc].op = gt_imm;
				else ins[pc].op = eq_imm;
				ins[pc].args[1] = values[ins[pc].args[1]];

			} else if (ctk[arg1]) {
				if (op == lt) ins[pc].op = lt_imm;
				else ins[pc].op = eq_imm;
				ins[pc].args[1] = values[ins[pc].args[1]];
			}
			if (ins[pc].gotos[0] < ins_count and 
				not visited[ins[pc].gotos[0]]) 
				stack[stack_count++] = ins[pc].gotos[0];
			if (ins[pc].gotos[1] < ins_count and 
				not visited[ins[pc].gotos[1]]) 
				stack[stack_count++] = ins[pc].gotos[1];

			continue;

		} else if (op == sc) {
			if (not ctk[arg0]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg0];
			const nat output_count = get_call_output_count(n);
			for (nat i = 0; i < output_count; i++) {
				if (ctk[ins[pc].args[1 + i]]) { puts("system call ct rt out"); abort(); }
			}
			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, "CFG termination point here"
				);
				getchar();
				ins[pc].gotos[0] = (nat) -1;
				ins[pc].gotos[1] = (nat) -1;
				goto skip_next;
			} else {
				printf("info: found %s sc!\n", systemcall_spelling[n]);
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					pc, systemcall_spelling[n]
				);
			}

			ins[pc].args[0] = values[arg0];

		} else if (op == zero) { 
			if (ctk[arg0]) { 
				values[arg0] = 0; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = set_imm; 
				ins[pc].args[1] = 0; 
			}

		} else if (op == incr) { 
			if (ctk[arg0]) { 
				values[arg0]++; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = add_imm; 
				ins[pc].args[1] = 1; 
			}
		} else if (op == decr) { 
			if (ctk[arg0]) { 
				values[arg0]--; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = sub_imm; 
				ins[pc].args[1] = 1; 
			}
		} else if (op == not_) { 
			if (ctk[arg0]) { 
				values[arg0] = ~values[arg0]; 
				ins[pc].ct = 1; 
			} else { 
				ins[pc].op = eor_imm; 
				ins[pc].args[1] = (nat) -1; 
			}
		} else {
			if (ctk[arg0] and ctk[arg1]) {
				ins[pc].ct = 1; 
				     if (op == set) values[arg0] = values[arg1];
				else if (op == add) values[arg0] += values[arg1];
				else if (op == sub) values[arg0] -= values[arg1];
				else if (op == mul) values[arg0] *= values[arg1];
				else if (op == div_)values[arg0] /= values[arg1];
				else if (op == and_)values[arg0] &= values[arg1];
				else if (op == or_) values[arg0] |= values[arg1];
				else if (op == eor) values[arg0] ^= values[arg1];
				else if (op == si)  values[arg0] <<= values[arg1];
				else if (op == sd)  values[arg0] >>= values[arg1];
				else {
					puts("internal error: op execution not specified");
					printf("op = %llu, op = %s\n", op, ins_spelling[op]);
					abort();
				}
				goto next_ins;
			}

			//printf("arg0 = 0x%016llx\n", arg0);
			//printf("arg1 = 0x%016llx\n", arg1);
			//fflush(stdout);

			if (op >= isa_count or not ctk[arg0] and not ctk[arg1]) goto next_ins;
			if (    ctk[arg0] and not ctk[arg1]) {
				puts("error: ct destination must have ct source."); 
				abort();
			}
			ins[pc].args[1] = values[arg1];
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
				ins[pc].op ==  sd_imm and values[arg1] == 0) ins[pc].ct = 1;
		} 
		next_ins: if (ins[pc].gotos[0] < ins_count) 
			stack[stack_count++] = ins[pc].gotos[0]; skip_next:;
	}

	for (nat i = 0; i < ins_count; i++) if (not visited[i]) ins[i].ct = 4;

	print_ct_values(names, name_count, ctk, values);
	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);

	getchar();


	exit(1);



	*/




































/*
	nat pred_count[4096] = {0};

	for (nat n = 0; n < name_count; n++) {
		if (locations[n] != (nat) -1) {			
			printf("FOUND LABEL %s: WITH LOCATION: %llu\n", 
				names[n], locations[n]
			);
			const nat this = locations[n];
			nat found_count = 0;
			for (nat i = 0; i < ins_count; i++) {
				if (ins[i].gotos[0] == this and not (ins[i].ct & ct_is_unreachable)) { 
					found_count++;
				//print_instruction_index(
				//ins, ins_count, 
				//names, name_count, 
				//i, "occurence"
			//); getchar(); 
				}
				if (ins[i].gotos[1] == this and not (ins[i].ct & ct_is_unreachable)) { 
					found_count++;
				//print_instruction_index(
				//ins, ins_count, 
				//names, name_count, 
				//i, "occurence"
			//); getchar(); 
				}
			}
			printf(" ---> this label had %llu goto occurences "
				"of instructions which went to this location.\n",
				found_count
			);
			pred_count[n] = found_count;
		}
	}

	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op != eq or ins[i].args[0] != ins[i].args[1]) continue;
		puts("FOUND A DO INSTRUCTION!!!!");
		if (pred_count[ins[i].args[2]] >= 2) {
			puts("warning: this do statement will be generated in actual machine code");
			print_instruction_index(
				ins, ins_count, 
				names, name_count, 
				i, "GENERATED IN MACHINE CODE."
			);
			ins[i].ct |= 8;
		} else {
			printf("warning: this do statement will be optimized away, "
				"as it only has %llu pred.\n", pred_count[ins[i].args[2]]
			);
			print_instruction_index(
				ins, ins_count, 
				names, name_count, 
				i, "IGNORED, OPTIMIZED AWAY."
			);
		}
		getchar();
	}

	print_ct_values(names, name_count, ctk, values);
	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);

	puts("starting ins sel..");
	struct instruction mi[4096] = {0};
	nat mi_count = 0;
	nat selected[4096] = {0};


	for (nat i = 0; i < ins_count; i++) {

		if (selected[i]) {
			printf("warning: [ins_index = %llu]: skipping instruction, it was already part of a pattern.\n", i);
			continue;
		}


		const nat op = ins[i].op;
		const nat arg0 = ins[i].args[0];
		const nat arg1 = ins[i].args[1];

		{const nat ct = ins[i].ct;
		const bool unreachable = ct & ct_is_unreachable;
		const bool compiletime = ct & ct_is_compiletime;
		const bool generated_do = ct & ct_is_generated_do;
		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);

		if (unreachable or compiletime and (not is_branch or is_branch and not generated_do)) continue;}

		print_instruction_index(ins, ins_count, names, name_count, i, "SELECTION ORIGIN");
		printf("selecting from i #%llu\n", i);
		print_instruction(ins[i], names, name_count); puts("");
		getchar();


		if (op == set) {
			const nat b = locate_instruction(si_imm, arg0, 0, 1, 0, i + 1, ins, ins_count, names, name_count);
			printf("b = %lld\n", b);
			if (b == (nat) -1) goto next0;
		
			const nat c = locate_instruction(add, arg0, 0, 1, 0, b + 1, ins, ins_count, names, name_count);
			printf("c = %lld\n", c);
			if (c == (nat) -1) goto next0;
			
			const nat d = arg0;
			const nat n = ins[c].args[1];
			const nat m = ins[i].args[1];
			const nat k = ins[b].args[1];
			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("ADD_SR   "
				"d=%llu(%s), "
				"n=%llu(%s), "
				"m=%llu(%s) << "
				"k=%llu\n",
				d, names[d], 
				n, names[n], 
				m, names[m], 
				k
			);
			struct instruction new = {0};
			new.op = addsr_lsl;
			new.args[0] = d;
			new.args[1] = n;
			new.args[2] = m;
			new.args[3] = k;
			mi[mi_count++] = new;

			selected[i] = 1;
			selected[b] = 1;
			selected[c] = 1;
		}
		next0:;


		if (op == set_imm) {

			const nat d = arg0;
			const nat k = arg1;
			printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
			printf("MOVZ   "
				"d=%llu(%s), "
				"k=%llu\n",
				d, names[d], 
				k
			);
			struct instruction new = {0};
			new.op = movz;
			new.args[0] = d;
			new.args[1] = k;
			mi[mi_count++] = new;

			selected[i] = 1;
		}

	
	}


	for (nat i = 0; i < ins_count; i++) {

		{const nat op = ins[i].op;
		//const nat arg0 = ins[i].args[0];
		//const nat arg1 = ins[i].args[1];
		const nat ct = ins[i].ct;
		const bool unreachable = ct & ct_is_unreachable;
		const bool compiletime = ct & ct_is_compiletime;
		const bool generated_do = ct & ct_is_generated_do;
		const bool is_branch = (op == lt or op == eq or op == gt_imm or op == lt_imm or op == eq_imm);

		if (unreachable or compiletime and (not is_branch or is_branch and not generated_do)) continue;}

		if (not selected[i]) {
			puts("error: instruction was not able to be processed by instruction selection: internal error");
			puts("not selected instruction: ");
			print_instruction_index(
				ins, ins_count, 
				names, name_count, 
				i, "this instruction failed to be lowered during instruction selection."
			); abort();
		}
	}

	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);
	print_machine_instructions(mi, mi_count);
	puts("stopped after ins sel.");
	//puts(text);
	exit(0);



	*/












































					//else if (unreachable) {
						/*printf("%s:%llu:%llu: warning: unreachable instruction\n", 
							filename, word_start, index
						);
						print_index(text, text_length, word_start, index);*/
					//}




/*
		const nat expecting_op = si;
		const nat expecting_arg0 = 

		const nat r = locate_data_instruction(
			expecting_op, expecting_arg0, expecting_arg1, 
			i, ins, ins_count, names, name_count
		);
		printf("r = %llu\n", r);


*/




/*	const nat start_from = 0;
	const nat expecting_op = si;
	const nat expecting_arg0 = ins[0].args[0];
	const nat expecting_arg1 = 0;
	
	const nat r = locate_data_instruction(
		expecting_op, expecting_arg0, expecting_arg1, 1, 0,
		start_from + 1, ins, ins_count, names, name_count
	);
	printf("r = %lld\n", r);

	exit(1);
*/
















/*


			} else if (state == ct) {
				state = 0;
				ctk[variable] = 1;

			} else if (state == rt) {
				if (arg_count < 2) goto next_word;
				state = 0;
				ctk[args[0]] = 0;
				bit_count[args[0]] = variable;


*/














/*		if (compiletime and not is_branch) {
			pc = ins[i].gotos[0]; continue;

		} else if (compiletime and is_branch and not generated_do) {
			pc = ins[i].gotos[(ins[i].ct >> 1) & 1]; continue;
		}





		if (unreachable) {
			puts("hit an unrechable instruction!?!");
			getchar();
		}
	







	for (nat i = 0; i < ins_count; i++) {
		if ((ins[i].ct & ct_is_unreachable) or (ins[i].ct & ct_is_compiletime)) continue;
		for (nat a = 0; a < (ins[i].op == sc ? 7 : 2); a++) {
			if (ins[i].op >= isa_count and a == 1) continue;
			const nat n = ins[i].args[a];
			if (locations[n] == (nat) -1 and (ins[definition[n]].ct & ct_is_unreachable)) {
				printf("warning: in argument %llu, variable \"%s\" used with unreachable initialization\n", a, names[n]);
				print_instruction_index(
					ins, ins_count, 
					names, name_count, 
					i, "initialization will never be executed"
				);
				getchar();			
			}
		}
	}

		^ this pass is not quite sound, because of ct-br conditional initialization. 
*/





















/*


	struct instruction mi[4096] = {0};
	nat mi_count = 0;
	memset(visited, 0, sizeof(nat) * (ins_count + 1));
	stack[stack_count++] = 0;

	while (stack_count) {

		print_stack(stack, stack_count);
		nat pc = stack[--stack_count];
		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");
		getchar();

		visited[pc] = 1;

		if (ins[pc].ct) goto done_with_instruction;

	//  set d m  si_imm d k  add d n
		{const nat op0 = ins[pc].op;
		const nat dest0 = ins[pc].args[0];
		const nat source0 = ins[pc].args[1];
		const nat gt0 = ins[pc].gotos[0];
		if (op0 != set) goto next0;

		const nat op1 = ins[gt0].op;
		const nat dest1 = ins[gt0].args[0];
		const nat source1 = ins[gt0].args[1];
		const nat gt1 = ins[gt0].gotos[0];
		if (op1 != si_imm or dest1 != dest0) goto next0;

		const nat op2 = ins[gt1].op;
		const nat dest2 = ins[gt1].args[0];
		const nat source2 = ins[gt1].args[1];
		if (op2 != add or dest2 != dest0) goto next0;


		// todo: 
		//ERROR ERROR 
		puts("we were in the middle of doing ins sel...");
		abort();


		const nat d = dest0;
		const nat n = source2;
		const nat m = source0;
		const nat k = source1;					
		printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
		printf("ADD_SR   "
			"d=%llu(%s), "
			"n=%llu(%s), "
			"m=%llu(%s) << "
			"k=%llu\n",
			d, names[d], 
			n, names[n], 
			m, names[m], 
			k
		);
		struct instruction new = {0};
		new.op = addsr;
		new.args[0] = d;
		new.args[1] = n;
		new.args[2] = m;
		new.args[3] = k;
		mi[mi_count++] = new;
		pc = gt1; } next0:;





	// set d m  add d n
		{const nat op0 = ins[pc].op;
		const nat dest0 = ins[pc].args[0];
		const nat source0 = ins[pc].args[1];
		const nat gt0 = ins[pc].gotos[0];
		if (op0 != set) goto next1;

		const nat op1 = ins[gt0].op;
		const nat dest1 = ins[gt0].args[0];
		const nat source1 = ins[gt0].args[1];
		if (op1 != add or dest1 != dest0) goto next1;

		const nat d = dest0;
		const nat n = source1;
		const nat m = source0;
		const nat k = 0;
		printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
		printf("ADD_SR   "
			"d=%llu(%s), "
			"n=%llu(%s), "
			"m=%llu(%s) << "
			"k=%llu\n",
			d, names[d], 
			n, names[n], 
			m, names[m], 
			k
		);
		struct instruction new = {0};
		new.op = addsr;
		new.args[0] = d;
		new.args[1] = n;
		new.args[2] = m;
		new.args[3] = k;
		mi[mi_count++] = new;
		pc = gt0; } next1:;

	// set_imm d k
		{const nat op0 = ins[pc].op;
		const nat dest0 = ins[pc].args[0];
		const nat source0 = ins[pc].args[1];
		if (op0 != set_imm) goto next2;

		const nat d = dest0;
		const nat k = source0;
		printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
		printf("MOVZ   "
			"d=%llu(%s), "
			"k=%llu\n",
			d, names[d], 
			k
		);
		struct instruction new = {0};
		new.op = movz;
		new.args[0] = d;
		new.args[1] = k;
		mi[mi_count++] = new; } next2:;

		done_with_instruction:;
		const nat op = ins[pc].op;
		const nat gt0 = ins[pc].gotos[0];
		const nat gt1 = ins[pc].gotos[1];
		if (op == lt or op == eq or op == lt_imm or op == gt_imm or op == eq_imm) 
		if (gt1 < ins_count and not visited[gt1]) stack[stack_count++] = gt1;
		if (gt0 < ins_count and not visited[gt0]) stack[stack_count++] = gt0;
	}

	print_dictionary(names, active, definition, ctk, values, locations, bit_count, name_count);
	print_instructions(ins, ins_count, names, name_count);
	print_machine_instructions(mi, mi_count);
	puts("stopped after ins sel.");

	exit(1);




	nat* list = calloc(8 * ins_count, sizeof(nat));
	nat list_count = 0;

	// .. loop over the mi instruction list, generating reads and writes for each instruction. 

	printf("found list: (%llu count)\n", list_count);
	for (nat i = 0; i < list_count; i++) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);
	}
	puts("done");

	bool* alive = calloc(name_count, sizeof(bool));

	for (nat i = list_count; i--;) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);

		const bool is_write = !!(list[i] >> 63);

		if (is_write) {			
			alive[variable_index] = 0;
		} else {
			alive[variable_index] = 1;			
		}
		
		printf("alive = { ");
		for (nat n = 0; n < name_count; n++) {
			if (alive[n]) printf("%s  ", names[n]);
		} 
		printf(" }\n");
		//getchar();
	}

	puts("compiled.");

	puts(text);
	exit(0);
}







*/





















	// serialize the cfg into instructions which use ip++?.. so that we can discover which goto's must reside. because then, obviously, if thats the case, we need to not allow instruction selection around that instruction. we need to treat it like an end, basically, if there are instructions after it. we are still going to represent this in the cfg as just .goto of some non ip++ thing, but we will actually note down the fact that this non-ip++ goto cannot be gotten rid of, because there are other people which go to the same place. so yeah. i think that should work. lets try to implement this lol. 


















/*
		// some additional checks: 
		// we need to make sure that the only instruction which goes to any of these later instructions after the first one,   are   only part of the pattern too. 
		// ie, we shouldnt let any   "at" statements happen in the middle here lol. 
		//


		alsooo i think theres  like a completely different way that we need to be doing our instruction selection.. 

			we need to take into account the actual data flow-   and just require control flow to be correct as well,  (ie, a straight line of executionnnn..)


			ie, we need some function to like, find the next instruction which has the right control flow, and alsoooo doesnt have data hazards in the way. thats the key. we'll follow the control flow, if see a branch, we abort (as thats lets say always a hazard!)

				and then if we find a hazard instruction (colliding data usage), then we know that we can't do this pattern. so yeah. we need to be doing that. 



			alsoooo... theres a bigger problem...

							i think uhh



					i think we need to schedule   the ir instructions, 


								(ie, form a linearizing sequence for thsi control flow graph. ie, doing like.. basic block ordering...)



							beforeeeee doing instruction selection. 





					because like, some of these... uhhh   "implicit do"    statements will actually have to end up being   actuallll machine instructions. because of course, the cpu that we target itself has those instructions, because it uses implicit ip++'s. so yeah. we need to be representing that, and literallyyyyy scheduling these instructions, in some ordering. 


						like, its just required. we just need that. 



					so yeah. i think i am going to look into how to like.. order basic blocks the best lol. 


				basically, the goal is to have blocks bleed into each other as much as posssible. thats the goal. 

				but like sometimes, you can't do that though. 


			hm
				so yeah. idk. we'll see how things go lol. 



*/






















	// here is where optimization is done!!!!

	// including: simplifying the ct execution, as well as the rt execution. both. 
	// eliminating memory variables, extraneous register variables, etc. 
	// simplifying control flow. you know that kind of stuff.








































































































































	/*

		addsr:  d  = n + (m << k);

			set d m
			si_imm d #k
			add d n

	//nat state = 0, dest = 0, source0 = 0, source1 = 0, immediate = 0;


	*/








/*





		if (state == 0) {
		retry:
			state = 0; 

			dest = 0; source0 = 0; source1 = 0; immediate = 0;

			if (op == set) { state = 1; dest = arg0; source1 = arg1; }
			else {}

		} else if (state == 1) {
			     if (op == si_imm and arg0 == dest) { state = 2; immediate = arg1; }
			else if (op == add and arg0 == dest) { immediate = 0; goto generate_addsr; }
			else goto retry;

		} else if (state == 2) {

			if (op == add and arg0 == dest) { 
			generate_addsr:
				source0 = arg1;
	
				printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
				printf("ADD_SR   dest=%llu(%s), source1=%llu(%s), source2=%llu(%s) << immediate=%llu\n",
					dest, names[dest], 
					source0, names[source0], 
					source1, names[source1], 
					immediate
				);
				struct instruction new = {0};
				new.op = addsr;
				new.args[0] = source0;
				new.args[1] = source1;
				new.args[2] = immediate;
				mi[mi_count++] = new;

				state = 0; 
			}
			else goto retry;

		} else if (state == 3) {

			goto retry;
		}



*/









/*


1202501061.175200
TODO:
		CURRENT STATE OF INS SEL:


	--->   redo where we are doing the pattern recognition to be in a seperate pass.

		instead of doing it in ct-eval stage, 



		1. generate a new list of ONLYYY RT instructions, 

						 (some of which the opcodes will 
						change to be elements in the  
							 "enum immediate_forms_instructions"!!!)
			{
	
				note, simply move pass (i++)   rt instructions, one generated. don't follow their execution.
					you only do this for compiletime branches. treat rt branches like  single nop instructions. 
			}
		FINE NOTE:

			if you encounter, a "do",  follow it, as its CT known. however, 
			just don't generate any RT instruction you've already visited before.
			this way, we will ignore unreachable rt code, 
			as well as avoid overtraversing the rt cfg.   NICEEEE YAYYYY




		2. then loop over this list, and doing pattern recognition on it. here, i don't think we should take into account the control flow of the RT instructions at all, so far.    
			this should generate a list of machine instructions,     the "mi" array above.  this ds uses the existing    "struct instruction"


		3. print out the generated machine instructions, and rt ins listing.

			then, you should start the process of looking at the reads and writes over those machine instructions. 

			this is when we start the process of register allocation. only here.  once the mi's have been generated in this mi[] list.
			
		
			3.1. we generate the list of reads and writes   based on the instruction semantics, 
					(note: we are doing it based on the mi instructions, in case a variable gets reduced away during ins sel.  very important. 


			3.2. then we go backwards through the reads and writes, generating the live-in lists, 
				keeping track of which variables are found in the same list, 
				and thus constructing the RIG from this information


			3.3.   we then use the RIG to do actual graph coloring based register allocation    ON THE     MI's. 


		4. generate machine code. we have everything we need now lol. op codes, and register numbers.    
			this step should be easy, as its already written lol.
			 
		5. done!!!! yayyyy


// TODO: recognize these three patterns:

ins sel   for      csinc     (conditional select increment)

RT COMPARISON:

ne X Y false
set d n
do done
at false
set d m incr d
at done


USING CONSTANTS IN COMPARISON:

ne X 3 false
set d n
do done
at false
set d m incr d
at done


NEGATING CONDITON:

eq X 3 false
set d m incr d
do done
at false
set d n
at done


	struct instruction mi[4096] = {0};  // arg[0] is in "enum arm64_ins_set". args are in order of assembly format.
	nat mi_count = 0;






ins sel patterns:	
	
	addsr {                 d = n + (m << k)

		set d m
		si d k
		add d n
		
		where k is ct, d, n and m are rt.
			k <= 63		
	}

	addsr (k = 0) {

		set d m
		add d n
		
		where d, n and m are rt. (k = 0)
	}










bad code:



		if (	top >= head and 
			top + 2 < ins_count and 
			ins[top + 0].args[0] == set and 
			ins[top + 1].args[0] == si and
			ins[top + 2].args[0] == add and
			
			ins[top + 0].args[1] == ins[top + 1].args[1] and
			ins[top + 1].args[1] == ins[top + 2].args[1] and
			ctk[ins[top + 1].args[2]]
		) {
			mi[mi_count++] = top;
			mi[mi_count++] = addsr;
			head += 3;
		}


		if (	top + 1 < ins_count and 
			ins[top + 0].args[0] == set and 
			ins[top + 1].args[0] == add and
			ins[top + 0].args[1] == ins[top + 1].args[1]
		) {
			mi[mi_count++] = top;
			mi[mi_count++] = addsr_k0;
			head += 2;
		}









use this code later:



		if (visited[top] == 1) {

			puts("found this instruction for the first time!!! : ");
			debug_instruction(ins[top], names);
			puts("");
			

			if (state == 0) {
			retry:
				state = 0;
				dest = 0; source1 = 0; source2 = 0; immediate = 0;

				// set d m
				if (op == set and not ctk[arg1] and not ctk[arg2]) { state = 1; dest = arg1; source2 = arg2; } 

				else {}

			} else if (state == 1) {

				// si d k
				      if (op == si and arg1 == dest and not ctk[arg1] and ctk[arg2]) { state = 2; immediate = values[arg2]; }
				else if (op == add and arg1 == dest and not ctk[arg1] and not ctk[arg2]) { immediate = 0; goto generate_addsr; }
				else goto retry;

			} else if (state == 2) {

				// add d n
				if (op == add and arg1 == dest and not ctk[arg1] and not ctk[arg2]) { 
				generate_addsr:
					source1 = arg2;

					printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
					printf("ADD_SR   dest=%llu(%s), source1=%llu(%s), source2=%llu(%s) << immediate=%llu\n",
							dest, names[dest], 
							source1, names[source1], 
							source2, names[source2], 
							immediate
					);				
					state = 0; 
				} 

				else goto retry;

			} else if (state == 3) {

				goto retry;
			}
			
		}
















			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;

	//nat top = 0;
	//struct instruction rt_ins[4096] = {0};
	//nat rt_count = 0;
	//struct instruction mi[4096] = {0}; 
	//nat mi_count = 0;

        // state =  0, dest = 0, source1 = 0, source2 = 0, immediate = 0;
	
	// top < ins_count

			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;


			//if (not ctk[arg2]) list[list_count++] = arg2;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;


			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg2]) list[list_count++] = arg2;

			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;


			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg2]) list[list_count++] = arg2;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;

			//rt_ins[rt_count++] = ins[top];
		//top++;		

	 

















*/




































/*


		if (is_branch(op)) {
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if ( true_side < ins_count and visited[true_side] < 1)  stack[stack_count++] = true_side;
			if (false_side < ins_count and visited[false_side] < 1) stack[stack_count++] = false_side;
		} else {
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if ( true_side < ins_count) stack[stack_count++] = true_side;
			if (false_side < ins_count) stack[stack_count++] = false_side;
		} 





	printf("found modified  ins instruction sequence {\n");
	for (nat i = 0; i < ins_count; i++) {
		const char* op_name = rt_ins[i].args[0] < isa_count ? 
			ins_spelling[rt_ins[i].args[0]] : 
			ins_imm_spelling[rt_ins[i].args[0] - isa_count];		
		printf("\trt[%llu] = { %llu(%s) %llu(%s) %llu %llu %llu %llu %llu %llu } \n",
			i,  
			rt_ins[i].args[0], op_name,
			rt_ins[i].args[1], names[rt_ins[i].args[1]],
			rt_ins[i].args[2],
			rt_ins[i].args[3],
			rt_ins[i].args[4],
			rt_ins[i].args[5],
			rt_ins[i].args[6],
			rt_ins[i].args[7]			
		);
	}





	const nat is_imm = this.args[0] >= isa_count;
	if (is_imm) printf(" %s ", ins_imm_spelling[this.args[0] - isa_count]);
	else printf(" %s ", ins_spelling[this.args[0]]);
	for (nat a = 1; a < this.count; a++) {
		if (a == 2 and is_imm) printf(" IMM=%llu ", this.args[a]);
		else printf(" %s ", names[this.args[a]]);
	}



	
			if (ctk[arg1] and ctk[arg2]) {
				if (condition) {
					ins[top].count = 2;
					ins[top].args[0] = do_;
					ins[top].args[1] = ins[top].args[3];
				} else {
					ins[top].count = 0;
				}
			}

*/

















































































/*


struct stack_entry {
	nat side;
	nat index;
};



	uint8_t* visited = calloc(ins_count + 1, 1);
	nat stack_count = 0;
	//stack[stack_count++] = 0; 
	struct instruction rt_ins[4096] = {0};
	nat rt_count = 0;


	struct stack_entry* stack = calloc(2 * ins_count, sizeof(struct stack_entry));
	nat pc = 0;
	while (stack_count or pc < ins_count) { 

		if (pc >= ins_count) {
			if (stack[stack_count - 1].side == 0 and not visited[ins[pc].gotos[1]]) {
				puts("PC AT END: trying other side of the TOS branch!");
				getchar();
				stack[stack_count - 1].side = 1;
				pc = ins[stack[stack_count - 1].index].gotos[1];

			} else {
				puts("PC AT END: popping off top element off stack!");
				getchar();
				stack_count--;
				if (not stack_count) {
					puts("PC AT END: unknown code state????");
					puts("breakinggggggg");
					break;
				} else {
					puts("PC AT END: setting pc to be a new value...");
					getchar();
					pc = ins[stack[stack_count].index].gotos[0];
				}
			}			
			
		}

		printf("stack: %llu { \n", stack_count);
		for (nat i = 0; i < stack_count; i++) {
			printf("stack[%llu] = { %llu %llu }\n", 
				i, stack[i].side, stack[i].index
			);
		}
		puts("}");

		printf("ct values: {\n");
		for (nat i = 0; i < name_count; i++) {
			if (not ctk[i]) continue;
			printf("\tvalues[%s] = %llu\n", names[i], values[i]);
		}
		puts("}");

		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");
		print_instructions(rt_ins, rt_count, names, name_count);
		getchar();

		visited[pc] = 1;

		struct instruction new = ins[pc];
		const nat 
			op = new.op,
			arg0 = new.args[0], 
			arg1 = new.args[1];
			//gt0 = new.gotos[0],
			//gt1 = new.gotos[1];

		if (op == lt or op == eq) {

			if (ctk[arg0] and ctk[arg1]) {
				bool condition = 0;
				if (op == eq and values[arg0] == values[arg1]) condition = 1;
				if (op == lt and values[arg0] <  values[arg1]) condition = 1;
				if (not condition) {
					pc = ins[pc].gotos[0];
				} else {
					pc = ins[pc].gotos[1];
				}

			} else if (not stack_count or stack[stack_count - 1].index != pc) {

				stack[stack_count++] =
				(struct stack_entry) {
					.side = 0,
					.index = pc
				};			
				if (not visited[ins[pc].gotos[0]]) pc = ins[pc].gotos[0];
				else stack[stack_count - 1].side++;

				puts("pushed new stack element!!");
				getchar();

				if (not ctk[arg0] and not ctk[arg1]) { }
				else if (ctk[arg0]) {
					nat t = new.args[0]; 
					new.args[0] = new.args[1]; 
					new.args[1] = t;
					if (op == lt) new.op = gt_imm; 
					else new.op = eq_imm;
					new.args[1] = values[new.args[1]];
				} else {
					if (op == lt) new.op = lt_imm; 
					else new.op = eq_imm;
					new.args[1] = values[new.args[1]];
				}

			push_rt_ins:;
				nat HERE = rt_count - 1;
				if (rt_count) rt_ins[HERE].gotos[0] = rt_count;
				rt_ins[rt_count++] = new;
				printf("JUST PUSHED NEW ELEMENT!!!\n");
				getchar();
			

			} else if (stack[stack_count - 1].side == 0 and not visited[ins[pc].gotos[1]]) {
				puts("trying other side of the TOS branch!");
				getchar();
				stack[stack_count - 1].side = 1;
				pc = ins[pc].gotos[1];

			} else {
				puts("popping off top element off stack!");
				getchar();
				stack_count--;
				if (not stack_count) {
					puts("NOTE: i think we found the end?..");
					puts("breaking out of loop.");
					break;
				} else {
					puts("note: setting pc to be new value...");
					getchar();
					pc = ins[stack[stack_count].index].gotos[0];
				}
			}

		} else {
			if (pc == ins_count) {
				continue;
			}

			printf("following .false=%llu side of ins...\n", ins[pc].gotos[0]);
			pc = ins[pc].gotos[0];

		if (op == zero) {
			if (ctk[arg0]) { values[arg0] = 0; }
			else {
				new.op = set_imm;
				new.args[1] = 0;
				goto push_rt_ins;
			}

		} else if (op == incr) {
			if (ctk[arg0]) { values[arg0]++; }
			else {
				new.op = add_imm;
				new.args[1] = 1;
				goto push_rt_ins;
			}

		} else if (op == decr) {
			if (ctk[arg0]) { values[arg0]--; }
			else {
				new.op = sub_imm;
				new.args[1] = 1;
				goto push_rt_ins;
			}

		} else if (op == not_) {
			if (ctk[arg0]) { values[arg0] = ~values[arg0]; }
			else {
				new.op = eor_imm;
				new.args[1] = (nat) -1;
				goto push_rt_ins;
			}

		} else if (op == add) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] += values[arg1];
		push_rt:;
			if (not ctk[arg0]) {
				if (ctk[arg1]) {
					new.args[1] = values[arg1];
					if (op == set) new.op = set_imm;
					if (op == add) new.op = add_imm;
					if (op == sub) new.op = sub_imm;
					if (op == mul) new.op = mul_imm;
					if (op == div_)new.op = div_imm;
					if (op == and_)new.op = and_imm;
					if (op == or_) new.op = or_imm;
					if (op == eor) new.op = eor_imm;
					if (op == si)  new.op = si_imm;
					if (op == sd)  new.op = sd_imm;
				}

				if (new.op == set and arg0 == arg1) { }
				else if (new.op == or_ and arg0 == arg1) { }
				else if (new.op == and_ and arg0 == arg1) { }
				else if (new.op == add_imm and values[arg1] == 0) { }
				else if (new.op == sub_imm and values[arg1] == 0) { }
				else if (new.op == mul_imm and values[arg1] == 1) { }
				else if (new.op == div_imm and values[arg1] == 1) { }
				else if (new.op == or_imm and values[arg1] == 0) { }
				else if (new.op == eor_imm and values[arg1] == 0) { }
				else if (new.op == si_imm and values[arg1] == 0) { }
				else if (new.op == sd_imm and values[arg1] == 0) { }
				else goto push_rt_ins;

			} else if (not ctk[arg1]) {
				puts("error: ct destination must have ct source."); 
				abort();
			}
		} else if (op == set) {
			if (ctk[arg1]) values[arg0] = values[arg1];
			goto push_rt;
		} else if (op == sub) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] -= values[arg1];
			goto push_rt;
		} else if (op == mul) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] *= values[arg1];
			goto push_rt;
		} else if (op == div_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] /= values[arg1];
			goto push_rt;
		} else if (op == and_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] &= values[arg1];
			goto push_rt;
		} else if (op == or_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] |= values[arg1];
			goto push_rt;
		} else if (op == eor) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] ^= values[arg1];
			goto push_rt;
		} else if (op == si) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] <<= values[arg1];
			goto push_rt;
		} else if (op == sd) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] >>= values[arg1];
			goto push_rt;

		} else {
			puts("internal error: unknown operation: execution not specified");
			printf("op = %llu, op = %s\n", op, ins_spelling[op]);
			abort();
		}


		}
	}

	printf("found compiletime values of variables: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (not ctk[i]) continue;
		printf("\tvalues[%s] = %llu\n", names[i], values[i]);
	}
	puts("}");

	puts("found rt instruction listing after CT-eval:");
	print_instructions(rt_ins, rt_count, names, name_count);

	puts("compiled.");
	exit(0);
}



*/



































/*

		printf("visited: { ");
		for (nat i = 0; i < ins_count; i++) {
			printf("%hhu ", visited[i]);
		}
		puts("}");

*/





	//const char* text = "ct 5 zero 5 zero i at loop incr i lt i 5 loop zero i";
	//const char* text = "do skip do done at skip zero i at done";
	//const char* text = "ct 5 zero 5 zero i at loop ge i 5 done incr i do loop at done zero i";
	//const char* text = "ct 5 zero 5 zero i at loop ge i 5 done do skip zero i incr i incr i at skip incr i do loop at done zero i";
	//const char* text = "ct 5 zero 5 zero i add i 5 add i 5 incr i";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";
	//const char* text = "add hello hello";
	//const char* text = "incr hello";
	//const char* text = "lt hello hello label";
	//const char* text = "zero i lt i i done";
	//const char* text = "sc 0  0 0 0  0 0 0 ";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";
	//const char* text = "zero hello do skip incr hello at done decr hello at skip add hello hello";













/*enum immediate_forms_of_instructions {
	null_imm_unused = isa_count,
};

static const char* ins_imm_spelling[] = {
	"null_imm_unused",
	"set_imm",
	"add_imm", 
	"sub_imm",  
	"mul_imm",   
	"div_imm",
	"and_imm",
	"or_imm", 
	"eor_imm", 
	"si_imm", 
	"sd_imm", 	
	"lt_imm", 
	"eq_imm", 
};*/







	/*while (stack_count or pc < ins_count) { 

		printf("stack: %llu { \n", stack_count);
		for (nat i = 0; i < stack_count; i++) {
			printf("stack[%llu] = %llu\n", i, stack[i]);
		}
		puts("}");
		printf("visited: { ");
		for (nat i = 0; i < ins_count; i++) {
			printf("%hhu ", visited[i]);
		}
		puts("}");
		printf("found compiletime values of variables: {\n");
		for (nat i = 0; i < name_count; i++) {
			if (not ctk[i]) continue;
			printf("\tvalues[%s] = %llu\n", names[i], values[i]);
		}
		puts("}");
		//const nat top = stack[--stack_count];
		print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
		printf("executing pc #%llu\n", pc);
		print_instruction(ins[pc], names, name_count); puts("");
		print_instructions(rt_ins, rt_count, names, name_count);
		getchar();
		//visited[pc] = 1;
		struct instruction new = ins[pc];
		const nat 
			op = new.op,
			arg0 = new.args[0], 
			arg1 = new.args[1],
			gt0 = new.gotos[0],
			gt1 = new.gotos[1];

		if (op == lt or op == eq) {
			if (not ctk[arg0] or not ctk[arg1]) goto generate_rt_branch;			
			bool condition = 0;
			if (op == eq and values[arg0] == values[arg1]) condition = 1;
			if (op == lt and values[arg0] <  values[arg1]) condition = 1;
			if (not condition) {
				if (gt0 < ins_count) stack[stack_count++] = gt0;
			} else {
				if (gt1 < ins_count) stack[stack_count++] = gt1;
			}
			goto next_instruction;
			generate_rt_branch:;

			if (not ctk[arg0] and not ctk[arg1]) { }
			else if (ctk[arg0]) {
				nat t = new.args[0]; 
				new.args[0] = new.args[1]; 
				new.args[1] = t;
				if (op == lt) new.op = gt_imm; 
				else new.op = eq_imm;
				new.args[1] = values[new.args[1]];
			} else {
				if (op == lt) new.op = lt_imm; 
				else new.op = eq_imm;
				new.args[1] = values[new.args[1]];
			}

		push_rt_ins:;
			if ((1)) {
				//if (rt_ins[previous_top]) 				
				nat HERE = rt_count - 1;
				if (rt_count) rt_ins[HERE].gotos[0] = rt_count;
				rt_ins[rt_count++] = new;
			} else {
				printf("we are here again!!!\n");
				getchar();
			}

		} else if (op == zero) {
			if (ctk[arg0]) { values[arg0] = 0; }
			else {
				new.op = set_imm;
				new.args[1] = 0;
				goto push_rt_ins;
			}

		} else if (op == incr) {
			if (ctk[arg0]) { values[arg0]++; }
			else {
				new.op = add_imm;
				new.args[1] = 1;
				goto push_rt_ins;
			}

		} else if (op == decr) {
			if (ctk[arg0]) { values[arg0]--; }
			else {
				new.op = sub_imm;
				new.args[1] = 1;
				goto push_rt_ins;
			}

		} else if (op == not_) {
			if (ctk[arg0]) { values[arg0] = ~values[arg0]; }
			else {
				new.op = eor_imm;
				new.args[1] = (nat) -1;
				goto push_rt_ins;
			}

		} else if (op == add) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] += values[arg1];
		push_rt:;
			if (not ctk[arg0]) {
				if (ctk[arg1]) {
					new.args[1] = values[arg1];
					if (op == set) new.op = set_imm;
					if (op == add) new.op = add_imm;
					if (op == sub) new.op = sub_imm;
					if (op == mul) new.op = mul_imm;
					if (op == div_)new.op = div_imm;
					if (op == and_)new.op = and_imm;
					if (op == or_) new.op = or_imm;
					if (op == eor) new.op = eor_imm;
					if (op == si)  new.op = si_imm;
					if (op == sd)  new.op = sd_imm;
				}

				if (new.op == set and arg0 == arg1) { }
				else if (new.op == or_ and arg0 == arg1) { }
				else if (new.op == and_ and arg0 == arg1) { }
				else if (new.op == add_imm and values[arg1] == 0) { }
				else if (new.op == sub_imm and values[arg1] == 0) { }
				else if (new.op == mul_imm and values[arg1] == 1) { }
				else if (new.op == div_imm and values[arg1] == 1) { }
				else if (new.op == or_imm and values[arg1] == 0) { }
				else if (new.op == eor_imm and values[arg1] == 0) { }
				else if (new.op == si_imm and values[arg1] == 0) { }
				else if (new.op == sd_imm and values[arg1] == 0) { }
				else goto push_rt_ins;

			} else if (not ctk[arg1]) {
				puts("error: ct destination must have ct source."); 
				abort();
			}
		} else if (op == set) {
			if (ctk[arg1]) values[arg0] = values[arg1];
			goto push_rt;
		} else if (op == sub) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] -= values[arg1];
			goto push_rt;
		} else if (op == mul) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] *= values[arg1];
			goto push_rt;
		} else if (op == div_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] /= values[arg1];
			goto push_rt;
		} else if (op == and_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] &= values[arg1];
			goto push_rt;
		} else if (op == or_) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] |= values[arg1];
			goto push_rt;
		} else if (op == eor) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] ^= values[arg1];
			goto push_rt;
		} else if (op == si) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] <<= values[arg1];
			goto push_rt;
		} else if (op == sd) {
			if (ctk[arg0] and ctk[arg1]) values[arg0] >>= values[arg1];
			goto push_rt;
		} else if (op == sc) {
			if (not ctk[arg0]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg0];
			//const nat input_count = get_call_input_count(n);
			const nat output_count = get_call_output_count(n);
			//for (nat i = 0; i < input_count; i++) {
			//	//if (ctk[ins[top].args[1 + i]]) { puts("system call ct rt in"); abort(); }
			//	//list[list_count++] = ins[top].args[1 + i];
			//}
			for (nat i = 0; i < output_count; i++) {
				if (ctk[ins[top].args[1 + i]]) { puts("system call ct rt out"); abort(); }
				//list[list_count++] = write_access | ins[top].args[1 + i];
			}
			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(ins, ins_count, names, name_count, top, "CFG termination point here");
				goto next_instruction;
			} else {
				printf("info: found %s system call!\n", systemcall_spelling[n]);
				print_instruction_index(ins, ins_count, names, name_count, top, systemcall_spelling[n]);
			}
			//goto push_rt_ins;
			abort(); // we need to be generating a seperate intsruction for each system call.
			// this gets rid of the ct n param entirely, so that we don't need any ct var refs in the rt ins seq.
			// each syscall will have its own  rt ins.  with a specific arity, in and out. 
		} else {
			puts("internal error: unknown operation: execution not specified");
			abort();
		}

		if ((op == lt or op == eq) and gt1 < ins_count and not visited[gt1]) 
			stack[stack_count++] = gt1;

		if (gt0 < ins_count and not visited[gt0]) 
			stack[stack_count++] = gt0;

		next_instruction:;
	}


*/




































/*


	1202501116.214153

	basically the root of the problem here is that     we need to skip the line    when we are ct executing stuff. ie, when we traverse a compiletime known execution edge, we need to actually 


				like    NOTTTT use the graph traversal (GT) machinery  ie the whole stack machinery stuff 
					we need to like   just go to that instruction directly, and start executing it.  not using the stack for any of it. 



					in fact, i think we'd only ever use the stack for rt branches, right???
				i think so.  i think thats literally the only time we push to the stack.. woww..



					okay so basically we need to   revise the GT machinery 

						to make it only stack push  sides and also   only on rt brs 
					we'll just write our own GT stuff i think 


					shouldnt be that harddddd i think lol 

		hmm interestinggg




but yeah thats the root of the problem, i think. 



yay



						
*/


























/*enum language_builtins {
	stacksize, stackpointer,
	builtin_count
};

static const char* builtin_spelling[builtin_count] = {
	"_stacksize",
	"_stackpointer", 
};*/










/*	printf("found list: (%llu count)\n", list_count);
	for (nat i = 0; i < list_count; i++) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);
	}
	puts("done");
*/



/*

			bool in0 = false;
			bool in1 = false;
			for (nat i = 0; i < stack_count; i++) {
				if (stack[i] == gt0) in0 = true;
				if (stack[i] == gt1) in1 = true;
			}

*/

































	//bool* ctk = calloc(name_count, sizeof(bool));
	//nat* values = calloc(name_count, sizeof(nat));	
	//nat* bit_counts = calloc(name_count, sizeof(nat));	
	//nat* list = calloc(8 * ins_count, sizeof(nat));
	//nat list_count = 0;
	//stack[stack_count++] = 0; 
	//const struct instruction nop = {0};	


/*
	while (stack_count) { 
		const nat top = stack[--stack_count];
		printf("visiting ins #%llu\n", top);
		print_instruction_index(ins, ins_count, names, top, "here");
		debug_instruction(ins[top], names); puts("");
		getchar();
		visited[top]++;
		const nat op = ins[top].args[0], arg1 = ins[top].args[1], arg2 = ins[top].args[2];
		if (op == ld) { abort();
		} else if (op == st) { abort();
		} else if (op == ct) { ctk[arg1] = 1; ins[top].count = 0;
		} else if (op == rt) {
			if (not ctk[arg2]) { puts("error: rt instruction arg2 (bit count) must be ct."); abort(); }
			bit_counts[arg1] = values[arg2];
			ctk[arg1] = 0;
			ins[top].count = 0;
		} else if (is_branch(op)) {
			if (not ctk[arg1] or not ctk[arg2]) goto generate_rt_branch;
			bool condition = 0;
			if (op == eq and values[arg1] == values[arg2]) condition = 1;
			if (op == ne and values[arg1] != values[arg2]) condition = 1;
			if (op == lt and values[arg1] <  values[arg2]) condition = 1;
			if (op == ge and values[arg1] >= values[arg2]) condition = 1;
			rt_ins[rt_count++] = ins[top];
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if (not condition) {
				if (false_side < ins_count) stack[stack_count++] = false_side;
			} else {
				if ( true_side < ins_count) stack[stack_count++] = true_side;
			}
			goto next_instruction;
			generate_rt_branch:;
		} else if (op == do_) {
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if (false_side < ins_count) stack[stack_count++] = false_side;
			goto next_instruction;
		} else if (op == zero) {
			if (ctk[arg1]) { values[arg1] = 0; ins[top].count = 0; }
		} else if (op == incr) {
			if (ctk[arg1]) { values[arg1]++; ins[top].count = 0; }
		} else if (op == not_) {
			if (ctk[arg1]) { values[arg1] = ~values[arg1]; ins[top].count = 0; }
		} else if (op == add) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] += values[arg2];
		push_rt:;
			if (not ctk[arg1]) {
				if (ctk[arg2]) {
					ins[top].args[2] = values[arg2];
					if (op == set) ins[top].args[0] = set_imm;
					if (op == add) ins[top].args[0] = add_imm;
					if (op == sub) ins[top].args[0] = sub_imm;
					if (op == mul) ins[top].args[0] = mul_imm;
					if (op == div_)ins[top].args[0] = div_imm;
					if (op == and_)ins[top].args[0] = and_imm;
					if (op == or_) ins[top].args[0] = or_imm;
					if (op == eor) ins[top].args[0] = eor_imm;
					if (op == si)  ins[top].args[0] = si_imm;
					if (op == sd)  ins[top].args[0] = sd_imm;
				}
			} else {
				if (ctk[arg2]) {
					ins[top].count = 0;
				} else {
					puts("error: compiletime destination must have compiletime source."); abort();
				}
			}
		} else if (op == set) {
			if (ctk[arg2]) values[arg1] = values[arg2];
			goto push_rt;
		} else if (op == sub) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] -= values[arg2];
			goto push_rt;
		} else if (op == mul) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] *= values[arg2];
			goto push_rt;
		} else if (op == div_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] /= values[arg2];
			goto push_rt;
		} else if (op == and_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] &= values[arg2];
			goto push_rt;
		} else if (op == or_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] |= values[arg2];
			goto push_rt;
		} else if (op == eor) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] ^= values[arg2];
			goto push_rt;
		} else if (op == si) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] <<= values[arg2];
			goto push_rt;
		} else if (op == sd) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] >>= values[arg2];
			goto push_rt;
		} else if (op == sc) {
			if (not ctk[arg1]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg1];
			//const nat input_count = get_call_input_count(n);
			const nat output_count = get_call_output_count(n);
			//for (nat i = 0; i < input_count; i++) {
			//	//if (ctk[ins[top].args[2 + i]]) { puts("system call ct rt in"); abort(); }
			//	//list[list_count++] = ins[top].args[2 + i];
			//}
			for (nat i = 0; i < output_count; i++) {
				if (ctk[ins[top].args[2 + i]]) { puts("system call ct rt out"); abort(); }
				//list[list_count++] = write_access | ins[top].args[2 + i];
				ctk[ins[top].args[2 + i]] = false;
			}

			rt_ins[rt_count++] = ins[top];

			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(ins, ins_count, names, top, "CFG termination point here");
				goto next_instruction;

			} else {
				printf("info: found %s system call!\n", systemcall_spelling[n]);
				print_instruction_index(ins, ins_count, names, top, systemcall_spelling[n]);
			}
		}

		nat true_side = (nat) -1;
		const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
		if (is_branch(op)) {
			if ( true_side < ins_count and visited[true_side] < 1)  stack[stack_count++] = true_side;
			if (false_side < ins_count and visited[false_side] < 1) stack[stack_count++] = false_side;
		} else {
			if (false_side < ins_count) stack[stack_count++] = false_side;
		}
		next_instruction:;
	}



	puts("ins:");
	debug_instructions(ins, ins_count, names);

	puts("rt_ins:");
	debug_instructions(rt_ins, rt_count, names);






	puts("}");
	printf("found compiletime values of variables: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (not ctk[i]) continue;
		printf("\tvalues[%s] = %llu\n", names[i], values[i]);
	}
	puts("}");
	printf("found list: (%llu count)\n", list_count);
	for (nat i = 0; i < list_count; i++) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);
	}
	puts("done");



	bool* alive = calloc(name_count, sizeof(bool));

	for (nat i = list_count; i--;) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);

		const bool is_write = !!(list[i] >> 63);

		if (is_write) {			
			alive[variable_index] = 0;
		} else {
			alive[variable_index] = 1;			
		}
		
		printf("alive = { ");
		for (nat n = 0; n < name_count; n++) {
			if (alive[n]) printf("%s  ", names[n]);
		} 
		printf(" }\n");
		//getchar();
	}





*/








/*


		printf("stack: %llu { \n", stack_count);
		for (nat i = 0; i < stack_count; i++) {
			printf("stack[%llu] = %llu\n", i, stack[i]);
		}
		puts("}");

		printf("visited: { ");
		for (nat i = 0; i < ins_count; i++) {
			printf("%hhu ", visited[i]);
		}
		puts("}");

		printf("ctk: { ");
		for (nat i = 0; i < name_count; i++) {
			if (ctk[i]) printf("%s ", names[i]);
		}
		puts("}");

		printf("CT values of variables: {\n");
		for (nat i = 0; i < name_count; i++) {
			if (not ctk[i]) continue;
			printf("\tvalues[%s] = %llu\n", names[i], values[i]);
		}
		puts("}");


*/





/*	printf("performing unreachable analysis...\n");
	for (nat i = 0; i < rt_count; i++) {

		//const nat op = rt_ins[i].args[0], arg1 = rt_ins[i].args[1], arg2 = rt_ins[i].args[2];

		/if (not visited[i]) {
			printf("warning: instruction is unreachable\n");
			print_instruction_index(ins, ins_count, names, i, "unreachable");
			puts("");
			ins[i].count = 0;
		}

		if (ctk[arg1] and op != sc) {
			ins[i].count = 0;
		}
	}
*/


/*




				ERROR ERROR ERROR       we are in the middle of making this pass   generate   rt_ins    never edit main instruction sequence, ins. 

					this is because we want to avoid generating    the at loop's,  and also  get the immediate to be different for each add_imm that we generate. we can't do that since we are overwriting the original instruction, which we used for execution lol. i think. sometihng like that. basically we need to do the rt_ins lol. its a must. don't worry about unreachable instructions, we won't generate them anyways, becuase we are traversing the graph in order to know the right instructions to output. i think. CRAP 


							BUT WHAT ABOUT THE FACT THAT 




									WE WONT BE GENERATING THE INSTRUCTIONS IN THE CORRECT ORDERRRR CRAPPPP



									DUE TO IMPLICIT IP++'s    NOT LINING UPPPP



				BECAUSE OF OUR GRAPH TRAVERSAL ORDERINGGGG   CRAPPPPPP









uh oh lol 


uhh





hmm

*/


				//printf("arg_count = %llu\n", arg_count);
				//printf("variable = %llu\n", variable);
				//printf("name_count = %llu\n", variable);






















/*

ct 5 
zero 5 

zero i 
at loop 
	incr i 
	lt i 5 loop 
zero i

*/






















































	// add names[] and ctk[] and values[] arrays. 

	// delete    ct and rt  ins first, 
	//  then figure out how we'll do unreachable analysis for  do instructions, 
	//  and also figure out how we'll only "execute" a do instruction if its reachable,
	//  and attribute the location into the .false side of the previous instruction. 

	// at the end of this whole process, we need to do the   branch complementation thingy 
	// to switch the .true and .false  if we are using a nonexistent condition:  ge ne

	// lf is handled already. same with eoi.     we need to make    at handled by 
	//  actually using like a mapping of   addresses to label names, i think!
	// and if we are using ip++, then we just need to figure out how to make the mapping between the frist instruction and the second in the chain   which ip++   connects. 








/*


struct instruction {
	nat op;
	nat label;
	nat gotos[2];
	nat args[7];
};








*/




/*static const nat arity[] = {
	0, 
	1, 1, 1, 
	1, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 
	3, 3, 3, 3, 7,
	0,
	1, 1, 1, 3, 3, 1, 2
};



			if (ctk[arg0]) { // we therefore know arg1 is   NOT ctk. 


				if (new.op == lt) {

				} else {

				}


			} else {
				if (ctk[arg1]) {

					if (new.op == lt) {

					} else {

					}
				} else { // we know that both are ct. generate the branch, normally. 
					if (new.op == lt) {

					} else {

					}

				}
			}



*/




			/*if (ctk[arg0] ^ ctk[arg1] and new.op == eq) new.op = eq_imm;


			if (not ctk[arg0] and ctk[arg1] and new.op == lt) new.op = lt_imm;
			if (ctk[arg0] and not ctk[arg1] and new.op == lt) { 
				new.op = gt_imm; nat t = new.args[0]; 
				new.args[0] = new.args[1]; new.args[1] = t;
			} 

			//if (ctk[arg

			rt_ins[rt_count++] = new;
*/






				//state = 0;
				//if (unreachable) goto next_word;
				// this is not correct. 
				//ins[ins_count - 1].gotos[0] = variable | is_label;
				// nop do label  at skip nop  at label do label2
				//   the above code breaks this method. 








/*static nat isa_arity(nat i) {
	if (i == sc) return 7;
	if (i == eoi) return 0;
	if (i >= zero and i <= lf) return 1;
	if (i >= set  and i <= rt) return 2;
	if (i >= lt   and i <= st) return 3;
	abort();
}

static nat get_call_input_count(nat n) {
	if (n == system_exit) return 1;
	if (n == system_read) return 3;
	if (n == system_write) return 3;
	if (n == system_close) return 1;
	if (n == system_open) return 3;
	abort();
}*/





/*


		" %s"
		" %s"
		//" {ct_side=%u}"
		" %s",
		this.gotos[0], this.gotos[1],
		"",//!!(this.ct & ct_is_generated_do) ? "[machine-do]" : "", 
		"",//!!(this.ct & ct_is_unreachable) ? "[unreachable]" : "",
		//0,//!!(this.ct & ct_is_ctbranch_side), 
		""//!!(this.ct & ct_is_compiletime) ? "[compiletime]" : ""
	);



	if (use_color) {
		if (this.ct & ct_is_unreachable) printf("\033[0m");
		else if (this.ct & ct_is_compiletime) printf("\033[0m");
	}*/




/*

//printf("PRED UNIDENT: c=%u, a=%u, b=%u\n", c, a, b);
				//getchar();		
if (pred_ctk[n] and ctk[n]) {
						puts("found both CTK!!!! unknown merge method");
						abort();



					}

static const char* systemcall_spelling[systemcall_count] = {
	"system_exit",
	"system_read", "system_write", 
	"system_open", "system_close",
};


*/


		//printf("info:   --> pred_count = %llu\n", pred_count);
/*printf("FOUND PREDECESSOR!!!! (%llu total so far)\n", pred_count);
			print_instruction_index(ins, ins_count, names, name_count, i, "predecessor of pc");
			print_instruction_index(ins, ins_count, names, name_count, pc, "PC");
			printf("DONE WITH PREDECESSOR\n");
			printf("COMPARING NAME VALUE CTK LISTS:\n");
			nat* pred_values = execution_state_values + name_count * i;
			nat* pred_ctk = execution_state_ctk + name_count * i;	


			for (nat n = 0; n < name_count; n++) {
				printf("%llu:   {PC: CTK[%llu],VALUES[%llu]}    |   {pred: CTK[%llu],VALUES[%llu]} \n",
					n,               ctk[n],values[n],          pred_ctk[n], pred_values[n]
				);
					if (not pred_ctk[n]) {

						// then we know that at least one of the predecessors of PC transfers RT known data about this variable.
						// if this is the case, we must assume that now, at this point, names[n] is RT here as well. 

						ctk[n] = 0;
						values[n] = 0x999999999999;
					}
				}
				puts("done comparing these two pred and pc lists.");
				getchar();
			}
			*/





		// 1. find list of pred of this instruction.
		
		// 2. synthesize the ctk/value listing for each other path. if both ctk, but mismatching values,
		//    then call it not rtk.

		// 3. ????
		
		//nat** CRAZY_ctk = calloc(2 * name_count, sizeof(nat*));
		//nat** CRAZY_values = calloc(2 * name_count, sizeof(nat*));



		/*for (nat i = 0; i < pred_count; i++) {
			print_instructions_ct_values_index(
				ins, ins_count, 
				names, name_count, locations, 
				execution_state_ctk, execution_state_values,
				preds[i], "PREDECESSOR"
			);
		}*/

/*static void print_instruction_index(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count,
	nat here, const char* message
) {
	printf("%s: at index: %llu: { \n", message, here);
	for (nat i = 0; i < ins_count; i++) {
		printf("\t#%04llu: ", i);
		print_instruction(ins[i], names, name_count);
		if (i == here)  printf("  <------ %s\n", message); else puts("");
	}
	puts("}");
}

static void print_ct_values(char** names, nat name_count, nat* ctk, nat* values) {
	printf("ct values: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (not ctk[i]) continue;
		printf("\tvalues[%s] = 0x%016llx\n", names[i], values[i]);
	}
	puts("}");
}*/
/*	if (use_color) {
		if (this.ct & ct_is_unreachable) printf("\033[38;5;239m");
		else if (this.ct & ct_is_compiletime) printf("\033[38;5;101m");
	}*/

	//printf("[.ct=%llx]", this.ct);







//EOI




