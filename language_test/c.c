// my programming language! (repl version)
// dwrr   started on 2208232.211844 
// dwrr   written on 2208243.231335
//         edited on 2208265.235140



// the next major todo is to:

//       document entirely how this code works, and why we made it work the way we did. 
//       in a manual.txt file. every single word/instruction, and its semantics.



//    we need to implement    strtoll   functoin   ourself
	// and make it work with unary,   and also with higher bases, all the way up to 85 characters?... not sure... hmm.. yeah... ill think about it... but yeah. 


// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wvla"

#include <stdio.h>
#include <stdbool.h>
#include <iso646.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef uint64_t nat;
typedef uint8_t byte;
struct instruction { nat op, _0, _1, _2; };

static const char digits[96] = 
	"0123456789abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ,."
	";:-_=+/?!@#$%^&*()<>[]{}|\\~`\'\"";

enum op_code {
	op_nop, op_add, op_addi, op_sub, 
	op_slt, op_slti, op_sll, op_slli, op_srl, op_blt, op_bne, 
	op_xor, op_or, op_and, op_mul, op_div, op_rem, 
	op_load64, op_store64, op_load32, op_store32,
	op_load16, op_store16, op_load8, op_store8,
	op_debug, op_ct_here
};

static nat 
	w_pc = 0, ct_pc = 0, rt_pc = 0, 
	base = 0, mode = 0, macro = 0, 
	name_count = 0, stack_count = 0,
	code_count = 0, ins_count = 0, rt_ins_count = 0;
	
static nat _[30] = {0};
static char* names[128] = {0};
static nat addresses[128] = {0};
static nat registers[128] = {0};
static nat ct_registers[128] = {0};
static nat stack[4096] = {0};
static byte* memory = NULL;
static byte* ct_memory = NULL;
static char* code[4096] = {0};
static struct instruction instructions[4096] = {0};
static struct instruction rt_instructions[4096] = {0};

static bool equal(const char* s1, const char* s2) {
	if (strlen(s1) != strlen(s2)) return false;
	else return not strcmp(s1, s2);
}

static void ins(nat op) {
	instructions[ins_count++] = (struct instruction) {
		.op = op | mode, ._0 = _[0], ._1 = _[1], ._2 = _[2]
	}; mode = 0;
}

static nat string_to_number(char* string, nat* length) {
	byte radix = 0, value = 0;
	nat result = 0, index = 0, place = 1;
begin:	if (index >= *length) goto done;
	value = 0;
top:	if (value >= 96) goto found;
	if (digits[value] == string[index]) goto found;
	value++;
	goto top;
found:	if (index) goto check;
	radix = value;
	goto next;
check:	if (value >= radix) goto done;
	result += place * (nat) value;
	place *= radix;
next:	index++;
	goto begin;
done:	*length = index;
	return result;
}

static void split_by_whitespace(char* text, nat text_length) {
	nat word_len = 0, i = 0;
	char word[4096] = {0};
begin:	if (i >= text_length) goto done;
	if (isspace(text[i])) goto skip;
	goto use;
skip:	i++;
	if (i >= text_length) goto done;
	if (isspace(text[i])) goto skip;
add:	if (not word_len) goto begin;
	word[word_len] = 0; 
	word_len = 0;
	code[code_count++] = strdup(word);
	if (i >= text_length) return;
use: 	word[word_len++] = text[i++];
	goto begin;
done:	if (word_len) goto add;
}

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

