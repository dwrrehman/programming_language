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








remaining features:

	- double check all forwards/backwards branches behavior

	- add bne,beq,bge,blt

	- add ld's and st's for arm64 backend

	- get mulitple files working, to include stdlib's foundation file, which creates constants. 

	- 
	


*/


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
	dup_, over, third, drop, swap, rot, 
	def, arc, cte, attr, 
	add, addi, sub, slt, slti, slts, sltis, 
	and_, andi, ior, iori, 
	eor, eori, sll, slli,  srl, srli, sra, srai, 
	blt, blts, bge, bges, bne, beq, 
	ldb, ldh, ldw, ldd, stb, sth, stw, std, 
	mul, mulh, mulhs, div_, divs, rem, rems, 
	jalr, jal, auipc, ecall, ecm, stl,
	isa_count
};

static const char* spelling[isa_count] = {
	"dup", "over", "third", "drop", "swap", "rot",
	"def", "arc", "cte", "attr", 
	"add", "addi", "sub", "slt", "slti", "slts", "sltis", 
	"and", "andi", "ior", "iori", 
	"eor", "eori",  "sll", "slli",  "srl", "srli","sra", "srai", 
	"blt", "blts", "bge", "bges", "bne", "beq", 
	"ldb", "ldh", "ldw", "ldd", "stb", "sth", "stw", "std", 
	"mul", "mulh", "mulhs", "div", "divs", "rem", "rems", 
	"jalr", "jal", "auipc", "ecall", "ecm", "stl",
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

static char reason[4096] = {0};

static nat byte_count = 0;
static u8* bytes = NULL;

static nat ins_count = 0;
static struct instruction* ins = NULL;

static nat text_length = 0;
static char* text = NULL;

static nat file_count = 0;
static struct file files[4096] = {0};

static nat* array = NULL;

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

static void print_instructions(void) {
	printf("instructions: {\n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t%llu\tins(.op=%llu (\"%s\"), .size=%llu, args:{ ", 
			i, ins[i].a[0], spelling[ins[i].a[0]], ins[i].size
		);
		for (nat a = 1; a < 4; a++) printf("%llu ", ins[i].a[a]);
		
		printf("}   -- \t[found @   ");
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
		printf("\t%llu:\t%016llx = %-7lld\t", i, array[i], array[i]);
	}
	puts("\n}\n");
}

enum diagnostic_type { no_message, error, warning, info, user, debug };

static const char* type_string[] = {
	"(none):", 
	"\033[1;31merror:\033[0m", 
	"\033[1;35mwarning:\033[0m", 
	"\033[1;36minfo:\033[0m", 
	"\033[1;32mdata:\033[0m", 
	"\033[1;33mdebug:\033[0m"
};

