// a simpler version of the language: 202406237.022917: dwrr 
// ...made a tonnnn more simpler without a ct system or arg stack, 
//  so that the isa is just the riscv isa plus the label attribution ins.
// very cool so far

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

static u32 arm64_macos_abi[] = {        // note:  x9 is call-clobbered. save it before calls. 
	31,30,31,13,14,15, 7,17,
	29, 9, 0, 1, 2, 3, 4, 5,
	 6,16,19,20,21,22,23,24,
	25,26,27,28,12, 8,11,10,    
};

enum language_isa {
	null_instruction, ecl, att, use, nsb, nse, 
	ldb, ldh, ldw, ldd, stb, sth, stw, std, jalr, jal, 
	add, sub, slt, slts, and_, ior, eor, sll, srl, sra, mul, muh, 
	muhs, div_, divs, rem, rems, blt, blts, bge, bges, bne, beq, 
	isa_count
};

static const char* spelling[isa_count] = {
	"", "ecl", "att", "use", "nsb", "nse",
	"ldb", "ldh", "ldw", "ldd", "stb", "sth", "stw", "std", "jalr", "jal",
	"add", "sub", "slt", "slts", "and", "ior", "eor", "sll", "srl", "sra", "mul", "muh", 
	"muhs", "div", "divs", "rem", "rems", "blt", "blts", "bge", "bges", "bne", "beq", 
};

enum compiletime_variables {
	var_zero,
	var_ra,
	var_sp,
	var_argn,
	var_arg0,
	var_arg1,
	var_arg2,
	var_arg3,
	var_arg4,
	var_arg5,
	var_stacksize,
	variable_count	
};

static const char* variable_spelling[variable_count] = {
	"zero",
	"ra",
	"sp",
	"argn",
	"arga",
	"argb",
	"argc",
	"argd",
	"arge",
	"argf",
	"stacksize",
};

struct instruction { 
	nat a[4];
	nat start[4];
	nat count[4];
};

struct file {
	nat index;
	nat count;
	const char* name;
	char* text;
};

struct namespace {
	char* name;
	char** names;
	nat* locations;
	nat* index;
	nat name_count;
};

static char reason[4096] = {0};

static nat stack_size = 0x1000000;
static nat architecture = arm64;
static nat output_format = macho_executable;
static bool preserve_existing_object = false;
static bool preserve_existing_executable = false;
static const char* object_filename = "object0.o";
static const char* executable_filename = "executable0.out";

static nat byte_count = 0;
static u8* bytes = NULL;

static nat ins_count = 0;
static struct instruction* ins = NULL;

static nat text_length = 0;
static char* text = NULL;
static const char* filename = NULL;

enum diagnostic_type { no_message, error, warning, info, user, debug };

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
		printf("\t#%-6llu:   %-20s ..... %-7llu\n", i, names[i], locations[i]);
	}
	puts("end of dictionary.");
}

static void print_instructions(char** names, nat name_count) {
	printf("instructions: {\n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t%-8llu\tins(.op = %-8s  { ", i, 
			ins[i].a[0] < isa_count 
				? spelling[ins[i].a[0]] 
				: "UNDEFINED OP ERR"
		);
		for (nat a = 1; a < 4; a++) 
			printf(" %-15s    ", 
				ins[i].a[a] < name_count 
					? names[ins[i].a[a]] 
					: "UNDEFINED OP ERR"
				);

		printf("} -- [");
		for (nat a = 0; a < 4; a++) {
			printf("%llu:%llx:%llx:%llx", a, ins[i].a[a], ins[i].start[a], ins[i].count[a]);
			if (a < 3) printf(",");
		}
		puts("]\n");
	}
	puts("}");
}

static void print_namespaces(nat* activens, nat activens_count, struct namespace* ns, nat ns_count) {
	printf("active ns = {");
	for (nat i = 0; i < activens_count; i++) {
		printf("%llu ", activens[i]);
	}
	puts("}");

	for (nat n = 0; n < ns_count; n++) {
		printf("ns #%llu :: .name = \"%s\" => {\n", n, ns[n].name);
		for (nat i = 0; i < ns[n].name_count; i++) {
			printf("\t.  (     %-15s   : l%lld : i%lld)\n", 
				ns[n].names[i], ns[n].locations[i], ns[n].index[i]);
		}
		puts("}");
		puts("");
	}
	puts("...end.");
}


static void print_message(nat type, const char* reason_string, nat spot, nat error_length) {
	printf("\033[1mlanguage: %s:", filename);
	if (spot or error_length) printf("%llu:%llu:", spot, error_length);
	printf(" %s \033[1m%s\033[m\n", type_string[type], reason_string);
	if (not filename) return;
	if (not spot and not error_length) goto finish;

	nat line_begin = spot;
	while (line_begin and text[line_begin - 1] != 10) line_begin--;
	nat line_end = spot;
	while (line_end < text_length and text[line_end] != 10) line_end++;

	printf("\n\t");
	for (nat i = line_begin; i < line_end; i++) {
		if (i == spot) printf("\033[38;5;%llum", 178LLU);
		if (text[i] == 9) putchar(32);
		else putchar(text[i]);
		if (i == spot + error_length) printf("\033[0m");
	}
	printf("\n\t");
	for (nat i = 0; i < spot - line_begin; i++) 
		printf(" "); 
	printf("\033[32;1m^");
	if (error_length) {
		for (nat i = 0; i < error_length - 1; i++) 
			printf("~"); 
	}
	printf("\033[0m\n"); 
	finish: puts("");
}

