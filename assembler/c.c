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
	todo:

		- implement a .js backend!   for web stuff. 




		- implement macros fully 
		- implement aliases fully 

	x	- add a ctnop instruction!!!! very useful for argument stuff. and generally useful.

		- add ctmalloc to the ct instructions! 
		- add more ct branches. at least cteq, ctge     (b/f versions!)	
		- add a ctsyscall instruction!

		- add emitctbyterange   rt instruction, for generating the data section!

		- work on generating the .data segment/section, and other bss/data sections. 

		- add more rt instructions to make the language actually usable:
			- shift left ins
			- mul ins
			- div ins
			- rem ins
			- store and load instructions!!!!!!!!!!!!!!!!!
			- adr, adrp


		- document the meaning of each argument for each ct/rt instruction more, 
				also accoridng to the arm isa ref manual!

		- 



*/
enum instruction_type {
	nop, emitd, svc, cfinv, br, b_, bc, 

	movzx, movzw,	movkx, movkw,	movnx, movnw,
	addix, addiw,	addhx, addhw,
	addixs, addiws,	addhxs, addhws,

	adcx, adcw, 	adcxs, adcws, 
	asrvx, asrvw, 
	
	cselx, cselw, 	csincx, csincw, 
	csinvx, csinvw, csnegx, csnegw, 

	adr,

	orrx, orrw,	ornx, ornw, 
	addx, addw, 	addxs, addws,
	subx, subw, 	subxs, subws,

	ld64b, 	st64b,	absx, absw, 
	clsx, 	clsw,	clzx, clzw,	ctzx, ctzw,	cntx, cntw,    
	rbitx, 	rbitw,	revx, revw,  	revhx, revhw,

	ctnop, ctzero, ctincr, cted, ctdc,
	ctadd, ctsub, ctmul, ctdiv, ctrem,
	ctnor, ctxor, ctor, ctand, ctshl, ctshr, ctprint, 
	ctld1, ctld2, ctld4, ctld8, ctst1, ctst2, ctst4, ctst8,
	ctat, ctpc, ctblt, ctgoto, ctstop,

	instruction_set_count
};

