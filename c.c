// my programming language! (repl version)
// dwrr   written on 2208243.231335

/*


------------ domain --------------


	this language aims to be a small, simple, concise, 
	optimized low-level language, for writing RISC 
	like archiechiture assembly, but also allow for 
	abstraction and flexibility to make writing of large 
	systems possible with optimal performance.




---------------------------- priorities of the language ------------------------------


	1. performance: maximal runtime performance

	2. simplicity: extreme minimalism and simplicity

	3. flexibility: good language/compiletime flexibility 

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
		
	step5.	 run the CT instructions. this also computes and fills in 
						certain relative branch distances. 
	
	step6.   run the resulting RISCV instructions.






				phase seq under construction




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

	"op_ct_debug",
	"op_debug",
};




struct instruction {
	nat op;
	nat _0;
	nat _1;
	nat _2; // we should make this an array,  and make it 32 long. ish. or more.
};


static char* read_file(const char* filename, size_t* out_length) {
	const int file = open(filename, O_RDONLY);
	if (file < 0) {
		perror("open"); 
		exit(1); 
	}
	struct stat file_data = {0};
	if (stat(filename, &file_data) < 0) { 
		perror("stat"); 
		exit(1); 
	}
	const size_t length = (size_t) file_data.st_size;
	char* buffer = not length ? NULL : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (buffer == MAP_FAILED) { 
		perror("mmap"); 
		exit(1); 
	}
	close(file);
	*out_length = length;
	return buffer;
}




static bool alive[128] = {0};
static char* names[128] = {0};
static nat registers[128] = {0};         // in the real compiler, you would have only 32?.... hmm...
static nat ct_registers[128] = {0};
static nat count = 0;
static struct instruction I = {0};
static nat base = 0;

static nat pc = 0; // program counter 

static nat labels[2048];

static uint64_t* memory = NULL;

static bool equal(const char* s1, const char* s2) {
	if (strlen(s1) != strlen(s2)) return false;
	else return not strcmp(s1, s2);
}


static void compile_in(const char* string, nat this_location) {
	
	if (base) {
		ct_registers[I._0] = (nat) strtoll(string, NULL, (int) base);
		base = 0;
		return;
	}

	if (equal(string, "pass")) {}

	else if (equal(string, "all")) { 		// 000
		nat t0 = I._0; 
		I._0 = t0; I._1 = t0; I._2 = t0;
	}
	else if (equal(string, "dup")) { 		// 001
		nat t0 = I._0, t1 = I._1;
		I._0 = t0; I._1 = t0; I._2 = t1;
	}
	else if (equal(string, "dup0")) { 		// 002
		nat t0 = I._0, t2 = I._2;
		I._0 = t0; I._1 = t0; I._2 = t2;
	}
	else if (equal(string, "cycle+")) { 		// 201
		nat t0 = I._0, t1 = I._1, t2 = I._2; 
		I._0 = t2; I._1 = t0; I._2 = t1;
	}
	else if (equal(string, "cycle-")) { 		// 120
		nat t0 = I._0, t1 = I._1, t2 = I._2; 
		I._0 = t1; I._1 = t2; I._2 = t0;
	}
	else if (equal(string, "switch")) { 		// 021
		nat t0 = I._0, t1 = I._1, t2 = I._2; 
		I._0 = t0; I._1 = t2; I._2 = t1;
	}
	else if (equal(string, "dup1")) { 		// 112
		nat t1 = I._1, t2 = I._2; 
		I._0 = t1; I._1 = t1; I._2 = t2;
	}
	else if (equal(string, "exchange")) {  		// 102
		nat t0 = I._0, t1 = I._1, t2 = I._2; 
		I._0 = t1; I._1 = t0; I._2 = t2;
	}
	else if (equal(string, "back")) {  		// 210
		nat t0 = I._0, t1 = I._1, t2 = I._2; 
		I._0 = t2; I._1 = t1; I._2 = t0;
	}
	else if (equal(string, "copy")) {		// 022
		nat t0 = I._0, t2 = I._2; 
		I._0 = t0; I._1 = t2; I._2 = t2;
	}

	else if (equal(string, "print")) I.op = op_debug;
	else if (equal(string, "ctprint")) I.op = op_ct_debug;

	else if (equal(string, "slt")) I.op = op_slt;
	else if (equal(string, "slti")) I.op = op_slti;

	else if (equal(string, "xor")) I.op = op_xor;
	else if (equal(string, "ct_xor")) I.op = op_ct_xor;	

	else if (equal(string, "add")) I.op = op_add;
	else if (equal(string, "addi")) I.op = op_addi;
	else if (equal(string, "ct_add")) I.op = op_ct_add;
	
	else if (equal(string, "sub")) I.op = op_sub;
	else if (equal(string, "ct_sub")) I.op = op_ct_sub;

	else if (equal(string, "sll")) I.op = op_sll;
	else if (equal(string, "slli")) I.op = op_slli;

	else if (equal(string, "load64")) I.op = op_load64;

	else if (equal(string, "hliteral")) { base = 16; }
	else if (equal(string, "dliteral")) { base = 10; }
	else if (equal(string, "bliteral")) { base = 2; }

	else if (equal(string, "delete")) {
		if (alive[I._0]) {
			free(names[I._0]);
			alive[I._0] = false;
		} else {
			printf("error: register is not allocated.\n");
		}
	}

	else if (equal(string, "label")) {
		printf("assigning here location: %llu\n", this_location);
		ct_registers[I._0] = this_location;
	}

	else {
		nat i = 0;
		while(i < count) {
			if (alive[i] and equal(string, names[i])) break;
			i++;
		}

		if (i == count) {
			printf("defining new _0 register: %s\n", string);
			
			nat j = 0;
			while (j < count) {
				if (not alive[j]) break;
				j++;
			}

			if (j == count) count++;
			names[j] = strdup(string);

			I._2 = I._1;
			I._1 = I._0;
			I._0 = j;
			alive[j] = true;
		} else {
			printf("found existing _0 register: %s [%llu]\n", string, i);
			I._2 = I._1;
			I._1 = I._0;
			I._0 = i;
		}
	}
}

static void execute_ct_instruction(struct instruction I) {

	if (I.op == op_nop) return;

	else if (I.op == op_ct_xor) {
		printf("executing: ct_xor[c%llu,c%llu,c%llu]\n", I._0,I._1,I._2);
		ct_registers[I._0] = ct_registers[I._1] ^ ct_registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_ct_add) {
		printf("executing: ct_add[c%llu,c%llu,c%llu]\n", I._0,I._1,I._2);
		ct_registers[I._0] = ct_registers[I._1] + ct_registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_ct_sub) {
		printf("executing: ct_sub[c%llu,c%llu,c%llu]\n", I._0,I._1,I._2);
		ct_registers[I._0] = ct_registers[I._1] - ct_registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_ct_debug) {
		printf("CT#%llu=%llu\n", I._0, ct_registers[I._0]);
	}
/*
	else {
		printf("error: unknown ct op code: %llu", I.op);
		// exit(1);
	}
*/
}




