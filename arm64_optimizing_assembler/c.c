// 202410292.021344: arm64 assembler by dwrr
// written as an experiment to replace the compiler
// with something simpler and faster hopefully.

/*
arm isa remaining to implement:

	addxr(s)  addsr(s)   subxr(s)  subsr(s) 

	adr(p)
	
	bic(s) 

	cls clz 

	andi(s)  andsr(s)  
	eonsr eorsr eori 
	ornsr orri orrsr 

	extr

	ldp ldri ldrl ldrr 
	ldrb ldrsb ldrh ldrsh ldrsw
	stp stri strr strbi strbr strhi strhr 

	rbit rev rev16 rev32

	sbfm  ubfm
	smulh  umulh


done:          (on same line means they are the same instruction!!!)

	bfm   sbfm  ubfm

	tbnz tbz
	sdiv  udiv 
	umaddl umsubl  smaddl smsubl    madd msub
	cbnz   cbz
	b.cond(16 conds)
	br   blr   ret
	b  bl
	ccmni  cmpi   cmpr   ccmnr  
	adc(s) sbc(s)
	addi(s) subi(s)
	lslv  lsrv   asrv   rorv 
	movk movn movz  
	csel csinc csinv csneg
	nop 
	svc



isa:

	nop
	svc
	mov
	csel
	do
	adc
	addi
	sh
	br
	ccmp
	if
	cbz 
	mul
	div 
	tbz
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <iso646.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

typedef uint64_t nat;

enum language_isa {
	nullins,
	size1, size2, size4, size8, 
	zero, incr, keep, inv, flags, 
	link_, regimm, return_, page, 
	up, down, rotate,
	signed_, unsigned_, 
	shift12, shift16, shift32, shift48, 
	not_, carry, negative, overflow, nev, always,
	r0,  r1,  r2,  r3,  r4,  r5,  r6,  r7, 
	r8,  r9,  r10, r11, r12, r13, r14, r15, 
	r16, r17, r18, r19, r20, r21, r22, r23, 
	r24, r25, r26, r27, r28, r29, r30, r31, 

	nop, svc, mov, csel, do_, adc, 
	addi, add, sh_, br, ccmp, if_, cbz, 
	mul, div_, tbz, bfm, adr, emit,

	at, def, loadfile, stringliteral, ignore, eoi, 
	isa_count
};

static const char* ins_spelling[isa_count] = {
	" ",
	"size1", "size2", "size4", "size8", 
	"zero", "incr", "keep", "inv", "flags", 
	"link", "regimm", "return", "page", 
	"up", "down", "rotate", 
	"signed", "unsigned",
	"shift12", "shift16", "shift32", "shift48", 

	"not", "carry", "negative", "overflow", "nev", "always",

	"r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7", 
	"r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15", 
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", 
	"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", 

	"nop", "svc", "mov", "csel", "do", "adc", 
	"addi", "add", "sh",  "br", "ccmp", "if", "cbz", 
	"mul", "div", "tbz", "bfm",  "adr", "emit", 

	"at", "def", "include", "stringliteral", "ignore", "eoi",
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

struct instruction {
	nat op;
	nat label;
	nat size;
	nat length;
	nat immediate_count;
	nat modifiers[6];
	nat registers[6];
	nat immediates[64];
};

static void get_input_string(char* string, nat max_length) {

	struct termios term = {0}, restore = {0};
	tcgetattr(0, &term);
	tcgetattr(0, &restore);
	term.c_lflag &= ~(size_t)(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &term);	
	write(1, "\033[6n", 4);
	nat row = 0, column = 0;
	scanf("\033[%llu;%lluR", &row, &column);
	nat length = 0, cursor = 0;
	printf("\033[?25l");
	while (1) {
		printf("\033[%llu;%lluH", row, column);
		for (nat i = 0; i < length; i++) {
			if (string[i] == 10) printf("\033[K");
			if (i == cursor) printf("\033[7m"); 
			putchar(string[i]);
			if (i == cursor) printf("\033[0m");
			if (string[i] == 10) printf("\033[K");
		}
		if (cursor == length) printf("\033[7m \033[0m");
		printf("\033[K");
		fflush(stdout);
		char c = 0;
		read(0, &c, 1);

		if (c == '`') { break; } 
		else if (c == '<') { if (cursor) cursor--; }
		else if (c == '>') { if (cursor < length) cursor++; }
		else if (c == 127) { 
			if (cursor and length) { 
				cursor--; length--;
				memmove(string + cursor, string + cursor + 1, length - cursor);
			}
		} else if (length < max_length - 2) {
			memmove(string + cursor + 1, string + cursor, length - cursor);
			string[cursor] = c;
			cursor++; length++;
		}
	}
	printf("\033[?25h");
	string[length] = 0;
	puts("done. got string: ");
	puts(string);
}


static void insert_byte(uint8_t** output_data, nat* output_data_count, uint8_t x) {
	*output_data = realloc(*output_data, *output_data_count + 1);
	(*output_data)[(*output_data_count)++] = x;
}

static void insert_bytes(uint8_t** d, nat* c, char* s, nat len) {
	for (nat i = 0; i < len; i++) insert_byte(d, c, (uint8_t) s[i]);
}

static void insert_u16(uint8_t** d, nat* c, uint16_t x) {
	insert_byte(d, c, (x >> 0) & 0xFF);
	insert_byte(d, c, (x >> 8) & 0xFF);
}

static void insert_u32(uint8_t** d, nat* c, uint32_t x) {
	insert_u16(d, c, (x >> 0) & 0xFFFF);
	insert_u16(d, c, (x >> 16) & 0xFFFF);
}

static void insert_u64(uint8_t** d, nat* c, uint64_t x) {
	insert_u32(d, c, (x >> 0) & 0xFFFFFFFF);
	insert_u32(d, c, (x >> 32) & 0xFFFFFFFF);
}

#define MH_MAGIC_64             0xfeedfacf
#define MH_EXECUTE              2
#define	MH_NOUNDEFS		1
#define	MH_PIE			0x200000
#define MH_DYLDLINK 		0x4
#define MH_TWOLEVEL		0x80
#define	LC_SEGMENT_64		0x19
#define LC_DYSYMTAB		0xb
#define LC_SYMTAB		0x2
#define LC_LOAD_DYLINKER	0xe
#define LC_LOAD_DYLIB		0xc
#define LC_REQ_DYLD		0x80000000
#define LC_MAIN			(0x28 | LC_REQ_DYLD)
#define LC_BUILD_VERSION 	0x32
#define LC_SOURCE_VERSION 	0x2A
#define LC_UUID            	0x1B
#define S_ATTR_PURE_INSTRUCTIONS 0x80000000
#define S_ATTR_SOME_INSTRUCTIONS 0x00000400
#define CPU_SUBTYPE_ARM64_ALL   0
#define CPU_TYPE_ARM            12
#define CPU_ARCH_ABI64          0x01000000 
#define VM_PROT_READ       	1
#define VM_PROT_WRITE   	2
#define VM_PROT_EXECUTE 	4
#define TOOL_LD			3
#define PLATFORM_MACOS 		1

enum word_types {
	type_undefined,
	type_keyword,
	type_label,
	type_immediate,
};

static const char* type_spelling[] = {
	"undefined",
	"keyword",
	"label",
	"immediate",
};

static void debug_dictionary(char** names, nat* types, nat* values, nat name_count) {
	printf("dictionary: %llu\n", name_count);
	for (nat i = 0; i < name_count; i++) 
		printf("\t%5llu  :   %10s : %-10s --->  %llu\n", i, names[i], type_spelling[types[i]], values[i]);
	puts("done printing dictionary.");
}

static void print_instructions(struct instruction* list, const nat count, char** names) {
	printf("printing %llu instructions...\n", count);
	for (nat i = 0; i < count; i++) {
		printf("ins { op=%s,.size=%s,.length=%llu,.label=%s,"
			".immediates=[%llu, %llu, %llu, %llu, %llu, %llu] } "
			".registers=[%s, %s, %s, %s, %s, %s] } "
			".modifiers=[%s, %s, %s, %s, %s, %s] } \n",
			ins_spelling[list[i].op], ins_spelling[list[i].size],  list[i].length,
			names[list[i].label], 

			list[i].immediates[0],
			list[i].immediates[1],
			list[i].immediates[2],
			list[i].immediates[3],
			list[i].immediates[4],
			list[i].immediates[5],

			ins_spelling[list[i].registers[0]], 
			ins_spelling[list[i].registers[1]], 
			ins_spelling[list[i].registers[2]], 
			ins_spelling[list[i].registers[3]], 
			ins_spelling[list[i].registers[4]], 
			ins_spelling[list[i].registers[5]], 

			ins_spelling[list[i].modifiers[0]], 
			ins_spelling[list[i].modifiers[1]], 
			ins_spelling[list[i].modifiers[2]], 
			ins_spelling[list[i].modifiers[3]], 
			ins_spelling[list[i].modifiers[4]], 
			ins_spelling[list[i].modifiers[5]]
		); 
	}
	puts("[done]");
}


static uint32_t parse_condition(nat* modifiers) {
	uint32_t 
		inv_cond = 0, 
		zero_set = 0, 
		carry_set = 0, 
		negative_set = 0, 
		overflow_set = 0, 
		nev_set = 0,
		always_set = 0
	;

	for (nat m = 0; m < 6; m++) {

		const nat k = modifiers[m];
		if (k == not_)		inv_cond = 1;
		if (k == zero) 		zero_set = 1;
		if (k == carry) 	carry_set = 1;
		if (k == negative) 	negative_set = 1;
		if (k == overflow) 	overflow_set = 1;
		if (k == nev) 		nev_set = 1;
		if (k == always) 	always_set = 1;
	}

	uint32_t cond = 0;

	if (carry_set and zero_set) cond = 4;
	else if (nev_set and zero_set) cond = 6;
	else if (zero_set) cond = 0;
	else if (carry_set) cond = 1;
	else if (negative_set) cond = 2;
	else if (overflow_set) cond = 3;
	else if (nev_set) cond = 5;
	else if (always_set) cond = 7;

	return (cond << 1) | inv_cond;
}

static nat calculate_offset(struct instruction* ins, nat here, nat target) {
	nat offset = 0;
	if (target > here) 
		for (nat i = here; i < target; i++) offset += ins[i].length;
	else  
		for (nat i = target; i < here; i++) offset -= ins[i].length;
	return offset;
}


int main(int argc, const char** argv) {

	char* names[4096] = {0};
	nat values[4096] = {0};
	nat types[4096] = {0};
	nat name_count = 0;

	struct instruction* ins = NULL;
	nat ins_count = 0;

	struct file filestack[4096] = {0};
	nat filestack_count = 1;
	const char* included_files[4096] = {0};
	nat included_file_count = 0;

	if (argc < 2 or not strcmp(argv[1], "string")) {
		char buffer[4096] = {0};
		if (argc < 2 or strcmp(argv[1], "string")) {
			puts(	"give the input string to process:\n"
				"(press '`' to terminate)\n"
			);
			get_input_string(buffer, sizeof buffer);
		} else {
			strlcpy(buffer, argv[2], sizeof buffer);
		}
		filestack[0].filename = "<top-level>";
		filestack[0].text = strdup(buffer);
		filestack[0].text_length = strlen(buffer);
		filestack[0].index = 0;
	} else {
		int file = open(argv[1], O_RDONLY);
		if (file < 0) { puts(argv[1]); perror("open"); exit(1); }
		const nat text_length = (nat) lseek(file, 0, SEEK_END);
		lseek(file, 0, SEEK_SET);
		char* text = calloc(text_length + 1, 1);
		read(file, text, text_length);
		close(file);
		filestack[0].filename = argv[1];
		filestack[0].text = text;
		filestack[0].text_length = text_length;
		filestack[0].index = 0;
	}

	for (nat i = 0; i < isa_count; i++) {
		types[name_count] = type_keyword;
		names[name_count] = strdup(ins_spelling[i]);
		values[name_count] = name_count;
		name_count++;
	}

process_file:;
	nat 	word_length = 0, word_start = 0, calling = 0, 
		in_filename = 0, in_define = 0, in_string = 0,
		register_count = 0, modifier_count = 0, immediate_count = 0;

	const nat starting_index = 	filestack[filestack_count - 1].index;
	const nat text_length = 	filestack[filestack_count - 1].text_length;
	char* text = 			filestack[filestack_count - 1].text;
	const char* filename = 		filestack[filestack_count - 1].filename;

	for (nat index = starting_index; index < text_length; index++) {
		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++; 
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;
		char* word = strndup(text + word_start, word_length);
		if (in_string) {
			if (not strcmp(word, ins_spelling[stringliteral])) {
				in_string = 0;
				ins[ins_count - 1].immediates[immediate_count++] = word_start - 1;
				ins[ins_count - 1].length = ins[ins_count - 1].immediates[1] -  
							    ins[ins_count - 1].immediates[0];
			}
			goto next_word;
		}
		if (in_filename) {
			in_filename = 0;
			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				printf("warning: %s: file already included\n", word);
				goto next_word;
			}
			included_files[included_file_count++] = word;
			int file = open(word, O_RDONLY);
			if (file < 0) { 
				printf("fatal error: loadfile %s: open: %s\n", 
					word, strerror(errno)
				); 
				exit(1); 
			}
			const nat new_text_length = (nat) lseek(file, 0, SEEK_END);
			lseek(file, 0, SEEK_SET);
			char* new_text = calloc(new_text_length + 1, 1);
			read(file, new_text, new_text_length);
			close(file);
			filestack[filestack_count - 1].index = index;
			filestack[filestack_count].filename = word;
			filestack[filestack_count].text = new_text;
			filestack[filestack_count].text_length = new_text_length;
			filestack[filestack_count++].index = 0;
			goto process_file;

		} else if (in_define == 1) {
			in_define = name_count;
			names[name_count] = word;
			types[name_count] = 0;
			name_count++;
			goto next_word;
		}

		for (nat i = 0; i < name_count; i++) 
			if (not strcmp(names[i], word)) { calling = i; goto found; }

		calling = name_count;
		names[name_count] = word;

		nat r = 0, s = 1;
		for (nat i = 0; i < strlen(word); i++) {
			if (word[i] == '1') r += s;
			else if (word[i] == '.') continue;
			else if (word[i] != '0') goto create_label;
			s <<= 1;
		}

		types[name_count] = type_immediate;
		values[name_count] = r;
		name_count++;
		goto found;
		
	create_label:
		types[name_count] = type_label;
		values[name_count] = (nat) -1;
		name_count++;

	found: 	if (in_define) {
			values[in_define] = values[calling];
			types[in_define] = types[calling];
			in_define = 0;
			goto next_word;
		}

		nat n = calling;
		const nat T = types[n];

		if (T == type_keyword) n = values[n];

		if (n == eoi) break;
		else if (n == ignore) { if (ins_count) ins_count--; }
		else if (n == loadfile) in_filename = 1;
		else if (n == def) in_define = 1;
		else if (n >= nop and n < isa_count) {
			
			register_count = 0;
			modifier_count = 0;
			immediate_count = 0;
			ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
			ins[ins_count++] = (struct instruction) {
				.op = n,
				.size = size8,
				.length = n != emit ? 4 : 0,
				.immediates = {0},
				.label = 0,
				.modifiers = {0},
				.registers = {r31, r31, r31, r31, r31, r31},
			};
			if (n == stringliteral) { 
				in_string = 1; 
				ins[ins_count - 1].immediates[immediate_count++] = index + 1;
			}

		} else if (n >= size1 and n <= size8) {
			if (not ins_count) { puts("bad fill size"); abort(); }
			ins[ins_count - 1].size = n;

		} else if (T == type_immediate) {
			if (not ins_count or immediate_count >= 64) { puts("bad fill imm"); abort(); }
			ins[ins_count - 1].immediates[immediate_count++] = values[n];
			ins[ins_count - 1].immediate_count = immediate_count;
			if (ins[ins_count - 1].op == emit) {
				if (ins[ins_count - 1].size == size8) ins[ins_count - 1].length += 8;
				if (ins[ins_count - 1].size == size4) ins[ins_count - 1].length += 4;
				if (ins[ins_count - 1].size == size2) ins[ins_count - 1].length += 2;
				if (ins[ins_count - 1].size == size1) ins[ins_count - 1].length += 1;
			}

		} else if (n >= r0 and n <= r31) {
			if (register_count >= 6) { puts("bad fill reg"); abort(); }
			ins[ins_count - 1].registers[register_count++] = n;

		} else if (n < isa_count) {
			if (modifier_count >= 6) { puts("bad fill mod"); abort(); }
			ins[ins_count - 1].modifiers[modifier_count++] = n;

		} else if (T == type_label)  {

			if (not ins_count) { 
				printf("error: unexpected label: "); puts(word);
				abort();
			}
			if (ins[ins_count - 1].label) { 
				puts("bad fill label"); 
				abort(); 
			}
			ins[ins_count - 1].label = n;
			if (ins[ins_count - 1].op == at) { values[n] = --ins_count; }

		} else { 
			puts("unknown symbol"); 
			printf("n = %llu, calling = %llu, values[n] = %llu, types[n] = %llu\n", 
				n, calling, values[n], types[n]
			);
			debug_dictionary(names, types, values, name_count);
			abort(); 
		}

		next_word: word_length = 0;
	}
	if (in_string) abort();
	if (in_define) abort();
	if (in_filename) abort();
	filestack_count--;
	if (filestack_count) goto process_file;

	debug_dictionary(names, types, values, name_count);
	print_instructions(ins, ins_count, names);

	uint8_t* my_bytes = NULL;
	nat my_count = 0;

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].op;
		const nat imm0 = ins[i].immediates[0];
		const nat imm1 = ins[i].immediates[1];
		const uint32_t Im0 = (uint32_t) imm0;
		const uint32_t Im1 = (uint32_t) imm1;
		const uint32_t sf = ins[i].size == size8;
		const uint32_t Rd = (uint32_t) (ins[i].registers[0] - r0);
		const uint32_t Rn = (uint32_t) (ins[i].registers[1] - r0);
		const uint32_t Rm = (uint32_t) (ins[i].registers[2] - r0);
		const uint32_t Ra = (uint32_t) (ins[i].registers[3] - r0);

		if (op == emit) {
			if (ins[i].size == size8) {
				for (nat t = 0; t < ins[i].immediate_count; t++) 
					insert_u64(&my_bytes, &my_count, (uint64_t) ins[i].immediates[t]); 
			} else if (ins[i].size == size4) {
				for (nat t = 0; t < ins[i].immediate_count; t++) 
					insert_u32(&my_bytes, &my_count, (uint32_t) ins[i].immediates[t]); 
			} else if (ins[i].size == size2) { 
				for (nat t = 0; t < ins[i].immediate_count; t++) 
					insert_u16(&my_bytes, &my_count, (uint16_t) ins[i].immediates[t]); 
			} else if (ins[i].size == size1) { 
				for (nat t = 0; t < ins[i].immediate_count; t++) 
					insert_byte(&my_bytes, &my_count, (uint8_t) ins[i].immediates[t]); 
			}

		} else if (op == stringliteral) {
			for (nat t = imm0; t < imm1; t++) {
				//printf("emitting: %c (%d)\n", text[t], text[t]);
				insert_byte(&my_bytes, &my_count, (uint8_t) text[t]);
			}
			//abort();
		}

		else if (op == nop) insert_u32(&my_bytes, &my_count, 0xD503201F);
		else if (op == svc) insert_u32(&my_bytes, &my_count, 0xD4000001);

		else if (op == br) {
			uint32_t l = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == link_) l = 1;
				if (k == return_) l = 2;
			}

			const uint32_t to_emit = 
				(0x6BU << 25U) | 
				(l << 21U) | 
				(0x1FU << 16U) | 
				(Rd << 5U);

			insert_u32(&my_bytes, &my_count, to_emit);
		}		

		else if (op == adc) {

			uint32_t op1 = 0xD0, op2 = 0, o1 = 0, s = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == flags) s = 1;
				if (k == inv) o1 = 1;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(s << 29U) | 
				(op1 << 21U) | 
				(Rm << 16U) | 
				(op2 << 10U) |
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == sh_) {

			uint32_t op1 = 0xD6, op2 = 8, o1 = 0, s = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == up) 		op2 = 8;
				if (k == down) 		op2 = 9;
				if (k == signed_) 	op2 = 10;
				if (k == rotate) 	op2 = 11;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(s << 29U) | 
				(op1 << 21U) | 
				(Rm << 16U) | 
				(op2 << 10U) |
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == if_) {
			const uint32_t cond = parse_condition(ins[i].modifiers);
			const nat target = values[ins[i].label];
			const nat count = calculate_offset(ins, i, target);
			const uint32_t offset = 0x7ffff & (count >> 2);

			const uint32_t to_emit = 
				(0x54U << 24U) | 
				(offset << 5U) | 
				(cond);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == adr) {

			uint32_t o1 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == page) o1 = 1;
			}

			const nat target = values[ins[i].label];
			nat count = calculate_offset(ins, i, target);
			if (o1) count /= 4096;
			const uint32_t offset = 0x1fffff & count;
			const uint32_t lo = offset & 3, hi = offset >> 2;

			const uint32_t to_emit = 
				(o1 << 31U) | 
				(lo << 29U) |
				(0x10U << 24U) | 
				(hi << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == cbz) {

			uint32_t o1 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == not_) o1 = 1;
			}

			const nat target = values[ins[i].label];
			const nat count = calculate_offset(ins, i, target);
			const uint32_t offset = 0x7ffff & (count >> 2);

			const uint32_t to_emit = 
				(sf << 31U) | 
				(0x1AU << 25U) | 
				(o1 << 24U) |
				(offset << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == tbz) {
			if (imm0 >= (1 << 6)) { puts("bad literal"); abort(); } 

			const uint32_t b40 = Im0 & 0x1F;
			const uint32_t b5 = Im0 >> 5;

			uint32_t o1 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == not_) o1 = 1;
			}

			const nat target = values[ins[i].label];
			const nat count = calculate_offset(ins, i, target);
			const uint32_t offset = 0x3fff & (count >> 2);

			const uint32_t to_emit = 
				(b5 << 31U) | 
				(0x1BU << 25U) | 
				(o1 << 24U) |
				(b40 << 19U) |
				(offset << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == ccmp) {

			uint32_t o1 = 0, r = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == inv) o1 = 1;
				if (k == regimm) r = 1;
			}

			const uint32_t cond = parse_condition(ins[i].modifiers);

			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(0x1D2 << 21U) | 
				(Rn << 16U) | 
				(cond << 12U) |
				(r << 11U) | 
				(Rd << 5U) | 
				(Rm & 0xF); 
			insert_u32(&my_bytes, &my_count, to_emit);
		}


		else if (op == addi) {
			if (imm0 >= (1 << 12)) { puts("bad literal"); abort(); } 

			uint32_t o1 = 0, s = 0, sh = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == flags) 	s = 1;
				if (k == inv) 		o1 = 1;
				if (k == shift12) 	sh = 1;
			}
			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(s << 29U) | 
				(0x22 << 23U) | 
				(sh << 22U) |
				(Im0 << 10U) |
				(Rn << 5U) | 
				(Rd);
			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == add) {
			if (imm0 >= (sf ? 64 : 32)) { puts("bad literal"); abort(); } 

			uint32_t o1 = 0, s = 0, sh = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == flags) 	s = 1;
				if (k == inv) 		o1 = 1;
				if (k == up)		sh = 0;
				if (k == down) 		sh = 1;
				if (k == signed_) 	sh = 2;
			}
			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(s << 29U) | 
				(0xB << 24U) | 
				(sh << 22U) | 
				(Rm << 16U) |
				(Im0 << 10U) | 
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == do_) {
			uint32_t l = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == link_) l = 1;
			}

			const nat target = values[ins[i].label];
			const nat count = calculate_offset(ins, i, target);
			const uint32_t offset = 0x3ffffff & (count >> 2);

			const uint32_t to_emit = 
				(l << 31U) | 
				(0x5U << 26U) | 
				(offset);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == mov) {
			if (imm0 >= (1 << 16)) { puts("bad literal"); abort(); } 

			uint32_t opc = 2, shift = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == keep) 		opc = 3;
				if (k == inv) 		opc = 0;
				if (k == shift16) 	shift = 1;
				if (k == shift32) 	shift = 2;
				if (k == shift48) 	shift = 3;
			}
			const uint32_t to_emit = 
				(sf << 31U) | 
				(opc << 29U) | 
				(0x25U << 23U) | 
				(shift << 21U) | 
				(Im0 << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		} 

		else if (op == bfm) {

			const uint32_t bit_count = Im0;
			const uint32_t position = Im1;
			const uint32_t max = (sf ? 64 : 32);

			if (not bit_count) { puts("bad field bit count"); abort(); }
			if (not position) { puts("???? bfm position zero??"); abort(); }
			if (position + bit_count > max or position >= max) {
				puts("bfm: bit field outside register width");
				abort();
			}

			uint32_t opc = 1, msb = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == signed_) 	opc = 0;
				if (k == unsigned_) 	opc = 2;
				if (k == not_) 		msb = 1;
			}

			const uint32_t N = sf;
			uint32_t imms = 0, immr = 0;
			if (not msb) {
				imms = position + bit_count - 1;
				immr = position;
			} else {
				imms = bit_count - 1;
				immr = max - position;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(opc << 29U) | 	
				(0x26U << 23U) | 
				(N << 22U) | 
				(immr << 16U) |
				(imms << 10U) |
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		} 

		else if (op == csel) {

			uint32_t o1 = 2, o2 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == incr) 		o2 = 1;
				if (k == inv) 		o1 = 1;
			}

			const uint32_t cond = parse_condition(ins[i].modifiers);

			const uint32_t to_emit = 
				(sf << 31U) | 
				(o1 << 30U) | 
				(0xD4 << 21U) | 
				(Rm << 16U) | 
				(cond << 12U) | 
				(o2 << 10U) | 
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		} 

		else if (op == mul) {

			uint32_t o0 = 0, o1 = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == inv) 		o0 = 1;
				if (k == signed_) 	o1 = 1;
				if (k == unsigned_) 	o1 = 5;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(0x1B << 24U) | 
				(o1 << 21U) |
				(Rm << 16U) |
				(o0 << 15U) | 
				(Ra << 10U) | 
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}

		else if (op == div_) {

			uint32_t op1 = 0xD6, s = 0;
			for (nat m = 0; m < 6; m++) {
				const nat k = ins[i].modifiers[m];
				if (k == unsigned_) s = 0;
				if (k == signed_) s = 1;
			}

			const uint32_t to_emit = 
				(sf << 31U) | 
				(op1 << 21U) | 
				(Rm << 16U) | 
				(1 << 11U) |
				(s << 10U) |
				(Rn << 5U) | 
				(Rd);

			insert_u32(&my_bytes, &my_count, to_emit);
		}
	}
	while (my_count % 16) insert_byte(&my_bytes, &my_count, 0);



	uint8_t* data = NULL;
	nat count = 0;	

	insert_u32(&data, &count, MH_MAGIC_64);
	insert_u32(&data, &count, CPU_TYPE_ARM | (int)CPU_ARCH_ABI64);
	insert_u32(&data, &count, CPU_SUBTYPE_ARM64_ALL);
	insert_u32(&data, &count, MH_EXECUTE);
	insert_u32(&data, &count, 11);
	insert_u32(&data, &count, 72 + (72 + 80) + 72 + 24 + 80 + 32 + 24 + 32 + 16 + 24 +  (24 + 32) ); 
	insert_u32(&data, &count, MH_NOUNDEFS | MH_PIE | MH_DYLDLINK | MH_TWOLEVEL);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'P', 'A', 'G', 'E', 'Z', 'E', 
		'R', 'O',  0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0x0000000100000000);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72 + 80);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'T', 'E', 'X', 'T',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0x0000000100000000);
	insert_u64(&data, &count, 0x0000000000004000);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 16384);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_EXECUTE);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_EXECUTE);
	insert_u32(&data, &count, 1);
	insert_u32(&data, &count, 0);
//
	insert_bytes(&data, &count, (char[]){
		'_', '_', 't', 'e', 'x', 't',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'T', 'E', 'X', 'T',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);

	insert_u64(&data, &count, 0x0000000100000000 + 16384 - my_count);
	insert_u64(&data, &count, my_count); 
	insert_u32(&data, &count, 16384 - (uint32_t) my_count);
	insert_u32(&data, &count, 4); 
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0); 
	insert_u32(&data, &count, S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS);
	insert_u32(&data, &count, 0); 
	insert_u32(&data, &count, 0); 
	insert_u32(&data, &count, 0); 
//
	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'L', 'I', 'N', 'K', 'E', 'D', 
		'I', 'T',  0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0x0000000100004000);
	insert_u64(&data, &count, 0x0000000000004000);
	insert_u64(&data, &count, 16384);
	insert_u64(&data, &count, 800);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_WRITE);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_WRITE);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_SYMTAB);
	insert_u32(&data, &count, 24); 
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_DYSYMTAB);
	insert_u32(&data, &count, 80); 
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_LOAD_DYLINKER);
	insert_u32(&data, &count, 32);
	insert_u32(&data, &count, 12);
	insert_bytes(&data, &count, (char[]){
		'/', 'u', 's', 'r', '/', 'l', 'i', 'b', 
		'/', 'd', 'y',  'l', 'd',  0,   0,   0
	}, 16);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_UUID);
	insert_u32(&data, &count, 24); 
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
//
	insert_u32(&data, &count, LC_BUILD_VERSION);
	insert_u32(&data, &count, 32);
	insert_u32(&data, &count, PLATFORM_MACOS);
	insert_u32(&data, &count, 13 << 16);
	insert_u32(&data, &count, (13 << 16) | (3 << 8));
	insert_u32(&data, &count, 1);
	insert_u32(&data, &count, TOOL_LD);
	insert_u32(&data, &count, (857 << 16) | (1 << 8));
//
	insert_u32(&data, &count, LC_SOURCE_VERSION);
	insert_u32(&data, &count, 16);
	insert_u64(&data, &count, 0);
//
	insert_u32(&data, &count, LC_MAIN);
	insert_u32(&data, &count, 24);
	insert_u64(&data, &count, 16384 - my_count);
	insert_u64(&data, &count, 0);
//
	insert_u32(&data, &count, LC_LOAD_DYLIB);
	insert_u32(&data, &count, 24 + 32);
	insert_u32(&data, &count, 24);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, (1319 << 16) | (100 << 8) | 3);
	insert_u32(&data, &count, 1 << 16);
	insert_bytes(&data, &count, (char[]){
		'/', 'u', 's', 'r', '/', 'l', 'i', 'b', 
		'/', 'l', 'i', 'b', 'S', 'y', 's', 't',
		'e', 'm', '.', 'B', '.', 'd', 'y', 'l', 
		'i', 'b',  0,   0,   0,   0,   0,   0
	}, 32);

	while (count < 16384 - my_count) insert_byte(&data, &count, 0);
	for (nat i = 0; i < my_count; i++) insert_byte(&data, &count, my_bytes[i]);
	for (nat i = 0; i < 800; i++) insert_byte(&data, &count, 0);

//	puts("");
//	puts("bytes: ");
//	for (nat i = 0; i < count; i++) {
//		if (i % 32 == 0) puts("");
//		if (data[i]) printf("\033[32;1m");
//		printf("%02hhx ", data[i]);
//		if (data[i]) printf("\033[0m");
//	}
//	puts("");

	//puts("preparing for writing out the data.");

	const bool overwrite_executable_always = true;

	if (not access("output_executable_new", F_OK)) {
		//printf("file exists. do you wish to remove the previous one? ");
		//fflush(stdout);
		if (overwrite_executable_always or getchar() == 'y') {
			puts("file was removed.");
			int r = remove("output_executable_new");
			if (r < 0) { perror("remove"); exit(1); }
		} else {
			puts("not removed");
		}
	}
	int file = open("output_executable_new", O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("could not create executable file"); exit(1); }
	int r = fchmod(file, 0777);
	if (r < 0) { perror("could not make the output file executable"); exit(1); }

	write(file, data, count);
	close(file);
	printf("wrote %llu bytes to file %s.\n", count, "output_executable_new");
	system("codesign -s - output_executable_new");


// debugging:

	//system("otool -htvxVlL output_executable_new");
	system("objdump -D output_executable_new");

}




















































































	//system("codesign -d -vvvvvvv output_executable_new");   // for debugging

		/*


		1202410196.201443:
				we need binary literals.. 


						basically the only way forwards are two paths:


						1. we have virtual regsiters, and a reg alloc, 
							and have constants just be particular const ct vr's, 

						2. we spell out binary literals as like its an instruction, and we allow us to rename them by putting the name after the binary constant. this is the simplest and cleaned approach. for the entire language. 












//if (calling >= isa_count and values[calling] < isa_count) calling = values[calling];





figuring out the names for all the conditions:


	equal
	not_equal
	carry_set
	carry_clear
	negative
	not_negative
	overflow_set
	overflow_clear

	unsigned_greater
	unsigned_less_than_or_equal_to

	greater_than_or_equal_to
	less_than

	less_than_or_equal_to
	greater_than

	always
	never
	


	//puts("printing instructions:");
	//puts("[INS_BEGIN]");
	//for (nat i = 0; i < ins_count; i++) {
	//	if (i and i % 8 == 0) puts("");
	//	if (ins[i] >> 32LLU) printf("#0x%llx  ", ins[i] ^ is_immediate);
	//	else printf("%s  ", ins_spelling[ins[i]]);
	//}
	//puts("\n[END]");


		*/