static void print_message(nat type, const char* reason_string, nat spot, nat error_length) {
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

	printf("\033[1masm: %s:", filename ? filename : "(invocation)");
	if (location or error_length) printf("%llu:%llu:", location, error_length);
	printf(" %s \033[1m%s\033[m\n", type_string[type], reason_string);
	if (not filename) return;
	if (not spot and not error_length) goto finish;

	nat line_begin = location;
	while (line_begin and text[line_begin - 1] != 10) line_begin--;
	nat line_end = location;
	while (line_end < text_length and text[line_end] != 10) line_end++;

	printf("\n\t");
	for (nat i = line_begin; i < line_end; i++) {
		if (i == location) printf("\033[38;5;%llum", 178LLU);
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
	printf("\033[0m\n"); 
finish: puts("");
}

static char* read_file(const char* name, nat* out_length, nat here) {
	int d = open(name, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }
	const int file = open(name, O_RDONLY, 0);
	if (file < 0) { 
		read_error:;
		snprintf(reason, sizeof reason, "%s: \"%s\"", 
			strerror(errno), name);
		print_message(error, reason, here, 0);
		
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

/*static void emith(nat x) {
	bytes = realloc(bytes, byte_count + 2);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
}*/

static void emitw(nat x) {
	bytes = realloc(bytes, byte_count + 4);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
	bytes[byte_count++] = (u8) (x >> 16);
	bytes[byte_count++] = (u8) (x >> 24);
}

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

static nat calculate_offset(nat here, nat label, struct instruction this) {
	printf("calculate_offset: called using here=%llu, label=%llu...\n", here, label);
	nat offset = 0;
	if (label < here) {
		printf("backwards branch...\n");
		for (nat i = label; i < here; i++) {
			if (i >= ins_count) {
				print_message(error, "invalid label given to a branching instruction", this.start[0], this.count[0]);
				exit(1);
			}
			offset -= ins[i].size;
		}
	} else {
		printf("forwards branch...\n");
		for (nat i = here; i < label; i++) {
			if (i >= ins_count) {
				print_message(error, "invalid label given to a branching instruction", this.start[0], this.count[0]);
				exit(1);
			}
			offset += ins[i].size;
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

		if (op == ecall)   emitw(0x00000073);

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

	

	nat im = calculate_offset(here, array[target], this);

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

	nat im = calculate_offset(here, array[target], this);
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

		if (op == ecm) {
			for (nat n = 0; n < a[1]; n++) emitb(((uint8_t*)a[0])[n]);
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
			here: snprintf(reason, sizeof reason, "arm64: unknown runtime instruction: \"%s\" (%llu)\n", spelling[op], op);
			print_message(error, reason, this.start[0], this.count[0]);
			exit(1);
		}
	}
}

static void make_elf_object_file(const char* object_filename) {
	puts("make_elf_object_file: unimplemented");
	// getchar();
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

static nat ins_size(nat op, nat target) {

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

static void print_source_instruction_mappings(void) {

	for (nat i = 0; i < ins_count; i++) {

		printf("-------------------[ins #%llu]---------------------\n", i);

		printf("\t%llu\tins(.op=%llu (\"%s\"), .size=%llu, args:{ ", 
			i, ins[i].a[0], spelling[ins[i].a[0]], ins[i].size
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

			printf("\033[1masm: %s:%llu:%llu:", 
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

int main(int argc, const char** argv) {

	struct argument arguments[4096] = {0};
	struct argument values[4096] = {0};
	char* names[4096] = {0};
	nat name_count = 0, arg_count = 0;
	
	if (argc != 2) 
		exit(puts("asm: \033[31;1merror:\033[0m "
			"usage: ./asm <source.s>")
		);

	for (nat i = 0; i < isa_count; i++) names[name_count++] = strdup(spelling[i]);

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

	nat stack_size = 0x1000000;
	nat architecture = arm64;
	nat output_format = macho_executable;
	bool preserve_existing_object = false;
	bool preserve_existing_executable = false;
	const char* object_filename = "object0.o";
	const char* executable_filename = "executable0.out";

	for (nat index = 0; index < text_length; index++) {
		if (not isspace(text[index])) {
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;
	process:;
		printf("found word at %llu:%llu... \"%.*s\"\n",  start, count, (int) count, text + start);
		struct argument a0 = arg_count > 0 ? arguments[arg_count - 1] : (struct argument){0};
		struct argument a1 = arg_count > 1 ? arguments[arg_count - 2] : (struct argument){0};
		struct argument a2 = arg_count > 2 ? arguments[arg_count - 3] : (struct argument){0};

		compact_print_arguments(arguments, arg_count);
		char* word = strndup(text + start, count);
		nat op = 0;
		for (nat n = 0; n < name_count; n++) {
			if (not strcmp(names[n], word)) {
				if (defining) {
					snprintf(reason, sizeof reason, "expected "
					"undefined word, word \"%s\" is already "
					"defined", word); print_message(error, 
					reason, start, count); exit(1);
				}
				op = n; goto process_word;
			}
		}
		if (skip) goto next;
		if (not defining) {
			snprintf(reason, sizeof reason, "undefined word \"%s\"", word);
			print_message(error, reason, start, count);
			exit(1);
		} 
		defining = false;
		printf("\033[32mdefining new word\033[0m \"%s\" to be %llu...\n", 
				word, a0.value);
		names[name_count] = word;
		values[name_count++] = a0;
		goto next;

	process_word:;
		if (skip) {
			printf("[in skip mode...]\n");
			if (op == cte) is_compiletime = true;
			else if (op == attr) goto execute_attr;
			else if (values[op].value == skip) goto push_name; 
			goto next;
		}
		
		     if (op == def)   defining = true;
		else if (op == cte)   is_compiletime = true;
		else if (op == drop)  arg_count--;
		else if (op == dup_)  arguments[arg_count++] = a0;
		else if (op == over)  arguments[arg_count++] = a1;
		else if (op == third) arguments[arg_count++] = a2;

		else if (op == swap) {
			arguments[arg_count - 1] = a1;
			arguments[arg_count - 2] = a0;

		} else if (op == rot) {
			arguments[arg_count - 1] = a2;
			arguments[arg_count - 2] = a0;
			arguments[arg_count - 3] = a1;

		} else if (op == arc) {
			printf("executing arc(%llu), replaced with %llu...\n", a0.value, array[a0.value]); 
			arguments[arg_count - 1].value = array[a0.value]; 
			arguments[arg_count - 1].start = start;
			arguments[arg_count - 1].count = count;

		} else if (op == attr) {
			execute_attr: 
			printf("executing attr(--> %llu)...\n", a0.value);
			array[a0.value] = is_compiletime ? index : ins_count;
			printf("loaded array[%llu] with the value %llu...\n", a0.value, array[a0.value]);
			if (skip == a0.value) skip = 0;
			is_compiletime = false;

		} else if (op >= isa_count) {
			push_name:
			printf("\033[31mpushing name %s\033[0m, pushing value %llu on the stack..\n", 
				names[op], values[op].value);

			arguments[arg_count++] = values[op];
			arguments[arg_count - 1].start = start;
			arguments[arg_count - 1].count = count;

		} else {
			if (op == ecall) {}
			else if (op == ecm or op == jalr or op == jal) { 
				if (arg_count < 2) goto er; arg_count -= 2; 

			} else if (op == blt or op == bge or op == bne or
				   op == beq or op == blts or op == bges) { 
				if (arg_count < 3) goto er; arg_count -= 3; 
			
			} else if (op == auipc) { 
				if (arg_count < 2) goto er; arg_count -= 2; 
				arguments[arg_count++] = a0;

			} else { 
				if (arg_count < 3) goto er; arg_count -= 3; 
				arguments[arg_count++] = a0; 
			}

			// printf("after executing: \n");
			// compact_print_arguments(arguments, arg_count);

			if ((1)) {

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
					a0.value, a0.start, a0.count, spelling[op], a1.value, a1.start, a1.count, a2.value, a2.start, a2.count
				);
			}

			if (is_compiletime) { 
				is_compiletime = false;
				nat d = a0.value, r = a1.value, s = a2.value;
				if (op == auipc) array[d] = index + r;
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

				else if (op == ecall) {
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
		}
		next: count = 0;
	}
	if (count) goto process;
	
	printf("processing the text now...\n");
	puts("DONE: finished assembling program.");
	print_dictionary(names, values, name_count);
	print_registers();
	print_arguments(arguments, arg_count);
	print_instructions();
	print_source_instruction_mappings();
	printf("SUCCESSFUL ASSEMBLING\n");

	printf("info: building for target:\n\tarchitecture:  "
		"\033[31;1m%s\033[0m\n\toutput_format: \033[32;1m%s\033[0m.\n\n", 
		target_spelling[architecture  % target_count], 
		output_format_spelling[output_format % output_format_count]
	);

	snprintf(reason, sizeof reason, "this assembler is currently a work in progress");
	print_message(warning, reason, 0, 0);


	if (architecture == noruntime) {
		if (not ins_count) exit(0);
		print_message(error, "encountered runtime instruction with noruntime target", ins[0].start[0], ins[0].count[0]);
		exit(1);

	} else if (architecture == riscv32 or architecture == riscv64) {
		generate_riscv_machine_code();

	} else if (architecture == arm64) {
		generate_arm64_machine_code();

	} else {
		print_message(error, "unknown target architecture specified", ins[0].start[0], ins[0].count[0]);
		exit(1);
	}

	if (output_format == print_binary) 
		dump_hex((uint8_t*) bytes, byte_count);

	else if (output_format == elf_objectfile or output_format == elf_executable)
		make_elf_object_file(object_filename);

	else if (output_format == macho_objectfile or output_format == macho_executable) 
		make_macho_object_file(object_filename, preserve_existing_object);
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


























































/*
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




*/	






