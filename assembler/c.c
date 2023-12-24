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
#include <stdnoreturn.h>
#include <mach-o/nlist.h>
#include <mach-o/loader.h>
typedef uint64_t nat;
typedef uint32_t u32;

static const bool debug = false;

enum instruction_type {
	nop, dw, svc, cfinv, br, blr, b_, bc, adr, adrp, 

	movzx, movzw,	movkx, movkw,	movnx, movnw,
	addix, addiw,	addhx, addhw,
	addixs, addiws,	addhxs, addhws,
	subix,  subiw,  subhx,  subhw,
	subixs, subiws,	subhxs, subhws,
	maddx, maddw,

	striux, striuw,  ldriux,  ldriuw,  striox, 
	striow, striex,  striew,  ldriox,  ldriow, 
	ldriex, ldriew,  ldurx,   ldtrx,   ldurw, 
	ldtrw,  ldtrsw,  ldtrh,   ldtrshx, ldtrshw, 
	ldtrb,  ldtrsbx, ldtrsbw, lslvx,   lslvw, 
	udivx,  udivw,   umaxx,   umaxw,   uminx, 
	uminw,  umaddlx, umaddlw, msubx,   msubw, 

	adcx, adcw, 	adcxs, adcws, 
	asrvx, asrvw, 	
	cselx, cselw, 	csincx, csincw, 
	csinvx, csinvw, csnegx, csnegw, 
	orrx, orrw,	ornx, ornw,
	addx, addw, 	addxs, addws,
	subx, subw, 	subxs, subws,
	ld64b, 	st64b,	absx, absw, 
	clsx, 	clsw,	clzx, clzw,	ctzx, ctzw,	cntx, cntw,    
	rbitx, 	rbitw,	revx, revw,  	revhx, revhw,

	ctnop, ctzero, ctincr, ctset, ctimm, 
	ctldi, ctlda, ctsta, ctdel,
	ctadd, ctsub, ctmul, ctdiv, ctrem,
	ctnor, ctxor, ctor, ctand, ctshl, ctshr, ctprint, 
	ctld1, ctld2, ctld4, ctld8, ctst1, ctst2, ctst4, ctst8,
	ctpc, ctblt, ctbge, ctbeq, ctbne, ctbr, ctgoto, ctat, ctstop,
	ctput, ctget, ctabort,

	instruction_set_count
};

static const char* const instruction_spelling[instruction_set_count] = {
	"nop", "dw", "svc", "cfinv", "br", "blr", "goto", "bc", "adr", "adrp", 

	"movzx",  "movzw",   "movkx", "movkw", "movnx", "movnw",
	"addix",  "addiw",   "addhx", "addhw",
	"addixs", "addiws", "addhxs", "addhws",
	"subix",  "subiw",   "subhx", "subhw",
	"subixs", "subiws", "subhxs", "subhws",
	"maddx",  "maddw",

	"striux", "striuw",  "ldriux",  "ldriuw",  "striox", 
	"striow", "striex",  "striew",  "ldriox",  "ldriow", 
	"ldriex", "ldriew",  "ldurx",   "ldtrx",   "ldurw", 
	"ldtrw",  "ldtrsw",  "ldtrh",   "ldtrshx", "ldtrshw", 
	"ldtrb",  "ldtrsbx", "ldtrsbw", "lslvx",   "lslvw", 
	"udivx",  "udivw",   "umaxx",   "umaxw",   "uminx", 
	"uminw",  "umaddlx", "umaddlw", "msubx",   "msubw", 

	"adcx",  "adcw", "adcxs", "adcws", 
	"asrvx", "asrvw",
	"cselx",  "cselw",  "csincx", "csincw", 
	"csinvx", "csinvw", "csnegx", "csnegw",
	"orrx", "orrw",	"ornx", "ornw", 
	"addx", "addw", "addxs", "addws",
	"subx", "subw", "subxs", "subws",
	"ld64b", "st64b", "absx", "absw",
	"clsx",  "clsw",  "clzx", "clzw", "ctzx", "ctzw",  "cntx", "cntw",
	"rbitx", "rbitw", "revx", "revw", "revhx", "revhw",

	"ctnop", "ctzero", "ctincr", "ctset", "ctimm", 
	"ctldi", "ctlda", "ctsta", "ctdel",
	"ctadd", "ctsub", "ctmul", "ctdiv", "ctrem",
	"ctnor", "ctxor", "ctor", "ctand", "ctshl", "ctshr", "ctprint",
	"ctld1", "ctld2", "ctld4", "ctld8", "ctst1", "ctst2", "ctst4", "ctst8",
	"ctpc", "ctblt", "ctbge", "ctbeq", "ctbne", "ctbr", "ctgoto", "ctat", "ctstop",
	"ctput", "ctget", "ctabort"
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
	nat immediate;
	struct argument arguments[6];
};

static const char* filename = NULL;
static nat text_length = 0;
static char* text = NULL;

static nat ins_count = 0;
static struct instruction ins[4096] = {0};

static nat byte_count = 0;
static uint8_t* bytes = NULL;

static nat arg_count = 0;
static struct argument arguments[4096] = {0};
static nat registers[4096] = {0};

static nat immediate = 0, stop = 0; 

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

static void emit(u32 x) {
	bytes = realloc(bytes, byte_count + 4);
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

static void check_branch(int r, int c, const struct argument a, const char* type) {
	if (r > -c or r < c) return;
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "invalid %s argument %d (%d <= %d or %d >= %d)", type, r, r, c, r, c);
	print_error(reason, a.start, a.count); 
	exit(1);
}

static u32 generate_mov(struct argument* a, u32 sf, u32 op, u32 im) {
	const u32 Rd = (u32) a[0].value;
	const u32 hw = (u32) a[1].value;
	check(Rd, 32, a[0], "register");
	check(hw, 4,  a[1], "register");
	check(im, 1 << 16U, a[2], "immediate");
	return  (sf << 31U) | 
		(op << 23U) | 
		(hw << 21U) | 
		(im <<  5U) | Rd;
}