static const char* instruction_spelling[instruction_set_count] = {
	"nop", "emitd", "svc", "cfinv", "br", "b", "bc", 

	"movzx", "movzw", "movkx", "movkw",	"movnx", "movnw",
	"addix", "addiw", "addhx", "addhw",
	"addixs", "addiws", "addhxs", "addhws",

	"adcx", "adcw", "adcxs", "adcws", 
	"asrvx", "asrvw", 

	"cselx", "cselw", 	"csincx", "csincw", 
	"csinvx", "csinvw", 	"csnegx", "csnegw", 

	"adr",

	"orrx", "orrw",	"ornx", "ornw", 
	"addx", "addw", "addxs", "addws",
	"subx", "subw", "subxs", "subws",

	"ld64b", "st64b", "absx", "absw",
	"clsx", "clsw",	"clzx", "clzw",	"ctzx", "ctzw",	"cntx", "cntw",
	"rbitx", "rbitw", "revx", "revw", "revhx", "revhw",

	"ctnop", "ctzero", "ctincr", "cted", "ctdc",
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

static nat data_count = 4;
static uint8_t data[4096] = {0xDE, 0xAD, 0xBE, 0xEF};

static nat text_length = 0;
static char* text = NULL;
static const char* filename = NULL;

static nat arg_count = 0;
static struct argument arguments[6] = {0};

static nat registers[32] = {0};

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

static void check_branch(int r, int c, const struct argument a, const char* type) {
	if (r > -c or r < c) return;
	char reason[4096] = {0};
	snprintf(reason, sizeof reason, "invalid %s argument %d (%d <= %d or %d >= %d)", type, r, r, c, r, c);
	print_error(reason, a.start, a.count); 
	exit(1);
}

static void emitdata(struct argument* a) {
	const nat Im = (u32) a[0].value;
	check(Im, 32, a[0], "ctregister");
	emit((u32) registers[Im]);
}

static u32 generate_mov(struct argument* a, u32 sf, u32 op) { 
	const nat Im = a[0].value;
	const u32 hw = (u32) a[1].value;
	const u32 Rd = (u32) a[2].value;
	check(Im, 32, a[0], "ctregister");
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
	check(Im, 32, a[0], "ctregister");
	const nat imm = registers[Im];
	check(imm, 1 << 12U, a[0], "immediate");
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
		(Rn << 5U)  | 
		 Rd;
}

static u32 generate_orr(struct argument* a, u32 sf, u32 ne, u32 op) { 

	const nat Im = 	     a[0].value;
	const u32 sh = (u32) a[1].value;
	const u32 Rm = (u32) a[2].value;
	const u32 Rn = (u32) a[3].value;
	const u32 Rd = (u32) a[4].value;

	check(Im, 32, a[0], "ctregister");
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
	check(Im, 32, a[0], "ctregister");
	const u32 offset = ((uint32_t) registers[Im] - pc);
	check_branch((int) offset, 1 << 25, a[0], "branch offset");
	return (0x05 << 26U) | (0x03FFFFFFU & offset);
}

static u32 generate_bc(struct argument* a, uint32_t pc) { 
	const u32 Im = (u32) a[0].value;
	const u32 cd = (u32) a[1].value;
	check(Im, 32, a[0], "ctregister");
	check(cd, 16, a[1], "condition");
	const u32 offset = ((uint32_t) registers[Im] - pc);
	check_branch((int) offset, 1 << 18, a[0], "branch offset");
	return (0x54U << 24U) | ((0x0007FFFFU & offset) << 5U) | cd;
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

	if (op == ctnop) {}
	else if (op == ctat)   registers[a0] = ins_count;
	else if (op == ctpc)   registers[a0] = *pc;
	else if (op == ctgoto) *pc = registers[a0]; 
	else if (op == ctblt)  { if (registers[a1] < registers[a0]) stop = registers[a2]; } 
	else if (op == ctincr) registers[a0]++;
	else if (op == ctzero) registers[a0] = 0;
	else if (op == ctdc)   registers[a0] = data_count;
	else if (op == cted)   data[data_count++] = (uint8_t) registers[a0];
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

static void parse(void) {

	nat count = 0, start = 0;

	for (nat index = 0; index < text_length; index++) {
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


		for (nat i = 0; i < 32; i++) {
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
			if (i >= ctnop) execute(i, &index); 
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
}


static void make_object_file(void) {

	data_count += (8 - data_count % 8);

	byte_count += (8 - byte_count % 8);


	struct mach_header_64 header = {0};
	header.magic = MH_MAGIC_64;
	header.cputype = (int)CPU_TYPE_ARM | (int)CPU_ARCH_ABI64;
	header.cpusubtype = (int) CPU_SUBTYPE_ARM64_ALL;
	header.filetype = MH_OBJECT;
	header.ncmds = 2;
	header.sizeofcmds = 0;
	header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;
	header.sizeofcmds = 	sizeof(struct segment_command_64) + 
				sizeof (struct section_64) + 
				sizeof (struct section_64) + 
				sizeof(struct symtab_command);


	struct segment_command_64 segment = {0};
	strncpy(segment.segname, "__TEXT", 16);
	segment.cmd = LC_SEGMENT_64;
	segment.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64) * 2;
	segment.maxprot = (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
	segment.initprot = (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);		
	segment.nsects = 2;
	segment.vmaddr = 0;
	segment.vmsize = byte_count + data_count;
	segment.filesize = byte_count + data_count;

	segment.fileoff = 	sizeof(struct mach_header_64) + 
				sizeof(struct segment_command_64) + 
				sizeof (struct section_64) + 
				sizeof (struct section_64) + 
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

	section.offset = 	sizeof (struct mach_header_64) + 
				sizeof (struct segment_command_64) + 
				sizeof (struct section_64) + 
				sizeof (struct section_64) + 
				sizeof (struct symtab_command);


	struct section_64 dsection = {0};
	strncpy(dsection.sectname, "__data", 16);
	strncpy(dsection.segname, "__TEXT", 16);
	dsection.addr = byte_count;
	dsection.size = data_count;
	dsection.align = 3;
	dsection.reloff = 0;
	dsection.nreloc = 0;
	dsection.flags = S_REGULAR;

	dsection.offset = 	sizeof (struct mach_header_64) + 
				sizeof (struct segment_command_64) + 
				sizeof (struct section_64) + 
				sizeof (struct section_64) + 
				sizeof (struct symtab_command) + 
				byte_count;


	const char strings[] = "\0_start\0";

	struct symtab_command table  = {0};
	table.cmd = LC_SYMTAB;
	table.cmdsize = sizeof(struct symtab_command);
	table.strsize = sizeof(strings);
	table.nsyms = 1; 
	table.stroff = 0;
	
	table.symoff = 		sizeof (struct mach_header_64) +
				sizeof (struct segment_command_64) + 
				sizeof (struct section_64) + 
				sizeof (struct section_64) + 
				sizeof (struct symtab_command) + 
				byte_count + 
				data_count;

	table.stroff = table.symoff + sizeof(struct nlist_64) * 1;

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
	write(file, &dsection, sizeof(struct section_64));
	write(file, &table, sizeof table);
	write(file, bytes, byte_count);
	write(file, data, data_count);
	write(file, symbols, sizeof(struct nlist_64));
	write(file, strings, sizeof strings);

	close(file);
}

static void generate(void) {

	for (nat i = 0; i < ins_count; i++) {

		const nat op = ins[i].op;
		struct argument* const a = ins[i].arguments;

		     if (op == svc)    emit(0xD4000001);
		else if (op == emitd)  emitdata(a);
		else if (op == nop)    emit(0xD503201F);
		else if (op == cfinv)  emit(0xD500401F);

		else if (op == br)     emit(generate_br(a));
		else if (op == b_)     emit(generate_b(a, (u32) i));
		else if (op == bc)     emit(generate_bc(a, (u32) i));

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

static void generate_web(void) {
	
}

int main(int argc, const char** argv) {
	if (argc < 2) return puts("usage: assembler <file1.asm> <file2.asm> ... <filen.asm>");

	filename = argv[1];
	text_length = read_file(filename);

	*registers = (nat)(void*) malloc(65536);

	parse();

	generate_machine_code();

}












