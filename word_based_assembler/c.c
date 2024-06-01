// my assembler: written on 202405304.122455 by dwrr.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <iso646.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdnoreturn.h>



/*


				ADD CONTEXTS FOR NAMES!!!!!!



							lexical stack of them, controlled via ct too!!!






	shifts:     slli   

	math:       mul   mulh     div   rem
*/






static const char* default_string = "hello there from space!\nthis is a test. yay!\n";





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

// static bool enable_debug_output = false;

static const nat ct_array_count = 1 << 16;
static const nat ct_memory_count = 1 << 16;

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

enum host_systems { use_linux, use_macos };

static u32 arm64_macos_abi[] = {        // note:  x9 is call-clobbered. save it before calls. 
	31,30,31,13,14,15, 7,17,
	29, 9, 0, 1, 2, 3, 4, 5,
	 6,16,19,20,21,22,23,24,
	25,26,27,28,12, 8,11,10,    
};

enum language_isa {
	null_instruction, 
	db, dh, dw, dd,
	drop, dup_, over, third, swap, rot, def, arc, ct, attr, 
	add, addi, sub, slt, slti, slts, sltis, 
	and_, andi, ior, iori, 
	eor, eori, sll, slli,  srl, srli, sra, srai, 
	blt, blts, bge, bges, bne, beq, 
	ldb, ldh, ldw, ldd, stb, sth, stw, std, 
	mul, mulh, mulhs, div_, divs, rem, rems, 
	jalr, jal, auipc, ecall, makestring, ctstrlen,
	isa_count
};

static const char* spelling[isa_count] = {
	"null_instruction", 
	"db", "dh", "dw", "dd",
	"drop", "dup", "over", "third", "swap", "rot",
	"def", "arc", "ct", "attr", 
	"add", "addi", "sub", 
	"slt", "slti", "slts", "sltis", 
	"and", "andi", "ior", "iori", 
	"eor", "eori",  "sll", "slli",  "srl", "srli","sra", "srai", 
	"blt", "blts", "bge", "bges", "bne", "beq", 
	"ldb", "ldh", "ldw", "ldd", "stb", "sth", "stw", "std", 
	"mul", "mulh", "mulhs", "div", "divs", "rem", "rems", 
	"jalr", "jal", "auipc", "ecall", "makestring", "ctstrlen",
};

struct argument {
	nat value;
	nat start;
	nat count;
};

struct instruction { 
	nat a[4];
	nat start[4];
	nat count[4];
	nat size;
};
 
struct file {
	nat start;
	nat count;
	const char* name;
};

static nat byte_count = 0;
static u8* bytes = NULL;

static nat ins_count = 0;
static struct instruction* ins = NULL;

static nat text_length = 0;
static char* text = NULL;

static nat file_count = 0;
static struct file files[4096] = {0};

static nat* array = NULL;



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


static void print_arguments(void) {
	printf("\narguments[]: { \n");
	for (nat i = 0; i < arg_count; i++) {
		printf("\targuments[%llu] = { %llu  :"
			"  (.start=%llu,.count=%llu)} \n", 
			i, (nat) arguments[i], 
			argument_locations[2 * i + 0], 
			argument_locations[2 * i + 1]
		);
	}
	puts("} \n");
}

static void print_dictionary(void) {
	puts("printing dictionary...");
	for (nat i = 0; i < name_count; i++) {
		printf("\t#%llu: name %s  ... value %llu\n", i, 
			names[i], values[i]
		);
	}
	puts("done.");
}

static void print_instructions(void) {
	printf("instructions: {\n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t%llu\tins(.op=%llu (\"%s\"), .size=%llu, args:{ ", 
			i, ins[i].a[0], spelling[ins[i].a[0]], ins[i].size
		);
		for (nat a = 0; a < 3; a++) printf("%llu ", ins[i].a[a + 1]);
		
		printf("} -- [found @ \n");
		for (nat a = 0; a < 4; a++) {
			printf("[%llu]:(%llu:%llu)", a, ins[i].start[a], ins[i].count[a]);
			if (a < 3) printf(", ");
		}
		puts("]");
	}
	puts("}");
}

