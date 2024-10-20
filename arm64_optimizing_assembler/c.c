/*
written on 1202410196.164450 dwrr    
an optimizing assembler that uses the 
arm64 (or machine code) isa as the ir 
for the optimization process.

----------------------------------------

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

typedef uint64_t nat;

enum language_isa {
	nullins,
	nop, svc, 
	mov, csel,
	lf, eoi, 

	isa_count
};

static const char* ins_spelling[isa_count] = {
	"_null_", 
	"nop", "svc", 
	"mov", "csel",
	"lf", "eoi", 
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

struct instruction {
	nat args[14];
	nat arg_count;
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
		else if (c == ',') { if (cursor) cursor--; }
		else if (c == '.') { if (cursor < length) cursor++; }
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
#define TOOL_LD	3
#define PLATFORM_MACOS 1



static void print_machine_instructions(struct instruction* mis, const nat mi_count) {
	printf("printing %llu machine instructions...\n", mi_count);
	for (nat i = 0; i < mi_count; i++) {
		printf("machine instruction {.op = %3llu (\"%s\"), .args = (%3llu)[%3llu, %3llu, %3llu, %3llu] }\n", 
			mis[i].op, ins_spelling[mis[i].op],
			mis[i].arg_count, 
			mis[i].args[0],mis[i].args[1],mis[i].args[2],mis[i].args[3]
		); 
	}
	puts("[done]");
}



int main(int argc, const char** argv) {

	if (argc > 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));

	char* names[4096] = {0};
	nat values[4096] = {0};
	bool active[4096] = {0};
	nat name_count = 0;

	nat* ins = NULL;
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
		active[name_count] = 1;
		names[name_count] = strdup(ins_spelling[i]);
		values[name_count++] = i;
	}

	nat calling = 0, previous_call = 0;


process_file:;
	nat word_length = 0, word_start = 0;

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
		for (nat i = 0; i < name_count; i++) {
			if (not strcmp(names[i], word)) { 
				calling = values[i]; 
				goto found; 
			}
		}
		if (previous_call >= 0 and previous_call <= r31);


















				we need binary literals.. 


						basically the only way forwards are two paths:


						1. we have virtual regsiters, and a reg alloc, 
							and have constants just be particular const ct vr's, 

						2. we spell out binary literals as like its an instruction, and we allow us to rename them by putting the name after the binary constant. this is the simplest and cleaned approach. for the entire language. 

















		calling = previous_call;
		active[name_count] = 1;
		names[name_count] = word;
		values[name_count++] = previous_call;
		goto next_word;
	found:	previous_call = calling;
		if (calling == lf) {
			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				printf("warning: %s: file already included\n", word);
				goto next_word;
			}
			included_files[included_file_count++] = "file.txt";

			int file = open("file.txt", O_RDONLY);
			if (file < 0) { puts(word); perror("open"); exit(1); }
			const nat new_text_length = (nat) lseek(file, 0, SEEK_END);
			lseek(file, 0, SEEK_SET);
			char* new_text = calloc(new_text_length + 1, 1);
			read(file, new_text, new_text_length);
			close(file);
			filestack[filestack_count - 1].index = index;
			filestack[filestack_count].filename = "file.txt";
			filestack[filestack_count].text = new_text;
			filestack[filestack_count].text_length = new_text_length;
			filestack[filestack_count++].index = 0;
			goto process_file;

		} else if (calling == eoi) break;
		else {
			printf("regular calling = %llu\n", calling);
			ins = realloc(ins, sizeof(nat) * (ins_count + 1));
			ins[ins_count++] = calling;
		}
		next_word: word_length = 0;
	}

	filestack_count--;
	if (filestack_count) goto process_file;


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
	//system("codesign -d -vvvvvvv output_executable_new");   // for debugging



	exit(1);

}





