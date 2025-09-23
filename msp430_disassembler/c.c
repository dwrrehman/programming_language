/*
	a disassmbler for the msp430 arch. 
	used for programming msp430frxxxx 
	chips that i have.

	written on 1202408106.143226  by dwrr 
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <iso646.h>

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t byte;

struct section {
	byte* data;
	nat length;
	u16 address;
	u16 padding;
	u32 padding2;
};

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

static void debug_sections(struct section* sections, const nat section_count) {
	printf("printing %llu sections: \n", section_count);
	for (nat s = 0; s < section_count; s++) {
		printf("section #%llu: .address = 0x%04hx, .length = %llu :: ", s, sections[s].address, sections[s].length);
		for (nat n = 0; n < sections[s].length; n++) {
			if (n % 16 == 0) printf("\n\t");
			printf("[%02hhx] ", sections[s].data[n]);
		}
		puts("[end of section]");
	}
	puts("[done]");
}


int main(int argc, const char** argv) {

	if (argc <= 1) return puts("usage error: ./run <file.txt>");

	int file = open(argv[1], O_RDONLY);
	if (file < 0) { perror("open"); puts(argv[1]); exit(1); }
	const nat length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(length + 1, 1);
	read(file, text, length);
	close(file);

	struct section sections[4096] = {0};
	nat section_count = 0;

	puts("printing the text: ");
	printf("text %p @ %llu chars...\n", (void*) text, length);
	for (nat i = 0; i < length; i++) {
		if (text[i] == 'q') {
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
		abort();
	}


	debug_sections(sections, section_count);
	
	printf("heres the text too...\n");
	puts(text);

	for (nat s = 0; s < section_count; s++) {
		const struct section this = sections[s];
		printf("dissassembly of section #%llu: [address=0x%08hx], [length=%llu]\n", s, this.address, this.length);

		for (nat i = 0; i < this.length; i += 2) {
			const u16 ins = (u16) (((u16)this.data[(i < this.length ? i : 0)]) | ((u16)this.data[(i < this.length ? i : 0) + 1] << 8U));
			printf("%08llx:   %04hx ", this.address + i, ins);

			const byte op = ins >> 12;
			const byte ob = ins >> 13;

			if (op == 0x0) puts("\t\t\t\textended ins: MOVA, CMPA, ADDA, SUBA, RRCM, RRAM, RLAM, RRUM");
			/*else if (op == 0x1 and cd == 0x10) puts("RRC, RRC.B SWP.B RRA, RRA.B SXT  PUSH  PUSH.B CALL RETI CALLA ");
			else if (op == 0x1 and cd == 0x14) puts("PUSHM.A POPM.A PUSHM.W POPM.W ");
			else if (op == 0x1 and (cd == 0x18 or cd == 0x1C)) puts("extension words format I and II.");
			else if (op == 0x1) puts("\t\t\t\tUNKNOWN extension instruction...");
			*/

			else if (ob == 0x1) {
				const u16 cd = (ins >> 10) & 0x7;
				const u16 offset = (ins >> 0) & 0x3FF;

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
				printf("\t%s\t [offset=%hx (%hd)]\n", name[cd], offset, (short) ((short) offset | (short) 0xFC00));
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



				//if (not strcmp(names[op - 4], "BIC"))printf("FOUND BIC: source = %hu, dest = %hu, As = %hu, Ad = %hu, BW = %hu\n", source, dest, As, Ad, BW);

				//if (not strcmp(names[op - 4], "BIC")) { getchar(); }

				//(this.source_mode == 1 and this.source_reg != 2 and this.source_reg != 3) or
				//(this.source_mode == 3 and not this.source_reg)


				if (As == 0) {
					printf("     ");
					if (Ad) {
						i += 2;
						const u16 imm = (u16) (((u16)this.data[(i < this.length ? i : 0)]) | ((u16)this.data[(i < this.length ? i : 0) + 1] << 8U));
						printf("%04hx ", imm);
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("(%s + 0x%04hx) <-- %s", R[dest], imm, R[source]);
					} else { 
						printf("     ");
						printf("\t%s.%c\t", names[op - 0x4], BW ? 'B' : 'W'); 
						printf("%s <-- %s", R[dest], R[source]); 
					}



				} else if (As == 1) {

					//if (not strcmp(names[op - 4], "BIC"))printf("IN AS 1:  source = %hu, dest = %hu, As = %hu, Ad = %hu, BW = %hu\n", source, dest, As, Ad, BW);
					//if (not strcmp(names[op - 4], "BIC")) { getchar(); }

					u16 source_imm = 0;
					if (source != 2 and source != 3) {
						i += 2;
						source_imm = (u16) (((u16)this.data[(i < this.length ? i : 0)]) | ((u16)this.data[(i < this.length ? i : 0) + 1] << 8U));
						printf("%04hx ", source_imm);
					} else {
						printf("     ");
					}


					//if (not strcmp(names[op - 4], "BIC"))printf("AFTER GEN OF SOURCE IMM:  source_imm = %hu, source = %hu, dest = %hu, As = %hu, Ad = %hu, BW = %hu\n", 
					//		source_imm, source, dest, As, Ad, BW
					//);

					//if (not strcmp(names[op - 4], "BIC")) { getchar(); }


					if (Ad) {
						i += 2;
						const u16 imm = (u16) (((u16)this.data[(i < this.length ? i : 0)]) | ((u16)this.data[(i < this.length ? i : 0) + 1] << 8U));
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
						const u16 imm = (u16) (((u16)this.data[i < this.length ? i : 0]) | ((u16)this.data[(i < this.length ? i : 0) + 1] << 8U));
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
						const u16 imm = (u16) (((u16)this.data[i < this.length ? i : 0]) | ((u16)this.data[(i < this.length ? i : 0) + 1] << 8U));
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
					const u16 source_imm = (u16) (((u16)this.data[i < this.length ? i : 0]) | ((u16)this.data[(i < this.length ? i : 0) + 1] << 8U));
					printf("%04hx ", source_imm);
					if (Ad) {
						i += 2;
						const u16 imm = (u16) (((u16)this.data[i < this.length ? i : 0]) | ((u16)this.data[(i < this.length ? i : 0) + 1] << 8U));
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

	exit(0);
}












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


























