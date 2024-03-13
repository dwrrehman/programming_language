/*
		risc-v 64-bit cross assembler 
	     written by dwrr on 202403111.010146

	this is made to be my primary cross-platform programming language, 
	to ideally my use of replace C for programming most of my projects.
	only the risc-v target has performance guarantees. other targets
	use a translation layer to translate the risc-v ISA instructions
	into the target's ISA (eg, arm64, arm32, x86, x86_64).

*/

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
#include <mach-o/nlist.h>
#include <mach-o/loader.h>

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

static bool debug = false;

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

enum language_ISA {
	dw, ecall, ebreak, fence,  // base ISA.
	fencei, add, sub, sll, 
	slt, sltu, xor_, srl, 
	sra, or_, and_, addw, 
	subw, sllw, srlw, sraw,
	lb, lh, lw, ld, 
	lbu, lhu, lwu, addi, 
	slti, sltiu, xori, ori, 
	andi, slli, srli, srai, 
	addiw, slliw, srliw, sraiw,
	jalr, csrrw, csrrs, csrrc, 
	csrrwi, csrrsi, csrrci, sb, 
	sh, sw, sd, lui, 
	auipc, beq, bne, blt, 
	bge, bltu, bgeu, jal, 

	mul, mulh, mulhsu, mulhu,       //  M extension
	div_, divu, rem, remu, 
	mulw, divw, divuw, remw, 
	remuw,

	dh, 
	cnop, caddi4spn, clw, cld, csw, csd,   // C extension
	caddi, cjal, caddiw, cli, caddi16sp, 
	clui, csrli, candi, csub, cxor, cor, 
	cand, csubw, caddw, cj, cbeqz, cbnez, 
	cslli, clwsp, cldsp, cjr, cmv, cebreak, 
	cjalr, cadd, cswsp, csdsp, 

	db, 

	ctclear, ctdel, ctls, ctarg, ctli, ctstop,   // compiletime system.
	ctat, ctpc, ctb, ctf, ctblt,
	ctbge, ctbeq, ctbne, ctincr, ctzero,
	ctadd, ctsub, ctmul, ctdiv, ctrem, ctnor, ctxor, ctand, ctor, 
	ctsl, ctsr, ctlb, ctlh, ctlw,
	ctld, ctsb, ctsh, ctsw, ctsd,
	ctprint, ctabort, ctget, ctput,
	instruction_set_count
};
static const char* instruction_spelling[instruction_set_count] = {
	"u32", "ecall", "ebreak", "fence", 
	"fencei", "add", "sub", "sll", 
	"slt", "sltu", "xor", "srl", 
	"sra", "or", "and", "addw", 
	"subw", "sllw", "srlw", "sraw",
	"lb", "lh", "lw", "ld", 
	"lbu", "lhu", "lwu", "addi", 
	"slti", "sltiu", "xori", "ori", 
	"andi", "slli", "srli", "srai", 
	"addiw", "slliw", "srliw", "sraiw",
	"jalr", "csrrw", "csrrs", "csrrc", 
	"csrrwi", "csrrsi", "csrrci", "sb", 
	"sh", "sw", "sd", "lui", 
	"auipc", "beq", "bne", "blt", 
	"bge", "bltu", "bgeu", "jal", 

	"mul", "mulh", "mulhsu", "mulhu", 
	"div_", "divu", "rem", "remu", 
	"mulw", "divw", "divuw", "remw", 
	"remuw",
	
	"u16", 
	"cnop", "caddi4spn", "clw", "cld", "csw", "csd", 
	"caddi", "cjal", "caddiw", "cli", "caddi16sp", 
	"clui", "csrli", "candi", "csub", "cxor", "cor", 
	"cand", "csubw", "caddw", "cj", "cbeqz", "cbnez", 
	"cslli", "clwsp", "cldsp", "cjr", "cmv", "cebreak", 
	"cjalr", "cadd", "cswsp", "csdsp", 

	"u8",

	"ctclear", "ctdel", "ctls", "ctarg", "ctli", "ctstop",
	"ctat", "ctpc", "ctb", "ctf", "ctblt",
	"ctbge", "ctbeq", "ctbne", "ctincr", "ctzero",
	"ctadd", "ctsub", "ctmul", "ctdiv", "ctrem", "ctnor", "ctxor", "ctand", "ctor", 
	"ctsl", "ctsr", "ctlb", "ctlh", "ctlw",
	"ctld", "ctsb", "ctsh", "ctsw", "ctsd",
	"ctprint", "ctabort", "ctget", "ctput",
};

struct instruction { u32 op; u32 arguments[3]; }; //  make arguments not an array. use a0, a1 and a2 directly, as variables. simpler. 

