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

#include <mach/vm_prot.h> 
#include <mach-o/loader.h>      // useful for inspecing the object file:        otool -tvVhlL object.o

typedef size_t nat;

enum instruction_type {
	nop,
	svc,
	orr,
};

struct word {
	char* name;
	nat length;
	nat ins_index;
	nat file_index;
	nat _unused_;
};

struct instruction {
	nat op;
	nat _unused_;
	nat arguments[6];
};

static nat ins_count = 0;
static struct instruction ins[4096] = {0};

static nat word_count = 0;
static struct word words[4096] = {0};

static nat byte_count = 0;
static uint8_t bytes[4096] = {0};

static nat text_length = 0;
static char* text = NULL;

static bool is(char* word, nat count, const char* this) {
	return strlen(this) == count and not strncmp(word, this, count);
}


static nat read_file(const char* filename) {
	int d = open(filename, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }

	const int file = open(filename, O_RDONLY, 0);
	if (file < 0) { read_error: perror("open"); exit(1); }

	size_t length = (size_t) lseek(file, 0, SEEK_END);
	text = malloc(length);
	lseek(file, 0, SEEK_SET);
	read(file, text, length);
	close(file); 
	return length;
}

static void emit(uint32_t x) {
	bytes[byte_count++] = (uint8_t) (x >> 0);
	bytes[byte_count++] = (uint8_t) (x >> 8);
	bytes[byte_count++] = (uint8_t) (x >> 16);
	bytes[byte_count++] = (uint8_t) (x >> 24);
}

static uint32_t generate_orr(nat* arguments) {

	return 0xD4000001;

}

static void push(nat op, nat* arguments) {

	struct instruction new = {
		.op = op, 
		.arguments = {0}
	};

	memcpy(new.arguments, arguments, sizeof new.arguments);
	ins[ins_count++] = new;
}

static void print_error(const char* reason, const char* filename, nat start_index, nat end_index) {

	nat at = 0, line = 1, column = 1;
	while (at < start_index) {
		if (text[at++] == '\n') { line++; column = 1; } else column++;
	}
	fprintf(stderr, "\033[1m%s:%lu:%lu:%lu:%lu: \033[1;31merror:\033[m \033[1m%s\033[m\n", 
			filename, start_index, end_index, line, column, reason);

	nat w = 0;
	nat b = line > 2 ? line - 2 : 0, e = line + 2;
	for (nat i = 0, l = 1, c = 1; i < text_length; i++) {
		if (c == 1 and l >= b and l <= e)  printf("\033[0m\n\033[90m%5lu\033[0m\033[32m │ \033[0m", l);
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




int main(int argc, const char** argv) {
	if (argc < 2) return puts("usage: assembler <file1.asm> <file2.asm> ... <filen.asm>");

	const char* filename = argv[1];
	text_length = read_file(filename);

	nat count = 0, start = 0;
	nat arguments[6] = {0};

	for (nat index = 0; index < text_length; index++) {
		if (not isspace(text[index])) { 
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;

		process:;
			char* const word = text + start;

			// printf("looking at: \"%.*s\"\n", (int) count, word);

			if (is(word, count, "at")) {}
			else if (is(word, count, "svc")) push(svc, arguments);
			else if (is(word, count, "nop")) push(nop, arguments);
			else if (is(word, count, "orr")) push(orr, arguments);
			else {
				// printf("user-defined label: \"%.*s\"... ignoring\n", (int) count, word);
				// labels[label_count++] = (struct label) {.name = word, .length = count};

				char reason[4096] = {0};
				snprintf(reason, sizeof reason, "undefined word found \"%.*s\"", (int) count, word);
				print_error(reason, filename, start, start + count);

				goto next;
			}

		next: count = 0;
	}

	if (count) goto process;

	for (nat i = 0; i < ins_count; i++) {

		const nat op = ins[i].op;

		if (op == svc) emit(0xD4000001);
		else if (op == nop) emit(0xD503201F);
		else if (op == orr) emit(generate_orr(ins[i].arguments));
		else {
			printf("error: unknown instruction: %lu\n", op);
			abort();
		}
	}

	printf("printing bytes: \n");
	for (nat i = 0; i < byte_count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx ", bytes[i]);
	}
	puts("");

	printf("generating program of %lu bytes...\n", byte_count);

	const char* output_filename = "program.out";
	const int flags = O_WRONLY | O_CREAT | O_TRUNC;// | O_EXCL;
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(output_filename, flags, mode);

	if (file < 0) { perror("open"); exit(1); }


	// write(file, header, header_count);
/*
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

	write(file, bytes, byte_count);




	close(file); 
}










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








