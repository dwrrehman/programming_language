/*
		risc-v 64-bit cross assembler 
	     written by dwrr on 202403111.010146

	this is made to be my primary cross-platform programming language, 
	to ideally my use of replace C for programming most of my projects.
	only the risc-v target has performance guarantees. other targets
	use a translation layer to translate the risc-v ISA instructions
	into the target's ISA (eg, arm64, arm32, x86, x86_64).


todo stuff:

	 - implement contexts for names. 

	 - implement including files.




branches:
------------

x	- flush out the branching system for risc-v compiletime system.

	- start trying to figure out branches for arm64. 

	- 
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdnoreturn.h>
#include <mach-o/nlist.h>
#include <mach-o/loader.h>

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

static bool debug = 0;

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

enum language_ISA {
	null_instruction, ctzero, ctincr, ctmode, ctat, 

	ctabort, ctprint, ctdebug, ctget, ctput, ctdel, ctarg, ctsetdebug, 

	db, dh, dw, 
	ecall, ebreak, fence, fencei, 
	add, sub, sll, slt, sltu, xor_, srl, sra, or_, and_, 
	addw, subw, sllw, srlw, sraw,
	lb, lh, lw, ld, lbu, lhu, lwu, 
	addi, slti, sltiu, xori, ori, andi, slli, srli, srai, 
	addiw, slliw, srliw, sraiw,
	jalr, 
	csrrw, csrrs, csrrc, csrrwi, csrrsi, csrrci, 
	sb, sh, sw, sd, 
	lui, auipc, 
	beq, bne, blt, bge, bltu, bgeu, jal, 
	mul, mulh, mulhsu, mulhu,
	div_, divu, rem, remu, 
	mulw, divw, divuw, remw, remuw, 

	instruction_set_count
};

struct location { nat start; nat count; };

struct instruction { 
	u32 a[4]; 
	struct location loc[4];
}; 

struct file {
	struct location location;
	const char* name;
};

static nat byte_count = 0;
static u8* bytes = NULL;
static nat ins_count = 0;
static struct instruction* ins = NULL;
static struct instruction current_ins = {0};
static nat registers[65536] = {0};
static nat arg_count = 0;
static u32 arguments[4096] = {0};
static struct location arg_locations[4096] = {0};

static nat file_count = 0;
static struct file files[4096] = {0};

static char* text = NULL;
static nat text_length = 0;

static nat names[4096] = {0};
static nat lengths[4096] = {0};
static nat values[4096] = {0};
static nat name_count = 0;

static bool is_compiletime = false;

static bool is(const char* literal, nat initial) {
	nat i = initial, j = 0;
	for (; text[i] != '"' and literal[j]; i++) {
		if ((unsigned char) text[i] < 33) continue;
		if (text[i] != literal[j]) return false;
		j++;
	}
	return text[i] == '"' and not literal[j];
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

// use this instead:
	//int file = open(argv[1], O_RDONLY);
	//if (file < 0) { perror("open"); exit(1); } 
	//struct stat st;
	//fstat (file, &st);
	//text_length = (nat) st.st_size;
	//text = mmap(0, text_length, PROT_READ, MAP_PRIVATE, file, 0);
	//close(file);

static void push_arg(nat r) {
	if (debug) printf("info: pushed %llu onto argument stack\n", (nat) r);
	arg_locations[arg_count] = (struct location) {0};
	arguments[arg_count++] = (u32) r;
}

static char* get_name(nat name) {
	const nat start = names[name];
	nat end = start;
	while (text[end] != '"') end++;
	char* string = calloc(end - start + 1, 1);
	memcpy(string, text + start, end - start);
	return string;
}



static void print_files(void) {
	printf("here are the current files used in the program: (%lld files) { \n", file_count);
	for (nat i = 0; i < file_count; i++) {
		printf("\t file #%-8lld :   name = \"%-30s\", .start = %-8lld, .size = %-8lld\n",
			i, files[i].name, files[i].location.start, files[i].location.count);
	}
	puts("}");
}


static void print_stack(nat* stack_i, nat* stack_f, nat* stack_o, nat stack_count) {
	printf("current stack: (%lld entries) { \n", stack_count);
	for (nat i = 0; i < stack_count; i++) {
		printf("\t entry #%-8lld :   name = \"%-30s\", i = %-8lld, f = %-8lld, o = %-8lld / %lld\n", 
			i, files[stack_f[i]].name, stack_i[i], stack_f[i], stack_o[i], files[stack_f[i]].location.count);
	}
	puts("}");
}

static void print_error(const char* reason, struct location spot) {

	//printf("the error was located at: {.start=%lld, .count=%lld}, (in absolute file offset space.)\n", spot.start, spot.count);
	//print_files();

	nat location = 0;
	const char* filename = NULL;

	//int colors[] = {31, 32, 33, 34, 35};
	nat stack_i[4096] = {0}, stack_f[4096] = {0}, stack_o[4096] = {0};
	nat stack_count = 0;

	for (nat index = 0; index < text_length; index++) {

		for (nat f = 0; f < file_count; f++) {

			const nat start = files[f].location.start;
			const nat count = files[f].location.count;

			if (index == start) {
				//printf("file %s begins at %llu!\n", files[f].name, index);
				stack_i[stack_count] = index;
				stack_f[stack_count] = f;
				stack_o[stack_count++] = 0;
				break;
			} 

			if (stack_o[stack_count - 1] == files[stack_f[stack_count - 1]].location.count)  {
				//print_stack(stack_i, stack_f, stack_o, stack_count);
				//printf("file %s reached the end the file! (stack_o[%llu] == count == %llu)\n", files[f].name, stack_count - 1, count);
				stack_count--;
				if (not stack_count) goto done; else break;
			}
		}

		if (index == spot.start) {
	//		printf("\033[38;5;255m(ERROR_HERE:%s:%llu)\033[0m", files[stack_f[stack_count - 1]].name, stack_o[stack_count - 1]);
			filename = files[stack_f[stack_count - 1]].name;
			location = stack_o[stack_count - 1];
		}

	//	printf("\033[%dm", colors[stack_count - 1]);
	//	putchar(text[index]);
	//	printf("\033[0m");

		stack_o[stack_count - 1]++;
		//printf("[%s]: incremented stack_o[top=%llu] to be now %llu...\n", files[stack_f[stack_count - 1]].name, stack_count - 1, stack_o[stack_count - 1]);
	}
done:;
	fprintf(stderr, "\033[1masm: %s:%llu:%llu:", filename, location, spot.count);
	fprintf(stderr, " \033[1;31merror:\033[m \033[1m%s\033[m\n", reason);
}











/*



if (file == (nat) -1) {
		puts("error: could not find any file with the right .start, .counts...\n");
		abort();
	} else {
		printf("info: spot was belevied to be located in file \"%s\"...\n", files[file].name);
	}







for (nat i = 0; i < file_count; i++) {

		offset[head] = 0;

		const nat start = files[i].location.start;
		const nat count = files[i].location.count;
		const nat end = start + count;
	
		if (spot.start >= start and spot.start < end) {
			printf("ERROR IS LOCATED IN \"%s\"!...\n", files[i].name);
			file = i;

		} else {
			
		}
	}







			//index += count;
			//printf("[%s]: added %llu to index... (index now %llu)\n", files[i].name, count, index);


		//spot.start -= files[file].location.start;
		//printf("[%s]: subtracted %llu from spot.start... (spot now %llu)\n", files[file].name, files[file].location.start, spot.start);

		//spot.start -= index;
		//printf("[%s]: subtracted index%llu from spot.start... (spot now %llu)\n", files[file].name, index, spot.start);



*/




































