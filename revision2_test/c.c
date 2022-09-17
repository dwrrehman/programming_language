// my programming language! (repl version)
// dwrr   started on 2208232.211844 
//        written on 2208243.231335
//         edited on 2208265.235140

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



#include <stdio.h>

#define red   "\x1B[31m"
#define green   "\x1B[32m"
#define yellow   "\x1B[33m"
#define blue   "\x1B[34m"
#define magenta   "\x1B[35m"
#define cyan   "\x1B[36m"
#define white   "\x1B[37m"
#define reset "\x1B[0m"

typedef uint64_t nat;
typedef uint8_t byte;
struct instruction { nat op, _0, _1, _2; };

static const char digits[96] = 
	"0123456789abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ,."
	";:-_=+/?!@#$%^&*()<>[]{}|\\~`\'\"";

enum op_code {
	op_nop, op_add, op_sub, op_xor, 
	op_or, op_and, op_sll, op_srl, 

	op_sra, op_mul, op_div, op_rem, 
	op_slt, op_sltu, op_blt, op_bge, 

	op_bne, op_beq, op_bltu, op_bgeu,    // first = 16
	op_jal, op_jalr, op_slli, op_srli,   // first = 20

	op_srai, op_slti, op_sltiu, op_addi, // 24
	op_xori, op_andi, op_ori, op_load64,  //28

	op_store64, op_load32, op_store32, op_load16,  //32
	op_store16, op_load8, op_store8, op_debug, // 36

	op_debug_halt, op_debug_exit, op_ct_here,  op_debug_kill, //40
	op_debug_name, op_debug_char,  //44

	// op_code_count = 46,
};

static nat 
	ct_pc = 0, rt_pc = 0, 
	literal = 0, mode = 0, macro = 0, comment = 0, literalmacro = 0, include = 0,
	name_count = 0, stack_count = 0,
	ins_count = 0, rt_ins_count = 0;
static nat dict_length = 0;
static nat dict_begin = 0;
static nat word_count = 0;
static nat word_pc = 0;

static nat save[10] = {0};
static nat _[30] = {0};

static char* names[128] = {0};
static nat addresses[128] = {0};

static nat registers[128] = {0};
static nat ct_registers[128] = {0};

static byte* memory = NULL;
static byte* ct_memory = NULL;

static char dict[4096] = {0};
static nat words[4096] = {0};
static nat stack[4096] = {0};
static struct instruction instructions[4096] = {0};
static struct instruction rt_instructions[4096] = {0};




struct file_frame {
	char* file;
	nat file_length;
	nat F_index;
	nat F_begin;
};

static nat file_stack_count = 0;
static struct file_frame file_stack[128] = {0};