static nat byte_count = 0;
static u8* bytes = NULL;
static nat ins_count = 0;
static struct instruction* ins = NULL;
static nat stop = 0;
static nat arg_count = 0;
static u32 arguments[4096] = {0};
static nat registers[65536] = {0};

static nat defining = 0;
static nat defining_length = 0;
static nat dict_count = 0; 
static char* names[4096] = {0};
static u32 values[4096] = {0};

static bool ct_flag = true;       // todo: make the ct system more risc-v like.  delete this by making the branch do the check.

static bool is(const char* word, nat count, const char* this) {
	return strlen(this) == count and not strncmp(word, this, count);
}

static char* read_file(const char* name, nat* out_length) {
	int d = open(name, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }

	const int file = open(name, O_RDONLY, 0);
	if (file < 0) { read_error: perror("open"); printf("filename: \"%s\"\n", name); exit(1); }

	size_t length = (size_t) lseek(file, 0, SEEK_END);
	char* string = malloc(length);
	lseek(file, 0, SEEK_SET);
	read(file, string, length);
	close(file); 

	*out_length = length;
	return string;
}

static void print_error(const char* reason, const nat start_index, const nat error_word_length) {
	const nat end_index = start_index + error_word_length;
	fprintf(stderr, "\033[1m%s:%llu:%llu:%llu:%llu: "
			"\033[1;31merror:\033[m \033[1m%s\033[m\n", 
		"file", //filename, 
		start_index, end_index, 
		0LLU,0LLU,//line, column, 
		reason
	);
}

static void push(u32 op) {
	if (stop) return;
	struct instruction new = { .op = op, .arguments = {0} };
	for (nat i = 0; i < 3; i++) {
		if (arg_count == i) break;
		new.arguments[i] = arguments[arg_count - 1 - i];
	}
	if (op >= beq and op <= jal) new.arguments[2] = (u32) registers[new.arguments[2]];
	ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
	ins[ins_count++] = new;
}

static void execute(nat op, nat* pc, char* text) {
	const nat a2 = arg_count >= 3 ? arguments[arg_count - 3] : 0;
	const nat a1 = arg_count >= 2 ? arguments[arg_count - 2] : 0;
	const nat a0 = arg_count >= 1 ? arguments[arg_count - 1] : 0;

	if (op == ctstop) { if (registers[a0] == stop) stop = 0; return; }

	if (stop) return;

	if (debug) printf("@%llu: info: executing \033[1;32m%s\033[0m(%llu) "
			" %lld %lld %lld\n", *pc, instruction_spelling[op], op, a0, a1, a2);

	if (op == ctclear) arg_count = 0;
	else if (op == ctdel)  { if (arg_count) arg_count--; }
	else if (op == ctarg)  arguments[arg_count++] = (u32) registers[a0]; 
	else if (op == ctls)   registers[a0] = (nat) text[registers[a1]];
	else if (op == ctli)   registers[a0] = a1;
	else if (op == ctat)   registers[a0] = ins_count;
	else if (op == ctpc)   registers[a0] = *pc;
	else if (op == ctb)    { if (ct_flag)  *pc = registers[a0]; }
	else if (op == ctf)    { if (ct_flag) stop = registers[a0]; }
	else if (op == ctblt)  ct_flag = registers[a0]  < registers[a1];
	else if (op == ctbge)  ct_flag = registers[a0] >= registers[a1];
	else if (op == ctbeq)  ct_flag = registers[a0] == registers[a1];
	else if (op == ctbne)  ct_flag = registers[a0] != registers[a1];

	else if (op == ctincr) registers[a0]++;
	else if (op == ctzero) registers[a0] = 0;
	
	else if (op == ctadd)  registers[a0] = registers[a1] + registers[a2]; 
	else if (op == ctsub)  registers[a0] = registers[a1] - registers[a2]; 
	else if (op == ctmul)  registers[a0] = registers[a1] * registers[a2]; 
	else if (op == ctdiv)  registers[a0] = registers[a1] / registers[a2]; 
	else if (op == ctrem)  registers[a0] = registers[a1] % registers[a2]; 
	else if (op == ctxor)  registers[a0] = registers[a1] ^ registers[a2]; 
	else if (op == ctand)  registers[a0] = registers[a1] & registers[a2]; 
	else if (op == ctor)   registers[a0] = registers[a1] | registers[a2]; 
	else if (op == ctnor)  registers[a0] = ~(registers[a1] | registers[a2]); 
	else if (op == ctsl)   registers[a0] = registers[a1] << registers[a2]; 
	else if (op == ctsr)   registers[a0] = registers[a1] >> registers[a2]; 

	else if (op == ctlb)  registers[a0] = *(u8*) registers[a1]; 
	else if (op == ctlh)  registers[a0] = *(u16*)registers[a1]; 
	else if (op == ctlw)  registers[a0] = *(u32*)registers[a1]; 
	else if (op == ctld)  registers[a0] = *(nat*)registers[a1]; 

	else if (op == ctsb)  *(u8*) registers[a0] = (u8)  registers[a1]; 
	else if (op == ctsh)  *(u16*)registers[a0] = (u16) registers[a1]; 
	else if (op == ctsw)  *(u32*)registers[a0] = (u32) registers[a1]; 
	else if (op == ctsd)  *(nat*)registers[a0] = (nat) registers[a1]; 
	
	else if (op == ctprint) printf("debug: \033[32m%llu (%lld)\033[0m \033[32m0x%llx\033[0m\n", registers[a0], registers[a0], registers[a0]); 
	else if (op == ctabort) abort();
	else if (op == ctget) registers[a0] = (nat) getchar();
	else if (op == ctput) putchar((char) registers[a0]);
}

