// my programming language! (repl version)
// dwrr   started on 2208232.211844 
// dwrr   written on 2208243.231335
// dwrr    edited on 2208265.235140

/*

------------------------ domain --------------------------


	this language aims to be a small, simple, concise, 
	optimized low-level language, for writing RISC 
	like archiechiture assembly, but also allow for 
	abstraction and flexibility to make writing of large 
	systems possible with optimal performance.



---------------------------- priorities of the language ------------------------------

		ranked in order of importance!


	1. performance: maximal runtime performance

	2. simplicity: extreme minimalism and simplicity

	3. flexibility: great language flexibility, very general compiletime 

	4. readability: decent programmer abstractions for readability


----------------------------------- main idea ----------------------------------------

	- the language uses a strictly statement-based syntax and design, where state change
	is the only real way to do anything in the language. (for: simplicity)

	- the language has a complete compiletime evaluation system, which allows arbitrary data and code to be generated at compiletime. (for: flexibility, performance)

	- the language has a simple macro system to allow the programmer to turn the low level language into a high level language. (for: readability)

	- the instructions/constructs in the language are modelled very closely off of RISC ISA's, in particular RISC-V. this makes the language very efficient to compile to RISC processors. (for: performance, simplicity)

	- 
















	----------- phases of the interpreter -----------





//	step2.  identify macros, and expand them?...




	step3.  generate RT, CT instructions, generating the 

			CT instructions array: (struct instruction[])
		and,
			RT instructions array: (struct instruction[])





//	step4.   macro expansion: expand all macros.
		




	step5.	 run the CT instructions, (with pc) this also computes and fills in 
						certain relative branch distances. 
	
	step6.   run the resulting RISCV instructions. (with pc)






				phase seq under construction











		// we are in the middle of implementing labels and gotos and branches. 





			// note: we totally dont even need the bool in_use array,because we can just use the name being a null pointer or not. and when we are done with a register, just set its name to be null. thats it. thats much better. 

			//   and then the general algorithm for the     defining new register section 

			// would be to just find an open null pointer in the array of names 
				// if you went through the entire array of names, and didnt find anything, then you know you ran out of registers, i think.

			// also when going through the array, you want to make sure the name  you are checking against, in the table is not a null pointer, of course. yeah this is good. 



*/










#include <stdio.h>
#include <stdbool.h>
#include <iso646.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


typedef uint64_t nat;


enum op_code {
	op_nop,
	op_ct_xor,
	op_xor,
	op_add,
	op_ct_add,
	op_addi,
	op_slt,
	op_slti,
	op_sub,
	op_ct_sub,
	op_sll,
	op_slli,
	op_load64,
	op_blt,

	op_ct_debug,
	op_debug,
};


static const char* op_code_spelling[] = {
	"op_nop",
	"op_ct_xor",
	"op_xor",
	"op_add",
	"op_ct_add",
	"op_addi",
	"op_slt",
	"op_slti",
	"op_sub",
	"op_ct_sub",
	"op_sll",
	"op_slli",
	"op_load64",
	"op_blt",

	"op_ct_debug",
	"op_debug",
};


struct instruction {
	nat op;
	nat _0;
	nat _1;
	nat _2; 
};

static char* read_file(const char* filename, size_t* out_length) {
	const int file = open(filename, O_RDONLY);
	if (file < 0) {
		perror("open"); 
		return NULL;
	}
	struct stat file_data = {0};
	if (stat(filename, &file_data) < 0) { 
		perror("stat"); 
		return NULL;
	}
	const size_t length = (size_t) file_data.st_size;
	char* buffer = not length ? NULL : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (buffer == MAP_FAILED) { 
		perror("mmap"); 
		return NULL;
	}
	close(file);
	*out_length = length;
	return buffer;
}


static bool equal(const char* s1, const char* s2) {
	if (strlen(s1) != strlen(s2)) return false;
	else return not strcmp(s1, s2);
}




static struct instruction instructions[4096] = {0};
static nat instruction_count = 0;

static nat _[4] = {0};
static nat base = 0;

static char* names[128] = {0};
static nat name_count = 0;

static char* label_names[2048] = {0};
static nat labels[2048] = {0};


static uint64_t* memory = NULL;
static nat registers[128] = {0};
static nat ct_registers[128] = {0};
static nat pc = 0;






static void ins(nat op) {
	instructions[instruction_count++] = (struct instruction) {
		.op = op, ._0 = _[0], ._1 = _[1], ._2 = _[2]
	};
}

static void print_ins(struct instruction O, nat p) {
	printf("---> [%llu] instruction: { operation=%s, dest=%llu, first=%llu, second=%llu }\n", 
		p, op_code_spelling[O.op], O._0, O._1, O._2);
}