static void print_registers(void) {
	nat printed_count = 0;
	printf("debug: registers = {\n");
	for (nat i = 0; i < sizeof registers / sizeof(nat); i++) {
		if (registers[i]) {
			if (printed_count % 4 == 0) puts("");
			printf("%02llu:%010llx, ", i, registers[i]);
			printed_count++;
		}
	}
	puts("}");
}

static void print_arguments(void) {
	printf("\narguments[]: { \n");
	for (nat i = 0; i < arg_count; i++) {
		printf("\targuments[%llu] = { %llu  :  (.start=%llu,.count=%llu)} \n", 
			i, (nat) arguments[i], 
			arg_locations[i].start, arg_locations[i].count
		);
	}
	puts("} \n");
}

static void print_instructions(void) {
	printf("instructions: {\n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t%llu\tins(.op=%u (\"ins__\"), args:{ ", 
			i, ins[i].a[0]
		);
		for (nat a = 0; a < 3; a++) printf("%llu ", (nat) ins[i].a[a + 1]);
		printf("} offsets:{ ");
		printf("(%llu:%llu)* ", 
			ins[i].loc[0].start, 
			ins[i].loc[0].count
		);
		for (nat o = 0; o < 3; o++) {
			printf("(%llu:%llu) ", 
				ins[i].loc[o + 1].start, 
				ins[i].loc[o + 1].count
			);
		}
		puts("}");
	}
	puts("}");
}

static void print_dictionary(void) {
	puts("dictionary = {");
	for (nat i = 0; i < name_count; i++) {
		printf("\t#%llu: (@%llu):(len=%llu):(val=%llu) ", i, names[i], lengths[i], values[i]);
		for (nat c = names[i]; text[c] != '"'; c++) putchar(text[c]);
		puts("");
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

static noreturn void zero_register_error(nat arg_index) {
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "use of zero register for argument %llu in %u instruction is not supported", 
			arg_index, current_ins.a[0]
	);
	print_error(reason, current_ins.loc[arg_index + 1]); 
	exit(1);
}

static void check(nat r, nat c, const char* type, nat arg_index) {
	if (r < c) return;
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "argument %llu: invalid %s, %llu (%llu >= %llu)", arg_index, type, r, r, c);
	print_error(reason, current_ins.loc[arg_index + 1]); 
	exit(1);
}

////////////////// riscv backend //////////////

static u32 r_type(u32* a, u32 o, u32 f, u32 g) {   //  r r r op
	check(a[0], 32, "register", 0);
	check(a[1], 32, "register", 1);
	check(a[2], 32, "register", 2);
	return (g << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o;
}

static u32 i_type(u32* a, u32 o, u32 f) {   //  i r r op
	check(a[0], 32, "register", 0);
	check(a[1], 32, "register", 1);
	check(a[2], 1 << 12, "immediate", 2);
	return (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | (a[0] << 7U) | o;
}

static u32 s_type(u32* a, u32 o, u32 f) {   //  i r r op
	check(a[0], 32, "register", 0);
	check(a[1], 32, "register", 1);
	check(a[2], 1 << 12, "immediate", 2);
	return ((a[0] >> 5U) << 25U) | (a[2] << 20U) | (a[1] << 15U) | (f << 12U) | ((a[0] & 0x1F) << 7U) | o;
}

static u32 u_type(u32* a, u32 o) {   	//  i r op
	check(a[0], 32, "register", 0);
	check(a[1], 1 << 20, "immediate", 1);
	return (a[1] << 12U) | (a[0] << 7U) | o;
}

static u32 calculate_offset(nat here, nat label) {
	u32 offset = 0;
	if (label < here) {
		for (nat i = label; i < here; i++) {
			     if (ins[i].a[0] <= jal) offset -= 4; 
			else if (ins[i].a[0] == dh) offset -= 2;
			else if (ins[i].a[0] == db) offset -= 1;
			else abort();
		}
	} else {
		for (nat i = here; i < label; i++) {
			     if (ins[i].a[0] <= jal) offset += 4; 
			else if (ins[i].a[0] == dh) offset += 2;
			else if (ins[i].a[0] == db) offset += 1;
			else abort();
		}
	}
	return offset;
}

static u32 j_type(nat here, u32* a, u32 o) {   	//  L r op
	check(a[0], 32, "register", 0);
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
		current_ins = ins[i];
		nat op = ins[i].a[0];
		u32* a = ins[i].a + 1;

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
			// printf("       unknown instruction: %s\n", instruction_spelling[op]);
			
		}
	}
}

/////////////////////////////////////////////////

