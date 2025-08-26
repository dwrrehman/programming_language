// the backend for the compiler!
// written on 1202508251.142842 by dwrr

/*

language ISA as of: 1202508251.143137

	set r r
	add r r
	sub r r
	mul r r
	div r r
	rem r r
	and r r
	or r r
	eor r r 
	si r r 
	sd r r 
	
	li r k
	la r l

	lt r r l
	ge r r l
	ne r r l
	eq r r l	
	at l
	do l
	sc
	halt
	
	ldb r r
	ldh r r
	ldw r r
	ldd r r
	stb r r
	sth r r
	stw r r
	std r r

	reg r k

	emit k
	sect k

	def r
	del r
	
	target k
	format k 
	stacksize k 
	overwrite

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

enum all_architectures { 
	no_arch,
	riscv_arch, 
	arm64_arch, 
	msp430_arch, 
	c_arch, 
};

enum all_output_formats {
	no_output,
	raw_binary,
	hex_array,
	c_source,
	macho_executable, macho_object,
	elf_executable, elf_object,
	ti_txt_executable,
	uf2_executable,
};

enum isa {
	nullins,
	set, add, sub, mul, div_, rem, 
	and_, or_, eor, si, sd, li, la, 
	ldb, ldh, ldw, ldd, 
	stb, sth, stw, std,
	reg, 

	lt, ge, ne, eq, 
	at, do_, sect, emit, 
	sc, halt, eoi,
	def, del, 

	target, format, stacksize, overwrite, 

	riscv_add, riscv_addi, riscv_bltu, riscv_jal,
isa_count
};

static const char* operations[isa_count] = {
	"",
	"set", "add", "sub", "mul", "div", "rem", 
	"and", "or", "eor", "si", "sd", "li", "la", 
	"ldb", "ldh", "ldw", "ldd", 
	"stb", "sth", "stw", "std",
	"reg", 

	"lt", "ge", "ne", "eq", 
	"at", "do", "sect", "emit", 
	"sc", "halt", "eoi", 
	"def", "del", 

	"target", "format", "stacksize", "overwrite", 

	"riscv_add", "riscv_addi", "riscv_bltu", "riscv_jal",
};

static const nat arity[isa_count] = {
	0,
	2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 
	2, 2, 2, 2, 
	2, 
	
	3, 3, 3, 3, 
	1, 1, 1, 1, 
	0, 0, 0, 
	1, 1, 

	1, 1, 1, 0, 

	3, 3, 3, 2,
};

struct instruction {
	nat op;
	nat imm;
	nat state;
	nat args[max_arg_count];
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
static nat register_index[max_variable_count] = {0};
static byte is_label[max_variable_count] = {0};
static nat var_count = 0;

static void print_nats(nat* array, nat count) {
	printf("(%llu) { ", count);
	for (nat i = 0; i < count; i++) {
		printf("%lld ", array[i]);
	}
	printf("}");
}

static char* load_file(const char* filename, nat* length) {
	int file = open(filename, O_RDONLY);
	if (file < 0) { 
		printf("compiler backend: error: could not open '%s': %s\n", 
			filename, strerror(errno)
		); 
		exit(1);
	}
	*length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(*length + 1, 1);
	read(file, text, *length);
	close(file);
	return text;
}

static void print_dictionary(void) {
	puts("variable dictionary: ");
	for (nat i = 0; i < var_count; i++) {
		printf("   %c [%5llu]  "
			"\"%20s\"  :  %5lld \n",
			is_label[i] ? 'L' : ' ', 
			i, variables[i], register_index[i]
		);
	}
	puts("[dictionary end]");
}

static void print_instruction(struct instruction this) {

	printf(" %10s ", operations[this.op]);

	for (nat a = 0; a < arity[this.op]; a++) {
		char string[4096] = {0};
		if (this.imm & (1 << a)) snprintf(string, sizeof string, "0x%llx(%lld)", this.args[a], this.args[a]);
		else if (this.args[a] < var_count) snprintf(string, sizeof string, "%s\033[38;5;235m(%llu)\033[0m", variables[this.args[a]], this.args[a]);
		else snprintf(string, sizeof string, "(INTERNAL ERROR)");

		printf("%s", string);
		putchar(' ');
	}
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

static void print_index(const char* text, nat length, nat begin, nat end) {
	printf("\n\t@%llu..%llu: ", begin, end); 
	while (end and isspace(text[end])) end--;
	const nat max_width = 100;
	nat start_at = 
		begin < max_width ? 0 : begin - max_width;

	nat end_at = 
		end + max_width >= length 
		? length : end + max_width;

	for (nat i = start_at; i < end_at; i++) {
		if (i == begin) printf("\033[32;1m[");
		putchar(text[i]);
		if (i == end) printf("]\033[0m");
	}	
	printf("\033[0m");
	puts("\n");
}

static const nat unreachable = (nat) -1;

static nat compute_label_location(nat label) {
	nat locations[4096] = {0};
	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) locations[ins[i].args[0]] = i;
	}
	return locations[label];
}

static nat compute_successors(const nat pc, nat* true_side) {
	nat locations[4096] = {0};
	for (nat i = 0; i < ins_count; i++) 
		if (ins[i].op == at) locations[ins[i].args[0]] = i;

	nat f = unreachable, t = unreachable;
	nat i = pc;
	const nat op = ins[i].op;
	if (op == halt) {} 
	else if (op == lt or op == ge or op == ne or op == eq or op == riscv_bltu) { 
		f = i + 1; t = locations[ins[i].args[2]]; 
	} else if (op == do_) f = locations[ins[i].args[0]];
	else if (op == riscv_jal) f = locations[ins[i].args[1]];
	else f = i + 1;
	*true_side = t;
	return f;
}

static nat* compute_predecessors(nat pc, nat* pred_count) {
	nat* gotos = calloc(2 * ins_count, sizeof(nat)); 
	for (nat i = 0; i < ins_count; i++) {
		nat t = unreachable;
		const nat f = compute_successors(i, &t);
		gotos[i * 2 + 0] = f;
		gotos[i * 2 + 1] = t;
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

/*static nat* compute_arm64_successors(nat pc) {

	nat* gotos = calloc(2 * ins_count, sizeof(nat)); 
	nat locations[4096] = {0};

	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].op == at) locations[ins[i].args[0]] = i;
	}

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;


		if (op == halt) {
			gotos[2 * i + 0] = (nat) -1;
			gotos[2 * i + 1] = (nat) -1;

		} else if (op == a6_cbz or op == a6_tbz or 
			(op == a6_bc and ins[i].args[0] != 14 and ins[i].args[0] != 15)
		) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = locations[ins[i].args[1]];

		} else if ((op == a6_bc and (ins[i].args[0] == 14 or ins[i].args[0] == 15)) or 
			op == a6_jmp
		) {
			gotos[2 * i + 0] = locations[ins[i].args[1]];
			gotos[2 * i + 1] = (nat) -1;


		} else if (	
			op == a6_divr
		) {
			gotos[2 * i + 0] = i + 1;
			gotos[2 * i + 1] = (nat) -1;

		} else {
			puts("error: compute_arm64_successors(): found a non A6 MI...\n");
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

static nat* compute_arm64_predecessors(nat pc, nat* pred_count) {

	return NULL;
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

*/