static void compile_in(const char* string) {
	
	if (base) {
		ct_registers[*_] = (nat) strtoll(string, NULL, (int) base);
		base = 0;
		return;
	}

	if (equal(string, "pass")) {}

	else if (equal(string, "000")) { _[1] = _[0]; _[2] = _[0]; }
	else if (equal(string, "001")) {nat t1 = _[1]; _[1] = _[0]; _[2] = t1;}

	else if (equal(string, "021")) { nat t1 = _[1], t2 = _[2]; _[1] = t2; _[2] = t1; }
	else if (equal(string, "10")) { nat t0 = _[0], t1 = _[1]; _[0] = t1; _[1] = t0; }
	else if (equal(string, "210")) { nat t0 = _[0], t2 = _[2]; _[0] = t2; _[2] = t0; }
	
	else if (equal(string, "201")) {
		nat t0 = _[0], t1 = _[1], t2 = _[2]; _[0] = t2; _[1] = t0; _[2] = t1;
	}
	else if (equal(string, "120")) {
		nat t0 = _[0], t1 = _[1], t2 = _[2]; _[0] = t1; _[1] = t2; _[2] = t0;
	}

	else if (equal(string, "11")) { _[0] = _[1]; }
	else if (equal(string, "21")) { _[0] = _[2]; }
	else if (equal(string, "00")) { _[1] = _[0]; }
	else if (equal(string, "02")) { _[1] = _[2]; }

	else if (equal(string, "slt")) ins(op_slt);
	else if (equal(string, "slti")) ins(op_slti);
	else if (equal(string, "xor")) ins(op_xor);
	else if (equal(string, "ct_xor")) ins(op_ct_xor);
	else if (equal(string, "add")) ins(op_add);
	else if (equal(string, "addi")) ins(op_addi);
	else if (equal(string, "ct_add")) ins(op_ct_add);
	else if (equal(string, "sub")) ins(op_sub);
	else if (equal(string, "ct_sub")) ins(op_ct_sub);
	else if (equal(string, "sll")) ins(op_sll);
	else if (equal(string, "slli")) ins(op_slli);
	else if (equal(string, "load64")) ins(op_load64);
	else if (equal(string, "blt")) ins(op_blt);
	else if (equal(string, "print")) ins(op_debug);
	else if (equal(string, "ctprint")) ins(op_ct_debug);
	else if (equal(string, "hliteral")) { base = 16; }
	else if (equal(string, "dliteral")) { base = 10; }
	else if (equal(string, "bliteral")) { base = 2; }
	else if (equal(string, "delete")) { free(names[*_]); names[*_] = NULL; }

	else if (equal(string, "here")) {

		printf("assigning here location: %llu\n", instruction_count);
		ct_registers[*_] = instruction_count;
	
	} else {
		nat name = 0;
		nat open = name_count;
		while (name < name_count) {
			if (names[name]) {
				if (equal(string, names[name])) break;
			} else if (open == name_count) {
				open = name;
			}
			name++;
		}

		if (name == name_count) {
			printf("new: %s\n", string);
			if (open == name_count) name_count++;
			names[open] = strdup(string);
			name = open;
		} 
		_[2] = _[1]; _[1] = _[0]; _[0] = name;
	}
}


static nat rt_ins_count = 0;
static struct instruction rt_instructions[1024] = {0};

static void execute_ct_instruction(struct instruction I) {
	if (I.op == op_nop) {}
	else if (I.op == op_ct_xor) ct_registers[I._0] = ct_registers[I._1] ^ ct_registers[I._2];
	else if (I.op == op_ct_add) ct_registers[I._0] = ct_registers[I._1] + ct_registers[I._2];
	else if (I.op == op_ct_sub) ct_registers[I._0] = ct_registers[I._1] - ct_registers[I._2];
	else if (I.op == op_ct_debug) printf("CT#%llu=%llu\n", I._0, ct_registers[I._0]);
	else rt_instructions[rt_ins_count++] = I;
	pc++;
}

static void execute_instruction(struct instruction I) {
	if (I.op == op_nop) {}
	else if (I.op == op_xor) registers[I._0] = registers[I._1] ^ registers[I._2];
	else if (I.op == op_add) registers[I._0] = registers[I._1] + registers[I._2];
	else if (I.op == op_sub) registers[I._0] = registers[I._1] - registers[I._2];
	else if (I.op == op_addi) registers[I._0] = registers[I._1] + ct_registers[I._2];
	else if (I.op == op_slt) registers[I._0] = registers[I._1] < registers[I._2];
	else if (I.op == op_slti) registers[I._0] = registers[I._1] < ct_registers[I._2];
	else if (I.op == op_sll) registers[I._0] = registers[I._1] << registers[I._2];
	else if (I.op == op_slli) registers[I._0] = registers[I._1] << ct_registers[I._2];
	else if (I.op == op_load64) registers[I._0] = *(((uint64_t*)memory) + (registers[I._1] + ct_registers[I._2])); 
	else if (I.op == op_blt) { if (registers[I._0] < registers[I._1]) pc += ct_registers[I._2]; }
	else if (I.op == op_debug) printf("R#%llu=%llu\n", I._0, registers[I._0]);
	pc++;
}