static void parse(char* string) {

	nat name = 0, open = 0;

	if (macro) { 
		if (equal(string, "stop")) macro = 0; 
		goto advance; 

	} else if (equal(string, "stop")) { 
		w_pc = stack[--stack_count]; 
		goto advance; 

	} else if (base) { 
		nat length = strlen(string);
		ct_registers[*_] = string_to_number(string, &length); 
		base = 0;
		goto advance; 
	}

	if (equal(string, "pass")) {}
	else if (equal(string, "11")) { _[0] = _[1]; }
	else if (equal(string, "21")) { _[0] = _[2]; }
	else if (equal(string, "00")) { _[1] = _[0]; }
	else if (equal(string, "02")) { _[1] = _[2]; }
	else if (equal(string, "000")) { _[1] = _[0]; _[2] = _[0]; }
	else if (equal(string, "001")) { _[2] = _[1]; _[1] = _[0]; }
	else if (equal(string, "021")) { nat t1 = _[1]; _[1] = _[2]; _[2] = t1; }
	else if (equal(string, "201")) { nat t2 = _[2]; _[2] = _[1]; _[1] = _[0]; _[0] = t2; }
	else if (equal(string, "120")) { nat t0 = _[0]; _[0] = _[1]; _[1] = _[2]; _[2] = t0; }
	else if (equal(string, "swap1")) { nat t0 = _[0]; _[0] = _[1]; _[1] = t0; }
	else if (equal(string, "swap2")) { nat t0 = _[0]; _[0] = _[2]; _[2] = t0; }
	else if (equal(string, "swap3")) { nat t0 = _[0]; _[0] = _[3]; _[3] = t0; }
	else if (equal(string, "swap4")) { nat t0 = _[0]; _[0] = _[4]; _[4] = t0; }
	else if (equal(string, "swap5")) { nat t0 = _[0]; _[0] = _[5]; _[5] = t0; }
	else if (equal(string, "swap6")) { nat t0 = _[0]; _[0] = _[6]; _[6] = t0; }
	else if (equal(string, "swap7")) { nat t0 = _[0]; _[0] = _[7]; _[7] = t0; }
	else if (equal(string, "nop")) ins(op_nop);
	else if (equal(string, "xor")) ins(op_xor);
	else if (equal(string, "add")) ins(op_add);
	else if (equal(string, "addi")) ins(op_addi);
	else if (equal(string, "sub")) ins(op_sub);
	else if (equal(string, "mul")) ins(op_mul);
	else if (equal(string, "div")) ins(op_div);
	else if (equal(string, "rem")) ins(op_rem);
	else if (equal(string, "slt")) ins(op_slt);
	else if (equal(string, "slti")) ins(op_slti);
	else if (equal(string, "sll")) ins(op_sll);
	else if (equal(string, "slli")) ins(op_slli);
	else if (equal(string, "blt")) ins(op_blt);
	else if (equal(string, "bne")) ins(op_bne);
	else if (equal(string, "load64")) ins(op_load64);
	else if (equal(string, "load32")) ins(op_load32);
	else if (equal(string, "load16")) ins(op_load16);
	else if (equal(string, "load8")) ins(op_load8);
	else if (equal(string, "store64")) ins(op_store64);
	else if (equal(string, "store32")) ins(op_store32);
	else if (equal(string, "store16")) ins(op_store16);
	else if (equal(string, "store8")) ins(op_store8);
	else if (equal(string, "print")) ins(op_debug);
	else if (equal(string, "here")) ins(op_ct_here);
	else if (equal(string, "literal")) base = 1;
	else if (equal(string, "now")) mode = 1 << 8;
	else if (equal(string, "cthere")) ct_registers[*_] = ins_count; 
	else if (equal(string, "define")) { addresses[*_] = w_pc; macro = 1; }
	else if (equal(string, "use")) { names[name = name_count++] = strdup(""); goto shift; }
	else if (equal(string, "undefine")) { free(names[*_]); names[*_] = NULL; addresses[*_] = 0; }
	



		// the idea, is that we should make the call site    beautiful.

		// just the macro name itself, nothing else. 

			// and so, i feel like we shouldnt really need to actually return from mulitple places inside the macro itself... 
					// which means that once we see a macro definition   "defineas" marker,  we are in macro mode, and can skip all the way until we see a return_from_macro statement. which then gets us out of macro mode. you cannot nest macro definitions, of course. thats not really useful at all. so yeah.

	
			// so yeah! now, we can consume the macro def, which means, we can store that branch address value away somewhere, (possibly even inside a compiletime register!) and then we can simply exit in_macro_mode,  and then start interpreting statements normally, until we find a call to it!

			// and we detect a call, because 

	//  *_ holds the macro name aready...?
		//  macros[macro_count++] = {.name = names[*_], .start = w_pc + 1};
	

// shoudld we just have an association btween the word, and its .start  by just storing the 

// *word_index + 1 means go to the statement after you found the defineas. 
//	thats where the body starts. and it ends where-ever you find the "macroreturn" statement. so yeah.

// this is kinda like getting the current address during parse time.
//  we need to find the statement after the function call.
// we need to gather the defintion as one thing, and store its location,
//      in assocation with the macro name. thats it. 

// then, when we see a call, we simply look up the name into the 
//	macro dict, and pull out its location, storing the location of
//	 where we need to return to,   on a stack of return adresses!
// then, when we finish with the macro, ie, we reach the position of 
//	the done call, of which, btw, we could always find ourselves in 
//	a macro, so we stop evaling it, and pop 

	// when you encounter a "macroreturn",  pop and resume execution given by tos!


	// this is essentially implementing comiletime(specifically at parse() time!) function definitions and function calls. basically.



	
	else {
		name = 0; 
		open = name_count;
		while (name < name_count) {
			if (names[name]) {
				if (equal(string, names[name])) break;
			} else if (open == name_count) open = name;
			name++;
		}
		if (name == name_count) {
			if (open == name_count) name_count++;
			names[open] = strdup(string);
			name = open;

		} else if (addresses[name]) {
			// on call of a macro, push the word_index on the stack!   
			stack[stack_count++] = w_pc;
			w_pc = addresses[name];
			goto advance;
		}
	shift:;	nat i = sizeof _ / sizeof(nat) - 1;
		while (i) { _[i] = _[i - 1]; i--; } *_ = name;
	}
advance: w_pc++;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"

static void execute_ct_instruction(struct instruction I) {

	if (not (I.op >> 8)) { rt_instructions[rt_ins_count++] = I; goto done; }
	const nat op = I.op & (nat)~(1 << 8);
	nat* r = ct_registers;
	nat* ctr = ct_registers;
	byte* m = ct_memory;

	if (op == op_nop) {}
	else if (op == op_xor) r[I._0] = r[I._1] ^ r[I._2];
	else if (op == op_and) r[I._0] = r[I._1] & r[I._2];
	else if (op == op_or)  r[I._0] = r[I._1] | r[I._2];
	else if (op == op_add) r[I._0] = r[I._1] + r[I._2];
	else if (op == op_sub) r[I._0] = r[I._1] - r[I._2];
	else if (op == op_mul) r[I._0] = r[I._1] * r[I._2];
	else if (op == op_div) r[I._0] = r[I._1] / r[I._2];
	else if (op == op_rem) r[I._0] = r[I._1] % r[I._2];
	else if (op == op_slt) r[I._0] = r[I._1] < r[I._2];
	else if (op == op_sll) r[I._0] = r[I._1] << r[I._2];
	else if (op == op_srl) r[I._0] = r[I._1] >> r[I._2];
	else if (op == op_load64) r[I._0] = * (uint64_t*) (m + r[I._1] + ctr[I._2]);
	else if (op == op_load32) r[I._0] = * (uint32_t*) (m + r[I._1] + ctr[I._2]);
	else if (op == op_load16) r[I._0] = * (uint16_t*) (m + r[I._1] + ctr[I._2]);
	else if (op == op_load8)  r[I._0] = * (uint8_t*)  (m + r[I._1] + ctr[I._2]);
	else if (op == op_store64) * (uint64_t*) (m + r[I._1] + ctr[I._2]) = (uint64_t) r[I._0]; 
	else if (op == op_store32) * (uint32_t*) (m + r[I._1] + ctr[I._2]) = (uint32_t) r[I._0]; 
	else if (op == op_store16) * (uint16_t*) (m + r[I._1] + ctr[I._2]) = (uint16_t) r[I._0]; 
	else if (op == op_store8)  * (uint8_t*)  (m + r[I._1] + ctr[I._2]) = (uint8_t)  r[I._0]; 	
	else if (op == op_blt) { if (r[I._0] < r[I._1]) ct_pc += ctr[I._2]; }
	else if (op == op_bne) { if (r[I._0] != r[I._1]) ct_pc += ctr[I._2]; }
	else if (op == op_debug) printf("CT#%llu=%lld\n", I._0, r[I._0]);
	else if (op == op_ct_here) ctr[I._0] = rt_ins_count; 
done:
	ct_pc++;
}

static void execute_instruction(struct instruction I) {
	
	const nat op = I.op;
	nat* r = registers;
	nat* ctr = ct_registers;
	byte* m = memory;
	
	if (op == op_nop) {}
	else if (op == op_addi) r[I._0] = r[I._1] + ctr[I._2];
	else if (op == op_slti) r[I._0] = r[I._1] < ctr[I._2];
	else if (op == op_slli) r[I._0] = r[I._1] << ctr[I._2];
	else if (op == op_xor) r[I._0] = r[I._1] ^ r[I._2];
	else if (op == op_and) r[I._0] = r[I._1] & r[I._2];
	else if (op == op_or)  r[I._0] = r[I._1] | r[I._2];
	else if (op == op_add) r[I._0] = r[I._1] + r[I._2];
	else if (op == op_sub) r[I._0] = r[I._1] - r[I._2];
	else if (op == op_mul) r[I._0] = r[I._1] * r[I._2];
	else if (op == op_div) r[I._0] = r[I._1] / r[I._2];
	else if (op == op_rem) r[I._0] = r[I._1] % r[I._2];
	else if (op == op_slt) r[I._0] = r[I._1] < r[I._2];
	else if (op == op_sll) r[I._0] = r[I._1] << r[I._2];
	else if (op == op_srl) r[I._0] = r[I._1] >> r[I._2];
	else if (op == op_load64) r[I._0] = * (uint64_t*) (m + r[I._1] + ctr[I._2]);
	else if (op == op_load32) r[I._0] = * (uint32_t*) (m + r[I._1] + ctr[I._2]);
	else if (op == op_load16) r[I._0] = * (uint16_t*) (m + r[I._1] + ctr[I._2]);
	else if (op == op_load8)  r[I._0] = * (uint8_t*)  (m + r[I._1] + ctr[I._2]);
	else if (op == op_store64) * (uint64_t*) (m + r[I._1] + ctr[I._2]) = (uint64_t) r[I._0]; 
	else if (op == op_store32) * (uint32_t*) (m + r[I._1] + ctr[I._2]) = (uint32_t) r[I._0]; 
	else if (op == op_store16) * (uint16_t*) (m + r[I._1] + ctr[I._2]) = (uint16_t) r[I._0]; 
	else if (op == op_store8)  * (uint8_t*)  (m + r[I._1] + ctr[I._2]) = (uint8_t)  r[I._0]; 	
	else if (op == op_blt) { if (r[I._0] < r[I._1]) rt_pc += ctr[I._2]; }
	else if (op == op_bne) { if (r[I._0] != r[I._1]) rt_pc += ctr[I._2]; }
	else if (op == op_debug) printf("R#%llu=%lld\n", I._0, r[I._0]);
	else { puts("unknown RT instruction"); abort(); }
	rt_pc++;
}

#pragma clang diagnostic pop

static void resetenv() {
	w_pc = 0; ct_pc = 0; rt_pc = 0;
	base = 0; macro = 0;
	code_count = 0; name_count = 0; rt_ins_count = 0;
	ins_count = 0;  stack_count = 0;
	memset(_, 0, sizeof _);
	memset(names, 0, sizeof names);
	memset(addresses, 0, sizeof addresses);
	memset(registers, 0xF0, sizeof registers);
	memset(ct_registers, 0xF0, sizeof ct_registers);
	memset(stack, 0, sizeof stack);
	memset(code, 0, sizeof code);
	memset(instructions, 0, sizeof instructions);
	memset(rt_instructions, 0, sizeof rt_instructions);
	free(memory);
	memory = aligned_alloc(8, 1 << 16);
	free(ct_memory);
	ct_memory = aligned_alloc(8, 1 << 16);
}

static void interpret_in(char* text, nat text_length) {
	split_by_whitespace(text, text_length);
	while (w_pc < code_count) parse(code[w_pc]);
	while (ct_pc < ins_count) execute_ct_instruction(instructions[ct_pc]);
	while (rt_pc < rt_ins_count) execute_instruction(rt_instructions[rt_pc]);
}

int main() {

	puts("a repl/interpreter for my language.");

	char line[4096] = {0};
	   memory = aligned_alloc(8, 1 << 16);
	ct_memory = aligned_alloc(8, 1 << 16);
	memset(   registers, 0xF0, sizeof    registers);
	memset(ct_registers, 0xF0, sizeof ct_registers);
	
_: 	printf(" • ");
	fgets(line, sizeof line, stdin);
	nat line_length = strlen(line);
	char* string = strdup(line);
	string[line_length - 1] = 0;

	if (equal(string, "")) {}
	else if (equal(string, "resetenv")) resetenv();
	else if (equal(string, "o") or equal(string, "clear")) printf("\033[H\033[J");
	else if (equal(string, "?") or equal(string, "help")) puts("see manual.txt");
	else if (equal(string, "q") or equal(string, "quit")) goto done;
	
	else if (equal(string, "f") or equal(string, "file")) {

		char buffer[4096] = {0};
		printf("filename: ");
		fgets(buffer, sizeof buffer, stdin);
		buffer[strlen(buffer) - 1] = 0;
		size_t length = 0;
		char* contents = read_file(buffer, &length);
		if (contents) interpret_in(contents, length);
	
	} else if (equal(string, "debugregisters")) {
		for (nat i = 0; i < 32; i++) printf("\tR#%llu = %llu\n", i, registers[i]);
	}
	else if (equal(string, "debugctregisters")) {
		for (nat i = 0; i < 32; i++) printf("\tCT#%llu = %llu\n", i, ct_registers[i]);
	}
	else if (equal(string, "debugaddr")) {
		for (nat i = 0; i < 32; i++) printf("\tA#%llu = %llu\n", i, addresses[i]);	
	}
	else if (equal(string, "debugops")) {
		for (nat i = 0; i < sizeof _ / sizeof(nat); i++) printf("\tO#%llu = %llu\n", i, _[i]);	
	}
	else if (equal(string, "debugstate")) {
		printf("state:\n\tbase=%llu, macro=%llu\n\n", base, macro);	
	}
	else if (equal(string, "debugmemory")) {
		char buffer[4096] = {0};
		printf("pointer: ");
		fgets(buffer, sizeof buffer, stdin);
		buffer[strlen(buffer) - 1] = 0;

		const nat pointer = (nat) atoi(buffer);
		printf("memory[%llu] = %02hhx\n", pointer, memory[pointer]);
	}
	else if (equal(string, "debugctmemory")) {
		char buffer[4096] = {0};
		printf("pointer: ");
		fgets(buffer, sizeof buffer, stdin);
		buffer[strlen(buffer) - 1] = 0;

		const nat pointer = (nat) atoi(buffer);
		printf("ct_memory[%llu] = %02hhx\n", pointer, ct_memory[pointer]);
	}
	else if (equal(string, "debugnames")) {
		for (nat i = 0; i < name_count; i++) 
			printf("name[%llu] =  %s \n", i, names[i] ? names[i] : "{NULL}");
	}

	else interpret_in(line, line_length);

	goto _;
done: 	printf("quitting...\n");
}































































/*
static const char* op_code_spelling[] = {
	"op_nop", "op_ct_nop",
	"op_ct_xor", "op_xor",
	"op_add", "op_ct_add", "op_addi",
	"op_slt", "op_slti",
	"op_sub", "op_ct_sub",
	"op_sll", "op_slli",
	
	"op_load64", "op_store64",
	"op_load32", "op_store32",
	"op_load16", "op_store16",
	"op_load8", "op_store8",

	"op_ct_load64", "op_ct_store64",
	"op_ct_load32", "op_ct_store32",
	"op_ct_load16", "op_ct_store16",
	"op_ct_load8", "op_ct_store8",

	"op_blt", "op_ct_blt",
	"op_ct_debug", "op_debug", "op_ct_here", 
	"op_mul", "op_div", "op_ct_mul", "op_ct_div",
	"op_rem", "op_ct_rem",
	"op_ct_sll", "op_ct_slt",
	"op_bne", "op_ct_bne",
	"op_or", "op_ct_or",
	"op_and", "op_ct_and",
	"op_srl", "op_ct_srl",	
};
static void print_ins(struct instruction O, nat p) {
	printf("---> [%llu] instruction: { operation=%s, dest=%llu, first=%llu, second=%llu }\n", 
		p, op_code_spelling[O.op], O._0, O._1, O._2);
}

static void print_strings(char** list, nat count) {
	printf("statement list(count=%llu): \n", count);
	for (nat i = 0; i < count; i++) {
		printf("\t\"%s\"\n", list[i]);
	}
	puts("[end-list]");
}*/









/*


		random piece of code lol

	{
		int a[5] = {4, 3, 2, 5, 6};
		
		for (int i = 5; i--;) {
			printf("%d\n", a[i]);
		}

		exit(1);
	}







	//else if (I.op == op_store64) *((uint8_t*)memory + registers[I._1] + ct_registers[I._2]) = (uint64_t) registers[I._0]; 
	
	//else if (I.op == op_load32) registers[I._0] = *((uint8_t*)memory + registers[I._1] + ct_registers[I._2]);
	//else if (I.op == op_store32) *((uint8_t*)memory + registers[I._1] + ct_registers[I._2]) = (uint32_t) registers[I._0]; 
	
	//else if (I.op == op_load16) registers[I._0] = *((uint8_t*)memory + registers[I._1] + ct_registers[I._2]);
	//else if (I.op == op_store16) *((uint8_t*)memory + registers[I._1] + ct_registers[I._2]) = (uint16_t) registers[I._0]; 

	//else if (I.op == op_load8) registers[I._0] = *((uint8_t*)memory + registers[I._1] + ct_registers[I._2]);
	//else if (I.op == op_store8) *((uint8_t*)memory + registers[I._1] + ct_registers[I._2]) = (uint8_t) registers[I._0]; 









2208287.163230:

	so how are we going to implement macros!?

			i think we are ready to implement those now. its a really important feature to get right.


	
	what is the syntax/words to do with macros?

		
	
		in forth, you say           :     macro-name     word-list-here    ;

		so yeah, thats cool i guess... but 



	



	well, we need to think about control flow, 


		and we need to think about how to make things... use less names?


			ie, use    imperative state,     to not have to specify   names of things


				but simply have the thing just knowww    what to name to use 


						thats the key 







		and for control flow, 


					i think we essentuail


	










previous help menu:          (use for manual.txt)





	printf(	
		"\n\t"
		"quit(q) : quit the utility.\n\t"
		"help(?) : this help menu. (abbrev = '?')\n\t"
		"file(f) : interpret a file. allows for control flow.\n\t"
		"clear(o) : clear screen. \n\t"
	
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
		"ct_blt : compiletime branch less than instruction. \n\t"

		"print : print register value for debug\n\t"
		"ctprint : print compiletime value for debug\n\t"
		
		"11 : _[0] = _[1]; \n\t"
		"21 : _[0] = _[2]; \n\t"
		"00 : _[1] = _[0]; \n\t"
		"02 : _[1] = _[2]; \n\t"
		"000 : _[1] = _[0]; _[2] = _[0]; \n\t"
		"001 : _[2] = _[1]; _[1] = _[0]; \n\t"
		"021 : nat t1 = _[1]; _[1] = _[2]; _[2] = t1; \n\t"
		"10 : nat t0 = _[0]; _[0] = _[1]; _[1] = t0; \n\t"
		"210 : nat t0 = _[0]; _[0] = _[2]; _[2] = t0; \n\t"
		"201 : nat t2 = _[2]; _[2] = _[1]; _[1] = _[0]; _[0] = t2; \n\t"
		"120 : nat t0 = _[0]; _[0] = _[1]; _[1] = _[2]; _[2] = t0; \n\t"

		"literal16 : treat next word as a hex literal.\n\t"
		"literal10 : treat next word as a decimal literal.\n\t"
		"literal2 : treat next word as a binary literal.\n\t"

		"delete : delete the 0th virtual from the defined list of names.\n\t"

		"here : fill ct_registers[_[0]] with the PC at ct_exec. used for implementing rt branches.\n\t"
		"cthere : fill ct_registers[_[0]] with the PC at parsing. used for implementing ct branches.\n\t"

		"(hex literal) {16} : if in state base = 16\n\t"
		"(decimal literal) {10} : if in state base = 10\n\t"
		"(binary literal) {2} : if in state base = 2\n\t"

		"(register name) {0} : if not found as any other ins. will be defined if not.\n\t"

		"\n"
	);




















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

	[4. readability: decent programmer abstractions for readability]


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






/*
		compile_in(string);
		if (instruction_count) {
			execute_ct_instruction(instructions[instruction_count - 1]);
			execute_instruction(instructions[instruction_count - 1]);
			instruction_count = 0;
		}
		pc = 0;
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