static void execute_instruction(struct instruction I) {

	if (I.op == op_nop) return;

	else if (I.op == op_xor) {
		printf("executing: xor[r%llu,r%llu,r%llu]\n", I._0,I._1,I._2);
		registers[I._0] = registers[I._1] ^ registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_add) {
		printf("executing: add[r%llu,r%llu,r%llu]\n", I._0,I._1,I._2);
		registers[I._0] = registers[I._1] + registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_sub) {
		printf("executing: sub[r%llu,r%llu,r%llu]\n", I._0,I._1,I._2);
		registers[I._0] = registers[I._1] - registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_addi) {
		printf("executing: addi[r%llu,r%llu,c%llu]\n", I._0,I._1,I._2);
		registers[I._0] = registers[I._1] + ct_registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_slt) {
		printf("executing: slt[r%llu,r%llu,r%llu]\n", I._0,I._1,I._2);
		registers[I._0] = registers[I._1] < registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_slti) {
		printf("executing: slt[r%llu,r%llu,r%llu]\n", I._0,I._1,I._2);
		registers[I._0] = registers[I._1] < ct_registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_sll) {
		printf("executing: sll[r%llu,r%llu,r%llu]\n", I._0,I._1,I._2);
		registers[I._0] = registers[I._1] << registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_slli) {
		printf("executing: slli[r%llu,r%llu,c%llu]\n", I._0,I._1,I._2);
		registers[I._0] = registers[I._1] << ct_registers[I._2];
		I.op = op_nop;
	}

	else if (I.op == op_load64) {
		printf("executing: load64[r%llu,r%llu,c%llu]\n", I._0,I._1,I._2);
		registers[I._0] = *(((uint64_t*)memory) + (registers[I._1] + ct_registers[I._2])); 
		I.op = op_nop;
	}

	
	else if (I.op == op_debug) {
		printf("R#%llu=%llu\n", I._0, registers[I._0]);
	}

/*
	else {
		printf("error: unknown op code: %llu", I.op);
		exit(1);
	}
*/
}


