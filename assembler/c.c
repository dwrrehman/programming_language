#include <stdio.h>    // arm64bit assembler written by dwrr on 202309262.203001
#include <stdlib.h>   // made for only my own personal use, not for anyone else. 
#include <string.h>   // and made specifically for the Macbook Pro's M1 Max CPU.
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

enum instruction_type {
	nop, svc, cfinv, 

	br, 	b_, 	ba, 
	bz, 	bc, 	bn, 	bv, 
	bcz, 	bnv, 	bnvz, 	
	bzn, 	bcn, 	bnn, 	bvn, 
	bczn, 	bnvn, 	bnvzn, 

	movzx, movzw,	movkx, movkw,	movnx, movnw,
	addix, addiw,	addhx, addhw,
	addixs, addiws,	addhxs, addhws,

	adcx, adcw, 	adcxs, adcws, 
	asrvx, asrvw, 

	orrx, orrw,	ornx, ornw, 
	addx, addw, 	addxs, addws,
	subx, subw, 	subxs, subws,

	ld64b, 	st64b,	absx, absw, 
	clsx, clsw,	clzx, clzw,	ctzx, ctzw,	cntx, cntw,    
	rbitx, rbitw,	revx, revw,  	revhx, revhw,

	ctzero, ctincr,
	ctadd, ctsub, ctmul, ctdiv, ctrem,
	ctnor, ctxor, ctor, ctand, ctshl, ctshr, ctprint, 
	ctld1, ctld2, ctld4, ctld8, ctst1, ctst2, ctst4, ctst8,
	ctat, ctpc, ctblt, ctgoto, ctstop,

	instruction_set_count
};

static const char* instruction_spelling[instruction_set_count] = {
	"nop", "svc", "cfinv", 

	"br", 	"b",  	"ba", 
	"bz", 	"bc", 	"bn", 	"bv", 
	"bcz", 	"bnv", 	"bnvz", 
	"bzn", 	"bcn", 	"bnn", 	"bvn", 
	"bczn", "bnvn", "bnvzn", 

	"movzx", "movzw", "movkx", "movkw",	"movnx", "movnw",
	"addix", "addiw", "addhx", "addhw",
	"addixs", "addiws", "addhxs", "addhws",

	"adcx", "adcw", "adcxs", "adcws", 
	"asrvx", "asrvw", 

	"orrx", "orrw",	"ornx", "ornw", 
	"addx", "addw", "addxs", "addws",
	"subx", "subw", "subxs", "subws",

	"ld64b", "st64b", "absx", "absw",
	"clsx", "clsw",	"clzx", "clzw",	"ctzx", "ctzw",	"cntx", "cntw",
	"rbitx", "rbitw", "revx", "revw", "revhx", "revhw",

	"ctzero", "ctincr",
	"ctadd", "ctsub", "ctmul", "ctdiv", "ctrem",
	"ctnor", "ctxor", "ctor", "ctand", "ctshl", "ctshr", "ctprint",
	"ctld1", "ctld2", "ctld4", "ctld8", "ctst1", "ctst2", "ctst4", "ctst8",
	"ctat", "ctpc", "ctblt", "ctgoto", "ctstop",
};

struct word {
	char* name;
	nat length;
	nat type;
	nat reg;
	nat file_index;
};

struct argument {
	nat value;
	nat start;
	nat count;
};

struct instruction {
	nat op;
	nat start;
	nat count;
	struct argument arguments[6];
};


static nat ins_count = 0;
static struct instruction ins[4096] = {0};

static nat word_count = 0;
static struct word words[4096] = {0};

static nat byte_count = 0;
static uint8_t bytes[4096] = {0};

static nat text_length = 0;
static char* text = NULL;
static const char* filename = NULL;

static nat arg_count = 0;
static struct argument arguments[6] = {0};

static nat registers[4096] = {0};

static bool is(char* word, nat count, const char* this) {
	return strlen(this) == count and not strncmp(word, this, count);
}

static bool equals(const char* word, nat count, const char* word2, nat count2) {
	return count == count2 and not strncmp(word, word2, count);
}

static nat read_file(const char* name) {
	int d = open(name, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }

	const int file = open(name, O_RDONLY, 0);
	if (file < 0) { read_error: perror("open"); exit(1); }

	size_t length = (size_t) lseek(file, 0, SEEK_END);
	text = malloc(length);
	lseek(file, 0, SEEK_SET);
	read(file, text, length);
	close(file); 

	return length;
}

static void print_error(const char* reason, const nat start_index, const nat error_word_length) {

	const nat end_index = start_index + error_word_length;

	nat at = 0, line = 1, column = 1;
	while (at < start_index) {
		if (text[at++] == '\n') { line++; column = 1; } else column++;
	}
	fprintf(stderr, "\033[1m%s:%llu:%llu:%llu:%llu: \033[1;31merror:\033[m \033[1m%s\033[m\n", 
			filename, start_index, end_index, line, column, reason);

	nat w = 0;
	nat b = line > 2 ? line - 2 : 0, e = line + 2;
	for (nat i = 0, l = 1, c = 1; i < text_length; i++) {
		if (c == 1 and l >= b and l <= e)  printf("\033[0m\n\033[90m%5llu\033[0m\033[32m │ \033[0m", l);
		if (text[i] != 10 and l >= b and l <= e) {
			if (i == start_index) printf("\033[1;33m");
			if (i == end_index) printf("\033[0m");
			if (i < start_index) { if (text[i] == '\t') w += 8 - w % 8; else w++; }
			printf("%c", text[i]); 
		}
		if (text[i] == 10) { l++; c = 1; } else c++;
		if (l == line + 1 and c == 1) {
			printf("\033[0m\n\033[90m%5s\033[0m\033[32m │ \033[0m", " ");
			for (nat ii = 0; ii < w; ii++) printf(" ");
			printf("\033[1;32m^");
			signed long long x = (signed long long)end_index - (signed long long)start_index - 1;
			if (x < 0) x = 0;
			for (nat ii = 0; ii < (nat) x; ii++) printf("~");
			printf("\033[0m");
		} 
		if (text[i] == 10) w = 0; 

	}
	puts("\033[0m\n");
}

static void push(nat op, nat start, nat count) {
	struct instruction new = {.op = op, .arguments = {0}, .start = start, .count = count};
	memcpy(new.arguments, arguments, sizeof new.arguments);
	ins[ins_count++] = new;
	arg_count = 0;
}

static void emit(u32 x) {
	bytes[byte_count++] = (uint8_t) (x >> 0);
	bytes[byte_count++] = (uint8_t) (x >> 8);
	bytes[byte_count++] = (uint8_t) (x >> 16);
	bytes[byte_count++] = (uint8_t) (x >> 24);
}

static void check(nat r, nat c, const struct argument a, const char* type) {
	if (r < c) return;
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "invalid %s argument %llu (%llu >= %llu)", type, r, r, c);
	print_error(reason, a.start, a.count); 
	exit(1);
}

