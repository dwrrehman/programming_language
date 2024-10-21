/*
written on 1202410196.164450 dwrr    
an optimizing assembler that uses the 
arm64 (or machine code) isa as the ir 
for the optimization process.

----------------------------------------



1202410207.191257
i finally got this code working!!

totally overhauled the way i am handling the dictionary lol.. i think it should work well now, i think!!



def hello 0001
def bubbles hello

mov r3 bubbles
def myvar r5

mov myvar hello





1202410207.002512
current state:

		so i just realized why the imm    macro   system  

		ie,    def hello 01      mov zero r0 hello


			isnt working. 


	its because 


			we need to actually differentiate what the type of the elements in our dictionary is. 

				like, the root of the difficulty is that we need to be able to represent a 64bit literal,

									(i think...!?!?!)


				buttt of course we don't have any way to tell apart an element in our dict that is a reference to another machinery, or a literal value of that value in the dict. 

							so yeah, we need to distinguish these.  AT LEAST. 



	buttt tbh theres probably a muchhh better way to do this whole   immediate thing altogether lol.   probablyyy involving making everything derived computationally and not even providing a way to write an immediate lol. 




	tbh yeah. thats probably it lol.  nice okay. coool.  letst just rip out the imm system then lol. yay











							CRAP  wait i just realized we can't rip out the literal system, because we would neeed 


							virtual registers. 


								like, of some sort. 

												DANGGG


				hmm i mean i guess we couldd just give a set of virtual registers too lollll like that coulddd worklol 

					hmmm interesting 


								i think i would probablyyy just want to just have literals though, i think instead of adding ct virtual registers. yeah probably.  



	okay cool so we have to figure out this whole literal situation, i think lol. shouldnt be thatttt bad, but yeah we need to sort it out. 




hmm maybe we make a dictionary entry for the literal!


		i think we still need a flag set to tell that that entry is a literal though... hmmmm crap 


									uhh



				hmmmmm  just i don't think theres any way around this, we need to annotate the type basically, inside the symbol table 

							at the very least, whether its a label, immediate, or regsiter/instruction.  


				i think we need those at leastttt    
						no other types other than those though lol. so yeah thats good i guess


		hmmmm



					i kinda wish the registers / instruction opcodes were seperate though hmmm idk 























arm isa that will be supported:

			adc(s) addxr(s) addi(s) addsr(s)  adr(p)   
			andi(s)  andsr(s)  asrv
			b  bl   b.cond(16 conds)   br   blr
			bfm bic(s) 
			cbnz   cbz   ccmni  ccmnr  cmpi   cmpr     cfinv cls clz 
			csel csinc csinv csneg
			eonsr eorsr eori  extr
			ldp ldri ldrl ldrr 
			ldrb ldrsb ldrh ldrsh ldrsw
			lslv   madd msub
			movk movn movz  nop
			ornsr orri orrsr
			rbit rev rev16 rev32
			ret   rorv sbc(s)  sbfm 
			sdiv smaddl smsubl smulh 
			stp stri strr strbi strbr strhi strhr 
			subxr(s) subi(s) subsr(s) 
			svc    tbnz tbz    ubfm
			udiv umaddl umsubl umulh



our previous isa:


	zero, incr, decr, set, add, sub, 
	mul, muh, mhs, div_, dvs, rem, rms, 
	si, sd, sds, and_, or_, eor, not_,
	ld, st, sta, bca, sc, at, lf, def, udf, 
	lt, ge, lts, ges, ne, eq, do_, 

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


/*


isa:

	nop

	svc



	mov type=z(default)/k/n shift=00 imm16=00000..0  Rd=00000  sf=1(default)/0

		mov [size8(default)/size4] [zero(default)/keep/inv]   Rd_bn    [imm16_bn default=0]
		


	csel incr=0 inv=0 Rn=00000 Rm=00000 Rd=00000 cond=0000 sf=1(default)/0


		csel [size8(default)/size4] [incr] [inv]  [cond]   Rd_bn    Rm_bn   Rn_bn




	lf file.txt

	eoi




	mov r16  101000 101011  010100110

	





*/

enum language_isa {
	nullins,
	size1, size2, size4, size8, 
	zero, incr, keep, inv, _true, _false, 
	carry, negative, overflow,
	sless, sgreater, ugreater, always,
	r0,  r1,  r2,  r3,  r4,  r5,  r6,  r7, 
	r8,  r9,  r10, r11, r12, r13, r14, r15, 
	r16, r17, r18, r19, r20, r21, r22, r23, 
	r24, r25, r26, r27, r28, r29, r30, r31, 
	nop, svc, 
	mov, csel,
	at, def, loadfile, eoi, 
	isa_count
};