int main() { 

	printf("a repl/interpreter for my language. \ndwrr 2208232.211844 \n");


	 // we need 8-byte aligned memory for load64.
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
	
	else if (equal(string, "file")) {
		printf("file: ");

		fgets(buffer, sizeof buffer, stdin);
		buffer[strlen(buffer) - 1] = 0;
	
		size_t text_length = 0;
		char* text = read_file(buffer, &text_length);

		char* code[1024] = {0};
		char b[4096] = {0};
		nat len = 0;

		nat count = 0;
		for (nat i = 0; i < text_length; i++) {
			if (text[i] == '.') {
				b[len] = 0;
				len = 0;
				code[count++] = strdup(b);
			}
			else if (text[i] >= 33) {
				b[len++] = text[i];
			}
		}

		
		
		printf("code: ");
		for (nat i = 0; i < count; i++) {
			printf("\t%s\n", code[i]);
		}

		nat ins_count = 0;
		struct instruction instructions[1024] = {0};

		printf("-----------------------compiling...-------------------\n");
		for (nat i = 0; i < count; i++) {
			printf(": parsing: %s\n", code[i]);
			compile_in(code[i], ins_count);
			if (I.op != op_nop) instructions[ins_count++] = I;
			I.op = op_nop;
		}

		printf("-------------------CT interpreting...-------------------\n");
		for (nat i = 0; i < ins_count; i++) {
			struct instruction O = instructions[i];
			
			printf("---> CT exec ins: { operation=%s, dest=%llu, "
				"first=%llu, second=%llu }\n",
				 op_code_spelling[O.op], O._0, O._1, O._2);
		
			execute_ct_instruction(O);
		}






		printf("----------------RT interpreting...-----------------------\n");
		for (nat i = 0; i < ins_count; i++) {
			struct instruction O = instructions[i];
			
			printf("---> RT exec ins: { operation=%s, dest=%llu, "
				"first=%llu, second=%llu }\n",
				 op_code_spelling[O.op], O._0, O._1, O._2);
		
			execute_instruction(O);
		}
	}


	else if (not strcmp(string, "help"))
		printf(	
			"quit \n"
			"help \n"
			"file \n"
			"o \n"
		
			"debugregisters \n"
			"debugctregisters \n"
			"debugnames \n"
			"debuginstruction \n"
			"print \n"
			
			"ct_xor \n"
			"xor \n"
			"ct_add \n"
			"add \n"
			"addi \n"
			"slt \n"
			"sub \n"
			"ct_sub \n"
			"load64 \n"

			"pass \n"
			"all \n"
			"cycle+ \n"
			"cycle- \n"
			"dup0 \n"
			"dup1 \n"
			"switch \n"
			"exchange \n"
			"copy \n"
			"back \n"

			"hliteral \n"
			"dliteral \n"
			"bliteral \n"

			"(hex literal) {16}\n"
			"(decimal literal) {10}\n"
			"(binary literal) {2}\n"

			"(register name) {0}\n"
		);




	else if (equal(string, "debugregisters")) {
		for (nat i = 0; i < 32; i++) printf("R#%llu=%llu\n", i, registers[i]);
	}
	else if (equal(string, "debugctregisters")) {
		for (nat i = 0; i < 32; i++) printf("CT#%llu=%llu\n", i, ct_registers[i]);
	}
	else if (equal(string, "debugi")) {
		printf("ins: { operation=%llu, dest=%llu, first=%llu, second=%llu }\n",
			 I.op, I._0, I._1,I._2);

	}
	else if (equal(string, "debugnames")) {
		for (nat i = 0; i < count; i++) printf("name[%llu] =  \"%s\" \n", i, names[i]);
	}

	else {
		compile_in(string, 0);
		execute_instruction(I);
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