static void print_registers(void) {
	nat printed_count = 0;
	printf("debug: registers = {\n");
	for (nat i = 0; i < sizeof registers / sizeof(nat); i++) {
		if (not (printed_count % 4)) puts("");
		if (registers[i]) {
			printf("%02llu:%010llx, ", i, registers[i]);
			printed_count++;
		}
	}
	puts("}");
}

static void print_arguments(void) {
	printf("arguments: { ");
	for (nat i = 0; i < arg_count; i++) {
		printf("{%llu} ", (nat) arguments[i]);
	}
	puts("} ");
}

static void print_instructions(void) {
	printf("instructions: {\n");
	for (nat i = 0; i < ins_count; i++) {
		const struct instruction I = ins[i];
		printf("\t%llu\tins(.op=%u (\"%s\"), args:{", i, I.op, instruction_spelling[I.op]);
		for (nat a = 0; a < 3; a++) printf("%llu ", (nat) I.arguments[a]);
		puts("}");
	}
	puts("}");
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


static void emit_byte(u32 x) {
	bytes = realloc(bytes, byte_count + 1);
	bytes[byte_count++] = (u8) x;
}

static void emith(u32 x) {
	bytes = realloc(bytes, byte_count + 2);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
}

static void emit(u32 x) {
	bytes = realloc(bytes, byte_count + 4);
	bytes[byte_count++] = (u8) (x >> 0);
	bytes[byte_count++] = (u8) (x >> 8);
	bytes[byte_count++] = (u8) (x >> 16);
	bytes[byte_count++] = (u8) (x >> 24);
}

static void zero_register_error(const char* instruction) {
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "zero register used with %s instruction is invalid", instruction);
	print_error(reason, 1, 1); 
	exit(1);
}

static void check(nat r, nat c, const char* type) {
	if (r < c) return;
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "invalid %s argument %llu (%llu >= %llu)", type, r, r, c);
	print_error(reason, 1, 1); 
	exit(1);
}

////////////////// riscv backend //////////////


