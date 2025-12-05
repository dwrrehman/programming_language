/*
	an assembler for the msp430 arch. 
	used for programming msp430frxxxx 
	chips that i have.
	written on 1202409043.013935 by dwrr.
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <iso646.h>
#include <time.h>
#include <sys/time.h>

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t byte;

struct section {
	byte* data;
	nat length;
	nat address;
};

struct instruction {
	nat opcode;
	nat type;

	nat dest_reg;
	nat source_reg;

	nat dest_mode;
	nat source_mode;

	nat dest_imm;
	nat source_imm;
};

enum language_instructions {
	undefined_ins,
	eof, setoutputname, def, 
	section_start, literal_byte, literal_word,
	mov, add, addc, sub, subc, cmp, dadd, bit, bic, bis, xor_, and_, branch, at, 
	pc, sp, sr, cg, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, 
	r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31,
	size_word, size_byte, size_address, 
	nonzero, zero, nocarry, carry, negative, greaterequal, less, always,
	direct, indexed, indirect, autoincr,
	isa_count
};

static const char* spelling[isa_count] = {
	"__[undefined]__",
	"eof", "setoutputname", "def", 
	"section", "literalbyte", "literalword", 
	"mov", "add", "addc", "sub", "subc", "cmp", "dadd", "bit", "bic", "bis", "xor", "and", "branch", "at", 
	"pc", "sp", "sr", "cg", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", 
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
	"word", "byte", "address", 
	"nonzero", "zero", "nocarry", "carry", "negative", "greaterequal", "less", "always",
	"direct", "index", "deref", "incr",
};

static void debug_sections(struct section* sections, const nat section_count) {
	printf("printing %llu sections: \n", section_count);
	for (nat s = 0; s < section_count; s++) {
		printf("section #%llu: .address = 0x%04llx, .length = %llu :: ", s, sections[s].address, sections[s].length);
		for (nat n = 0; n < sections[s].length; n++) {
			if (n % 16 == 0) printf("\n\t");
			printf("[%02hhx] ", sections[s].data[n]);
		}
		puts("[end of section]");
	}
	puts("[done]");
}

static void print_instructions(struct instruction* ins, nat ins_count) {
	puts("printing instructions");
	for (nat i = 0; i < ins_count; i++) {
		printf("%llu: { .opcode = %s : .dest=%llu, .src=%llu     .destm=%llu, .srcm=%llu     .desti=%llu, .srci=%llu  }\n", 
			i, spelling[ins[i].opcode], 
			ins[i].dest_reg, ins[i].source_reg, 
			ins[i].dest_mode, ins[i].source_mode, 
			ins[i].dest_imm, ins[i].source_imm
		);
	}
	puts("done");
}

static void write_string(const char* directory, char* w_string, nat w_length, nat should_set_output_name) {
        char name[4096] = {0};
        srand((unsigned)time(0)); rand();
        char datetime[32] = {0};
        struct timeval t = {0};
        gettimeofday(&t, NULL);
        struct tm* tm = localtime(&t.tv_sec);
        strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm);
        snprintf(name, sizeof name, "%s%s_%08x%08x.txt", directory, datetime, rand(), rand());
        int flags = O_WRONLY | O_TRUNC | O_CREAT | (should_set_output_name ? 0 : O_EXCL);
        mode_t permission = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int file = open(should_set_output_name ? "output_machine_code.txt" : name, flags, permission);
        if (file < 0) { perror("save: open file"); puts(name); getchar(); }
        write(file, w_string, w_length);
        close(file);
	printf("write_string: successfully wrote out %llu bytes to file %s.\n", 
		w_length, should_set_output_name ? "output_machine_code.txt" : name
	);
}

int main(int argc, const char** argv) {
	if (argc <= 1) return puts("usage: ./assemble <file.s>");
	const char* filename = argv[1];
	int file = open(filename, O_RDONLY);
	if (file < 0) { perror("open"); printf("error: could not open: %s\n", filename); exit(1); }
	const nat text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(text_length + 1, 1);
	read(file, text, text_length);
	close(file);

	puts("printing the text: ");
	printf("text %p @ %llu chars...\n", (void*) text, text_length);

	puts("file contents:");
	puts("------------------------");
	puts(text);
	puts("------------------------");

	struct instruction ins[4096] = {0};
	nat ins_count = 0;
	nat word_length = 0, word_start = 0, arg = 0, ignoring = 0, should_set_output_name = 0;

	for (nat index = 0; index < text_length; index++) {
		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++;
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;
		char* word = strndup(text + word_start, word_length);
		printf("start: %llu, count: %llu  |   word = %s\n", word_start, word_length, word);
		if (not strcmp(word, "comment")) {
			ignoring = not ignoring;
			if (not ignoring) goto next;
		} 
		if (ignoring) goto next;
		nat n = 0;
		for (nat i = 0; i < isa_count; i++) {
			if (not strcmp(word, spelling[i])) { n = i; goto found; }
		}

		if (*word == '0' or *word == '1') {
			nat r = 0; 
			nat s = 1;
			for (nat i = 0; i < word_length; i++) {
				if (word[i] == '0') s <<= 1;
				else if (word[i] == '1') { r += s; s <<= 1; }
				else if (word[i] == '_') {}
				else goto binary_error;
			}

			printf("info: found binary constant literal = %08llx\n", r);
			if (not ins_count) { puts("usage error"); abort(); }
			if (arg < 2) ins[ins_count - 1].dest_imm = r;
			else ins[ins_count - 1].source_imm = r;
			goto next;
		binary_error:
			puts("binary number error");
			puts("error: unknown binary number encountered");
			printf("index = %llu\n", index);
			puts(word);
			abort();
		}
		puts("error: unknown word encountered!!...");
		printf("index = %llu\n", index);
		puts(word);
		abort();
	found:
		if (n == eof) { puts("found eof"); break; }
		else if (n == setoutputname) should_set_output_name = 1;
		else if (n >= section_start and n <= at) {
			ins[ins_count++] = (struct instruction) {.opcode = n};
			arg = 0;

		} else if (n >= pc and n <= r31) {
			const nat r = n - pc;
			if (not ins_count) { puts("usage error"); abort(); }
			if (arg == 0) ins[ins_count - 1].dest_reg = r;
			else if (arg == 1) ins[ins_count - 1].source_reg = r;
			else { puts("usage error"); abort(); }
			arg++;

		} else if (n >= direct and n <= autoincr) {
			const nat r = n - direct;
			if (not ins_count) { puts("usage error"); abort(); }
			if (arg < 2) ins[ins_count - 1].dest_mode = r;
			else ins[ins_count - 1].source_mode = r;

		} else if (n >= size_word and n <= always) {
			if (not ins_count) { puts("usage error"); abort(); }
			ins[ins_count - 1].type = n;

		} else {
			puts("error: unimpl ins");
			puts(word);
			abort();	
		}
	next:	word_length = 0;
	}

	print_instructions(ins, ins_count);
	struct section sections[128] = {0};
	nat section_count = 0;
	nat labels[32] = {0};

	for (nat pass = 0; pass < 2; pass++) {
		if (pass) {
			for (nat s = 0; s < section_count; s++) 
				free(sections[s].data);
			section_count = 0;
		}
	for (nat i = 0; i < ins_count; i++) {
		printf("ins: %llu: \n", i);
		struct instruction this = ins[i];
		const nat op = this.opcode;
		const nat type = this.type;
		struct section* section = sections + section_count - 1;

		if (op == section_start) {
			printf("new section starts at: 0x%08llx\n", this.dest_imm);
			sections[section_count++] = (struct section) { .data = calloc(65536, 1), .address = this.dest_imm, .length = 0 };

		} else if (op == literal_byte) {
			section->data[section->length++] = (this.dest_imm) & 0xFF;

		} else if (op == literal_word) {
			const u16 word = (u16) this.dest_imm;
			section->data[section->length++] = (word >> 0) & 0xFF;
			section->data[section->length++] = (word >> 8) & 0xFF;

		} else if (op >= mov and op <= and_) {

			if (not section_count) { puts("error: no section given for instruction"); abort(); }
			printf("generating double operand instruction : %s\n", spelling[op]);

			if (type == size_address) abort();
			if (this.dest_reg >= 16) abort();
			if (this.source_reg >= 16) abort();
			if (this.dest_mode >= 2) abort();
			if (this.source_mode >= 4) abort();

			u16 word = (u16) (
				((op - mov + 4) << 12) | 
				(this.source_reg << 8) | 
				(this.dest_mode << 7) | 
				((nat)(type == size_byte) << 6) | 
				(this.source_mode << 4) | 
				(this.dest_reg)
			);

			section->data[section->length++] = (word >> 0) & 0xFF;
			section->data[section->length++] = (word >> 8) & 0xFF;
			printf("generating word = 0x%04hx\n", word);

			if (
				(this.source_mode == 1 and this.source_reg != 2 and this.source_reg != 3) or
				(this.source_mode == 3 and not this.source_reg)
			) {
				word = (u16) this.source_imm;
				section->data[section->length++] = (word >> 0) & 0xFF;
				section->data[section->length++] = (word >> 8) & 0xFF;
				printf("generating source imm word = 0x%04hx\n", word);
			}

			if (this.dest_mode == 1) {
				word = (u16) this.dest_imm;
				section->data[section->length++] = (word >> 0) & 0xFF;
				section->data[section->length++] = (word >> 8) & 0xFF;
				printf("generating dest imm word = 0x%04hx\n", word);
			}

		} else if (op == branch) {

			printf("generating branch...\n");
			const nat offset = ((labels[this.dest_reg] - (section->length + 2)) / 2);
			if ((int) offset <= -500 or (int) offset >= 500) { printf("error: offset too large in branch: %lld\n", offset); abort(); }
			const u16 word = (u16) ((0x1 << 13) | ((type - nonzero) << 10) | (offset & 0x3FF));
			section->data[section->length++] = (word >> 0) & 0xFF;
			section->data[section->length++] = (word >> 8) & 0xFF;

		} else if (op == at) {
			puts("executing at directive...");
			labels[this.dest_reg] = section->length;
		} else {
			printf("unknown instruction to generate: op = %s\n", spelling[op]);
			getchar();
		}
	}
	}

	char out[4096] = {0};
	int len = 0;
	debug_sections(sections, section_count);

	for (nat s = 0; s < section_count; s++) {
		len += snprintf(out + len, sizeof out, "@%04llx", sections[s].address);
		for (nat n = 0; n < sections[s].length; n++) {
			if (n % 16 == 0) len += snprintf(out + len, sizeof out, "\n");
			len += snprintf(out + len, sizeof out, "%02hhX ", sections[s].data[n]);
		}
		len += snprintf(out + len, sizeof out, "\n");
	}
	len += snprintf(out + len, sizeof out, "q\n");
	printf("about to write out: \n-------------------\n<<<%.*s>>>\n----------------\n", len, out);
	printf("write out machine code to file? (y/n) ");
	fflush(stdout);
	write_string("./", out, (nat) len, should_set_output_name);
}
















































//const int c = getchar();
	//if (c == 'y') {
	//	write_string("./", out, (nat) len, should_set_output_name);
	//} else {
	//	puts("not written");
	//}

/*





ti txt   output hex file format that we are using:





@e000
21 83 B2 40 80 5A 20 01 F2 D3 22 00 F2 E3 21 00 
B1 40 10 27 00 00 91 83 00 00 81 93 00 00 FB 23 
F5 3F 31 40 00 04 B2 40 48 E0 00 02 B2 40 48 E0 
02 02 B0 12 40 E0 0C 43 B0 12 00 E0 B0 12 44 E0 
1C 43 30 41 03 43 FF 3F 30 41 
@fffe
22 E0 
q



*/