static u32 generate_mov(struct argument* a, u32 sf, u32 op) { 
	const nat Im = a[0].value;
	const u32 hw = (u32) a[1].value;
	const u32 Rd = (u32) a[2].value;
	check(Im, 4096, a[0], "ctregister");
	check(hw, 4,  a[1], "register");
	check(Rd, 32, a[2], "register");
	const nat imm = registers[Im];
	check(imm, 65536, a[0], "immediate");
	return  (sf << 31U) | 
		(op << 23U) | 
		(hw << 21U) | 
		((u32) imm << 5U) | 
		 Rd;
}

static u32 generate_addi(struct argument* a, u32 sf, u32 sh, u32 op) { 
	const nat Im = a[0].value;
	const u32 Rn = (u32) a[1].value;
	const u32 Rd = (u32) a[2].value;
	check(Rn, 32, a[1], "register");
	check(Rd, 32, a[2], "register");
	check(Im, 4096, a[0], "ctregister");
	const nat imm = registers[Im];
	check(imm, 4096, a[0], "immediate");
	return  (sf << 31U) | 
		(op << 23U) | 
		(sh << 22U) | 
		((u32) imm << 10U) | 
		(Rn << 5U)  | 
		 Rd;
}

static u32 generate_adc(struct argument* a, u32 sf, u32 op, u32 o2) {  
	const u32 Rm = (u32) a[0].value;
	const u32 Rn = (u32) a[1].value;
	const u32 Rd = (u32) a[2].value;
	check(Rm, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	check(Rd, 32, a[2], "register");
	return  (sf << 31U) | 
		(op << 21U) | 
		(Rm << 16U) | 
		(o2 << 10U) | 
		(Rn << 5U)  | 
		 Rd;
}

static u32 generate_orr(struct argument* a, u32 sf, u32 ne, u32 op) { 

	const nat Im = 	     a[0].value;
	const u32 sh = (u32) a[1].value;
	const u32 Rm = (u32) a[2].value;
	const u32 Rn = (u32) a[3].value;
	const u32 Rd = (u32) a[4].value;

	check(Im, 4096, a[0], "ctregister");
	check(sh, 4,  a[1], "register");
	check(Rm, 32, a[2], "register");
	check(Rn, 32, a[3], "register");
	check(Rd, 32, a[4], "register");

	const nat imm = registers[Im];
	check(imm, 32U << sf, a[0], "immediate");

	return  (sf << 31U) | 
		(op << 24U) | 
		(sh << 22U) | 
		(ne << 21U) | 
		(Rm << 16U) | 
		((u32) imm << 10U) | 
		(Rn << 5U)  | 
		 Rd;
}

static u32 generate_br(struct argument* a) { 
	const u32 Rn = (u32) a[0].value;
	check(Rn, 32, a[0], "register");
	return (0x3587C0U << 10U) | (Rn << 5U);
}

static u32 generate_b(struct argument* a, uint32_t pc) { 
	const u32 Im = (u32) a[0].value;
	check(Im, 4096, a[0], "ctregister");
	return (0x05 << 26U) | (0x03FFFFFFU & ((uint32_t) registers[Im] - pc));
}

static u32 generate_bc(struct argument* a, uint32_t pc, uint32_t cond) { 
	const u32 Im = (u32) a[0].value;
	check(Im, 4096, a[0], "ctregister");
	return (0x54U << 24U) | ((0x0007FFFFU & ((uint32_t) registers[Im] - pc)) << 5U) | cond;
}

static u32 generate_abs(struct argument* a, u32 sf, u32 op) {  
	const u32 Rn = (u32) a[0].value;
	const u32 Rd = (u32) a[1].value;
	check(Rn, 32, a[0], "register");
	check(Rd, 32, a[1], "register");
	return  (sf << 31U) | 
		(op << 10U) | 
		(Rn << 5U)  | 
		 Rd;
}

static void dump_hex(uint8_t* local_bytes, nat local_byte_count) {
	printf("dumping hex bytes: (%llu)\n", local_byte_count);
	for (nat i = 0; i < local_byte_count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx ", local_bytes[i]);
	}
	puts("");
}



static nat stack_count = 0;
static nat stack[4096] = {0};

static nat return_count = 0;
static char* return_word = NULL;
static nat macro = 0;

static nat stop = 0;

static void execute(nat op, nat* pc) {
	const nat a0 = arguments[0].value;
	const nat a1 = arguments[1].value;
	const nat a2 = arguments[2].value;

	if (op == ctstop) {if (registers[a0] == stop) stop = 0; arg_count = 0; return; }
	else if (stop) return;

	if (false) {}
	else if (op == ctat)   registers[a0] = ins_count;
	else if (op == ctpc)   registers[a0] = *pc;
	else if (op == ctgoto) *pc = registers[a0]; 
	else if (op == ctblt)  { if (registers[a1] < registers[a0]) stop = registers[a2]; } 
	else if (op == ctincr) registers[a0]++; 
	else if (op == ctzero) registers[a0] = 0; 
	else if (op == ctadd)  registers[a2] = registers[a1] + registers[a0]; 
	else if (op == ctsub)  registers[a2] = registers[a1] - registers[a0]; 
	else if (op == ctmul)  registers[a2] = registers[a1] * registers[a0]; 
	else if (op == ctdiv)  registers[a2] = registers[a1] / registers[a0]; 
	else if (op == ctrem)  registers[a2] = registers[a1] % registers[a0]; 
	else if (op == ctxor)  registers[a2] = registers[a1] ^ registers[a0]; 
	else if (op == ctor)   registers[a2] = registers[a1] | registers[a0]; 
	else if (op == ctand)  registers[a2] = registers[a1] & registers[a0]; 
	else if (op == ctnor)  registers[a2] = ~(registers[a1] | registers[a0]); 
	else if (op == ctshl)  registers[a2] = registers[a1] << registers[a0]; 
	else if (op == ctshr)  registers[a2] = registers[a1] >> registers[a0]; 
	else if (op == ctprint)  printf("debug: \033[32m%llu\033[0m\n", registers[a0]); 

	else if (op == ctld1)  registers[a1] = *(uint8_t*) registers[a0]; 
	else if (op == ctld2)  registers[a1] = *(uint16_t*)registers[a0]; 
	else if (op == ctld4)  registers[a1] = *(uint32_t*)registers[a0]; 
	else if (op == ctld8)  registers[a1] = *(uint64_t*)registers[a0]; 

	else if (op == ctst1)  *(uint8_t*) registers[a1] = (uint8_t)  registers[a0]; 
	else if (op == ctst2)  *(uint16_t*)registers[a1] = (uint16_t) registers[a0]; 
	else if (op == ctst4)  *(uint32_t*)registers[a1] = (uint32_t) registers[a0]; 
	else if (op == ctst8)  *(uint64_t*)registers[a1] = (uint64_t) registers[a0]; 
	
	arg_count = 0;
}

static void generate(void) {

	for (nat i = 0; i < ins_count; i++) {

		const nat op = ins[i].op;
		struct argument* const a = ins[i].arguments;

		     if (op == svc)    emit(0xD4000001);
		else if (op == nop)    emit(0xD503201F);
		else if (op == cfinv)  emit(0xD500401F);

		else if (op == br)     emit(generate_br(a));
		else if (op == b_)     emit(generate_b(a, (uint32_t) i));
		else if (op == ba)     emit(generate_bc(a, (uint32_t) i, 0xE));

		else if (op == bz)     emit(generate_bc(a, (uint32_t) i, 0x0));
		else if (op == bc)     emit(generate_bc(a, (uint32_t) i, 0x2));
		else if (op == bn)     emit(generate_bc(a, (uint32_t) i, 0x4));
		else if (op == bv)     emit(generate_bc(a, (uint32_t) i, 0x6));
		else if (op == bcz)    emit(generate_bc(a, (uint32_t) i, 0x8));
		else if (op == bnv)    emit(generate_bc(a, (uint32_t) i, 0xA));
		else if (op == bnvz)   emit(generate_bc(a, (uint32_t) i, 0xC));

		else if (op == bzn)    emit(generate_bc(a, (uint32_t) i, 0x1));
		else if (op == bcn)    emit(generate_bc(a, (uint32_t) i, 0x3));
		else if (op == bnn)    emit(generate_bc(a, (uint32_t) i, 0x5));
		else if (op == bvn)    emit(generate_bc(a, (uint32_t) i, 0x7));
		else if (op == bczn)   emit(generate_bc(a, (uint32_t) i, 0x9));
		else if (op == bnvn)   emit(generate_bc(a, (uint32_t) i, 0xB));
		else if (op == bnvzn)  emit(generate_bc(a, (uint32_t) i, 0xD));

		else if (op == absx)   emit(generate_abs(a, 1, 0x16B008U));
		else if (op == absw)   emit(generate_abs(a, 0, 0x16B008U));
		else if (op == clzx)   emit(generate_abs(a, 1, 0x16B004U));
		else if (op == clzw)   emit(generate_abs(a, 0, 0x16B004U));
		else if (op == clsx)   emit(generate_abs(a, 1, 0x16B005U));
		else if (op == clsw)   emit(generate_abs(a, 0, 0x16B005U));
		else if (op == ctzx)   emit(generate_abs(a, 1, 0x16B006U));
		else if (op == ctzw)   emit(generate_abs(a, 0, 0x16B006U));
		else if (op == cntx)   emit(generate_abs(a, 1, 0x16B007U));
		else if (op == cntw)   emit(generate_abs(a, 0, 0x16B007U));
		else if (op == rbitx)  emit(generate_abs(a, 1, 0x16B000U));
		else if (op == rbitw)  emit(generate_abs(a, 0, 0x16B000U));
		else if (op == revx)   emit(generate_abs(a, 1, 0x16B003U));
		else if (op == revw)   emit(generate_abs(a, 1, 0x16B002U));
		else if (op == revhx)  emit(generate_abs(a, 1, 0x16B001U));
		else if (op == revhw)  emit(generate_abs(a, 0, 0x16B001U));
		else if (op == ld64b)  emit(generate_abs(a, 1, 0x1E0FF4U));
		else if (op == st64b)  emit(generate_abs(a, 1, 0x1E0FE4U));

		else if (op == movzx)  emit(generate_mov(a, 1, 0xA5U));
		else if (op == movzw)  emit(generate_mov(a, 0, 0xA5U));
		else if (op == movkx)  emit(generate_mov(a, 1, 0xE5U));
		else if (op == movkw)  emit(generate_mov(a, 0, 0xE5U));
		else if (op == movnx)  emit(generate_mov(a, 1, 0x25U));
		else if (op == movnw)  emit(generate_mov(a, 0, 0x25U));

		else if (op == addix)  emit(generate_addi(a, 1, 0, 0x22U));
		else if (op == addiw)  emit(generate_addi(a, 0, 0, 0x22U));
		else if (op == addhx)  emit(generate_addi(a, 1, 1, 0x22U));
		else if (op == addhw)  emit(generate_addi(a, 0, 1, 0x22U));
		else if (op == addixs) emit(generate_addi(a, 1, 0, 0x62U));
		else if (op == addiws) emit(generate_addi(a, 0, 0, 0x62U));
		else if (op == addhxs) emit(generate_addi(a, 1, 1, 0x62U));
		else if (op == addhws) emit(generate_addi(a, 0, 1, 0x62U));

		else if (op == adcx)   emit(generate_adc(a, 1, 0x0D0U, 0x00));
		else if (op == adcw)   emit(generate_adc(a, 0, 0x0D0U, 0x00));
		else if (op == adcxs)  emit(generate_adc(a, 1, 0x1D0U, 0x00));
		else if (op == adcws)  emit(generate_adc(a, 0, 0x1D0U, 0x00));
		else if (op == asrvx)  emit(generate_adc(a, 1, 0x0D6U, 0x0A));
		else if (op == asrvw)  emit(generate_adc(a, 0, 0x0D6U, 0x0A));

		//else if (op == cselx)  emit(generate_adc(a, 0, 0x0D6U, 0x0A));
		//else if (op == cselx)  emit(generate_adc(a, 0, 0x0D6U, 0x0A));

		else if (op == orrx)   emit(generate_orr(a, 1, 0, 0x2AU));
		else if (op == orrw)   emit(generate_orr(a, 0, 0, 0x2AU));
		else if (op == ornx)   emit(generate_orr(a, 1, 1, 0x2AU));
		else if (op == ornw)   emit(generate_orr(a, 0, 1, 0x2AU));
		else if (op == addx)   emit(generate_orr(a, 1, 0, 0x0BU));
		else if (op == addw)   emit(generate_orr(a, 0, 0, 0x0BU));
		else if (op == addxs)  emit(generate_orr(a, 1, 0, 0x2BU));
		else if (op == addws)  emit(generate_orr(a, 0, 0, 0x2BU));
		else if (op == subx)   emit(generate_orr(a, 1, 0, 0x4BU));
		else if (op == subw)   emit(generate_orr(a, 0, 0, 0x4BU));
		else if (op == subxs)  emit(generate_orr(a, 1, 0, 0x6BU));
		else if (op == subws)  emit(generate_orr(a, 0, 0, 0x6BU));
		else {
			printf("error: unknown instruction: %llu\n", op);
			printf("       unknown instruction: %s\n", instruction_spelling[op]);
			abort();
		}
	}
}

int main(int argc, const char** argv) {
	if (argc < 2) return puts("usage: assembler <file1.asm> <file2.asm> ... <filen.asm>");

	filename = argv[1];
	text_length = read_file(filename);

	nat count = 0, start = 0;

	*registers = (nat)(void*) malloc(65536);

	static nat index = 0; 
	for (index = 0; index < text_length; index++) {
		if (not isspace(text[index])) { 
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;

	process:;
		char* const word = text + start;
		struct argument arg = { .value = 0, .start = start, .count = count };

		if (macro) {
			if (equals(word, count, return_word, return_count)) {
				macro = 0; goto next;
			} else goto next;
		}

		//if (count == return_count_stack[stack_count - 1] and 
			//	is(word, count, return_stack[])) {}
			//continue;


		for (nat i = 0; i < 4096; i++) {
			char r[5] = {0};
			snprintf(r, sizeof r, "%llu", i);
			if (is(word, count, r)) { 
				arg.value = i;
				if (arg_count >= 6) {
					char reason[4096] = {0};
					snprintf(reason, sizeof reason, "argument list full");
					print_error(reason, start, count); 
					exit(1);
				}
				if (not stop) arguments[arg_count++] = arg; 
				goto next;
			}
		}

		for (nat i = nop; i < instruction_set_count; i++) {
			if (not is(word, count, instruction_spelling[i])) continue;
			if (i >= ctzero) execute(i, &index); 
			else if (not stop) push(i, start, count);
			goto next;
		}

		words[word_count++] = (struct word) {
			.name = word,
			.length = count,
			.reg = ins_count,
			.file_index = start,
		};


		if (not arg_count) { 
			

			macro = 1;
			return_word = word;
			return_count = count;
			goto next;

		} else {

			printf("simple register alias macro!!!!\n");

			// ...
			// d cfinv d
			// printf("comment / operational macro!!!\n");

			//arg.value = ~word_count; 
			//arguments[arg_count++] = arg; 
			// getchar();
		}
		
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, 
			"undefined word found \"%.*s\"", 
			(int) count, word
		);
		print_error(reason, start, count);
		exit(1);


		next: count = 0;
	}

	if (count) goto process;


	if (macro) {
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "unterminated operation macro");
		print_error(reason, start, count);
		exit(1);
	}

	generate();


	struct mach_header_64 header = {0};
	header.magic = MH_MAGIC_64;
	header.cputype = (int)CPU_TYPE_ARM | (int)CPU_ARCH_ABI64;
	header.cpusubtype = (int) CPU_SUBTYPE_ARM64_ALL;
	header.filetype = MH_OBJECT;
	header.ncmds = 1;
	header.sizeofcmds = 0;
	header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;
	

	struct segment_command_64 segment = {0};
	segment.cmd = LC_SEGMENT_64;
	segment.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64) * 1;

	header.sizeofcmds += segment.cmdsize;

	strncpy(segment.segname, "__TEXT", 16);
	segment.vmsize = 
			sizeof (struct mach_header_64) + 
			sizeof (struct segment_command_64) + 
			sizeof(struct section_64) * 1 + 
			byte_count;

	segment.vmaddr = 0;
	segment.fileoff = 0;
	segment.filesize = 
			sizeof (struct mach_header_64) + 
			sizeof (struct segment_command_64) + 
			sizeof(struct section_64) * 1 + 
			byte_count;

	segment.maxprot = (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
	segment.initprot = (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
	segment.nsects = 1;
	

	struct section_64 section = {0};
	strncpy(section.sectname, "__text", 16);
	strncpy(section.segname, "__TEXT", 16);
	section.addr = 0;
	section.size = byte_count;
	section.offset = sizeof (struct mach_header_64) + sizeof (struct segment_command_64) + sizeof section * 1;
	section.align = 3;
	section.reloff = 0;
	section.nreloc = 0;
	section.flags = S_ATTR_PURE_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS;

	struct symtab_command table  = {0};
	table.cmd               = LC_SYMTAB;
	table.cmdsize           = sizeof(struct symtab_command);
	table.symoff            = 0;   
	table.nsyms             = 1;
	table.stroff            = 0;   
	table.strsize           = 0;

	const char strings[] = "\0_start\0";

	struct nlist_64 symbols[] = {
	        (struct nlist_64) {
	            .n_un.n_strx = 1,
	            .n_type = N_SECT | N_EXT,  
	            .n_sect = 1,
	            .n_desc = REFERENCE_FLAG_DEFINED,
	            .n_value = 0,
	        }
	};


	const char* output_filename = "object.o";
	const int flags = O_WRONLY | O_CREAT | O_TRUNC;               // | O_EXCL;
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(output_filename, flags, mode);

	if (file < 0) { perror("open"); exit(1); }

	header.ncmds = 2; 

	header.sizeofcmds = 	sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	write(file, &header, sizeof(struct mach_header_64));
	//counter += sizeof(struct mach_header_64);

	segment.vmsize = byte_count;
	segment.filesize = byte_count;
	segment.fileoff = header.sizeofcmds + sizeof(struct mach_header_64); 
	segment.nsects = 1;

	write(file, &segment, sizeof (struct segment_command_64));
	//counter += sizeof(struct segment_command_64);

	section.size = segment.filesize;
	section.offset = (uint32_t) segment.fileoff;
	section.reloff = (uint32_t) (segment.fileoff + segment.filesize); 
	section.nreloc = 0;

	write(file, &section, sizeof(struct section_64));
	//counter += sizeof(struct section_64);

	table.symoff = section.reloff; 
	table.nsyms = 1; 
	table.stroff = table.symoff + 1 * sizeof(struct nlist_64);
	table.strsize = sizeof(strings);

	write(file, &table, sizeof table);
	//counter += sizeof(table);

	write(file, bytes, byte_count);
	write(file, symbols, sizeof(struct nlist_64));
	write(file, strings, sizeof strings);
	close(file);

	printf("\ndebugging bytes bytes:\n------------------------\n");
	dump_hex((uint8_t*) bytes, byte_count);

	system("otool -txvVhlL object.o");
	system("objdump object.o -DSast --disassembler-options=no-aliases");

	system("ld -v "
		"object.o " 
		"-o executable.out "
		"-arch arm64 "
		"-e _start "
		"-platform_version macos 13.0.0 13.3 "
		"-lSystem "
		"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk "
	);
}














// printf("generating orr:  sf=%u op=%u sh=%u ne=%u Rm=%u imm=%llu Rn=%u Rd=%u\n", sf, op, sh, ne, Rm, imm, Rn, Rd); 



/*
	system("/Library/Developer/CommandLineTools/usr/bin/ld -v "
		"-demangle "
		"-lto_library /Library/Developer/CommandLineTools/usr/lib/libLTO.dylib "
		"-dynamic "
		"-arch arm64 "
		"-e _start "
		"-platform_version macos 13.0.0 13.3 "
		"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk "
		"-o executable.out "
		"-L/usr/local/lib "
		"object.o "
		"-lSystem"        //""/Library/Developer/CommandLineTools/usr/lib/clang/14.0.3/lib/darwin/libclang_rt.osx.a"
	);
*/


/* ----- dynamic simpler linking -------------


	system("/Library/Developer/CommandLineTools/usr/bin/ld -v "
		//"-demangle "
		//"-lto_library /Library/Developer/CommandLineTools/usr/lib/libLTO.dylib "
		//"-dynamic "
		"-arch arm64 "
		"-e _start "
		"-platform_version macos 13.0.0 13.3 "
		"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk "
		"-o executable.out "
		//"-L/usr/local/lib "
		"object.o "
		"-lSystem"        //""/Library/Developer/CommandLineTools/usr/lib/clang/14.0.3/lib/darwin/libclang_rt.osx.a"
	);

----- final linking ------------- */  






/*



	system("otool -txvVhlL object.o");
	system("objdump object.o -DSast --disassembler-options=no-aliases");

	system("/Library/Developer/CommandLineTools/usr/bin/ld -v "
		"-demangle "
		"object.o "
		"-o executable.out "
		"-lSystem Library/Developer/CommandLineTools/usr/lib/clang/14.0.3/lib/darwin/libclang_rt.osx.a "
		"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk "
		"-e _main "
		"-arch arm64 "
	);

	//system("ld -v -dynamic -arch arm64 object.o -o executable.out -e _start -lSystem -syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk");

	system("otool -txvVhlL executable.out");







symtabCommand.symoff = sectionText.reloff + sectionText.nreloc * sizeof(relocation_info);
symtabCommand.nsyms = 1; 

symtabCommand.stroff = symtabCommand.symoff + symtabCommand.nsyms * sizeof(nlist_64);

symtabCommand.strsize = sizeof(stringTable);

fwrite(&symtabCommand, 1, sizeof(symtabCommand), binary);
offsetCounter += sizeof(symtabCommand);

fwrite(&dysymtabCommand, 1, sizeof(dysymtabCommand), binary);
offsetCounter += sizeof(dysymtabCommand);

fwrite(&code, 1, sizeof(code), binary);

fwrite(&relocations, 1, sizeof(relocations), binary);

fwrite(&symbols, 1, sizeof(symbols), binary);

fwrite(&stringTable, 1, sizeof(stringTable), binary);

*/


	//printf("generating object file \"%s\" comprised of %llu bytes...\n", 
	//	output_filename, sizeof(struct mach_header_64) + sizeof command + sizeof section + byte_count);






//	printf("\ndebugging header bytes:\n------------------------\n");
//	dump_hex((uint8_t*) &header, sizeof header);

//	printf("\ndebugging command bytes:\n------------------------\n");
//	dump_hex((uint8_t*) &command, sizeof command);

//	printf("\ndebugging section bytes:\n------------------------\n");
//	dump_hex((uint8_t*) &section, sizeof section);






































/*
	strncpy(section.sectname, "__data", 16);
	strncpy(section.segname, "__TEXT", 16);
	section.addr = 0;
	section.size = byte_count;
	section.offset = sizeof header + sizeof command + sizeof section * 2;
	section.align = 3;
	section.reloff = 0;
	section.nreloc = 0;
*/








/*


// printf("user-defined label: \"%.*s\"... ignoring\n", (int) count, word);
				// labels[label_count++] = (struct label) {.name = word, .length = count};



	struct mach_header {
	   uint32_t magic;
	   cpu_type_t cputype;
	   cpu_subtype_t cpusubtype;
	   uint32_t filetype;
	   uint32_t ncmds;
	   uint32_t sizeofcmds;
	   uint32_t flags;
	};
*/

























/*



	align8();

	const int number_of_sections = 1;

	struct mach_header_64 header = {0};	
	struct segment_command_64 command = {0};
	struct section_64 section = {0};


	header.magic = MH_MAGIC_64;
	header.cputype = (int)CPU_TYPE_X86 | (int)CPU_ARCH_ABI64;
	header.cpusubtype = (int)CPU_SUBTYPE_I386_ALL | (int)CPU_SUBTYPE_LIB64;
	header.filetype = MH_OBJECT;
	header.ncmds = 1;
	header.sizeofcmds = 0;
	header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;
	
	command.cmd = LC_SEGMENT_64;
	command.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64) * number_of_sections;

	header.sizeofcmds += command.cmdsize;

	strncpy(command.segname, "__TEXT", 16);
	command.vmsize = sizeof header + sizeof command + sizeof section * number_of_sections + size;
	command.vmaddr = 0;
	command.fileoff = 0;
	command.filesize = sizeof header + sizeof command + sizeof section * number_of_sections + size;
	command.maxprot = VM_PROT_ALL;
	command.initprot = VM_PROT_ALL;
	command.nsects = number_of_sections;
	
	strncpy(section.sectname, "__text", 16);
	strncpy(section.segname, "__TEXT", 16);
	section.addr = 0;
	section.size = size;
	section.offset = sizeof header + sizeof command + sizeof section * number_of_sections;
	section.align = 3;
	section.reloff = 0;
	section.nreloc = 0;

	printf("\ndebugging header bytes:\n------------------------\n");
	dumphex((void*) &header, sizeof(header));

	printf("\ndebugging command bytes:\n------------------------\n");
	dumphex((void*) &command, sizeof(command));

	printf("\ndebugging section bytes:\n------------------------\n");
	dumphex((void*) &section, sizeof(section));

	printf("\ndebugging bytes bytes:\n------------------------\n");
	dumphex((void*) bytes, size);
	
	printf("\n\n--> outputting %zd bytes to output file...\n\n", size);

	int out_file = open("object.o", O_WRONLY | O_CREAT);
	if (out_file < 0) { perror("open"); exit(4); }

	write(out_file, &header, sizeof header);
	write(out_file, &command, sizeof command);
	write(out_file, &section, sizeof section);
	write(out_file, bytes, size);

	close(out_file);












	if (argc != 2) return printf("usage: ./compiler <input>\n");
	const int limit = 8192, ctm_limit = 4096,
		args_limit = 64, ctr_limit = 16;
	int* output = malloc(limit * sizeof(int));
	memset(output, 0x0F, limit * sizeof(int));
	int index = 0, top = 0, begin = 0, done = 0;
	int var = 0, length = 0, where = 0, best = 0;
	struct stat file_data = {0};
	int file = open(argv[1], O_RDONLY);
	if (file < 0 or stat(argv[1], &file_data) < 0) { perror("open"); exit(3); }
	length = (int) file_data.st_size;
	char* input = not length ? 0 : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) { perror("mmap"); exit(4); }
	close(file);






fprintf(stderr, "%u %u %u %u %u %u %u %u\n", 
			line, column, at, wline, wcolumn, wat, top, limit);
	munmap(input, (size_t) length);
	free(output);


*/


















/*










target syntax:
------------------------------






	r4 r2 r5 add
	label jump                  

label at 

	r3 r5 r21 orr 
	label jump






------------------------------








*/




/*

	printf("\n\n  did you mean:   ");
	int* n = context + candidate;
	for (int j = 0; j <= n[1]; j++) {
		int c = n[j + 2];
		if (c < 33) printf(" char{%d} ", c);
		else if (c < 128) printf("%c ", c);
		else if (c < 256) printf(" unicode{%d} ", c);
		else printf(" (%d) ", c);
	}

*/







// labels[label_count++] = (struct label) {.name = word, .length = count};







/*













comment this is my assembler! comment


segment readable executable

label at
	nop
	svc
	r4 r1 r5 orr
	label jump

[end]

segment readable writable 

label at  
	"hello"

















assembly_example: ls
hello.s
assembly_example: as hello.s -o hello.o
assembly_example: otool -tvVhlL hello.o  
hello.o:
Mach header
      magic  cputype cpusubtype  caps    filetype ncmds sizeofcmds      flags
MH_MAGIC_64    ARM64        ALL  0x00      OBJECT     5        376 0x00000000
Load command 0
      cmd LC_SEGMENT_64
  cmdsize 232
  segname 
   vmaddr 0x0000000000000000
   vmsize 0x000000000000003e
  fileoff 408
 filesize 62
  maxprot rwx
 initprot rwx
   nsects 2
    flags (none)
Section
  sectname __text
   segname __TEXT
      addr 0x0000000000000000
      size 0x0000000000000030
    offset 408
     align 2^3 (8)
    reloff 472
    nreloc 1
      type S_REGULAR
attributes PURE_INSTRUCTIONS SOME_INSTRUCTIONS
 reserved1 0
 reserved2 0
Section
  sectname __data
   segname __DATA
      addr 0x0000000000000030
      size 0x000000000000000e
    offset 456
     align 2^0 (1)
    reloff 0
    nreloc 0
      type S_REGULAR
attributes (none)
 reserved1 0
 reserved2 0
Load command 1
      cmd LC_BUILD_VERSION
  cmdsize 24
 platform MACOS
    minos 13.0
      sdk n/a
   ntools 0
Load command 2
      cmd LC_DATA_IN_CODE
  cmdsize 16
  dataoff 480
 datasize 8
Load command 3
     cmd LC_SYMTAB
 cmdsize 24
  symoff 488
   nsyms 5
  stroff 568
 strsize 32
Load command 4
            cmd LC_DYSYMTAB
        cmdsize 80
      ilocalsym 0
      nlocalsym 4
     iextdefsym 4
     nextdefsym 1
      iundefsym 5
      nundefsym 0
         tocoff 0
           ntoc 0
      modtaboff 0
        nmodtab 0
   extrefsymoff 0
    nextrefsyms 0
 indirectsymoff 0
  nindirectsyms 0
      extreloff 0
        nextrel 0
      locreloff 0
        nlocrel 0
(__TEXT,__text) section
_start:

0000000000000000        mov     x0, #0x1
0000000000000004        ldr     x1, #0x1c
0000000000000008        ldr     x2, #0x20
000000000000000c        mov     w8, #0x40
0000000000000010        svc     #0
0000000000000014        mov     x0, #0x0
0000000000000018        mov     w8, #0x5d
000000000000001c        svc     #0

0000000000000020        udf     #0x0
0000000000000024        udf     #0x0
0000000000000028        udf     #0xe
000000000000002c        udf     #0x0


























ssembly_example: objdump hello.o -DSast --disassembler-options=no-aliases

hello.o:	file format mach-o arm64

SYMBOL TABLE:
0000000000000000 l     F __TEXT,__text ltmp0
0000000000000030 l     O __DATA,__data ltmp1
0000000000000030 l     O __DATA,__data msg
000000000000000e l       *ABS* len
0000000000000000 g     F __TEXT,__text _start
Contents of section __TEXT,__text:
 0000 200080d2 e1000058 02010058 08088052   ......X...X...R
 0010 010000d4 000080d2 a80b8052 010000d4  ...........R....
 0020 00000000 00000000 0e000000 00000000  ................
Contents of section __DATA,__data:
 0030 48656c6c 6f2c2041 524d3634 210a      Hello, ARM64!.

Disassembly of section __TEXT,__text:

0000000000000000 <ltmp0>:
       0: 20 00 80 d2  	mov	x0, #1
       4: e1 00 00 58  	ldr	x1, 0x20 <ltmp0+0x20>
       8: 02 01 00 58  	ldr	x2, 0x28 <ltmp0+0x28>
       c: 08 08 80 52  	mov	w8, #64
      10: 01 00 00 d4  	svc	#0
      14: 00 00 80 d2  	mov	x0, #0
      18: a8 0b 80 52  	mov	w8, #93
      1c: 01 00 00 d4  	svc	#0
		...
      28: 0e 00 00 00  	udf	#14
      2c: 00 00 00 00  	udf	#0

Disassembly of section __DATA,__data:

0000000000000030 <msg>:
      30: 48 65 6c 6c  	ldnp	d8, d25, [x10, #-320]
      34: 6f 2c 20 41  	<unknown>
      38: 52 4d 36 34  	cbz	w18, 0x6c9e0 <msg+0x6c9b0>
      3c: 21           	<unknown>
      3d: 0a           	<unknown>
assembly_example: 






	08 08 80 52    	mov	w8, #64              0x52800808 -> 

							 52 80 08 08      ->    0][101 0010    1][00][0 0000   0000 1000   000][0 1000]



	08 08 80 52  	mov	w8, #64





























*/






		// printf("looking at: \"%.*s\"\n", (int) count, word);






/*

// static const char digits[36] = "0123456789abcdefghijklmnopqrstuvwxyz";
static nat string_to_number(char* string, nat* length) {
	nat radix = 0, value = 0;
	nat result = 0, index = 0, place = 1;
begin:	if (index >= *length) goto done;
	value = 0;
top:	if (value >= 36) abort();
	if (digits[value] == string[index]) goto found;
	value++;
	goto top;
found:	if (index) goto check;
	radix = value;
	goto next;
check:	if (value >= radix) goto done;
	result += place * value;
	place *= radix;
next:	index++;
	goto begin;
done:	*length = index;
	return result;
}







































mach_header_64 header = {};
header.magic          = MH_MAGIC_64;
header.cputype        = CPU_TYPE_X86_64;
header.cpusubtype     = CPU_SUBTYPE_X86_64_ALL;
header.filetype       = MH_OBJECT;
header.ncmds          = 0; 
header.sizeofcmds     = 0; 
header.flags          = MH_SUBSECTIONS_VIA_SYMBOLS;

segment_command_64 segment = {};
segment.cmd                = LC_SEGMENT_64;
segment.cmdsize            = sizeof(segment) + sizeof(section_64);
segment.vmaddr             = 0;
segment.vmsize             = 0;
segment.fileoff            = 0;
segment.filesize           = 0;
segment.maxprot            = VM_PROT_READ | VM_PROT_EXECUTE;
segment.initprot           = VM_PROT_READ | VM_PROT_EXECUTE;
segment.nsects             = 0; 

section_64 sectionText     = {};
strcpy(sectionText.segname,  SEG_TEXT ); // segname  <- __TEXT 
strcpy(sectionText.sectname, SECT_TEXT); // sectname <- __text 
sectionText.addr           = 0;
sectionText.size           = 0;    
sectionText.offset         = 0;    
sectionText.align          = 4;          // 2^4 code alignment 
sectionText.reloff         = 0;         
sectionText.nreloc         = 0;         
sectionText.flags          = S_REGULAR |
                             S_ATTR_PURE_INSTRUCTIONS |
                             S_ATTR_SOME_INSTRUCTIONS;

const unsigned char code[] = {
        0xE8, 0x00, 0x00, 0x00, 0x00,      // call <address> - someFuncExternal
        0xE8, 0x00, 0x00, 0x00, 0x00,      // call <address> - someFunc
        0xB8, 0x01, 0x00, 0x00, 0x02,      // mov     rax, 0x2000001 ; exit
        0xBF, 0x00, 0x00, 0x00, 0x00,      // mov     rdi, 0
        0x0F, 0x05,                        // syscall
        // someFunc:
        0x48, 0x31, 0xC0,                  // xor rax, rax
        0xC3                               // ret
};

symtab_command symtabCommand    = {};
symtabCommand.cmd               = LC_SYMTAB;
symtabCommand.cmdsize           = sizeof(symtab_command);
symtabCommand.symoff            = 0;   
symtabCommand.nsyms             = 0;   
symtabCommand.stroff            = 0;   
symtabCommand.strsize           = 0;   

const char stringTable[]        = "\0_someFunc0\0_someFuncExternal0\0";

nlist_64 symbols[2] = {
        {
            1,                      // first index in string table
            N_SECT | N_EXT,         // defined in the file, available externally
            1,                      // first section
            REFERENCE_FLAG_DEFINED, // defined in the file
            4 * 5 + 2               // offset of this symbol in the section
        },
        {
            12,                      // second string in string table
            N_UNDF  | N_EXT,         // undefined in the file,
                                     // must be defined externally
            NO_SECT,                 // no section specified
            REFERENCE_FLAG_UNDEFINED_NON_LAZY, // external non-lazy symbol
            0                        // unused
        }
};

dysymtab_command dysymtabCommand      = {};
dysymtabCommand.cmd                   = LC_DYSYMTAB;
dysymtabCommand.cmdsize               = sizeof(dysymtabCommand);
dysymtabCommand.ilocalsym             = 0; // first symbol in symbol table
dysymtabCommand.nlocalsym             = 1; // only one locally defined symbol
dysymtabCommand.iextdefsym            = 1; // second symbol in symbol table
dysymtabCommand.nextdefsym            = 1; // only one externally defined symbol

relocation_info relocations[] = {
        {
            1,      // after first byte address to someFuncExternal
            1,      // second symbol
            1,      // relative call, PC counted
            2,      // 4 bytes
            1,      // external
            GENERIC_RELOC_SECTDIFF
        },
        {
            6,      // second call address
            0,      // first symbol
            1,      // relative call, PC counted
            2,      // 4 bytes
            1,      // external
            GENERIC_RELOC_SECTDIFF
        },
};

size_t offsetCounter = 0;
FILE* binary = fopen("object.o", "wb");

// Write header;
header.ncmds = 3; // segment + symtab + dysymtab
header.sizeofcmds = sizeof(segment) + sizeof(sectionText) + sizeof(symtabCommand) + sizeof(dysymtabCommand);
fwrite(&header, 1, sizeof(header), binary);
offsetCounter += sizeof(header);

// Write segment
segment.vmsize  = segment.filesize = sizeof(code);
segment.fileoff = header.sizeofcmds + sizeof(header); // we'll place code just after all load commands.
segment.nsects  = 1;
fwrite(&segment, 1, sizeof(segment), binary);
offsetCounter += sizeof(segment);

// Write section
sectionText.size   = segment.filesize;
sectionText.offset = segment.fileoff;
sectionText.reloff = segment.fileoff + segment.filesize; // just after the code
sectionText.nreloc = sizeof(relocations) / sizeof(relocations[0]); // two calls
fwrite(&sectionText, 1, sizeof(sectionText), binary);
offsetCounter += sizeof(sectionText);

// Write symtab
symtabCommand.symoff = sectionText.reloff +
                        sectionText.nreloc * sizeof(relocation_info); // just after relocations
symtabCommand.nsyms = 2; // two functions
symtabCommand.stroff = symtabCommand.symoff +
                        symtabCommand.nsyms * sizeof(nlist_64); // just after symbol table
symtabCommand.strsize = sizeof(stringTable);
fwrite(&symtabCommand, 1, sizeof(symtabCommand), binary);
offsetCounter += sizeof(symtabCommand);

// Write dysymtab
fwrite(&dysymtabCommand, 1, sizeof(dysymtabCommand), binary);
offsetCounter += sizeof(dysymtabCommand);

// Write code
fwrite(&code, 1, sizeof(code), binary);

// Write relocations
fwrite(&relocations, 1, sizeof(relocations), binary);

// Write symbol table
fwrite(&symbols, 1, sizeof(symbols), binary);

// Write string table
fwrite(&stringTable, 1, sizeof(stringTable), binary);

fclose(binary);









struct symtab_command {
   uint32_t   cmd;      // * LC_SYMTAB 
   uint32_t   cmdsize;  // * sizeof(struct symtab_command) *
   uint32_t   symoff;   // * symbol table offset *
   uint32_t   nsyms;    // * number of symbol table entries *
   uint32_t   stroff;   // * string table offset *
   uint32_t   strsize;  //  string table size in bytes *
};


























cprogram: otool -rIlDhtvVdC a.out 
a.out:
Mach header
      magic  cputype cpusubtype  caps    filetype ncmds sizeofcmds      flags
MH_MAGIC_64    ARM64        ALL  0x00     EXECUTE    16        744   NOUNDEFS DYLDLINK TWOLEVEL PIE
Load command 0
      cmd LC_SEGMENT_64
  cmdsize 72
  segname __PAGEZERO
   vmaddr 0x0000000000000000
   vmsize 0x0000000100000000
  fileoff 0
 filesize 0
  maxprot ---
 initprot ---
   nsects 0
    flags (none)
Load command 1
      cmd LC_SEGMENT_64
  cmdsize 232
  segname __TEXT
   vmaddr 0x0000000100000000
   vmsize 0x0000000000004000
  fileoff 0
 filesize 16384
  maxprot r-x
 initprot r-x
   nsects 2
    flags (none)
Section
  sectname __text
   segname __TEXT
      addr 0x0000000100003fb0
      size 0x0000000000000008
    offset 16304
     align 2^2 (4)
    reloff 0
    nreloc 0
      type S_REGULAR
attributes PURE_INSTRUCTIONS SOME_INSTRUCTIONS
 reserved1 0
 reserved2 0
Section
  sectname __unwind_info
   segname __TEXT
      addr 0x0000000100003fb8
      size 0x0000000000000048
    offset 16312
     align 2^2 (4)
    reloff 0
    nreloc 0
      type S_REGULAR
attributes (none)
 reserved1 0
 reserved2 0
Load command 2
      cmd LC_SEGMENT_64
  cmdsize 72
  segname __LINKEDIT
   vmaddr 0x0000000100004000
   vmsize 0x0000000000004000
  fileoff 16384
 filesize 450
  maxprot r--
 initprot r--
   nsects 0
    flags (none)
Load command 3
      cmd LC_DYLD_CHAINED_FIXUPS
  cmdsize 16
  dataoff 16384
 datasize 56
Load command 4
      cmd LC_DYLD_EXPORTS_TRIE
  cmdsize 16
  dataoff 16440
 datasize 48
Load command 5
     cmd LC_SYMTAB
 cmdsize 24
  symoff 16496
   nsyms 2
  stroff 16528
 strsize 32
Load command 6
            cmd LC_DYSYMTAB
        cmdsize 80
      ilocalsym 0
      nlocalsym 0
     iextdefsym 0
     nextdefsym 2
      iundefsym 2
      nundefsym 0
         tocoff 0
           ntoc 0
      modtaboff 0
        nmodtab 0
   extrefsymoff 0
    nextrefsyms 0
 indirectsymoff 0
  nindirectsyms 0
      extreloff 0
        nextrel 0
      locreloff 0
        nlocrel 0
Load command 7
          cmd LC_LOAD_DYLINKER
      cmdsize 32
         name /usr/lib/dyld (offset 12)
Load command 8
     cmd LC_UUID
 cmdsize 24
    uuid 58E064A7-8D07-369A-838F-6508F27989CD
Load command 9
      cmd LC_BUILD_VERSION
  cmdsize 32
 platform MACOS
    minos 13.0
      sdk 13.3
   ntools 1
     tool LD
  version 857.1
Load command 10
      cmd LC_SOURCE_VERSION
  cmdsize 16
  version 0.0
Load command 11
       cmd LC_MAIN
   cmdsize 24
  entryoff 16304
 stacksize 0
Load command 12
          cmd LC_LOAD_DYLIB
      cmdsize 56
         name /usr/lib/libSystem.B.dylib (offset 24)
   time stamp 2 Wed Dec 31 16:00:02 1969
      current version 1319.100.3
compatibility version 1.0.0
Load command 13
      cmd LC_FUNCTION_STARTS
  cmdsize 16
  dataoff 16488
 datasize 8
Load command 14
      cmd LC_DATA_IN_CODE
  cmdsize 16
  dataoff 16496
 datasize 0
Load command 15
      cmd LC_CODE_SIGNATURE
  cmdsize 16
  dataoff 16560
 datasize 274
Linker optimiztion hints (0 total bytes)
(__TEXT,__text) section
_main:
0000000100003fb0	mov	w0, #0x0
0000000100003fb4	ret





otool -rIlDhtvVdC a.out
objdump -D a.out














	r0 r0 r0 movzx
	r1 r0 r8 movzw
	svc

	r0 r0 r0 movkx
	r1 r0 r8 movkw
	svc


	r0 r0 r0 movnx
	r1 r0 r8 movnw
	svc



	r5 r2 r1 addix
	r5 r2 r1 addiw
	r5 r2 r1 addhx
	r5 r2 r1 addhw


	nop
	nop
	nop
	nop
	nop
	nop


































assembler: otool -txvVhlL executable.out
executable.out:
Mach header
      magic  cputype cpusubtype  caps    filetype ncmds sizeofcmds      flags
MH_MAGIC_64    ARM64        ALL  0x00     EXECUTE    16        744   NOUNDEFS DYLDLINK TWOLEVEL PIE
Load command 0
      cmd LC_SEGMENT_64
  cmdsize 72
  segname __PAGEZERO
   vmaddr 0x0000000000000000
   vmsize 0x0000000100000000
  fileoff 0
 filesize 0
  maxprot ---
 initprot ---
   nsects 0
    flags (none)
Load command 1
      cmd LC_SEGMENT_64
  cmdsize 232
  segname __TEXT
   vmaddr 0x0000000100000000
   vmsize 0x0000000000004000
  fileoff 0
 filesize 16384
  maxprot r-x
 initprot r-x
   nsects 2
    flags (none)
Section
  sectname __text
   segname __TEXT
      addr 0x0000000100003fa8
      size 0x0000000000000010
    offset 16296
     align 2^3 (8)
    reloff 0
    nreloc 0
      type S_REGULAR
attributes PURE_INSTRUCTIONS SOME_INSTRUCTIONS
 reserved1 0
 reserved2 0
Section
  sectname __unwind_info
   segname __TEXT
      addr 0x0000000100003fb8
      size 0x0000000000000048
    offset 16312
     align 2^2 (4)
    reloff 0
    nreloc 0
      type S_REGULAR
attributes (none)
 reserved1 0
 reserved2 0
Load command 2
      cmd LC_SEGMENT_64
  cmdsize 72
  segname __LINKEDIT
   vmaddr 0x0000000100004000
   vmsize 0x0000000000004000
  fileoff 16384
 filesize 459
  maxprot r--
 initprot r--
   nsects 0
    flags (none)
Load command 3
      cmd LC_DYLD_CHAINED_FIXUPS
  cmdsize 16
  dataoff 16384
 datasize 56
Load command 4
      cmd LC_DYLD_EXPORTS_TRIE
  cmdsize 16
  dataoff 16440
 datasize 48
Load command 5
     cmd LC_SYMTAB
 cmdsize 24
  symoff 16496
   nsyms 2
  stroff 16528
 strsize 32
Load command 6
            cmd LC_DYSYMTAB
        cmdsize 80
      ilocalsym 0
      nlocalsym 0
     iextdefsym 0
     nextdefsym 2
      iundefsym 2
      nundefsym 0
         tocoff 0
           ntoc 0
      modtaboff 0
        nmodtab 0
   extrefsymoff 0
    nextrefsyms 0
 indirectsymoff 0
  nindirectsyms 0
      extreloff 0
        nextrel 0
      locreloff 0
        nlocrel 0
Load command 7
          cmd LC_LOAD_DYLINKER
      cmdsize 32
         name /usr/lib/dyld (offset 12)
Load command 8
     cmd LC_UUID
 cmdsize 24
    uuid 8B2C0B21-254F-38C9-AEE1-161F29F2E419
Load command 9
      cmd LC_BUILD_VERSION
  cmdsize 32
 platform MACOS
    minos 13.0
      sdk 13.3
   ntools 1
     tool LD
  version 857.1
Load command 10
      cmd LC_SOURCE_VERSION
  cmdsize 16
  version 0.0
Load command 11
       cmd LC_MAIN
   cmdsize 24
  entryoff 16296
 stacksize 0
Load command 12
          cmd LC_LOAD_DYLIB
      cmdsize 56
         name /usr/lib/libSystem.B.dylib (offset 24)
   time stamp 2 Wed Dec 31 16:00:02 1969
      current version 1319.100.3
compatibility version 1.0.0
Load command 13
      cmd LC_FUNCTION_STARTS
  cmdsize 16
  dataoff 16488
 datasize 8
Load command 14
      cmd LC_DATA_IN_CODE
  cmdsize 16
  dataoff 16496
 datasize 0
Load command 15
      cmd LC_CODE_SIGNATURE
  cmdsize 16
  dataoff 16560
 datasize 283
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1319.100.3)
	time stamp 2 Wed Dec 31 16:00:02 1969
(__TEXT,__text) section
_main:
0000000100003fa8	add	x27, x27, x27
0000000100003fac	mov	x0, #0xff
0000000100003fb0	mov	w16, #0x1
0000000100003fb4	svc	#0
assembler: 





	useful for inspecing the object file: 
		otool -txvVhlL program.o
		objdump program.o -DSast --disassembler-options=no-aliases
*/

