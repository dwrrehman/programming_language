// a simpler version of the language: 202406237.022917: dwrr 
// ...made a tonnnn more simpler without a ct system or arg stack, 
//  so that the isa is just the riscv isa plus the label attribution ins.
// very cool so far


/*
language isa:
===================


set d r
add d r
sub d r
mul d r
muh d r
mhs d r
div d r
dvs d r
rem d r
rms d r
and d r
or  d r
eor d r
sr  d r
srs d r
sl  d r
s d
z d
not d
lt  l r s
ge  l r s
lts l r s
ges l r s
eq  l r s
ne  l r s
ld  d p t
st  p r t
lf  f
at  l 
reg r
rdo r
env
def o
ar r
ret
obs




example code, usage of the "obs" instruction


		def local_scope

			def actual_logic ar x

				...something with x...

				ret


			def public_interface obs ar x
				actual_logic x
				ret


		ret


		set my_x 4
		public_interface my_x








*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdnoreturn.h>

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

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

enum diagnostic_type { no_message, error, warning, info, user, debug };

enum target_architecture { 
	noruntime, 
	riscv32, riscv64, 
	arm32, arm64, 
	x86_32, x86_64, 
	target_count 
};

static const char* target_spelling[target_count] = { 
	"noruntime", 
	"riscv32", "riscv64", 
	"arm32", "arm64", 
	"x86_32", "x86_64" 
};

enum output_formats { 
	print_binary, 
	elf_objectfile, elf_executable, 
	macho_objectfile, macho_executable, 
	output_format_count 
};
static const char* output_format_spelling[output_format_count] = { 
	"print_binary", 
	"elf_objectfile", "elf_executable", 
	"macho_objectfile", "macho_executable"
};

enum host_systems { host_linux, host_macos };

enum language_isa {
	_nullins, 
	set, add, sub, mul, muh, mhs,
	div_, dvs, rem, rms, and_, or_, eor, 
	sr, srs, sl, s_, z_, not_, 
	lt, ge, lts, ges, eq, ne, 
	ld, st, lf 
at  l 
reg r
rdo r
env
def o
ar r
ret
obs


	isa_count
};

static const char* spelling[isa_count] = {
        "__null_ins__", 
	"set", "add", "sub", "mul", "muh", "mhs",
	"div", "dvs", "rem", "rms", "and", "or", "eor", 
	"sr", "srs", "sl", "s", "z", "not", 
	"lt", "ge", "lts", "ges", "eq", "ne", 
	"ld", "st", "lf", "at", "reg", "env",
};


enum arm64_isa {
	arm64_mov,  arm64_addi,
	arm64_memiu, arm64_add,
	arm64_adc, arm64_csel,
	arm64_slli, arm64_srli,
	arm64_adr, arm64_blr,
	arm64_bl, arm64_bc,
	arm64_madd,
	arm64_isa_count,
};

static const char* arm64_spelling[arm64_isa_count] = {
	"arm64_mov", "arm64_addi",
	"arm64_memiu", "arm64_add",
	"arm64_adc", "arm64_csel",
	"arm64_slli", "arm64_srli",
	"arm64_adr", "arm64_blr",
	"arm64_bl", "arm64_bc",
	"arm64_madd",
};

enum compiletime_variables {
	var_nullvar,
	var_stackpointer,
	var_stacksize,
	var_argn,
	var_arg0,
	var_arg1,
	var_arg2,
	var_arg3,
	var_arg4,
	var_arg5,
	variable_count
};

static const char* variable_spelling[variable_count] = {
	"__null_var__",
	"stackpointer",
	"stacksize",
	"argn",
	"arga",
	"argb",
	"argc",
	"argd",
	"arge",
	"argf",
};

struct file {
	nat index;
	nat parent;
	nat count;
	const char* name;
	char* text;
};

struct source_location {
	nat start;
	nat count;
	nat file;
};

struct instruction { 
	nat args[4];
	struct source_location source[4];
	nat flags[4];
};

struct machine_instruction {
	nat args[16];
	nat arg_count;
	nat instructions[16];
	nat ins_count;
	nat op;
};

struct node {
	nat data_outputs[32];
	nat data_output_count;
	nat output_reg;
	nat input0;
	nat input1;
	nat op;
	nat statically_known; 
	nat output_value;
};

struct basic_block {
	nat* data_outputs;
	nat data_output_count;
	nat* data_inputs;
	nat data_input_count;
	nat* predecessors;
	nat predecessor_count;
	nat successor;
	nat dag_count;
	nat* dag;
};

static nat stack_size = 0x1000000;

static nat architecture = arm64;
static nat output_format = macho_executable;

static bool preserve_existing_object = false;
static bool preserve_existing_executable = false;

static const char* object_filename = "object0.o";
static const char* executable_filename = "executable0.out";


static nat byte_count = 0;
static u8* bytes = NULL;

static nat mi_count = 0;
static struct machine_instruction mis[4096] = {0};

static nat ins_count = 0;
static struct instruction* ins = NULL;

static nat file_count = 0;
static struct file files[4096] = {0};

static char reason[4096] = {0};

static const struct source_location undefined_location = {0};


static const char* type_string[] = {
	"(none):", 
	"\033[1;31merror:\033[0m", 
	"\033[1;35mwarning:\033[0m", 
	"\033[1;36minfo:\033[0m", 
	"\033[1;32mdata:\033[0m", 
	"\033[1;33mdebug:\033[0m"
};

static void print_dictionary(char** names, nat* locations, nat name_count) {
	puts("printing dictionary...");
	for (nat i = 0; i < name_count; i++) {
		printf("\t#%-6llu:   %-20s .....     @{%llu,%llu}\n", // %-7lld
		i, names[i], locations[2 * i], locations[2 * i + 1]);
	}
	puts("end of dictionary.");
}

static void print_instructions(char** names, nat name_count) {
	printf("instructions: {\n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t%-8llu\tins(.op = %-5s  { ", i, 
			ins[i].args[0] < isa_count 
				? spelling[ins[i].args[0]] 
				: "UNDEFINED OP ERR"
		);
		for (nat a = 1; a < 4; a++) 
			printf(" %-8s    ", 
				ins[i].args[a] < name_count 
					? names[ins[i].args[a]] 
					: "UNDEFINED OP ERR"
				);

		printf("} -- [");
		printf("todo: add me]");
		//for (nat a = 0; a < 4; a++) {
		//	printf("%llu:%llx:%llx:%llx", a, ins[i].args[a], ins[i].start[a], ins[i].count[a]);
		//	if (a < 3) printf(",");
		//}
		puts("]");
	}
	puts("}");
}

static void print_message(nat type, const char* reason_string, struct source_location location) {
	printf("\033[1mlanguage: %s:", files[location.file].name);

	if (location.start or location.count) printf("%llu:%llu:", location.start, location.count);
	printf(" %s \033[1m%s\033[m\n", type_string[type], reason_string);
	if (not files[location.file].name) return;
	if (not location.start and not location.count) goto finish;

	nat line_begin = location.start;
	while (line_begin and files[location.file].text[line_begin - 1] != 10) line_begin--;
	nat line_end = location.start;
	while (line_end < files[location.file].count and files[location.file].text[line_end] != 10) line_end++;

	printf("\n\t");
	for (nat i = line_begin; i < line_end; i++) {
		if (i == location.start) printf("\033[38;5;%llum", 178LLU);
		if (files[location.file].text[i] == 9) putchar(32);
		else putchar(files[location.file].text[i]);
		if (i == location.start + location.count) printf("\033[0m");
	}
	printf("\n\t");
	for (nat i = 0; i < location.start - line_begin; i++) 
		printf(" "); 
	printf("\033[32;1m^");
	if (location.count) {
		for (nat i = 0; i < location.count - 1; i++) 
			printf("~"); 
	}
	printf("\033[0m\n"); 
	finish: puts("");
}

static char* read_file(const char* name, nat* out_length, struct source_location location) {
	int d = open(name, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }
	const int file = open(name, O_RDONLY, 0);
	if (file < 0) { 
		read_error:;
		snprintf(reason, sizeof reason, "%s: \"%s\"", 
			strerror(errno), name);
		print_message(error, reason, location);		
		exit(1); 
	}
	size_t length = (size_t) lseek(file, 0, SEEK_END);
	char* string = malloc(length);
	lseek(file, 0, SEEK_SET);
	read(file, string, length);
	close(file); 
	*out_length = length;
	return string;
}

static void dump_hex(uint8_t* memory, nat count) {
	printf("dumping bytes: (%llu)\n", count);
	for (nat i = 0; i < count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx(%c) ", memory[i], memory[i] >= 32 ? memory[i] : ' ');
	}
	puts("");
}



static void emitb(nat x) {
	bytes = realloc(bytes, byte_count + 1);
	bytes[byte_count++] = (u8) (x >> 0);
}

static void emitw(nat x) {
	bytes = realloc(bytes, byte_count + 4);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
	bytes[byte_count++] = (u8) (x >> 16);
	bytes[byte_count++] = (u8) (x >> 24);
}




static void make_elf_object_file(const char* given_object_filename) {
	puts("make_elf_object_file: unimplemented");
	// getchar();
	const int flags = O_WRONLY | O_CREAT | O_TRUNC | O_EXCL;
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(given_object_filename, flags, mode);
	if (file < 0) { perror("obj:open"); exit(1); }
	write(file, NULL, 0);
	write(file, NULL, 0);
	write(file, NULL, 0);
	close(file);
}

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


static void execute_instructions(nat* label, nat name_count) {

	nat reg[1 << 16];
	memcpy(reg, label, sizeof(nat) * name_count);
	reg[var_stackpointer] = (nat) (void*) calloc(1 << 16, 1);
	reg[var_stacksize] = 1 << 16;

	for (nat pc = 0; pc < ins_count; ) {

		const nat op = ins[pc].args[0];
		const nat d =  ins[pc].args[1];
		const nat r =  ins[pc].args[2];
		const nat s =  ins[pc].args[3];
		
	//	printf("executing ins  @pc=%llu:  [ %s (%llu)  :: d %llu  r %llu  s %llu ]\n", 
	//		pc, spelling[op], op, d, r, s
	//	);

		     if (op == add)   reg[d] = reg[r] + reg[s];
		else if (op == sub)   reg[d] = reg[r] - reg[s];
		else if (op == or_)   reg[d] = reg[r] | reg[s];
		else if (op == eor)   reg[d] = reg[r] ^ reg[s];
		else if (op == and_)  reg[d] = reg[r] & reg[s];
		else if (op == sl)    reg[d] = reg[r] << reg[s];
		else if (op == sr)    reg[d] = reg[r] >> reg[s];
		else if (op == div_)  reg[d] = reg[r] / reg[s];
		else if (op == mul)   reg[d] = reg[r] * reg[s];
		else if (op == rem)   reg[d] = reg[r] % reg[s];

		else if (op == st) {
			     if (reg[s] == 1) { *( u8*)(reg[d]) = ( u8)reg[r]; }
			else if (reg[s] == 2) { *(u16*)(reg[d]) = (u16)reg[r]; }
			else if (reg[s] == 4) { *(u32*)(reg[d]) = (u32)reg[r]; }
			else if (reg[s] == 8) { *(nat*)(reg[d]) = (nat)reg[r]; }
			else abort();

		} else if (op == ld) {
			     if (reg[s] == 1) { reg[d] = *( u8*)(reg[r]); }
			else if (reg[s] == 2) { reg[d] = *(u16*)(reg[r]); }
			else if (reg[s] == 4) { reg[d] = *(u32*)(reg[r]); }
			else if (reg[s] == 8) { reg[d] = *(nat*)(reg[r]); }
			else abort();
		}
		else if (op == lt)   { if (reg[r] <  reg[s]) { pc = label[d]; continue; } }
		else if (op == ge)   { if (reg[r] >= reg[s]) { pc = label[d]; continue; } }
		else if (op == ne)   { if (reg[r] != reg[s]) { pc = label[d]; continue; } }
		else if (op == eq)   { if (reg[r] == reg[s]) { pc = label[d]; continue; } }

		//else if (op == dr)  { reg[r] = pc; pc = reg[d]; continue; } 
		//else if (op == do_)   { reg[r] = pc; pc = label[d]; continue; } 

		else if (op == env) {
			const nat a0 = reg[var_arg0];
			const nat a1 = reg[var_arg1];
			const nat a2 = reg[var_arg2];
			//const nat a3 = reg[var_arg3];
			//const nat a4 = reg[var_arg4];
			//const nat a5 = reg[var_arg5];
			const nat n = reg[var_argn];

			if (n == 0) {
				snprintf(reason, sizeof reason, "%lld (0x%016llx)", a0, a0);
				print_message(user, reason, ins[pc].source[0]);
			} 

			else if (n == 1) exit((int) a0);
			else if (n == 2) fork();
			else if (n == 3) read((int) a0, (void*) a1, (size_t) a2);
			else if (n == 4) write((int) a0, (void*) a1, (size_t) a2);
			else if (n == 5) open((const char*) a0, (int) a1, (int) a2);	
			else if (n == 6) close((int) a0);
			else if (n == 59) execve((char*) a0, (char**) a1, (char**) a2);
			
			else {
				snprintf(reason, sizeof reason, "unknown ct ecl number %llu", n);
				print_message(error, reason, ins[pc].source[0]);
				exit(1);
			}
		} else { 
			puts("error: internal ct execute error: unknown instruction encountered"); 
			printf("found bad instruction: %llu : %s\n", op, op < isa_count ? spelling[op] : "(NON-INS OP CODE)");
			abort(); 
		}

		reg[0] = 0;
		pc++;
	}
}





static nat arity(const nat op) { 
	if (not op) {
		printf("null ins does not have arity: internal error\n");
		abort();
	}

	if (op == env) return 0;

	if (
		op == s_ or 
		op == z_ or 
		op == not_ or 
		op == lf or 
		op == at or 
		op == reg
	) return 1;


	if (
		op == set or
		op == add or
		op == sub or
		op == mul or
		op == muh or
		op == mhs or
		op == div_ or
		op == dvs or
		op == rem or
		op == rms or
		op == and_ or
		op == or_ or
		op == eor or
		op == sr  or
		op == srs or
		op == sl
	) return 2;

	if (
		op == lt or
		op == ge or
		op == lts or
		op == ges or
		op == eq or
		op == ne or
		op == ld or
		op == st
	) return 3;

	printf("unknown arity for %llu...\n", op);
	abort();
}







static void print_nodes(struct node* nodes, nat node_count, char** names) {
	printf("printing %llu nodes...\n", node_count);
	for (nat n = 0; n < node_count; n++) {

		printf("[%s] node #%-5llu: {"
			".op=%2llu (\"\033[35;1m%-10s\033[0m\") "
			".or=%2llu (\"\033[36;1m%-10s\033[0m\") "
			".0=%2llu (\"\033[33;1m%-10s\033[0m\") "
			".1=%2llu (\"\033[33;1m%-10s\033[0m\") "
			//".0v=%2llu "
			//".1v=%2llu "
			".ov=%2llu "
			".oc=%2llu "
			".o={ ", 
			nodes[n].statically_known ? "\033[32;1mSK\033[0m" : "  ", n, 
			nodes[n].op, spelling[nodes[n].op],
			nodes[n].output_reg, names[nodes[n].output_reg],
			nodes[n].input0, "[i0]", //names[nodes[nodes[n].input0].output_reg],
			nodes[n].input1, "[i1]", //names[nodes[nodes[n].input1].output_reg],
			//nodes[n].input0_value,
			//nodes[n].input1_value,
			nodes[n].output_value,
			nodes[n].data_output_count
		);
		for (nat j = 0; j < nodes[n].data_output_count; j++) {
			printf("%llu ", nodes[n].data_outputs[j]);
		}
		puts(" } }");
		
	}
	puts("done");
}


static void print_machine_instructions(void) {
	printf("printing %llu machine instructions...\n", mi_count);
	for (nat i = 0; i < mi_count; i++) {
		printf("machine instruction {.op = %llu (\"%s\"), .args = (%llu)[%llu, %llu, %llu, %llu] }\n", 
			mis[i].op, arm64_spelling[mis[i].op],
			mis[i].arg_count, 
			mis[i].args[0],mis[i].args[1],mis[i].args[2],mis[i].args[3]
		); 
	}
	puts("[done]");
}


static void print_basic_blocks(struct basic_block* blocks, nat block_count, 
			struct node* nodes, nat node_count, char** names
) {
	puts("blocks:");
	for (nat b = 0; b < block_count; b++) {
	
		printf("block #%llu: {.count = %llu, .dag = { ", b, blocks[b].dag_count);

		for (nat d = 0; d < blocks[b].dag_count; d++) {
			printf("%llu ", blocks[b].dag[d]);
		}
		puts("}");
	}
	puts("[end of cfg]");

	puts("printing out cfg with node data: ");

	for (nat b = 0; b < block_count; b++) {
	
		printf("block #%llu:\n", b);

		for (nat d = 0; d < blocks[b].dag_count; d++) {
			printf("\tnode %llu:   %s  %llu(\"%s\") %llu %llu\n\n", blocks[b].dag[d], spelling[nodes[blocks[b].dag[d]].op], nodes[blocks[b].dag[d]].output_reg, names[nodes[blocks[b].dag[d]].output_reg], nodes[blocks[b].dag[d]].input0, nodes[blocks[b].dag[d]].input1 );
		}
		puts("}");
	}
	puts("[end of node cfg]");
}




int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("language: \033[31;1merror:\033[0m usage: ./asm <source.s>"));

	print_message(warning, "this assembler is currently a work in progress, "
				"backend is currently not fully implemented yet...", 
			undefined_location
	);


	nat name_count = 0;
	char* names[4096] = {0};
	// nat values[4096] = {0};
	nat locations[4096 * 2] = {0};

	for (nat i = 0; i < variable_count; i++) {
		//values[name_count] = (nat) 0;
		names[name_count++] = strdup(variable_spelling[i]);
	}

	
	{
		nat count = 0;
		const char* filename = argv[1];
		char* text = read_file(filename, &count, undefined_location);
		printf("read file: (length = %llu): \n<<<", count);
		fwrite(text, 1, count, stdout);
		puts(">>>");
		files[file_count++] = (struct file) {
			.index = 0,
			.count = count,
			.name = filename,
			.text = text,
		};
	}
	
	nat stack_count = 0;
	nat stack[4096] = {0};

	ins_count = (nat) -1;
{
	nat begin_at = 0;

parse_file:

	for (nat op = 0, s = 0, c = 0, a = 0, i = begin_at; i < files[stack[stack_count]].count; i++) {

		if (not isspace(files[stack[stack_count]].text[i])) {
			if (not c) s = i;  c++; 
			if (i + 1 < files[stack[stack_count]].count) continue;
		} else if (not c) continue;

		char* word = strndup(files[stack[stack_count]].text + s, c);

		nat n = 0;

		if (expecting_instruction) {

			for (n = 0; n < isa_count; n++) if (not strcmp(spelling[n], word)) goto ins;

			puts("found a forward declared user defined operation!");
			puts("going to look for it..");

			// DEFINING IT IN ADVANCEEEEE     ie, push it into the array of udf ops,   

				BUT WAIT CRAP WE DONT KNOW THE ARITYYYYYY  DANG ITTTTT    CRAPPPP




		} else {
			for (n = 0; n < name_count; n++) if (not strcmp(names[n], word)) goto arg;

			locations[2 * name_count + 0] = s;
			locations[2 * name_count + 1] = c;
			names[name_count++] = word;

			goto arg;
		}
	ins:	
		a = 0;
		ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
		ins[ins_count] = (struct instruction) { 
			.flags = {0, 0, 0, 0},
			.args  = {1, 1, 1, 1},
			.source = {undefined_location, undefined_location, undefined_location, undefined_location},
		};
		op = n;
	arg:	if (a >= arity(op) + 1) {
			snprintf(reason, sizeof reason, "excess arguments to %s", spelling[op]);
			print_message(error, reason, undefined_location);
			exit(1);
		}
		ins[ins_count].args[a] = n;
		a++;
		if (op == ms and a == 1) { 
			if (not ins_count) {
				snprintf(reason, sizeof reason, "excess arguments to %s", spelling[op]);
				print_message(error, reason, undefined_location);
				exit(1);
			}
			ins_count--;
			if (ins[ins_count].args[0] == ec) { ins_count--; break; }
			ins[ins_count].flags[0] = 1;
		}
		if (op == rn and a == 2) names[n] = strdup(""); 
		if (op == rn and a == 3) { names[ins[ins_count].args[1]] = names[n]; name_count--; ins_count--; }
		// if (op == at and a == 2) values[n] = ins_count--;
		if (op == use and a == 2) {
			ins_count--;
			struct file new = { .name = word };
			new.text = read_file(new.name, &(new.count), undefined_location);
			files[stack[stack_count]].index = i; 
			stack[++stack_count] = file_count;
			files[file_count++] = new;
			begin_at = 0;
			goto parse_file;
		}
		c = 0;
	}
	if (stack_count) {
		stack_count--;
		begin_at = files[stack[stack_count]].index;
		goto parse_file;
	}
	ins_count++; 
}

	puts("stage: checking for dictionary undefined/usage errors...");

/*	for (nat i = 0; i < ins_count; i++) {

		nat op = ins[i].args[0];
		// nat dest = ins[i].args[1];
		nat input0 = ins[i].args[2];
		nat input1 = ins[i].args[3];

		printf("checking instruction: "
			"{%s %s %s %s}\n",
			spelling[ins[i].args[0]],
			names[ins[i].args[1]],
			names[ins[i].args[2]],
			names[ins[i].args[3]]
		);

		if (not define_on_use(op, 2) ) { //and values[input0] == (nat) -1
			struct source_location s = ins[i].source[2];
			snprintf(reason, sizeof reason, "use of undefined word \"%s\"", names[input0]);
			print_message(error, reason, s);
			exit(1);
		}

		if (not define_on_use(op, 3) ) { // and values[input1] == (nat) -1
			struct source_location s = ins[i].source[3]; 
			snprintf(reason, sizeof reason, "use of undefined word \"%s\"", names[input1]);
			print_message(error, reason, s);
			exit(1);
		}

		// if (values[dest] == (nat) -1) values[dest] = 0;
	}
*/




	//puts("debug: finished parsing all input files.");


	puts("passed. finished parsing.");

	print_dictionary(names, locations, name_count);
	print_instructions(names, name_count);

	
	// constant propagation: 	static ct execution:
	// 1. form data flow dag	
	// 2. track which instructions have statically knowable inputs and outputs. (constants)

	struct node nodes[4096] = {0};
	nat node_count = 0;

	nodes[node_count++] = (struct node) { .output_reg = var_nullvar, .statically_known = 1 }; // used for any other name besides these.
	nodes[node_count++] = (struct node) { .output_reg = var_zero, .statically_known = 1 };
	nodes[node_count++] = (struct node) { .output_reg = var_ra };
	nodes[node_count++] = (struct node) { .output_reg = var_sp };
	nodes[node_count++] = (struct node) { .output_reg = var_argn };
	nodes[node_count++] = (struct node) { .output_reg = var_arg0 };
	nodes[node_count++] = (struct node) { .output_reg = var_arg1 };
	nodes[node_count++] = (struct node) { .output_reg = var_arg2 };
	nodes[node_count++] = (struct node) { .output_reg = var_arg3 };
	nodes[node_count++] = (struct node) { .output_reg = var_arg4 };
	nodes[node_count++] = (struct node) { .output_reg = var_stacksize, .statically_known = 1 };
	
	puts("stage: constructing data flow DAG...");

	struct basic_block blocks[4096] = {0};
	nat block_count = 0;

	for (nat i = 0; i < ins_count; i++) {

		printf("instruction: "
			"{%s %s %s %s}\n",
			spelling[ins[i].args[0]],
			names[ins[i].args[1]],
			names[ins[i].args[2]],
			names[ins[i].args[3]]
		);
		
		//const nat op = ins[i].args[0];
		const nat output_reg = ins[i].args[1];
		//const nat first = ins[i].args[2];
		//const nat second = ins[i].args[3];
		nat input0 = 0, input1 = 0;

		

	//push:;
		const nat statically_known = 
			output_reg == var_zero or 
			//input0 == -1 or
			//input1 == -1 or
			(nodes[input0].statically_known and 
			 nodes[input1].statically_known
			);


		if (ins[i].args[0] == at and blocks[block_count].dag_count) block_count++;

		struct basic_block* block = blocks + block_count;

		block->dag = realloc(block->dag, sizeof(nat) * (block->dag_count + 1));
		block->dag[block->dag_count++] = node_count;
		nodes[node_count++] = (struct node) {
			.data_outputs = {0},
			.data_output_count = 0,
			.input0 = input0,
			.input1 = input1,
			.op = ins[i].args[0],
			.output_reg = output_reg,
			.statically_known = statically_known,
			.output_value = 0,
		};

		if (	ins[i].args[0] == lt or 
			ins[i].args[0] == ge or 
			ins[i].args[0] == ne or 
			ins[i].args[0] == eq or 
			ins[i].args[0] == do_ or 
			ins[i].args[0] == dr
		) block_count++;
	}
 	block_count++;
	

	puts("done creating isa nodes... printing nodes:");
	print_nodes(nodes, node_count, names);

	puts("done creating basic blocks... printing cfg/dag:");
	print_basic_blocks(blocks, block_count, nodes, node_count, names);



	puts("finished the trickiest stage.");
	abort();

















	


	puts("stage: evaluating statically-known data_DAG nodes..");

	for (nat i = 0; i < node_count; i++) {

		struct node this = nodes[i];

		if (not this.statically_known) {
			// printf("not statically_known: skipping over node %llu...\n", i);

			//if (nodes[nodes[i].input0].statically_known) {
			//	nodes[i].input0_value = nodes[nodes[i].input0].output_value;
			//}

			//if (nodes[nodes[i].input1].statically_known) {
			//	nodes[i].input1_value = nodes[nodes[i].input1].output_value;
			//}

			continue;
		}


		const nat op = this.op;
		//const nat first = this.input0;
		//const nat second = this.input1;
		nat first_value = 0; //this.input0_value;
		nat second_value = 0; //this.input1_value;

		//if (this.input0) first_value = nodes[i].input0_value = nodes[this.input0].output_value;
		//if (this.input1) second_value = nodes[i].input1_value = nodes[this.input1].output_value;

		if (this.op == 0) {
			puts("found null instruction node... ignoring...");
			continue;


		} else if (this.op == add) {
			nodes[i].output_value = first_value + second_value;

		} else if (this.op == sub) {
			nodes[i].output_value = first_value - second_value;

		} else if (this.op == mul) {
			nodes[i].output_value = first_value * second_value;

		} else if (this.op == div_) {
			nodes[i].output_value = first_value / second_value;
	
		} else if (this.op == rem) {
			nodes[i].output_value = first_value % second_value;

		} else if (this.op == slt) {
			nodes[i].output_value = first_value < second_value;

		} else if (this.op == and_) {
			nodes[i].output_value = first_value % second_value;

		} else if (this.op == or_) {
			nodes[i].output_value = first_value | second_value;

		} else if (this.op == eor) {
			nodes[i].output_value = first_value ^ second_value;

		} else if (this.op == sl) {
			nodes[i].output_value = first_value << second_value;

		} else if (this.op == sr) {
			nodes[i].output_value = first_value >> second_value;

		} else if (this.op == dr) {
			puts("tried to evaluate dr, no known methods so far...");

		} else {
			printf("error: operation \"%s\": evaluation not implemented\n", spelling[op]);
			abort();
		}
	}

	puts("done evaluating and filling in all constants into rt instructions: here are the results: ");

	print_nodes(nodes, node_count, names);




	// do optimization passes here, using:   nodes[].


	

	puts("beginning backend");

	if (architecture >= target_count) abort();
	if (output_format >= output_format_count) abort();

	printf("info: building for target:\n\tarchitecture:  "
		"\033[31;1m%s\033[0m\n\toutput_format: \033[32;1m%s\033[0m.\n\n", 
		target_spelling[architecture], output_format_spelling[output_format]
	);

	

	if (architecture == noruntime) {
		puts("executing output instructions now...");
		//execute_instructions(values, name_count);
		exit(0);
	}

	puts("stage: performing instruction selection.... ");

	if (architecture == arm64) {

		mi_count = 0;

		for (nat i = 0; i < node_count; i++) {
			

			if (nodes[i].op == 0) { 
				puts("found zero op... ignoring?"); 
				continue; 

			} else if (nodes[i].op == add) {

				
				const nat i0 = nodes[i].input0;
				const nat i1 = nodes[i].input1;
				const nat or_sk = nodes[i].statically_known;
				const nat i0_sk = nodes[i0].statically_known;
				const nat i1_sk = nodes[i1].statically_known;

				printf("found add node instruction! transforming into machineinstruction arm64_add...\n");
				printf("found sk info: %llu, %llu, %llu\n", or_sk, i0_sk, i1_sk);
				

				if (or_sk) { 
					puts("not generating, because this instruction is fully statically unknown, and thus already evaluated...");

				} else if (i0_sk) {
					struct machine_instruction new = {0};
					new.op = arm64_addi;
					new.args[new.arg_count++] = nodes[i].output_reg;
					new.args[new.arg_count++] = nodes[i].input1;
					new.args[new.arg_count++] = nodes[i].input0;
					new.instructions[new.ins_count++] = i;
					mis[mi_count++] = new;

				} else if (i1_sk) {
					struct machine_instruction new = {0};
					new.op = arm64_addi;
					new.args[new.arg_count++] = nodes[i].output_reg;
					new.args[new.arg_count++] = nodes[i].input0;
					new.args[new.arg_count++] = nodes[i].input1;
					new.instructions[new.ins_count++] = i;
					mis[mi_count++] = new;
				} else {
					struct machine_instruction new = {0};
					new.op = arm64_add;
					new.args[new.arg_count++] = nodes[i].output_reg;
					new.args[new.arg_count++] = nodes[i].input0;
					new.args[new.arg_count++] = nodes[i].input1;
					new.instructions[new.ins_count++] = i;
					mis[mi_count++] = new;
				}


			} else if (0) {
			
				
			} else {
				puts("unknown op to translate into machine instruction...");
				abort();
			}

		}


	} else {
		print_message(error, "unknown target architecture specified", undefined_location);
		exit(1);
	}

	print_machine_instructions();




	puts("[done with final stage]");

	abort();

	

	if (architecture == noruntime) exit(0);

	if (output_format == print_binary) dump_hex((uint8_t*) bytes, byte_count);
	else if (output_format == elf_objectfile or output_format == elf_executable)
		make_elf_object_file(object_filename);
	else if (output_format == macho_objectfile or output_format == macho_executable) 
		make_macho_object_file();
	else {
		print_message(error, "unknown output format specified", undefined_location);
		exit(1);
	}

	if (output_format == elf_executable or output_format == macho_executable) {

		if (preserve_existing_executable and not access(executable_filename, F_OK)) {
			puts("asm: executable_file: file exists");  // TODO: use print_error();
			puts(executable_filename);
			exit(1);
		}
		
		char link_command[4096] = {0};
		snprintf(link_command, sizeof link_command, "ld -S -x "
			"-dead_strip "
			"-dead_strip_dylibs "
			"-no_eh_labels "
			"-no_uuid "
			"-no_weak_imports "
			"-no_function_starts "
			"-warn_compact_unwind "
			"-warn_unused_dylibs "
			"-fatal_warnings "
			"%s -o %s "
			"-arch arm64 "
			"-e _start "
			"-stack_size 0x%llx "
			"-platform_version macos 13.0.0 13.3 "
			"-lSystem "
			"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk ", 
			object_filename, executable_filename, stack_size
		);
		printf("executing linker command: \"%s\"...\n", link_command);
		system(link_command);

		system("otool -txvVhlL object0.o");
		system("otool -txvVhlL executable0.out");
		system("objdump object0.o -DSast --disassembler-options=no-aliases");
		system("objdump executable0.out -DSast --disassembler-options=no-aliases");
	}
}

















