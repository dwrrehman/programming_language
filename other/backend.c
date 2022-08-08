
// compile me with        

///      clang -g backend.c -fsanitize=address,undefined -o out -O0 -Weverything 

#include <unistd.h>
#include <stdio.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>

enum instruction_types {

	nop_instruction,

	add_imm_instruction,  // r = r + c
	add_instruction,   // r = r + r

	sub_instruction,   // r = r - r

	xor_imm_instruction,  // r = r xor c
	xor_instruction,   // r = r xor r

	or_imm_instruction,  // r = r or c
	or_instruction,   // r = r or r

	and_imm_instruction,  // r = r and c
	and_instruction,   // r = r and r

	shift_left_imm_instruction,  // r = r << c
	shift_left_instruction,      // r = r << r

	shift_right_imm_instruction, // r = r (logical)>> c
	shift_right_instruction,    // r = r (logical)>> r

	shift_arithmetic_right_imm_instruction, // r = r (arith)>> c
	shift_arithmetic_right_instruction,     // r = r (arith)>> r

	set_less_than_imm_instruction,   // r = r < c
	set_less_than_instruction,     // r = r < r

	set_less_than_unsigned_imm_instruction, // r = (unsigned) r < c
	set_less_than_unsigned_instruction, // r = (unsigned) r < r

	load_8_instruction,	
	load_16_instruction,
	load_32_instruction,
	load_64_instruction,

	load_8_unsigned_instruction,
	load_16_unsigned_instruction,
	load_32_unsigned_instruction,

	store_8_instruction,	
	store_16_instruction,
	store_32_instruction,
	store_64_instruction,


	goto_instruction,  // goto l

};

struct instruction {
	int type;
	int out;
	int in0;
	int in1;
};

static void print_instructions(struct instruction* instructions, int instruction_count) {

	printf("printing instructions:\n");
	
	for (int i = 0; i < instruction_count; i++) {
		printf("\t");

		const struct instruction ins = instructions[i];

		if (ins.type == nop_instruction) {
			printf("nop;");

		} else if (ins.type == add_instruction) {
			printf("r%d = r%d + r%d", ins.out, ins.in0, ins.in1);

		} else if (ins.type == goto_instruction) {
			printf("goto @%d", ins.in0);
		}
		printf("\n");
	}
}

int main() {

	struct instruction* instructions = NULL;
	int instruction_count = 0;
	
	instructions = realloc(instructions, sizeof(struct instruction) * (size_t)(instruction_count + 1));
	instructions[instruction_count++] = (struct instruction){.type = nop_instruction, .out = 0, .in0 = 1, .in1 = 2};

	instructions = realloc(instructions, sizeof(struct instruction) * (size_t)(instruction_count + 1));
	instructions[instruction_count++] = (struct instruction){.type = add_instruction, .out = 0, .in0 = 1, .in1 = 2};

	instructions = realloc(instructions, sizeof(struct instruction) * (size_t)(instruction_count + 1));
	instructions[instruction_count++] = (struct instruction){.type = nop_instruction, .out = 0, .in0 = 1, .in1 = 2};

	instructions = realloc(instructions, sizeof(struct instruction) * (size_t)(instruction_count + 1));
	instructions[instruction_count++] = (struct instruction){.type = add_instruction, .out = 0, .in0 = 1, .in1 = 2};

	instructions = realloc(instructions, sizeof(struct instruction) * (size_t)(instruction_count + 1));
	instructions[instruction_count++] = (struct instruction){.type = nop_instruction, .out = 0, .in0 = 1, .in1 = 2};

	instructions = realloc(instructions, sizeof(struct instruction) * (size_t)(instruction_count + 1));
	instructions[instruction_count++] = (struct instruction){.type = goto_instruction, .out = 0, .in0 = -1, .in1 = 0};

	print_instructions(instructions, instruction_count);
	free(instructions);
	
}
















/*




memory:


	load 8-bit 
	
	load 16-bit 

	load 32-bit 

	load unsigned 8-bit 
	
	load unsigned 16-bit 

	load unsigned 32-bit

	load 64-bit 



	store 8-bit 
	
	store 16-bit 

	store 32-bit 

	store 64-bit 



operations:


	shift left      (zero-ext imm) 

	shift right       (zero-ext imm)

	shift arithmetic right     (zero-ext imm)



	xor                (sign-ext imm)

	or                 (sign-ext imm)

	and                (sign-ext imm) 



	add                       (sign-ext imm)

	subtract	          no imm version



	set less than 	          (sign ext imm)

	set less than unsigned    (sign ext imm)



branches:

	branch lt
	
	branch lt unsigned


	branch geq

	branch geq unsigned


	branch eq

	branch neq




	multiply lower

	multiply upper   (sign x sign)

	multiply upper   (unsign x unsign)

	multiply upper   (unsign x sign)


	divide

	divide unsigned


	remainder

	remainder unsigned

	



*/