static const char* ins_spelling[isa_count] = {
	" ",
	"size1", "size2", "size4", "size8", 
	"zero", "incr", "keep", "inv", "true", "false", 
	"carry", "negative", "overflow",
	"sless", "sgreater", "ugreater", "always",
	"r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7", 
	"r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15", 
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", 
	"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", 
	"nop", "svc", 
	"mov", "csel",
	"at", "def", "include", "eoi",
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

struct instruction {
	nat modifiers[6];
	nat registers[6];
	nat immediate;
	nat label;
	nat size;
	nat op;
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
		printf("ins { op=%s,.size=%s,.label=%s,.immediate=%llu,"
			".registers=[%s, %s, %s, %s, %s, %s] } "
			".modifiers=[%s, %s, %s, %s, %s, %s] } \n",
			ins_spelling[list[i].op], ins_spelling[list[i].size], 
			names[list[i].label], list[i].immediate,
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

int main(int argc, const char** argv) {

	if (argc > 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));

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

	if (argc < 2) {
		char buffer[4096] = {0};
		puts(	"give the input string to process:\n"
			"(press '`' to terminate)\n"
		);
		get_input_string(buffer, sizeof buffer);
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
		in_filename = 0, in_define = 0,
		register_count = 0, modifier_count = 0;

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
		if (in_filename) {
			in_filename = 0;
			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				printf("warning: %s: file already included\n", word);
				goto next_word;
			}
			included_files[included_file_count++] = word;
			int file = open(word, O_RDONLY);
			if (file < 0) { printf("fatal error: loadfile %s: open: %s\n", word, strerror(errno)); exit(1); }
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
		else if (n == loadfile) in_filename = 1;
		else if (n == def) in_define = 1;
		else if (n >= nop and n < isa_count) {
			register_count = 0;
			modifier_count = 0;
			ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
			ins[ins_count++] = (struct instruction) {
				.op = n,
				.size = size8,
				.immediate = 0,
				.label = 0,
				.modifiers = {0},
				.registers = {0},
			};

		} else if (T == type_immediate) {
			if (not ins_count or ins[ins_count - 1].immediate) { puts("bad fill imm"); abort(); }
			ins[ins_count - 1].immediate = values[n];

		} else if (n >= size1 and n <= size8) {
			if (not ins_count) { puts("bad fill size"); abort(); }
			ins[ins_count - 1].size = n;

		} else if (n >= r0 and n <= r31) {
			if (register_count >= 6) { puts("bad fill reg"); abort(); }
			ins[ins_count - 1].registers[register_count++] = n;

		} else if (n < isa_count) {
			if (modifier_count >= 6) { puts("bad fill mod"); abort(); }
			ins[ins_count - 1].modifiers[modifier_count++] = n;

		} else if (T == type_label)  {
			if (not ins_count or ins[ins_count - 1].label) { puts("bad fill label"); abort(); }
			ins[ins_count - 1].label = n;

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
	filestack_count--;
	if (filestack_count) goto process_file;

	debug_dictionary(names, types, values, name_count);

	print_instructions(ins, ins_count, names);



/*


def hello 100011100
at loop
        mov zero loop r0 11010
	mov zero loop r0 hello



def hello 100011100
mov zero r0 hello

*/




	exit(1);

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

	insert_u64(&data, &count, 0x0000000100000000 + 16384 - 16);
	insert_u64(&data, &count, 16); 
	insert_u32(&data, &count, 16384 - 16);
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
	insert_u64(&data, &count, 16384 - 16);
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

	const uint8_t my_bytes[] = {

		0x30, 0x00, 0x80, 0xD2, // mov x16, 1
		0xe0, 0x01, 0x80, 0xD2, // mov x0, 15
		0x01, 0x00, 0x00, 0xD4, // svc 0

		0x00, 0x00, 0x00, 0x00, // (padding)
	};
	const nat my_count = sizeof my_bytes;
	while (count < 16384 - my_count) insert_byte(&data, &count, 0);
	for (nat i = 0; i < my_count; i++) insert_byte(&data, &count, my_bytes[i]);
	for (nat i = 0; i < 800; i++) insert_byte(&data, &count, 0);

	puts("");
	puts("bytes: ");
	for (nat i = 0; i < count; i++) {
		if (i % 32 == 0) puts("");
		if (data[i]) printf("\033[32;1m");
		printf("%02hhx ", data[i]);
		if (data[i]) printf("\033[0m");
	}
	puts("");

	puts("preparing for writing out the data.");

	if (not access("output_executable_new", F_OK)) {
		printf("file exists. do you wish to remove the previous one? ");
		fflush(stdout);
		int c = getchar();
		if (c == 'y') {
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

	exit(1);

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




