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

/*
==============================================

	open major issues:

3		3- implement multiple files....

1		1- implement aliases.
		1.1- implement macros. 

2		2- implement user strings.

==============================================

	old todo:
-----------------------------------------------------


	x	- we only have 32 ctregisters to use to give labels to statements.   bad. 
			...we should have an unlimited amount of them.  plz. 


	x	- implement a .js backend!   for web stuff.      [actually, no. do this later]

------------------------------------------------------


	usability:
	=======================================
		- implement macros fully 
		- implement aliases fully 



	COMPILE TIME STUFF:
	=======================================
	x	- add a ctnop instruction!!!!    ( very useful for argument stuff. and generally useful.)



		- add ctmalloc to the ct instructions?

	x	- add more ct branches. at least cteq, ctge     (b/f versions!)	

		- add a ctsyscall instruction!..?

	x	- add emitctbyterange   rt instruction, for generating the data section!


	OBJECT FILE STUFF 
	=======================================

	x	- work on generating the .data segment/section, 

		- and other bss/data sections. 




	ARM64 ISA RUNTIME STUFF 
	=======================================
	x	- add more rt instructions to make the language actually usable:

		- add the emmision of bytes   instread of words   in the text section!!      			"emitb"
						just make sure its aligned afterwards though. yay. 


		x	- shift left ins
		x	- mul ins
		x	- div ins
		x	- rem ins
		x	- store ins!
		x	- load ins!
		x	- adr
		x	- adrp




	DOCUMENATION STUFF
	=======================================

		- document the meaning of each argument for each ct/rt instruction more, 
				also accoridng to the arm isa ref manual!

		- write a hello world program


*/
enum instruction_type {
	nop, dw, svc, cfinv, br, blr, b_, bc, adr, adrp,

	movzx, movzw,	movkx, movkw,	movnx, movnw,
	addix, addiw,	addhx, addhw,
	addixs, addiws,	addhxs, addhws,
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


	ctnop, ctzero, ctincr, 
	ctadd, ctsub, ctmul, ctdiv, ctrem,
	ctnor, ctxor, ctor, ctand, ctshl, ctshr, ctprint, 
	ctld1, ctld2, ctld4, ctld8, ctst1, ctst2, ctst4, ctst8,
	ctat, ctpc, ctblt, ctbge, ctbeq, ctbne, ctgoto, ctstop, 
	cthalt,

	instruction_set_count
};

static const char* const instruction_spelling[instruction_set_count] = {
	"nop", "dw", "svc", "cfinv", "br", "blr", "b", "bc", "adr", "adrp",

	"movzx", "movzw", "movkx", "movkw", "movnx", "movnw",
	"addix", "addiw", "addhx", "addhw",
	"addixs", "addiws", "addhxs", "addhws",
	"maddx", "maddw",

	"striux", "striuw",  "ldriux",  "ldriuw",  "striox", 
	"striow", "striex",  "striew",  "ldriox",  "ldriow", 
	"ldriex", "ldriew",  "ldurx",   "ldtrx",   "ldurw", 
	"ldtrw",  "ldtrsw",  "ldtrh",   "ldtrshx", "ldtrshw", 
	"ldtrb",  "ldtrsbx", "ldtrsbw", "lslvx",   "lslvw", 
	"udivx",  "udivw",   "umaxx",   "umaxw",   "uminx", 
	"uminw",  "umaddlx", "umaddlw", "msubx",   "msubw", 

	"adcx", "adcw", "adcxs", "adcws", 
	"asrvx", "asrvw", 

	"cselx", "cselw",  "csincx", "csincw", 
	"csinvx", "csinvw", "csnegx", "csnegw",

	"orrx", "orrw",	"ornx", "ornw", 
	"addx", "addw", "addxs", "addws",
	"subx", "subw", "subxs", "subws",

	"ld64b", "st64b", "absx", "absw",
	"clsx", "clsw",	"clzx", "clzw",	"ctzx", "ctzw",	"cntx", "cntw",
	"rbitx", "rbitw", "revx", "revw", "revhx", "revhw",

	"ctnop", "ctzero", "ctincr", 
	"ctadd", "ctsub", "ctmul", "ctdiv", "ctrem",
	"ctnor", "ctxor", "ctor", "ctand", "ctshl", "ctshr", "ctprint",
	"ctld1", "ctld2", "ctld4", "ctld8", "ctst1", "ctst2", "ctst4", "ctst8",
	"ctat", "ctpc", "ctblt", "ctbge", "ctbeq", "ctbne", "ctgoto", "ctstop", 
	"cthalt"
};

