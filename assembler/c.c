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
#include <mach-o/loader.h>      

/* 
	useful for inspecing the object file: 

		otool -tvVhlL program.o

		objdump -D program.o                       <---- includes binary too!


 otool -txvVhlL hello.o

 objdump hello.o -DSast --disassembler-options=no-aliases

*/

typedef uint64_t nat;

enum instruction_type {
	nop,
	svc,
	movzx,
	movzw,
	movk,
	movn,
	addi,
	orr,

	ctzero,
	ctincr,
	ctadd,
	ctsub,
	ctmul,
	ctdiv,
	ctnor,
	ctxor,
	ctprint,
};

struct word {
	char* name;
	nat length;
	nat value;
	nat file_index;
	nat _unused_;
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

static nat registers[32] = {0};

static bool is(char* word, nat count, const char* this) {
	return strlen(this) == count and not strncmp(word, this, count);
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
			for (nat ii = 0; ii < end_index - start_index - 1; ii++) printf("~");
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

static void emit(uint32_t x) {
	bytes[byte_count++] = (uint8_t) (x >> 0);
	bytes[byte_count++] = (uint8_t) (x >> 8);
	bytes[byte_count++] = (uint8_t) (x >> 16);
	bytes[byte_count++] = (uint8_t) (x >> 24);
}

static uint32_t generate_movzx(struct argument* a) { // usage:    <imm16> <hw> <Rd> movzx
	const nat imm_index = a[0].value;
	const uint32_t hw = (uint32_t) a[1].value;
	const uint32_t Rd = (uint32_t) a[2].value;

	if (imm_index >= 31) {
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "movzx: invalid use of compiletime stackpointer register or label");
		print_error(reason, a[0].start, a[0].count); 
		exit(1);
	}

	const nat imm = registers[imm_index];

	if (imm >= 65536) {
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "movzx: immediate value must be a 16-bit integer (%llu >= 65536)", imm);
		print_error(reason, a[0].start, a[0].count); 
		exit(1);
	}

	if (hw >= 32) {
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "movzx: invalid immediate-shift-register argument: %u", hw);
		print_error(reason, a[1].start, a[1].count); 
		exit(1);
	}

	if (Rd >= 32) {
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "movzx: invalid register argument: %u", Rd);
		print_error(reason, a[2].start, a[2].count); 
		exit(1);
	}

	printf("generating MOVZX with {%llu, %u, %u}\n", imm, hw, Rd);

	return (1U << 31U) | (0xA5 << 23U) | (hw << 21U) | ((uint32_t) imm << 5U) | Rd;
}
static uint32_t generate_movk(struct argument* a) { return 0xD503201F; }
static uint32_t generate_movn(struct argument* a) { return 0xD503201F; }
static uint32_t generate_addi(struct argument* a) { return 0xD503201F; }

static void dump_hex(uint8_t* local_bytes, nat local_byte_count) {
	printf("dumping hex bytes: (%llu)\n", local_byte_count);
	for (nat i = 0; i < local_byte_count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx ", local_bytes[i]);
	}
	puts("");
}

static void execute(nat op) {
	const nat a0 = arguments[0].value;
	const nat a1 = arguments[1].value;
	const nat a2 = arguments[2].value;

	if (false) {}
	else if (op == ctincr) registers[a0]++; 
	else if (op == ctzero) registers[a0] = 0; 
	else if (op == ctadd)  registers[a2] = registers[a1] + registers[a0]; 
	else if (op == ctsub)  registers[a2] = registers[a1] - registers[a0]; 
	else if (op == ctmul)  registers[a2] = registers[a1] * registers[a0]; 
	else if (op == ctdiv)  registers[a2] = registers[a1] / registers[a0]; 
	else if (op == ctxor)  registers[a2] = registers[a1] ^ registers[a0]; 
	else if (op == ctnor)  registers[a2] = ~(registers[a1] | registers[a0]); 
	else if (op == ctprint)  printf("debug: \033[32m%llu\033[0m\n", registers[a0]); 
	
	arg_count = 0;
}