static void interpret_file() {

	char buffer[4096] = {0};

	printf("file: ");
	fgets(buffer, sizeof buffer, stdin);
	buffer[strlen(buffer) - 1] = 0;

	size_t text_length = 0;
	char* text = read_file(buffer, &text_length);
	if (not text) return;

	const char delimiter = '.';    


	char* code[1024] = {0};
	char b[4096] = {0};
	nat len = 0;

	nat count = 0;
	for (nat i = 0; i < text_length; i++) {
		if (text[i] == delimiter) {
			b[len] = 0; len = 0;
			code[count++] = strdup(b);
		} else if (text[i] >= 33) b[len++] = text[i];
	}

	printf("file statements: ");
	for (nat i = 0; i < count; i++) {
		printf("\t%s\n", code[i]);
	}

	instruction_count = 0;
	memset(instructions, 0, sizeof instructions);

	printf("----------------------- compiling... -------------------\n");
	for (nat i = 0; i < count; i++) {
		printf(": parsing: %s\n", code[i]);
		compile_in(code[i]);
	}

	pc = 0; 
	printf("------------------- CT interpreting : rt_generation... -------------------\n");
	while (pc < instruction_count) {
		print_ins(instructions[pc], pc);
		execute_ct_instruction(instructions[pc]);
	}

	pc = 0;
	printf("---------------- interpreting... -----------------------\n");
	while (pc < rt_ins_count) {
		print_ins(rt_instructions[pc], pc);
		execute_instruction(rt_instructions[pc]);
	}
}


int main() { 

	printf("a repl/interpreter for my language.\nwork in progress.\n");


	memory = aligned_alloc(8, 1 << 16);

	char buffer[2048] = {0}, string[2048] = {0};

	memset(registers, 0xF0, sizeof registers);
	memset(ct_registers, 0xF0, sizeof ct_registers);

_: 	printf(" • ");
	fgets(buffer, sizeof buffer, stdin);
	nat len = strlen(buffer), length = 0;
	for (nat i = 0; i < len; i++)
		if (buffer[i] >= 33) string[length++] = buffer[i];
	string[length++] = 0;

	if (equal(string, "")) {}
	else if (equal(string, "o")) printf("\033[H\033[J");
	else if (equal(string, "quit")) goto done;
	else if (equal(string, "file")) interpret_file();


	else if (not strcmp(string, "help"))

		printf(	
		"\n\t"
		"quit : quit the utility.\n\t"
		"help : this help menu. \n\t"
		"file : interpret a file. allows for control flow.\n\t"
		"o : clear screen. \n\t"
	
		"debugregisters debug the current state of the registers.\n\t"
		"debugctregisters : debug the current state of the compiletime registers.\n\t"
		"debugnames : debug the currently defined names.\n\t"
		"debugops : print the 4 operand registers for debug.\n\t"

		"pass : nop operation.\n\t"

		"ct_xor : compiletime xor.\n\t"
		"xor : runtime xor instruction.\n\t"

		"ct_add : compiletime add.\n\t"
		"add : runtime add instruction.\n\t"
		"addi : runtime add immediate instruction.\n\t"

		"slt : runtime set less than instruction.\n\t"
		"slti : runtime set less than immediate instruction\n\t"

		"sub : runtime sub instruction.\n\t"
		"ct_sub compiletime sub instruction.\n\t"

		"slli : runtime shift logical left immediate instruction.\n\t"
		"sll : runtime shift logical left instruction.\n\t"

		"load64 : runtime load 64-bit word instruction.\n\t"
		"blt : runtime branch less than instruction. \n\t"

		"print : print register value for debug\n\t"
		"ctprint : print compiletime value for debug\n\t"
		
		"10 : nat t0 = _[0], t1 = _[1]; _[0] = t1; _[1] = t0; \n\t"
		"11 : _[0] = _[1]; \n\t"
		"21 : _[0] = _[2]; \n\t"
		"00 : _[1] = _[0]; \n\t"
		"02 : _[1] = _[2]; \n\t"

		"000 : _[1] = _[0]; _[2] = _[0]; \n\t"
		"001 : nat t1 = _[1]; _[1] = _[0]; _[2] = t1; \n\t"
		"021 : nat t1 = _[1], t2 = _[2]; _[1] = t2; _[2] = t1; \n\t"
		"210 : nat t0 = _[0], t2 = _[2]; _[0] = t2; _[2] = t0; \n\t"
		"201 : nat t0 = _[0], t1 = _[1], t2 = _[2]; _[0] = t2; _[1] = t0; _[2] = t1; \n\t"
		"120 : nat t0 = _[0], t1 = _[1], t2 = _[2]; _[0] = t1; _[1] = t2; _[2] = t0; \n\t"

		"hliteral : treat next word as a hex literal.\n\t"
		"dliteral : treat next word as a decimal literal.\n\t"
		"bliteral : treat next word as a binary literal.\n\t"

		"delete : delete the 0th virtual from the defined list of names.\n\t"
		"here : fill virtual[0] with the PC at parsing. used for implementing labels.\n\t"

		"(hex literal) {16} : if in state base = 16\n\t"
		"(decimal literal) {10} : if in state base = 10\n\t"
		"(binary literal) {2} : if in state base = 2\n\t"

		"(register name) {0} : if not found as any other ins. will be defined if not.\n\t"

		"\n"
		);	




	else if (equal(string, "debugregisters")) {
		for (nat i = 0; i < 32; i++) printf("\tR#%llu = %llu\n", i, registers[i]);
	}
	else if (equal(string, "debugctregisters")) {
		for (nat i = 0; i < 32; i++) printf("\tCT#%llu = %llu\n", i, ct_registers[i]);
	}
	else if (equal(string, "debugops")) {
		for (nat i = 0; i < 4; i++) printf("\tO#%llu = %llu\n", i, _[i]);	
	}

	else if (equal(string, "debugnames")) {
		for (nat i = 0; i < name_count; i++) printf("name[%llu] =  \"%s\" \n", i, names[i] ? names[i] : "(null)");
	}

	else {
		compile_in(string);
		if (instruction_count) {
			execute_ct_instruction(instructions[instruction_count - 1]);
			execute_instruction(instructions[instruction_count - 1]);
			instruction_count = 0;
		}
		pc = 0;
	}

	goto _;
done: 
	printf("quitting...\n");
}











