/*

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

static void generate_mov(nat Rd, nat op, nat im, 
			nat sf, nat oc, nat sh, 
		struct instruction this
) {  

	check(Rd, 32, 0, this);

	Rd = arm64_macos_abi[Rd];

	check(im, 1 << 12U, 1, this);
	emitw(  (sf << 31U) | 
		(oc << 29U) | 
		(op << 23U) | 
		(sh << 21U) | 
		(im <<  5U) | Rd);
}

static void generate_addi(nat Rd, nat Rn, nat im, 
			nat op, nat sf, nat sb, 
			nat st, nat sh, struct instruction this
) {  
	if (not Rd) { emitw(0xD503201F); return; }
	if (not Rn) { generate_mov(Rd, 0x25U, im, sf, 2, 0, this); return; }
	
	check(Rd, 32, 0, this);
	check(Rn, 32, 1, this);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	check(im, 1 << 12U, 2, this);

	emitw(  (sf << 31U) | 
		(sb << 30U) | 
		(st << 29U) | 
		(op << 23U) | 
		(sh << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd);
}


static void generate_memiu(nat Rt, nat Rn, nat im, nat op, nat oc, nat sf, struct instruction this) {

	check(Rt, 32, 0, this);
	check(Rn, 32, 1, this);

	Rt = arm64_macos_abi[Rt];
	Rn = arm64_macos_abi[Rn];

	check(im, 1 << 12U, 2, this);

	emitw(  (sf << 30U) | 
		(op << 24U) | 
		(oc << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rt);
}

static void generate_add(nat Rd, nat Rn, nat Rm, 
			nat op, nat im, nat sf, 
			nat st, nat sb, nat sh, struct instruction this
) {

	check(Rd, 32, 0, this);
	check(Rn, 32, 1, this);
	check(Rm, 32, 2, this);
	check(im, 32U << sf, 3, this);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];
	Rm = arm64_macos_abi[Rm];

	emitw(  (sf << 31U) |
		(sb << 30U) |
		(st << 29U) |
		(op << 24U) | 
		(sh << 21U) | 
		(Rm << 16U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd);
}

static void generate_adc(nat Rd, nat Rn, nat Rm, 
			nat op, nat o2, nat sf, 
			nat st, nat sb, struct instruction this
) {
	check(Rd, 32, 0, this);
	check(Rn, 32, 1, this);
	check(Rm, 32, 2, this);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];
	Rm = arm64_macos_abi[Rm];

	emitw(  (sf << 31U) |
		(sb << 30U) |
		(st << 29U) |
		(op << 21U) |
		(Rm << 16U) |
		(o2 << 10U) |
		(Rn <<  5U) | Rd);
}

static void generate_csel(nat Rd, nat Rn, nat Rm, 
			nat cd, nat op, nat sf, 
			nat ic, nat iv, struct instruction this) {

	check(Rd, 32, 0, this);
	check(Rn, 32, 1, this);
	check(Rm, 32, 2, this);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];
	Rm = arm64_macos_abi[Rm];

	emitw(  (sf << 31U) | 
		(iv << 30U) | 
		(op << 21U) | 
		(Rm << 16U) | 
		(cd << 12U) | 
		(ic << 10U) | 
		(Rn <<  5U) | Rd);
}

static void generate_slli(nat Rd, nat Rn, nat im, nat op, nat oc, nat sf, struct instruction this) {

	check(Rd, 32, 0, this);
	check(Rn, 32, 1, this);
	check(im, 64, 2, this);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	nat nn = sf, is = 63 - im, ir = (64 - im) % 64;
	//printf("generating ubfm using the values: \n");
	//printf("\t sf = %lld\n", sf);
	//printf("\t N = %lld\n", nn);
	//printf("\t immr = %lld\n", ir);
	//printf("\t imms = %lld\n", is);

	emitw(  (sf << 31U) | 
		(oc << 29U) | 
		(op << 23U) | 
		(nn << 22U) | 
		(ir << 16U) | 
		(is << 10U) | 
		(Rn <<  5U) | Rd);
}

static void generate_srli(nat Rd, nat Rn, nat im, nat op, nat oc, nat sf, struct instruction this) {

	check(Rd, 32, 0, this);
	check(Rn, 32, 1, this);
	check(im, 64, 2, this);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	nat nn = sf, is = 63, ir = im;
	//printf("generating ubfm using the values: \n");
	//printf("\t sf = %lld\n", sf);
	//printf("\t N = %lld\n", nn);
	//printf("\t immr = %lld\n", ir);
	//printf("\t imms = %lld\n", is);

	emitw(  (sf << 31U) | 
		(oc << 29U) | 
		(op << 23U) | 
		(nn << 22U) | 
		(ir << 16U) | 
		(is << 10U) | 
		(Rn <<  5U) | Rd);
}

static void generate_adr(nat Rd, nat op, nat oc, nat target, nat here, struct instruction this) { 

	nat im = calculate_offset(here, target, this);

	check(Rd, 32, 0, this);
	check_offset(im, 1 << 20, 1, this);

	Rd = arm64_macos_abi[Rd];

	nat lo = 0x03U & im, hi = 0x07FFFFU & (im >> 2);
	emitw(  (oc << 31U) | 
		(lo << 29U) | 
		(op << 24U) | 
		(hi <<  5U) | Rd);
}

static void generate_jalr(nat Rd, nat Rn, nat op, struct instruction this) {

	check(Rd, 32, 0, this);
	check(Rn, 32, 1, this);

	nat oc = Rd;
	if (Rd != 0 and Rd != 1) {
		print_message(error, "non return address register destination specified, but not supported yet", this.source[0]);
		exit(1);
	}

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	emitw( ((nat)(Rd == 1) << 22) | (oc << 21U) | (op << 10U) | (Rn << 5U));
}

static void generate_jal(nat Rd, nat here, nat target, struct instruction this) {  

	nat oc = Rd;
	if (Rd != 0 and Rd != 1) {
		// print_message(error, "non return address register destination specified, but not supported yet", 0, 0);
		abort();
	}

	nat im = calculate_offset(here, target, this);
	check(Rd, 32, 0, this);
	check_offset(im, 1 << 25, 1, this);
	emitw( (oc << 31U) | (0x05 << 26U) | (0x03FFFFFFU & im));
}

static void generate_bc(nat condition, nat here, nat target, struct instruction this) {
	const nat byte_offset = calculate_offset(here, target, this);
	const nat imm = byte_offset >> 2;
	check_offset(imm, 1 << 0, 0, this);
	emitw((0x54U << 24U) | ((0x0007FFFFU & imm) << 5U) | condition);
}

static void generate_branch(nat R_left, nat R_right, nat target, nat here, nat condition, struct instruction this) {
	generate_add(0, R_left, R_right, 0x0BU, 0, 1, 1, 1, 0, this);
	generate_bc(condition, here, target, this);
}

static void generate_slt(nat Rd, nat Rn, nat Rm, nat cond, struct instruction this) {
	generate_add(0, Rn, Rm, 0x0BU, 0, 1, 1, 1, 0, this);
	generate_csel(Rd, 0, 0, cond, 0x0D4U, 1, 1, 0, this);
}



static u32 generate_madd(struct argument* a, u32 op) {       // Ra Rm Rn Rd madd/umaddl
	u32 Rd = (u32) a[0].value;
	u32 Rn = (u32) a[1].value;
	u32 Rm = (u32) a[2].value;
	u32 Ra = (u32) a[3].value;
	check(Rd, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	check(Rm, 32, a[2], "register");
	check(Ra, 32, a[3], "register");
	return  (sf << 31U) |
		(op << 21U) |
		(Rm << 16U) |
		(sb << 15U) |
		(Ra << 10U) |
		(Rn <<  5U) | Rd;
}
*/
// else if (op == madd)   emit(generate_madd(a, 0xD8));
// else if (op == umadd)  emit(generate_madd(a, 0xDD));