static bool equal(const char* s1, const char* s2) {
	return not strcmp(s1, s2);
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

static void parse(nat word_begin) {

	nat _wc = 0, _i = word_begin;
	char w[4096]={0};
	
	while (dict[_i] != ' ') {
		w[_wc++] = dict[_i++];
	}




	if (comment) { 
		if (equal(w, "endcomment")) comment = 0; 

	}

	else if (include) {

		size_t file_length = 0;
		char* file = read_file(w, &file_length);
		
		if (not file) abort();

		file_stack[file_stack_count++] = (struct file_frame) {
			.file = file, 
			.file_length = file_length, 
			.F_index = 0, 
			.F_begin = 0
		};

		//printf("\n\tMY_TOS={%s,%llu,%llu,%llu}\n\n", file_stack[file_stack_count - 1].file, file_stack[file_stack_count - 1].file_length, file_stack[file_stack_count - 1].F_index, file_stack[file_stack_count - 1].F_begin);

		include = false;

	}

	else if (macro) { 
		if (equal(w, "endmacro")) macro = 0; 

	} else if (equal(w, "endmacro")) { 
		word_pc = stack[--stack_count]; 

	} else if (literal) { 
		nat length = strlen(w);
		ct_registers[*_] = string_to_number(w, &length); 
		literal = 0;
	} 



	else if (equal(w, "pass")) {}



	else if (equal(w, "abort")) {abort(); }
	else if (equal(w, "debugabort1")) {printf(red "DEBUG_ABORT1();\n" reset); }
	else if (equal(w, "debugabort2")) {printf(cyan "DEBUG_ABORT2();\n" reset); }
	else if (equal(w, "debugabort3")) {printf(yellow "DEBUG_ABORT3();\n" reset); }
	else if (equal(w, "debugabort4")) {printf(green "DEBUG_ABORT4();\n" reset); }
	else if (equal(w, "debugabort5")) {printf(magenta "DEBUG_ABORT5();\n" reset); }

	else if (equal(w, "11")) { _[0] = _[1]; }
	else if (equal(w, "21")) { _[0] = _[2]; }
	else if (equal(w, "00")) { _[1] = _[0]; }
	else if (equal(w, "02")) { _[1] = _[2]; }
	else if (equal(w, "000")) { _[1] = _[0]; _[2] = _[0]; }
	else if (equal(w, "001")) { _[2] = _[1]; _[1] = _[0]; }
	else if (equal(w, "021")) { nat t1 = _[1]; _[1] = _[2]; _[2] = t1; }
	else if (equal(w, "201")) { nat t2 = _[2]; _[2] = _[1]; _[1] = _[0]; _[0] = t2; }
	else if (equal(w, "120")) { nat t0 = _[0]; _[0] = _[1]; _[1] = _[2]; _[2] = t0; }

	else if (equal(w, "save0")) { save[0] = _[0]; }    // can these be done using the ct framework? no...?
	else if (equal(w, "give0")) { _[0] = save[0]; }
	else if (equal(w, "save1")) { save[1] = _[0]; }
	else if (equal(w, "give1")) { _[0] = save[1]; }

	else if (equal(w, "swap1")) { nat t0 = _[0]; _[0] = _[1]; _[1] = t0; }
	else if (equal(w, "swap2")) { nat t0 = _[0]; _[0] = _[2]; _[2] = t0; }
	else if (equal(w, "swap3")) { nat t0 = _[0]; _[0] = _[3]; _[3] = t0; }
	else if (equal(w, "swap4")) { nat t0 = _[0]; _[0] = _[4]; _[4] = t0; }
	else if (equal(w, "swap5")) { nat t0 = _[0]; _[0] = _[5]; _[5] = t0; }
	else if (equal(w, "swap6")) { nat t0 = _[0]; _[0] = _[6]; _[6] = t0; }
	else if (equal(w, "swap7")) { nat t0 = _[0]; _[0] = _[7]; _[7] = t0; }
	
	else if (equal(w, "nop")) ins(op_nop);
	else if (equal(w, "xor")) ins(op_xor);
	else if (equal(w, "add")) ins(op_add);
	else if (equal(w, "sll")) ins(op_sll);
	else if (equal(w, "srl")) ins(op_srl);
	else if (equal(w, "sub")) ins(op_sub);
	else if (equal(w, "mul")) ins(op_mul);
	else if (equal(w, "div")) ins(op_div);
	else if (equal(w, "rem")) ins(op_rem);
	else if (equal(w, "slt")) ins(op_slt);
	else if (equal(w, "blt")) ins(op_blt);
	else if (equal(w, "bne")) ins(op_bne);
	else if (equal(w, "beq")) ins(op_beq);
	else if (equal(w, "bge")) ins(op_bge);
	else if (equal(w, "slti")) ins(op_slti);
	else if (equal(w, "addi")) ins(op_addi);
	else if (equal(w, "slli")) ins(op_slli);
	else if (equal(w, "load64")) ins(op_load64);
	else if (equal(w, "load32")) ins(op_load32);
	else if (equal(w, "load16")) ins(op_load16);
	else if (equal(w, "load8")) ins(op_load8);
	else if (equal(w, "store64")) ins(op_store64);
	else if (equal(w, "store32")) ins(op_store32);
	else if (equal(w, "store16")) ins(op_store16);
	else if (equal(w, "store8")) ins(op_store8);

	else if (equal(w, "print")) ins(op_debug);
	else if (equal(w, "debugexit")) ins(op_debug_exit);
	else if (equal(w, "debughalt")) ins(op_debug_halt);
	else if (equal(w, "debugname")) ins(op_debug_name);
	else if (equal(w, "debugchar")) ins(op_debug_char);
	else if (equal(w, "debugkill")) ins(op_debug_kill);
	else if (equal(w, "here")) ins(op_ct_here);
	
	else if (equal(w, "include")) include = 1;
	else if (equal(w, "literal")) literal = 1;
	else if (equal(w, "literalmacro")) literalmacro = 1;
	else if (equal(w, "comment")) comment = 1;
	else if (equal(w, "now")) mode = 1 << 8;
	else if (equal(w, "cthere")) ct_registers[*_] = ins_count; 
	else if (equal(w, "define")) { addresses[*_] = word_pc; macro = 1; }
	else if (equal(w, "use")) { 
		names[name_count++] = strdup(""); 
		nat i = sizeof _ / sizeof(nat) - 1;
		while (i) { _[i] = _[i - 1]; i--; } *_ = name_count;
	}
	else if (equal(w, "undefine")) { free(names[*_]); names[*_] = NULL; addresses[*_] = 0; }
	
	else {
		printf("I AM HERE:  %s\n", w);
		nat name = 0; 
		nat open = name_count;
		while (name < name_count) {
			if (names[name]) {
				if (equal(w, names[name])) break;
			} else if (open == name_count) open = name;
			name++;
		}
		if (name == name_count) {
			if (open == name_count) name_count++;
			names[open] = strdup(w);
			name = open;
		} else if (not literalmacro and addresses[name]) {
			stack[stack_count++] = word_pc;
			word_pc = addresses[name];
			goto advance;
		} else if (literalmacro) literalmacro = 0;
		nat i = sizeof _ / sizeof(nat) - 1;
		while (i) { _[i] = _[i - 1]; i--; } *_ = name;
	}

advance: word_pc++;

}





static void lex() {

top:;

	//printf("lexing [%llu]\n", file_stack_count - 1);

	struct file_frame* tos;

	tos = file_stack + file_stack_count - 1;

	//printf("\n\ttos={%s,%llu,%llu,%llu}\n\n", tos->file, tos->file_length, tos->F_index, tos->F_begin);


	while (tos->F_index < tos->file_length and isspace(tos->file[tos->F_index])) tos->F_index++; 

	tos->F_begin = tos->F_index;     // ?....

	while (tos->F_index < tos->file_length) {

		if (not isspace(tos->file[tos->F_index])) {
			dict[dict_length++] = tos->file[tos->F_index++];
			continue;
		}

	push:	if (dict_length == dict_begin) goto done;
		dict[dict_length++] = ' ';
		words[word_count++] = dict_begin;

		// parse(word) should happen here.

		while (tos->F_index < tos->file_length and isspace(tos->file[tos->F_index])) tos->F_index++; 
		tos->F_begin = tos->F_index;
		dict_begin = dict_length;

		tos = file_stack + file_stack_count - 1;
		while (word_pc < word_count) parse(words[word_pc]);
		tos = file_stack + file_stack_count - 1;
	}
	if (tos->F_begin != tos->F_index) goto push;
	

done:
	munmap(tos->file, tos->file_length);


	file_stack_count--;

	if (file_stack_count == 0) {
		//printf("all done lexing!!!\n");
		// we are done! we finished the main file. 
		// move on to cteval.
	} else goto top; // else, finish lexing the previous file. 
}






#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"

static void execute_ct_instruction(struct instruction I) {

	//printf("LLL\nexecutingCT: [%llu,PRE,%llu]\n\nLLL", ct_pc, I.op);

	if (not (I.op >> 8)) { rt_instructions[rt_ins_count++] = I; goto done; }
	const nat op = I.op & (nat)~(1 << 8);
	nat* r = ct_registers, * ctr = ct_registers;
	byte* m = ct_memory;

	//printf("LLL\nexecutingCT: [%llu,%llu]\n\nLLL", ct_pc, op);

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
	else if (op == op_blt) { if (r[I._0] < r[I._1]) ct_pc += (ctr[I._2] - ct_pc) - 1; }
	else if (op == op_bge) { if (r[I._0] >= r[I._1]) ct_pc += (ctr[I._2] - ct_pc) - 1; }
	else if (op == op_bne) { if (r[I._0] != r[I._1]) ct_pc += (ctr[I._2] - ct_pc) - 1; }
	else if (op == op_beq) { if (r[I._0] == r[I._1]) ct_pc += (ctr[I._2] - ct_pc) - 1; }
	else if (op == op_debug) printf("CT#%llu=%lld\n", I._0, r[I._0]);
	else if (op == op_debug_kill) r[I._0] = 0xF0F0F0F0F0F0F0F0;
	else if (op == op_debug_halt) ct_pc = ins_count - 1;
	else if (op == op_debug_exit) 
		//printf("---exiting now!!!!!!!-----NOTICE MEEEEEE PLEASEEEE\n");//
		{exit(0);}
	else if (op == op_debug_name) {
		//printf("HERE IT IS: >>>> %s <<<\nXXX\nXXX", names[I._0]);
		printf("%s\n", names[I._0]);
		//if (equal(names[I._0], "testing")) abort();
		//if (equal(names[I._0], "bubbles")) abort();
	}
	else if (op == op_debug_char) printf("%c", (char) r[I._0]);
	else if (op == op_ct_here) ctr[I._0] = rt_ins_count;
	else { puts("unknown CT instruction"); abort(); }
done: 	ct_pc++;
}

static void execute_instruction(struct instruction I) {
	
	const nat op = I.op;
	nat* r = registers, * ctr = ct_registers;
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
	else if (op == op_blt) { if (r[I._0] < r[I._1]) rt_pc += (ctr[I._2] - rt_pc) - 1; }
	else if (op == op_bge) { if (r[I._0] >= r[I._1]) rt_pc += (ctr[I._2] - rt_pc) - 1; }
	else if (op == op_bne) { if (r[I._0] != r[I._1]) rt_pc += (ctr[I._2] - rt_pc) - 1; }
	else if (op == op_beq) { if (r[I._0] == r[I._1]) rt_pc += (ctr[I._2] - rt_pc) - 1; }
	else if (op == op_debug) printf("R#%llu=%lld\n", I._0, r[I._0]);
	else if (op == op_debug_kill) r[I._0] = 0xF0F0F0F0F0F0F0F0;
	else if (op == op_debug_halt) rt_pc = ins_count - 1;
	else if (op == op_debug_exit) exit(0);
	else if (op == op_debug_name) 
		//printf("RUNTIME HERE IS: >>>> %s <<<\nXXX\nXXX", names[I._0]);
		printf("%s\n", names[I._0]);
	else if (op == op_debug_char) printf("%c", (char) r[I._0]);
	else { puts("unknown RT instruction"); abort(); }
	rt_pc++;
}

#pragma clang diagnostic pop

static void resetenv() {
	dict_length = 0; dict_begin = 0;
	word_pc = 0; ct_pc = 0; rt_pc = 0;
	literal = 0; mode = 0; macro = 0; comment = 0; literalmacro = 0; include = 0;
	name_count = 0; stack_count = 0;
	word_count = 0; ins_count = 0; rt_ins_count = 0;
	memset(save, 0, sizeof save);
	memset(_, 0, sizeof _);
	memset(names, 0, sizeof names);
	memset(addresses, 0, sizeof addresses);
	memset(registers, 0xF0, sizeof registers);
	memset(ct_registers, 0xF0, sizeof ct_registers);
	memset(stack, 0, sizeof stack);
	memset(words, 0, sizeof words);
	memset(dict, 0, sizeof dict);
	memset(instructions, 0, sizeof instructions);
	memset(rt_instructions, 0, sizeof rt_instructions);
	free(memory);
	memory = aligned_alloc(8, 1 << 16);
	free(ct_memory);
	ct_memory = aligned_alloc(8, 1 << 16);
}




static void check_for_mistakes() {

	bool error = false;
	for (nat i = 0; i < name_count; i++) {
		const nat uninitialized_value = 0xF0F0F0F0F0F0F0F0;
		const nat zero_value = 0;
		const bool C = not memcmp(ct_registers + i, &uninitialized_value, 8);
		const bool R = not memcmp(   registers + i, &uninitialized_value, 8);
		const bool A = not memcmp(   addresses + i, &zero_value, 8);

		if (C and R and A and names[i]) {
			printf("error: register \"%s\" unused.\n", names[i]);
			error = true;
		}
	}

	if (error) puts("ERROR: compiliation failed.");

}

static void interpret(char* text, nat text_length) {
	
	file_stack[file_stack_count++] = (struct file_frame) {
		.file = text, 
		.file_length = text_length, 
		.F_index = 0, 
		.F_begin = 0
	};

	lex();

	while (ct_pc < ins_count) execute_ct_instruction(instructions[ct_pc]);
	while (rt_pc < rt_ins_count) execute_instruction(rt_instructions[rt_pc]);

	check_for_mistakes();
}














int main() {

	puts("a repl/interpreter for my language.");
	
	   memory = aligned_alloc(8, 1 << 16);
	ct_memory = aligned_alloc(8, 1 << 16);
	memset(   registers, 0xF0, sizeof    registers);
	memset(ct_registers, 0xF0, sizeof ct_registers);
	memset(addresses, 0, sizeof addresses);

	char line[4096] = {0};
	
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

		char filename[4096] = {0};
		printf("filename: ");
		fgets(filename, sizeof filename, stdin);
		filename[strlen(filename) - 1] = 0;
		size_t length = 0;
		char* contents = read_file(filename, &length);
		if (contents) interpret(contents, length);

	} else if (equal(string, "i") or equal(string, "interpret")) {

		char filename[4096] = {0};
		printf("filename: ");
		fgets(filename, sizeof filename, stdin);
		filename[strlen(filename) - 1] = 0;
		size_t length = 0;
		char* contents = read_file(filename, &length);
		if (contents) { resetenv(); interpret(contents, length); }
	
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
		

		printf("state: ");

	printf("\n\t");

		printf("w_pc=%llu ", word_pc);
		printf("ct_pc=%llu ", ct_pc);
		printf("rt_pc=%llu ", rt_pc);

	printf("\n\t");

		printf("literal=%llu ", literal);
		printf("mode=%llu ", mode);
		printf("macro=%llu ", macro);
		printf("comment=%llu ", comment);
		printf("literalmacro=%llu ", literalmacro);
		printf("include=%llu ", include);

	printf("\n\t");

		printf("name_count=%llu ", name_count);
		printf("stack_count=%llu ", stack_count);

	printf("\n\t");

		printf("word_count=%llu ", word_count);
		printf("ins_count=%llu ", ins_count);
		printf("rt_ins_count=%llu ", rt_ins_count);

	printf("\n\t");

		printf("dict_length=%llu ", dict_length);	
		printf("dict_begin=%llu ", dict_begin);	

	printf("\n");



	} else if (equal(string, "debugmemory")) {
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

	else if (equal(string, "debugdict")) {
		puts("printing dictionary:");
		puts(dict);
	}

	else if (equal(string, "debugwords")) {
		for (nat i = 0; i < word_count; i++) {
			printf(" %llu ", i);
			nat c = words[i];

			if (i == word_pc) printf(green " * " reset);
			else printf(" - ");

			while (dict[c] != ' ') {
				putchar(dict[c]);
				c++;
			}

			putchar(10);
		}
	}

	else interpret(line, line_length);

	goto _;
done: 	printf("quitting...\n");
}