/*
here is the current language isa:
-------------------------------




	RV(32/64)I:
========================


memory:

	load (8-bit/16-bit/32-bit/64-bit)
	load unsigned (8-bit/16-bit/32-bit)

	store (8-bit/16-bit/32-bit/64-bit)

operations:

	shift left          (zero-ext imm) 
	shift right         (zero-ext imm)
	shift arith right   (zero-ext imm)

	xor    (sign-ext imm)
	or     (sign-ext imm)
	and    (sign-ext imm)

	add         (sign-ext imm)
	subtract    [no imm version]

	set less than 	          (sign-ext imm)
	set less than unsigned    (sign-ext imm)

branches:

	branch <
	branch < unsigned

	branch ≥
	branch ≥ unsigned

	branch =
	branch ≠


[end]














...




thats it





later on, we will add more complex instructions into the language, conditionally based on whether it is 

	RV64M
========================


	multiply lower

	multiply upper   (sign x sign)
	multiply upper   (unsign x unsign)
	multiply upper   (unsign x sign)

	divide
	divide unsigned

	remainder
	remainder unsigned

	



other instructions i'm probably not going to add:
-----------------------------------------------------------


	load upper imm 
	
	add upper imm to pc

	jump and link

	jump and link register








else if (not strcmp(string, "da")) {
	for (nat i = 0; i < 32; i++) printf("R#%llu=%llu\n", i, registers[i]);
	for (nat i = 0; i < count; i++) printf("name[%llu] =  \"%s\" \n", i, names[i]);
}



*/










/// lets implement:
///
///       x =  *(buffer + i * 4) >= 33
///
///
///   assume: 	buffer = feedbeef
///          	i = 3
///
///   calculate x.


/// implementation transcript:
/*




	zero 
	all
	xor

	one
	bliteral
	1

	one
	zero
	one
	addi

	buffer 
	hliteral 
	feedbeef

	buffer 
	zero 
	buffer 
	addi

	i 
	dliteral
	3
	
	i 
	zero 
	i
	addi






	one
	i
	T
	slli
	dup0
	slli

	
	buffer
	TT
	add

	(((    all xor    )))           // i did this so i wont segfault. you wouldnt need this.

	
	zero
	TT
	*
	load64

	33
	*
	result
	slt

	one
	result
	dup
	slt
	
	
	
	

	


	{0/1} < 1      // is a trick for inverting an slt.  very cool.

	



	
	
	






	oh my gosh!! macros are just text replacement now!!! there are literally not arguments to any call ever!!! thats so cool!!!! i love that. 

		i want to implement macros now. thats what i want to do next. and also get this thing to use a file. 

			and then i also want to add control flow. somehow... 

			yeah.. 

					thats going to be tricky... but i think we can do it. 


					
*/