int main(int argc, const char** argv) {
	if (argc < 2) return puts("usage: assembler <file1.asm> <file2.asm> ... <filen.asm>");

	filename = argv[1];
	text_length = read_file(filename);

	nat count = 0, start = 0;

	registers[31] = (nat)(void*) malloc(65536);

	for (nat index = 0; index < text_length; index++) {
		if (not isspace(text[index])) { 
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;

	process:;
		char* const word = text + start;

		struct argument arg = { .value = 0, .start = start, .count = count };


		if (is(word, count, "at")) { }

		else if (is(word, count, "r0")) { arg.value = 0; arguments[arg_count++] = arg; }
		else if (is(word, count, "r1")) { arg.value = 1; arguments[arg_count++] = arg; }
		else if (is(word, count, "r2")) { arg.value = 2; arguments[arg_count++] = arg; }
		else if (is(word, count, "r3")) { arg.value = 3; arguments[arg_count++] = arg; }
		else if (is(word, count, "r4")) { arg.value = 4; arguments[arg_count++] = arg; }

		else if (is(word, count, "svc"))    push(svc,   start, count);
		else if (is(word, count, "nop"))    push(nop,   start, count);
		else if (is(word, count, "movzx"))  push(movzx, start, count);
		else if (is(word, count, "movk"))   push(movk,  start, count);
		else if (is(word, count, "movn"))   push(movn,  start, count);
		else if (is(word, count, "addi"))   push(addi,  start, count);

		else if (is(word, count, "ctadd"))   execute(ctadd);
		else if (is(word, count, "ctsub"))   execute(ctsub);
		else if (is(word, count, "ctmul"))   execute(ctmul);
		else if (is(word, count, "ctdiv"))   execute(ctdiv);
		else if (is(word, count, "ctnor"))   execute(ctnor);
		else if (is(word, count, "ctincr"))  execute(ctincr);
		else if (is(word, count, "ctzero"))  execute(ctzero);
		else if (is(word, count, "ctprint")) execute(ctprint);
		
		else {
			words[word_count++] = (struct word) {
				.name = word,
				.length = count,
				.value = ins_count,
				.file_index = start,
			};

			if (not arg_count) { 
				arg.value = ~word_count; 
				arguments[arg_count++] = arg; 
			}
			
			char reason[4096] = {0};
			snprintf(reason, sizeof reason, 
				"undefined word found \"%.*s\"", 
				(int) count, word
			);
			print_error(reason, start, count);
			exit(1);
		}
		count = 0;
	}

	if (count) goto process;



	for (nat i = 0; i < ins_count; i++) {

		const nat op = ins[i].op;

		     if (op == svc)   emit(0xD4000001);
		else if (op == nop)   emit(0xD503201F);
		else if (op == movzx) emit(generate_movzx(ins[i].arguments));
		else if (op == movk)  emit(generate_movk(ins[i].arguments));
		else if (op == movn)  emit(generate_movn(ins[i].arguments));
		else if (op == addi)  emit(generate_addi(ins[i].arguments));
		else {
			printf("error: unknown instruction: %llu\n", op);
			abort();
		}
	}






	const nat number_of_sections = 1;

	struct mach_header_64 header = {0};	
	struct segment_command_64 command = {0};
	struct section_64 section = {0};

	header.magic = MH_MAGIC_64;
	header.cputype = (int)CPU_TYPE_ARM | (int)CPU_ARCH_ABI64;
	header.cpusubtype = (int) CPU_SUBTYPE_ARM64_ALL;
	header.filetype = MH_OBJECT;
	header.ncmds = 1;
	header.sizeofcmds = 0;
	header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;
	
	command.cmd = LC_SEGMENT_64;
	command.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64) * number_of_sections;

	header.sizeofcmds += command.cmdsize;

	strncpy(command.segname, "__TEXT", 16);
	command.vmsize = sizeof header + sizeof command + sizeof section * number_of_sections + byte_count;
	command.vmaddr = 0;
	command.fileoff = 0;
	command.filesize = sizeof header + sizeof command + sizeof section * number_of_sections + byte_count;
	command.maxprot = (VM_PROT_READ | /*VM_PROT_WRITE | */VM_PROT_EXECUTE);
	command.initprot = (VM_PROT_READ | /*VM_PROT_WRITE | */VM_PROT_EXECUTE);
	command.nsects = number_of_sections;
	
	strncpy(section.sectname, "__text", 16);
	strncpy(section.segname, "__TEXT", 16);
	section.addr = 0;
	section.size = byte_count;
	section.offset = sizeof header + sizeof command + sizeof section * number_of_sections;
	section.align = 3;
	section.reloff = 0;
	section.nreloc = 0;

	printf("\ndebugging header bytes:\n------------------------\n");
	dump_hex((uint8_t*) &header, sizeof header);

	printf("\ndebugging command bytes:\n------------------------\n");
	dump_hex((uint8_t*) &command, sizeof command);

	printf("\ndebugging section bytes:\n------------------------\n");
	dump_hex((uint8_t*) &section, sizeof section);

	printf("\ndebugging bytes bytes:\n------------------------\n");
	dump_hex((uint8_t*) bytes, byte_count);

	
	const char* output_filename = "program.o";
	const int flags = O_WRONLY | O_CREAT | O_TRUNC;// | O_EXCL;
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(output_filename, flags, mode);

	if (file < 0) { perror("open"); exit(1); }

	printf("generating object file \"%s\" comprised of %llu bytes...\n", 
		output_filename, sizeof header + sizeof command + sizeof section + byte_count);

	write(file, &header, sizeof header);
	write(file, &command, sizeof command);
	write(file, &section, sizeof section);
	write(file, bytes, byte_count);
	close(file);
}































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
}*/