// syntax:          add indirect r12 autoincr sr


/*
static nat hex_to_nat(const char* string, nat length) {

	nat result = 0; 
	nat power = 1LLU << (4LLU * length);

	for (nat i = 0; i < length; i++) {
		const char c = (char) tolower(string[i]);
		nat n = 0;

		if (isdigit(c)) n = (nat) (c - '0');
		else if (isalpha(c)) n = (nat) (c - 'a' + 10);
		else abort();

		power >>= 4;
		result += power * n;
	}
	return result;
}
*/
















/*
	for (nat s = 0; s < section_count; s++) {
		const struct section this = sections[s];
		printf("dissassembly of section #%llu: [address=0x%08hx], [length=%llu]\n", s, this.address, this.length);

		for (nat i = 0; i < this.length; i += 2) {
			const u16 ins = (u16) (((u16)this.data[i]) | ((u16)this.data[i + 1] << 8U));
			printf("%08llx:   %04hx ", this.address + i, ins);

			const byte op = ins >> 12;
			const byte ob = ins >> 13;

			if (op == 0x0) puts("\t\t\t\textended ins: MOVA, CMPA, ADDA, SUBA, RRCM, RRAM, RLAM, RRUM");
			else if (op == 0x1 and cd == 0x10) puts("RRC, RRC.B SWP.B RRA, RRA.B SXT  PUSH  PUSH.B CALL RETI CALLA ");
			else if (op == 0x1 and cd == 0x14) puts("PUSHM.A POPM.A PUSHM.W POPM.W ");
			else if (op == 0x1 and (cd == 0x18 or cd == 0x1C)) puts("extension words format I and II.");
			else if (op == 0x1) puts("\t\t\t\tUNKNOWN extension instruction...");
			

			else if (ob == 0x1) {
				const u16 cd = (ins >> 10) & 0x7;
				const u16 sign = (ins >> 9) & 0x1;
				const u16 offset = (ins >> 0) & 0x1FF;

				const char* name[] = {
					"JNE/JNZ",
					"JEQ/JZ",
					"JNC",
					"JC",
					"JN",
					"JGE",
					"JL",
					"JMP",
				};
				printf("     ");
				printf("     ");
				printf("\t%s\t[s=%hx] [offset=%hx]\n", name[cd], sign, offset);
			}
			else if (op >= 0x4 and op <= 0xF) {

				const char* names[] = {
					"MOV", 
					"ADD", "ADDC",
					"SUB", "SUBC",
					"CMP",
					"DADD",
					"BIT", "BIC", "BIS",
					"XOR", "AND",
				};


				const char* R[] = {
					"PC", "SP", "SR", "CG2", 
					"R4", "R5", "R6", "R7", 
					"R8", "R9", "R10", "R11", 
					"R12", "R13", "R14", "R15", 
				};


				const u16 source = (ins >> 8) & 0xF;
				const u16 dest = (ins >> 0) & 0xF;
				const u16 As = (ins >> 4) & 0x3;
				const u16 Ad = (ins >> 7) & 0x1;
				const u16 BW = (ins >> 6) & 0x1;

				if (As == 0) {
					printf("     ");
					if (Ad) {
						i += 2;
						const u16 imm = (u16) (((u16)this.data[i]) | ((u16)this.data[i + 1] << 8U));
						printf("%04hx ", imm);
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("(%s + 0x%04hx) <-- %s", R[dest], imm, R[source]);
					} else { 
						printf("     ");
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("%s <-- %s", R[dest], R[source]); 
					}

				} else if (As == 1) {
					i += 2;
					const u16 source_imm = (u16) (((u16)this.data[i]) | ((u16)this.data[i + 1] << 8U));
					printf("%04hx ", source_imm);
					if (Ad) {
						i += 2;
						const u16 imm = (u16) (((u16)this.data[i]) | ((u16)this.data[i + 1] << 8U));
						printf("%04hx ", imm);
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("(%s + 0x%04hx) <-- (%s + 0x%04hx)", R[dest], imm, R[source], source_imm);
					} else {
						printf("     ");
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("%s <-- (%s + 0x%04hx)", R[dest], R[source], source_imm);
					}

				} else if (As == 2) {
					printf("     ");
					if (Ad) {
						i += 2;
						const u16 imm = (u16) (((u16)this.data[i]) | ((u16)this.data[i + 1] << 8U));
						printf("%04hx ", imm);
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("(%s + 0x%04hx) <-- *(%s)", R[dest], imm, R[source]);
					} else {
						printf("     ");
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("%s <-- *(%s)", R[dest], R[source]);
					}

				} else if (As == 3 and source) {
					printf("     ");
					if (Ad) {
						i += 2;
						const u16 imm = (u16) (((u16)this.data[i]) | ((u16)this.data[i + 1] << 8U));
						printf("%04hx ", imm);
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("(%s + 0x%04hx) <-- *(%s), %s++", R[dest], imm, R[source], R[source]);
					} else {
						printf("     ");
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("%s <-- *(%s), %s++", R[dest], R[source], R[source]);
					}

				} else if (As == 3 and not source) {
					i += 2;
					const u16 source_imm = (u16) (((u16)this.data[i]) | ((u16)this.data[i + 1] << 8U));
					printf("%04hx ", source_imm);
					if (Ad) {
						i += 2;
						const u16 imm = (u16) (((u16)this.data[i]) | ((u16)this.data[i + 1] << 8U));
						printf("%04hx ", imm);
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("(%s + 0x%04hx) <-- #0x%hx", R[dest], imm, source_imm);
					} else {
						printf("     ");
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("%s <-- #0x%hx", R[dest], source_imm);
					}
				} 
				//printf("    [As=%hu] [Ad=%hu]\n", As, Ad);
				puts("");

			} else printf("\t\t\t\t[error: undefined op code %hhu]\n", op);
			puts("");
		}
	}
*/






		/*if (text[i] == 'q') {
			puts("found terminating character, q.");
			break;
		}
		if (text[i] == '@') {
			section_count++;
			i++;
			const nat n = hex_to_nat(text + i, 4);
			i += 4;
			sections[section_count - 1].address = (uint16_t) n;
			printf("set address of section #%llu to be %llu...\n", section_count - 1, n);
			continue;
		} 

		if (isdigit(text[i]) or isalpha(text[i])) {
			const nat b = hex_to_nat(text + i, 2);
			i += 2;
			sections[section_count - 1].data = realloc(sections[section_count - 1].data, sections[section_count - 1].length + 1);
			sections[section_count - 1].data[sections[section_count - 1].length++] = (byte) b;
			//printf("pushed byte %llu to section #%llu...\n", b, section_count - 1);
			continue;
		}
		if (isspace(text[i])) {
			printf("FOUND %d...\n", text[i]);
			continue;
		}
		abort();*/



