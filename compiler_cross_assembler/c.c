// 1202504045.155147 a compiler / assembler for a simpler version of the language.

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
#define max_arg_count (14)

enum all_architectures { no_arch, arm64_arch, arm32_arch, rv64_arch, rv32_arch, msp430_arch, };
enum all_output_formats { debug_output_only, macho_executable, elf_executable, ti_txt_executable };

enum core_language_isa {
	nullins,

	halt, sc, do_, at, lf,
	set, add, sub, mul, div_, rem, 
	and_, or_, eor, si, sd, la, rt,
	ld, st, lt, ge, ne, eq,
	
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

	"halt", "sc", "do", "at", "lf",
	"set", "add", "sub", "mul", "div", "rem", 
	"and", "or", "eor", "si", "sd", "la", "rt", 
	"ld", "st", "lt", "ge", "ne", "eq",

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

	0, 0, 
	1, 1, 1,
	2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 
		
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
	nat imm;
	nat args[max_arg_count];
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

static void print_dictionary(
	char** variables, 
	nat* is_undefined,
	nat var_count
) {
	puts("variable dictionary: ");
	for (nat i = 0; i < var_count; i++) {
		printf("     %c [%llu]  \"%s\"\n",  // 0x%016llx
			is_undefined[i] ? 'U' : ' ', 
			i, variables[i]
		);
	}
	puts("[end]");
}

static void print_instruction(
	struct instruction this, 
	char** variables, nat var_count
) {
	int max_name_width = 0;
	for (nat i = 0; i < var_count; i++) {
		if (max_name_width < (int) strlen(variables[i])) {
			max_name_width = (int) strlen(variables[i]);
		}
	}

	printf(" %4s  ", operations[this.op]);
	/*int left_to_print = max_name_width - (int) strlen(operations[this.op]);
	if (left_to_print < 0) left_to_print = 0;
	for (int i = 0; i < left_to_print; i++) putchar(' ');
	putchar(' '); }*/

	for (nat a = 0; a < arity[this.op]; a++) {

		char string[4096] = {0};
		if (this.imm & (1 << a)) snprintf(string, sizeof string, "%llu", this.args[a]);
		else snprintf(string, sizeof string, "%s", variables[this.args[a]]);

		printf("%s", string);
		int left_to_print = max_name_width - (int) strlen(string);
		if (left_to_print < 0) left_to_print = 0;
		for (int i = 0; i < left_to_print; i++) putchar(' ');
		putchar(' ');
	}	
}

static void print_instructions(
	struct instruction* ins, 
	nat ins_count,
	char** variables, nat var_count,
	bool should_number_them
) {
	printf("instructions: (%llu count) {\n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op != at) putchar(9); else putchar(10);
		if (should_number_them) printf("%4llu: ", i);
		print_instruction(ins[i], variables, var_count);
		puts("");
	}
	puts("}");
}

static void print_instruction_window_around(
	nat this, 
	struct instruction* ins, 
	nat ins_count, 
	char** variables, 
	nat var_count,
	nat* visited
) {
	nat row_count = 0;
	printf("\033[H\033[2J");
	const int64_t window_width = 12;
	const int64_t pc = (int64_t) this;
	for (int64_t i = -window_width; i < window_width; i++) {

		const int64_t here = pc + i;

		if (ins[here].op == at) { putchar(10); row_count++; }

		if (not i) printf("\033[48;5;238m");

		if (	here < 0 or 
			here >= (int64_t) ins_count
		) { puts("\033[0m"); row_count++; continue; }

		printf("  %s%4llu │ ", 
			not i and visited[here] ? 
			"\033[32;1m•\033[0m\033[48;5;238m"
			: (visited[here] ? "\033[32;1m•\033[0m" : " "), 
			here
		);
		if (not i and visited[here]) printf("\033[48;5;238m");

		if (ins[here].op != at) putchar(9);
		print_instruction(ins[here], variables, var_count);
		puts("\033[0m");
		row_count++; 
	}
	while (row_count < 2 * window_width + 6) { row_count++; putchar(10); } 
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
		printf("%llu ", array[i]);
	}
	printf("}");
}