static char* read_file(const char* name, nat* out_length, nat start, nat count) {
	int d = open(name, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }
	const int file = open(name, O_RDONLY, 0);
	if (file < 0) { 
		read_error:;
		snprintf(reason, sizeof reason, "%s: \"%s\"", 
			strerror(errno), name);
		print_message(error, reason, start, count);		
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

static void check(nat value, nat limit, nat a, struct instruction this) {
	if (value >= limit) {
		puts("check error");
		snprintf(reason, sizeof reason, "check: value %llu >= limit %llu check did not succeed for instruction", value, limit);
		print_message(error, reason, this.start[a + 1], this.count[a + 1]);
		exit(1);
	}
}

static void check_offset(nat value, nat limit, nat a, struct instruction this) {
	if ((0)) {
		puts("check_offset error");
		print_message(error, "sorry bad logic or something", this.start[0], this.count[0]);
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
		if (	op == slt or  op == slts or 
			op == blt or  op == bge or 
			op == bne or  op == beq or 
			op == blts or op == bges
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
						this.start[0], this.count[0]);
				exit(1);
			}
			offset -= ins_size(ins[i].a[0]);
		}
	} else {
		printf("forwards branch...\n");
		for (nat i = here; i < label; i++) {
			if (i >= ins_count) {
				print_message(error, "invalid label given to a branching instruction", 
						this.start[0], this.count[0]);
				exit(1);
			}
			offset += ins_size(ins[i].a[0]);
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

	for (nat i = 0; i < ins_count; i++) {

		struct instruction this = ins[i];
		nat op = this.a[0];
		nat* a = this.a + 1;

		if (op == ecl)   emitw(0x00000073);

		else if (op == add)     r_type(a, 0x33, 0x0, 0x00, this);
		else if (op == sub)     r_type(a, 0x33, 0x0, 0x20, this);
		else if (op == sll)     r_type(a, 0x33, 0x1, 0x00, this);
		else if (op == slts)    r_type(a, 0x33, 0x2, 0x00, this);
		else if (op == slt)     r_type(a, 0x33, 0x3, 0x00, this);
		else if (op == eor)     r_type(a, 0x33, 0x4, 0x00, this);
		else if (op == srl)     r_type(a, 0x33, 0x5, 0x00, this);
		else if (op == sra)     r_type(a, 0x33, 0x5, 0x20, this);
		else if (op == ior)     r_type(a, 0x33, 0x6, 0x00, this);
		else if (op == and_)    r_type(a, 0x33, 0x7, 0x00, this);

		else if (op == ldb)     i_type(a, 0x03, 0x0, this);
		else if (op == ldh)     i_type(a, 0x03, 0x1, this);
		else if (op == ldw)     i_type(a, 0x03, 0x2, this);
		else if (op == ldd)     i_type(a, 0x03, 0x3, this);
		//else if (op == addi)    i_type(a, 0x13, 0x0, this);
		//else if (op == sltis)   i_type(a, 0x13, 0x2, this);
		//else if (op == slti)    i_type(a, 0x13, 0x3, this);
		//else if (op == eori)    i_type(a, 0x13, 0x4, this);
		//else if (op == iori)    i_type(a, 0x13, 0x6, this);
		//else if (op == andi)    i_type(a, 0x13, 0x7, this);
		//else if (op == slli)    i_type(a, 0x13, 0x1, this);
		//else if (op == srli)    i_type(a, 0x13, 0x5, this);
		//else if (op == srai)    i_type(a, 0x13, 0x5, this);
		else if (op == jalr)    i_type(a, 0x67, 0x0, this);

		else if (op == stb)      s_type(a, 0x23, 0x0, this);
		else if (op == sth)      s_type(a, 0x23, 0x1, this);
		else if (op == stw)      s_type(a, 0x23, 0x2, this);
		else if (op == std)      s_type(a, 0x23, 0x3, this);

		// else if (op == lui)  u_type(a, 0x37, this);
		// else if (op == lpa)   u_type(a, 0x17, this);

		else if (op == jal)     j_type(i, a, 0x6F, this);

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
		print_message(error, "non return address register destination specified, but not supported yet", 0, 0);
		exit(1);
	}

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	emitw( ((nat)(Rd == 1) << 22) | (oc << 21U) | (op << 10U) | (Rn << 5U));
}

static void generate_jal(nat Rd, nat here, nat target, struct instruction this) {  

	nat oc = Rd;
	if (Rd != 0 and Rd != 1) {
		print_message(error, "non return address register destination specified, but not supported yet", 0, 0);
		exit(1);
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


/*
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








static void generate_arm64_machine_code(void) {

	for (nat i = 0; i < ins_count; i++) {

		struct instruction this = ins[i];
		nat op = this.a[0];
		nat* a = this.a + 1;

		//if (op == ecm) {
		//	for (nat n = 0; n < a[1]; n++) emitb(((uint8_t*)a[0])[n]);
		//}
		//else 

		if (op == ecl)  emitw(0xD4000001);

		//else if (op == addi)   generate_addi(a[0], a[1], a[2], 0x22U, 1, 0, 0, 0, this);
		//else if (op == slli)   generate_slli(a[0], a[1], a[2], 0x26U, 2, 1, this);
		//else if (op == srli)   generate_srli(a[0], a[1], a[2], 0x26U, 2, 1, this);
		//else if (op == srai)   generate_srli(a[0], a[1], a[2], 0x26U, 0, 1, this);

		else if (op == div_)   generate_adc(a[0], a[1], a[2], 0xD6, 2, 1, 0, 0, this);
		else if (op == divs)   generate_adc(a[0], a[1], a[2], 0xD6, 3, 1, 0, 0, this);
		else if (op == muh)   generate_adc(a[0], a[1], a[2], 0xDE, 31, 1, 0, 0, this);

		// else if (op == mul)    generate_umaddl(a[0], a[1], a[2], 0xDE, 31, 1, 0, 0, this);
		// else if (op == lpa)  generate_adr(a[0], 0x10, 0, a[1], i, this);

		else if (op == add)    generate_add(a[0], a[1], a[2], 0x0BU, 0, 1, 0, 0, 0, this);
		else if (op == ior)    generate_add(a[0], a[1], a[2], 0x2AU, 0, 1, 0, 0, 0, this);
		else if (op == sub)    generate_add(a[0], a[1], a[2], 0x0BU, 0, 1, 0, 1, 0, this);
		else if (op == eor)    generate_add(a[0], a[1], a[2], 0x0AU, 0, 1, 0, 1, 0, this);
		else if (op == and_)   generate_add(a[0], a[1], a[2], 0x0AU, 0, 1, 0, 0, 0, this);

		else if (op == slt)    generate_slt(a[0], a[1], a[2], 2, this);
		else if (op == slts)   generate_slt(a[0], a[1], a[2], 10, this);

		else if (op == beq)    generate_branch(a[0], a[1], a[2], i, 0, this);
		else if (op == bne)    generate_branch(a[0], a[1], a[2], i, 1, this);
		else if (op == bge)    generate_branch(a[0], a[1], a[2], i, 2, this);
		else if (op == blt)    generate_branch(a[0], a[1], a[2], i, 3, this);

		else if (op == jalr)   generate_jalr(a[0], a[1], 0x3587C0U, this);
		else if (op == jal)    generate_jal(a[0], i, a[1], this);

		else if (op == sll)    generate_adc(a[0], a[1], a[2], 0x0D6U, 0x08, 1, 0, 0, this);
		else if (op == srl)    generate_adc(a[0], a[1], a[2], 0x0D6U, 0x09, 1, 0, 0, this);
		else if (op == sra)    generate_adc(a[0], a[1], a[2], 0x0D6U, 0x0A, 1, 0, 0, this);

		else if (op == ldb)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == ldh)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == ldw)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == ldd)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);

		else if (op == stb)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == sth)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == stw)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);
		else if (op == std)    generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0, this);

		//else if (op == sltis)  goto here;
		//else if (op == slti)   goto here;
		//else if (op == eori)   goto here;
		//else if (op == iori)   goto here;
		//else if (op == andi)   goto here;
		//else if (op == srli)   goto here;

		else {
			snprintf(reason, sizeof reason, "arm64: unknown runtime instruction: \"%s\" (%llu)\n", spelling[op], op);
			print_message(error, reason, this.start[0], this.count[0]);
			exit(1);
		}
	}
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
	reg[var_sp] = (nat) (void*) calloc(1 << 16, 1);

	for (nat pc = 0; pc < ins_count; ) {

		const nat op = ins[pc].a[0];
		const nat d =  ins[pc].a[1];
		const nat r =  ins[pc].a[2];
		const nat s =  ins[pc].a[3];
		
	//	printf("executing ins  @pc=%llu:  [ %s (%llu)  :: d %llu  r %llu  s %llu ]\n", 
	//		pc, spelling[op], op, d, r, s
	//	);

		     if (op == add)   reg[d] = reg[r] + reg[s];
		else if (op == sub)   reg[d] = reg[r] - reg[s];
		else if (op == ior)   reg[d] = reg[r] | reg[s];
		else if (op == eor)   reg[d] = reg[r] ^ reg[s];
		else if (op == and_)  reg[d] = reg[r] & reg[s];
		else if (op == slt)   reg[d] = reg[r] < reg[s];
		else if (op == slts)  reg[d] = reg[r] < reg[s];
		else if (op == sll)   reg[d] = reg[r] << reg[s];
		else if (op == srl)   reg[d] = reg[r] >> reg[s];
		else if (op == sra)   reg[d] = reg[r] >> reg[s];
		else if (op == div_)  reg[d] = reg[r] / reg[s];
		else if (op == mul)   reg[d] = reg[r] * reg[s];
		else if (op == rem)   reg[d] = reg[r] % reg[s];

		else if (op == ldb)   reg[d] = *( u8*)(reg[r]);
		else if (op == ldh)   reg[d] = *(u16*)(reg[r]);
		else if (op == ldw)   reg[d] = *(u32*)(reg[r]);
		else if (op == ldd)   reg[d] = *(nat*)(reg[r]);

		else if (op == stb)   *( u8*)(reg[r]) = ( u8)reg[d];
		else if (op == sth)   *(u16*)(reg[r]) = (u16)reg[d];
		else if (op == stw)   *(u32*)(reg[r]) = (u32)reg[d];
		else if (op == std)   *(nat*)(reg[r]) = (nat)reg[d];

		else if (op == blt)   { if (reg[r] <  reg[s]) { pc = label[d]; continue; } }
		else if (op == bge)   { if (reg[r] >= reg[s]) { pc = label[d]; continue; } }
		else if (op == bne)   { if (reg[r] != reg[s]) { pc = label[d]; continue; } }
		else if (op == beq)   { if (reg[r] == reg[s]) { pc = label[d]; continue; } }
		else if (op == blts)  { if (reg[r] <  reg[s]) { pc = label[d]; continue; } }
		else if (op == bges)  { if (reg[r] >= reg[s]) { pc = label[d]; continue; } }

		else if (op == jalr)  { reg[r] = pc; pc = reg[d]; continue; } 
		else if (op == jal)   { reg[r] = pc; pc = label[d]; continue; } 

		else if (op == ecl) {
			const nat a0 = reg[var_arg0];
			const nat a1 = reg[var_arg1];
			const nat a2 = reg[var_arg2];
			//const nat a3 = reg[var_arg3];
			//const nat a4 = reg[var_arg4];
			//const nat a5 = reg[var_arg5];
			const nat n = reg[var_argn];

			if (n == 0) {
				snprintf(reason, sizeof reason, "%lld (0x%016llx)", a0, a0);
				print_message(user, reason, ins[pc].start[0], ins[pc].count[0]);
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
				print_message(error, reason, ins[pc].start[0], ins[pc].count[0]);
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
	if (not op) abort();
	if (op == ecl or op == nse) return 1; 
	if (op == nsb or op == att or op == use) return 2;
	if (op < add) return 3; 
	if (op < isa_count) return 4; 
	abort();
}

static bool define_on_use(nat op, nat count) {
	if (not op) abort();
	if (op == jal) 	return true;
	if (op >= stb and op <= std) return false;
	if (op == jalr) return count == 2;
	if (op < isa_count) return count == 1;
	abort();
}

static char* generate_global_qualified_name(struct namespace* ns, nat* activens, nat activens_count, char* word) {
	char buffer[4096] = {0};
	for (nat i = 1; i < activens_count; i++) {
		strlcat(buffer, ns[activens[i]].name, sizeof buffer);
		strlcat(buffer, ".", sizeof buffer);
	}
	strlcat(buffer, word, sizeof buffer);
	return strdup(buffer);
}

int main(int argc, const char** argv) {

	if (argc != 2) exit(puts("language: \033[31;1merror:\033[0m usage: ./asm <source.s>"));

	nat ns_count = 0;
	struct namespace* ns = calloc(1, sizeof(struct namespace));
	
	nat activens_count = 0;
	nat activens[4096] = {0};

	nat dictionary_count = 0;
	char* dictionary[4096] = {0};
	nat locations[4096] = {0};
	
	nat file_count = 0;
	struct file files[4096] = {0};
	
	for (nat i = 0; i < variable_count; i++) {
		nat ni = ns_count;
		char* word = strdup(variable_spelling[i]);
		ns[ni].name = strdup("");
		ns[ni].names = realloc(ns[ni].names, sizeof(char*) * (ns[ni].name_count + 1)); 
		ns[ni].names[ns[ni].name_count] = word;
		ns[ni].locations = realloc(ns[ni].locations, sizeof(char*) * (ns[ni].name_count + 1)); 
		ns[ni].locations[ns[ni].name_count] = 0;
		ns[ni].index = realloc(ns[ni].index, sizeof(char*) * (ns[ni].name_count + 1)); 
		ns[ni].index[ns[ni].name_count++] = dictionary_count;

		locations[dictionary_count] = 0;
		dictionary[dictionary_count++] = word;
	}

	activens[activens_count++] = ns_count++;

	text_length = 0;
	filename = argv[1];
	text = read_file(filename, &text_length, 0, 0);
	printf("read file: (length = %llu): \n<<<", text_length);
	fwrite(text, 1, text_length, stdout);
	puts(">>>");

	nat qualifier = 0, index = 0;
	ins_count = (nat) -1;
parse_file:
	for (nat op = 0, s = 0, c = 0, a = 0, i = index; i < text_length; i++) {
		if (not isspace(text[i])) {
			if (not c) s = i;  c++; 
			if (i + 1 < text_length) continue;
		} else if (not c) continue;
		char* word = strndup(text + s, c);

		nat* location = NULL;
		nat n = 0;
		for (n = 0; n < isa_count; n++) if (not strcmp(spelling[n], word)) goto ins;

		if (not qualifier) { 
			for (nat e = activens_count; e--;) {
				struct namespace this = ns[activens[e]];
				for (n = 0; n < this.name_count; n++) {
					if (not strcmp(this.names[n], word)) {
						location = this.locations + n;
						n = this.index[n];
						goto arg;
					}
				}
			}
		} else {
			struct namespace this = ns[qualifier];  qualifier = 0;
			for (n = 0; n < this.name_count; n++) {
				if (not strcmp(this.names[n], word)) {
					location = this.locations + n;
					n = this.index[n];
					goto arg;
				}
			}
		}

		if (not define_on_use(op, a)) {
			snprintf(reason, sizeof reason, "use of undefined word \"%s\"", word);
			print_message(error, reason, s, c);
			exit(1);
		}

		const nat e = activens[activens_count - 1];
		struct namespace* this = ns + e;
		const nat nc = this->name_count;
		n = dictionary_count;

		this->names = realloc(this->names, sizeof(char*) * (nc + 1)); 
		this->names[nc] = word;

		this->locations = realloc(this->locations, sizeof(char*) * (nc + 1)); 
		this->locations[nc] = 0;
		
		this->index = realloc(this->index, sizeof(char*) * (nc + 1)); 
		this->index[this->name_count++] = n;

		locations[dictionary_count] = 0;
		dictionary[dictionary_count++] = generate_global_qualified_name(ns, activens, activens_count, word);

		location = this->locations + nc; 
		goto arg;

	ins:	
		if (op == nse and a == 1) {
			if (not activens_count) { puts("ans error"); abort(); }
			ins_count--; activens_count--;
		}
		if (op == nsb and a == 1) {
			ins_count--; 
			ns = realloc(ns, sizeof(struct namespace) * (ns_count + 1));
			ns[ns_count] = (struct namespace) {.name = strdup("")};
			activens[activens_count++] = ns_count++;
		}
		
		ins_count++; a = 0;
		ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
		ins[ins_count] = (struct instruction) { .start = {s, s, s, s}, .count = {c, c, c, c} };
		op = n;
	arg: 	if (a >= arity(op)) {
			snprintf(reason, sizeof reason, "excess arguments to %s instruction", spelling[op]);
			print_message(error, reason, s, c);
			exit(1);
		}

		if (location and (int64_t) *location < 0) { qualifier = -*location; c = 0; continue; }
		
		ins[ins_count].a[a] = n;
		ins[ins_count].start[a] = s;
		ins[ins_count].count[a] = c;
		a++;

		if (op == att and a == 2) locations[n] = ins_count--;
		if (op == nsb and a == 2) { 
			*location = -ns_count;
			ins_count--; 
			ns = realloc(ns, sizeof(struct namespace) * (ns_count + 1));
			ns[ns_count] = (struct namespace) {.name = word};
			activens[activens_count++] = ns_count++;
		}
		if (op == use and a == 2) {
			ins_count--;
			files[file_count++] = (struct file) {
				.index = i,
				.count = text_length,
				.name = filename,
				.text = text,
			};
			text_length = 0;
			filename = word;
			text = read_file(filename, &text_length, s, c);
			index = 0;
			goto parse_file;
		}
		c = 0;
	}
	if (ns_count != 1) {
			snprintf(reason, sizeof reason, "unbalanced namespace delimiters, ns_count = %llu", ns_count);
			print_message(error, reason, 0, 0);
			exit(1);
	}
	if (file_count) {
		struct file this = files[--file_count];
		text = this.text;
		filename = this.name;
		text_length = this.count;
		index = this.index;
		goto parse_file;
	}
	ins_count++;

	puts("debug: finished parsing all input files.");
	print_namespaces(activens, activens_count, ns, ns_count);
	print_dictionary(dictionary, locations, dictionary_count);
	print_instructions(dictionary, dictionary_count);
	print_message(warning, "this assembler is currently a work in progress, optimization phase not implemented yet...", 0, 0);



	/*
		code will create constants starting from the   pc   and   zero    because those two things are statically known. 
	*/



	// constant propagation: 	static ct execution:
	// 1. form data flow dag	
	// 2. track which instructions have statically knowable inputs and outputs. (constants)



	struct node {
		
		nat data_inputs[4];
		nat data_output[32];
		nat data_input_count;
		nat data_output_count;
	};

	struct node nodes[4096] = {0};

	

	for (nat i = 0; i < ins_count; i++) {
		printf("instruction: "
			"{%s %s %s %s}\n",
			spelling[ins[i].a[0]],
			dictionary[ins[i].a[1]],
			dictionary[ins[i].a[2]],
			dictionary[ins[i].a[3]]
		);

		
		
	}








	//abort();



	if (architecture >= target_count) abort();
	if (output_format >= output_format_count) abort();
	printf("info: building for target:\n\tarchitecture:  "
		"\033[31;1m%s\033[0m\n\toutput_format: \033[32;1m%s\033[0m.\n\n", 
		target_spelling[architecture], output_format_spelling[output_format]
	);





	puts("executing now...");

	if (architecture == noruntime) execute_instructions(locations, dictionary_count);
	else if (architecture == riscv32 or architecture == riscv64) generate_riscv_machine_code();
	else if (architecture == arm64) generate_arm64_machine_code();
	else {
		print_message(error, "unknown target architecture specified", ins[0].start[0], ins[0].count[0]);
		exit(1);
	}


	if (architecture == noruntime) exit(0);

	if (output_format == print_binary) dump_hex((uint8_t*) bytes, byte_count);
	else if (output_format == elf_objectfile or output_format == elf_executable)
		make_elf_object_file(object_filename);
	else if (output_format == macho_objectfile or output_format == macho_executable) 
		make_macho_object_file();
	else {
		print_message(error, "unknown output format specified", ins[0].start[0], ins[0].count[0]);
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









































































	//files[file_count].name = argv[1];
	//files[file_count].count = text_length;
	//files[file_count].text = text;
	//files[file_count].index = 0;



// print_source_instruction_mappings();














































































// else if (op == stl)   memcpy((uint8_t*) array[a0.value], text + array[a1.value], array[a2.value]);





/*	else if (n < isa_count) {
			snprintf(reason, sizeof reason, "invalid use of op code \"%s\" as argument", word);
			print_message(error, reason, s, c);
			exit(1);
		}



			else ins[ins_count] = (struct instruction) {
				.a = {jal, isa_count + var_ra, n},
				.start = {s, s, s},
				.count = {c, c, c}
			}; 









	stb/h/w/d source source     				<----- hmmmmmmm danggg
	
	jal dest label           				<------ another special case  
	lpa dest label          				<---- another one!
*/



	



	
	// 			optimization phases here!!

	// 		register-only based register allocation goes here!!









/*














	for (nat o = 0, n = 0, s = 0, c = 0, a = 0, i = 0; i < text_length; i++) {
		if (not isspace(text[i])) {
			if (not c) s = i;  c++; 
			if (i + 1 < text_length) continue;
		} else if (not c) continue;
		char* word = strndup(text + s, c);

		puts("\n\n------------------------------------------");
		printf("one: word: \"%s\"\n", word);
		puts("------------------------------------------");

		puts("searching for matching operation word...");
		for (n = 0; n < isa_count; n++) if (not strcmp(spelling[n], word)) { printf("FOUND OP %llu :: %s!\n", n, spelling[n]); goto ins; }
		puts("searching for matching variable_name word...");
		for (n = 0; n < name_count; n++) if (not strcmp(names[n], word)) { printf("FOUND VAR %llu :: %s!\n", n, names[n]); goto arg; }

		puts("could not find any op or var_name that matched...");
		if (not define_on_use(o, a)) {
			snprintf(reason, sizeof reason, "use of undefined \"%s\"", word);
			print_message(error, reason, s, c);
			exit(1);
		} else {
			names[name_count++] = word;
			printf("instead, defined word \"%s\"...\n", word);
			goto arg;
		}
	ins:	printf("at :ins: before, ins_count = %llu, a = %llu...\n", ins_count, a);
		ins_count++; a = 0;
		printf("at :ins: after, ins_count = %llu, a = %llu...\n", ins_count, a);
		ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
		ins[ins_count] = (struct instruction) {0};
		o = n;
		printf("assigning o to be %llu...\n", o);

	arg: 	if (a >= arity(o)) {
			snprintf(reason, sizeof reason, "excess arguments to %s instruction", spelling[o]);
			print_message(error, reason, s, c);
			exit(1);
		}
		ins[ins_count].a[a] = n;
		ins[ins_count].start[a] = s;
		ins[ins_count].count[a] = c;
		a++;

		printf("pushed argument at %llu: { %llu %llu %llu }\n", a - 1, n, s, c); 
		printf("there are now %llu arguments now...\n", a);

		//if (o == att and a == 2) locations[n] = ins_count--;

		puts("going to next word...");

		c = 0;
	}
	ins_count++;









struct instruction operations[4096] = {
		{.a = {add,0,0,0}}
	};



print_registers();
		print_arguments(arguments, arg_count);

	}







		//else if (op == addi)  reg[d] = reg[r] + s;
		//else if (op == slti)  reg[d] = reg[r] < s;
		//else if (op == iori)  reg[d] = reg[r] | s;
		//else if (op == eori)  reg[d] = reg[r] ^ s;
		//else if (op == andi)  reg[d] = reg[r] & s;
		//else if (op == slli)  reg[d] = reg[r] << s;
		//else if (op == srli)  reg[d] = reg[r] >> s;
		//else if (op == srai)  reg[d] = reg[r] >> s;









*/




































/*
	





if (not ins_count) exit(0);
		print_message(error, "encountered runtime instruction with noruntime target", ins[0].start[0], ins[0].count[0]);
		exit(1);















	//bool interpreting = 0;
	//nat arg_count = 0;


	//struct argument arguments[4096] = {0};
	//array = calloc(1 << 16, sizeof(nat));
	//array[2] = (nat) (void*) calloc(1 << 16, sizeof(nat));



	




	for (nat pc = 0; pc < ct_ins_count; pc++) {

		const struct argument a0 = arg_count > 0 ? arguments[arg_count - 1] : (struct argument){0};
		const struct argument a1 = arg_count > 1 ? arguments[arg_count - 2] : (struct argument){0};
		const struct argument a2 = arg_count > 2 ? arguments[arg_count - 3] : (struct argument){0};
		const nat op = ct_ins[i].op, start = ct_ins[i].start, count = ct_ins[i].count;
		const nat d = a0.value, r = a1.value, s = a2.value, 
		printf("two: executing ct_ins #%llu:  [ %s : %llu : %llu %llu %llu]  ::  (start=%llu, count=%llu)\n", 
			i, spelling[op], op, d, r, s, start, count
		);
		compact_print_arguments(arguments, arg_count);
		drop_top_n(arity[op], &arg_count, start, count);
		compact_print_arguments(arguments, arg_count);
		
		if (op == drop) {}
		else if (op == dup_)  arguments[arg_count++] = a0;
		else if (op == over)  arguments[arg_count++] = a1;
		else if (op == back)  arguments[arg_count++] = a2;
		else if (op == swap) { arguments[arg_count - 1] = a1; arguments[arg_count - 2] = a0; } 
		else if (op == arc) {
			printf("executing arc(%llu), replaced with %llu...\n", a0.value, array[a0.value]); 
			arguments[arg_count++] = (struct argument) {array[a0.value], start, count};

		} else if (op == attr) {			
			printf("executing attr(--> %llu)...\n", a0.value);
			array[a0.value] = interpreting ? pc : ins_count;
			printf("loaded array[%llu] with the value %llu...\n", a0.value, array[a0.value]);

		} else if (not interpreting) {
			

		} else {
			if (op == aipc) array[d] = index + r;
			else if (op == addi)  array[d] = array[r] + s;
			else if (op == slti)  array[d] = array[r] < s;
			else if (op == iori)  array[d] = array[r] | s;
			else if (op == eori)  array[d] = array[r] ^ s;
			else if (op == andi)  array[d] = array[r] & s;
			else if (op == slli)  array[d] = array[r] << s;
			else if (op == srli)  array[d] = array[r] >> s;
			else if (op == srai)  array[d] = array[r] >> s;
			else if (op == add)   array[d] = array[r] + array[s];
			else if (op == sub)   array[d] = array[r] - array[s];
			else if (op == ior)   array[d] = array[r] | array[s];
			else if (op == eor)   array[d] = array[r] ^ array[s];
			else if (op == and_)  array[d] = array[r] & array[s];
			else if (op == slt)   array[d] = array[r] < array[s];
			else if (op == slts)  array[d] = array[r] < array[s];
			else if (op == sll)   array[d] = array[r] << array[s];
			else if (op == srl)   array[d] = array[r] >> array[s];
			else if (op == sra)   array[d] = array[r] >> array[s];
			else if (op == ldb)   array[d] = *( u8*)(array[r] + s);
			else if (op == ldh)   array[d] = *(u16*)(array[r] + s);
			else if (op == ldw)   array[d] = *(u32*)(array[r] + s);
			else if (op == ldd)   array[d] = *(nat*)(array[r] + s);
			else if (op == stb)   *( u8*)(array[d] + r) = ( u8)array[s];
			else if (op == sth)   *(u16*)(array[d] + r) = (u16)array[s];
			else if (op == stw)   *(u32*)(array[d] + r) = (u32)array[s];
			else if (op == std)   *(nat*)(array[d] + r) = (nat)array[s];
			else if (op == blt)  { if (array[d] <  array[r]) { jump: if (array[s]) index = array[s]; else skip = s; } } 
			else if (op == bge)  { if (array[d] >= array[r]) goto jump; } 
			else if (op == bne)  { if (array[d] != array[r]) goto jump; } 
			else if (op == beq)  { if (array[d] == array[r]) goto jump; } 
			else if (op == blts) { if (array[d] <  array[r]) goto jump; } 
			else if (op == bges) { if (array[d] >= array[r]) goto jump; }
			else if (op == jalr) { if (d) array[d] = index; index = array[r]; } 
			else if (op == jal) { if (d) array[d] = index; if (array[r]) index = array[r]; else skip = r; } 

			else if (op == ecl) {
				d = array[17]; r = array[10];

				if (d == 1) { count = 0; break; }

				else if (d == 2) {
					snprintf(reason, sizeof reason, "%lld (0x%016llx)", r, r);
					print_message(user, reason, start, count);

				} else if (d == 3) {     // ctabort(); for debugging.
					abort();
				}

				else if (d == 4) array[10] = (nat) getchar();
				else if (d == 5) putchar((char) r);
				else if (d == 6) print_dictionary(names, values, name_count);
				else if (d == 7) print_instructions();
				else if (d == 8) print_registers();
				else if (d == 9) print_arguments(arguments, arg_count);
				else if (d == 10) dump_hex((uint8_t*) array[2], 4096);
				else {
					snprintf(reason, sizeof reason, "unknown ct ecl number %llu", d);
					print_message(error, reason, index - count, count);
					exit(1);
				}
			} else { 
				puts("error: internal ct execute error"); 
				abort(); 
			}
		}
	}




*/


/*




static void drop_top_n(nat n, nat* arg_count, nat start, nat count) {
	if (*arg_count >= n) *arg_count -= n;
	else {
		print_message(error, "insufficient arguments for instruction", start, count);
		exit(1);
	}
}









if (	op == blt  or op == bge or
			op == bne  or op == beq or
			op == blts or op == bges

		)	printf("%s %s(%llu %llu --> @%llu)\n",
				is_compiletime
					? "executing compiletime" 
					: "generating runtime",
				spelling[op], a0.value, a1.value, a2.value
			);
		else 
			printf("%s %llu(%llu:%llu) = %s(%llu(%llu:%llu) %llu(%llu:%llu))\n", 
				is_compiletime
					? "executing compiletime" 
					: "generating runtime",
				a0.value, a0.start, a0.count, 
				spelling[op], 
				a1.value, a1.start, a1.count, 
				a2.value, a2.start, a2.count
			);









printf("\033[31mpushing name %s\033[0m, "
				"pushing value %llu on the stack..\n", 
				names[op], values[op].value
			);








202406215.021857:
in process word   in lexing, 



// note: this stage also implements call-on-used semantics here too, by just generating a runtime/compiletime jalr ra <r> instruction when we see a name. if not call on use, (which is a bit you set for a given name)  then we just generate a push_argument instruction instead, which just takes a single number as argument, and pushes it onto the stack. 


				// i think an arc instruction is actually a ct ins too..


				// in fact, we can't hard code the arguments to any of these instructions,  which means that the master ins array is the rt array, and we need to make a new array for the ct instructions. which will just be an array of nats actually!!   or rather, i guess arguments because we need the source locations of these words lololololol idk  yeah probably 




















			if (is_compiletime) { 

 

				else if (op == ecl) {
					d = array[17]; r = array[10];

					if (d == 1) { count = 0; break; }

					else if (d == 2) {
						snprintf(reason, sizeof reason, "%lld (0x%016llx)", r, r);
						print_message(user, reason, start, count);

					} else if (d == 3) {     // ctabort(); for debugging.
						abort();
					}

					else if (d == 4) array[10] = (nat) getchar();
					else if (d == 5) putchar((char) r);
					else if (d == 6) print_dictionary(names, values, name_count);
					else if (d == 7) print_instructions();
					else if (d == 8) print_registers();
					else if (d == 9) print_arguments(arguments, arg_count);
					else if (d == 10) dump_hex((uint8_t*) array[2], 4096);
					else {
						snprintf(reason, sizeof reason, "unknown ct ecall number %llu", d);
						print_message(error, reason, index - count, count);
						exit(1);
					}
				} else { 
					puts("error: internal ct execute error"); 
					abort(); 
					er: print_message(error, "insufficient arguments for instruction", start, count);
					exit(1);
				}
			} else {
				struct instruction new = {
					.size = ins_size(op, architecture),
					.a =     {   op, a0.value, a1.value, a2.value},
					.start = {start, a0.start, a1.start, a2.start},
					.count = {count, a0.count, a1.count, a2.count},
				};
				if (op == ecm) {
					new.a[1] = array[a0.value];
					new.a[2] = array[a1.value];
					new.size = array[a1.value];
				}
				ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
				ins[ins_count++] = new;
			}



















				nat d = a0.value, r = a1.value, s = a2.value;
				if (op == aipc) array[d] = index + r;
				else if (op == stl)   memcpy((uint8_t*) array[a0.value], text + array[a1.value], array[a2.value]);
				else if (op == addi)  array[d] = array[r] + s;
				else if (op == slti)  array[d] = array[r] < s;
				else if (op == iori)  array[d] = array[r] | s;
				else if (op == eori)  array[d] = array[r] ^ s;
				else if (op == andi)  array[d] = array[r] & s;
				else if (op == slli)  array[d] = array[r] << s;
				else if (op == srli)  array[d] = array[r] >> s;
				else if (op == srai)  array[d] = array[r] >> s;
				else if (op == add)   array[d] = array[r] + array[s];
				else if (op == sub)   array[d] = array[r] - array[s];
				else if (op == ior)   array[d] = array[r] | array[s];
				else if (op == eor)   array[d] = array[r] ^ array[s];
				else if (op == and_)  array[d] = array[r] & array[s];
				else if (op == slt)   array[d] = array[r] < array[s];
				else if (op == slts)  array[d] = array[r] < array[s];
				else if (op == sll)   array[d] = array[r] << array[s];
				else if (op == srl)   array[d] = array[r] >> array[s];
				else if (op == sra)   array[d] = array[r] >> array[s];

				else if (op == ldb)   array[d] = *( u8*)(array[r] + s);
				else if (op == ldh)   array[d] = *(u16*)(array[r] + s);
				else if (op == ldw)   array[d] = *(u32*)(array[r] + s);
				else if (op == ldd)   array[d] = *(nat*)(array[r] + s);
				else if (op == stb)   *( u8*)(array[d] + r) = ( u8)array[s];
				else if (op == sth)   *(u16*)(array[d] + r) = (u16)array[s];
				else if (op == stw)   *(u32*)(array[d] + r) = (u32)array[s];
				else if (op == std)   *(nat*)(array[d] + r) = (nat)array[s];

				else if (op == blt)  { if (array[d] <  array[r]) { jump: if (array[s]) index = array[s]; else skip = s; } } 
				else if (op == bge)  { if (array[d] >= array[r]) goto jump; } 
				else if (op == bne)  { if (array[d] != array[r]) goto jump; } 
				else if (op == beq)  { if (array[d] == array[r]) goto jump; } 
				else if (op == blts) { if (array[d] <  array[r]) goto jump; } 
				else if (op == bges) { if (array[d] >= array[r]) goto jump; }
				else if (op == jalr) { if (d) array[d] = index; index = array[r]; } 
				else if (op == jal) { if (d) array[d] = index; if (array[r]) index = array[r]; else skip = r; }












if (skip) {
			printf("[in skip mode...]\n");
			if (op == comp) is_compiletime = not;
			else if (op == attr) goto execute_attr;
			else if (values[op].value == skip) goto push_name; 
			goto next;
		}











if (	op == comp or 
			op == drop or 
			op == swap or 
			op == dup_ or 
			op == over or 
			op == back or 
			op == ecall
		) {}

		else if (op == ecm or op == jalr or op == jal) {  if (arg_count < 2) goto er; arg_count -= 2; } 
		else if (op == blt or op == bge or op == bne or op == beq or op == blts or op == bges) { 
			if (arg_count < 3) goto er; arg_count -= 3; 
		} else if (op == aipc) { 
			if (arg_count < 2) goto er; arg_count -= 2;
			arguments[arg_count++] = a0;
		} else { 
			if (arg_count < 3) goto er; arg_count -= 3; 
			arguments[arg_count++] = a0; 
		}

















		} else if (op == rot) {
			arguments[arg_count - 1] = a2;
			arguments[arg_count - 2] = a0;
			arguments[arg_count - 3] = a1;






	i think i am actaully going to make binary constants in the language simply use:




			'        for 1  

			.        for 0 



	ie, natural numbers would be:


	0	.

	1	'
	
	2	.'

	3	''
		
	4	..'

	5	'.'
	
	6	.''

	7	'''

	8	...'

	9	'..'

	10	.'.'

	11	''.'
	
	12 	..''




							oh also, you can have "/" in your literals too, as a seperator, its just ignored lol. so yeah. easy stuff. 






yeah, i think this is the way we are going to write binary constants. pretty cool huh!

		i like it alot actaully. its so simple, yet so elegant, and really easy to read too!!! nice..








simplest program now:

	
	'.' . .'.' addi
	' . '...' addi
	ecall


would it be more readable like this?:


	,., . .,., addi
	, . ,..., addi
	ecall

				<--- yeah that version is actually a lot more readable! 
						okay lets just use that then. lol. 


	


	so that this language only uses these symbols in total:

		abcdefghijklmnopqrstuvwxyz,.


					perfection!   nice. love ittttt   thats so cool 



					ideallyyyyyy it would just be the letters, but this is still good.

					doing constants without.. hm.. 




							waitttt






		okay wait technically we could get away with not having the . and , lol 


			ie, no binary constants at all... hm.. 


					because,  like, we can construct them at compiletime lolol 



						soooooooo  yeah 




				i mean, technically we could just use    "arc"  to computationally construct literals at compiletime,  and then once they are assigned to a variable, we have them forever lol.



							soooooo likeeeee





			yeah honestly, ngl, i feel like we can do this without literals. omg. 



	okay 



					lets do it.   we got this.   lets JUST have   arc, 


									thats it 






			okay wait, but then in order to properly boostrap outselves, we alsooooo need to have a couple of things already pre defined, so for instance, i think we probably need the numbers 0 through 5 already defined, but thats it, i think. cool!  okay we can do this lol. yay. 




			lets call them the abi names too,      so 2 is sp    1 is ra   0 is zero 

								gp is 3,    tp is 4    



			actuallyyyy i might rename them idk... hm.. 



				i mean, i guess it makes sense to keep the names that riscv gave for them.. idk.. hm.. 

					yeah probably i guess okay  



					i think we only need the           wait yeah 


								we only need ra and 0 right? becuase then from there we can construct any other lol 


			okay yeah, i think so 



			well      hmmm idk    yeah 

									lets build in    0, 1, 2,    thats it 



							i think 3 will not be built in, i don't think we need it i think lol 




		yay


	okay lets try this, omg






 



	
	
*/	




















//print_message(info, reason, start, count);
//print_message(warning, reason, start, count);
//print_message(error, reason, start, count);
//print_message(no_message, reason, start, count);





/*
	} else if (not is_compiletime) {

		struct instruction new = {0};
		new.a[0] = (u32) op;
		new.start = index;
		new.is_signed = is_signed;
		new.is_immediate = is_immediate;

		is_signed = false; is_immediate = false;

		for (nat i = 1; i < 4; i++) {
			new.a[i] = (u32) (arg_count > i - 1 
				? arguments[arg_count - 1 - (i - 1)] 
				: 0
				);
			dd printf(" ... argument #%llu : u32 = %u\n", 
				i, new.a[i]);
		}
		new.size = 4;
		ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
		ins[ins_count++] = new;

*/






































/*
begin:	if (name >= name_count) goto error;
	if (at and at == lengths[name]) goto found;
	if (index >= text_length) goto done;
	if ((unsigned char) text[index] < 33) goto nextc;
	if (names[name][at] != text[index]) goto fail;
	at++; goto nextc;
advance: save = index; name = 0; at = 0; goto begin;
fail: 	name++; index = save; at = 0; goto begin;
nextc:	index++; if (index > max) max = index; goto begin;
error:	
	if (defining and start) {
		dd puts("skipping over undefined char becuase in defining state..."); 
		index++; goto advance; 
	}
	print_error("unresolved symbol", save, max);
	exit(1);
found:
	dd printf("debug: found name: %s\n", names[name]);
	if (defining) {
		dd puts("[in defining state]");
		if (not start) {
			start = index;
			delimiter = name;
			dd puts("AT OPENING DELIMITER");
		} else if (delimiter == name) {
			dd printf("note: name is between: %llu and %llu... (used delimiter %s)\n", start, save, names[delimiter]);
			dd print_index_pair(start, save);
			push_name(text + start, save - start);
			dd puts("AT CLOSING DELIMITER");
			defining = 0; start = 0;
		}
		goto advance;
	}

	for (nat i = 0; i < isa_count; i++) {
		if (not strcmp(spelling[i], names[name])) { 
			op = i; goto builtin;
		}
	}

	dd printf("info: found user-defined name:     "
		"calling \"%s\"!! \n", names[name]
	);

	arguments[arg_count++] = values[name];
	dd printf("----> pushed: user-defined value %llu onto stack...\n", 
		arguments[arg_count - 1]);

	goto advance;

builtin:;

	dd printf("debug: found builtin name: %s\n", spelling[op]);

	nat a0 = arg_count > 0 ? arguments[arg_count - 1] : 0;
	nat a1 = arg_count > 1 ? arguments[arg_count - 2] : 0;
	nat a2 = arg_count > 2 ? arguments[arg_count - 3] : 0;

	if ((false)) {}

	else if (op == def) { dd puts("executing def..."); defining = true; }
	else if (op == cte) { dd puts("executing cte..."); is_compiletime = true; }
	else if (op == imm) { dd puts("executing imm..."); is_immediate = true; }
	else if (op == sgn) { dd puts("executing sgn..."); is_signed = true; }

	else if (op == ari) { dd puts("executing mi..."); arguments[arg_count - 1]++; }
	else if (op == arz) { dd puts("executing mz..."); arguments[arg_count - 1] = 0; }
	else if (op == drp) { dd puts("executing drop..."); arg_count--; }
	else if (op == arc) { dd puts("executing carg..."); arguments[arg_count - 1] = array[a0]; }

	else if (op == atr) {
		dd puts("executing atr...");
		array[a0] = is_compiletime ? index : ins_count;
		if (skip == a0) skip = 0;
		is_compiletime = false;

	} else if (op == dup_) {
		dd puts("executing dup...");
		arguments[arg_count++] = a0;

	} else if (op == ovr) {
		dd puts("executing over...");
		arguments[arg_count++] = a1;

	} else if (op == thd) {
		dd puts("executing third...");
		arguments[arg_count++] = a2;

	} else if (op == swp) {
		dd puts("executing swap..."); 
		arguments[arg_count - 1] = a1;
		arguments[arg_count - 2] = a0;

	} else if (op == rot) {
		dd puts("executing rot...");
		// 1 2 3 -> 2 3 1
		arguments[arg_count - 1] = a2;
		arguments[arg_count - 2] = a0;
		arguments[arg_count - 3] = a1;




	} else if (not is_compiletime) {

		// TODO: we still need to 
		// edit the argstack after 
		// pushing the rt ins!!!!

		dd printf("info: pushing rt ins %llu(\"%s\")...\n", 
			op, spelling[op]
		);
		struct instruction new = {0};
		new.a[0] = (u32) op;
		new.start = index;
		new.is_signed = is_signed;
		new.is_immediate = is_immediate;

		is_signed = false; is_immediate = false;

		for (nat i = 1; i < 4; i++) {
			new.a[i] = (u32) (arg_count > i - 1 
				? arguments[arg_count - 1 - (i - 1)] 
				: 0
				);
			dd printf(" ... argument #%llu : u32 = %u\n", 
				i, new.a[i]);
		}
		new.size = 4;
		ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
		ins[ins_count++] = new;

	} else {
		nat e = op;
		
		dd printf("------> info: CT: EXECUTING: op = %llu (\"%s\"), "
			"args={a0:%llu, a1:%llu, a2:%llu}\n", 
			op, spelling[op], a0, a1, a2
		);

		if (e == add) { 
			array[a0] = array[a1] + (is_immediate ? a2 : array[a2]); 
			arg_count -= 2;
			arguments[arg_count - 1] = a0; 
		}

		else if (e == sub) { 
			array[a0] = array[a1] - array[a2]; 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == mul) { 
			array[a0] = array[a1] * array[a2]; 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == div_) { 
			array[a0] = array[a1] / array[a2]; 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == rem) { 
			array[a0] = array[a1] % array[a2]; 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}
		
		else if (e == and_) {
			array[a0] = array[a1] & (is_immediate ? a2 : array[a2]); 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == ior) {
			array[a0] = array[a1] | (is_immediate ? a2 : array[a2]); 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == eor) {
			array[a0] = array[a1] ^ (is_immediate ? a2 : array[a2]); 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == slt) {
			array[a0] = array[a1] < (is_immediate ? a2 : array[a2]); 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == sll) {
			array[a0] = array[a1] << (is_immediate ? a2 : array[a2]); 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == srl) {
			array[a0] = array[a1] >> (is_immediate ? a2 : array[a2]); 
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == ldb)   { 
			array[a0] = *( u8*)(array[a1] + a2);
			arg_count -= 2; 
			arguments[arg_count - 1] = a0;
		}
		
		else if (e == ldh) { 
			array[a0] = *(u16*)(array[a1] + a2);
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == ldw)   { 
			array[a0] = *(u32*)(array[a1] + a2);
			arg_count -= 2; 
			arguments[arg_count - 1] = a0;
		}

		else if (e == ldd) { 
			array[a0] = *(nat*)(array[a1] + a2);
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == stb)  { 
			*( u8*)(array[a0] + a1) = ( u8)array[a2];
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}
		
		else if (e == sth)  { 
			*(u16*)(array[a0] + a1) = (u16)array[a2];
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}
		
		else if (e == stw)  { 
			*(u32*)(array[a0] + a1) = (u32)array[a2];
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}
		
		else if (e == std) { 
			*(nat*)(array[a0] + a1) = (nat)array[a2];
			arg_count -= 2; 
			arguments[arg_count - 1] = a0; 
		}

		else if (e == blt) { 
			arg_count -= 3; 
			if (array[a0]  < array[a1]) { 
				if (array[a2]) index = array[a2]; 
				else skip = a2; 
			} 
		} else if (e == bge)  { 
			arg_count -= 3; 
			if (array[a0] >= array[a1]) { 
				if (array[a2]) index = array[a2]; 
				else skip = a2; 
			} 
		} else if (e == beq)  { 
			arg_count -= 3; 
			if (array[a0] == array[a1]) { 
				if (array[a2]) index = array[a2]; 
				else skip = a2; 
			}
		} else if (e == bne)  { 
			arg_count -= 3; 
			if (array[a0] != array[a1]) { 
				if (array[a2]) index = array[a2]; 
				else skip = a2; 
			} 

		} else if (e == jlr) {
			if (not is_immediate) { // jalr
				arg_count -= 3; 
				array[a0] = index;
				index = array[a1] + a2;
			} else { // jal 
				arg_count -= 2; 
				array[a0] = index;
				if (array[a1]) index = array[a1]; else skip = a1;
			}
		}

		else if (e == ecl) {
			dd puts("executing ecall at ct...");

			if (a0 == 1) { at = 0; goto done; }
			else if (a0 == 2) printf("debug: %lld (hex 0x%016llx)\n", array[a1], array[a1]);
			else if (a0 == 3) array[a1] = (nat) getchar();
			else if (a0 == 4) putchar((char) array[a1]);
			else if (a0 == 5) print_dictionary();
			else if (a0 == 6) print_instructions();
			else if (a0 == 7) print_registers();
			else if (a0 == 8) print_arguments();
			else {
				printf("error: ct: unknown ecall number %llu... aborting...\n", a0);
				abort();
			}

		} else { 
			printf("internal error: found unimplemented "
				"operation: %llu (\"%s\")...\n", 
				op, spelling[op]
			);
			printf("error: unknown ct ins = %llu\n", op); 
			print_error("unknown ct ins", index, 0); 
			abort(); 
		}
		is_compiletime = false;
		is_signed = false;
		is_immediate = false;
	}
	goto advance;



*/













/*


"		// push_name(spelling[i], strlen(spelling[i]));



	if (defining) {
		printf("error: expected closing delimiter for name...\n");
		print_error("missing name closing delimiter", index, max);
		exit(1);
	}

	if (at) {
		printf("error: unexpected end of input\n");
		print_error("unexpected end of input\n", index, max);
		exit(1);
	}












static void push_name(const char* raw_string, const nat raw_length) {


	nat length = 0;
	char* string = calloc(raw_length + 1, 1);

	for (nat i = 0; i < raw_length; i++) {
		if ((unsigned char) raw_string[i] < 33) continue;
		string[length++] = raw_string[i];
	}

	nat spot = 0;
	for (; spot < name_count; spot++) 
		if (length >= lengths[spot]) break;

	memmove(lengths + spot + 1, lengths + spot, 
		sizeof(nat) * (name_count - spot));

	memmove(values + spot + 1, values + spot, 
		sizeof(nat) * (name_count - spot));

	memmove(names+ spot + 1, names + spot, 
		sizeof(const char*) * (name_count - spot));
	
	lengths[spot] = length;





}


*/

















/*
2405201.222347:

	currently the bug is that we are not processing the entire file, we are missing the last instruction that is said, which is a problem. 


		print out where we are in the file inside of our parsing algorithm!





static void check(nat r, nat c, const char* type, nat arg_index) {
	if (r < c) return;
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "argument %llu: invalid %s, %llu (%llu >= %llu)", arg_index, type, r, r, c);
	print_error(reason, arg_index, 0); 
	exit(1);
}






*/






/*static nat generate_adr(nat* a, nat op, 
			nat im, nat oc) {   //      adrp: oc = 1 

	nat Rd = (nat) a[0];
	nat Im = * (nat*) im;
	check(Rd, 32);
	nat lo = 0x03U & Im, hi = 0x07FFFFU & (Im >> 2);

	return  (oc << 31U) | 
		(lo << 29U) | 
		(op << 24U) |
		(hi <<  5U) | Rd;
}*/









/*
static nat generate_memi(nat Rt, nat Rn, nat im, nat oe, nat op, nat oc, nat sf) {     
	
	check(Rt, 32);
	check(Rn, 32);

	Rt = arm64_macos_abi[Rt];
	Rn = arm64_macos_abi[Rn];

	check(im, 1 << 9U);

	return  (sf << 30U) |
		(op << 24U) |
		(oc << 22U) |
		(im << 12U) |
		(oe << 10U) |
		(Rn <<  5U) | Rt;
}














example testing code on 202405304.122037:






101 dup 111 add 

def hello

001 dup hello swap sub

001 001 001 ecall

011 011 011 addi








puts("asm: \033[31;1merror:\033[0m unknown target architecture specified, valid values: "); // TODO: use print_error();
		for (nat i = 0; i < target_count; i++) {
			printf("\t%llu : %s\n", i, target_spelling[i]);
		}






puts("asm: \033[31;1merror:\033[0m unknown output format specified, valid values: "); // TODO: use print_error();
		for (nat i = 0; i < output_format_count; i++) {
			printf("\t%llu : %s\n", i, output_format_spelling[i]);
		}





static bool execute(nat op, nat a0, nat a1, nat a2, nat index, nat count) {

}


static void execute_branch(nat op, nat a0, nat a1, nat a2, nat* skip, nat* index) {
	if ((0)) {}
	
}



*/






/*
static void print_files(void) {
	printf("here are the current files used "
		"in the program: (%lld files) { \n", 
		file_count
	);
	for (nat i = 0; i < file_count; i++) {
		printf("\t file #%-8lld :   "
			"name = \"%-20s\", "
			".start = %-8lld, "
			".size = %-8lld\n",
			i, files[i].name, 
			files[i].start, 
			files[i].count
		);
	}
	puts("}");
}

static void print_stack(nat* stack_i, 
	nat* stack_f, 
	nat* stack_o, 
	nat stack_count
) {
	printf("current stack: (%lld entries) { \n", stack_count);
	for (nat i = 0; i < stack_count; i++) {
		printf("\t entry #%-8lld :   "
			"name = \"%-20s\", "
			"i = %-8lld, "
			"f = %-8lld, "
			"o = %-8lld / %lld\n", 
			i, files[stack_f[i]].name, 
			stack_i[i], stack_f[i], 
			stack_o[i], files[stack_f[i]].count
		);
	}
	puts("}");
}


static void print_index_pair(nat start, nat end) {
	for (nat i = 0; i < text_length; i++) {
		if (i == start or i == end) printf("\033[32;1m");
		putchar(text[i]);
		if (i == start or i == end) printf("\033[0m");
	}
	puts("");
}


*/






/*


				ADD CONTEXTS FOR NAMES!!!!!!



							lexical stack of them, controlled via ct too!!!






	shifts:    x  slli   

	math:       mul   x mulh    x  div   rem







fprintf(stdout, "asm: \033[31;1merror:\033[0m %s: "
				"\"%s\"\n", strerror(errno), name); 





*/








//  
		// } else 

			// defining = false;

			/*nat r = 0, s = 1;
			for (nat i = 0; i < count; i++) {
				if (word[i] == seperator_digit) continue;
				if (word[i] == zero_digit) s <<= 1;
				else if (word[i] == one_digit) { r += s; s <<= 1; }
				else goto unknown;
			}*/

			// printf("pushing literal %llu on the stack.. 
			// (found at .start=%llu,.count=%llu)\n", r, start, count);
			// getchar();
			//arguments[arg_count++] = (struct argument) {.value = r, 
			// .start = start, .count = count};


			// puts("you did a dumb dumb lol");
			// getchar();

			
			// goto next;

			// unknown: 
		//}








/*if (is_binary_literal(word)) {
			snprintf(reason, sizeof reason, "expected "
			"undefined word, word \"%s\" is a "
			"defined binary literal", word); print_message(error, 
			reason, start, count); exit(1);
		}*/






/*static bool is_binary_literal(char* word) {
	for (nat i = 0; i < strlen(word); i++) 
		if (	word[i] != zero_digit and 
			word[i] != one_digit  and
			word[i] != seperator_digit) 
			return false; 
	return true;
}




sp zero zero ct addi
ra zero zero ct addi
arc def three drop drop

three three three ct addi
ra zero three ct addi
arc def four drop drop






*/







// static const char zero_digit = '0', one_digit = '1', seperator_digit = '/';


/* 

programmers api:


	BYTE_COUNT CTM_POINTER ecm

	END_INDEX BEGIN_INDEX CTM_DEST_ADDR stl


----------------------------------------------




static const char* default_string = "hello there from space!\nthis is a test. yay!\n";






printf("found word at %llu:%llu... \"%.*s\"\n",  
			start, count, (int) count, text + start);







// stage two: ct system:
	// ct_ins_count and ct_ins will be set to an array of instructions that we can process/execute now!








// printf("after executing: \n");
			// compact_print_arguments(arguments, arg_count);











static const u8 arity[isa_count] = {
	0, 0, 1, 0, 
	0, 2, 0, 0, 

	0, 0, 0, 0, 
	2, 2, 2, 2, 

	2, 2, 2, 2, 
	2, 2, 2, 2, 



	2, 2, 2, 2, 
	2, 2, 2, 2, 

	2, 2, 2, 2, 
	2, 2, 2, 2, 

	2, 2, 2, 2, 
	2, 2, 3, 3, 



	3, 3, 3, 3, 
	2, 2, 1

};










*/	


/*static void emith(nat x) {
	bytes = realloc(bytes, byte_count + 2);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
}*/


/*static void emitd(nat x) {
	bytes = realloc(bytes, byte_count + 8);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
	bytes[byte_count++] = (u8) (x >> 16);
	bytes[byte_count++] = (u8) (x >> 24);
	bytes[byte_count++] = (u8) (x >> 32);
	bytes[byte_count++] = (u8) (x >> 40);
	bytes[byte_count++] = (u8) (x >> 48);
	bytes[byte_count++] = (u8) (x >> 56);
}*/













/*


				snprintf(reason, sizeof reason, 
					"expected undefined word, "
					"word \"%s\" is defined", word
				); 
				print_message(error, reason, start, count); 
				exit(1);



			printf("\033[32mdefining new dest word\033[0m \"%s\"...\n", word);

*/





/*

//static nat* array = NULL;

static void compact_print_arguments(struct argument* arguments, nat arg_count) {
	printf("args { ");
	for (nat i = 0; i < arg_count; i++) {
		printf("%llu ", arguments[i].value);
	}
	puts("} \n");
}

static void print_arguments(struct argument* arguments, nat arg_count) {
	printf("\narguments[]: { \n");
	for (nat i = 0; i < arg_count; i++) {
		printf("\targuments[%llu] = { %llu  :"
			"  (.start=%llu,.count=%llu)} \n", 
			i, arguments[i].value, arguments[i].start, arguments[i].count
		);
	}
	puts("} \n");
}

static void print_dictionary(char** names, struct argument* values, nat name_count) {
	puts("printing dictionary...");
	for (nat i = 0; i < name_count; i++) {
		printf("\t#%-6llu: name %-8s ..... %-5lld \t\t(.start=%llu,.count=%llu))\n", i, 
			names[i], values[i].value, values[i].start, values[i].count
		);
	}
	puts("done.");
}


static void print_registers(void) {
	printf("registers: {\n");
	for (nat i = 0; i < 32; i++) {
		if (i % 2 == 0) puts("");
		printf("\t%llu:\t%016llx = %-7lld\t", i, array[i], array[i]);
	}
	puts("\n}\n");
}


	// print_files();
	// print_stack(stack_i, stack_f, stack_o, stack_count);

	// const int colors[] = {31, 32, 33, 34, 35};
	//printf("\033[%dm", colors[1]);

	//printf("file %s begins at %llu!\n", 
	//files[f].name, index);


	// const nat count = files[f].count;


	//print_stack(stack_i, stack_f, stack_o, stack_count);
				//printf("file %s reached the end the "
				//	"file! (stack_o[%llu] == count == %llu)\n", 
				// files[f].name, stack_count - 1, count);


//printf("\033[38;5;255m(ERROR_HERE:%s:%llu)\033[0m",
			// files[stack_f[stack_count - 1]].name, stack_o[stack_count - 1]);




//printf("\033[%dm", colors[stack_count - 1]);
		// putchar(text[index]);
		//printf("\033[0m");
		//printf("[%s]: incremented stack_o[top=%llu] to be 
		// now %llu...\n", files[stack_f[stack_count - 1]].name, 
		// stack_count - 1, stack_o[stack_count - 1]);

	// printf("\033[0m");




*/





















 
/*




202406237.152400:


	note:


		i think we are going to have to allow for macros   via       detecting if a variable is used at the first position, 

				if it is,  then we can basically do something special, there are a couple of things that we could do:



							1.  we could make it so that thats label!   this might be optimal actually!



							2. if its already defined as a label,   then we could actually call it as a function!

										ie, generate a runtime     jal ra labelname



										



									that could work also work great!    basically to allow for calling functions just by saying the name of the function! which we definitely want to support lol.




									so yeah, that will make it call on use. 


												thats nice 





						
	so code would end up looking like:




		myfunction
			add hello 4 5 

			sub hello hello 3

			jalr zero ra



		main
			myfunction





		


pretty cool huh!


	that actually would work great i think 

	hm

			interesting 


	wow


	yeah, i think i like this a lot lol 



			its just   what about argument passing? 


				well, i think the only real way to do it now, is via   assigning to particular registers,


					and just happen to using those registers, 

						which i think is fine honestly, that gets the job done lol 

							hm
										ill think about it more though 




							maybe theres a way to not require assigning to registers?..





			well wait no 

						because you have to realize that function inlining will happennnnn



						

	which will end up transforming:



		myfunction
			add hello 4 5 

			sub hello hello 3

			jalr zero ra



		main
			myfunction




	into simplfy being:




		main
			add hello 4 5 

			sub hello hello 3






	and so like, data flow needs to be optimized accordingly


		like, if you don't want those extraneous moves and copies of the arguments to particular input registers,

					 to happen,  then you shouldnt write them lol


							just literally write out the variables you want to manipulate lol 


								not too hard i think 









				interesting 

	hm

							so yeah, for now that will work i think. 







		just supporting that 




					if the label is defined, (ie, set to some value other than -1)


								then we will treat the occurence of the label as a    "jal ra label"


								but if you say a label at the first position,   with no operation before it, 


								andddd it hasnt been defined yet, 


											(ie, given a label address value / location)




									then we will say that its a label attribution statement!

								pretty easy i think

















202406237.003701:


		changed the language up completely, to be prefix now, and not use an argument stack at all.
		heres what code looks like now. the first argument is the destination, which is defined if not defined. only dest does this though. 


			add hello 4 5 

			sub hello hello 3
	
			mul result 6 4 33 5 56       // error: too many arguments given to instruction mul.


			mul result result 2       
						// these two are equivalent of course lol
			sll result result 1



			aipc pointer 0         // gets the pc

			ldd a pointer 4          // loads from the pc+4 a double-word (64 bits)


			



	how do we do compiletime computation now then?... 


			thats the major question im trying to figure out now. 
	hm
						



									hoenstly this is kinda calling into question the entire language now, actually, 






					because like, 








			



											.... 










					is there a functionally any difference between     the optimizer running something   because its statically known    and the user saying that they want something run  becuase they set the ct bit?                   NO. theres not. 


									theres no difference, and there shouldnt be a difference.   we should prefer and only supply the first method.    delete the second. 



yay





							ie, this is now a   2 main passes   assembler-     	parsing, which does label attribution, and word parsing, 


									and instruction generation, 







					with an analysis and optimization phase in the middle!!!




								which allows staticaly known ct computation to happen.   YAYYY











old:


	- implement 3 stages:

		- parsing/lexing:   names, def, strings, comments, ins op codes, [add a name-push instruction to get args?]
					wait... how do we do arc.... CRAPPPPP

		- compiletime interpretting:    execute rt instructions, and regenerate rt ins

		- code generation / byte emmision / instruction selection  on rt instructions, according to arch/target








old:



	new semantics to implment:


		- make variable names the default construct,
		     make register assignment implicit

		- make labels easier to work with?..


		- make the complietime system not do file-offset ct-branching.... do it properly.


		- make the language isa the IR of the compiler.


		- make strings use a different system....


		- add an interpreter mode!   where the runtime code is just interpreted 
			in a virtual machine, for maximum portability lol. 


		- 







old:
remaining features:

	- double check all forwards/backwards branches behavior

	- add bne,beq,bge,blt

	- add ld's and st's for arm64 backend

	- get mulitple files working, to include stdlib's foundation file, which creates constants. 

	- 




















ecall

ldb/h/w/d dest source		good
stb/h/w/d source source     				<----- hmmmmmmm danggg

jalr dest source		good
jal dest label           				<------ another special case  
lpa dest label          				<---- another one!

add dest source source		good
sub dest source source		good
slt dest source source		good
slts dest source source		good
and dest source source		good
ior dest source source		good
eor dest source source		good
sll dest source source		good
srl dest source source		good
sra dest source source		good
mul dest source source		good
mulh dest source source		good
mlhs dest source source		good
div dest source source		good
divs dest source source		good
rem dest source source		good
rems dest source source		good

blt label source source		good
blts label source source	good
bge label source source		good
bges label source source	good
bne label source source		good
beq label source source 	good




cool okay so only    3 special casese technicallyyyyy nice nice 


	because yeah, for all instructions, (EXCEPT FOR THE STORE INS's)   the destination register   is able to be defined by the user on its first occurence in that position. very cool. 


		oh, same for the zeroth position too,  in replace of the op code lol.  this would be an label attribution though.  so yeah. 




	then, we just need to know that          store instructions have no destination    or labels, and therefore don't define values ever 


		AND



			the jal and lpa (aka auipc) instructions     actually ONLY take values  which can be defined on their first use,

						ie, it only takes destinations- kinda      labels are destinations,   in a way


						but with the lpa, it literally has a destination,  a dest register, 

								ANDDD it takes a label, 



							and then with jal,   it has a destination register, AND it performs a branch. so yeah. a label "dest" too. 


	
								


						so yeah, thats everything i think. nice.




			lets code that up, just 3 special casese in total    not too bad 




	



											oh plus  ecall, which is kinda special in every way LOL


										we'll just ignore ecall, its super easy, it takes nothing, and is as plain as you could make an instruction loolol. a single op code with no arguments

					very amazing actually 

	but yeah 

anyways







ISA  as of 202406237.182942:
================================================


	ecl
	nse
	use	filename
	att 	label
	nsb 	namespace
	jalr 	source 	dest
	jal 	label 	dest
	ldb 	dest 	pointer
	ldh 	dest 	pointer 
	ldw 	dest 	pointer
	ldd 	dest 	pointer 
	stb 	source 	pointer
	sth 	source 	pointer 
	stw 	source 	pointer
	std 	source 	pointer
	add 	dest 	source 	source 
	sub 	dest 	source 	source
	slt 	dest 	source 	source
	slts 	dest 	source 	source
	and 	dest 	source 	source
	ior 	dest 	source 	source
	eor 	dest 	source 	source
	sll 	dest 	source 	source
	srl 	dest 	source 	source
	sra 	dest 	source 	source
	mul 	dest 	source 	source
	mulh 	dest 	source 	source
	mlhs 	dest 	source 	source
	div 	dest 	source 	source
	divs 	dest 	source 	source
	rem 	dest 	source 	source
	rems 	dest 	source 	source
	blt 	label 	source 	source 
	blts 	label 	source 	source
	bge 	label 	source 	source
	bges 	label 	source 	source
	bne 	label 	source 	source
	beq 	label 	source 	source
	

================================================



*/





















// old
// rewritten on 202406215.012903: word based, but simpler stage structure
//  and simpler system of doing strings and comments.

// old: my assembler: written on 202405304.122455 by dwrr.

/*



	202406241.013212:
		just implemented mulitple files in the language, it was really simple becuase our lexing/parsing is super simple now lol, and the language as a whole is much simpler lol.


			now, we have to actually do   scopes,   and namespaces.   ie, contexts.    ie, some way of grouping names, in a way that allows you to encapsulate things,  defining the same name to mulitple entities,  each in their own context, and used for different things. this is a MUST, as often in human language, the same name is used for several things, but when used in different contexts,  they mean different things.   same idea in programming, its a fundemental way of humans speaking, kinda.  so yeah. its kind of not that ergonomic to require that everything be a unique name, by its name alone. it would be nice to define a context, that allows for names to be shorter, on average, while operating within that context.  this is a almost universal ergonomic feature of humans procressing, i think. and just generally speaking. 


				so acheive this, i propose a way of creating a scope, 


					and then attributing a name to that scope. 


			so, basically, we will at the very least, have some sort of opening and closing tags, 


					ns_begin  ns_end                        ns standing for namespace, lets say lololol   just some rough names


			and then ns_end actually takes an argument, which is the name of the namespace. so yeah. its not defined until you leave the namespace, basically. 

					or rather maybe you should have the begin    define the name?   yeah lets try that i think. 





			so yeah, basically, you do 




							ns_begin my_namespace_name_here

								...code...

								add hello                   <--- defines hello, and sets it to zero. 

							ns_end



			and that basically sets it up 




	and then as far as using elements in the name space, 


			you actually refer to them like this:

							my_namespace_name_here hello



			ie, you say the namespace, and then the hello. 





			and basically, 


					(first, it should be noted:    the LANG ISA    is actually universally across accessible across namespaces, 

											wihch makes sense)



						(but the list of names, are not neccessarilyyy. rather, there is a default namespace,  which has the name <EMPTY_STRING>, ie "",  and then the default builtin variables, like zero, ra, etc     are in there, 


									and then when you open up a new namespace, it makes a new    user variable dictionary   for names    in that namespace       basicallyyy





					the look up for looking up sometihng  basically firsttttt involves   selecting the namespace, 



						OH!!


										wait!!






					a NAMESPACE NAME     IS ITSELF     IN THE GLOBAL NAMESPACEEEE           LOLOLOL




						okay cool lol thats neat





					so yeah, the my_namespace_name_here    name       that name is in the global namespace,   



							ie, the namespace  "" 




				so yeah even taht name is itself technically      namespace-qualified      lol technically   you just can't see the ns because its the empty string lolol 

							so yeah 






							basically, if you refer to a name,  it doesnt actually push an arg, 





								it just sets the         ns            nat 



							which,  then causes the NEXT word to be looked up   in that namespace with that ns index 




										and then after pushing a name succesfully, we ALWAYS revert back to the global namespace. always. 


								so yeah, that has the effect that things which are not in the global space, always need to be qualified, basically. 


							so yeah! very cool. i like this lol. yay!!!!







so cool 




		this is actually so good so far 


					

		hm

					now, note 


										technically, namespaces are ALSOOO   kinda like 





								scopes!!!








					(its just, obviusly, theres no stack allocations happening in them that get automatically deallocated lolol, like in c lol)



								but yeah, minus that whole thing, they operate like in c,      you can have shadowing of variable names, and the local closest copy gets used, just as you would think, 



								and of course, you don't have to qualify names while in that namespace lolol
 







		OHHH 		

						okay cool so when you define a namespace, you actually set the ns  nat    like permentently 



								until you leave that namespace, 





								BUTTTTTT


										okay crap lol 




										so actually, we need  like      3 lookups





											first we try to look up the word in the operation list, 

												which is universal across everythin, 





											then we try to look it up in the local namespace, 


												then we try to look it up in the local namespace, 


													







		oh, and btw, you cannot nest namespaces.   

				... 


										is that a good idea?







		hmmm actually 


	idkkkk 
			i feel like you should be able to nest them... 


butt


		yeah, that would just get a bit too complex to implement, so i will say that no  you cannot nest them lolol 

		i don't exactlyy wanttttt this language to be expresion based in any way 

					like, i don't even really likethe fact taht the    nsbegin and nsend        ie   nsb   and nse 


										kinda look like {    and      }          lol 



				ie,           namespace hello { ... }          from c++         LOL          like, its quite similar to that, 



									except for the fact that you cannot nest namespaces, 




								ANDDDD     qualifying a ns  to a variable looks like 

												ns::name   in c++,    but   ns name     in our language,


							and 




								yeah  					ns name          is MUCHHH more readable lolol 




													and easier to type  





				so yeah 





		so yeah,              not nestable, and more readable   i think its a fine way to do things 

















but yeah, 3 lookups, 



		1. look up in the operation isa listing

		2. look up in the current namespace's variable dictionary

		3. look up in the global namespaces variable dictionary 







	note the order-  this is how we support shadowing lol. so yeah. i think that makes sense. 





	yayyyyyyy this is quite cool 




				honestly the more i think about it, the more nesting these things kindaaaa makes sense and is really useful-



							particularly when thinking about namespaces as actually just straight up scopes lololol





							butttttt on the other hand.. idk... ill probably pass on it for now.. idk.. hm.. 






			ill think about it. and see how complex it might be to implement it.. 



							basically, we would need a stack of currently open namespaces, as opposed to just one currently open one, given by the ns index... 


	

								like,     ALSOOO   OMG  the call would need to be super complex then 



												like,




									if lets say you had:






		nsb a

			nsb b

				nsb c


					add hello


				nse

			nse

		nse





		like,   you would look it up like 


					a b c hello           right?








				GAHH


						yeah thats actually so complex to implement i think lololo 


						hm







		ERR WAIT

				IS IT?



					because like, we would just be saying that 





						we are calling          a          first     which then puts us in the a ns







								and then   b             which is a name    in           a 




												which ends up putting us in the b namespace, 



									and then  finally         c         similar procress, again, 




										and then finally            hello 



										which is in c 














		so like!?!?



					okay interesting so actually call kinda solves itself lololol



					thats interesting 



							wow 





		okay maybe we do allow for nesting- 

							like, the implmentation is really just complicated by the     opening of nested ones






						like, 





							here,              nsb a nsb b nsb c add hello nse nse nse              



										in that code, we have to keep track of a stack, of currently open namespaces, 




										buttt... 




										thats really the only complicatation to the typical case that we need to add!!?!?



										so yeah lets make these lil guys nestable loolo 




										its makes sense to now 






				yay












	oh my gosh!!!





			ANDDDD technically     ALLL NAMESPACES   ARE NESTED




						because     THE GLOBAL NAMESPACE!!!





					like everything is AT LEAST nested in thereeeee loloolol



								WOWWW okay thast prety cool lololol
					nice 


			i like that 




					thats pretty cool 




					




		oh wait, and also, 




	waittt






		okay so then wait 





			we also have this notion of the currently open, and then not open ones,




			like we need to know 







waittt



			okay so   we know that if we are in a given namespace, we don't need to qualify things, 



				(lets ignore the fact that the global namespace doesnt have a name. in fact, lets give it a name, lets say the global ns is called like "global_namespace" or something,  


					)


					BUTTTT we never have to qualify   things         with that 




								because    WE ARE IN IT




							ALL FILES  and their CONTENTS    are in it 




					so no need to qualify






		but if you are outside the namespace, then you need to qualify things 





		so 



			if you did 







		nsb a 


			add hello


			nsb b

				add cat

			nse


			nsb c

				...use "b cat"....         <---- see how we needed to backtrack to looking at a's ns, 
									to find b, which too us to its ns?... yes. this is important. 

			nse

		nse





	oh, and note, a namespace can be anonymous,  ie, 

		you are allowed to just say 


				nsb  add hello  nse 



			and now, hello is technically entirely local, theres no way of using it out side that scope basically oll 


			quite cool



	this allows you to effectively create public and private interfaces,  for implmentation level stuff, and interface stuff, 



				for example, 










						nsb myinterface

							add function0

							add function1

							nsb 
								...some complex implementation details...
								add function0 value_BLAH          // 
								add function1 value_BLAH          // <--- these two function as assignments.
							nse
						nse
						

						...use "myinterface function0"...

						...use "myinterface function1"...


			







			and see how the implementation stuff is completely hidden and doesnt pollute the symbol table at all, 


				and has all the wonderful scoping semantics that is useful in c,   


						YETTTT you can choose to keep   function0 and function1  accessible from the outside!!







	so yeah, i think we literally just make a stack of open dictionaries

			the global one is the base of the stack,


				and when you open a new   ns             via nsb <name>        you push onto the stack,


					and when you look up a name, you always look backwards through our dictionary, 
							looking for a match, 



							if you see the name of a namespace  that matches,   then its a qualification, 

								we qualify by actually 




		ohhh wowwww



		okay so actually 


			we push ALLL namespacessss    to   MASTER namespace array


				for use later, when we are doing a qualified lookup   


					and then, we actually  push an    index    corresponding to the new ns that we just pushed to the master namespace array


							ie, the stack of namespaces,  is actually just a stack of indicies lool 





							and then calling a namespace name     just sets the current namespace we are looking at, to be that ns index. in the master array.  so yeah. its a tiny bit more complicated, but still totally simple i thinkkk nice 






					




*/





/*

nat stack_i[4096] = {0}, stack_f[4096] = {0}, stack_o[4096] = {0};
	nat stack_count = 0;
	for (nat index = 0; index < text_length; index++) {
		for (nat f = 0; f < file_count; f++) {
			const nat start = files[f].start;
			if (index == start) {				
				stack_i[stack_count] = index;
				stack_f[stack_count] = f;
				stack_o[stack_count++] = 0;
				break;
			} 
			if (stack_o[stack_count - 1] == files[
				stack_f[stack_count - 1]].count)  {
				stack_count--;
				if (not stack_count) goto done; else break;
			}
		}
		if (index == spot) {			
			filename = files[stack_f[stack_count - 1]].name;
			location = stack_o[stack_count - 1];
			goto done;
		}
		stack_o[stack_count - 1]++;
	}



















static void print_source_instruction_mappings(void) {

	for (nat i = 0; i < ins_count; i++) {

		printf("-------------------[ins #%llu]---------------------\n", i);

		printf("\t%llu\tins(.op=%llu (\"%s\"), .size=%lld, args:{ ", 
			i, ins[i].a[0], spelling[ins[i].a[0]], (nat) -1
		);
		for (nat a = 0; a < 3; a++) printf("%llu ", ins[i].a[a + 1]);
		
		printf("}   -- \t[found @   ");
		for (nat a = 0; a < 4; a++) {
			printf("[%llu]:(%llu:%llu)", a, ins[i].start[a], ins[i].count[a]);
			if (a < 3) printf(", ");
		}
		puts("]");


		for (nat a = 0; a < 4; a++) {

			nat start = ins[i].start[a];
			nat count = ins[i].count[a];

			printf("\033[1mlanguage: %s:%llu:%llu:", 
				"filename ? filename : (top-level)", 
				start, count
			);
			printf(" \033[1;32msource:\033[m \033[1m%s%llu\033[m\n", "debugging argument: ", a);

			nat line_begin = start;
			while (line_begin and text[line_begin - 1] != 10) line_begin--;
			nat line_end = start;
			while (line_end < text_length and text[line_end] != 10) line_end++;

			printf("\n\t");
			for (nat c = line_begin; c < line_end; c++) {
				if (c == start) printf("\033[38;5;%llum", 178LLU);
				if (text[c] == 9) putchar(32);
				else putchar(text[c]);
				if (c == start + count) printf("\033[0m");
			}
			printf("\n\t");
			for (nat _ = 0; _ < start - line_begin; _++) 
				printf(" "); 
			printf("\033[32;1m^");
			if (count) {
				for (nat _ = 0; _ < count - 1; _++) 
					printf("~"); 
			}
			printf("\033[0m\n\n"); 

			puts("\n");
		}
		puts("-----------------------------------------");
	}
}







*/





