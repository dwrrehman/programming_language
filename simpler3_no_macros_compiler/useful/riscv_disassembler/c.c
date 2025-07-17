// a risc-v dissassembler to test our risc-v backend in the compiler! 
// written on 1202505165.135702 by dwrr.

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
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t byte;

static bool debug = 0;

static char* load_file(const char* filename, nat* text_length) {
	int file = open(filename, O_RDONLY);
	if (file < 0) { 
		printf("risc-v disassembler: \033[31;1merror:\033[0m could not open '%s': %s\n", 
			filename, strerror(errno)
		); 
		exit(1);
	}
	*text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(*text_length + 1, 1);
	read(file, text, *text_length);
	close(file);
	return text;
}


static u32 read_raw(int fd, void* p, size_t nn, u32* e) {
	struct termios terminal = {0};
	tcgetattr(0, &terminal);
	struct termios copy = terminal; 
	copy.c_cc[VMIN] = 1; 
	copy.c_cc[VTIME] = 0;
	copy.c_lflag &= ~((size_t) ECHO | ICANON);
	tcsetattr(0, TCSANOW, &copy);
	ssize_t n = read(fd, p, nn);
	*e = (u32) errno;
	tcsetattr(0, TCSANOW, &terminal);
	return (u32) n;
}



static void dump_hex(byte* memory, nat count) {
	printf("dumping bytes: (%llu)\n", count);
	for (nat i = 0; i < count; i++) {
		if (not (i % 16)) printf("\n\t");
		if (not (i % 4)) printf(" ");
		printf("%02hhx(%c) ", memory[i], memory[i] >= 32 ? memory[i] : ' ');
	}
	puts("");

	puts("second debug output:    debugging executable bytes:\n");
	for (nat i = 0; i < count; i++) {
		if (i % 32 == 0) puts("");
		if (i % 4 == 0) putchar(32);
		if (memory[i]) printf("\033[32;1m");
		printf("%02hhx ", memory[i]);
		if (memory[i]) printf("\033[0m");
	}
	puts("\n");
}

static int ecall(u32* registers, byte* memory) {

	const u32 n = registers[17];
	const u32 a0 = registers[10];
	const u32 a1 = registers[11];
	const u32 a2 = registers[12];

	if (n == 0) { puts("system call number error: null system call (n == 0)"); abort(); } 
	else if (n == 1) exit((int) a0);

	else if (n == 2) { registers[10] = read_raw((int) a0, (void*) (memory + a1), 
					(size_t) a2, registers + 11); 
			} 

	else if (n == 3) { registers[10] = (u32) write((int) a0, (void*) (memory + a1), (size_t) a2); registers[11] = (u32) errno; } 
	else { printf("error: unknown system call: x[17] = %x\n", n); abort(); } 

	return 0;
}