/*

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


static nat* compute_predecessors(
	struct instruction* ins, 
	nat ins_count, nat pc, 
	nat* pred_count
) {
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


static nat* compute_successors(
	struct instruction* ins, 
	nat ins_count, nat pc
) {
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
	result[0] = gotos[2 * pc + 0];
	result[1] = gotos[2 * pc + 1];
	free(gotos);
	return result;
}


int main(int argc, const char** argv) {

	if (argc != 2) 
		exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));

	//const nat min_stack_size = 16384 + 1;
	//nat stack_size = min_stack_size;
	//const char* output_filename = "output_executable_new";

	struct instruction ins[max_instruction_count] = {0};
	nat ins_count = 0;

	char* variables[max_variable_count] = {0};
	nat is_undefined[max_variable_count] = {0};
	nat var_count = 0;

	const char* included_files[4096] = {0};
	nat included_file_count = 0;
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

	nat 	word_length = 0, word_start = 0, 
		arg_count = 0, is_immediate = 0;

	nat args[max_arg_count] = {0};

	for (nat var = 0, op = 0, pc = starting_index; pc < text_length; pc++) {

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
					//printf("skipping over: '%c'  "
					//	"(comment = %llu)\n", 
					//	text[i], comment
					//);
					if (text[i] == '(') comment++;
					if (text[i] == ')') comment--;
					i++;
				}
				if (comment) goto print_error;
				pc = i;
				goto next_word; 
			}

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
				//last_used = var;
				goto push_argument;
			}
		} 

		if (not(  
			op == set and arg_count == 0 or
			op == ld  and arg_count == 0 or
			op == rt  and arg_count == 0 or
			op == do_ or op == at or op == la or 
			(op == lt or op == ge or 
	 		 op == ne or op == eq) 
			and arg_count == 2
		)) {
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
		variables[var] = word; 
		var_count++;

	push_argument: 
		args[arg_count++] = var;

	process_op: 
		if (arg_count < arity[op]) goto next_word;
		//else if (op == ud) is_undefined[last_used] = 1;
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
			struct instruction new = { .op = op, .imm = is_immediate };
			memcpy(new.args, args, sizeof args);
			is_immediate = 0;
			ins[ins_count++] = new;
		}
		arg_count = 0; op = 0;
		next_word: word_length = 0;
	}
	file_count--;
	if (file_count) goto process_file; 

	print_dictionary(variables, is_undefined, var_count);
	print_instructions(ins, ins_count, variables, var_count, 0);
	getchar();

	nat* type = calloc(ins_count * var_count, sizeof(nat));  // is_compiletime[...]
	nat* value = calloc(ins_count * var_count, sizeof(nat));  // values[...]

	nat stack[4096] = {0};
	nat stack_count = 1;
	nat visited[4096] = {0};
	nat last_pc = (nat) -1;
	nat is_runtime[4096] = {0};



	const nat debug_ct_eval = 1;










	while (stack_count) {
		nat pc = stack[--stack_count];

	execute_ins:
		if (pc >= ins_count) continue;
		nat pred_count = 0;
		nat* preds = compute_predecessors(ins, ins_count, pc, &pred_count);
		nat* gotos = compute_successors(ins, ins_count, pc);

		if (debug_ct_eval) {

		print_instruction_window_around(pc, ins, ins_count, variables, var_count, visited);
		print_dictionary(variables, is_undefined, var_count);
		printf("     ");
		for (nat j = 0; j < var_count; j++) {
			printf("%3lld ", j);
		}
		puts("\n-------------------------------------------------");
		for (nat i = 0; i < ins_count; i++) {
			printf("%3llu: ", i);
			for (nat j = 0; j < var_count; j++) {
				if (type[i * var_count + j])
					printf("%3lld ", value[i * var_count + j]);
				else 	printf("\033[90m%3llx\033[0m ", value[i * var_count + j]);
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
		}

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

		const nat this_var = e;  // for now. this would be a for loop over the variables in general. 
		nat future_type = 0; // assume its runtime known.
		nat future_value = 0;
		nat mismatch = 0;

		for (nat iterator_p = 0; iterator_p < pred_count; iterator_p++) {

			const nat pred = preds[iterator_p];

			if (not visited[pred]) { 
				continue;
			}
			const nat t = type[pred * var_count + this_var];
			const nat v = value[pred * var_count + this_var];	
			if (t == 0) { future_type = 0; future_value = v; break; } 
			
			if (not future_type) { 
				future_type = 1; 
				future_value = v; 
			} else if (future_value == v) { 
			} else {
				mismatch = 1; 
			}
		}

		if (mismatch and future_type) {

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
				abort(); 
			}
			if (last_pc == (nat) -1) abort();

			const nat pred = last_pc;

			if (not visited[pred]) { 
				continue; 
			}
			const nat t = type[pred * var_count + this_var];
			const nat v = value[pred * var_count + this_var];
			future_type = t;
			future_value = v;
		}

		type[pc * var_count + this_var] = future_type;
		value[pc * var_count + this_var] = future_value;

		} // for e 

		//printf("pc = %llu, a0 = %llu, a1 = %llu, a1 = %llu\n", pc, a0, a1, a2); fflush(stdout);
		const nat ct0 = i0 or type[pc * var_count + a0];
		const nat ct1 = i1 or type[pc * var_count + a1];
		const nat ct2 = i2 or type[pc * var_count + a2];
		const nat v0 = i0 ? a0 : value[pc * var_count + a0];
		const nat v1 = i1 ? a1 : value[pc * var_count + a1];
		//const nat v2 = i2 ? a2 : value[pc * var_count + a2];

		nat out_t = ct0, out_v = v0;
		last_pc = pc;

		if (op == halt) {
			//puts("executing a halt statement..."); getchar();
			continue;
		}

		else if (op == at) { }
		else if (op == do_) { pc = gt0; goto execute_ins; } 		
		else if (op == set) { if (not is_runtime[a0]) { out_t = ct1; out_v = v1; } }
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

		} else if (op == st) {	
			if (not ct2) { puts("error: size of store must be ctk."); abort(); }

		} else if (op == ld) {
			if (not ct2) { puts("error: size of load must be ctk."); abort(); }
			out_t = 0;

		} else if (op == la) {
			out_t = 0;
			out_v = 0;
						
		} else if (op == lt or op == ge or op == ne or op == eq) {
			if (not ct0 or not ct1) {
				last_pc = (nat) -1;
				if (gt0 < ins_count and visited[gt0] < 2) stack[stack_count++] = gt0;
				if (gt1 < ins_count and visited[gt1] < 2) stack[stack_count++] = gt1;
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
			if ((int64_t) v1 < 0) { puts("STORING REGSITER INDEX"); }
			else { puts("STORING BIT COUNT"); }
			getchar();

			is_runtime[a0] = 1;
			out_t = 0;

		} else {
			puts("WARNING: EXECUTING AN UNKNOWN INSTRUCTION WITHOUT AN IMPLEMENTATION!!!");
			puts(operations[op]);
			puts("note this!!!"); getchar();
			abort();
		}

		type[pc * var_count + a0] = out_t;
		value[pc * var_count + a0] = out_v;
		pc++; goto execute_ins;
	}


	print_instruction_window_around(0, ins, ins_count, variables, var_count, visited);
	print_instructions(ins, ins_count, variables, var_count, 0);
	print_dictionary(variables, is_undefined, var_count);
	puts("types and values");
	puts("------------------------------------------");
	printf("     ");
	for (nat j = 0; j < var_count; j++) {
		printf("%3lld ", j);
	}
	puts("\n------------------------------------------");
	for (nat i = 0; i < ins_count; i++) {
		printf("%3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (type[i * var_count + j])
				printf("%3lld ", value[i * var_count + j]);
			else 	printf("\033[90m%3llx\033[0m ", value[i * var_count + j]);
		}
		puts("");
	}
	puts("------------------------------------------");
	printf("stack: "); print_nats(stack, stack_count);
	print_nats(visited, ins_count);




	for (nat i = 0; i < ins_count; i++) {

		//print_instruction_window_around(i, ins, ins_count, variables, var_count, visited);
		//getchar();

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		
		nat ignore = 1;

		if (	op == halt or 
			op == sc or 
			op == do_ or 
			op == at or 
			op == rt or 
			op == la or 
			op == ld or 
			op == st
		) ignore = 0;

		puts("looking at: "); 
		print_instruction(ins[i], variables, var_count); puts("");
		
		if (ignore) for (nat a = 0; a < arity[op]; a++) {

			if (op == at and a == 0) continue;
			if (op == do_ and a == 0) continue;
			if (op == lt and a == 2) continue;
			if (op == ge and a == 2) continue;
			if (op == ne and a == 2) continue;
			if (op == eq and a == 2) continue;
			if (op == la and a == 1) continue; 
						
			// TODO: work this out:     are la's CT OR NOT!?!?!?!    .....they arent. lol. i think. 

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
			puts("ignoring instruction...\n");

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

			} else visited[i] = 0;

		} else {
			puts("found real RT isntruction!"); putchar('\t');
			print_instruction(ins[i], variables, var_count); puts("");

			if ((op >= set and op <= sd) or op == rt) {
				const nat ct1 = (imm & 2) or type[i * var_count + ins[i].args[1]];
				const nat v1 = (imm & 2) ? ins[i].args[1] : value[i * var_count + ins[i].args[1]];

				if (ct1 and not (imm & 2)) {
					puts("found an instruction which has a ct argument!");
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
					) visited[i] = 0;
				}

			} else if (op == ld or op == st) {
				puts("we still need to embed the immediates into the load and store instructions.");
				abort();	

			} else if (op == lt or op == ge or op == ne or op == eq) {
				puts("we still need to embed the immediates into the conditioal branch instructions.");

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

		//getchar();
	}



//////////////////opt////////////////

	// optimizations left to perform:
	/*

		- copy propagation / simplification of dataflow

		- control flow simplification, eliminate chained jumps

		- simplify conditions?...


		-   TODO: look up some more lol.

	*/
	for (nat i = 0; i < ins_count; i++) {
		if (not visited[i]) continue;
		const nat op = ins[i].op;

		// lets try to simplify things now?
	}

//////////////////////////////////


	for (nat i = 0; i < ins_count; i++) {
		if (not visited[i]) continue;
		print_instruction_window_around(i, ins, ins_count, variables, var_count, visited);
		getchar();
	}

	nat final_ins_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		if (not visited[i] and op != at and op != rt) continue;
		ins[final_ins_count++] = ins[i];
	}
	ins_count = final_ins_count;

	memset(visited, 0, sizeof visited);

	print_dictionary(variables, is_undefined, var_count);
	print_instructions(ins, ins_count, variables, var_count, 0);
	puts("[done]");
	getchar();

	exit(0);
}




























































































































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
		else if (op >= a6_nop and op <= m4_br) {}

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