static void print_registers(void) {
	printf("registers: {\n");
	for (nat i = 0; i < 32; i++) {
		if (i % 2 == 0) puts("");
		printf("\t%llu:\t%016llx\t", i, array[i]);
	}
	puts("\n}\n");
}

static void print_error(const char* reason, nat spot, nat error_length) {
	// const int colors[] = {31, 32, 33, 34, 35};
	//printf("\033[%dm", colors[1]);
	nat location = 0;
	const char* filename = NULL;
	nat stack_i[4096] = {0}, stack_f[4096] = {0}, stack_o[4096] = {0};
	nat stack_count = 0;
	for (nat index = 0; index < text_length; index++) {
		for (nat f = 0; f < file_count; f++) {
			const nat start = files[f].start;
			// const nat count = files[f].count;
			if (index == start) {
				//printf("file %s begins at %llu!\n", 
				//files[f].name, index);
				stack_i[stack_count] = index;
				stack_f[stack_count] = f;
				stack_o[stack_count++] = 0;
				break;
			} 
			if (stack_o[stack_count - 1] == files[
				stack_f[stack_count - 1]].count)  {
				//print_stack(stack_i, stack_f, stack_o, stack_count);
				//printf("file %s reached the end the "
				//	"file! (stack_o[%llu] == count == %llu)\n", 
				// files[f].name, stack_count - 1, count);
				stack_count--;
				if (not stack_count) goto done; else break;
			}
		}
		if (index == spot) {
			//printf("\033[38;5;255m(ERROR_HERE:%s:%llu)\033[0m",
			// files[stack_f[stack_count - 1]].name, stack_o[stack_count - 1]);
			filename = files[stack_f[stack_count - 1]].name;
			location = stack_o[stack_count - 1];
			goto done;
		}
		//printf("\033[%dm", colors[stack_count - 1]);
		// putchar(text[index]);
		//printf("\033[0m");
		stack_o[stack_count - 1]++;
		//printf("[%s]: incremented stack_o[top=%llu] to be 
		// now %llu...\n", files[stack_f[stack_count - 1]].name, 
		// stack_count - 1, stack_o[stack_count - 1]);
	}
	// printf("\033[0m");
done:;

	// print_files();
	// print_stack(stack_i, stack_f, stack_o, stack_count);


	printf("\033[1masm: %s:%lld:%lld:", 
		filename ? filename : "(top-level)", 
		location, error_length
	);
	printf(" \033[1;31merror:\033[m \033[1m%s\033[m\n", reason);

	nat line_begin = location;
	while (line_begin and text[line_begin - 1] != 10) line_begin--;
	nat line_end = location;
	while (line_end < text_length and text[line_end] != 10) line_end++;

	printf("\n\t");
	for (nat i = line_begin; i < line_end; i++) {
		if (i == location) printf("\033[33;1m");
		if (text[i] == 9) putchar(32);
		else putchar(text[i]);
		if (i == location + error_length) printf("\033[0m");
	}
	printf("\n\t");
	for (nat i = 0; i < location - line_begin; i++) 
		printf(" "); 
	printf("\033[32;1m^");
	if (error_length) {
		for (nat i = 0; i < error_length - 1; i++) 
			printf("~"); 
	}
	printf("\033[0m\n\n"); 
}

static char* read_file(const char* name, nat* out_length, nat here) {
	int d = open(name, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }
	const int file = open(name, O_RDONLY, 0);
	if (file < 0) { 
		read_error:;
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "%s: \"%s\"", 
			strerror(errno), name);
		print_error(reason, here, 0);
		fprintf(stdout, "asm: \033[31;1merror:\033[0m %s: "
				"\"%s\"\n", strerror(errno), name); 
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


static void dump_hex(uint8_t* local_bytes, nat local_byte_count) {
	printf("\ndebugging bytes bytes:\n------------------------\n");
	printf("dumping hex bytes: (%llu)\n", local_byte_count);
	for (nat i = 0; i < local_byte_count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx ", local_bytes[i]);
	}
	puts("");
}

static void emitb(nat x) {
	bytes = realloc(bytes, byte_count + 1);
	bytes[byte_count++] = (u8) (x >> 0);
}

static void emith(nat x) {
	bytes = realloc(bytes, byte_count + 2);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
}

static void emitw(nat x) {
	bytes = realloc(bytes, byte_count + 4);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
	bytes[byte_count++] = (u8) (x >> 16);
	bytes[byte_count++] = (u8) (x >> 24);
}

static void emitd(nat x) {
	bytes = realloc(bytes, byte_count + 8);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
	bytes[byte_count++] = (u8) (x >> 16);
	bytes[byte_count++] = (u8) (x >> 24);
	bytes[byte_count++] = (u8) (x >> 32);
	bytes[byte_count++] = (u8) (x >> 40);
	bytes[byte_count++] = (u8) (x >> 48);
	bytes[byte_count++] = (u8) (x >> 56);
}

static void check(nat value, nat limit, struct instruction this) {
	if (value >= limit) {
		puts("check error");
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "check: value %llu >= limit %llu check did not succeed for instruction", value, limit);
		print_error(reason, this.start[0], this.count[0]);
		exit(1);
	}
}