static u32 generate_br(u32 Rn, u32 im, u32 op, u32 oc) { 
	if (oc >= 2 or im) zero_register_error(1);
	Rn = arm64_macos_abi[Rn];
	check(Rn, 32, "register", 0);
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
	check(cd, 16, "condition", 0);
	return (0x54U << 24U) | ((0x0007FFFFU & Im) << 5U) | cd;
}

static u32 generate_adr(u32* a, u32 op, nat im, u32 oc) {   //      adrp: oc = 1 

	u32 Rd = (u32) a[0];
	u32 Im = * (u32*) im;

	check(Rd, 32, "register", 0);
	//check_branch((int) Im, 1 << (21 - 1), a[1], "pc-relative address");
	u32 lo = 0x03U & Im, hi = 0x07FFFFU & (Im >> 2);

	return  (oc << 31U) | 
		(lo << 29U) | 
		(op << 24U) |
		(hi <<  5U) | Rd;
}

//     im Rd mov     movz: oc = 2   movk: oc = 3   movn: oc = 0    // lui
static u32 generate_mov(u32 Rd, u32 op, u32 im, u32 sf, u32 oc, u32 sh) {  

	check(Rd, 32, "register", 0);

	Rd = arm64_macos_abi[Rd];

	check(im, 1 << 12U, "immediate", 1);
	return  (sf << 31U) | 
		(oc << 29U) | 
		(op << 23U) | 
		(sh << 21U) | 
		(im <<  5U) | Rd;
}

