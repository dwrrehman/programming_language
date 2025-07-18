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
	ld, st, emit, sect,
	at, do_, lt, eq, 
	file, del, str, eoi, 
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
	"file", "del", "str", "eoi", 
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

static struct instruction ins[max_instruction_count] = {0};
static nat ins_count = 0;

static struct instruction rt_ins[max_instruction_count] = {0};
static nat rt_ins_count = 0;

static char* variables[max_variable_count] = {0};
static nat values[max_variable_count] = {0};
static nat is_label[max_variable_count] = {0}; // if you do     at x     then x has is_label == 1.
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
		printf("[%5llu]:%c:%16s:%016llx(%lld)\t",
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
		if (i % 32 == 0) puts("");
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









/*

1202507174.221709

we need to sort out exactly how labels will work in this language. 

	first, i feel like we should just get   compiletime arguments working perfectly, 

	because really, the only difference between    labels   and pc rel offsets  

			is the branch offset calculation, which we can just expose to the programmer, to allow them to use it in anyway they see fit! 


	basically, you'll compute the appropriate pc-rel offset, 

		ie, you pretty much have fullllll control over the actual data that is stored in that immediate possition, 

			meaning that generation of the machine code doesnt even really need to deal or now about that logic. it can just emit each immediate argument  one by one    simply    
			

		and then user level code needs to handle the computations involved in taking a label, and turning it into a compiletime argument lol. 


			and for that, we are going to use the fact that 


				first of all,  we are      NOT     going to emit any   rt_at  statements. 

				instead, we are going to consider  "at"      FULLYYYY CT. 

										

				ANDDD that even simplifies the mental model about the branches, because now, we can just consider the pc rel offsets    in actual instructions! not generated instructionssss



						which 

			hm

				yeah i mean
				actually wait is  this   a dumb idea to do lol 



				because now  generated instructions don't work 


			or like, i mean, branching over a sequence of generated instructions lolll 



				ie, to calculate the branch offsets, we need to      firsttt generate the instructions??


					CRAP





	hmm 

	okay nevermind uhh


	hmmmmmm crapppp



this just got wayyy harderrr than i thought it would be loll






i feeelllllll like we mightttttt need the    rt_at   mechanism idk hmm
















*/
















int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("error: exactly one source file must be specified."));
	
	const nat min_stack_size = 16384 + 1;
	nat target_arch = no_arch;
	nat output_format = no_output;
	nat should_overwrite = false;
	nat stack_size = min_stack_size;

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
			(op == set or op == zero) and arg_count == 0 or
			(op == do_ or op == at) and arg_count == 0
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
		values[var] = (nat) -1;
		var_count++;
	push_argument: 
		args[arg_count++] = var;
	process_op:
		if (op == eoi) break;		
		if (op == str) { in_string = 1; goto next_word; } 
		else if (op < isa_count and arg_count < arity[op]) goto next_word;
		else if (op == file) {
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
			if (op == at) values[*args] = ins_count;
			else if (op == del) is_undefined[*args] = 1;
			struct instruction new = { .op = op, .imm = is_immediate };
			is_immediate = 0;
			memcpy(new.args, args, sizeof args);
			memset(args, 0, sizeof args);
			ins[ins_count++] = new;
		}
		arg_count = 0; op = 0;
		next_word: word_length = 0;
	}
	file_count--;
	if (file_count) goto process_file; }

	if (debug) {
		print_dictionary();
		print_instructions();
		puts("parsing finished.");
	}

	memset(is_undefined, 0, sizeof(nat) * var_count);

	{ nat memory[65536] = {0};

	for (nat pc = 0; pc < ins_count; pc++) {
		nat op = ins[pc].op, imm = ins[pc].imm;

		if (memory[compiler_should_debug]) {
			print_dictionary(); puts("");
			print_instruction(ins[pc]); puts("");
			for (nat i = 0; i < rt_ins_count; i++) {
				putchar(9); print_instruction(rt_ins[i]); puts("");
			}
			getchar();
		}

		nat arg0 = ins[pc].args[0];
		nat arg1 = ins[pc].args[1];
		nat arg2 = ins[pc].args[2];
		nat val0 = imm & 1 ? arg0 : values[arg0];
		nat val1 = imm & 2 ? arg1 : values[arg1];
		nat val2 = imm & 4 ? arg2 : values[arg2];

		if (op == do_ and val0 >= ins_count) {
			printf("error: [pc = %llu] cannot jump to invalid address: 0x%016llx\n", pc, val0);
			abort();
		}
		if (op >= lt and op <= eq and val2 >= ins_count) {
			printf("error: [pc = %llu] cannot jump to invalid address: 0x%016llx\n", pc, val2);
			abort();
		}

		if (op == del) {
			is_undefined[arg0] = var_count;	
			variables[var_count] = strdup("_generated_");
			is_undefined[var_count] = 0;
			var_count++;
		} else if (op == str) {
			for (nat s = 0; s < arg0; s++) {
				struct instruction new = { .op = emit, .imm = 3 };
				new.args[0] = 1;
				new.args[1] = (nat) string_list[arg1][s];
				rt_ins[rt_ins_count++] = new;
			}

		} else if (op == at) {
			values[arg0] = pc;
			if (is_undefined[arg0]) e = values[is_undefined[arg0]];
			rt_ins[rt_ins_count++] = ins[pc];

		} else if (op > eoi) {
			struct instruction new = { .op = op };
			for (nat a = 0; a < arity[op]; a++) {
				nat e = ins[pc].args[a];
				if (not ((imm >> a) & 1)) {
					if (is_undefined[e]) e = values[is_undefined[e]];
					else e = values[e];
				}
				new.args[a] = e;
			}
			rt_ins[rt_ins_count++] = new;
		}
		else if (op == set)  values[arg0]  = val1;
		else if (op == add)  values[arg0] += val1;
		else if (op == sub)  values[arg0] -= val1;
		else if (op == mul)  values[arg0] *= val1;
		else if (op == div_) values[arg0] /= val1;
		else if (op == nor)  values[arg0] = ~(val0 | val1);
		else if (op == si)   values[arg0] <<= val1;
		else if (op == sd)   values[arg0] >>= val1;
		else if (op == ld)   values[arg0] = memory[val1];
		else if (op == st)   memory[val0] = val1;
		else if (op == do_) { *memory = pc; pc = val0; }
		else if (op == lt) { if (val0 < val1) pc = val2; }
		else if (op == eq) { if (val0 == val1) pc = val2; }
		else { 
			printf("CTE: fatal internal error: "
				"unknown instruction executed: %s...\n", 
				operations[op]
			); 
			abort(); 
		}
	}

	memcpy(ins, rt_ins, ins_count * sizeof(struct instruction));
	ins_count = rt_ins_count; 

	target_arch = memory[compiler_target];
	output_format = memory[compiler_format];
	should_overwrite = memory[compiler_should_ovewrite];
	stack_size = memory[compiler_stack_size]; }

	if (target_arch == msp430_arch and stack_size) { 
		puts("fatal error: nonzero stack size for msp430 is not permitted"); 
		abort();

	} else if (target_arch == arm64_arch and stack_size < min_stack_size) {
		puts("warning: stack size less than the minimum size for arm64");
	}

	if (debug) {
		print_dictionary();
		print_instructions();
		puts("CT-EXECUTION finished.");
	}

	const char* output_filename = "output_file_from_compiler";
	if (output_format == uf2_executable) output_filename = "output_file_from_compiler.uf2";
	if (output_format == c_source) output_filename = "output_file_from_compiler.c";


}
































































































































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