static u32 r_type(u32* a, u32 o, u32 f, u32 g) {   //  r r r op
	check(a[0], 32, "register");
	check(a[1], 32, "register");
	check(a[2], 32, "register");
	return (g << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o;
}

static u32 i_type(u32* a, u32 o, u32 f) {   //  i r r op
	check(a[0], 32, "register");
	check(a[1], 32, "register");
	check(a[2], 1 << 12, "immediate");
	return (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o;
}

static u32 s_type(u32* a, u32 o, u32 f) {   //  i r r op
	check(a[0], 32, "register");
	check(a[1], 32, "register");
	check(a[2], 1 << 12, "immediate");
	return ((a[0] >> 5U) << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | ((a[0] & 0x1F) << 7U) | o;
}

static u32 u_type(u32* a, u32 o) {   	//  i r op
	check(a[0], 32, "register");
	check(a[1], 1 << 20, "immediate");
	return (a[1] << 12U) | (a[0] << 7U) | o;
}

static u32 calculate_offset(nat here, nat label) {
	u32 offset = 0;
	if (label < here) {
		for (nat i = label; i < here; i++) {
			     if (ins[i].op <= jal) offset -= 4; 
			else if (ins[i].op == dh) offset -= 2;
			else if (ins[i].op == db) offset -= 1;
			else abort();
		}
	} else {
		for (nat i = here; i < label; i++) {
			     if (ins[i].op <= jal) offset += 4; 
			else if (ins[i].op == dh) offset += 2;
			else if (ins[i].op == db) offset += 1;
			else abort();
		}
	}
	return offset;
}

static u32 j_type(nat here, u32* a, u32 o) {   	//  L r op
	check(a[0], 32, "register");
	const u32 e = calculate_offset(here, a[1]);
	const u32 imm19_12 = (e & 0x000FF000);
	const u32 imm11    = (e & 0x00000800) << 9;
	const u32 imm10_1  = (e & 0x000007FE) << 20;
	const u32 imm20    = (e & 0x00100000) << 11;
	const u32 imm = imm20 | imm10_1 | imm11 | imm19_12;
	return (imm << 12U) | (a[0] << 7U) | o;
}

static u32 b_type(__attribute__((unused)) nat here, __attribute__((unused)) u32* a, __attribute__((unused)) u32 o, __attribute__((unused)) u32 f) {   //  L r r op
	return 0;
//	abort();
//	check(a[0], 32, "register");
//	check(a[1], 32, "register");
	//const u32 e = calculate_offset(here, a[2]);

	//const u32 imm19_12 = (e & 0x000FF000);
	//const u32 imm11    = (e & 0x00000800) << 9;
	//const u32 imm10_1  = (e & 0x000007FE) << 20;
	//const u32 imm20    = (e & 0x00100000) << 11;

	//const u32 imm = imm20 | imm10_1 | imm11 | imm19_12;

	//return (imm << 12U) | (a[0] << 7U) | o;
	//return (a[1] << 12U) | (a[0] << 7U) | o;

//	return 0;
}

/*
static u16 cr_type(u32* a, u16 o, u32 f, u32 g) {
	check(a[0], 32, "register");
	check(a[1], 32, "register");
	check(a[2], 32, "register");
	return (u16) ((g << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o);
}
static u32 ci_type(u32* a, u32 o, u32 f, u32 g) { return 0; }
static u32 css_type(u32* a, u32 o, u32 f, u32 g) { return 0; }
static u32 ciw_type(u32* a, u32 o, u32 f, u32 g) { return 0; }
static u32 cl_type(u32* a, u32 o, u32 f, u32 g) { return 0; } 
static u32 cs_type(u32* a, u32 o, u32 f, u32 g) { return 0; } 
static u32 cb_type(u32* a, u32 o, u32 f, u32 g) { return 0; } 
static u32 cj_type(u32* a, u32 o, u32 f, u32 g) { return 0; } 
*/

static void generate_riscv_machine_code(void) {
	for (nat i = 0; i < ins_count; i++) {
		nat op = ins[i].op;
		u32* a = ins[i].arguments;

		     if (op == db)	emit_byte(a[0]);
		else if (op == dh)	emith(a[0]);
		else if (op == dw)	emit(a[0]);
		else if (op == ecall)   emit(0x00000073);
		else if (op == ebreak)  emit(0x00100073);
		else if (op == fence)   emit(0x0000000F); // todo: not implemented fully. 
		else if (op == fencei)  emit(0x0000200F);

		else if (op == add)     emit(r_type(a, 0x33, 0x0, 0x00));
		else if (op == sub)     emit(r_type(a, 0x33, 0x0, 0x20));
		else if (op == sll)     emit(r_type(a, 0x33, 0x1, 0x00));
		else if (op == slt)     emit(r_type(a, 0x33, 0x2, 0x00));
		else if (op == sltu)    emit(r_type(a, 0x33, 0x3, 0x00));
		else if (op == xor_)    emit(r_type(a, 0x33, 0x4, 0x00));
		else if (op == srl)     emit(r_type(a, 0x33, 0x5, 0x00));
		else if (op == sra)     emit(r_type(a, 0x33, 0x5, 0x20));
		else if (op == or_)     emit(r_type(a, 0x33, 0x6, 0x00));
		else if (op == and_)    emit(r_type(a, 0x33, 0x7, 0x00));
		else if (op == addw)    emit(r_type(a, 0x3B, 0x0, 0x00));
		else if (op == subw)    emit(r_type(a, 0x3B, 0x0, 0x20));
		else if (op == sllw)    emit(r_type(a, 0x3B, 0x1, 0x00));
		else if (op == srlw)    emit(r_type(a, 0x3B, 0x5, 0x00));
		else if (op == sraw)    emit(r_type(a, 0x3B, 0x5, 0x20));

		else if (op == lb)      emit(i_type(a, 0x03, 0x0));
		else if (op == lh)      emit(i_type(a, 0x03, 0x1));
		else if (op == lw)      emit(i_type(a, 0x03, 0x2));
		else if (op == ld)      emit(i_type(a, 0x03, 0x3));
		else if (op == lbu)     emit(i_type(a, 0x03, 0x4));
		else if (op == lhu)     emit(i_type(a, 0x03, 0x5));
		else if (op == lwu)     emit(i_type(a, 0x03, 0x6));
		else if (op == addi)    emit(i_type(a, 0x13, 0x0));
		else if (op == slti)    emit(i_type(a, 0x13, 0x2));
		else if (op == sltiu)   emit(i_type(a, 0x13, 0x3));
		else if (op == xori)    emit(i_type(a, 0x13, 0x4));
		else if (op == ori)     emit(i_type(a, 0x13, 0x6));
		else if (op == andi)    emit(i_type(a, 0x13, 0x7));
		else if (op == slli)    emit(i_type(a, 0x13, 0x1));
		else if (op == srli)    emit(i_type(a, 0x13, 0x5));   // TODO: make this not use the immediate as a bit in the opcode. toggle this bit if the user gives a sraiw/srai.  comment version old:(for srai/sraiw, give the appropriate a[2] with imm[10] set.)
		else if (op == addiw)   emit(i_type(a, 0x1B, 0x0));
		else if (op == slliw)   emit(i_type(a, 0x1B, 0x1));
		else if (op == srliw)   emit(i_type(a, 0x1B, 0x5));
		else if (op == jalr)    emit(i_type(a, 0x67, 0x0));
		else if (op == csrrw)   emit(i_type(a, 0x73, 0x1));
		else if (op == csrrs)   emit(i_type(a, 0x73, 0x2));
		else if (op == csrrc)   emit(i_type(a, 0x73, 0x3));
		else if (op == csrrwi)  emit(i_type(a, 0x73, 0x5));
		else if (op == csrrsi)  emit(i_type(a, 0x73, 0x6));
		else if (op == csrrci)  emit(i_type(a, 0x73, 0x7));

		else if (op == sb)      emit(s_type(a, 0x23, 0x0));
		else if (op == sh)      emit(s_type(a, 0x23, 0x1));
		else if (op == sw)      emit(s_type(a, 0x23, 0x2));
		else if (op == sd)      emit(s_type(a, 0x23, 0x3));

		else if (op == lui)     emit(u_type(a, 0x37));
		else if (op == auipc)   emit(u_type(a, 0x17));
		
		else if (op == beq)     emit(b_type(i, a, 0x63, 0x0));
		else if (op == bne)     emit(b_type(i, a, 0x63, 0x1));
		else if (op == blt)     emit(b_type(i, a, 0x63, 0x4));
		else if (op == bge)     emit(b_type(i, a, 0x63, 0x5));
		else if (op == bltu)    emit(b_type(i, a, 0x63, 0x6));
		else if (op == bgeu)    emit(b_type(i, a, 0x63, 0x7));

		else if (op == jal)     emit(j_type(i, a, 0x6F));

		//else if (op == cnop emith(0);  // TODO: finish C extension implementation. 
		//else if (op == caddi4spn) emith(ci_type(a, ));   
		else {
			printf("error: unknown instruction: %llu\n", op);
			printf("       unknown instruction: %s\n", instruction_spelling[op]);
			
		}
	}
}

/////////////////////////////////////////////////






static u32 generate_br(u32* a, u32 op, u32 oc) {  //          blr: oc = 1
	u32 Rn = (u32) a[0];
	check(Rn, 32, "register");
	return (oc << 21U) | (op << 10U) | (Rn << 5U);
}

static u32 generate_b(u32* a, nat im, u32 oc) {   //           bl: oc = 1
	//u32 Im = * (u32*) a[0];
	//check_branch((int) Im, 1 << (26 - 1), a[0], "branch offset");
	//return (oc << 31U) | (0x05 << 26U) | (0x03FFFFFFU & Im);
	return 0;
}

static u32 generate_bc(u32* a, nat im) { 
	u32 cd = (u32) a[0];
	u32 Im = * (u32*) im;
	//check_branch((int) Im, 1 << (19 - 1), a[1], "branch offset");
	check(cd, 16, "condition");
	return (0x54U << 24U) | ((0x0007FFFFU & Im) << 5U) | cd;
}

static u32 generate_adr(u32* a, u32 op, nat im, u32 oc) {   //      adrp: oc = 1 

	u32 Rd = (u32) a[0];
	u32 Im = * (u32*) im;

	check(Rd, 32, "register");
	//check_branch((int) Im, 1 << (21 - 1), a[1], "pc-relative address");
	u32 lo = 0x03U & Im, hi = 0x07FFFFU & (Im >> 2);

	return  (oc << 31U) | 
		(lo << 29U) | 
		(op << 24U) | 
		(hi <<  5U) | Rd;
}

static u32 generate_mov(u32 Rd, u32 op, u32 im, u32 sf, u32 oc) {  //     im Rd mov     movz: oc = 2   movk: oc = 3   movn: oc = 0    // lui
	check(Rd, 32, "register");
	check(im, 1 << 12U, "immediate");
	return  (sf << 31U) | 
		(oc << 29U) | 
		(op << 23U) | 
		(sh << 21U) | 
		(im <<  5U) | Rd;
}

static u32 generate_addi(u32 Rd, u32 Rn, u32 op, u32 im, u32 sf, u32 sb, u32 st, u32 sh) {  // im Rn Rd addi    // addi
	if (not Rd) zero_register_error("addi");
	if (not Rn) return generate_mov(Rd, 0x25U, im, 0, 0);
	check(Rd, 32, "register");
	check(Rn, 32, "register");
	check(im, 1 << 12U, "immediate");
	return  (sf << 31U) | 
		(sb << 30U) | 
		(st << 29U) | 
		(op << 23U) | 
		(sh << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_memi(u32* a, u32 op, u32 im, u32 sf, u32 oc) {     // im Rn Rt oe memi            // sd,sw,sh,sb,   ld,lw,lh,lb
	u32 oe = (u32) a[0];
	u32 Rt = (u32) a[1];
	u32 Rn = (u32) a[2];
	check(oe,  4, "mode");
	check(Rt, 32, "register");
	check(Rn, 32, "register");
	check(im, 1 << 9U, "immediate");
	return  (sf << 30U) | 
		(op << 24U) | 
		(oc << 22U) |
		(im << 12U) | 
		(oe << 10U) |
		(Rn <<  5U) | Rt;
}

static u32 generate_memiu(u32* a, u32 op, u32 im, u32 sf, u32 oc) {    // im Rn Rt memiu
	u32 Rt = (u32) a[0];
	u32 Rn = (u32) a[1];
	check(Rt, 32, "register");
	check(Rn, 32, "register");
	check(im, 1 << 12U, "immediate");
	return  (sf << 30U) | 
		(op << 24U) | 
		(oc << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rt;
}

static u32 generate_add(u32 Rd, u32 Rn, u32 Rm, u32 op, u32 im, u32 sf, u32 st, u32 sb, u32 sh) {   //  im Rm Rn Rd or/add
	if (Rd == 0) Rd = 31;
	if (Rn == 0) Rn = 31;
	if (Rm == 0) Rm = 31;
	check(Rd, 32, "register");
	check(Rn, 32, "register");
	check(Rm, 32, "register");
	check(im, 32U << sf, "immediate");
	return  (sf << 31U) |
		(sb << 30U) |
		(st << 29U) |
		(op << 24U) | 
		(sh << 21U) |
		(Rm << 16U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd;
}



/*	
	// original:
	for (nat i = 0; i < ins_count; i++) {

		nat op = ins[i].op;
		u32 im = (u32) ins[i].op;
		u32 a[3] = {0}; 
		memcpy(a, ins[i].arguments, sizeof a);

		     if (op == dw)     emit(im);
		else if (op == ecall)  emit(0xD4000001);

		else if (op == br)     emit(generate_br(a, 0x3587C0U));
		else if (op == b_)     emit(generate_b(a, ins[i].immediate));
		else if (op == bc)     emit(generate_bc(a, ins[i].immediate));
		else if (op == adr)    emit(generate_adr(a, 0x10U, ins[i].immediate));
		else if (op == mov)    emit(generate_mov(a,  0x25U, im));
		else if (op == addi)   emit(generate_addi(a, 0x22U, im));
		else if (op == madd)   emit(generate_madd(a, 0xD8));
		else if (op == umadd)  emit(generate_madd(a, 0xDD));
		else if (op == adc)    emit(generate_adc(a, 0x0D0U, 0x00));
		else if (op == udiv)   emit(generate_adc(a, 0x0D6U, 0x02));
		else if (op == umax)   emit(generate_adc(a, 0x0D6U, 0x19));
		else if (op == umin)   emit(generate_adc(a, 0x0D6U, 0x1B));
		else if (op == lslv)   emit(generate_adc(a, 0x0D6U, 0x08));
		else if (op == lsrv)   emit(generate_adc(a, 0x0D6U, 0x09));
		else if (op == asrv)   emit(generate_adc(a, 0x0D6U, 0x0A));
		else if (op == rorv)   emit(generate_adc(a, 0x0D6U, 0x0B));
		else if (op == or_)    emit(generate_add(a, 0x2AU, im));
		else if (op == add)    emit(generate_add(a, 0x0BU, im));
		else if (op == rbit)   emit(generate_rev(a, 0x16B000U));
		else if (op == revh)   emit(generate_rev(a, 0x16B001U));
		else if (op == rev)    emit(generate_rev(a, 0x16B002U));
		else if (op == clz)    emit(generate_rev(a, 0x16B004U));
		else if (op == cls)    emit(generate_rev(a, 0x16B005U));
		else if (op == csel)   emit(generate_csel(a, 0x0D4U));
		else if (op == memi)   emit(generate_memi(a,  0x38, im));
		else if (op == memiu)  emit(generate_memiu(a, 0x39, im));


		else {
			printf("error: unknown instruction: %llu\n", op);
			printf("       unknown instruction: %s\n", instruction_spelling[op]);
			
		}
	}


*/



static void generate_arm64_machine_code(void) {

	// risv version: use this one as a template for the op==opcode's, but with 
	//  the above arm instruction encoding functions.., 
	// with the apropriote arguments hardcoded to get the right semantics. 

	for (nat i = 0; i < ins_count; i++) {
		nat op = ins[i].op;
		u32* a = ins[i].arguments;

		     if (op == db)	emit_byte(a[0]);
		else if (op == dh)	emith(a[0]);
		else if (op == dw)	emit(a[0]);
		else if (op == ecall)   emit(0xD4000001);

		else if (op == add)    emit(generate_add(a[0], a[1], a[2], 0x0BU, 0, 0, 0, 0, 0));
		else if (op == sub)    {}
		else if (op == sll)    {}
		else if (op == slt)    {}
		else if (op == sltu)   {}
		else if (op == xor_)   {}
		else if (op == srl)    {}
		else if (op == sra)    {}
		else if (op == or_)    emit(generate_add(a[0], a[1], a[2], 0x2AU, 0, 0, 0, 0, 0));
		else if (op == and_)   {}
		else if (op == addw)   {}
		else if (op == subw)   {}
		else if (op == sllw)   {}
		else if (op == srlw)   {}
		else if (op == sraw)   {}
		else if (op == lb)     {}
		else if (op == lh)     {}
		else if (op == lw)     {}
		else if (op == ld)     {}
		else if (op == lbu)    {}
		else if (op == lhu)    {}
		else if (op == lwu)    {}
		else if (op == addi)   emit(generate_addi(a[0], a[1], 0x22U, a[2], 0, 0, 0, 0));
		else if (op == slti)   {}
		else if (op == sltiu)  {}
		else if (op == xori)   {}
		else if (op == ori)    {}
		else if (op == andi)   {}
		else if (op == slli)   {}
		else if (op == srli)   {}
		else if (op == addiw)  {}
		else if (op == slliw)  {}
		else if (op == srliw)  {}
		else if (op == jalr)   {}
		else if (op == sb)     {}
		else if (op == sh)     {}
		else if (op == sw)     {}
		else if (op == sd)     {}
		else if (op == lui)    {}
		else if (op == auipc)  {}
		else if (op == beq)    {}
		else if (op == bne)    {}
		else if (op == blt)    {}
		else if (op == bge)    {}
		else if (op == bltu)   {}
		else if (op == bgeu)   {}
		else if (op == jal)    {}
		else {
			printf("error: arm64: unknown instruction: %llu\n", op);
			printf("       unknown instruction name: %s\n", instruction_spelling[op]);
			abort();
		}
	}
}


static void make_elf_object_file(const char* object_filename) {
	puts("make_elf_object_file: unimplemented");
	abort();
//   system("objdump program.out -DSast --disassembler-options=no-aliases");    // eventually, we'll use objdump and  readelf   to debug the output. 
//   system "readelf program.out");
/*		
	const int flags = O_WRONLY | O_CREAT | O_TRUNC | O_EXCL;
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(argv[3], flags, mode);
	if (file < 0) { perror("obj:open"); exit(1); }
	write(file, NULL, 0);
	write(file, NULL, 0);
	write(file, NULL, 0);
	close(file);
*/
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

int main(int argc, const char** argv) {

	if (argc != 2) 
		exit(puts("asm: \033[31;1merror:\033[0m usage: ./asm <source.s>"));
	
	const char* filename = argv[1];
	nat text_length = 0;
	char* text = read_file(filename, &text_length);

	*registers = (nat)(void*) malloc(65536); 

	const char* object_filename = "object0.o";
	const char* executable_filename = "executable0.out";
	bool preserve_existing_object = true;
	bool preserve_existing_executable = true;
	nat architecture = 0;
	nat output_format = 0;

	nat count = 0, start = 0, index = 0;
	for (; index < text_length; index++) {
		if (not isspace(text[index])) { 
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;
	process:;
		const char* const word = text + start;

		if (debug) printf(". %s: \"\033[32;1m%.*s\033[0m\"...\n", 
				stop ? "\033[31mignoring\033[0m" : "\033[32mprocessing\033[0m", 
				(int) count, word);

		if (is(word, count, "define")) {
			if (not arg_count or stop) goto next;
			const u32 r = arguments[arg_count - 1];
			
			names[dict_count] = strndup(text + defining, defining_length);
			values[dict_count++] = r;

			if (debug) printf("info: defining macro with value: %llu,  the macro is named: \"%s\"\n", 
				(nat) values[dict_count - 1], names[dict_count - 1]
			);

			defining_length = 0; defining = 0;
			goto next;
		}

		if (defining) goto error;
		if (is(word, count, "eof")) goto generate_ins;
		if (is(word, count, "settarget")) { architecture = arguments[arg_count - 1]; goto next; } 
		if (is(word, count, "setoutputformat")) { output_format = arguments[arg_count - 1]; goto next; }
		
		if (is(word, count, "setobjectname")) { object_filename = names[dict_count - 1]; goto next; }
		if (is(word, count, "setexecutablename")) { executable_filename = names[dict_count - 1]; goto next; }
		if (is(word, count, "preserveexistingobject")) { preserve_existing_object = arguments[arg_count - 1]; goto next; }
		if (is(word, count, "preserveexistingexecutable")) { preserve_existing_executable = arguments[arg_count - 1]; goto next; }
		if (is(word, count, "enabledebugoutput")) { debug = arguments[arg_count - 1]; goto next; }


		if (is(word, count, "include")) {
			if (not dict_count or stop) goto next;
			char new[128] = {0};
			snprintf(new, sizeof new, "%s.s", names[dict_count - 1]);
			if (debug) printf("\033[32mIncluding file \"%s\"...\033[0m\n", new);
			nat l = 0;
			const char* s = read_file(new, &l);
			text = realloc(text, text_length + l + 1);
			memmove(text + index + 1 + l + 1, text + index + 1, text_length - (index + 1));
			memcpy(text + index + 1, s, l);
			text[index + 1 + l] = ' ';
			text_length += l + 1;
			goto next;
		}

		
		if (is(word, count, "debugregisters")) { print_registers(); goto next; }
		if (is(word, count, "debugarguments")) { print_arguments(); goto next; }
		if (is(word, count, "debuginstructions")) { print_instructions(); goto next; }
		
		for (u32 i = dw; i < instruction_set_count; i++) {
			// printf("trying \"%s\"...\n", instruction_spelling[i]);
			if (not is(word, count, instruction_spelling[i])) continue;
			if (i < ctclear) push(i); else execute(i, &index, text);
			goto next;
		}

		u32 r = 0, s = 1;
		for (nat i = 0; i < count; i++) {
			if (word[i] == '0') s <<= 1;
			else if (word[i] == '1') { r += s; s <<= 1; }
			else goto other_word;
		}
		if (debug) printf("[info: pushed %llu onto argument stack]\n", (nat) r);
		arguments[arg_count++] = r;
		goto next;
	other_word:
		for (nat i = 0; i < dict_count; i++) {
			if (not is(word, count, names[i])) continue;
			if (debug) printf("[info: FOUND USER-DEFINED NAME: pushed (%s)  value %llu  onto argument stack]\n", 
					names[i], (nat) values[i]);
			arguments[arg_count++] = values[i]; 
			goto next;
		}
		if (stop) goto next;
		if (debug) printf("found undefined word %.*s... setting defined = %llu.\n", (int) count, word, start);
		defining = start; defining_length = count; 
		goto next;

	error:;
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "unknown word \"%.*s\"", (int) defining_length, text + defining);
		print_error(reason, start, count);
		exit(1);
		next: count = 0;
	}
	if (count) goto process;

generate_ins:

	if (debug) {
		printf("info: building for target:\n\tarchitecture:  \033[31;1m%s\033[0m\n\toutput_format: \033[32;1m%s\033[0m.\n\n", 
			target_spelling[architecture  % target_count], output_format_spelling[output_format % output_format_count]
		);
	}

	if (architecture == noruntime) {
		if (not ins_count) exit(0);
		print_error("encountered runtime instruction with target \"noruntime\"", 0, 0);
		exit(1);

	} else if (architecture == riscv32 or architecture == riscv64) {
		generate_riscv_machine_code();

	} else if (architecture == arm64) {
		generate_arm64_machine_code();

	} else {
		puts("asm: \033[31;1merror:\033[0m unknown target architecture specified, valid values: ");
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
		puts("asm: \033[31;1merror:\033[0m unknown output format specified, valid values: ");
		for (nat i = 0; i < output_format_count; i++) {
			printf("\t%llu : %s\n", i, output_format_spelling[i]);
		}
	}

	if (output_format == elf_executable or output_format == macho_executable) {

		if (preserve_existing_executable and not access(executable_filename, F_OK)) {
			puts("asm: executable_file: file exists"); 
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
		system("otool -txvVhlL object.o");
		system("otool -txvVhlL program.out");
		system("objdump object.o -DSast --disassembler-options=no-aliases");
		system("objdump program.out -DSast --disassembler-options=no-aliases");	
	}
}














