/*

	zero, carry, negative, overflow,
	sless, sgreater, ugreater, always,


	uint8_t* my_bytes = {
		//0x30, 0x00, 0x80, 0xD2, // mov x16, 1
		//0xe0, 0x01, 0x80, 0xD2, // mov x0, 15
		//0x01, 0x00, 0x00, 0xD4, // svc 0
		0x00, 0x00, 0x00, 0x00, // (padding)
	};
	const nat my_count = sizeof my_bytes;    // make sure my_count is a multiple of 16 bytes. 





printing 1 instructions...
ins { op=mov,.size=size8,.label= ,.immediate=1,.registers=[r16,  ,  ,  ,  ,  ] } .modifiers=[zero, shift0,  ,  ,  ,  ] } 
[done]
my_bytes: 

30 00 80 d2 00 00 00 00 00 00 00 00 00 00 00 00 
*/










//if (argc > 3) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));








// just put the value five at index twenty six and then put the imm two six colon zero zero. ie, the immediate offset is encoded as the divide by four version, ithink. so yeah. 

			//printf("ins[i].label = %llu\n", ins[i].label);
			//printf("i = %llu\n", i);
			//printf("target = %llu\n", target);









	/*
	puts("generated machine code: ");
	for (nat i = 0; i < my_count; i++) {
		if (i % 8 == 0) puts("");
		printf("%02hhx ", my_bytes[i]);
	}
	puts("");
	*/



	//const nat multiples = my_count / 16;

	



/*
















csel r3 r4 r5 inv incr
mov r16 1




*/
	//exit(1);