// im Rn Rd addi    // addi
static u32 generate_addi(u32 Rd, u32 Rn, u32 op, u32 im, u32 sf, u32 sb, u32 st, u32 sh) {  
	if (not Rd) return 0xD503201F;
	if (not Rn) return generate_mov(Rd, 0x25U, im, sf, 2, 0);
	
	check(Rd, 32, "register", 0);
	check(Rn, 32, "register", 1);

	Rd = arm64_macos_abi[Rd];
	Rn = arm64_macos_abi[Rn];

	check(im, 1 << 12U, "immediate", 2);
	return  (sf << 31U) | 
		(sb << 30U) | 
		(st << 29U) | 
		(op << 23U) | 
		(sh << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd;
}

// im Rn Rt oe memi            // sd,sw,sh,sb,   ld,lw,lh,lb
static u32 generate_memi(u32* a, u32 op, u32 im, u32 sf, u32 oc) {     
	u32 oe = (u32) a[0];
	u32 Rt = (u32) a[1];
	u32 Rn = (u32) a[2];

	check(oe,  4, "mode", 0);
	check(Rt, 32, "register", 1);
	check(Rn, 32, "register", 2);

	Rt = arm64_macos_abi[Rt];
	Rn = arm64_macos_abi[Rn];

	check(im, 1 << 9U, "immediate", 3);
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
	check(Rt, 32, "register", 0);
	check(Rn, 32, "register", 1);
	check(im, 1 << 12U, "immediate", 2);
	return  (sf << 30U) | 
		(op << 24U) | 
		(oc << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rt;
}

//  im Rm Rn Rd or/add
static u32 generate_add(u32 Rd, u32 Rn, u32 Rm, u32 op, u32 im, u32 sf, u32 st, u32 sb, u32 sh) {

	check(Rd, 32, "register", 0);
	check(Rn, 32, "register", 1);
	check(Rm, 32, "register", 2);
	check(im, 32U << sf, "immediate", 3);

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

static void generate_arm64_machine_code(void) {

	// risv version: use this one as a template for the op==opcode's, but with 
	//  the above arm instruction encoding functions.., 
	// with the apropriote arguments hardcoded to get the right semantics. 

	for (nat i = 0; i < ins_count; i++) {
		current_ins = ins[i];
		nat op = ins[i].a[0];
		u32* a = ins[i].a + 1;

		     if (op == db)	emit_byte(a[0]);
		else if (op == dh)	emith(a[0]);
		else if (op == dw)	emit(a[0]);
		else if (op == ecall)   emit(0xD4000001);

		else if (op == add)    emit(generate_add(a[0], a[1], a[2], 0x0BU, 0, 1, 0, 0, 0));
		else if (op == sub)    emit(generate_add(a[0], a[1], a[2], 0x0BU, 0, 1, 0, 1, 0));
		else if (op == sll)    {abort();}    // next lets do      emit(generate_adc(a, 0x0D6U, 0x08));  // lslv
		else if (op == slt)    {abort();} // omg do we impl the slt/sltu via CSEL!!?!!?
		else if (op == sltu)   {abort();}
		else if (op == xor_)   {abort();}
		else if (op == srl)    {abort();}
		else if (op == sra)    {abort();}
		else if (op == or_)    emit(generate_add(a[0], a[1], a[2], 0x2AU, 0, 1, 0, 0, 0));
		else if (op == and_)   {abort();}
		else if (op == addw)   emit(generate_add(a[0], a[1], a[2], 0x0BU, 0, 0, 0, 0, 0));
		else if (op == subw)   emit(generate_add(a[0], a[1], a[2], 0x0BU, 0, 0, 0, 1, 0));
		else if (op == sllw)   {abort();}
		else if (op == srlw)   {abort();}
		else if (op == sraw)   {abort();}
		else if (op == lb)     {abort();}
		else if (op == lh)     {abort();}
		else if (op == lw)     {abort();}
		else if (op == ld)     {abort();}
		else if (op == lbu)    {abort();}
		else if (op == lhu)    {abort();}
		else if (op == lwu)    {abort();}
		else if (op == addi)   emit(generate_addi(a[0], a[1], 0x22U, a[2], 1, 0, 0, 0));
		else if (op == slti)   {abort();}
		else if (op == sltiu)  {abort();}
		else if (op == xori)   {abort();}
		else if (op == ori)    {abort();}
		else if (op == andi)   {abort();}
		else if (op == slli)   {abort();}
		else if (op == srli)   {abort();}
		else if (op == addiw)  emit(generate_addi(a[0], a[1], 0x22U, a[2], 0, 0, 0, 0));
		else if (op == slliw)  {abort();}
		else if (op == srliw)  {abort();}
		else if (op == jalr)   emit(generate_br(a[0], a[1], 0x3587C0U, a[2]));
		else if (op == sb)     {abort();}
		else if (op == sh)     {abort();}
		else if (op == sw)     {abort();}
		else if (op == sd)     {abort();}
		else if (op == lui)    {abort();}
		else if (op == auipc)  {abort();}
		else if (op == beq)    {puts("found rt beq, no impl though."); abort();}
		else if (op == bne)    {abort();}
		else if (op == blt)    {abort();}
		else if (op == bge)    {abort();}
		else if (op == bltu)   {abort();}
		else if (op == bgeu)   {abort();}
		else if (op == jal)    {abort();}
		else {
			printf("error: arm64: unknown instruction: %llu\n", op);
			//printf("       unknown instruction name: %s\n", instruction_spelling[op]);
			abort();
		}
	}
}




/*	
	// original:
	for (nat i = 0; i < ins_count; i++) {

		nat op = ins[i].op;
		u32 im = (u32) ins[i].op;
		u32 a[3] = {0}; 

		memcpy(a, ins[i].arguments, sizeof a);

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
		else if (op == lslv)   emit(generate_adc(a, 0x0D6U, 0x08));
		else if (op == lsrv)   emit(generate_adc(a, 0x0D6U, 0x09));
		else if (op == asrv)   emit(generate_adc(a, 0x0D6U, 0x0A));
		else if (op == rorv)   emit(generate_adc(a, 0x0D6U, 0x0B));
		else if (op == or_)    emit(generate_add(a, 0x2AU, im));
		else if (op == add)    emit(generate_add(a, 0x0BU, im));
		else if (op == csel)   emit(generate_csel(a, 0x0D4U));
		else if (op == memi)   emit(generate_memi(a,  0x38, im));
		else if (op == memiu)  emit(generate_memiu(a, 0x39, im));


		else {
			printf("error: unknown instruction: %llu\n", op);
			printf("       unknown instruction: %s\n", instruction_spelling[op]);
			
		}
	}
*/


static noreturn void make_elf_object_file(const char* object_filename) {
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

int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("asm: \033[31;1merror:\033[0m usage: ./asm <source.s>"));

	const char* filename = argv[1];
	text_length = 0;
	text = read_file(filename, &text_length);

	const char* object_filename = "object0.o";
	const char* executable_filename = "executable0.out";
	bool preserve_existing_object = true;
	bool preserve_existing_executable = true;
	nat architecture = 0;
	nat output_format = 0;

	registers[2] = (nat)(void*) malloc(65536);
	files[file_count].name = filename;
	files[file_count++].location = (struct location) {.start = 0, .count = text_length};

	struct location here = {0};
	nat start = 0, length = 0, name_starts_at = 0, r = 0, s = 1, called_name = 0, spot = 0;
	nat forwards_branching = 0;

	for (nat index = 0; index < text_length; index++) {
		if ((unsigned char) text[index] < 33) goto next_char;
		if (text[index] == '"') {
			if (not start) { start = index + 1; length = 0; goto next_char; } 
			spot = 0;
			for (; spot < name_count; spot++) if (length >= lengths[spot]) break;
			memmove(lengths + spot + 1, lengths + spot, sizeof(nat) * (name_count - spot));
			memmove(names + spot + 1, names + spot, sizeof(nat) * (name_count - spot));
			memmove(values + spot + 1, values + spot, sizeof(nat) * (name_count - spot));
			lengths[spot] = length;
			names[spot] = start;
			values[spot] = arg_count ? arguments[arg_count - 1] : 0;
			name_count++; 
			start = 0;
			goto next_char;
		}
		if (start) { length++; goto next_char; } 
		nat imax = 0;
		for (nat name = 0; name < name_count; name++) {
			called_name = name;
			name_starts_at = names[name];
			nat c = name_starts_at, i = 0;
			for (; text[c] != '"'; ) {
				if (i > imax) imax = i;
				if (index + i >= text_length) goto next_name;
				if ((unsigned char) text[index + i] < 33) { i++; continue; }
				if ((unsigned char) text[c] < 33) { c++; continue; }
				if (text[index + i] != text[c]) goto next_name;
				i++; c++;
			}
			index += i - 1;
			goto process_name;
			next_name: continue;
		}
		print_error("unresolved symbol", (struct location){index + imax, 0});
		exit(1);

	process_name:;
		*registers = 0;
		nat a2 = arg_count > 2 ? arguments[arg_count - 1 - 2] : 0;
		nat a1 = arg_count > 1 ? arguments[arg_count - 1 - 1] : 0;
		nat a0 = arg_count > 0 ? arguments[arg_count - 1 - 0] : 0;
		if (debug) {
			printf("info: calling: \"\033[32;1m");
			for (nat cc = names[called_name]; text[cc] != '"'; cc++) putchar(text[cc]);
			printf("\033[0m\",args={a0:%llu,a1:%llu,a2:%llu\n", a0, a1, a2);
		}
		nat e = name_starts_at, op = 0;

		if (is("eof", e)) break;

		else if (is("include", e)) {
			files[file_count].name = get_name(spot);
			//printf("info: including file \"%s\"...\n", files[file_count].name);

			nat l = 0;
			const char* str = read_file(files[file_count].name, &l);
			text = realloc(text, text_length + l + 1);
			memmove(text + index + 1 + l + 1, text + index + 1, text_length - (index + 1));
			memcpy(text + index + 1, str, l);
			text[index + 1 + l] = ' ';
			text_length += l + 1;

			//printf(" found %llu characters.\n", l + 1);
			files[file_count].location = (struct location) {.start = index + 1, .count = l + 1};
		 	file_count++;
		}

		else if (is("enabledebug", e)) 		debug = true;
		else if (is("disabledebug", e)) 	debug = false;
		else if (is("setcompiletime", e)) 	is_compiletime = true;
		else if (is("setruntime", e)) 		is_compiletime = false;

		else if (is("setarchitecture", e)) 	architecture = arguments[arg_count - 1];
		else if (is("setoutputformat", e)) 	output_format = arguments[arg_count - 1]; 
		else if (is("preserveobject", e)) 	preserve_existing_object = arguments[arg_count - 1]; 
		else if (is("preserveexecutable", e)) 	preserve_existing_executable = arguments[arg_count - 1]; 
		else if (is("setobjectname", e)) 	object_filename = get_name(spot); 
		else if (is("setexecutablename", e)) 	executable_filename = get_name(spot); 

		else if (is("ctat", e)) {
			registers[a0] = index;
			if (forwards_branching == a0) forwards_branching = 0;
		}

		else if (is("ctzero", e)) { op = ctzero; goto push; }
		else if (is("ctincr", e)) { op = ctincr; goto push; }
		else if (is("ecall", e)) { op = ecall; goto push; }
		else if (is("ebreak", e)) { op = ebreak; goto push; }
		else if (is("fence", e)) { op = fence; goto push; }
		else if (is("fencei", e)) { op = fencei; goto push; }
		else if (is("add", e)) { op = add; goto push; }
		else if (is("sub", e)) { op = sub; goto push; }
		else if (is("and", e)) { op = and_; goto push; }
		else if (is("or",  e)) { op = or_; goto push; }
		else if (is("xor", e)) { op = xor_; goto push; }
		else if (is("slt", e)) { op = slt; goto push; }
		else if (is("sltu",e)) { op = sltu; goto push; }
		else if (is("addi", e)) { op = addi; goto push; }
		else if (is("andi", e)) { op = andi; goto push; }
		else if (is("ori",  e)) { op = ori; goto push; }
		else if (is("xori", e)) { op = xori; goto push; }
		else if (is("slti", e)) { op = slti; goto push; }
		else if (is("sltiu",e)) { op = sltiu; goto push; }
		else if (is("lb", e)) { op = lb; goto push; }
		else if (is("lh", e)) { op = lh; goto push; }
		else if (is("lw", e)) { op = lw; goto push; }
		else if (is("ld", e)) { op = ld; goto push; }
		else if (is("sb", e)) { op = sb; goto push; }
		else if (is("sh", e)) { op = sh; goto push; }
		else if (is("sw", e)) { op = sw; goto push; }
		else if (is("sd", e)) { op = sd; goto push; }
		else if (is("bltu", e)) { op = bltu; goto push; }
		else if (is("beq", e))  { op = beq; goto push; }
		else if (is("jalr", e)) { op = jalr; goto push; }
		else if (is("jal",  e)) { op = jal; goto push; }

		else if (is("0", e)) 	s <<= 1;                        // todo: delete these eventually. 
		else if (is("1", e)) 	{ r += s; s <<= 1; }            // and make these user-level-made.
		else if (is("=", e)) 	{ push_arg(r); r = 0; s = 1; }  // using ct system / macros.

		else if (is("ctabort",  e))    { op = ctabort; goto push; }
		else if (is("ctdel",  e))      { op = ctdel; goto push; }
		else if (is("ctarg",  e))      { op = ctarg; goto push; }
		else if (is("ctget",  e))      { op = ctget; goto push; }
		else if (is("ctput",  e))      { op = ctput; goto push; }
		else if (is("ctprint",  e))    { op = ctprint; goto push; }
		else if (is("ctdebug",  e))   { op = ctdebug; goto push; }

		else if (is("debugarguments", e)) 	print_arguments();
		else if (is("debugregisters", e)) 	print_registers();
		else if (is("debuginstructions", e))	print_instructions();
		else if (is("debugdictionary", e))	print_dictionary();

		// else if (names[i].is_callonuse) {} // call macro
		else push_arg(values[called_name]);

		*registers = 0;
		goto next_char;

	push:
		if (debug) printf("info:[forwards_branching=%llu]:[is_compiletime=%u]: processing op = %llu...\n", 
					forwards_branching, is_compiletime, op);
		if (forwards_branching) goto next_char;
		if (not is_compiletime) goto push_rt;

		*registers = 0;

		if (op == ctzero) 	registers[a0] = 0;
		else if (op == ctincr) 	registers[a0]++;

		else if (op == ctabort) abort();
		else if (op == ctdel) { if (arg_count) arg_count--; }
		else if (op == ctarg) push_arg(registers[a0]);
		else if (op == ctget) registers[a0] = (nat) getchar();
		else if (op == ctput) putchar((char) registers[a0]);
		else if (op == ctprint) puts(get_name(spot));
		else if (op == ctdebug) printf("debug: \033[32m%llu (%lld)\033[0m "
					"\033[32m0x%llx\033[0m\n", registers[a0], registers[a0], registers[a0]); 

		else if (op == add)   registers[a0] = registers[a1] + registers[a2]; 
		else if (op == sub)   registers[a0] = registers[a1] - registers[a2]; 
		else if (op == mul)   registers[a0] = registers[a1] * registers[a2]; 
		else if (op == div_)  registers[a0] = registers[a1] / registers[a2]; 
		else if (op == and_)  registers[a0] = registers[a1] & registers[a2]; 
		else if (op == or_)   registers[a0] = registers[a1] | registers[a2]; 
		else if (op == xor_)  registers[a0] = registers[a1] ^ registers[a2]; 
		else if (op == slt)   registers[a0] = registers[a1] < registers[a2]; 
		else if (op == sltu)  registers[a0] = registers[a1] < registers[a2]; 

		else if (op == addi)   registers[a0] = registers[a1] + a2; 
		else if (op == andi)   registers[a0] = registers[a1] & a2; 
		else if (op == ori)    registers[a0] = registers[a1] | a2; 
		else if (op == xori)   registers[a0] = registers[a1] ^ a2; 
		else if (op == slti)   registers[a0] = registers[a1] < a2; 
		else if (op == sltiu)  registers[a0] = registers[a1] < a2; 

		else if (op == lb) registers[a0] = *( u8*)(registers[a1] + a2); 
		else if (op == lh) registers[a0] = *(u16*)(registers[a1] + a2); 
		else if (op == lw) registers[a0] = *(u32*)(registers[a1] + a2); 
		else if (op == ld) registers[a0] = *(nat*)(registers[a1] + a2); 

		else if (op == sb) *( u8*)(registers[a0] + a1) = ( u8)registers[a2]; 
		else if (op == sh) *(u16*)(registers[a0] + a1) = (u16)registers[a2]; 
		else if (op == sw) *(u32*)(registers[a0] + a1) = (u32)registers[a2]; 
		else if (op == sd) *(nat*)(registers[a0] + a1) = (nat)registers[a2]; 

		else if (op == bltu) {   
			if (registers[a0] < registers[a1]) {
				if (registers[a2]) {
					index = registers[a2]; 
				} else {
					forwards_branching = a2;
				}
			}
		}

		else if (op == jal) {
			registers[a0] = index;
			if (registers[a1]) index = registers[a1]; else forwards_branching = a1;
		}

		else { puts("unknown ct instruction!"); printf("op = %llu\n", op); abort(); }

		*registers = 0;
		goto next_char;

	push_rt:;
		if (debug) puts("inside push_rt! ...pushing runtime instruction!");
		struct instruction new = {0};
		new.a[0] = (u32) op;
		new.loc[0] = here;
		for (nat i = 1; i < 4; i++) {
			if (i > arg_count) break;
			new.loc[i] = arg_locations[arg_count - i];
			new.a[i] = arguments[arg_count - i];
		}
		
		ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
		ins[ins_count++] = new;
		next_char: continue;
	}


	if (debug) {
		printf("info: building for target:\n\tarchitecture:  \033[31;1m%s\033[0m\n\toutput_format: \033[32;1m%s\033[0m.\n\n", 
			target_spelling[architecture  % target_count], output_format_spelling[output_format % output_format_count]
		);
	}

	if (architecture == noruntime) {

		if (not ins_count) 
			exit(0);

		current_ins = ins[0];
		print_error("encountered runtime instruction with target \"noruntime\"", ins[0].loc[0]);
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
		system("otool -txvVhlL object.o");
		system("otool -txvVhlL program.out");
		system("objdump object.o -DSast --disassembler-options=no-aliases");
		system("objdump program.out -DSast --disassembler-options=no-aliases");	
	}

	munmap(text, length);
}






























/*



// if (op >= beq and op <= jal) new.a[3] = (u32) registers[new.a[3]];







//fileparent[file_count] = (nat) ~0;






//printf("asm: \033[31;1merror:\033[0m %s:%llu:%llu: unresolved symbol\n", argv[1], index, index + imax);







					//puts("backwards branching using regs[a2]!");
					//puts("forwards branching using a2!");









//puts("inside bltu ct impl...");

//puts("inside ctat!");
			//printf("I AM HERE IN THE FILE: %llu\n", index);
			//getchar();





//printf("currently inside of file \"%s\"...\n", filenames[file_head]);



			//nat current = file_head;
			//while (current != (nat) -1) {
			//	files[current].count += l + 1;
			//	current = fileparent[current];
			//}

			//filecounts[file_count] = l + 1;
			
			//fileparent[file_count] = file_head;
			
			//file_head = file_count++;


		//if (index >= files[file_head].start + files[file_head].count) {
		//	file_head = fileparent[file_head];
		//}









			 this is a forwards branch...
					


					// FACT: we need to skip ahead to find the place in the code where the user says:

						//             label name here   ctat


					// we need to find where that is. somehow. 



					// or rather,         we need to skip over instructions,     NOT executing anything anymore, 

												(or generating anything btw!   (rt)) 


					and we need to wait until we EXECUTE a    ctat   instruction,    where the register  is     label name here 

					ie, we keep track of the register given to us,     "registers[a2]" 


						or rather,     a2, specifically         we want a2,            we are looking for the value of a2    to be found being given as an argument to the ctat instruction. and when that happens, we start executing things agian. 




			wow!!!      no reparsing or double parsing of the document neccessary at all!



		neat! 
						just need to set a flag that says we are ignoring stuff, 


					and set it when we do a forwards branch. 


					note,      the flag is actually the value of a2 itself 



						if a2 is zero,    we are executing things         if its nonzero,   then we are skipping, waiting for a  "a2 ctat"  instance to pop up.    note. we cannot skip ctats obviously lol. 




so yeah, thats how it works! nice
202403251.171256
















// todo: add atomic (A) extension, and F/D extensions, as well.  eventually lol.

enum lattntgtutagtet_tItSA {
	ctclear, ctls, ctarg, ctli, ctstop,   // compiletime system.
	ctpc, ctb, ctf, ctblt,
	ctbge, ctbeq, ctbne, ctincr, ctzero,
	ctadd, ctsub, ctmul, ctdiv, ctrem, 
	ctnor, ctxor, ctand, ctor,
	ctsl, ctsr, ctlb, ctlh, ctlw,
	ctld, ctsb, ctsh, ctsw, ctsd,
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
	"u8",
	"ctclear", "ctdel", "ctls", "ctarg", "ctli", "ctstop",
	"ctat", "ctpc", "ctb", "ctf", "ctblt",
	"ctbge", "ctbeq", "ctbne", "ctincr", "ctzero",
	"ctadd", "ctsub", "ctmul", "ctdiv", "ctrem", "ctnor", "ctxor", "ctand", "ctor", 
	"ctsl", "ctsr", "ctlb", "ctlh", "ctlw",
	"ctld", "ctsb", "ctsh", "ctsw", "ctsd",
	"ctprint", "ctabort", "ctget", "ctput",
};















static void process(nat i) {


		const char* const word = text + start;
		const struct location here = {start, count};

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
			filenames[file_count] = strdup(new);
			files[file_count++] = (struct location) {.start = index + 1, .count = l + 1};
			goto next;
		}
	



		for (u32 i = dw; i < instruction_set_count; i++) {
			if (not is(word, count, instruction_spelling[i])) continue;
			if (i < ctclear) push(i, here); else execute(i, &index, text, here);
			goto next;
		}

	
	
		u32 r = 0, s = 1;
		for (nat i = 0; i < count; i++) {
			if (word[i] == '0') s <<= 1;
			else if (word[i] == '1') { r += s; s <<= 1; }
			else goto other_word;
		}
		if (debug) printf("info: pushed %llu onto argument stack\n", (nat) r);
		arg_locations[arg_count] = here;
		arguments[arg_count++] = r;
		
		goto next;
	other_word:
		for (nat i = 0; i < dict_count; i++) {
			if (not is(word, count, names[i])) continue;
			if (debug) printf("[info: found user-defined name: pushed (%s)  value %llu  onto argument stack]\n", 
					names[i], (nat) values[i]);
			arg_locations[arg_count] = here;
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
		print_error(reason, here);
		exit(1);
		next: count = 0;
	


}























Riscv and arm ABI:




arm64:
====================

	x0-x7 : function arguments  			   (call-clobbered) *  A0-A7

	x8 : temporary on macos, callnumber on linux.      (call-clobbered) *       syscall on linux.

	x9-x15 : temporary registers			   (call-clobbered) * 

	x16-x17 : intraprocedural call registers	   (call-clobbered) *       syscall on macos.
	
	x18 : platform specific, not usable at all         (call-clobbered) *   .

	x19-x28 : callee-saved registers 		   (call-preserved)    S0-S9

	x29 : frame pointer  				   (call-preserved)    FP

	x30 : link register				   (call-clobbered) *  LR

	x31 : zero regsiter or stack pointer		   (call-preserved)    SP



has:
	10 (x8,x9-x15,x16,x17) temporaries (call-clobbered) registers
	10 (x19-x28) callee-saved (call-preserved) registers



riscv64:
====================

	x0 : zero register                                     neither

	x1 : return address / link register                (call-clobbered) *  LR

	x2 : stack pointer 				   (call-preserved)    SP

	x3 : global pointer (temporary)                        neither

	x4 : thread pointer (temporary)                        neither

	x5-x7 : temporaries				   (call-clobbered) *

	x8 : frame pointer or callee-saved		   (call-preserved)    FP

	x9 : callee-saved register			   (call-preserved)    

	x10-x17 : function arguments			   (call-clobbered) *  A0-A7

	x18-x27 : callee-saved registers 		   (call-preserved)    S0-S9
	
	x28-x31 : temporaries 				   (call-clobbered) * 


has:
	9 (x3,x4,x5-x7,x28-x31) temporaries (call-clobbered) registers
	11 (x9,x18-x27) callee-saved (call-preserved) registers






	// todo:    wait....  do any of these C-extension ins even do anything that the main instructions can't do!?!? i feel like we can just have the assembler know to use these instructions when appropriate?  and just know how to use the main instructions, to get these instructions, i feel like that makes alot more sense!!! that way it keeps the language both small and portable, and still fully performance-expressable for riscv archs.

	//cnop, caddi4spn, clw, cld, csw, csd,   // C extension
	//caddi, cjal, caddiw, cli, caddi16sp, 
	//clui, csrli, candi, csub, cxor, cor, 
	//cand, csubw, caddw, cj, cbeqz, cbnez, 
	//cslli, clwsp, cldsp, cjr, cmv, cebreak, 
	//cjalr, cadd, cswsp, csdsp, 


// see above.
//	"cnop", "caddi4spn", "clw", "cld", "csw", "csd", 
//	"caddi", "cjal", "caddiw", "cli", "caddi16sp", 
//	"clui", "csrli", "candi", "csub", "cxor", "cor", 
//	"cand", "csubw", "caddw", "cj", "cbeqz", "cbnez", 
//	"cslli", "clwsp", "cldsp", "cjr", "cmv", "cebreak", 
//	"cjalr", "cadd", "cswsp", "csdsp", 


//  make arguments not an array. use a0, a1 and a2 directly, as variables. simpler. 
// or make op be simply arguments[0]. either one. 









*/



































/*          

202403247.203537:



we can't use indicies,  of the dictionary entries






	   the index of a name changes, as we sort it via insertion sort. crap. we will either need to have the sorted version,a nd use the indicies then,  
					ie, order        wait no we don't even know what names they gave the
							crpa


					we need to hard code the names   theres no way around it 

	lol 
						okay so lets do that now.   we will need the equal function. dang it. 
	hm











// static nat literal_r = 0, literal_p = 1;
// static nat stop = 0;
// static bool ct_flag = 0;       // todo: make the ct system more risc-v like.  delete this by making the branch do the check.

//static bool is(const char* word, nat count, const char* this) {
//	return strlen(this) == count and not strncmp(word, this, count);
//}









*/
















/*



static const char* spelling[instruction_set_count] = {
	"ins_eof", 
	"ins_0", 
	"ins_1", 
	"ins_l", 

	"ins_d", 
	"ins_da", 
	"ins_dr", 
	"ins_di", 
	"ins_dd",

	"ins_ar", 
	"ins_of", 
	"ins_on", 
	"ins_en", 
	"ins_po", 
	"ins_pe",

	"ins_del", 
	"ins_arg", 

	"ctabort", 
	"ctprint", 
	"ctmode", 
	"ctat", 
	"ctget", 
	"ctput",


	"ctclear", 
	"ctls", 
	"ctli", 
	"ctstop",

	"ctpc", 
	"ctb", 
	"ctf", 
	"ctblt",
	"ctbge", 
	"ctbeq", 
	"ctbne",
 
	"ctincr", 
	"ctzero",

	"ctadd", 
	"ctsub", 
	"ctmul", 
	"ctdiv", 
	"ctrem", 

	"ctnor", 
	"ctxor", 
	"ctand", 
	"ctor",
	"ctsl", 
	"ctsr",
 
	"ctlb", 
	"ctlh", 
	"ctlw",
	"ctld", 

	"ctsb", 
	"ctsh", 
	"ctsw", 
	"ctsd",

	"db", 
	"dh", 
	"dw", 

	"ecall", 
	"ebreak", 
	"fence", 
	"fencei", 
	
	"add", 
	"sub", 
	"sll", 
	"slt", 
	"sltu", 
	"xor_", 
	"srl", 
	"sra", 
	"or_", 
	"and_", 
	"addw", 
	"subw", 
	"sllw", 
	"srlw", 
	"sraw",
	"lb", 
	"lh", 
	"lw", 
	"ld", 
	"lbu", 
	"lhu", 
	"lwu", 
	"addi", 
	"slti", 
	"sltiu", 
	"xori", 
	"ori", 
	"andi", 
	"slli", 
	"srli", 
	"srai", 
	"addiw", 
	"slliw", 
	"srliw", 
	"sraiw",
	"jalr", 
	"csrrw", 
	"csrrs", 
	"csrrc", 
	"csrrwi", 
	"csrrsi", 
	"csrrci", 
	"sb", 
	"sh", 
	"sw", 
	"sd", 
	"lui", 
	"auipc", 
	"beq", 
	"bne", 
	"blt", 
	"bge", 
	"bltu", 
	"bgeu", 
	"jal", 
	"mul", 
	"mulh", 
	"mulhsu", 
	"mulhu",
	"div_", 
	"divu", 
	"rem", 
	"remu", 
	"mulw", 
	"divw", 
	"divuw", 
	"remw", 
	"remuw", 

	
};

*/







//static nat defining = 0;
//static nat defining_length = 0;
//static nat dict_count = 0; 
//static char* names[4096] = {0};
//static u32 values[4096] = {0};












/*
static void execute(nat op, nat* pc, char* text, struct location here) {
	const nat a2 = arg_count >= 3 ? arguments[arg_count - 3] : 0;
	const nat a1 = arg_count >= 2 ? arguments[arg_count - 2] : 0;
	const nat a0 = arg_count >= 1 ? arguments[arg_count - 1] : 0;

	//if (op == ctstop) { if (registers[a0] == stop) stop = 0; return; }
	//if (stop) return;

	//if (debug) printf("@%llu: info: executing \033[1;32m%s\033[0m(%llu) "
	//		" %lld %lld %lld\n", *pc, instruction_spelling[op], op, a0, a1, a2);

	//if (op == ctclear) arg_count = 0;
	

//	else if (op == ctarg)  {
//		arg_locations[arg_count] = here;
//		arguments[arg_count++] = (u32) registers[a0]; 
//	}

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
	
	
}
*/












	/*
		

		else if (e == ctli)   registers[a0] = a1;
		else if (e == ctat)   registers[a0] = ins_count;
		else if (e == ctpc)   registers[a0] = index;
		else if (e == ctb)    { if (ct_flag) index = registers[a0]; }
		else if (e == ctf)    { if (ct_flag) stop = registers[a0]; }
		else if (e == ctblt)  ct_flag = registers[a0]  < registers[a1];
		else if (e == ctbge)  ct_flag = registers[a0] >= registers[a1];
		else if (e == ctbeq)  ct_flag = registers[a0] == registers[a1];
		else if (e == ctbne)  ct_flag = registers[a0] != registers[a1];

		else if (e == ctincr) 
		else if (e == ctzero) 
		
		else if (e == ctsub)  registers[a0] = registers[a1] - registers[a2]; 
		else if (e == ctmul)  registers[a0] = registers[a1] * registers[a2]; 
		else if (e == ctdiv)  registers[a0] = registers[a1] / registers[a2]; 
		else if (e == ctrem)  registers[a0] = registers[a1] % registers[a2]; 
		else if (e == ctxor)  registers[a0] = registers[a1] ^ registers[a2]; 
		else if (e == ctand)  registers[a0] = registers[a1] & registers[a2]; 
		else if (e == ctor)   registers[a0] = registers[a1] | registers[a2]; 
		else if (e == ctnor)  registers[a0] = ~(registers[a1] | registers[a2]); 
		else if (e == ctsl)   registers[a0] = registers[a1] << registers[a2]; 
		else if (e == ctsr)   registers[a0] = registers[a1] >> registers[a2]; 

		else if (e == ctlb)  registers[a0] = *(u8*) registers[a1]; 
		else if (e == ctlh)  registers[a0] = *(u16*)registers[a1]; 
		else if (e == ctlw)  registers[a0] = *(u32*)registers[a1]; 
		else if (e == ctld)  registers[a0] = *(nat*)registers[a1]; 

		else if (e == ctsb)  *(u8*) registers[a0] = (u8)  registers[a1]; 
		else if (e == ctsh)  *(u16*)registers[a0] = (u16) registers[a1]; 
		else if (e == ctsw)  *(u32*)registers[a0] = (u32) registers[a1]; 
		else if (e == ctsd)  *(nat*)registers[a0] = (nat) registers[a1]; 
		
		
			
		


	*/

	









/*
					we left off here

			make this function not use  lengths[]       instead, determine the length of the string ourselves, including all whitespace!!




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


prveiosu implmentation:


	return strndup(text + names[name], lengths[name]);
*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
















//   WRONG:     we solved this properly actually. 

    //  TODO:   problem:   we arent translating the syscall number   regs  properly between riscv-linux (a7)   and arm64 linux/arm64 macos. we need to do this properly! 





















/*	
	const nat this = spot.start;


	nat index = 0;

	nat file = (nat) -1;
	for (nat i = 0; i < file_count; i++) {

	
		const nat start = files[i].start;
		const nat end = files[i].start + files[i].count;
		
		if (this >= end) {

			printf("info: error was not in file \"%s\"... (spot=%llu >= end=%llu)\n", filenames[i], this, end);

			index += filecounts[i];
			printf("[%s]: added %llu to index... (index now %llu)\n", filenames[i], files[i].count, index);

		} else { 
			printf("info: overwriting \"file\" to look at i = \"%s\"... (spot=%llu < end=%llu)\n", filenames[i], this, end);
			file = i; 

			//if (i) { 
			//	index -= files[i].start;
			//	printf("[%s]: subtracted %llu from index... (index now %llu)\n", filenames[i], files[i].start, index);
			//}
		} 
	}
	
	if (file == (nat) -1) {
		puts("error: could not find any file with the right .start, .counts...\n");
		abort();
	} else 
		printf("info: spot was belevied to be located in file \"%s\"...\n", filenames[file]);


	//spot.start -= files[file].start;
	spot.start -= index;

	printf("FINAL RESULT = %lld\n", spot.start);
*/	




/*	struct instruction this = current_ins;
	for (nat a = 0; a < 4; a++) {
		for (nat i = 1; i < file_count; i++) {
			if (files[i].start < this.loc[a].start) this.loc[a].start -= files[i].count;
		}
	}


	//for (nat i = 0; i < 4; i++) fprintf(stderr, "%llu,%llu|", this.loc[i].start, this.loc[i].count);



//nat file = 0;



			puts("found correct file!");
			puts(filenames[i]);

			spot.start -= files[i].start;

			file = i;

			break;

			

		}



*/










//static nat filecounts[4096] = {0};
//static nat fileparent[4096] = {0};