/*static void generate_arm64_machine_code(void) {

	for (nat i = 0; i < ins_count; i++) {

		struct instruction this = ins[i];
		nat op = this.args[0];
		nat* a = this.args + 1;

		nat is_signed = false;

		//if (op == ecm) {
		//	for (nat n = 0; n < a[1]; n++) emitb(((uint8_t*)a[0])[n]);
		//}
		//else 

		if (op == ec)  emitw(0xD4000001);

		//else if (op == addi)   generate_addi(a[0], a[1], a[2], 0x22U, 1, 0, 0, 0, this);
		//else if (op == slli)   generate_slli(a[0], a[1], a[2], 0x26U, 2, 1, this);
		//else if (op == srli)   generate_srli(a[0], a[1], a[2], 0x26U, 2, 1, this);
		//else if (op == srai)   generate_srli(a[0], a[1], a[2], 0x26U, 0, 1, this);

		else if (op == div_)   generate_adc(a[0], a[1], a[2], 0xD6, 2, 1, 0, 0, this);
		else if (op == div_ and is_signed)   generate_adc(a[0], a[1], a[2], 0xD6, 3, 1, 0, 0, this);
		else if (op == muh)   generate_adc(a[0], a[1], a[2], 0xDE, 31, 1, 0, 0, this);

		// else if (op == mul)    generate_umaddl(a[0], a[1], a[2], 0xDE, 31, 1, 0, 0, this);
		// else if (op == lpa)  generate_adr(a[0], 0x10, 0, a[1], i, this);

		else if (op == add)    generate_add(a[0], a[1], a[2], 0x0BU, 0, 1, 0, 0, 0, this);
		else if (op == or_)    generate_add(a[0], a[1], a[2], 0x2AU, 0, 1, 0, 0, 0, this);
		else if (op == sub)    generate_add(a[0], a[1], a[2], 0x0BU, 0, 1, 0, 1, 0, this);
		else if (op == eor)    generate_add(a[0], a[1], a[2], 0x0AU, 0, 1, 0, 1, 0, this);
		else if (op == and_)   generate_add(a[0], a[1], a[2], 0x0AU, 0, 1, 0, 0, 0, this);

		else if (op == slt)    generate_slt(a[0], a[1], a[2], 2, this);
		else if (op == slt and is_signed)   generate_slt(a[0], a[1], a[2], 10, this);

		else if (op == eq)    generate_branch(a[0], a[1], a[2], i, 0, this);
		else if (op == ne)    generate_branch(a[0], a[1], a[2], i, 1, this);
		else if (op == ge)    generate_branch(a[0], a[1], a[2], i, 2, this);
		else if (op == lt)    generate_branch(a[0], a[1], a[2], i, 3, this);

		else if (op == dr)   generate_jalr(a[0], a[1], 0x3587C0U, this);
		else if (op == do_)    generate_jal(a[0], i, a[1], this);

		else if (op == sl)    generate_adc(a[0], a[1], a[2], 0x0D6U, 0x08, 1, 0, 0, this);
		else if (op == sr)    generate_adc(a[0], a[1], a[2], 0x0D6U, 0x09, 1, 0, 0, this);
		else if (op == sr and is_signed)    generate_adc(a[0], a[1], a[2], 0x0D6U, 0x0A, 1, 0, 0, this);

		else if (op == lb)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == lh)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == lw)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == ld)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);

		else if (op == sb)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == sh)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == sw)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == sd)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);

		//else if (op == sltis)  goto here;
		//else if (op == slti)   goto here;
		//else if (op == eori)   goto here;
		//else if (op == iori)   goto here;
		//else if (op == andi)   goto here;
		//else if (op == srli)   goto here;

		else {
			snprintf(reason, sizeof reason, "arm64: unknown runtime instruction: \"%s\" (%llu)\n", spelling[op], op);
			print_message(error, reason, undefined_location);
			exit(1);
		}
	}
}*/





	/*if (not op or not define_on_use(op, a)) {
			snprintf(reason, sizeof reason, "use of undefined word \"%s\"", word);
			print_message(error, reason, s, c);
			exit(1);
		}*/


	/*
		code will create constants starting from the   pc   and   zero    because those two things are statically known. 
	*/










/*static bool define_on_use(nat op, nat count) {
	if (not op) {
		printf("zero op does not have arguments\n");
		abort();
	}
	if (op == do_) 	return true;
	if (op == dr or op == rn) return count == 2;
	if (op < isa_count) return count == 1;
	printf("unknown defineonuse semantics for %llu...\n", op);
	abort();
}





example code for namespaces and functions:



	lf math.s  

	ns graphics

		set default_fps 25

		do display ar fps

			set current_fps fps
			...
			...code here...
			...  
		eo
	ens
	

	use math
	set x graphics default_fps
	sqrt x
	graphics display x 
























grouped by arity:




0 argument:

env
ens
ret

1 argument:

inc d
rst d
not d
lf  f
at  l 
reg r
rdo r
use n
ns n 
do o
ar r

2 arguments:

set d r
add d r
sub d r
mul d r
mh  d r
mhs d r
div d r
dvs d r
rem d r
rms d r
and d r
or  d r
eor d r
sr  d r
srs d r
sl  d r

3 arguments:

lt  l r s
ge  l r s
lts l r s
ges l r s
eq  l r s
ne  l r s
ld  d p t
st  p r t





*/