/*

			else if (op == 0x5) {
				puts("ADD");
			}

			else if (op == 0x6) {
				puts("ADDC");
			}

			else if (op == 0x7) puts("SUBC");
			else if (op == 0x8) puts("SUB");
			else if (op == 0x9) puts("CMP");
			else if (op == 0xA) puts("DADD");
			else if (op == 0xB) puts("BIT");
			else if (op == 0xC) puts("BIC");
			else if (op == 0xD) puts("BIS");
			else if (op == 0xE) puts("XOR");
			else if (op == 0xF) puts("AND");

jump instructions:
----------------------

15 	13 opcode
12 	10 cd
9 	s
8 0 	signed pc offset




               000  thru  3C0



0	JNE    20xx      0010 0000

1	JEQ    24xx      0010 0100

2	JNC    28xx      0010 1000

3	JC     2Cxx      0010 1100

4	JN     30xx      0011 0000

5	JGE    34xx      0011 0100

6	JL     38xx      0011 1000

7	JMP    3Cxx      0011 1100




   27C0
 

0010 0111 1100 0000

0010 0111 1100 0000

0010 0100 0000 0000

[001] [0 01]   [00 0000 0000]



000 040 080 0C0 100 140 180 1C0 200 240 280 2C0 300 340 380 3C0
*/


























