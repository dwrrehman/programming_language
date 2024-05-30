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
#include <mach-o/nlist.h>
#include <mach-o/loader.h>

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

static bool enable_debug_output = false;

static const nat ct_array_count = 1 << 16;
static const nat ct_memory_count = 1 << 16;

enum language_isa {
	null_instruction, 
	db, dh, dw, dd,
	drop, dup_, over, third, swap, rot, def, arc,  ct, attr, 
	add, addi, sub, slt, slti, slts, sltis, 
	and_, andi,  ior, iori, 
	eor, eori,   sll, slli,  srl, srli, sra, srai, 
	blt, blts, bge, bges, bne, beq, 
	ldb, ldh, ldw, ldd, stb, sth, stw, std, 
	mul, mulh, mulhs, div_, divs, rem, rems, 
	jalr, jal, auipc, ecall, isa_count
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
	"jalr", "jal", "auipc", "ecall"
};


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

enum host_systems { linux, macos };

static u32 arm64_macos_abi[] = {        // note:  x9 is call-clobbered. save it before calls. 
	31,30,31,13,14,15, 7,17,
	29, 9, 0, 1, 2, 3, 4, 5,
	 6,16,19,20,21,22,23,24,
	25,26,27,28,12, 8,11,10,    
};

struct instruction { 
	nat a[4];
	nat start;
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

static nat arg_count = 0;
static nat arguments[4096] = {0};

static nat name_count = 0;
static char* names[4096] = {0};
static nat values[4096] = {0};

static nat* array = NULL;

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

static void print_arguments(void) {
	printf("\narguments[]: { \n");
	for (nat i = 0; i < arg_count; i++) {
		printf("\targuments[%llu] = { %llu  :"
			"  (.start=%llu,.count=%llu)} \n", 
			i, (nat) arguments[i], 
			0LLU, 0LLU
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
		
		printf("} -- [found @ %llu]\n", ins[i].start);

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

static void print_error(const char* reason, nat spot, nat spot2) {
	const int colors[] = {31, 32, 33, 34, 35};
	printf("\033[%dm", colors[1]);
	nat location = 0;
	const char* filename = NULL;
	nat stack_i[4096] = {0}, stack_f[4096] = {0}, stack_o[4096] = {0};
	nat stack_count = 0;
	for (nat index = 0; index < text_length; index++) {
		for (nat f = 0; f < file_count; f++) {
			const nat start = files[f].start;
			const nat count = files[f].count;
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
				print_stack(stack_i, stack_f, stack_o, stack_count);
				printf("file %s reached the end the "
					"file! (stack_o[%llu] == count == %llu)\n", 
				files[f].name, stack_count - 1, count);
				stack_count--;
				if (not stack_count) goto done; else break;
			}
		}
		if (index == spot) {
			printf("\033[38;5;255m(ERROR_HERE:%s:%llu)\033[0m",
			 files[stack_f[stack_count - 1]].name, stack_o[stack_count - 1]);
			filename = files[stack_f[stack_count - 1]].name;
			location = stack_o[stack_count - 1];
			goto done;
		}
		//printf("\033[%dm", colors[stack_count - 1]);
		putchar(text[index]);
		//printf("\033[0m");
		stack_o[stack_count - 1]++;
		//printf("[%s]: incremented stack_o[top=%llu] to be 
		// now %llu...\n", files[stack_f[stack_count - 1]].name, 
		// stack_count - 1, stack_o[stack_count - 1]);
	}
	printf("\033[0m");
done:	
	print_files();
	print_stack(stack_i, stack_f, stack_o, stack_count);
	fprintf(stdout, "\033[1masm: %s:%lld:%lld:", 
		filename ? filename : "(top-level)", 
		location, spot2
	);
	fprintf(stdout, " \033[1;31merror:\033[m \033[1m%s\033[m\n", reason);
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

static void print_index_pair(nat start, nat end) {
	for (nat i = 0; i < text_length; i++) {
		if (i == start or i == end) printf("\033[32;1m");
		putchar(text[i]);
		if (i == start or i == end) printf("\033[0m");
	}
	puts("");
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
	bytes = realloc(bytes, byte_count + 4);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
	bytes[byte_count++] = (u8) (x >> 16);
	bytes[byte_count++] = (u8) (x >> 24);
	bytes[byte_count++] = (u8) (x >> 32);
	bytes[byte_count++] = (u8) (x >> 40);
	bytes[byte_count++] = (u8) (x >> 48);
	bytes[byte_count++] = (u8) (x >> 56);
}

static void check(nat value, nat limit) {
	if (value >= limit) {
		puts("check error");
		print_error("error: sorry bad logic or something", 0, 0);
		abort();
	}
}

static nat r_type(nat* a, nat o, nat f, nat g) {
	check(a[0], 32);
	check(a[1], 32);
	check(a[2], 32);
	return (g << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o;
}

static nat i_type(nat* a, nat o, nat f) { 
	check(a[0], 32);
	check(a[1], 32);
	check(a[2], 1 << 12);
	return (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o;
}

static nat s_type(nat* a, nat o, nat f) {
	check(a[0], 32);
	check(a[1], 32);
	check(a[2], 1 << 12);
	return ((a[0] >> 5U) << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | ((a[0] & 0x1F) << 7U) | o;
}

static nat u_type(nat* a, nat o) { 
	check(a[0], 32);
	check(a[1], 1 << 20);
	return (a[1] << 12U) | (a[0] << 7U) | o;
}

static nat calculate_offset(nat here, nat label) {
	nat offset = 0;
	if (label < here) {
		for (nat i = label; i < here; i++) {
			offset -= ins[i].size;
		}
	} else {
		for (nat i = here; i < label; i++) {
			offset += ins[i].size;
		}
	}
	return offset;
}

static nat j_type(nat here, nat* a, nat o) {   	//  L r op
	check(a[0], 32);
	const nat e = calculate_offset(here, a[1]);
	const nat imm19_12 = (e & 0x000FF000);
	const nat imm11    = (e & 0x00000800) << 9;
	const nat imm10_1  = (e & 0x000007FE) << 20;
	const nat imm20    = (e & 0x00100000) << 11;
	const nat imm = imm20 | imm10_1 | imm11 | imm19_12;
	return (imm << 12U) | (a[0] << 7U) | o;
}

static void generate_riscv_machine_code(void) {

	for (nat i = 0; i < ins_count; i++) {

		const nat op = ins[i].a[0];
		nat* a = ins[i].a + 1;

		     if (op == db)	emitb(a[0]);
		else if (op == dh)	emith(a[0]);
		else if (op == dw)	emitw(a[0]);
		else if (op == dd)	emitd(a[0]);
		else if (op == ecall)   emitw(0x00000073);

		else if (op == add)     emitw(r_type(a, 0x33, 0x0, 0x00));
		else if (op == sub)     emitw(r_type(a, 0x33, 0x0, 0x20));
		else if (op == sll)     emitw(r_type(a, 0x33, 0x1, 0x00));
		else if (op == slts)    emitw(r_type(a, 0x33, 0x2, 0x00));
		else if (op == slt)     emitw(r_type(a, 0x33, 0x3, 0x00));
		else if (op == eor)     emitw(r_type(a, 0x33, 0x4, 0x00));
		else if (op == srl)     emitw(r_type(a, 0x33, 0x5, 0x00));
		else if (op == sra)     emitw(r_type(a, 0x33, 0x5, 0x20));
		else if (op == ior)     emitw(r_type(a, 0x33, 0x6, 0x00));
		else if (op == and_)    emitw(r_type(a, 0x33, 0x7, 0x00));

		else if (op == ldb)      emitw(i_type(a, 0x03, 0x0));
		else if (op == ldh)      emitw(i_type(a, 0x03, 0x1));
		else if (op == ldw)      emitw(i_type(a, 0x03, 0x2));
		else if (op == ldd)      emitw(i_type(a, 0x03, 0x3));
		else if (op == addi)    emitw(i_type(a, 0x13, 0x0));
		else if (op == sltis)   emitw(i_type(a, 0x13, 0x2));
		else if (op == slti)    emitw(i_type(a, 0x13, 0x3));
		else if (op == eori)    emitw(i_type(a, 0x13, 0x4));
		else if (op == iori)    emitw(i_type(a, 0x13, 0x6));
		else if (op == andi)    emitw(i_type(a, 0x13, 0x7));
		else if (op == slli)    emitw(i_type(a, 0x13, 0x1));

		else if (op == srli)    emitw(i_type(a, 0x13, 0x5));   

			// TODO: make this not use the immediate as a bit in the opcode. 
			// toggle this bit if the user gives a sraiw/srai.  comment 
			// version old:(for srai/sraiw, give the appropriate a[2] with imm[10] set.)

		else if (op == jalr)    emitw(i_type(a, 0x67, 0x0));

		else if (op == stb)      emitw(s_type(a, 0x23, 0x0));
		else if (op == sth)      emitw(s_type(a, 0x23, 0x1));
		else if (op == stw)      emitw(s_type(a, 0x23, 0x2));
		else if (op == std)      emitw(s_type(a, 0x23, 0x3));

		// else if (op == lui)     emitw(u_type(a, 0x37));
		else if (op == auipc)   emitw(u_type(a, 0x17));
		else if (op == jal)     emitw(j_type(i, a, 0x6F));

		else {
			printf("error: riscv: unknown runtime instruction: %s : %llu\n", spelling[op], op);
			print_error("unknown instruction", (nat)~0, (nat)~0);
			abort();
		}
	}
}






/////////////////////////////////////////////////



static nat generate_mov(nat Rd, nat op, nat im, 
			nat sf, nat oc, nat sh) {  

	check(Rd, 32);

	Rd = arm64_macos_abi[Rd];

	check(im, 1 << 12U);
	return  (sf << 31U) | 
		(oc << 29U) | 
		(op << 23U) | 
		(sh << 21U) | 
		(im <<  5U) | Rd;
}

static nat generate_addi(nat Rd, nat Rn, nat im, 
			nat op, nat sf, nat sb, 
			nat st, nat sh) {  
	if (not Rd) return 0xD503201F;
	if (not Rn) return generate_mov(Rd, 0x25U, im, sf, 2, 0);
	
	check(Rd, 32);
	check(Rn, 32);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	check(im, 1 << 12U);

	return  (sf << 31U) | 
		(sb << 30U) | 
		(st << 29U) | 
		(op << 23U) | 
		(sh << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd;
}


static nat generate_memiu(nat Rt, nat Rn, nat im, nat op, nat oc, nat sf) {

	check(Rt, 32);
	check(Rn, 32);

	Rt = arm64_macos_abi[Rt];
	Rn = arm64_macos_abi[Rn];

	check(im, 1 << 12U);

	return  (sf << 30U) | 
		(op << 24U) | 
		(oc << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rt;
}

static nat generate_add(nat Rd, nat Rn, nat Rm, 
			nat op, nat im, nat sf, 
			nat st, nat sb, nat sh) {

	check(Rd, 32);
	check(Rn, 32);
	check(Rm, 32);
	check(im, 32U << sf);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];
	Rm = arm64_macos_abi[Rm];

	return  (sf << 31U) |
		(sb << 30U) |
		(st << 29U) |
		(op << 24U) | 
		(sh << 21U) | 
		(Rm << 16U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd;
}

static nat generate_bc(nat condition, nat here, nat target) { 

	printf("target = %llu, here = %llu\n", target, here);
	getchar();
	const nat byte_offset = calculate_offset(here, target) - 4;
	const nat imm = byte_offset >> 2;
	return (0x54U << 24U) | ((0x0007FFFFU & imm) << 5U) | condition;
}

static void emit_and_generate_branch(nat R_left, nat R_right, nat target, nat here, nat condition) {
	emitw(generate_add(0, R_left, R_right, 0x0BU, 0, 1, 1, 1, 0));
	emitw(generate_bc(condition, here, array[target]));
}

static void generate_arm64_machine_code(void) {

	for (nat i = 0; i < ins_count; i++) {
		nat op = ins[i].a[0];
		nat* a = ins[i].a + 1;

		     if (op == db)	emitb(a[0]);
		else if (op == dh)	emith(a[0]);
		else if (op == dw)	emitw(a[0]);
		else if (op == dd)      emitd(a[0]);

		else if (op == ecall)   emitw(0xD4000001);

		else if (op == add)    emitw(generate_add(a[0], a[1], a[2], 0x0BU, 0, 1, 0, 0, 0));
		else if (op == sub)    emitw(generate_add(a[0], a[1], a[2], 0x0BU, 0, 1, 0, 1, 0));
		else if (op == ior)    emitw(generate_add(a[0], a[1], a[2], 0x2AU, 0, 1, 0, 0, 0));
		else if (op == addi)   emitw(generate_addi(a[0], a[1], a[2], 0x22U, 1, 0, 0, 0));

		else if (op == blt)   emit_and_generate_branch(a[0], a[1], a[2], i, 3);
		else if (op == bge)   emit_and_generate_branch(a[0], a[1], a[2], i, 2);

		else if (op == sll)    goto here; 
		else if (op == slts)    goto here;
		else if (op == slt)   goto here;
		else if (op == eor)   goto here;
		else if (op == srl)    goto here;
		else if (op == sra)    goto here;
		else if (op == and_)   goto here;

		else if (op == ldb)     emitw(generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0));
		else if (op == ldh)     emitw(generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0));
		else if (op == ldw)     emitw(generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0));
		else if (op == ldd)     emitw(generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0));

		else if (op == stb)     emitw(generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0));
		else if (op == sth)     emitw(generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0));
		else if (op == stw)     emitw(generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0));
		else if (op == std)     emitw(generate_memiu(a[0], a[1], a[2], 0x00, 0x00, 0));

		else if (op == sltis)   goto here;
		else if (op == slti)  goto here;
		else if (op == eori)   goto here;
		else if (op == iori)    goto here;
		else if (op == andi)   goto here;
		else if (op == slli)   goto here;
		else if (op == srli)   goto here;
		else if (op == jalr)   goto here;
		
		else if (op == auipc)  goto here;
		else if (op == beq)    goto here;
		else if (op == bne)    goto here;
		else if (op == blt)    goto here;
		else if (op == bge)    goto here;
		else if (op == jal)    goto here;
		else {
			here: printf("error: arm64: unknown runtime instruction: %s : %llu\n", spelling[op], op);
			print_error("unknown instruction", (nat) ~0, (nat) ~0);
			abort();
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

#define dd if (enable_debug_output)

int main(int argc, const char** argv) {

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

	dd printf("read file: (length = %llu): \n<<<", text_length);
	dd fwrite(text, 1, text_length, stdout);
	dd puts(">>>");

	array = calloc(ct_array_count, sizeof(nat));
	array[2] = (nat) (void*) calloc(ct_memory_count, sizeof(nat));

	files[file_count].name = filename;
	files[file_count].count = text_length;
	files[file_count++].start = 0;

	arguments[arg_count++] = 0;

	nat defining = 0, skip = 0, is_compiletime = 0, start = 0, count = 0;

	for (nat index = 0; index < text_length; index++) {
		dd printf("%llu: %c...\n", index, text[index]);

		if (not isspace(text[index])) {
			if (not count) start = index;
			count++;

		} else if (count) {
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

				unknown: print_error("undefined word", start, count);
				puts("error: unknown word...");
				print_index_pair(start, index);
				abort();
			}
		process_word:;
			nat a0 = arg_count > 0 ? arguments[arg_count - 1] : 0;
			nat a1 = arg_count > 1 ? arguments[arg_count - 2] : 0;
			nat a2 = arg_count > 2 ? arguments[arg_count - 3] : 0;

			if (op == def) {
				puts("executing def...");
				defining = true;

			} else if (op == drop) { 
				puts("executing drop...");
				arg_count--;

			} else if (op == arc) {
				printf("executing carg(%llu)...\n", a0); 
				arguments[arg_count - 1] = array[a0]; 

			} else if (op == attr) {
				puts("executing atr...");
				array[a0] = is_compiletime ? index : ins_count;
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
				// 1 2 3 -> 2 3 1
				arguments[arg_count - 1] = a2;
				arguments[arg_count - 2] = a0;
				arguments[arg_count - 3] = a1;

			} else if (op == add) {

				arg_count -= 2;
				arguments[arg_count - 1] = a0; 

				if (is_compiletime) {
					printf("executing %llu = add(%llu %llu)\n", a0, a1, a2);
					array[a0] = array[a1] + array[a2];
				} else {
					printf("generating %llu = add(%llu %llu)\n", a0, a1, a2);
					struct instruction new = {0};
					new.a[0] = add;
					new.size = 4;
					new.a[1] = a0;
					new.a[2] = a1;
					new.a[3] = a2;
					new.start = index;
					ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
					ins[ins_count++] = new;		
				}


			} else if (op == addi) {

				arg_count -= 2;
				arguments[arg_count - 1] = a0; 

				if (is_compiletime) {
					printf("executing %llu = addi(%llu #%llu)\n", a0, a1, a2);
					array[a0] = array[a1] + a2;
				} else {
					printf("generating %llu = addi(%llu #%llu)\n", a0, a1, a2);
					struct instruction new = {0};
					new.a[0] = addi;
					new.size = 4;
					new.a[1] = a0;
					new.a[2] = a1;
					new.a[3] = a2;
					new.start = index;
					ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
					ins[ins_count++] = new;		
				}


			} else if (op == sub) {

				arg_count -= 2;
				arguments[arg_count - 1] = a0; 

				if (is_compiletime) {
					printf("executing %llu = sub(%llu %llu)\n", a0, a1, a2);
					array[a0] = array[a1] - array[a2];
				} else {
					printf("generating %llu = sub(%llu %llu)\n", a0, a1, a2);
					struct instruction new = {0};
					new.a[0] = sub;
					new.size = 4;
					new.a[1] = a0;
					new.a[2] = a1;
					new.a[3] = a2;
					new.start = index;
					ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
					ins[ins_count++] = new;		
				}

			} else {
				printf("pushing name %llu on the stack..\n", values[op]);
				arguments[arg_count++] = values[op];
			}
			next: count = 0;
		}
	}
	
	printf("processing the text now...\n");
	puts("DONE: finished assembling program.");
	print_dictionary();
	print_registers();
	print_arguments();
	print_instructions();
	printf("SUCCESSFUL ASSEMBLING\n");

	const nat architecture = arm64;
	const nat output_format = macho_executable;
	const bool debug = true;
	const bool preserve_existing_object = true;
	const bool preserve_existing_executable = true;
	
	const char* object_filename = "object0.o";
	const char* executable_filename = "executable0.out";

	if (debug) {
		printf("info: building for target:\n\tarchitecture:  "
			"\033[31;1m%s\033[0m\n\toutput_format: \033[32;1m%s\033[0m.\n\n", 
			target_spelling[architecture  % target_count], 
			output_format_spelling[output_format % output_format_count]
		);
	}

	if (architecture == noruntime) {
		if (not ins_count) exit(0);
		print_error("encountered runtime instruction with target \"noruntime\"", (nat) ~0, (nat) ~0);
		exit(1);

	} else if (architecture == riscv32 or architecture == riscv64) {
		generate_riscv_machine_code();

	} else if (architecture == arm64) {
		generate_arm64_machine_code();

	} else {
		puts("asm: \033[31;1merror:\033[0m unknown target architecture specified, valid values: "); // TODO: use print_error();
		for (nat i = 0; i < target_count; i++) {
			printf("\t%llu : %s\n", i, target_spelling[i]);
		}
		exit(1);
	}

	if (output_format == print_binary) 
		dump_hex((uint8_t*) bytes, byte_count);

	else if (output_format == elf_objectfile or output_format == elf_executable)
		make_elf_object_file(object_filename);

	else if (output_format == macho_objectfile or output_format == macho_executable) 
		make_macho_object_file(object_filename, preserve_existing_object);
	else {
		puts("asm: \033[31;1merror:\033[0m unknown output format specified, valid values: "); // TODO: use print_error();
		for (nat i = 0; i < output_format_count; i++) {
			printf("\t%llu : %s\n", i, output_format_spelling[i]);
		}
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
*/