static u32 generate_addi(struct argument* a, u32 sf, u32 sh, u32 op, u32 im) {
	const u32 Rd = (u32) a[0].value;
	const u32 Rn = (u32) a[1].value;
	check(Rd, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	check(im, 1 << 12U, a[2], "immediate");
	return  (sf << 31U) | 
		(op << 23U) | 
		(sh << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_stri(struct argument* a, u32 si, u32 op, u32 o2, u32 im) {
	const u32 Rt = (u32) a[0].value;
	const u32 Rn = (u32) a[1].value;
	check(Rt, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	check(im, 1 << 9U, a[2], "immediate");
	return  (si << 30U) | 
		(op << 21U) | 
		(im << 12U) | 
		(o2 << 10U) |
		(Rn <<  5U) | Rt;
}

static u32 generate_striu(struct argument* a, u32 si, u32 op, u32 im) {
	const u32 Rt = (u32) a[0].value;
	const u32 Rn = (u32) a[1].value;
	check(Rt, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	check(im, 1 << 12U, a[2], "immediate");
	return  (si << 30U) | 
		(op << 21U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rt;
}

static u32 generate_adc(struct argument* a, u32 sf, u32 op, u32 o2) {  
	const u32 Rd = (u32) a[0].value;
	const u32 Rn = (u32) a[1].value;
	const u32 Rm = (u32) a[2].value;
	check(Rd, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	check(Rm, 32, a[2], "register");
	return  (sf << 31U) | 
		(op << 21U) | 
		(Rm << 16U) | 
		(o2 << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_madd(struct argument* a, u32 sf, u32 op, u32 o2) {  
	const u32 Rd = (u32) a[0].value;
	const u32 Rn = (u32) a[1].value;
	const u32 Rm = (u32) a[2].value;
	const u32 Ra = (u32) a[3].value;
	check(Rd, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	check(Rm, 32, a[2], "register");
	check(Ra, 32, a[3], "register-z");
	return  (sf << 31U) | 
		(op << 21U) | 
		(Rm << 16U) | 
		(o2 << 15U) |
		(Ra << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_csel(struct argument* a, u32 sf, u32 op, u32 o2) {  
	const u32 cd = (u32) a[0].value;
	const u32 Rd = (u32) a[1].value;
	const u32 Rn = (u32) a[2].value;
	const u32 Rm = (u32) a[3].value;
	check(cd, 16, a[0], "condition");
	check(Rd, 32, a[1], "register");
	check(Rn, 32, a[2], "register");
	check(Rm, 32, a[3], "register");
	return  (sf << 31U) | 
		(op << 21U) | 
		(Rm << 16U) | 
		(cd << 12U) | 
		(o2 << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_orr(struct argument* a, u32 sf, u32 ne, u32 op, u32 im) { 
	const u32 Rd = (u32) a[0].value;
	const u32 Rn = (u32) a[1].value;
	const u32 Rm = (u32) a[2].value;
	const u32 sh = (u32) a[3].value;
	check(Rd, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	check(Rm, 32, a[2], "register");
	check(sh, 4,  a[3], "register");
	check(im, 32U << sf, a[4], "immediate");
	return  (sf << 31U) |
		(op << 24U) | 
		(sh << 22U) | 
		(ne << 21U) | 
		(Rm << 16U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_abs(struct argument* a, u32 sf, u32 op) {  
	const u32 Rd = (u32) a[0].value;
	const u32 Rn = (u32) a[1].value;
	check(Rd, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	return  (sf << 31U) | 
		(op << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_br(struct argument* a, u32 op) { 
	const u32 Rn = (u32) a[0].value;
	check(Rn, 32, a[0], "register");
	return  (op << 10U) | (Rn << 5U);
}

static u32 generate_b(struct argument* a, nat im) {
	const u32 Im = * (u32*) im;
	check_branch((int) Im, 1 << (26 - 1), a[0], "branch offset");
	return (0x05 << 26U) | (0x03FFFFFFU & im);
}

static u32 generate_bc(struct argument* a, nat im) { 
	const u32 cd = (u32) a[0].value;
	const u32 Im = * (u32*) im;
	check_branch((int) Im, 1 << (19 - 1), a[1], "branch offset");
	check(cd, 16, a[0], "condition");
	return (0x54U << 24U) | ((0x0007FFFFU & Im) << 5U) | cd;
}

static u32 generate_adr(struct argument* a, u32 op, u32 o2, nat im) {
	const u32 Rd = (u32) a[0].value;
	const u32 Im = * (u32*) im;
	check(Rd, 32, a[0], "register");
	check_branch((int) Im, 1 << (21 - 1), a[1], "pc-relative address");
	const u32 lo = 0x03U & Im, hi = 0x07FFFFU & (Im >> 2);
	return  (o2 << 31U) | 
		(lo << 29U) | 
		(op << 24U) | 
		(hi <<  5U) | Rd;
}

static void push(nat op, nat start, nat count) {
	if (stop) return;
	struct instruction new = {
		.op = op,
		.immediate = immediate,
		.arguments = {0}, 
		.start = start,
		.count = count,
	};
	for (nat i = 0; i < 6; i++) {
		if (arg_count == i) break;
		new.arguments[i] = arguments[arg_count - 1 - i];
	}
	ins[ins_count++] = new;
}

static void execute(nat op, nat* pc) {
	const nat a2 = arg_count >= 3 ? arguments[arg_count - 3].value : 0;
	const nat a1 = arg_count >= 2 ? arguments[arg_count - 2].value : 0;
	const nat a0 = arg_count >= 1 ? arguments[arg_count - 1].value : 0;

	if (op == ctstop) {
		if (debug) printf("info: found stop instruction, currently in stop mode %llu, "
				"looking for stop mode %llu  ...  ", stop, registers[a0]);
		if (registers[a0] == stop) { if (debug) printf("SUCCESS\n"); stop = 0; } 
		else { if (debug) printf("[no-match]\n"); }
		if (debug) printf("[stop = %llu]\n", stop);
		return; 
	} else if (stop) {
		if (debug) printf("info: skipping over %llu (\"%s\"), in stop mode %llu\n", 
				op, instruction_spelling[op], stop);
		return;
	}
	if (debug) printf("@%llu: info: executing \033[1;32m%s\033[0m(%llu) "
			" %lld %lld %lld\n", *pc, instruction_spelling[op], op, a0, a1, a2);
	if (debug) getchar();

	if (op == ctnop) {}
	else if (op == ctdel)  { if (arg_count) arg_count--; }
	else if (op == ctimm)  immediate = registers[a0];
	else if (op == ctat)   *(u32*)registers[a0] = (u32) ins_count;
	else if (op == ctpc)   registers[a0] = (u32) *pc;
	else if (op == ctgoto) *pc  = registers[a0];
	else if (op == ctbr)   stop = registers[a0];
	else if (op == ctblt)  { if (registers[a1]  < registers[a2]) stop = registers[a0]; } 
	else if (op == ctbge)  { if (registers[a1] >= registers[a2]) stop = registers[a0]; } 
	else if (op == ctbeq)  { if (registers[a1] == registers[a2]) stop = registers[a0]; } 
	else if (op == ctbne)  { if (registers[a1] != registers[a2]) stop = registers[a0]; }
	else if (op == ctincr) registers[a0]++;
	else if (op == ctzero) registers[a0] = 0;
	else if (op == ctldi)  registers[a0] = a1;
	else if (op == ctset)  registers[a0] = registers[a1];
	else if (op == ctadd)  registers[a0] = registers[a1] + registers[a2]; 
	else if (op == ctsub)  registers[a0] = registers[a1] - registers[a2]; 
	else if (op == ctmul)  registers[a0] = registers[a1] * registers[a2]; 
	else if (op == ctdiv)  registers[a0] = registers[a1] / registers[a2]; 
	else if (op == ctrem)  registers[a0] = registers[a1] % registers[a2]; 
	else if (op == ctxor)  registers[a0] = registers[a1] ^ registers[a2]; 
	else if (op == ctor)   registers[a0] = registers[a1] | registers[a2]; 
	else if (op == ctand)  registers[a0] = registers[a1] & registers[a2]; 
	else if (op == ctnor)  registers[a0] = ~(registers[a1] | registers[a2]); 
	else if (op == ctshl)  registers[a0] = registers[a1] << registers[a2]; 
	else if (op == ctshr)  registers[a0] = registers[a1] >> registers[a2]; 
	else if (op == ctld1)  registers[a0] = *(uint8_t*) registers[a1]; 
	else if (op == ctld2)  registers[a0] = *(uint16_t*)registers[a1]; 
	else if (op == ctld4)  registers[a0] = *(uint32_t*)registers[a1]; 
	else if (op == ctld8)  registers[a0] = *(uint64_t*)registers[a1]; 
	else if (op == ctst1)  *(uint8_t*) registers[a0] = (uint8_t)  registers[a1]; 
	else if (op == ctst2)  *(uint16_t*)registers[a0] = (uint16_t) registers[a1]; 
	else if (op == ctst4)  *(uint32_t*)registers[a0] = (uint32_t) registers[a1]; 
	else if (op == ctst8)  *(uint64_t*)registers[a0] = (uint64_t) registers[a1]; 
	else if (op == ctsta)  *(nat*)registers[a0] = a1;
	else if (op == ctlda)  arguments[arg_count++].value = *(nat*)registers[a0]; 
	else if (op == ctput)  putchar((char) registers[a0]);
	else if (op == ctget)  registers[a0] = (nat) getchar();
	else if (op == ctprint) printf("debug: \033[32m%llu\033[0m \033[32m0x%llx\033[0m\n", registers[a0], registers[a0]); 
	else if (op == ctabort) abort();
}

static void print_registers(void) {
	for (nat i = 0; i < sizeof registers / sizeof(nat); i++) {
		if (not (i % 4)) puts("");
		printf("%02llu:%010llx, ", i, registers[i]);
	}
	puts("");
}

static void print_arguments(void) {
	printf("arguments: { ");
	for (nat i = 0; i < arg_count; i++) {
		printf("{%llu,(%llu,%llu)} ", 
			arguments[i].value,
			arguments[i].start,
			arguments[i].count
		);
	}
	puts("} ");
}

static void print_instructions(void) {
	printf("instructions: {\n");
	for (nat i = 0; i < ins_count; i++) {
		const struct instruction I = ins[i];
		printf("\t%llu\tins(.op=%llu (\"%s\"),(.start=%llu,.count=%llu),"
			"imm=%llu, args:{", 
			i, I.op, instruction_spelling[I.op], 
			I.start, I.count, I.immediate
		);
		for (nat a = 0; a < 6; a++) 
			printf("{%llu,(%llu,%llu)}, ",  
				I.arguments[a].value, 
				I.arguments[a].start, 
				I.arguments[a].count
			);
		puts("}");
	}
	puts("}");
}

static void parse(void) {
	if (debug) printf("info: parsing file: %s...\n", filename);
	nat count = 0, start = 0, index = 0, end = text_length;
	for (; index < end; index++) {
		if (not isspace(text[index])) { 
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;
	process:;
		const char* const word = text + start;
		struct argument arg = { .value = 0, .start = start, .count = count };

		if (debug) printf("%s: processing: \"\033[32m%.*s\033[0m\"...\n", filename, (int) count, word);

		if (is(word, count, "eof")) return;
		if (is(word, count, "debugregisters")) { print_registers(); goto next; }
		if (is(word, count, "debugarguments")) { print_arguments(); goto next; }
		if (is(word, count, "debuginstructions")) { print_instructions(); goto next; }
		
		for (nat i = 0; i < sizeof registers / sizeof(nat); i++) {
			char r[10] = {0};
			snprintf(r, sizeof r, "%llx", i);
			if (is(word, count, r)) {
				arg.value = i;
				arguments[arg_count++] = arg; 
				goto next;
			}
		}

		for (nat i = nop; i < instruction_set_count; i++) {
			if (not is(word, count, instruction_spelling[i])) continue;
			if (i >= ctnop) execute(i, &index); else push(i, start, count);
			goto next;
		}
		if (stop) goto next;
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "unknown word \"%.*s\"", (int) count, word);
		print_error(reason, start, count);
		exit(1);
		next: count = 0;
	}
	if (count) goto process;
}

static void make_object_file(const char* object_filename) {

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

	const int flags = O_WRONLY | O_CREAT | O_TRUNC | O_EXCL;
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

static void debug_output(void) { 
	system("otool -txvVhlL object.o");
	system("otool -txvVhlL program.out");
	system("objdump object.o -DSast --disassembler-options=no-aliases");
	system("objdump program.out -DSast --disassembler-options=no-aliases");	
}

static void generate_machine_code(const char* object_filename, const char* executable_filename) {

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		const u32 im = (u32) ins[i].immediate;
		struct argument* const a = ins[i].arguments;

		     if (op == dw)     emit((u32) im);
		else if (op == svc)    emit(0xD4000001);
		else if (op == nop)    emit(0xD503201F);
		else if (op == cfinv)  emit(0xD500401F);
		else if (op == br)     emit(generate_br(a, 0x3587C0U));
		else if (op == blr)    emit(generate_br(a, 0x358FC0U));
		else if (op == b_)     emit(generate_b(a, ins[i].immediate));
		else if (op == bc)     emit(generate_bc(a, ins[i].immediate));
		else if (op == adr)    emit(generate_adr(a, 0x10U, 0, ins[i].immediate));
		else if (op == adrp)   emit(generate_adr(a, 0x10U, 1, ins[i].immediate));

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

		else if (op == movzx)  emit(generate_mov(a, 1, 0xA5U, im));
		else if (op == movzw)  emit(generate_mov(a, 0, 0xA5U, im));
		else if (op == movkx)  emit(generate_mov(a, 1, 0xE5U, im));
		else if (op == movkw)  emit(generate_mov(a, 0, 0xE5U, im));
		else if (op == movnx)  emit(generate_mov(a, 1, 0x25U, im));
		else if (op == movnw)  emit(generate_mov(a, 0, 0x25U, im));

		else if (op == addix)  emit(generate_addi(a, 1, 0, 0x22U, im));
		else if (op == addiw)  emit(generate_addi(a, 0, 0, 0x22U, im));
		else if (op == addhx)  emit(generate_addi(a, 1, 1, 0x22U, im));
		else if (op == addhw)  emit(generate_addi(a, 0, 1, 0x22U, im));
		else if (op == subix)  emit(generate_addi(a, 1, 0, 0xA2U, im));
		else if (op == subiw)  emit(generate_addi(a, 0, 0, 0xA2U, im));
		else if (op == subhx)  emit(generate_addi(a, 1, 1, 0xA2U, im));
		else if (op == subhw)  emit(generate_addi(a, 0, 1, 0xA2U, im));
		else if (op == addixs) emit(generate_addi(a, 1, 0, 0x62U, im));
		else if (op == addiws) emit(generate_addi(a, 0, 0, 0x62U, im));
		else if (op == addhxs) emit(generate_addi(a, 1, 1, 0x62U, im));
		else if (op == addhws) emit(generate_addi(a, 0, 1, 0x62U, im));
		else if (op == subixs) emit(generate_addi(a, 1, 0, 0xE2U, im));
		else if (op == subiws) emit(generate_addi(a, 0, 0, 0xE2U, im));
		else if (op == subhxs) emit(generate_addi(a, 1, 1, 0xE2U, im));
		else if (op == subhws) emit(generate_addi(a, 0, 1, 0xE2U, im));

		else if (op == striux)  emit(generate_striu(a, 3, 0xE4U, im));
		else if (op == striuw)  emit(generate_striu(a, 2, 0xE4U, im));
		else if (op == ldriux)  emit(generate_striu(a, 3, 0x1C2U, im));
		else if (op == ldriuw)  emit(generate_striu(a, 2, 0x1C2U, im));

		else if (op == striox)  emit(generate_stri(a, 3, 0x01C0U, 0x1U, im));
		else if (op == striow)  emit(generate_stri(a, 2, 0x01C0U, 0x1U, im));
		else if (op == striex)  emit(generate_stri(a, 3, 0x01C0U, 0x3U, im));
		else if (op == striew)  emit(generate_stri(a, 2, 0x01C0U, 0x3U, im));
		else if (op == ldriox)  emit(generate_stri(a, 3, 0x01C2U, 0x1U, im));
		else if (op == ldriow)  emit(generate_stri(a, 2, 0x01C2U, 0x1U, im));
		else if (op == ldriex)  emit(generate_stri(a, 3, 0x01C2U, 0x3U, im));
		else if (op == ldriew)  emit(generate_stri(a, 2, 0x01C2U, 0x3U, im));
		else if (op == ldurx)   emit(generate_stri(a, 3, 0x01C2U, 0x0U, im));
		else if (op == ldtrx)   emit(generate_stri(a, 3, 0x01C2U, 0x2U, im));
		else if (op == ldurw)   emit(generate_stri(a, 2, 0x01C2U, 0x0U, im));
		else if (op == ldtrw)   emit(generate_stri(a, 2, 0x01C2U, 0x2U, im));
		else if (op == ldtrsw)  emit(generate_stri(a, 2, 0x01C4U, 0x2U, im));
		else if (op == ldtrh)   emit(generate_stri(a, 1, 0x01C2U, 0x2U, im));
		else if (op == ldtrshx) emit(generate_stri(a, 1, 0x01C4U, 0x2U, im));
		else if (op == ldtrshw) emit(generate_stri(a, 1, 0x01C6U, 0x2U, im));
		else if (op == ldtrb)   emit(generate_stri(a, 0, 0x01C2U, 0x2U, im));
		else if (op == ldtrsbx) emit(generate_stri(a, 0, 0x01C4U, 0x2U, im));
		else if (op == ldtrsbw) emit(generate_stri(a, 0, 0x01C6U, 0x2U, im));
		
		else if (op == adcx)   emit(generate_adc(a, 1, 0x0D0U, 0x00));
		else if (op == adcw)   emit(generate_adc(a, 0, 0x0D0U, 0x00));
		else if (op == adcxs)  emit(generate_adc(a, 1, 0x1D0U, 0x00));
		else if (op == adcws)  emit(generate_adc(a, 0, 0x1D0U, 0x00));
		else if (op == asrvx)  emit(generate_adc(a, 1, 0x0D6U, 0x0A));
		else if (op == asrvw)  emit(generate_adc(a, 0, 0x0D6U, 0x0A));
		else if (op == lslvx)  emit(generate_adc(a, 1, 0x0D6U, 0x08));
		else if (op == lslvw)  emit(generate_adc(a, 0, 0x0D6U, 0x08));
		else if (op == udivx)  emit(generate_adc(a, 1, 0x0D6U, 0x02));
		else if (op == udivw)  emit(generate_adc(a, 0, 0x0D6U, 0x02));
		else if (op == umaxx)  emit(generate_adc(a, 1, 0x0D6U, 0x19));
		else if (op == umaxw)  emit(generate_adc(a, 0, 0x0D6U, 0x19));
		else if (op == uminx)  emit(generate_adc(a, 1, 0x0D6U, 0x1B));
		else if (op == uminw)  emit(generate_adc(a, 0, 0x0D6U, 0x1B));

		else if (op == maddx)   emit(generate_madd(a, 1, 0x0D8, 0));
		else if (op == maddw)   emit(generate_madd(a, 0, 0x0D8, 0));
		else if (op == umaddlx) emit(generate_madd(a, 1, 0x0DD, 0));
		else if (op == umaddlw) emit(generate_madd(a, 1, 0x0DD, 0));
		else if (op == msubx)   emit(generate_madd(a, 1, 0x0D8, 1));
		else if (op == msubw)   emit(generate_madd(a, 0, 0x0D8, 1));

		else if (op == cselx)   emit(generate_csel(a, 1, 0x0D4U, 0));
		else if (op == cselw)   emit(generate_csel(a, 0, 0x0D4U, 0));
		else if (op == csincx)  emit(generate_csel(a, 1, 0x0D4U, 1));
		else if (op == csincw)  emit(generate_csel(a, 0, 0x0D4U, 1));
		else if (op == csinvx)  emit(generate_csel(a, 1, 0x2D4U, 0));
		else if (op == csinvw)  emit(generate_csel(a, 0, 0x2D4U, 0));
		else if (op == csnegx)  emit(generate_csel(a, 1, 0x2D4U, 1));
		else if (op == csnegw)  emit(generate_csel(a, 0, 0x2D4U, 1));

		else if (op == orrx)   emit(generate_orr(a, 1, 0, 0x2AU, im));
		else if (op == orrw)   emit(generate_orr(a, 0, 0, 0x2AU, im));
		else if (op == ornx)   emit(generate_orr(a, 1, 1, 0x2AU, im));
		else if (op == ornw)   emit(generate_orr(a, 0, 1, 0x2AU, im));
		else if (op == addx)   emit(generate_orr(a, 1, 0, 0x0BU, im));
		else if (op == addw)   emit(generate_orr(a, 0, 0, 0x0BU, im));
		else if (op == addxs)  emit(generate_orr(a, 1, 0, 0x2BU, im));
		else if (op == addws)  emit(generate_orr(a, 0, 0, 0x2BU, im));
		else if (op == subx)   emit(generate_orr(a, 1, 0, 0x4BU, im));
		else if (op == subw)   emit(generate_orr(a, 0, 0, 0x4BU, im));
		else if (op == subxs)  emit(generate_orr(a, 1, 0, 0x6BU, im));
		else if (op == subws)  emit(generate_orr(a, 0, 0, 0x6BU, im));
		else {
			printf("error: unknown instruction: %llu\n", op);
			printf("       unknown instruction: %s\n", instruction_spelling[op]);
			abort();
		}
	}

	make_object_file(object_filename);

	if (not access(executable_filename, F_OK)) {
    		errno = EEXIST;
		perror("exec:ld"); 
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

	if (debug) debug_output();
}

static noreturn void usage(void) { 
	exit(puts("\033[31;1merror: \033[0m\033[1musage: assembler <source.s> -c <object.o> -o <executable>\033[0m"));
}

int main(int argc, const char** argv) {
	if (argc != 6 or strcmp(argv[2], "-c") or strcmp(argv[4], "-o")) usage();
	filename = argv[1];
	text_length = 0;
	text = read_file(filename, &text_length);
	*registers = (nat)(void*) malloc(65536); 
	parse();
	generate_machine_code(argv[3], argv[5]);
}
































/*
static const char digits[36] = "0123456789abcdefghijklmnopqrstuvwxyz"

static nat string_to_number(char* string, nat* length) {
	nat radix = 0, value = 0;
	nat result = 0, index = 0, place = 1;
begin:	if (index >= *length) goto done;
	value = 0;
top:	if (value >= 36) goto found;
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
*/















/*


//if (is(word, count, "debugwords")) { print_words(); goto next; }



static void print_words(void) {
	printf("dicitonary of words: (%llu){\n", word_count);
	for (nat i = 0; i < word_count; i++) {
		printf("struct word { .name = \033[31m\"%s\"\033[0m, .length = %llu, .body = [%llu, %llu] (\"\033[32m%.*s\033[0m\")}\n",
			strndup(words[i].name, words[i].length), words[i].length, 
			words[i].begin, words[i].end, 
			(int)(words[i].end - words[i].begin), text + words[i].begin
		);
		puts("");
	}
	puts("}");
}





//static nat word_count = 0;
//static struct word words[4096] = {0};
static nat macro = 0;
static const char* return_word = NULL;
static nat return_count = 0;

static nat indexstack[4096] = {0};
static nat endstack[4096] = {0};
static nat stack_count = 0;





if (is(word, count, "remove")) { words[word_count - 1].length = 0; goto next; }







static bool equals(const char* word, nat count, const char* word2, nat count2) {
	return count == count2 and not strncmp(word, word2, count);
}



static void emit_sequence(const char* string, const nat count) {
	memcpy(bytes + byte_count, string, count);
	byte_count += count;
	// if (byte_count % 4) byte_count += (4 - byte_count % 4);     // TODO: do this in its own defined word:   "align <X>" 
}









	emit_sequence(
				text + words[a->value].begin + 1, 
				words[a->value].end - words[a->value].begin - 2
			);

			// text + w.begin + 1, w.end - w.begin - 2    






if (stack_count) {
		if (debug) printf("\033[35m RETURNING FROM A MACRO!! %.*s...\033[0m\n", (int) 0, "UNKNOWN-MACRO-NAME-HERE");
		index = indexstack[--stack_count];
		end = endstack[stack_count];
		goto next;
	}








		if (is(word, count, "include")) {
			char newfilename[4096] = {0};
			const struct word w = words[--word_count];
			strncat(newfilename, text + w.begin + 1, w.end - w.begin - 2);
			if (debug) printf("\033[32mIncluding file \"%s\"...\033[0m\n", newfilename);
			nat newtext_length = 0;
			const char* newtext = read_file(newfilename, &newtext_length);
			text = realloc(text, text_length + newtext_length + 1);
			memmove(text + index + 1 + newtext_length + 1, text + index + 1, text_length - (index + 1));
			memcpy(text + index + 1, newtext, newtext_length);
			text[index + 1 + newtext_length] = ' ';
			for (nat i = 0; i < stack_count; i++) endstack[i] += newtext_length + 1;
			end += newtext_length + 1;
			text_length += newtext_length + 1;
			goto next;
		}









		for (nat w = 0; w < word_count; w++) {
			if (not equals(word, count, words[w].name, words[w].length)) continue;
			if (debug) printf("\033[35m CALLING A MACRO!! %.*s...\033[0m\n", (int) words[w].length, words[w].name);
			indexstack[stack_count] = index;
			endstack[stack_count++] = end;
			index = words[w].begin;
			end = words[w].end;
			goto next;
		}









		words[word_count++] = (struct word) {
			.name = strndup(word, count), 
			.length = count, .begin = index, .end = 0,
		};

		if (debug) printf("\033[34m STARTING A MACRO!! %.*s...\033[0m\n", (int) count, word);

		macro = 1;

		return_word = word;
		return_count = count;







if (not macro) return;
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "unterminated operation macro");
	print_error(reason, start, count);
	exit(1);




	






//if (debug) printf("[%s]: ret:[%.*s] %s: processing: \"\033[31m%.*s\033[0m\"...\n", macro ? "MACRO-state" : "[no-macro]", (int) return_count, return_word, filename, (int) count, word);


//printf("macro = %d\n", macro); 





		if (macro and equals(word, count, return_word, return_count)) {
			if (debug) printf("\033[32m returnword=\"%.*s\": terminated macro at %llu...\033[0m\n", (int) return_count, return_word, start);
			words[word_count - 1].end = start;
			return_count = 0;
			macro = 0; 
			goto next;

		} else if (macro) goto next;
		*/





/*





if (is(word, count, "startmacro")) { 
			
			words[word_count++] = (struct word) {
				.name = strndup("word", 4), 
				.length = 4, .begin = index, .end = 0,
			};

			if (debug) printf("\033[34m STARTING A NON-USER-DEFINED MACRO!! %.*s...\033[0m\n", (int) count, word);
			macro = 1;
			return_word = "return";
			return_count = 6;
			goto next;
		}

		if (is(word, count, "string")) { 
			arg.value = word_count - 1; 
			arguments[arg_count++] = arg; 
			goto next;
		}





		if (not registers[31]) {
			char reason[4096] = {0};
			snprintf(reason, sizeof reason, "undefined word \"%.*s\"", (int) count, word);
			print_error(reason, start, count);
			exit(1);
		}


*/

		//if (debug) printf("[%s]: ret:[%.*s] %s: after processing: \"\033[31m%.*s\033[0m\"...\n", macro ? "MACRO-state" : "[no-macro]", (int) return_count, return_word, filename, (int) count, word);

		//if (count == 10) abort();
		//else printf("count = %d\n", (int) count);
		

/*
						if (arg_count >= ) {
							char reason[4096] = {0};
							snprintf(reason, sizeof reason, "argument list full");
							print_error(reason, start, count); 
							exit(1);
						}



else if (macro) {
			if (debug) printf("trying for macro calls at %llu...\n", start);
			//goto process_macro_call;
		}




				*/
















//TODO:  make the foundation file implicitly included.  make this disableable   by supplying a command line flag, "-raw"...? maybe.

			// implmeent this by simply mecpying a string into the beginning of the text buffer, then passing the new start of the text buffer to read file, and read file "appends" the contents to the end of where the include string left off. simple ish
















/*





// push the count onto the stack, and the previous "index" value! 



======================
	todo:
======================





x	- redo how multiple files and macros are implemented, to make it do a string insertion into the current file, when a file is included. then, macros can get by without storing the body as a string in the dictionary for each word,  
			 instead just merely storing a start and end   or start and length numbers. 







	- make strings emit their bytes into the compiletime memory instead of the text byte section!!!!

			so that we can essentially make a c_string table at compiletime, using ct instructions!


						if we write the data onlyyy directly to the text section, we must write strings only at the end of the file, which isnt great.. so yeah. lets make them "relocatable" by making them output the raw bytes to the ct memory, and then move them at compiletime to the right runtime place in the text section, after the code, basically. so yeah. should work, i think. 


					


	



	- seperate out alignment from the emit string    or emit_sequence function      so that we can put strings back to back without any alignment reqiured! very important. make the  emit byte function not align at all. important. 

					ie, only emit one byte. 


		make suer there are no generated zero bytes by accident unless requested!!!


					yay


 







	x - add a state variable using ctr[30]   which controls which macros are interpreted! you need the name to match, and the state variable to match. so yeah. 

				this allows so much sort of syntax. its crazy. 

					for instance, we could have spaces in names if we do that lol. 

						seriously lol. 
								like, we could define functions that are called

										print usage and exit with error 2

								thats the name of a single function. nice.
									oh, and we could implement a type system with this state thingy too.  pretty amazing. lol. nice. 


					OH MY GOSH WE SHOULD ADD THE ABILITY TO RENAME A MACROOOOO


							YESSSSS LETS ADDD THAT TOOOOOO


						super useful!!! maybe we don't need remove-name now?   
							because thats simply renaming it to the empty stringgg lololol
								idkkkk 


								i feel like no.. we want to make the name uncallable... but we cannot spell the empty string at all. so we need remove,   aka    "rename-empty"   instruction. 










										



	
	- make labels actually work properly, by having the value be persistent in memory!! ie, labels have an _attr_ ct ins, which fills in a number in a place in memory, 
		and branch/label-taking instructions always deref the pointer that is given in memory, and we use a ctregisters value at parse time for the memory location/address, to store the persistent value in memory. 
			.....yup. its that complicated lol.  



			the way labels will work will be to 


				basically we first need to add a       attr         ins         "ctat"

				and then we need to make it cache the value of ctregisters?

					

	
				and thennn we need to make all  branches/etc   use the deref'd ctr as the imm argument.



				simple as that!! done. 


				
	




*/














































/*








//if (debug) printf("contents for %s: = \"%.*s\"\n", filename, (int) text_length, text);
			//if (debug) printf("contents for %s: = \"%.*s\"\n", newfilename, (int) newtext_length, newtext);









struct afile {
	nat text_length;
	nat index;
	const char* text;
	const char* filename;
};
//static nat filecount = 0;
//static struct afile filestack[4096] = {0};




//filename = strdup(newfilename);




//goto begin; 
			//end: if (debug) printf("info: finished processing that file, continuing to process %s...\n", filename);
			

//char newfilename[4096] = {0};
			//memcpy(newfilename, filename, strlen(filename));
			//strncat(newfilename, ":", 1);
			//strncat(newfilename, words[w].name, words[w].length);
			//filename = strdup(newfilename); 
			//text = words[w].body;
			//text_length = words[w].body_length;







if ((int) words[word_count - 1].count < 0) {
	puts("macro is empty");
	//print_words();
	abort();
}

//printf("words[word_count - 1].body_length = %llu\n", words[word_count - 1].body_length);
words[word_count - 1].body = 
	strndup(
		words[word_count - 1].body, 
		words[word_count - 1].body_length
	);






filestack[filecount++] = (struct afile) {
				.filename = filename,
				.text = text,
				.text_length = text_length,
				.index = index,
			};




filestack[filecount++] = (struct afile) {
				.filename = filename,
				.text = text,
				.text_length = text_length,
				.index = index,
			};








if (debug) {
				for (nat i = 0; i < filecount; i++) {
					printf("struct file: { filename: %s, text:%.*s, text_length:%llu, index:%llu } \n", 
						filestack[i].filename, 
						(int) filestack[i].text_length, filestack[i].text, 
						filestack[i].text_length, 
						filestack[i].index
					);

					const struct afile save = {
						.filename = filename, 
						.text = text, 
						.text_length = text_length, 
						.index = index, 
					};

					filename = filestack[i].filename;
					text = filestack[i].text;
					text_length = filestack[i].text_length;

					print_error("filestack[i]", filestack[i].index, 1);

					filename = save.filename;
					text = save.text;
					text_length = save.text_length;
					index = save.index;

					puts("");
				}
				printf("---> struct file: { filename: %s, text:%.*s, text_length:%llu, index:%llu } \n", 
						filename, 
						(int) text_length, text, 
						text_length, 
						index
				);
				print_error("filestack[i]", index, 1);

				print_registers();

			}




// if (debug) print_words(); 



//if (not filecount) return;
	//struct afile f = filestack[--filecount];
	//filename = f.filename;
	//text = f.text;
	//text_length = f.text_length;
	//index = f.index;
	//goto end;







//		if (not arg_count) { 


		} else {
			
		}

*/


















		/* put this else where: todo



			so i found a problem with the language.  and its big:

				how do we specify constnats?     if we can't give an offset that is large.

							and we can't have a large number of virtual registers, 

					like, i want to have registers referable by some name ,  but i don't want to have named labels be the only option 

						like, ideally there is some other way to refer to labels. 
						like,  what about computation???

								that could workkk


						i think 


							ie, you make a pointer have a certain value and then thats what you give to the branch!

								okay, cool, so it actually just derefs the pointer!!! that the trick. lol. thats easy cool. okay. 
									it just derefs it. 

								nice. 

	
		*/


















//"-fno-unwind-tables -fno-asynchronous-unwind-tables "   //" -version_details "





//printf("simple register alias macro!!!!\n");
			
			// ...
			// d cfinv d
			// printf("comment / operational macro!!!\n");

			//arg.value = ~word_count; 
			//arguments[arg_count++] = arg; 
			// getchar();












/// 		  * ( ( u32 * )  * registers + a0 ) = ins_count;




//	filename = argv[1];
//	text_length = read_file(filename);





/*


//static u32 load(const nat offset) { return ; }  
					///TODO: right here. this should be a simply deref, to either ctr[0] or ctr[i] 
					//where the user gives i for the instruction. i tihnk if there are ever two immediates that 
					//they need to give, which never happens, then we need to give an arbirary pointer. i think. hm. 




struct section_64 dsection = {0};
	strncpy(dsection.sectname, "__data", 16);
	strncpy(dsection.segname, "__TEXT", 16);
	dsection.addr = byte_count;
	dsection.size = data_count;
	dsection.align = 3;
	dsection.reloff = 0;
	dsection.nreloc = 0;
	dsection.flags = S_REGULAR;

	dsection.offset = (uint32_t) (
				sizeof (struct mach_header_64) + 
				sizeof (struct segment_command_64) + 
				sizeof (struct section_64) + 
				sizeof (struct section_64) + 
				sizeof (struct symtab_command) + 
				byte_count
			);



















0		- implment   adding two relocation entries for  the .data section  and .bss section,   
									called    __data_start   and  __bss_start.
				then   do a fixup on a pc-rel offset      and then use immediate offsets in ldr and str's to 
					access globals and bssglobals. so yeah. this should work. 



// we only need one of these!



do we use these?

 adrp x0, _foo@PAGE
 *         r_type=ARM64_RELOC_PAGE21, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
 *         0x90000000



or these?


 *     adrp x0, _foo@PAGE + 0x24
 *         r_type=ARM64_RELOC_ADDEND, r_length=2, r_extern=0, r_pcrel=0, r_symbolnum=0x000024
 *         r_type=ARM64_RELOC_PAGE21, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
 *         0x90000000
 *

			^---------- this is the one we will use!    we will genreate one of these every time we see the ins      adrpd 




					   and the usage of adrpd  is                <ct_constant> <rt_reg> adrpd


						ie, you give it a ct constant offset from the b:only; relocated symbol   __data_start
	
						which is the implicit argument to   adrpd. 


				

					nice. i think this is pretty cool actually. nice. 



		god i hope this works.  this seems complicated but we can do it. 



			
	x	6-   add an instruction to specify the bss size at compiletime.      "<imm: ctr0 + offset> bss"











// write(file, &dsection, sizeof(struct section_64));



// 
	// if (byte_count % 8) byte_count += (8 - byte_count % 8);


// write(file, data, data_count);


//else if (op == ctdc)   registers[a0] = data_count;
	//else if (op == cted)   data[data_count++] = (uint8_t) registers[a0];
	//else if (op == cter)   { for (nat i = 0; i < registers[a0]; i++) data[data_count++] = *(uint8_t*) (registers[a1] + i); }







#include <mach-o/reloc.h>
#include <mach-o/arm64/reloc.h>


//static nat data_count = 4;
//static uint8_t data[4096] = {0xDE, 0xAD, 0xBE, 0xEF}; 			// for testing: 0xDE, 0xAD, 0xBE, 0xEF






static void emit_sequence(uint8_t* data, const nat count) {
	memcpy(bytes + byte_count, data, count);
	byte_count += count;
	if (byte_count % 4) byte_count += (4 - byte_count % 4);
}


























================================= DONE ======================================
================================= DONE ======================================
================================= DONE ======================================



	x	3- implement multiple files....

	x	1.1- implement macros. 

	x	- implement macros fully 

	x	- we only have 32 ctregisters to use to give labels to statements.   bad. 
			...we should have an unlimited amount of them.  plz. 


	x	- implement a .js backend!   for web stuff.      [actually, no. do this later]


	x	- work on generating the .data segment/section, 

	x	- and other bss/data sections. 


	x	- add more rt instructions to make the language actually usable:

	x	- add the emmision of bytes   instread of words   in the text section!!      			"emitb"
						just make sure its aligned afterwards though. yay. 


		x	- shift left ins
		x	- mul ins
		x	- div ins
		x	- rem ins
		x	- store ins!
		x	- load ins!
		x	- adr
		x	- adrp

	x	- add a ctnop instruction!!!!    ( very useful for argument stuff. and generally useful.)


	x	- add emitctbyterange   rt instruction, for generating the data section!

	x   	- add ctmalloc to the ct instructions?

	x	- add more ct branches. at least cteq, ctge     (b/f versions!)	






================================= DONE ======================================
================================= DONE ======================================
================================= DONE ======================================








} else {
				printf("found label: \"%.*s\"\n", (int) words[w].length, words[w].name);
				if (words[w].at != ins_count) words[w].at = (u32) ins_count;
				arguments[arg_count++] = w; 
			}





*/






//nat length;
//nat type;
//nat reg;
//nat end;
//nat value;
//nat at;
//nat file_index;






/*
			printf("found macro string: %.*s --> %llu...%llu\n", (int) w.length, w.name, w.value, w.end);
			print_error("just for testing!", w.value + w.length + 1, 1);
			print_error("just for testing!", w.end - 1, 1);

			
			memcpy(new_filename, text + w.value + w.length + 1, (w.end - 1) - (w.value + w.length + 1));
			*/






//static nat stack_count = 0;
//static nat stack[4096] = {0};
//static nat word_stack[4096] = {0};

//static nat return_count = 0;
//static char* return_word = NULL;





/*
if (filecount) {
			const nat w = word_stack[stack_count - 1];
			if (equals(word, count, words[w].name, words[w].length)) {

				puts("EXITING MACRO: found return statement!");
				print_error("just for testing!", start, count);

				--stack_count;
				index = stack[stack_count];
				string = string_stack[stack_count];
				string_length = length_stack[stack_count];

				// print_error("going back to here!", index, 1);
				goto next;
			}
		}



char newfilename[4096] = {0};
			snprintf(newfilename, sizeof newfilename, "macro:%.*s", (int) words[w].length, words[w].name);






*/







// static char newfilename[4096] = {0};






//arg_count = 0;        // TODO:  use a stack-like argument thingy.       don't do this unconditionally.







      // put this back in!! don't use  pc +=   for jumps. ever.


/*

struct word {
	const char* name;
	nat length;
	nat begin;
	nat end;
};



static void emit_byte(u32 x) {
	bytes[byte_count++] = (uint8_t) x;
}



//printf("\ndebugging bytes bytes:\n------------------------\n");
	//dump_hex((uint8_t*) bytes, byte_count);



static void dump_hex(uint8_t* local_bytes, nat local_byte_count) {
	printf("dumping hex bytes: (%llu)\n", local_byte_count);
	for (nat i = 0; i < local_byte_count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx ", local_bytes[i]);
	}
	puts("");
}


*/