static nat locate_instruction(struct expected_instruction expected, nat starting_from) {

	nat pc = starting_from;

	while (pc < ins_count) {

		nat pred_count = 0;
		compute_predecessors(pc, &pred_count);

		nat gotos[2] = {0};
		gotos[0] = compute_successors(pc, gotos + 1);

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

		if (use_arg0 and arg0 == expected.args[0]) break;
		if (use_arg0 and arg1 == expected.args[0]) break;
		if (use_arg1 and arg0 == expected.args[1]) break;
		if (use_arg1 and arg1 == expected.args[1]) break;

		pc = gotos[0];
	}
	return unreachable;
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

static void debug_data_flow_state(
	nat pc,
	nat* preds, nat pred_count,
	nat* stack, nat stack_count,
	nat* value, nat* type, 
	nat* is_copy, nat* copy_of
) {

	print_instruction_window_around(pc, 0, "PC");
	//print_dictionary(0);

	printf("  "); 
	for (nat j = 0; j < var_count; j++) { 
		if (is_label[j]) continue; 
		printf("%3s(%llu) ", variables[j], j); 
	}
	puts("");

	
	for (nat i = 0; i < ins_count; i++) {
		printf("ct %3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_label[j]) continue; 
			if (not type[i * var_count + j]) printf("\033[90m");
			printf("%4lld ", value[i * var_count + j]);
			if (not type[i * var_count + j]) printf("\033[0m");
		}
		putchar(9);

		printf("cp %3llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_label[j]) continue; 
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

		//printf("\033[38;5;235m");
		//for (nat _ = 0; _ < 350; _++) printf("-");
		//printf("\033[0m");
		//putchar(10);

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
		if (is_label[j]) continue; 
		printf("%20s(%04llu) ", variables[j], j); 
	} 
	puts("");

	for (nat i = 0; i < ins_count; i++) {
		printf("%2llu: ", i);
		for (nat j = 0; j < var_count; j++) {
			if (is_label[j]) continue; 
			if (not alive[i * var_count + j]) printf("\033[38;5;235m");
			printf("%4llu  ", alive[i * var_count + j]);
			if (not alive[i * var_count + j]) printf("\033[0m");
		}
		print_instruction(ins[i]);
		
		if (i == pc) { 
			putchar(32); 
			print_nats(gotos, goto_count); 
			print_nats(preds, pred_count); 
			putchar(32); 
			printf("\033[32;1m     <------- PC\033[0m"); 
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
	char* text, nat length, 
	nat begin, nat end
) {
	printf("%s:%llu:%llu: error: %s\n", filename, begin, end, message);  
	print_index(text, length, begin, end);
	abort();
}


int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("compiler backend: error: exactly one source file must be supplied"));

	const nat min_stack_size = 16384 + 1;
	nat target_arch = no_arch;
	nat output_format = no_output;
	nat should_overwrite = false;
	nat stack_size = min_stack_size;

	memset(register_index, 255, sizeof register_index);

{ 	const char* filename = argv[1];
	nat length = 0;
	char* text = load_file(filename, &length);
	nat is_undefined[max_variable_count] = {0}; 
	nat len = 0, start = 0, arg_count = 0;
	nat args[8] = {0}; 

	for (nat var = 0, op = 0, pc = 0; pc < length; pc++) {
		if (not isspace(text[pc])) {
			if (not len) start = pc;
			len++; 
			if (pc + 1 < length) continue;
		} else if (not len) continue;
		char* word = strndup(text + start, len);
		if (not op) {
			for (op = 0; op < isa_count; op++) 
				if (not strcmp(word, operations[op])) goto process_op;
			print_error("nonexistent operation", filename, text, length, start, pc);
		}
		else if (op == def) goto define_name;
		else if (arg_count == 1 and (
			op == li or 
			op == target or 
			op == format or 
			op == stacksize or 
			op == reg)) goto parse_binary_literal;
		else if (arg_count == 0 and (op == sect or op == emit)) goto parse_binary_literal;
		for (var = var_count; var--;)
			if (not is_undefined[var] and not strcmp(word, variables[var])) goto push_argument;
		print_error("undefined variable", filename, text, length, start, pc);
	parse_binary_literal:;
		nat r = 0, s = 1;
		for (nat i = 0; i < len; i++) {
			if (word[i] == '0') s <<= 1;
			else if (word[i] == '1') { r += s; s <<= 1; }
			else if (word[i] == '_') continue;
			else print_error("invalid binary literal", filename, text, length, start, pc);
		}
		var = r;
		goto push_argument;
		define_name: var = var_count;
		variables[var] = word; 
		var_count++;
		push_argument: args[arg_count++] = var;
		process_op: 
		if (op < isa_count and arg_count < arity[op]) goto next_word;
		else if (op == def) {}
		else if (op == eoi) break;
		else if (op == del) is_undefined[args[0]] = 1;
		else if (op == reg) register_index[args[0]] = args[1];
		else if (op == overwrite) should_overwrite = 1;
		else if (op == target) target_arch = args[1];
		else if (op == format) output_format = args[1];
		else if (op == stacksize) stack_size = args[1];
		else {
			if (op == do_ or op == at) is_label[args[0]] = 1;
			if (op == la) is_label[args[1]] = 1;
			else if (op == lt or op == ge or op == ne or op == eq) is_label[args[2]] = 1;
			struct instruction new = { .op = op };
			if (op == li) new.imm = 2;
			if (op == emit) new.imm = 1;
			memcpy(new.args, args, sizeof args);
			memset(args, 0, sizeof args);
			ins[ins_count++] = new;
		}
		arg_count = 0; op = 0; next_word: len = 0;
	}}

	if (not ins_count or ins[ins_count - 1].op != halt) 
		ins[ins_count++] = (struct instruction) { .op = halt };

	if (debug) {
		print_dictionary();
		print_instructions(0);
		puts("parsing finished.");
		getchar();
	}

	for (nat i = 0; i < ins_count; i++) {
		nat pc = i;
		nat pred_count = 0;
		nat* preds = compute_predecessors(pc, &pred_count);
		nat gotos[2] = {0};
		gotos[0] = compute_successors(pc, gotos + 1);
		print_instruction(ins[i]);
		printf(" : ");	
		printf("[PC = %llu], pred:", pc);
		print_nats(preds, pred_count); putchar(32);
		printf("suc: "); 
		print_nats(gotos, 2); putchar(32);		
		putchar(10);
	}


	if (target_arch == msp430_arch and stack_size) { 
		puts("fatal error: nonzero stack size for msp430 is not permitted"); 
		abort();

	} else if (target_arch == arm64_arch and stack_size < min_stack_size) {
		puts("warning: stack size less than the minimum size for arm64");
	}

	{ nat* type = calloc(ins_count * var_count, sizeof(nat));
	nat* value = calloc(ins_count * var_count, sizeof(nat));
	nat* is_copy = calloc(ins_count * var_count, sizeof(nat));
	nat* copy_of = calloc(ins_count * var_count, sizeof(nat));

	const nat traversal_count = 2; 
	nat stack[4096] = {0};
	nat stack_count = traversal_count;

	for (nat i = 0; i < ins_count; i++)  ins[i].state = 0;          we are missing side information in the state!!!!!!!!!!!!!! 1202508251.181443 

	while (stack_count) {
		nat pc = stack[--stack_count];
		nat pred_count = 0;
		nat* preds = compute_predecessors(pc, &pred_count);

		nat gotos[2] = {0};
		gotos[0] = compute_successors(pc, gotos + 1);

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

		const nat gt0 = gotos[0];
		const nat gt1 = gotos[1];

		const nat a0 = ins[pc].args[0];
		const nat a1 = ins[pc].args[1];

		for (nat var = 0; var < var_count; var++) {

			nat future_type = 0;
			nat future_value = 0;
			nat future_is_copy = 0;
			nat future_copy_of = 0;

			nat first_ct = 1;
			nat first_cp = 1;

			const nat dd = debug;
		
			for (nat p = 0; p < pred_count; p++) {
				const nat pred = preds[p];

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
	
		const nat ct0 = 0 < arity[op] and pc < ins_count and a0 < var_count ? type[pc * var_count + a0] : 0;
		const nat ct1 = 1 < arity[op] and pc < ins_count and a1 < var_count ? type[pc * var_count + a1] : 0;
		const nat v0 = 0 < arity[op] and pc < ins_count and a0 < var_count ? value[pc * var_count + a0] : 0;
		const nat v1 = 1 < arity[op] and pc < ins_count and a1 < var_count ? value[pc * var_count + a1] : 0;
		const nat a0_is_copy = 0 < arity[op] and pc < ins_count and a0 < var_count ? is_copy[pc * var_count + a0] : 0;
		const nat a0_copy_of = 0 < arity[op] and pc < ins_count and a0 < var_count ? copy_of[pc * var_count + a0] : 0;

		nat out_t = ct0, out_v = v0;
		nat out_is_copy = a0_is_copy, out_copy_of = a0_copy_of;

		if (op == halt) continue;
		else if (op == at) { }
		else if (op == sect) { }
		else if (op == emit) { }
		else if (op == sc) { }
		else if (op == do_) { }

		else if (op == li) {
			if (register_index[a0] == (nat) -1) { out_t = 1; out_v = a1; }
		}

		else if (op == set) {
			if (register_index[a0] == (nat) -1) { out_t = ct1; out_v = v1; }
			out_is_copy = 1;
			out_copy_of = is_copy[pc * var_count + a1] ? copy_of[pc * var_count + a1] : a1;
		}

		else if (op >= add and op <= sd) {

			// todo: give error when statically detecting dividing by zero!

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
			
		} else if (op == stb or op == sth or op == stw or op == std) {

		} else if (op == ldb or op == ldh or op == ldw or op == ldd) {
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

				const nat targetn = cond ? gt1 : gt0;
				if (targetn < ins_count and ins[targetn].state < traversal_count) 
					stack[stack_count++] = targetn; 
				continue;
			}		

		} else {
			puts("WARNING: EXECUTING AN UNKNOWN INSTRUCTION WITHOUT AN IMPLEMENTATION!!!");
			puts(operations[op]);
			abort();
		}

		if (pc < ins_count and a0 < var_count) {
			type[pc * var_count + a0] = out_t;
			value[pc * var_count + a0] = out_v;
			is_copy[pc * var_count + a0] = out_is_copy;
			copy_of[pc * var_count + a0] = out_copy_of;
		}
		if (gt0 < ins_count) stack[stack_count++] = gt0; 

		if (op >= set and op <= ldd) {
			for (nat i = 0; i < var_count; i++) {
				if (	is_copy[pc * var_count + i] and 
					copy_of[pc * var_count + i] == a0
				) is_copy[pc * var_count + i] = 0;
			}
		}
	}

	if (debug) {
		debug_data_flow_state(0, NULL, 0, stack, stack_count, value, type, is_copy, copy_of);
		puts("data flow: [FINAL VALUES]");
		print_instructions(0);
		puts("OPT2 finished.");
		puts("pruning ctk instructions...");
		getchar();
	}


	for (nat i = 0; i < ins_count; i++) {

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
			op == at	or op == sect  or

			op == ldd 	or op == std or
			op == ldw 	or op == stw or
			op == ldh 	or op == sth or
			op == ldb 	or op == stb or

			op == emit
			)
		) keep = 1;

		if (not keep and ins[i].state)
		for (nat a = 0; a < arity[op]; a++) {
						
			if (((imm >> a) & 1)) continue;
			if (is_label[ins[i].args[a]]) continue;

			if (register_index[ins[i].args[a]] != (nat) -1) {
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



		for (nat a = 0; a < arity[ins[i].op]; a++) {

			if (a == 0 and (op >= set and op <= ldd)) continue;

			const nat this_arg = ins[i].args[a];
			const nat is_ct = ((ins[i].imm >> a) & 1) or type[i * var_count + this_arg];

			if (not is_ct and is_copy[i * var_count + this_arg]) {

				const nat cp = copy_of[i * var_count + this_arg];

				if (is_label[cp]) {
					if (ins[i].op == set) {
						printf("WARNING: inlining a label into a set!!\n");
						getchar();
						ins[i].args[a] = cp;
					}
				} else {				
					printf("note: inlining copy reference: a1=%llu imm=%llu copy_of=%llu, i=%llu...\n", 
						this_arg, ins[i].imm, 
						cp,
						i
					);
					puts("original:");
					print_instruction(ins[i]); puts("");	
					ins[i].args[a] = cp;
					puts("modified form:"); 
					print_instruction(ins[i]); 
					puts("");
					getchar();	
				}
			}
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


		} else if (op == ldd or op == ldw or op == ldh or op == ldb) {

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

		} else if (op == std or op == stw or op == sth or op == stb) {

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

	exit(0);
}



/*
	if (target_arch != riscv_arch) goto skip_branch_imm_replace;

	puts("replacing branch and store immediates with branch and store register instructions, for rv32...");

	for (nat i = 0; i < ins_count; i++) {

		const nat op = ins[i].op;
		const nat imm = ins[i].imm;
		const nat i0 = !!(imm & 1);
		const nat i1 = !!(imm & 2);
		//const nat a0 = ins[i].args[0];
		//const nat a1 = ins[i].args[1];

		if (not (op == lt or op == ge or op == ne or op == eq or op == st)) continue;

		if (i0) {
			const nat n = ins[i].args[0];
			if (not n and (op == lt or op == ge or op == ne or op == eq)) continue;
			variables[var_count] = strdup("NEW");
			var_count++;
			memmove(ins + i + 1, ins + i, sizeof(struct instruction) * (ins_count - i));
			ins[i] = (struct instruction) { set, 0x2, 0, { var_count - 1, n } };
			ins_count++;
			ins[i + 1].args[0] = var_count - 1;
			ins[i + 1].imm &= (nat) ~1;
			puts("rv32 replace st/lt arg0 imm: info: inserted a set statement!");
			i++;
		} 
		
		if (i1) {
			const nat n = ins[i].args[1];
			if (not n and (op == lt or op == ge or op == ne or op == eq)) continue;
			variables[var_count] = strdup("NEW");
			var_count++;
			memmove(ins + i + 1, ins + i, sizeof(struct instruction) * (ins_count - i));
			ins[i] = (struct instruction) { set, 0x2, 0, { var_count - 1, n } };
			ins_count++;
			ins[i + 1].args[1] = var_count - 1;
			ins[i + 1].imm &= (nat) ~2;
			puts("rv32 replace replace st/lt arg1 imm: info: inserted a set statement!");
			i++;
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






		//XXX

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


				if (op_A[this] == sub) { 
					puts("we need to be handling the subi "
					"case better on rv32 isel... whoops lol"); 
					abort(); 
				} 

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
		//   OP_A d k   -->   OP_B d d k
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
				else if (op == mul or op == div_ or op == rem) goto skip_op_r_i;

				else if (op == sub) {
					new = (struct instruction) { 
						r5_i, 0x15, 0, {
							0x13, 
							arg0, 
							op_B1[this], 
							arg0, 
							(-arg1) & 0xfff, 
							0, 0
						} 
					};
					goto r5_push_single_mi;

				} else { 
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
				}				
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

			// set d l ->   auipc d l[31:12] ;  addi d d l[11:0]

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

		//else if (op == add and imm and arg1 < 2048) { // add d #k -> addi d d k
		//	new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg0, arg1,0,  0,0 } };
		//	goto r5_push_single_mi;
		}

		//else if (op == sub and imm) { // sub d #k -> addi d d -k
		//	nat k = (-arg1) & 0xFFF;
		//	new = (struct instruction) { r5_i, 0x15, 0,   { 0x13, arg0, 0, arg0, k, 0,  0,0 } };
		//	goto r5_push_single_mi;
		//}

		else if (op == ld and is_label[arg1]) {  // ld d l N ->   auipc d l[31:12] ;  lwu d d l[11:0]

			nat n = 0;
			     if (arg2 == 1) n = 4;
			else if (arg2 == 2) n = 5;
			else if (arg2 == 4) n = 6;
			else if (arg2 == 8) n = 3;
			else abort();

			new = (struct instruction) { r5_u, 0x5, 0,  { 0x17, arg0, arg1, 0x42,  0,0,0,0 } };
			mi[mi_count++] = new;

			new = (struct instruction) { r5_i, 0x15, 0,   { 0x03, arg0, n, arg0, arg1,   0x42, 0,0 } };
			goto r5_push_single_mi;
		}

		else if (op == ld) {
			nat n = 0;
			     if (arg2 == 1) n = 4;
			else if (arg2 == 2) n = 5;
			else if (arg2 == 4) n = 6;
			else if (arg2 == 8) n = 3;
			else abort();

			new = (struct instruction) { r5_i, 0x15, 0,   { 0x03, arg0, n, arg1, 0x0000,  0,0,0 } };
			goto r5_push_single_mi;
		}

		else if (op == do_) {
			new = (struct instruction) { r5_j, 0x7, 0,  { 0x6f, 0, arg0, 0,0,0,0,0 } };
			goto r5_push_single_mi;
		}

		puts("error: unknown instruction selection pattern");
		print_instruction_window_around(i, 1, "unknown isel pattern for this instruction");
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
	const nat reg_mode = 0;
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

		if (	
			op == m4_op or op == adr or op == m4_br or
			op == at or op == emit or op == halt
		) { 
			new = ins[i]; 
			goto msp430_push_single_mi; 
		}


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
			
		puts("error: unknown instruction selection pattern");
		abort();

	msp430_push_single_mi:
		mi[mi_count++] = new;
		ins[i].state = 1;
	}}

	goto finish_instruction_selection;


















arm64_instruction_selection:;

	puts("arm64: instruction selection starting...");
	{ struct instruction new = {0};
	const nat unrecognized = (nat) -1;

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

			

			puts("unimplemented: error unknown instruction selection pattern!");
			abort();


		}

		if (op == set) {
			const nat b = locate_instruction(
				(struct expected_instruction){ 
					.op = si, 
					.imm = 2, 
					.use = 1, 
					.args[0] = arg0 
				}, i + 1
			);
			if (b == unrecognized) goto addsrlsl_bail;

			const nat c = locate_instruction(
				(struct expected_instruction) {
					.op = add,
					.use = 1,
					.args[0] = arg0
				}, b + 1
			);
			if (c == unrecognized) goto addsrlsl_bail;

			new = (struct instruction) { 
				a6_addr, 0xf8, 0, {
					arg0, ins[c].args[1], 
					arg1, ins[b].args[1], 
					0, 0
				} 
			};

			ins[b].state = 1; 
			ins[c].state = 1;
			goto push_arm64_ins;

		} addsrlsl_bail:

		if (op == set and not imm) {
			new = (struct instruction) { a6_orr, 0xff, 0, { arg0, 0, arg1, 0 } };
			goto push_arm64_ins;
		}

		else if (op == set and imm and arg1 < (1LLU << 16LLU)) {
			new = (struct instruction) { a6_mov, 0xfe, 0, { arg0, arg1, 0, 0, 0 } };
			goto push_arm64_ins;
		}

		if (op == lt and not imm) {
			new = (struct instruction) { .op = a6_addr, 0x00, 0, {0, arg0, arg1, 0} };
			mi[mi_count++] = new;
			new = (struct instruction) { a6_bc, 0xff, 0, { lt, 0x0000000 } };
			goto push_arm64_ins;
		}

		if (op == eq and not imm) {
			new = (struct instruction) { a6_orr, 0xff, 0, {0, arg0, arg1, 0 } };
			mi[mi_count++] = new;
			new = (struct instruction) { a6_bc, 0xff, 0, { eq, 0x0000000 } };
			goto push_arm64_ins;
 		}

		if (op == lt and imm) {			
			new = (struct instruction) { .op = a6_addi, 0x00, 0, {0, arg0, arg1, 0} };
			mi[mi_count++] = new;
			new = (struct instruction) { a6_bc, 0xff, 0, { eq, 0x0000000 } };
			goto push_arm64_ins;
		}

		if (op == eq and imm) {
			new = (struct instruction) { a6_ori, 0x00, 0, {0, arg0, arg1, 0} };
			mi[mi_count++] = new;
			new = (struct instruction) { a6_bc, 0xff, 0, { eq, 0x00000000 } };
			goto push_arm64_ins;
		}

		if (op == sc) {
			new = (struct instruction) { a6_svc, 0, 0, {0,0,0,0, 0,0,0,0 } };
			goto push_arm64_ins;
		}
		continue;
	push_arm64_ins:;
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

	if (debug) {
		puts("we just finished instruction selection!!!");
		getchar();
	}

	for (nat i = 0; i < ins_count; i++) {
		if (not ins[i].state) {
			puts("error: instruction unprocessed by ins sel: internal error");
			puts("error: this instruction failed to be lowered:\n");
			print_instruction_window_around(i, 1, "not selected instruction!");
			puts("");
			abort();
		}
	}



	if (debug) {
		puts("we just verified instruction selection!!!");
		getchar();
	}

	
	for (nat i = 0; i < mi_count; i++) ins[i] = mi[i];
	ins_count = mi_count;

	if (debug) {
		puts("finished instruction selection!");
		printf("info: preliminary machine code prior to RA: for target = %llu\n", target_arch);
		print_instructions(1);
		getchar();
	}


	if (debug) {
		puts("i just showed you instruction selection!!!");
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

	if (debug) {
		puts("selected arch for RA liveness!!");
		getchar();
	}


	{ nat* alive = calloc(ins_count * var_count, sizeof(nat)); 
	nat stack[4096] = {0};
	nat stack_count = 0;	
	for (nat i = 0; i < ins_count; i++) 
		if (ins[i].op == halt) stack[stack_count++] = i;
	for (nat i = 0; i < ins_count; i++)  ins[i].state = 0;


	if (not stack_count) {
		if (debug) {
			printf("error: no control flow graph terminations were found! this means that liveness cannot take place.\n");

			puts("warning: instead, we are just going to push the last instruction index, with the assumption that the infinite loop is at the end of the program lol.");
		getchar();
		}

		stack[stack_count++] = ins_count - 1;
	}


	while (stack_count) {

		nat pc = stack[--stack_count];

		nat pred_count = 0;  nat* preds = NULL;
		nat goto_count = 0;  nat* gotos = NULL;

		if (target_arch == rv32_arch) {
			preds = compute_riscv_predecessors(pc, &pred_count);
			gotos = compute_riscv_successors(pc);

		} else if (target_arch == arm64_arch) {
			preds = compute_arm64_predecessors(pc, &pred_count);
			gotos = compute_arm64_successors(pc);

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
				if (alive[gt * var_count + var] == 1) future_alive = 1;
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
				if (not i1) alive[pc * var_count + a1] = 2;
				if (not i3) alive[pc * var_count + a3] = 1;
				if (not i4) alive[pc * var_count + a4] = 1;
			} else if (op == r5_i) { // addi D A
				if (not i1) alive[pc * var_count + a1] = 2;
				if (not i3) alive[pc * var_count + a3] = 1;
			} else if (op == r5_s or op == r5_b) { // BLT X Y   or // STW A A
				if (not i2) alive[pc * var_count + a2] = 1;   // 0x23  0x010  arg0 arg1 0x00000 0 0 0 
				if (not i3) alive[pc * var_count + a3] = 1;
			} else if (op == r5_u or op == r5_j) { // JAL D   or  // LUI D
				if (not i1) alive[pc * var_count + a1] = 2;
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
		while (pc < ins_count and alive[pc * var_count + var] != 2) pc++;
		while (pc < ins_count and alive[pc * var_count + var] == 2) pc++;

		if (pc < ins_count and alive[pc * var_count + var] != 1) continue;

		while (pc < ins_count) {

			if (debug) {
			printf("[begin]: \tlive range var = %s: { ", variables[var]);
			for (nat i = 0; i < ins_count; i++) {
				if (i != pc) printf("  %llu  ", alive[i * var_count + var]);
				if (i == pc) printf(" [%llu] ", alive[i * var_count + var]);
			}
			puts("} ");
			} 

			nat begin = pc;

			while (pc < ins_count and alive[pc * var_count + var] != 2) pc++; 
			pc--;
			while (pc and alive[pc * var_count + var] == 0) pc--; 
			pc++;

			if (debug) { printf("[  end]: \tlive range var = %s: { ", variables[var]);
			for (nat i = 0; i < ins_count; i++) {
				if (i != pc) printf("  %llu  ", alive[i * var_count + var]);
				if (i == pc) printf(" [%llu] ", alive[i * var_count + var]);
			}
			puts("} \n");
			getchar(); }

			range_begin[range_count] = begin;
			range_end[range_count] = pc;
			range_var[range_count++] = var;

			while (pc < ins_count and alive[pc * var_count + var] != 2) pc++; 
			while (pc < ins_count and alive[pc * var_count + var] == 2) pc++;
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



	// TODO: machine opt: 
	// we have to do    ecall analysis, to see if a value contributes to ecalls, and then we can delete it, if it doesnt! 
	// we are going to do this on the final machine code, though, because it exposes many other things lol. 
	// 


	//    volatile st   ==>   execute st    execute ld            (actually, we should do this analysis on the language isa instructions!! prior to isel) 



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
				const nat n = compute_label_location(a4);
				const u32 im = calculate_offset(lengths, i - 1, n) & 0x00000FFF;
				k = im;
			}

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
			const uint32_t to_emit = (0x54U << 24U) | (offset << 5U) | (a0);
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


}




*/





















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

	const nat index_mode = 1;
	const nat deref_mode = 2;
 	const nat incr_mode = 3;


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