int main(int argc, const char** argv) {

	if (argc != 3) exit(puts("usage error: ./run (print/execute/debug) <file.hex>"));

	bool executing = argv[1][0] == 'e';
	debug = argv[1][0] == 'd';
	if (debug) executing = 1;

	nat text_length = 0;
	char* text = load_file(argv[2], &text_length);

	byte memory[65536] = {0};
	u32 count = 0;

	if (	text[0] == 'U' and 
		text[1] == 'F' and 
		text[2] == '2' and 
		text[3] == '\n'
	) { 
		printf("loading uf2 executable...\n");
		for (nat i = 0; i < text_length; i++) {
			if (i % 512 < 32 or i % 512 >= 508) continue;
			memory[count++] = (byte) text[i];
		}
	} else { 
		printf("loading hex array executable...\n");
		for (nat i = 0; i < text_length; i++) {
			if (text[i] == '0') {
				unsigned long n = strtoul(text + i, 0, 16);
				memory[count++] = (byte) n;
				i += 4;
			}
		}
	}


if (not executing) {

	while (count % 4) count++;

	puts("disassemblying the following executable: ");
	dump_hex(memory, count);
	puts(""); 
	
	for (u32 pc = 0; pc < count; pc += 4) {

		u32 word = 
			((u32) memory[pc + 0U] <<  0U) | 
			((u32) memory[pc + 1U] <<  8U) | 
			((u32) memory[pc + 2U] << 16U) | 
			((u32) memory[pc + 3U] << 24U) ;

		printf(" 0x%08x:   %02x %02x %02x %02x   ", pc, 
			memory[pc + 0], memory[pc + 1], memory[pc + 2], memory[pc + 3]
		); fflush(stdout);

		u32 op = word & 0x7F;
		u32 bit30 = (word >> 30) & 1;
		u32 Rd = (word >> 7) & 0x1F;
		u32 fn = (word >> 12) & 0x7;
		u32 Rs1 = (word >> 15) & 0x1F;
		u32 Rs2 = (word >> 20) & 0x1F;
		u32 imm12 = (word >> 20) & 0xFFF;

		u32 f7 = (word >> 25) & 0x3F;

		//if (((imm12 >> 11) & 0x1) == 1) imm12 |= 0xFFFFF000;

		u32 U_imm20 = word & 0xFFFFF000;

		if (op == 0x37) { // LUI
			printf("lui  x%u  #0x%08x\n", Rd, U_imm20); 
			
		} else if (op == 0x17) { // AUIPC
			printf("auipc  x%u  #0x%08x\n", Rd, U_imm20); 
			
			
		} else if (op == 0x6F) { // JAL
			u32 imm10_1 = (word >> 21U) & 0x3FF;
			u32 imm20 = (word >> 31U) & 0x1;
			u32 imm11 = (word >> 20U) & 0x1;
			u32 imm19_12 = (word >> 12U) & 0xFF;
			u32 imm = (imm20 << 20U) | (imm19_12 << 12U) | (imm11 << 11U) | (imm10_1 << 1U);
			if (imm20 == 1U) imm |= 0xFFE00000;

			printf("jal  x%u  #0x%08x (@0x%08x)\n", Rd, imm, pc + imm); 
			
			
		} else if (op == 0x67) { // JALR

			printf("jalr  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);

			//u32 target = registers[Rs1] + imm12;
			//target &= 0xFFFFFFFE;
			
			
		} else if (op == 0x63) { // BEQ / BNE / BLT / BGE / BLTU / BGEU 
			u32 limm12 = (word >> 31) & 0x1;
			u32 imm10_5 = (word >> 25) & 0x3F;
			u32 imm11 = (word >> 7) & 0x1;
			u32 imm4_1 = (word >> 8) & 0xF;
			u32 imm = (limm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
			if (limm12 == 1) imm |= 0xFFFFE000;
			
			if (fn == 0) { // BEQ
				printf("beq  x%u  x%u  #0x%08x (@0x%08x)\n", Rs1, Rs2, imm, pc + imm);
			} else if (fn == 1) { // BNE
				printf("bne  x%u  x%u  #0x%08x (@0x%08x)\n", Rs1, Rs2, imm, pc + imm);
			} else if (fn == 4) { // BLT
				printf("blt  x%u  x%u  #0x%08x (@0x%08x)\n", Rs1, Rs2, imm, pc + imm);
			} else if (fn == 5) { // BGE
				printf("bge  x%u  x%u  #0x%08x (@0x%08x)\n", Rs1, Rs2, imm, pc + imm);
			} else if (fn == 6) { // BLTU
				printf("bltu  x%u  x%u  #0x%08x (@0x%08x)\n", Rs1, Rs2, imm, pc + imm);
			} else if (fn == 7) { // BGEU
				printf("bgeu  x%u  x%u  #0x%08x (@0x%08x)\n", Rs1, Rs2, imm, pc + imm);
			}
			
		} else if (op == 0x03) { // LB / LH / LW / LBU / LHU

			if (fn == 0) { // LB
				printf("lb  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 1) { // LH
				printf("lh  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);	
			} else if (fn == 2) { // LW
				printf("lw  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 4) { // LBU 
				printf("lbu  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 5) { // LHU
				printf("lhu  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} 


		} else if (op == 0x23) { // SB / SH / SW
			u32 imm = Rd | (f7 << 5U); 
			if (fn == 0) {
				printf("sb  x%u  #0x%08x  x%u\n", Rs1, imm, Rs2);
			} else if (fn == 1) {
				printf("sh  x%u  #0x%08x  x%u\n", Rs1, imm, Rs2);
			} else if (fn == 2) {
				printf("sw  x%u  #0x%08x  x%u\n", Rs1, imm, Rs2);
			}

		} else if (op == 0x13) { // ADDI / SLTI / SLTIU / XORI / ORI / ANDI / SLLI / SRLI / SRAI

			if (fn == 0) { // ADDI
				printf("addi  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 1) { // SLLI
				printf("slli  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 2) { // SLTI
				printf("slti  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 3) { // SLTIU
				printf("sltiu  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 4) { // XORI
				printf("xori  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 5 && bit30 == 0) { // SRLI
				printf("srli  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 5 && bit30 == 1) { // SRAI
				printf("srai  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 6) { // ORI
				printf("ori  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			} else if (fn == 7) { // ANDI
				printf("andi  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
			}

		} else if (op == 0x33) { // ADD / SUB / SLL / SLT / SLTU / XOR / SRL / SRA / OR / AND     MUL / DIV / REM

			if (f7 == 0 or bit30) {

				if (fn == 0 && bit30 == 0) { // ADD
					printf("add  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				} else if (fn == 0 && bit30 == 1) { // SUB
					printf("sub  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				} else if (fn == 1) { // SLL
					printf("sll  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				} else if (fn == 2) { // SLT
					printf("slt  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				} else if (fn == 3) { // SLTU
					printf("sltu  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				} else if (fn == 4) { // XOR
					printf("xor  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				} else if (fn == 5 && bit30 == 0) { // SRL 
					printf("srl  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				} else if (fn == 5 && bit30 == 1) { // SRA
					printf("sra  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				} else if (fn == 6) { // OR
					printf("or  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				} else if (fn == 7) { // AND
					printf("and  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				}
	
			} else {

				if (fn == 0) { // MUL
					printf("mul  x%u  x%u  x%u\n", Rd, Rs1, Rs2);

				} else if (fn == 1) { // MULH
					printf("mulh  x%u  x%u  x%u\n", Rd, Rs1, Rs2);

				} else if (fn == 2) { // MULHSU
					printf("mulhsu  x%u  x%u  x%u\n", Rd, Rs1, Rs2);

				} else if (fn == 3) { // MULHU
					printf("mulhu  x%u  x%u  x%u\n", Rd, Rs1, Rs2);


				} else if (fn == 4) { // DIV
					printf("div  x%u  x%u  x%u\n", Rd, Rs1, Rs2);

				} else if (fn == 5) { // DIVU
					printf("divu  x%u  x%u  x%u\n", Rd, Rs1, Rs2);


				} else if (fn == 6) { // REM
					printf("rem  x%u  x%u  x%u\n", Rd, Rs1, Rs2);

				} else if (fn == 7) { // REMU
					printf("remu  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				}
			}

		} else if (op == 0x1F) { // FENCE / FENCE.I

			printf("fence / fence.i ...    --> ");
			printf("[internal error: unimplemented]");

		} else if (op == 0x73) { // ECALL / EBREAK / CSRRW / CSRRW / CSRRS / CSRRC / CSRRWI / CSRRSI / CSRRCI

			if (op == word) {
				printf("ecall\n");

			} else {
				printf("ebreak / csr_xx ...\n");
				printf("\nerror: illegal instruction opcode: 0x%u\n", op);
			}

		} else {
			printf("[illegal instruction opcode: 0x%u]\n", op);
		}

	}

	puts("\n[error: process exited without system call]");
	exit(0);


} else { // not executing:

	u32 registers[32] = {0};
	const u32 instruction_count = count;			
	registers[2] = instruction_count; // stack pointer, x2/sp.

	u32 pc = 0; 
	while (pc < instruction_count) {

		u32 word = 
			((u32) memory[pc + 0U] <<  0U) | 
			((u32) memory[pc + 1U] <<  8U) | 
			((u32) memory[pc + 2U] << 16U) | 
			((u32) memory[pc + 3U] << 24U) ;

		if (debug) {
			printf("[REGISTERS]: \n");
			for (u32 i = 0; i < 16; i++) {
				printf("\tx%-3u: 0x%08x \t x%-3u: 0x%08x\n", 
					i, registers[i], 16 + i, registers[16 + i]
				);
			}
			puts("");

			printf("[MEMORY]: \n");
			for (u32 i = 0; i < count; i += 8) {
				printf("0x%08x:  %02hhx %02hhx %02hhx %02hhx   %02hhx %02hhx %02hhx %02hhx\n", 
					i, memory[i + 0], memory[i + 1], memory[i + 2], memory[i + 3], 
					memory[i + 4], memory[i + 5], memory[i + 6], memory[i + 7]
				);
			}
			puts("");			
		}

		if (debug) printf("\n 0x%08x:   %02x %02x %02x %02x   ", pc, 
			memory[pc + 0], memory[pc + 1], memory[pc + 2], memory[pc + 3]
		); if (debug) fflush(stdout);

		u32 op = word & 0x7F;
		u32 bit30 = (word >> 30) & 1;
		u32 Rd = (word >> 7) & 0x1F;
		u32 fn = (word >> 12) & 0x7;
		u32 Rs1 = (word >> 15) & 0x1F;
		u32 Rs2 = (word >> 20) & 0x1F;
		u32 imm12 = (word >> 20) & 0xFFF;

		u32 f7 = (word >> 25) & 0x3F;

		if (((imm12 >> 11) & 0x1) == 1) imm12 |= 0xFFFFF000;
		u32 U_imm20 = word & 0xFFFFF000;

		registers[0] = 0;

		//u32 save_pc = pc;

		if (op == 0x37) { // LUI
			if (debug) printf("LUI  x%u  #0x%08x\n", Rd, U_imm20); 
			registers[Rd] = U_imm20;
			pc += 4;
			
		} else if (op == 0x17) { // AUIPC
			if (debug) printf("AUIPC  x%u  #0x%08x\n", Rd, U_imm20); 
			registers[Rd] = U_imm20 + pc;
			pc += 4;
			
		} else if (op == 0x6F) { // JAL
			u32 imm10_1 = (word >> 21U) & 0x3FF;
			u32 imm20 = (word >> 31U) & 0x1;
			u32 imm11 = (word >> 20U) & 0x1;
			u32 imm19_12 = (word >> 12U) & 0xFF;
			u32 imm = (imm20 << 20U) | (imm19_12 << 12U) | (imm11 << 11U) | (imm10_1 << 1U);
			if (imm20 == 1U) imm |= 0xFFE00000;

			if (debug) printf("JAL  x%u  #0x%08x\n", Rd, imm); 

			registers[Rd] = pc + 4;
			pc += imm;
			
		} else if (op == 0x67) { // JALR

			if (debug) printf("JALR  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);

			u32 target = registers[Rs1] + imm12;
			target &= 0xFFFFFFFE;
			registers[Rd] = pc + 4;
			pc = target;
			
		} else if (op == 0x63) { // BEQ / BNE / BLT / BGE / BLTU / BGEU 
			u32 limm12 = (word >> 31) & 0x1;
			u32 imm10_5 = (word >> 25) & 0x3F;
			u32 imm11 = (word >> 7) & 0x1;
			u32 imm4_1 = (word >> 8) & 0xF;
			u32 imm = (limm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
			if (limm12 == 1) imm |= 0xFFFFE000;
			
			if (fn == 0) { // BEQ
				if (debug) printf("BEQ  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);
				if (registers[Rs1] == registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 1) { // BNE
				if (debug) printf("BNE  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);
				if (registers[Rs1] != registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 4) { // BLT
				if (debug) printf("BLT  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);   // bug: make these signed checks. 
				if (registers[Rs1] < registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 5) { // BGE
				if (debug) printf("BGE  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);   // bug: make these signed checks. 
				if (registers[Rs1] >= registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 6) { // BLTU
				if (debug) printf("BLTU  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);
				if (registers[Rs1] < registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 7) { // BGEU
				if (debug) printf("BGEU  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);
				if (registers[Rs1] >= registers[Rs2]) pc += imm; else pc += 4;
			}
			
		} else if (op == 0x03) { // LB / LH / LW / LBU / LHU

			if (fn == 0) { // LB
				if (debug) printf("LB  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);

				u32 x = (u32) (memory[registers[Rs1] + imm12 + 0] << 0); 
				registers[Rd] = x;

			} else if (fn == 1) { // LH
				if (debug) printf("LH  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);

				u32 x = 0;
				x |= (u32) (memory[registers[Rs1] + imm12 + 0] << 0U);
				x |= (u32) (memory[registers[Rs1] + imm12 + 1] << 8U);
				registers[Rd] = x;
	
			} else if (fn == 2 || fn == 6) { // LW
				if (debug) printf("LW  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);

				u32 x = 0;
				x |= (u32) (memory[registers[Rs1] + imm12 + 0] << 0U);
				x |= (u32) (memory[registers[Rs1] + imm12 + 1] << 8U);
				x |= (u32) (memory[registers[Rs1] + imm12 + 2] << 16U);
				x |= (u32) (memory[registers[Rs1] + imm12 + 3] << 24U);
				registers[Rd] = x;

			} else if (fn == 4) { // LBU
				if (debug) printf("LBU  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				u32 x = (u32) (memory[registers[Rs1] + imm12 + 0] << 0U);
				registers[Rd] = x;

			} else if (fn == 5) { // LHU
				if (debug) printf("LHU  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				u32 x = 0;
				x |= (u32) (memory[registers[Rs1] + imm12 + 0] << 0U);
				x |= (u32) (memory[registers[Rs1] + imm12 + 1] << 8U);
				registers[Rd] = x;
			} 
			pc += 4;


		} else if (op == 0x23) { // SB / SH / SW

			u32 imm = Rd | (f7 << 5);

			if (fn == 0) {
				if (debug) printf("SB  x%u  #0x%08x  x%u\n", Rs1, imm, Rs2);
				memory[registers[Rs1] + imm + 0] = (registers[Rs2] >> 0U) & 0xFF;

			} else if (fn == 1) {
				if (debug) printf("SH  x%u  #0x%08x  x%u\n", Rs1, imm, Rs2);
				memory[registers[Rs1] + imm + 0] = (registers[Rs2] >> 0U) & 0xFF;
				memory[registers[Rs1] + imm + 1] = (registers[Rs2] >> 8U) & 0xFF;

			} else if (fn == 2) {
				if (debug) printf("SW  x%u  #0x%08x  x%u\n", Rs1, imm, Rs2);
				memory[registers[Rs1] + imm + 0] = (registers[Rs2] >> 0U) & 0xFF;
				memory[registers[Rs1] + imm + 1] = (registers[Rs2] >> 8U) & 0xFF;
				memory[registers[Rs1] + imm + 2] = (registers[Rs2] >> 16U) & 0xFF;
				memory[registers[Rs1] + imm + 3] = (registers[Rs2] >> 24U) & 0xFF;
			}

			pc += 4;


		} else if (op == 0x13) { // ADDI / SLTI / SLTIU / XORI / ORI / ANDI / SLLI / SRLI / SRAI

			if (fn == 0) { // ADDI
				if (debug) printf("ADDI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] + imm12;

			} else if (fn == 1) { // SLLI
				if (debug) printf("SLLI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] << imm12;

			} else if (fn == 2) { // SLTI
				if (debug) printf("SLTI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] < imm12;        // bug: make this check a signed-less-than. 

			} else if (fn == 3) { // SLTIU
				if (debug) printf("SLTIU  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] < imm12;

			} else if (fn == 4) { // XORI
				if (debug) printf("XORI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] ^ imm12;

			} else if (fn == 5 && bit30 == 0) { // SRLI
				if (debug) printf("SRLI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] >> (imm12 & 0x1F);

			} else if (fn == 5 && bit30 == 1) { // SRAI
				if (debug) printf("SRAI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] >> (imm12 & 0x1F);    // bug: make this do an actual arithmetic shift right. 

			} else if (fn == 6) { // ORI
				if (debug) printf("ORI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] | imm12;

			} else if (fn == 7) { // ANDI
				if (debug) printf("ANDI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] & imm12;
			}

			pc += 4;



		} else if (op == 0x33) { // ADD / SUB / SLL / SLT / SLTU / XOR / SRL / SRA / OR / AND


			if (f7 == 0 or bit30) { 

			if (fn == 0 && bit30 == 0) { // ADD
				if (debug) printf("ADD  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] + registers[Rs2];

			} else if (fn == 0 && bit30 == 1) { // SUB
				if (debug) printf("SUB  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] - registers[Rs2];

			} else if (fn == 1) { // SLL
				if (debug) printf("SLL  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] << registers[Rs2];

			} else if (fn == 2) { // SLT
				if (debug) printf("SLT  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] < registers[Rs2];        // bug: make this check a signed-less-than. 

			} else if (fn == 3) { // SLTU
				if (debug) printf("SLTU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] < registers[Rs2];

			} else if (fn == 4) { // XOR
				if (debug) printf("XOR  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] ^ registers[Rs2];

			} else if (fn == 5 && bit30 == 0) { // SRL 
				if (debug) printf("SRL  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] >> registers[Rs2];

			} else if (fn == 5 && bit30 == 1) { // SRA
				if (debug) printf("SRA  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] >> registers[Rs2];       // bug: make this an actual arithmetic shift right. 

			} else if (fn == 6) { // OR
				if (debug) printf("OR  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] | registers[Rs2];

			} else if (fn == 7) { // AND
				if (debug) printf("AND  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] & registers[Rs2];
			}

			} else {

				if (fn == 0) { // MUL
					if (debug) printf("MUL  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] * registers[Rs2];

				} else if (fn == 1) { // MULH
					if (debug) printf("MULH  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] * registers[Rs2];

				} else if (fn == 2) { // MULHSU
					if (debug) printf("MULHSU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] * registers[Rs2];

				} else if (fn == 3) { // MULHU
					if (debug) printf("MULHU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] * registers[Rs2];

				} else if (fn == 4) { // DIV
					if (debug) printf("DIV  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] / registers[Rs2];

				} else if (fn == 5) { // DIVU
					if (debug) printf("DIVU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] / registers[Rs2];

				} else if (fn == 6) { // REM
					if (debug) printf("REM  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] % registers[Rs2];

				} else if (fn == 7) { // REMU
					if (debug) printf("REMU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] % registers[Rs2];
				}

			}
			pc += 4;


		} else if (op == 0x1F) { // FENCE / FENCE.I
			if (debug) printf("FENCE / FENCE.I ...\n");
			printf("\nerror: illegal instruction opcode: 0x%u\n", op);
			pc += 4;


		} else if (op == 0x73) { // ECALL / EBREAK / CSRRW / CSRRW / CSRRS / CSRRC / CSRRWI / CSRRSI / CSRRCI

			if (op == word) {
				if (debug) printf("ECALL\n");
				if (ecall(registers, memory)) return 0; 

			} else {
				if (debug) printf("EBREAK / CSRxx ...\n");
				printf("\nerror: illegal instruction opcode: 0x%u\n", op);
			}
			pc += 4;

		} else {
			printf("\nerror: illegal instruction opcode: 0x%u\n", op);
			pc += 4;
		}

		//pc = save_pc + 4;

		if (debug) { printf("\n[pc now %u (0x%08x)]\n", pc, pc); getchar(); } 
	}

	puts("\n[process exited]");
	//puts("internal error: execution complete!");
	//dump_hex(memory, count);
	//puts(""); exit(0);

} // executing

	exit(0);

}



































/*















	if (executing) {

	u32 registers[32] = {0};

	const u32 instruction_count = count;
			
	registers[2] = instruction_count; // stack pointer, x2/sp.

	u32 pc = 0; 
	while (pc < instruction_count) {

		u32 word = 
			((u32) memory[pc + 0U] <<  0U) | 
			((u32) memory[pc + 1U] <<  8U) | 
			((u32) memory[pc + 2U] << 16U) | 
			((u32) memory[pc + 3U] << 24U) ;


		if (debug) printf("\n 0x%08x:   %02x %02x %02x %02x   ", pc, 
			memory[pc + 0], memory[pc + 1], memory[pc + 2], memory[pc + 3]
		); if (debug) fflush(stdout);

		u32 op = word & 0x7F;
		u32 bit30 = (word >> 30) & 1;
		u32 Rd = (word >> 7) & 0x1F;
		u32 fn = (word >> 12) & 0x7;
		u32 Rs1 = (word >> 15) & 0x1F;
		u32 Rs2 = (word >> 20) & 0x1F;
		u32 imm12 = (word >> 20) & 0xFFF;

		u32 f7 = (word >> 25) & 0x3F;

		if (((imm12 >> 11) & 0x1) == 1) imm12 |= 0xFFFFF000;
		u32 U_imm20 = word & 0xFFFFF000;

		registers[0] = 0;

		//u32 save_pc = pc;

		if (op == 0x37) { // LUI
			if (debug) printf("LUI  x%u  #0x%08x\n", Rd, U_imm20); 
			registers[Rd] = U_imm20;
			pc += 4;
			
		} else if (op == 0x17) { // AUIPC
			if (debug) printf("AUIPC  x%u  #0x%08x\n", Rd, U_imm20); 
			registers[Rd] = U_imm20 + pc;
			pc += 4;
			
		} else if (op == 0x6F) { // JAL
			u32 imm10_1 = (word >> 21) & 0x3FF;
			u32 imm20 = (word >> 31) & 0x1;
			u32 imm11 = (word >> 10) & 0x1;
			u32 imm19_12 = (word >> 12) & 0xFF;
			u32 imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
			if (imm20 == 1) imm |= 0xFFE00000;

			if (debug) printf("JAL  x%u  #0x%08x\n", Rd, imm); 

			registers[Rd] = pc + 4;
			pc += imm;
			
		} else if (op == 0x67) { // JALR

			if (debug) printf("JALR  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);

			u32 target = registers[Rs1] + imm12;
			target &= 0xFFFFFFFE;
			registers[Rd] = pc + 4;
			pc = target;
			
		} else if (op == 0x63) { // BEQ / BNE / BLT / BGE / BLTU / BGEU 
			u32 limm12 = (word >> 31) & 0x1;
			u32 imm10_5 = (word >> 25) & 0x3F;
			u32 imm11 = (word >> 7) & 0x1;
			u32 imm4_1 = (word >> 8) & 0xF;
			u32 imm = (limm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
			if (limm12 == 1) imm |= 0xFFFFE000;
			
			if (fn == 0) { // BEQ
				if (debug) printf("BEQ  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);
				if (registers[Rs1] == registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 1) { // BNE
				if (debug) printf("BNE  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);
				if (registers[Rs1] != registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 4) { // BLT
				if (debug) printf("BLT  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);   // bug: make these signed checks. 
				if (registers[Rs1] < registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 5) { // BGE
				if (debug) printf("BGE  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);   // bug: make these signed checks. 
				if (registers[Rs1] >= registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 6) { // BLTU
				if (debug) printf("BLTU  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);
				if (registers[Rs1] < registers[Rs2]) pc += imm; else pc += 4;

			} else if (fn == 7) { // BGEU
				if (debug) printf("BGEU  x%u  x%u  #0x%08x\n", Rs1, Rs2, imm);
				if (registers[Rs1] >= registers[Rs2]) pc += imm; else pc += 4;
			}
			
		} else if (op == 0x03) { // LB / LH / LW / LBU / LHU

			if (fn == 0) { // LB
				if (debug) printf("LB  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0);      // make this sign extend the destination.

			} else if (fn == 1) { // LH
				if (debug) printf("LH  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0U);     // make this sign extend the destination.
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 1] << 8U);
	
			} else if (fn == 2) { // LW
				if (debug) printf("LW  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0U);
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 1] << 8U);
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 2] << 16U);
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 3] << 24U);

			} else if (fn == 4) { // LBU 
				if (debug) printf("LBU  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0U);

			} else if (fn == 5) { // LHU
				if (debug) printf("LHU  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0U);
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 1] << 8U);	
			} 
			pc += 4;


		} else if (op == 0x23) { // SB / SH / SW

			u32 imm = 0; // TODO: determine this immediate for the r5_s encoding. 

			if (fn == 0) {
				if (debug) printf("SB  x%u  #0x%08x  x%u\n", Rs1, imm, Rs2);

				memory[registers[Rs1] + imm + 0] = (registers[Rs2] >> 0U) & 0xFF;

			} else if (fn == 1) {
				if (debug) printf("SH  x%u  #0x%08x  x%u\n", Rs1, imm, Rs2);
				memory[registers[Rs1] + imm + 0] = (registers[Rs2] >> 0U) & 0xFF;
				memory[registers[Rs1] + imm + 1] = (registers[Rs2] >> 8U) & 0xFF;

			} else if (fn == 2) {
				if (debug) printf("SW  x%u  #0x%08x  x%u\n", Rs1, imm, Rs2);
				memory[registers[Rs1] + imm + 0] = (registers[Rs2] >> 0U) & 0xFF;
				memory[registers[Rs1] + imm + 1] = (registers[Rs2] >> 8U) & 0xFF;
				memory[registers[Rs1] + imm + 2] = (registers[Rs2] >> 16U) & 0xFF;
				memory[registers[Rs1] + imm + 3] = (registers[Rs2] >> 24U) & 0xFF;
			}

			pc += 4;


		} else if (op == 0x13) { // ADDI / SLTI / SLTIU / XORI / ORI / ANDI / SLLI / SRLI / SRAI

			if (fn == 0) { // ADDI
				if (debug) printf("ADDI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] + imm12;

			} else if (fn == 1) { // SLLI
				if (debug) printf("SLLI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] << imm12;

			} else if (fn == 2) { // SLTI
				if (debug) printf("SLTI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] < imm12;        // bug: make this check a signed-less-than. 

			} else if (fn == 3) { // SLTIU
				if (debug) printf("SLTIU  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] < imm12;

			} else if (fn == 4) { // XORI
				if (debug) printf("XORI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] ^ imm12;

			} else if (fn == 5 && bit30 == 0) { // SRLI
				if (debug) printf("SRLI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] >> (imm12 & 0x1F);

			} else if (fn == 5 && bit30 == 1) { // SRAI
				if (debug) printf("SRAI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] >> (imm12 & 0x1F);    // bug: make this do an actual arithmetic shift right. 

			} else if (fn == 6) { // ORI
				if (debug) printf("ORI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] | imm12;

			} else if (fn == 7) { // ANDI
				if (debug) printf("ANDI  x%u  x%u  #0x%08x\n", Rd, Rs1, imm12);
				registers[Rd] = registers[Rs1] & imm12;
			}

			pc += 4;



		} else if (op == 0x33) { // ADD / SUB / SLL / SLT / SLTU / XOR / SRL / SRA / OR / AND


			if (f7 == 0 or bit30) { 

			if (fn == 0 && bit30 == 0) { // ADD
				if (debug) printf("ADD  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] + registers[Rs2];

			} else if (fn == 0 && bit30 == 1) { // SUB
				if (debug) printf("SUB  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] - registers[Rs2];

			} else if (fn == 1) { // SLL
				if (debug) printf("SLL  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] << registers[Rs2];

			} else if (fn == 2) { // SLT
				if (debug) printf("SLT  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] < registers[Rs2];        // bug: make this check a signed-less-than. 

			} else if (fn == 3) { // SLTU
				if (debug) printf("SLTU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] < registers[Rs2];

			} else if (fn == 4) { // XOR
				if (debug) printf("XOR  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] ^ registers[Rs2];

			} else if (fn == 5 && bit30 == 0) { // SRL 
				if (debug) printf("SRL  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] >> registers[Rs2];

			} else if (fn == 5 && bit30 == 1) { // SRA
				if (debug) printf("SRA  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] >> registers[Rs2];       // bug: make this an actual arithmetic shift right. 

			} else if (fn == 6) { // OR
				if (debug) printf("OR  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] | registers[Rs2];

			} else if (fn == 7) { // AND
				if (debug) printf("AND  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
				registers[Rd] = registers[Rs1] & registers[Rs2];
			}

			} else {

				if (fn == 0) { // MUL
					if (debug) printf("MUL  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] * registers[Rs2];

				} else if (fn == 1) { // MULH
					if (debug) printf("MULH  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] * registers[Rs2];

				} else if (fn == 2) { // MULHSU
					if (debug) printf("MULHSU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] * registers[Rs2];

				} else if (fn == 3) { // MULHU
					if (debug) printf("MULHU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] * registers[Rs2];

				} else if (fn == 4) { // DIV
					if (debug) printf("DIV  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] / registers[Rs2];

				} else if (fn == 5) { // DIVU
					if (debug) printf("DIVU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] / registers[Rs2];

				} else if (fn == 6) { // REM
					if (debug) printf("REM  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] % registers[Rs2];

				} else if (fn == 7) { // REMU
					if (debug) printf("REMU  x%u  x%u  x%u\n", Rd, Rs1, Rs2);
					registers[Rd] = registers[Rs1] % registers[Rs2];
				}

			}
			pc += 4;


		} else if (op == 0x1F) { // FENCE / FENCE.I
			if (debug) printf("FENCE / FENCE.I ...\n");
			printf("\nerror: illegal instruction opcode: 0x%u\n", op);
			pc += 4;


		} else if (op == 0x73) { // ECALL / EBREAK / CSRRW / CSRRW / CSRRS / CSRRC / CSRRWI / CSRRSI / CSRRCI

			if (op == word) {
				if (debug) printf("ECALL\n");
				if (ecall(registers, memory)) return 0; 

			} else {
				if (debug) printf("EBREAK / CSRxx ...\n");
				printf("\nerror: illegal instruction opcode: 0x%u\n", op);
			}
			pc += 4;

		} else {
			printf("\nerror: illegal instruction opcode: 0x%u\n", op);
			pc += 4;
		}

		//pc = save_pc + 4;

		if (debug) { printf("\n[pc now %u]\n", pc); getchar(); } 

	}

	puts("\n[process exited]");

	} else {
		dump_hex(memory, count);
		puts(""); exit(0);
	}
}
























*/


































































/*



// executable autogenerated by my compiler
export const executable = [
	0x13,0x00,0x10,0x00,0x97,0x10,0x02,0x00,
	0x93,0x80,0x10,0x02,0x13,0x01,0xE0,0x00,
	0x13,0x08,0x40,0x00,0x73,0x00,0x00,0x00,
	0x13,0x00,0xC0,0x00,0x13,0x08,0x10,0x00,
	0x73,0x00,0x00,0x00,
];






async function riscv_virtual_machine(instruction_count) {
	
	registers[2] = instruction_count * 4; // stack pointer, x2/sp.

	let pc = 0; 
	while (pc < instruction_count) {

		let word = memory[pc + 0] | (memory[pc + 1] << 8) | (memory[pc + 2] << 16) | (memory[pc + 3] << 24);

		console.log("[pc = " + pc + "]: executing: " + word.toString(16));

		let op = word & 0x7F;
		let bit30 = (word >> 30) & 1;
		let Rd = (word >> 7) & 0x1F;
		let fn = (word >> 12) & 0x7;
		let Rs1 = (word >> 15) & 0x1F;
		let Rs2 = (word >> 20) & 0x1F;
		let imm12 = (word >> 20) & 0xFFF;
		if (((imm12 >> 11) & 0x1) == 1) imm12 |= 0xFFFFF000;
		let U_imm20 = word & 0xFFFFF000;

		registers[0] = 0;

		let save_pc = pc;

		if (op == 0x37) { // LUI
			console.log("LUI x" + Rd + " #" + U_imm20.toString(16));
			registers[Rd] = U_imm20;
			pc += 4;
			
		} else if (op == 0x17) { // AUIPC
			console.log("AUIPC x" + Rd + " #" + U_imm20.toString(16));
			registers[Rd] = U_imm20 + pc;
			pc += 4;
			
		} else if (op == 0x6F) { // JAL
			let imm10_1 = (word >> 21) & 0x3FF;
			let imm20 = (word >> 31) & 0x1;
			let imm11 = (word >> 10) & 0x1;
			let imm19_12 = (word >> 12) & 0xFF;
			let imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);
			if (imm20 == 1) imm |= 0xFFE00000;
			console.log("JAL x" + Rd + " #" + imm.toString(16));
			registers[Rd] = pc + 4;
			pc += imm;
			
		} else if (op == 0x67) { // JALR
			console.log("JALR x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
			let target = registers[Rs1] + imm12;
			target &= 0xFFFFFFFE;
			registers[Rd] = pc + 4;
			pc = target;
			
		} else if (op == 0x63) { // BEQ / BNE / BLT / BGE / BLTU / BGEU 
			let limm12 = (word >> 31) & 0x1;
			let imm10_5 = (word >> 25) & 0x3F;
			let imm11 = (word >> 7) & 0x1;
			let imm4_1 = (word >> 8) & 0xF;
			let imm = (limm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
			if (limm12 == 1) imm |= 0xFFFFE000;
			
			if (fn == 0) { // BEQ
				console.log("BEQ x" + Rs1 + " x" + Rs2 + " #" + imm.toString(16));
				if (registers[Rs1] == registers[Rs2]) pc += imm;

			} else if (fn == 1) { // BNE
				console.log("BNE x" + Rs1 + " x" + Rs2 + " #" + imm.toString(16));
				if (registers[Rs1] != registers[Rs2]) pc += imm;

			} else if (fn == 4) { // BLT
				console.log("BLT x" + Rs1 + " x" + Rs2 + " #" + imm.toString(16));   // bug: make these signed checks. 
				if (registers[Rs1] < registers[Rs2]) pc += imm;

			} else if (fn == 5) { // BGE
				console.log("BGE x" + Rs1 + " x" + Rs2 + " #" + imm.toString(16));   // bug: make these signed checks. 
				if (registers[Rs1] >= registers[Rs2]) pc += imm;

			} else if (fn == 6) { // BLTU
				console.log("BLTU x" + Rs1 + " x" + Rs2 + " #" + imm.toString(16));
				if (registers[Rs1] < registers[Rs2]) pc += imm;

			} else if (fn == 7) { // BGEU
				console.log("BGEU x" + Rs1 + " x" + Rs2 + " #" + imm.toString(16));
				if (registers[Rs1] >= registers[Rs2]) pc += imm;
			}
			
		} else if (op == 0x03) { // LB / LH / LW / LBU / LHU

			if (fn == 0) { // LB
				console.log("LB x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0);      // make this sign extend the destination.

			} else if (fn == 1) { // LH
				console.log("LH x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0);     // make this sign extend the destination.
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 1] << 8);
	
			} else if (fn == 2) { // LW
				console.log("LW x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0);
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 1] << 8);
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 2] << 16);
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 3] << 24);

			} else if (fn == 4) { // LBU 
				console.log("LB x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0);

			} else if (fn == 5) { // LHU
				console.log("LH x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = 0;
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 0] << 0);
				registers[Rd] |= (memory[registers[Rs1] + imm12 + 1] << 8);	
			} 
			pc += 4;


		} else if (op == 0x23) { // SB / SH / SW

			let imm = 0;

			if (fn == 0) {
				console.log("SB x" + Rs1 + " #" + imm.toString(16) + " x" + Rs2);
				memory[registers[Rs1] + imm + 0] = (registers[Rs2] >> 0) & 0xFF;

			} else if (fn == 1) {
				console.log("SH x" + Rs1 + " #" + imm.toString(16) + " x" + Rs2);
				memory[registers[Rs1] + imm + 0] = (registers[Rs2] >> 0) & 0xFF;
				memory[registers[Rs1] + imm + 1] = (registers[Rs2] >> 8) & 0xFF;

			} else if (fn == 2) {
				console.log("SW x" + Rs1 + " #" + imm.toString(16) + " x" + Rs2);
				memory[registers[Rs1] + imm + 0] = (registers[Rs2] >> 0) & 0xFF;
				memory[registers[Rs1] + imm + 1] = (registers[Rs2] >> 8) & 0xFF;
				memory[registers[Rs1] + imm + 2] = (registers[Rs2] >> 16) & 0xFF;
				memory[registers[Rs1] + imm + 3] = (registers[Rs2] >> 24) & 0xFF;
			}

			pc += 4;



		} else if (op == 0x13) { // ADDI / SLTI / SLTIU / XORI / ORI / ANDI / SLLI / SRLI / SRAI

			if (fn == 0) { // ADDI
				console.log("ADDI x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = registers[Rs1] + imm12;

			} else if (fn == 1) { // SLLI
				console.log("SLLI x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = registers[Rs1] << imm12;

			} else if (fn == 2) { // SLTI
				console.log("SLTI x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = registers[Rs1] < imm12;        // bug: make this check a signed-less-than. 

			} else if (fn == 3) { // SLTIU
				console.log("SLTIU x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = registers[Rs1] < imm12;

			} else if (fn == 4) { // XORI
				console.log("XORI x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = registers[Rs1] ^ imm12;

			} else if (fn == 5 && bit30 == 0) { // SRLI
				console.log("SRLI x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = registers[Rs1] >> (imm12 & 0x1F);

			} else if (fn == 5 && bit30 == 1) { // SRAI
				console.log("SRAI x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = registers[Rs1] >> (imm12 & 0x1F);    // bug: make this do an actual arithmetic shift right. 

			} else if (fn == 6) { // ORI
				console.log("ORI x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = registers[Rs1] | imm12;

			} else if (fn == 7) { // ANDI
				console.log("ANDI x" + Rd + " x" + Rs1 + " #" + imm12.toString(16));
				registers[Rd] = registers[Rs1] & imm12;
			}
			pc += 4;



		} else if (op == 0x33) { // ADD / SUB / SLL / SLT / SLTU / XOR / SRL / SRA / OR / AND


			if (fn == 0 && bit30 == 0) { // ADD
				console.log("ADD x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] + registers[Rs2];

			} else if (fn == 0 && bit30 == 1) { // SUB
				console.log("SUB x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] - registers[Rs2];

			} else if (fn == 1) { // SLL
				console.log("SLL x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] << registers[Rs2];

			} else if (fn == 2) { // SLT
				console.log("SLT x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] < registers[Rs2];        // bug: make this check a signed-less-than. 

			} else if (fn == 3) { // SLTU
				console.log("SLTU x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] < registers[Rs2];

			} else if (fn == 4) { // XOR
				console.log("XOR x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] ^ registers[Rs2];

			} else if (fn == 5 && bit30 == 0) { // SRL / SRA
				console.log("SRL x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] >> registers[Rs2];

			} else if (fn == 5 && bit30 == 1) { // SRL / SRA
				console.log("SRA x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] >> registers[Rs2];       // bug: make this an actual arithmetic shift right. 

			} else if (fn == 6) { // OR
				console.log("OR x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] | registers[Rs2];

			} else if (fn == 7) { // AND
				console.log("AND x" + Rd + " x" + Rs1 + " x" + Rs2);
				registers[Rd] = registers[Rs1] & registers[Rs2];
			}
			pc += 4;


		} else if (op == 0x1F) { // FENCE / FENCE.I
			console.log("FENCE / FENCE.I ...");
			putstring("\nerror: illegal instruction opcode: 0x" + op.toString(16));
			pc += 4;


		} else if (op == 0x73) { // ECALL / EBREAK / CSRRW / CSRRW / CSRRS / CSRRC / CSRRWI / CSRRSI / CSRRCI

			if (op == word) {
				console.log("ECALL");
				if (await ecall()) return; 

			} else {
				console.log("EBREAK / CSRxx ...");
				putstring("\nerror: illegal instruction opcode: 0x" + op.toString(16));
			}
			pc += 4;

		} else {
			putstring("\nerror: illegal instruction opcode: 0x" + op.toString(16));
			pc += 4;
		}

		//pc = save_pc + 4;
	}

	// screen = startup;
	// save();	
	putstring("[process exited]");
}


*/