static void check_offset(nat value, nat limit, struct instruction this) {
	if ((0)) {
		puts("check_offset error");
		print_error("error: sorry bad logic or something", this.start[0], this.count[0]);
		exit(1);
	}
}

static void r_type(nat* a, nat o, nat f, nat g, struct instruction this) {
	check(a[0], 32, this);
	check(a[1], 32, this);
	check(a[2], 32, this);
	emitw( (g << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o);
}

static void i_type(nat* a, nat o, nat f, struct instruction this) { 
	check(a[0], 32, this);
	check(a[1], 32, this);
	check(a[2], 1 << 12, this);
	emitw( (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o);
}

static void s_type(nat* a, nat o, nat f, struct instruction this) {
	check(a[0], 32, this);
	check(a[1], 32, this);
	check(a[2], 1 << 12, this);
	emitw( ((a[0] >> 5U) << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | ((a[0] & 0x1F) << 7U) | o);
}

static void u_type(nat* a, nat o, struct instruction this) { 
	check(a[0], 32, this);
	check(a[1], 1 << 20, this);
	emitw( (a[1] << 12U) | (a[0] << 7U) | o);
}

static nat calculate_offset(nat here, nat label, struct instruction this) {
	printf("calculate_offset: called using here=%llu, label=%llu...\n", here, label);
	nat offset = 0;
	if (label < here) {
		printf("backwards branch...\n");
		for (nat i = label; i < here; i++) {
			if (i >= ins_count) {
				print_error("invalid label given to a branching instruction", this.start[0], this.count[0]);
				abort();
			}
			offset -= ins[i].size;
		}
	} else {
		printf("forwards branch...\n");
		for (nat i = here; i < label; i++) {
			if (i >= ins_count) {
				print_error("invalid label given to a branching instruction", this.start[0], this.count[0]);
				abort();
			}
			offset += ins[i].size;
		}
	}
	printf("output: found an offset of %llu bytes.\n", offset);
	return offset;
}

static void j_type(nat here, nat* a, nat o, struct instruction this) {
	
	const nat e = calculate_offset(here, a[1], this);

	check(a[0], 32, this);
	check_offset(e, 1 << 0, this);

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

		     if (op == db)	emitb(a[0]);
		else if (op == dh)	emith(a[0]);
		else if (op == dw)	emitw(a[0]);
		else if (op == dd)	emitd(a[0]);
		else if (op == ecall)   emitw(0x00000073);

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
		else if (op == addi)    i_type(a, 0x13, 0x0, this);
		else if (op == sltis)   i_type(a, 0x13, 0x2, this);
		else if (op == slti)    i_type(a, 0x13, 0x3, this);
		else if (op == eori)    i_type(a, 0x13, 0x4, this);
		else if (op == iori)    i_type(a, 0x13, 0x6, this);
		else if (op == andi)    i_type(a, 0x13, 0x7, this);
		else if (op == slli)    i_type(a, 0x13, 0x1, this);
		else if (op == srli)    i_type(a, 0x13, 0x5, this);
		else if (op == srai)    i_type(a, 0x13, 0x5, this);
		else if (op == jalr)    i_type(a, 0x67, 0x0, this);

		else if (op == stb)      s_type(a, 0x23, 0x0, this);
		else if (op == sth)      s_type(a, 0x23, 0x1, this);
		else if (op == stw)      s_type(a, 0x23, 0x2, this);
		else if (op == std)      s_type(a, 0x23, 0x3, this);

		// else if (op == lui)  u_type(a, 0x37, this);
		else if (op == auipc)   u_type(a, 0x17, this);
		else if (op == jal)     j_type(i, a, 0x6F, this);

		else {
			char reason[4096] = {0};
			snprintf(reason, sizeof reason, "riscv: unknown runtime instruction: \"%s\" (%llu)\n", spelling[op], op);
			print_error(reason, this.start[0], this.count[0]);
			exit(1);
		}
	}
}






/////////////////////////////////////////////////



static void generate_mov(nat Rd, nat op, nat im, 
			nat sf, nat oc, nat sh, 
		struct instruction this
) {  

	check(Rd, 32, this);

	Rd = arm64_macos_abi[Rd];

	check(im, 1 << 12U, this);
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
	
	check(Rd, 32, this);
	check(Rn, 32, this);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	check(im, 1 << 12U, this);

	emitw(  (sf << 31U) | 
		(sb << 30U) | 
		(st << 29U) | 
		(op << 23U) | 
		(sh << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd);
}


static void generate_memiu(nat Rt, nat Rn, nat im, nat op, nat oc, nat sf, struct instruction this) {

	check(Rt, 32, this);
	check(Rn, 32, this);

	Rt = arm64_macos_abi[Rt];
	Rn = arm64_macos_abi[Rn];

	check(im, 1 << 12U, this);

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

	check(Rd, 32, this);
	check(Rn, 32, this);
	check(Rm, 32, this);
	check(im, 32U << sf, this);

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
	check(Rd, 32, this);
	check(Rn, 32, this);
	check(Rm, 32, this);

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

	check(iv,  2, this);
	check(ic,  2, this);
	check(cd, 16, this);
	check(Rd, 32, this);
	check(Rn, 32, this);
	check(Rm, 32, this);

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

	check(Rd, 32, this);
	check(Rn, 32, this);
	check(im, 64, this);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	nat nn = sf, is = 63 - im, ir = (64 - im) % 64;
	printf("generating ubfm using the values: \n");
	printf("\t sf = %lld\n", sf);
	printf("\t N = %lld\n", nn);
	printf("\t immr = %lld\n", ir);
	printf("\t imms = %lld\n", is);

	emitw(  (sf << 31U) | 
		(oc << 29U) | 
		(op << 23U) | 
		(nn << 22U) | 
		(ir << 16U) | 
		(is << 10U) | 
		(Rn <<  5U) | Rd);
}


static void generate_srli(nat Rd, nat Rn, nat im, nat op, nat oc, nat sf, struct instruction this) {

	check(Rd, 32, this);
	check(Rn, 32, this);
	check(im, 64, this);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	nat nn = sf, is = 63, ir = im;
	printf("generating ubfm using the values: \n");
	printf("\t sf = %lld\n", sf);
	printf("\t N = %lld\n", nn);
	printf("\t immr = %lld\n", ir);
	printf("\t imms = %lld\n", is);

	emitw(  (sf << 31U) | 
		(oc << 29U) | 
		(op << 23U) | 
		(nn << 22U) | 
		(ir << 16U) | 
		(is << 10U) | 
		(Rn <<  5U) | Rd);
}

static void generate_adr(nat Rd, nat op, nat oc, nat target, nat here, struct instruction this) { 

	nat im = calculate_offset(here, array[target], this);

	check(Rd, 32, this);
	check_offset(im, 1 << 20, this);

	Rd = arm64_macos_abi[Rd];

	nat lo = 0x03U & im, hi = 0x07FFFFU & (im >> 2);
	emitw(  (oc << 31U) | 
		(lo << 29U) | 
		(op << 24U) | 
		(hi <<  5U) | Rd);
}

static void generate_jalr(nat Rd, nat Rn, nat op, struct instruction this) {

	check(Rd, 32, this);
	check(Rn, 32, this);

	nat oc = Rd;
	if (Rd != 0 and Rd != 1) {
		print_error("non return address register destination specified, but not supported yet", 0, 0);
		abort();
	}

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	emitw( ((nat)(Rd == 1) << 22) | (oc << 21U) | (op << 10U) | (Rn << 5U));
}

static void generate_jal(nat Rd, nat here, nat target, struct instruction this) {  

	nat oc = Rd;
	if (Rd != 0 and Rd != 1) {
		print_error("non return address register destination specified, but not supported yet", 0, 0);
		abort();
	}

	nat im = calculate_offset(here, array[target], this);
	check_offset(im, 1 << 25, this);
	emitw( (oc << 31U) | (0x05 << 26U) | (0x03FFFFFFU & im));
}

static void generate_bc(nat condition, nat here, nat target, struct instruction this) {
	const nat byte_offset = calculate_offset(here, target, this);
	const nat imm = byte_offset >> 2;
	check_offset(imm, 1 << 0, this);
	emitw((0x54U << 24U) | ((0x0007FFFFU & imm) << 5U) | condition);
}

static void generate_branch(nat R_left, nat R_right, nat target, nat here, nat condition, struct instruction this) {
	generate_add(0, R_left, R_right, 0x0BU, 0, 1, 1, 1, 0, this);
	generate_bc(condition, here, array[target], this);
}

static void generate_slt(nat Rd, nat Rn, nat Rm, nat cond, struct instruction this) {
	generate_add(0, Rn, Rm, 0x0BU, 0, 1, 1, 1, 0, this);
	generate_csel(Rd, 0, 0, cond, 0x0D4U, 1, 1, 0, this);
}

static void generate_arm64_machine_code(void) {

	for (nat i = 0; i < ins_count; i++) {

		struct instruction this = ins[i];
		nat op = this.a[0];
		nat* a = this.a + 1;

		     if (op == db)	emitb(a[0]);
		else if (op == dh)	emith(a[0]);
		else if (op == dw)	emitw(a[0]);
		else if (op == dd)      emitd(a[0]);

		else if (op == makestring) {
			for (nat c = 0; c < strlen(default_string) + 1; c++) {
				emitb((nat) default_string[c]);
			}
		}

		else if (op == ecall)  emitw(0xD4000001);

		else if (op == addi)   generate_addi(a[0], a[1], a[2], 0x22U, 1, 0, 0, 0, this);
		else if (op == slli)   generate_slli(a[0], a[1], a[2], 0x26U, 2, 1, this);
		else if (op == srli)   generate_srli(a[0], a[1], a[2], 0x26U, 2, 1, this);
		else if (op == srai)   generate_srli(a[0], a[1], a[2], 0x26U, 0, 1, this);


		else if (op == div_)   generate_adc(a[0], a[1], a[2], 0xD6, 2, 1, 0, 0, this);
		else if (op == divs)   generate_adc(a[0], a[1], a[2], 0xD6, 3, 1, 0, 0, this);
		else if (op == mulh)   generate_adc(a[0], a[1], a[2], 0xDE, 31, 1, 0, 0, this);

		// else if (op == mul)    generate_umaddl(a[0], a[1], a[2], 0xDE, 31, 1, 0, 0, this);

		else if (op == auipc)  generate_adr(a[0], 0x10, 0, a[1], i, this);

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

		else if (op == sltis)  goto here;
		else if (op == slti)   goto here;
		else if (op == eori)   goto here;
		else if (op == iori)   goto here;
		else if (op == andi)   goto here;
		else if (op == srli)   goto here;

		else {
			here:;
			char reason[4096] = {0};
			snprintf(reason, sizeof reason, "arm64: unknown runtime instruction: %s : %llu", spelling[op], op);
			print_error(reason, this.start[0], this.count[0]);
			exit(1);
		}
	}
}

static void make_elf_object_file(const char* object_filename) {
	puts("make_elf_object_file: unimplemented");
	getchar();
	const int flags = O_WRONLY | O_CREAT | O_TRUNC | O_EXCL;
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(object_filename, flags, mode);
	if (file < 0) { perror("obj:open"); exit(1); }
	write(file, NULL, 0);
	write(file, NULL, 0);
	write(file, NULL, 0);
	close(file);
}

static void make_macho_object_file(const char* object_filename, const bool preserve_existing_object) {

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
	            .n_value = 0,
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



static bool execute(nat op, nat a0, nat a1, nat a2, nat index, nat count) {

}


static void execute_branch(nat op, nat a0, nat a1, nat a2, nat* skip, nat* index) {
	if ((0)) {}
	
}

static nat ins_size(nat op, nat target) {

	if (op == makestring) return strlen(default_string);

	if (op == db) return 1;
	if (op == dh) return 2;
	if (op == dw) return 4;
	if (op == dd) return 8;
	if (target == arm64) {
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

static void push_ins(

	nat op, 
	nat a0, 
	nat a1, 
	nat a2, 

	nat so, nat co, 
	nat s0, nat c0, 
	nat s1, nat c1, 
	nat s2, nat c2,	

	nat size
) {
	struct instruction new = {0};
	
	new.a[0] = op;
	new.a[1] = a0; 
	new.a[2] = a1; 
	new.a[3] = a2;

	new.start[0] = so;
	new.count[0] = co;

	new.start[1] = s0;
	new.count[1] = c0;

	new.start[2] = s1;
	new.count[2] = c1;

	new.start[3] = s2;
	new.count[3] = c2;

	new.size = size;
	
	ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
	ins[ins_count++] = new;
}

int main(int argc, const char** argv) {

	nat arg_count = 0;
	struct argument arguments[4096] = {0};

	nat name_count = 0;
	char* names[4096] = {0};
	nat values[4096] = {0};

	if (argc != 2) exit(puts("asm: \033[31;1merror:\033[0m "
				"usage: ./asm <source.s>"));

	for (nat i = 0; i < isa_count; i++) {
		names[name_count] = strdup(spelling[i]);
		values[name_count] = 0;
		name_count++;
	}

	const char* filename = argv[1];
	text_length = 0;
	text = read_file(filename, &text_length, 0);

	printf("read file: (length = %llu): \n<<<", text_length);
	fwrite(text, 1, text_length, stdout);
	puts(">>>");

	array = calloc(ct_array_count, sizeof(nat));
	array[2] = (nat) (void*) calloc(ct_memory_count, sizeof(nat));

	files[file_count].name = filename;
	files[file_count].count = text_length;
	files[file_count++].start = 0;

	bool defining = 0, is_compiletime = 0;
	nat skip = 0, start = 0, count = 0;

	nat architecture = arm64;
	nat output_format = macho_executable;
	bool debug = true;
	bool preserve_existing_object = false;
	bool preserve_existing_executable = false;
	const char* object_filename = "object0.o";
	const char* executable_filename = "executable0.out";

	for (nat index = 0; index < text_length; index++) {
		printf("%llu: %c...\n", index, text[index]);

		if (not isspace(text[index])) {
			if (not count) start = index;
			count++;

		} else if (count) {
			process:
			printf("found word at %llu:%llu... \"%.*s\"\n", 
				start, count, (int) count, text + start);
			
			char* word = strndup(text + start, count);

			nat op = 0;
			for (nat i = 0; i < name_count; i++) {
				if (not strcmp(names[i], word)) {
					op = i;
					goto process_word;
				}
			}
			
			if (defining) {
				printf("defining new word \"%s\"...\n", word);
				names[name_count] = word;
				values[name_count] = arguments[arg_count - 1];
				name_count++;
				defining = false;
				goto next;
			} else {
				nat r = 0, s = 1;
				for (nat i = 0; i < count; i++) {
					if (word[i] == '0') s <<= 1;
					else if (word[i] == '1') { r += s; s <<= 1; }
					else goto unknown;
				}

				printf("pushing literal %llu on the stack..\n", values[op]);
				arguments[arg_count++] = r;
				goto next;

				unknown:;
				char reason[4096] = {0};
				snprintf(reason, sizeof reason, "undefined word \"%s\"", word);
				print_error(reason, start, count);
				exit(1);
			}
		process_word:;
			struct argument a0 = arguments[arg_count - 1];
			struct argument a1 = arguments[arg_count - 2];
			struct argument a2 = arguments[arg_count - 3];

			if (op == def) {
				puts("executing def...");
				defining = true;

			} else if (op == ct) {
				puts("executing ct...");
				is_compiletime = true;

			} else if (op == drop) { 
				puts("executing drop...");
				arg_count--;

			} else if (op == arc) {
				printf("executing arc(%llu)...\n", a0); 
				arguments[arg_count - 1].value = array[a0]; 
				arguments[arg_count - 1].start = start;
				arguments[arg_count - 1].count = count;

			} else if (op == attr) {
				printf("executing attr(--> %llu)...\n", a0);
				array[a0] = is_compiletime ? index : ins_count;
				printf("loaded array[%llu] with the value %llu...\n", a0, array[a0]);
				if (skip == a0) skip = 0;
				is_compiletime = false;

			} else if (op == dup_) {
				puts("executing dup...");
				arguments[arg_count++] = a0;

			} else if (op == over) {
				puts("executing over...");
				arguments[arg_count++] = a1;

			} else if (op == third) {
				puts("executing third...");
				arguments[arg_count++] = a2;

			} else if (op == swap) {
				puts("executing swap..."); 
				arguments[arg_count - 1] = a1;
				arguments[arg_count - 2] = a0;

			} else if (op == rot) {
				puts("executing rot...");
				arguments[arg_count - 1] = a2;
				arguments[arg_count - 2] = a0;
				arguments[arg_count - 3] = a1;

			} else if (op == ctstrlen) {
				puts("executing ctstrlen...");
				array[a0] = strlen(default_string);


			} else if (op < isa_count) {

				if (op == ecall or op == makestring) {}
				else if (op == db or op == dh or op == dw or op == dd) arg_count--;
				else if (op == jalr or op == jal) arg_count -= 2;
				else if (op == blt  or op == bge or op == bne  or op == beq or
					 op == blts or op == bges) arg_count -= 3;
				else if (op == auipc) { arg_count--; arguments[arg_count - 1] = a0; } 
				else { arg_count -= 2; arguments[arg_count - 1] = a0; }

				if (	op == blt  or op == bge or
					op == bne  or op == beq or
					op == blts or op == bges

				)	printf("%s %s(%llu %llu --> @%llu)\n",
						is_compiletime
							? "executing compiletime" 
							: "generating runtime",
						spelling[op], a0, a1, a2
					);
				else 
					printf("%s %llu = %s(%llu %llu)\n", 
						is_compiletime
							? "executing compiletime" 
							: "generating runtime",
						a0, spelling[op], a1, a2
					);


				if (is_compiletime) { 

					if (op == auipc) array[a0] = index + a2;

	else if (op == addi)  array[a0] = array[a1] + a2;
	else if (op == slti)  array[a0] = array[a1] < a2;
	else if (op == iori)  array[a0] = array[a1] | a2;
	else if (op == eori)  array[a0] = array[a1] ^ a2;
	else if (op == andi)  array[a0] = array[a1] & a2;
	else if (op == slli)  array[a0] = array[a1] << a2;
	else if (op == srli)  array[a0] = array[a1] >> a2;
	else if (op == srai)  array[a0] = array[a1] >> a2;

	else if (op == add)   array[a0] = array[a1] + array[a2];
	else if (op == sub)   array[a0] = array[a1] - array[a2];
	else if (op == ior)   array[a0] = array[a1] | array[a2];
	else if (op == eor)   array[a0] = array[a1] ^ array[a2];
	else if (op == and_)  array[a0] = array[a1] & array[a2];
	else if (op == slt)   array[a0] = array[a1] < array[a2];
	else if (op == slts)  array[a0] = array[a1] < array[a2];
	else if (op == sll)   array[a0] = array[a1] << array[a2];
	else if (op == srl)   array[a0] = array[a1] >> array[a2];
	else if (op == sra)   array[a0] = array[a1] >> array[a2];

	else if (op == ecall) {
		if (a0 == 1) return true;
		else if (a0 == 2) printf("debug: %lld (hex 0x%016llx)\n", array[a1], array[a1]);
		else if (a0 == 3) array[a1] = (nat) getchar();
		else if (a0 == 4) putchar((char) array[a1]);
		else if (a0 == 5) print_dictionary();
		else if (a0 == 6) print_instructions();
		else if (a0 == 7) print_registers();
		else if (a0 == 8) print_arguments();
		else {
			char reason[4096] = {0};
			snprintf(reason, sizeof reason, "unknown compiletime system call number %llu", a0);
			print_error(reason, index - count, count);
			exit(1);
		}
	}

	else if (op == blt)  { if (array[a0] <  array[a1]) goto jump; } 
	else if (op == bge)  { if (array[a0] >= array[a1]) goto jump; } 
	else if (op == bne)  { if (array[a0] != array[a1]) goto jump; } 
	else if (op == beq)  { if (array[a0] == array[a1]) goto jump; } 
	else if (op == blts) { if (array[a0] <  array[a1]) goto jump; } 
	else if (op == bges) { if (array[a0] >= array[a1]) goto jump; }
	else if (op == jalr) {
		array[a0] = *index;
		*index = array[a1] + a2;
	} else if (op == jal) {
		array[a0] = *index;
		if (array[a1]) *index = array[a1]; 
		else *skip = a1;
	} 

	else { puts("error: internal ct execute error"); abort(); }
	return;

jump: 	if (array[a2]) *index = array[a2]; 
	else *skip = a2;






					is_compiletime = false; 
				}



				else 	push_ins(op, a0, a1, a2, 

						start, count, 
						argument_locations[2 * (arg_count - 1) + 0],
						argument_locations[2 * (arg_count - 1) + 1],
						0,0,0,0,

						ins_size(op, architecture)
					);
			

			} else {
				printf("pushing name %llu on the stack..\n", values[op]);
				arguments[arg_count++] = values[op];
			}
			next: count = 0;
		}
	}
	if (count) goto process;
	
	printf("processing the text now...\n");
	puts("DONE: finished assembling program.");
	print_dictionary();
	print_registers();
	print_arguments();
	print_instructions();
	printf("SUCCESSFUL ASSEMBLING\n");

	if (debug) {
		printf("info: building for target:\n\tarchitecture:  "
			"\033[31;1m%s\033[0m\n\toutput_format: \033[32;1m%s\033[0m.\n\n", 
			target_spelling[architecture  % target_count], 
			output_format_spelling[output_format % output_format_count]
		);
	}

	if (architecture == noruntime) {
		if (not ins_count) exit(0);
		print_error("encountered runtime instruction with target \"noruntime\"", ins[0].start[0], ins[0].count[0]);
		exit(1);

	} else if (architecture == riscv32 or architecture == riscv64) {
		generate_riscv_machine_code();

	} else if (architecture == arm64) {
		generate_arm64_machine_code();

	} else {
		print_error("unknown target architecture specified", ins[0].start[0], ins[0].count[0]);
		exit(1);
	}

	if (output_format == print_binary) 
		dump_hex((uint8_t*) bytes, byte_count);

	else if (output_format == elf_objectfile or output_format == elf_executable)
		make_elf_object_file(object_filename);

	else if (output_format == macho_objectfile or output_format == macho_executable) 
		make_macho_object_file(object_filename, preserve_existing_object);
	else {
		print_error("unknown output format specified", ins[0].start[0], ins[0].count[0]);
		exit(1);
	}

	if (output_format == elf_executable or output_format == macho_executable) {

		if (preserve_existing_executable and not access(executable_filename, F_OK)) {
			puts("asm: executable_file: file exists");  // TODO: use print_error();
			puts(executable_filename);
			exit(1);
		}
		
		char link_command[4096] = {0};
		snprintf(link_command, sizeof link_command, "ld -S -x " //  -v
			"-dead_strip "
			"-no_weak_imports "
			"-fatal_warnings "
			"-no_eh_labels "
			"-warn_compact_unwind "
			"-warn_unused_dylibs "
			"%s -o %s "
			"-arch arm64 "
			"-e _start "
			"-stack_size 0x1000000 "
			"-platform_version macos 13.0.0 13.3 "
			"-lSystem "
			"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk ", 
			object_filename, executable_filename
		);
		system(link_command);
	}

	if (debug) {
		system("otool -txvVhlL object0.o");
		system("otool -txvVhlL executable0.out");
		system("objdump object0.o -DSast --disassembler-options=no-aliases");
		system("objdump executable0.out -DSast --disassembler-options=no-aliases");
	}
}






































































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






*/