struct word {
	char* name;
	nat length;
	nat type;
	nat reg;
	nat end;
	nat value;
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
static struct instruction ins[100] = {0};

static nat word_count = 0;
static struct word words[100] = {0};

static nat arg_count = 0;
static struct argument arguments[6] = {0};

static nat stack_count = 0;
static nat stack[100] = {0};
static nat word_stack[100] = {0};

static nat return_count = 0;
static char* return_word = NULL;
static nat macro = 0;
static nat stop = 0;

static nat registers[32] = {0};

static nat byte_count = 0;
static uint8_t bytes[100] = {0};

static const char* filename = NULL;
static nat text_length = 0;
static char* text = NULL;


struct afile {
	const char* filename;
	char* text;
	nat text_length;
	nat start;
	nat count;
	nat index;
	nat macro;
	nat stop;
};

static nat filecount = 0;
static struct afile filestack[4096] = {0};


static bool is(char* word, nat count, const char* this) {
	return strlen(this) == count and not strncmp(word, this, count);
}

static bool equals(const char* word, nat count, const char* word2, nat count2) {
	return count == count2 and not strncmp(word, word2, count);
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

static void check_branch(int r, int c, const struct argument a, const char* type) {
	if (r > -c or r < c) return;
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "invalid %s argument %d (%d <= %d or %d >= %d)", type, r, r, c, r, c);
	print_error(reason, a.start, a.count); 
	exit(1);
}

//static u32 load(const nat offset) { return ; }  
					///TODO: right here. this should be a simply deref, to either ctr[0] or ctr[i] 
					//where the user gives i for the instruction. i tihnk if there are ever two immediates that 
					//they need to give, which never happens, then we need to give an arbirary pointer. i think. hm. 

static u32 generate_mov(struct argument* a, u32 sf, u32 op) { 
	const u32 im = *(u32*)*registers;
	const u32 hw = (u32) a[0].value;
	const u32 Rd = (u32) a[1].value;
	check(im, 1 << 16U, a[0], "immediate");
	check(hw, 4,  a[0], "register");
	check(Rd, 32, a[1], "register");
	return  (sf << 31U) | 
		(op << 23U) | 
		(hw << 21U) | 
		(im <<  5U) | Rd;
}

static u32 generate_addi(struct argument* a, u32 sf, u32 sh, u32 op) { 
	const u32 im = *(u32*)*registers;
	const u32 Rn = (u32) a[0].value;
	const u32 Rd = (u32) a[1].value;
	check(im, 1 << 12U, a[0], "immediate");
	check(Rn, 32, a[0], "register");
	check(Rd, 32, a[1], "register");
	return  (sf << 31U) | 
		(op << 23U) | 
		(sh << 22U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_stri(struct argument* a, u32 si, u32 op, u32 o2) {
	const u32 im = *(u32*)*registers;
	const u32 Rn = (u32) a[0].value;
	const u32 Rt = (u32) a[1].value;
	check(im, 1 << 9U, a[0], "immediate");
	check(Rn, 32, a[0], "register");
	check(Rt, 32, a[1], "register");
	return  (si << 30U) | 
		(op << 21U) | 
		(im << 12U) | 
		(o2 << 10U) |
		(Rn <<  5U) | Rt;
}

static u32 generate_striu(struct argument* a, u32 si, u32 op) {
	const u32 im = *(u32*)*registers;
	const u32 Rn = (u32) a[0].value;
	const u32 Rt = (u32) a[1].value;
	check(im, 1 << 12U, a[0], "immediate");
	check(Rn, 32, a[0], "register");
	check(Rt, 32, a[1], "register");
	return  (si << 30U) | 
		(op << 21U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rt;
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
		(Rn <<  5U) | Rd;
}

static u32 generate_madd(struct argument* a, u32 sf, u32 op, u32 o2) {  
	const u32 Ra = (u32) a[0].value;
	const u32 Rm = (u32) a[1].value;
	const u32 Rn = (u32) a[2].value;
	const u32 Rd = (u32) a[3].value;
	check(Ra, 32, a[0], "register-z");
	check(Rm, 32, a[1], "register");
	check(Rn, 32, a[2], "register");
	check(Rd, 32, a[3], "register");
	return  (sf << 31U) | 
		(op << 21U) | 
		(Rm << 16U) | 
		(o2 << 15U) |
		(Ra << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_csel(struct argument* a, u32 sf, u32 op, u32 o2) {  
	const u32 Rm = (u32) a[0].value;
	const u32 Rn = (u32) a[1].value;
	const u32 Rd = (u32) a[2].value;
	const u32 cd = (u32) a[3].value;
	check(Rm, 32, a[0], "register");
	check(Rn, 32, a[1], "register");
	check(Rd, 32, a[2], "register");
	check(cd, 16, a[3], "condition");
	return  (sf << 31U) | 
		(op << 21U) | 
		(Rm << 16U) | 
		(cd << 12U) | 
		(o2 << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_orr(struct argument* a, u32 sf, u32 ne, u32 op) { 
	const u32 im = *(u32*)*registers;
	const u32 sh = (u32) a[0].value;
	const u32 Rm = (u32) a[1].value;
	const u32 Rn = (u32) a[2].value;
	const u32 Rd = (u32) a[3].value;
	check(im, 32U << sf, a[0], "immediate");
	check(sh, 4,  a[0], "register");
	check(Rm, 32, a[1], "register");
	check(Rn, 32, a[2], "register");
	check(Rd, 32, a[3], "register");
	return  (sf << 31U) | 
		(op << 24U) | 
		(sh << 22U) | 
		(ne << 21U) | 
		(Rm << 16U) | 
		(im << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_abs(struct argument* a, u32 sf, u32 op) {  
	const u32 Rn = (u32) a[0].value;
	const u32 Rd = (u32) a[1].value;
	check(Rn, 32, a[0], "register");
	check(Rd, 32, a[1], "register");
	return  (sf << 31U) | 
		(op << 10U) | 
		(Rn <<  5U) | Rd;
}

static u32 generate_br(struct argument* a, u32 op) { 
	const u32 Rn = (u32) a[0].value;
	check(Rn, 32, a[0], "register");
	return  (op << 10U) | (Rn << 5U);
}

static u32 generate_b(struct argument* a, uint32_t pc) { 
	const u32 im = *(u32*)*registers;
	const u32 offset = im - pc;
	check_branch((int) offset, 1 << (26 - 1), a[0], "branch offset");
	return (0x05 << 26U) | (0x03FFFFFFU & offset);
}

static u32 generate_bc(struct argument* a, uint32_t pc) { 
	const u32 im = *(u32*)*registers;
	const u32 cd = (u32) a[0].value;
	const u32 offset = im - pc;
	check_branch((int) offset, 1 << (19 - 1), a[0], "branch offset");
	check(cd, 16, a[0], "condition");
	return (0x54U << 24U) | ((0x0007FFFFU & offset) << 5U) | cd;
}

static u32 generate_adr(struct argument* a, u32 op, u32 o2, uint32_t pc) { 
	const u32 im = *(u32*)*registers;
	const u32 Rd = (u32) a[0].value;
	const u32 offset = im - pc;
	check_branch((int) offset, 1 << (21 - 1), a[0], "pc-relative address");
	check(Rd, 32, a[0], "register");
	const u32 lo = 0x03U & offset, hi = 0x07FFFFU & (offset >> 2);
	return  (o2 << 31U) | 
		(lo << 29U) | 
		(op << 24U) | 
		(hi <<  5U) | Rd;
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

static void execute(nat op, nat* pc) {
	const nat a0 = arguments[0].value;
	const nat a1 = arguments[1].value;
	const nat a2 = arguments[2].value;

	if (op == ctstop) {if (registers[a0] == stop) stop = 0; arg_count = 0; return; }
	else if (stop) return;

	if (op == ctnop) {}
	else if (op == ctat)   *((u32*)*registers+a0) = (u32) ins_count;
	else if (op == ctpc)   *((u32*)*registers+a0) = (u32) *pc;
	else if (op == ctgoto) *pc = *((u32*)*registers+a0); 

	else if (op == ctblt)  { if (registers[a1]  < registers[a0]) stop = registers[a2]; } 
	else if (op == ctbge)  { if (registers[a1] >= registers[a0]) stop = registers[a2]; } 
	else if (op == ctbeq)  { if (registers[a1] == registers[a0]) stop = registers[a2]; } 
	else if (op == ctbne)  { if (registers[a1] != registers[a0]) stop = registers[a2]; } 
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
	else if (op == ctprint) printf("debug: \033[32m%llu\033[0m \033[32m0x%llx\033[0m\n", registers[a0], registers[a0]); 
	else if (op == ctld1)  registers[a1] = *(uint8_t*) registers[a0]; 
	else if (op == ctld2)  registers[a1] = *(uint16_t*)registers[a0]; 
	else if (op == ctld4)  registers[a1] = *(uint32_t*)registers[a0]; 
	else if (op == ctld8)  registers[a1] = *(uint64_t*)registers[a0]; 
	else if (op == ctst1)  *(uint8_t*) registers[a1] = (uint8_t)  registers[a0]; 
	else if (op == ctst2)  *(uint16_t*)registers[a1] = (uint16_t) registers[a0]; 
	else if (op == ctst4)  *(uint32_t*)registers[a1] = (uint32_t) registers[a0]; 
	else if (op == ctst8)  *(uint64_t*)registers[a1] = (uint64_t) registers[a0]; 
	else if (op == cthalt) abort();
	
	arg_count = 0;
}


static void parse(void) {

begin:
	printf("info begining of processing for file: %s...\n", filename);
	nat count = 0, start = 0;
	nat index = 0;
	for (; index < text_length; index++) {
		if (not isspace(text[index])) { 
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;

	process:;
		char* const word = text + start;
		printf("%s: processing: \"\033[31m%.*s\033[0m\"...\n", filename, (int) count, word);
		struct argument arg = { .value = 0, .start = start, .count = count };
		
		if (macro) {
			if (equals(word, count, return_word, return_count)) {
				words[word_count - 1].end = start;
				macro = 0; goto next;
			} else goto next;
		}

		if (stack_count) {
			const nat w = word_stack[stack_count - 1];
			if (equals(word, count, words[w].name, words[w].length)) {

				puts("EXITING MACRO: found return statement!");
				//print_error("just for testing!", start, count);
				index = stack[--stack_count];
				//print_error("going back to here!", index, 1);
				goto next;
			}
		}

		else if (is(word, count, "include")) {
			const struct word w = words[--word_count];

			//printf("found macro string: %.*s --> %llu...%llu\n", (int) w.length, w.name, w.value, w.end);
			//print_error("just for testing!", w.value + w.length + 1, 1);
			//print_error("just for testing!", w.end - 1, 1);

			char new_filename[4096] = {0};
			memcpy(new_filename, text + w.value + w.length + 1, (w.end - 1) - (w.value + w.length + 1));
			printf("\033[32mIncluding file \"%s\"...\033[0m\n", new_filename);



			filestack[filecount++] = (struct afile) {
				.filename = filename,
				.text = text,
				.text_length = text_length,
				.start = start,
				.count = count, 
				.index = index,
				.macro = macro,
				.stop = stop,
			};

			filename = new_filename;
			text = read_file(filename, &text_length);
			printf("contents for %s: = \"%.*s\"\n", filename, (int) text_length, text);
			
			goto begin;
			end:  
			printf("info: finished processing that file, continuing to process %s...\n", filename);
			goto next;
		}



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


		for (nat i = 0; i < 32; i++) {
			char r[5] = {0};
			snprintf(r, sizeof r, "r%llu", i);
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
			if (i >= ctnop) execute(i, &index); 
			else if (not stop) push(i, start, count);
			goto next;
		}

		for (nat w = 0; w < word_count; w++) {
			if (not equals(word, count, words[w].name, words[w].length)) continue;
			printf("\033[35m CALLING A MACRO!! %.*s...\033[0m\n", (int) words[w].length, words[w].name);
			stack[stack_count] = index;
			word_stack[stack_count] = w;
			stack_count++;
			index = words[w].file_index;
			goto next;
		}


		if (not arg_count) { 
			
			words[word_count++] = (struct word) {
				.name = word, 
				.length = count, 
				.reg = 9999999LLU, 
				.value = start, 
				.file_index = index,
			};

			macro = 1;
			return_word = word;
			return_count = count;
			goto next;

		} else {
			char reason[4096] = {0};
			snprintf(reason, sizeof reason, 
				"undefined word found \"%.*s\"", 
				(int) count, word
			);
			print_error(reason, start, count);
			exit(1);
		}

		next: count = 0;
	}

	if (count) goto process;

	if (macro) {
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "unterminated operation macro");
		print_error(reason, words[word_count - 1].value, words[word_count - 1].length);
		exit(1);
	}

	if (not filecount) return;
	
	const struct afile f = filestack[--filecount];
	filename = f.filename;
	text = f.text;
	text_length = f.text_length;
	start = f.start;
	count = f.count;
	index = f.index;
	macro = f.macro;
	stop = f.stop;
	goto end;
}


static void make_object_file(void) {

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

	const char* output_filename = "object.o";
	const int flags = O_WRONLY | O_CREAT | O_TRUNC;               // | O_EXCL;
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(output_filename, flags, mode);
	if (file < 0) { perror("open"); exit(1); }

	write(file, &header, sizeof(struct mach_header_64));
	write(file, &segment, sizeof (struct segment_command_64));
	write(file, &section, sizeof(struct section_64));
	write(file, &table, sizeof table);
	write(file, bytes, byte_count);
	write(file, symbols, sizeof(struct nlist_64));
	write(file, strings, sizeof strings);

	close(file);
}

static void debug(void) {
	printf("\ndebugging bytes bytes:\n------------------------\n");
	dump_hex((uint8_t*) bytes, byte_count);
	system("otool -txvVhlL object.o");
	system("otool -txvVhlL executable.out");
	system("objdump object.o -DSast --disassembler-options=no-aliases");
	system("objdump executable.out -DSast --disassembler-options=no-aliases");
}

static void generate_machine_code(void) {

	for (nat i = 0; i < ins_count; i++) {

		const nat op = ins[i].op;
		struct argument* const a = ins[i].arguments;

		     if (op == dw)     emit(*(u32*)*registers);
		else if (op == svc)    emit(0xD4000001);
		else if (op == nop)    emit(0xD503201F);
		else if (op == cfinv)  emit(0xD500401F);
		
		else if (op == br)     emit(generate_br(a, 0x3587C0U));
		else if (op == blr)    emit(generate_br(a, 0x358FC0U));
		else if (op == b_)     emit(generate_b(a, (u32) i));
		else if (op == bc)     emit(generate_bc(a, (u32) i));
		else if (op == adr)    emit(generate_adr(a, 0x10U, 0, (u32) i));
		else if (op == adrp)   emit(generate_adr(a, 0x10U, 1, (u32) i));

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

		else if (op == striux)  emit(generate_striu(a, 3, 0xE4U));
		else if (op == striuw)  emit(generate_striu(a, 2, 0xE4U));
		else if (op == ldriux)  emit(generate_striu(a, 3, 0x1C2U));
		else if (op == ldriuw)  emit(generate_striu(a, 2, 0x1C2U));

		else if (op == striox)  emit(generate_stri(a, 3, 0x01C0U, 0x1U));
		else if (op == striow)  emit(generate_stri(a, 2, 0x01C0U, 0x1U));
		else if (op == striex)  emit(generate_stri(a, 3, 0x01C0U, 0x3U));
		else if (op == striew)  emit(generate_stri(a, 2, 0x01C0U, 0x3U));
		else if (op == ldriox)  emit(generate_stri(a, 3, 0x01C2U, 0x1U));
		else if (op == ldriow)  emit(generate_stri(a, 2, 0x01C2U, 0x1U));
		else if (op == ldriex)  emit(generate_stri(a, 3, 0x01C2U, 0x3U));
		else if (op == ldriew)  emit(generate_stri(a, 2, 0x01C2U, 0x3U));
		else if (op == ldurx)   emit(generate_stri(a, 3, 0x01C2U, 0x0U));
		else if (op == ldtrx)   emit(generate_stri(a, 3, 0x01C2U, 0x2U));
		else if (op == ldurw)   emit(generate_stri(a, 2, 0x01C2U, 0x0U));
		else if (op == ldtrw)   emit(generate_stri(a, 2, 0x01C2U, 0x2U));
		else if (op == ldtrsw)  emit(generate_stri(a, 2, 0x01C4U, 0x2U));
		else if (op == ldtrh)   emit(generate_stri(a, 1, 0x01C2U, 0x2U));
		else if (op == ldtrshx) emit(generate_stri(a, 1, 0x01C4U, 0x2U));
		else if (op == ldtrshw) emit(generate_stri(a, 1, 0x01C6U, 0x2U));
		else if (op == ldtrb)   emit(generate_stri(a, 0, 0x01C2U, 0x2U));
		else if (op == ldtrsbx) emit(generate_stri(a, 0, 0x01C4U, 0x2U));
		else if (op == ldtrsbw) emit(generate_stri(a, 0, 0x01C6U, 0x2U));
		
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

	make_object_file();

	system("ld -v "
		"-dead_strip "
		"-print_statistics "
		//" -version_details "
		"-no_weak_imports "
		"-fatal_warnings "
		"-no_eh_labels "
		"-warn_compact_unwind "
		"-warn_unused_dylibs "
		"-t "
		"-S "
		"-x "
		//"-fno-unwind-tables -fno-asynchronous-unwind-tables "
		"object.o " 
		"-o executable.out "
		"-arch arm64 "
		"-e _start "
		"-platform_version macos 13.0.0 13.3 "
		"-lSystem "
		"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk "
		
		
	);
}

int main(int argc, const char** argv) {

	if (argc < 2) return puts("usage: assembler <file1.asm> <file2.asm> ... <filen.asm>");
	filename = argv[1];
	text_length = 0;
	text = read_file(filename, &text_length);	
	*registers = (nat)(void*) malloc(65536);
	parse();
	generate_machine_code();
	debug();
}

















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

*/


