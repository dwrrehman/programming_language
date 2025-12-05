// programming language compiler 
// written on 2411203.160950 dwrr

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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef uint64_t nat;

enum language_isa {
	nullins, 
	zero, incr, decr, set, add, sub, mul, div_, rem, 
	not_,and_, or_, eor, si, sd, lt, ge, ne, eq, 
	ld, st, sta, bca, sc, at, lf, eoi,
	isa_count
};

static const char* ins_spelling[isa_count] = {
	"_nullins_", 
	"zero", "incr", "decr", "set", "add", "sub", "mul", "div", "rem", 
	"not", "and", "or", "eor", "si", "sd", "lt", "ge", "ne", "eq", 
	"ld", "st", "sta", "bca", "sc", "at", "lf", "eoi",
};

enum language_builtins {
	nullvar,
	discardunused, stackpointer, stacksize,
	builtin_count
};

static const char* builtin_spelling[builtin_count] = {
	"(nv)",
	"_discardunused", 
	"_process_stackpointer", "_process_stacksize",
};

struct instruction {
	nat* args;
	nat count;
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

struct node {
	nat* data_outputs;
	nat data_output_count;
	nat* data_inputs;
	nat data_input_count;
	nat output_reg;
	nat op;
	nat statically_known;
};

struct basic_block {
	nat* data_outputs;
	nat data_output_count;
	nat* data_inputs;
	nat data_input_count;
	nat* predecessors;
	nat predecessor_count;
	nat successor;
	nat dag_count;
	nat* dag;
};

static nat isa_arity(nat i) {
	if (i == sc) return 7;
	if (not i or i == eoi) return 0;
	if (i == incr or i == decr or i == zero or i == not_ or i == lf or i == at) return 1;
	if (i == lt or i == ge or i == ne or i == eq or i == ld or i == st) return 3;
	return 2;
}

static void print_nodes(struct node* nodes, nat node_count) {
	printf("printing %3llu nodes...\n", node_count);
	for (nat n = 0; n < node_count; n++) {

		printf("[%s] node #%-5llu: {"
			".opcode=%2llu (\"\033[35;1m%-10s\033[0m\") "
			".outreg=%2llu (\"\033[36;1m%-4llu\033[0m\") "
			".oc=%2llu "
			".ic=%2llu "
			".io={ ", 
			nodes[n].statically_known ? "\033[32;1mSK\033[0m" : "  ", n, 
			nodes[n].op, ins_spelling[nodes[n].op],
			nodes[n].output_reg, nodes[n].output_reg,
			nodes[n].data_output_count,
			nodes[n].data_input_count
		);

		for (nat j = 0; j < nodes[n].data_output_count; j++) {
			printf("%llu ", nodes[n].data_outputs[j]);
		}
		printf(" | ");

		for (nat j = 0; j < nodes[n].data_input_count; j++) {
			printf("%llu ", nodes[n].data_inputs[j]);
		}
		puts(" } }");		
	}
	puts("done");
}

static void print_basic_blocks(struct basic_block* blocks, nat block_count, 
			struct node* nodes
) {
	puts("blocks:");
	for (nat b = 0; b < block_count; b++) {
		printf("block #%3llu: {.count = %llu, .dag = { ", b, blocks[b].dag_count);
		for (nat d = 0; d < blocks[b].dag_count; d++) 
			printf("%3llu ", blocks[b].dag[d]);
		puts("}");
	}
	puts("[end of cfg]");

	puts("printing out cfg with node data: ");
	for (nat b = 0; b < block_count; b++) {
		printf("block #%5llu:\n", b);
		for (nat d = 0; d < blocks[b].dag_count; d++) {
			printf("\tnode %3llu:   \033[32;1m%7s\033[0m  %3llu(\"%4llu\") %3llu %3llu\n\n", 
				blocks[b].dag[d], ins_spelling[nodes[blocks[b].dag[d]].op], 
				nodes[blocks[b].dag[d]].output_reg, 
				nodes[blocks[b].dag[d]].output_reg, 
				(nat) -1,//nodes[blocks[b].dag[d]].input0, 
				(nat) -1//nodes[blocks[b].dag[d]].input1 
			);
		}
		puts("}");
	}
	puts("[end of node cfg]");
}

static void debug_instructions(struct instruction* ins, nat ins_count) {
	printf("instructions: (%llu count) \n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("ins[%llu] = { ", i);
		for (nat a = 0; a < ins[i].count; a++) {

			const char* str = "";
			if (a == 0 and ins[i].args[0] < isa_count) str = ins_spelling[ins[i].args[0]];

			 printf(".args[%llu]=%lld %s      ", a, ins[i].args[a], str);
		}
		puts("} ");
	}
	puts("done\n");
}

static void debug_dictionary(char** names, bool* active, nat name_count) {
	printf("dictionary: %llu\n", name_count);
	for (nat i = 0; i < name_count; i++) 
		printf("[ %c ] var #%5llu:   %10s\n", active[i] ? ' ' : 'd', i, names[i]);
	puts("done printing dictionary.");
}

int main(int argc, const char** argv) {

	if (argc > 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));
	
	char* names[4096] = {0};
	nat name_count = 0;

	struct instruction* ins = NULL;
	nat ins_count = 0;

	struct file filestack[4096] = {0};
	nat filestack_count = 1;
	const char* included_files[4096] = {0};
	nat included_file_count = 0;

	int file = open(argv[1], O_RDONLY);
	if (file < 0) { puts(argv[1]); perror("open"); exit(1); }
	const nat text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(text_length + 1, 1);
	read(file, text, text_length);
	close(file);
	filestack[0].filename = argv[1];
	filestack[0].text = text;
	filestack[0].text_length = text_length;
	filestack[0].index = 0;

	for (nat i = 0; i < builtin_count; i++) {
		active[name_count] = 1;
		names[name_count++] = strdup(builtin_spelling[i]);
	}

process_file:;
	nat word_length = 0, word_start = 0, expecting_var = 0;
	const nat starting_index = 	filestack[filestack_count - 1].index;
	const nat text_length = 	filestack[filestack_count - 1].text_length;
	char* text = 			filestack[filestack_count - 1].text;
	const char* filename = 		filestack[filestack_count - 1].filename;

	for (nat index = starting_index; index < text_length; index++) {
		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++; 
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;
		char* word = strndup(text + word_start, word_length);
		nat calling = 0;

		if (not expecting_var) {
			for (nat i = 0; i < isa_count; i++) 
				if (not strcmp(ins_spelling[i], word)) { calling = i; goto found; }
			goto next_word;

		} else {
			const nat my_op = ins[ins_count - 1].args[0];
			if (my_op == def or my_op == lf) goto create_name;
			for (nat i = name_count; i--;) 
				if (active[i] and not strcmp(names[i], word)) { calling = i; goto found; } 
			printf("fatal error: %s:%llu: undefined variable: %s\n", filename, index, word);
			abort();

		create_name:
			calling = name_count;
			active[name_count] = 1;
			names[name_count++] = word;
		}
	found:
		if (not expecting_var) {
			ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
			ins[ins_count++] = (struct instruction) {0};
			expecting_var = 1;
		}
		struct instruction* this = ins + ins_count - 1;
		this->args = realloc(this->args, sizeof(nat) * (this->count + 1));
		this->args[this->count++] = calling;
		if (this->count != isa_arity(this->args[0]) + 1) goto next_word;
		if (this->args[0] == lf) {
			ins_count--;
			active[this->args[1]] = 0;
			for (nat i = 0; i < included_file_count; i++) {
				if (strcmp(included_files[i], word)) continue;
				printf("warning: %s: file already included\n", word);
				goto finish_instruction;
			}

			included_files[included_file_count++] = word;
			int file = open(word, O_RDONLY);
			if (file < 0) { puts(word); perror("open"); exit(1); }
			const nat new_text_length = (nat) lseek(file, 0, SEEK_END);
			lseek(file, 0, SEEK_SET);
			char* new_text = calloc(new_text_length + 1, 1);
			read(file, new_text, new_text_length);
			close(file);
			filestack[filestack_count - 1].index = index;
			filestack[filestack_count].filename = word;
			filestack[filestack_count].text = new_text;
			filestack[filestack_count].text_length = new_text_length;
			filestack[filestack_count++].index = 0;
			goto process_file;

		} else if (this->args[0] == eoi) {
			ins_count--;
			break;

		}
		finish_instruction: expecting_var = 0;
		next_word: word_length = 0;
	}

	filestack_count--;
	if (filestack_count) goto process_file;





/*


	puts("finding label attrs...");
	nat* locations = calloc(name_count, sizeof(nat));
	for (nat pc = 0; pc < ins_count; pc++) {
		if (ins[pc].args[0] == at) {
			const nat d = ins[pc].args[1];
			printf("executed AT instruction: assigned labels[%llu].value = %llu...\n", d, pc);
			locations[d] = pc;
		}
	}

	puts("locations: ");
	for (nat i = 0; i < name_count; i++) 
		printf("locations[%llu] = %llu\n", i, locations[i]);
	puts("done");





	struct node nodes[4096] = {0}; 
	nat node_count = 0;
	nodes[node_count++] = (struct node) { .output_reg = 0, .statically_known = 1 };
	//nodes[node_count++] = (struct node) { .output_reg = var_stacksize, .statically_known = 1 };


	puts("stage: constructing data flow DAG...");

	struct basic_block blocks[4096] = {0};
	nat block_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].args[0];
		printf("generating DAG node for ins: { %s ", ins_spelling[op]);
		for (nat a = 0; a < ins[i].count; a++) printf(" %llu   ", ins[i].args[a]);
		// const nat output_reg = d;
		const nat statically_known = 0; //(nodes[input0].statically_known and nodes[input1].statically_known);
		printf("statically_known = %llu\n", statically_known);
		if (ins[i].args[0] == at and blocks[block_count].dag_count) block_count++;
		struct basic_block* block = blocks + block_count;
		block->dag = realloc(block->dag, sizeof(nat) * (block->dag_count + 1));
		block->dag[block->dag_count++] = node_count;

		nodes[node_count++] = (struct node) {
			.data_outputs = NULL,
			.data_output_count = 0,
			.data_inputs = NULL,
			.data_input_count = 0,
			.op = ins[i].args[0],
			.statically_known = statically_known,
		};

		printf("generated node #%llu into block %llu (of %llu): \n", 
			node_count, block_count, block->dag_count);

		puts("inputs and outputs null for now");

		if (	ins[i].args[0] == lt or 
			ins[i].args[0] == ge or 
			ins[i].args[0] == ne or 
			ins[i].args[0] == eq or
			ins[i].args[0] == lts or 
			ins[i].args[0] == ges
		) block_count++;
	}
 	block_count++;
	

	puts("done creating isa nodes... printing nodes:");
	print_nodes(nodes, node_count);

	puts("done creating basic blocks... printing cfg/dag:");
	print_basic_blocks(blocks, block_count, nodes);


*/







	exit(1);









	nat* R = calloc(name_count, sizeof(nat));
	R[stacksize] = 65536;
	R[stackpointer] = (nat) (void*) malloc(65536);

	for (nat pc = 0; pc < ins_count; pc++) {
		if (ins[pc].args[0] == at) {
			const nat d = ins[pc].args[1];
			R[d] = pc;
		}
	}
	for (nat pc = 0; pc < ins_count; pc++) {
		const nat op = ins[pc].args[0];
		nat d = ins[pc].count >= 2 ? ins[pc].args[1] : 0;
		nat r = ins[pc].count >= 3 ? ins[pc].args[2] : 0;
		nat s = ins[pc].count >= 4 ? ins[pc].args[3] : 0;

		if (not op) {}
		else if (op == at) R[d] = pc;
		else if (op == zero) R[d] = 0;
		else if (op == incr) R[d]++;
		else if (op == decr) R[d]--;
		else if (op == not_) R[d] = ~R[d];
		else if (op == set)  R[d]  = R[r];
		else if (op == add)  R[d] += R[r];
		else if (op == sub)  R[d] -= R[r];
		else if (op == mul)  R[d] *= R[r];
		else if (op == div_) R[d] /= R[r];
		else if (op == rem)  R[d] %= R[r];
		else if (op == si)   R[d]<<= R[r];
		else if (op == sd)   R[d]>>= R[r];
		else if (op == sds)  R[d]>>= R[r];
		else if (op == and_) R[d] &= R[r];
		else if (op == or_)  R[d] |= R[r];
		else if (op == eor)  R[d] ^= R[r];

		else if (op == st) {
			     if (R[s] == 1) { *( uint8_t*)(R[d]) = ( uint8_t)R[r]; }
			else if (R[s] == 2) { *(uint16_t*)(R[d]) = (uint16_t)R[r]; }
			else if (R[s] == 4) { *(uint32_t*)(R[d]) = (uint32_t)R[r]; }
			else if (R[s] == 8) { *(uint64_t*)(R[d]) = (uint64_t)R[r]; }
			else abort();

		} else if (op == ld) {
			     if (R[s] == 1) { R[d] = *( uint8_t*)(R[r]); }
			else if (R[s] == 2) { R[d] = *(uint16_t*)(R[r]); }
			else if (R[s] == 4) { R[d] = *(uint32_t*)(R[r]); }
			else if (R[s] == 8) { R[d] = *(uint64_t*)(R[r]); }
			else abort();
		}
		else if (op == lt)  { if (R[d] < R[r])  { pc = R[s]; } } 
		else if (op == ge)  { if (R[d] >= R[r]) { pc = R[s]; } } 
		else if (op == lts) { if (R[d] < R[r])  { pc = R[s]; } } 
		else if (op == ges) { if (R[d] >= R[r]) { pc = R[s]; } } 
		else if (op == ne)  { if (R[d] != R[r]) { pc = R[s]; } } 
		else if (op == eq)  { if (R[d] == R[r]) { pc = R[s]; } } 
		else if (op == do_)  { pc = R[d]; } 
		else if (op == sc) {
			const nat n = R[ins[pc].args[1]];
			const nat a0 = ins[pc].args[2];
			const nat a1 = ins[pc].args[3];
			const nat a2 = ins[pc].args[4];
			const nat a3 = ins[pc].args[5];
			const nat a4 = ins[pc].args[6];
			const nat a5 = ins[pc].args[7];

			if (n == system_call_undefined) {
				     if (R[a1] == 0) printf("\033[32;1mdebug:   0x%llx : %lld\033[0m\n", R[a0], R[a0]); 
				else if (R[a1] == 1) debug_registers(R, name_count);
				else if (R[a1] == 2) debug_dictionary(names, active, name_count);
				else if (R[a1] == 3) debug_instructions(ins, ins_count);

			} else if (n == system_exit) 	exit((int) R[a0]);
			else if (n == system_fork) 	R[a0] = (nat) fork(); 
			else if (n == system_openat) 	R[a0] = (nat) openat((int) R[a0], (const char*) R[a1], (int) R[a2], (int) R[a3]);
			else if (n == system_close) 	R[a0] = (nat) close((int) R[a0]);
			else if (n == system_read) 	R[a0] = (nat) read((int) R[a0], (void*) R[a1], (size_t) R[a2]);
			else if (n == system_write) 	R[a0] = (nat) write((int) R[a0], (void*) R[a1], (size_t) R[a2]);
			else if (n == system_lseek) 	R[a0] = (nat) lseek((int) R[a0], (off_t) R[a1], (int) R[a2]);
			else if (n == system_mmap) 	R[a0] = (nat) (void*) mmap((void*) R[a0], (size_t) R[a1], (int) R[a2], (int) R[a3], (int) R[a4], (off_t) R[a5]);
			else if (n == system_munmap) 	R[a0] = (nat) munmap((void*) R[a0], (size_t) R[a1]); 
			else {
				puts("bad system call");
				printf("%llu: system call not supported.\n", n);
				abort();
			}
		}
		else {
			printf("error: executing unknown instruction: %llu (%s)\n", op, ins_spelling[op]);
			abort();
		}
	}
}




















































































/*
	insert_u32(&data, &count, LC_UNIXTHREAD);
	insert_u32(&data, &count, 288);
	insert_u32(&data, &count, ARM_THREAD_STATE64);
	insert_u32(&data, &count, 2 * (32 + 2));
	for (nat i = 0; i < 32; i++) {
		if (i == 16) insert_u64(&data, &count, 0x1);
		else if (i == 0) insert_u64(&data, &count, 0x42);
		else insert_u64(&data, &count, 0);
	}
	insert_u64(&data, &count, 0x0000000100000000 + 16384 - 16);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
*/









	//system("codesign --remove-signature output_executable_new");



	//uint8_t* code_bytes = 0;//(uint8_t*) "hello world ashtasht";
	//const nat code_byte_count = 4096;//(nat) strlen("hello world ashtasht");
	//for (nat i = 0; i < code_byte_count; i++) insert_byte(&data, &count, (uint8_t) (rand() % 0xFF));
	//while (count < 4096 or count % 4096) insert_byte(&data, &count, 0);




//	printf("code_start = %llu, code_end = %llu\n", code_start, code_end);
//	puts("offsets");
//	printf("\t command_end - command_start = %llu\n", command_end - command_start);
//	printf("\t pagezero_end - pagezero_start = %llu\n", pagezero_end - pagezero_start);
//	printf("\t text_end - text_start = %llu\n", text_end - text_start);
//	printf("\t linkedit_end - linkedit_start = %llu\n", linkedit_end - linkedit_start);
//	printf("\t thread_end - thread_start = %llu\n", thread_end - thread_start);
//	printf("\t code_end - code_start = %llu\n", code_end - code_start);
//



//	const nat thread_start = count;
//	insert_u32(&data, &count, LC_UNIXTHREAD);
//	insert_u32(&data, &count, 288);                       // thread_end - thread_start
//	insert_u32(&data, &count, ARM_THREAD_STATE64);
//	insert_u32(&data, &count, 2 * (32 + 2));
//	for (nat i = 0; i < 32; i++) {
//		if (i == 16) insert_u64(&data, &count, 0x1);
//		else if (i == 0) insert_u64(&data, &count, 0x42);
//		else insert_u64(&data, &count, 0);
//	}
//	insert_u64(&data, &count, 0x0000000100000000 + 16340); // pc
//	insert_u32(&data, &count, 0); // cpsr
//	insert_u32(&data, &count, 0);
//	const nat thread_end = count;




	//printf("\t uuid_end - uuid_start = %llu\n", uuid_end - uuid_start);
	//printf("\t code_signature_end - code_signature_start = %llu\n", code_signature_end - code_signature_start);	
	//printf("\t signature_end - signature_start = %llu\n", signature_end - signature_start);	










/*
100003f80: ff 83 00 d1 	sub	sp, sp, #32
100003f84: fd 7b 01 a9 	stp	x29, x30, [sp, #16]
100003f88: fd 43 00 91 	add	x29, sp, #16
100003f8c: a8 00 80 52 	mov	w8, #5

100003f90: e8 03 00 f9 	str	x8, [sp]
100003f94: 20 00 80 52 	mov	w0, #1
100003f98: 05 00 00 94 	bl	0x100003fac <_syscall+0x100003fac>
100003f9c: 00 00 80 52 	mov	w0, #0

100003fa0: fd 7b 41 a9 	ldp	x29, x30, [sp, #16]
100003fa4: ff 83 00 91 	add	sp, sp, #32
100003fa8: c0 03 5f d6 	ret
*/

/*
	const nat code_signature_start = count;
	insert_u32(&data, &count, LC_CODE_SIGNATURE);
	insert_u32(&data, &count, 16);    // code_signature_end - code_signature_start
	insert_u32(&data, &count, 16384);
	insert_u32(&data, &count, 800);
	const nat code_signature_end = count;
*/






	//system("otool -txvVhlL object0.o");
	//system("otool -txvVhlL executable0.out");
	//system("objdump object0.o -DSast --disassembler-options=no-aliases");
	//system("objdump executable0.out -DSast --disassembler-options=no-aliases");





/*











//#define LC_CODE_SIGNATURE	0x1d
//#define LC_SOURCE_VERSION 	0x2A
// #define ARM_THREAD_STATE64      6

//          /usr/lib/libSystem.B.dylib        the lib we want to use, lets say. 
//          /usr/lib/dyld                  the linker we want to use, lets say. 


struct section_64 { * for 64-bit architectures *
	char		sectname[16];	* name of this section *
	char		segname[16];	* segment this section goes in *
	uint64_t	addr;		* memory address of this sectio *
	uint64_t	size;		* size in bytes of this section 
	uint32_t	offset;		* file offset of this section *
	uint32_t	align;		* section alignment (power of 2) *
	uint32_t	reloff;		* file offset of relocation entries *
	uint32_t	nreloc;		* number of relocation entries *
	uint32_t	flags;		* flags (section type and attributes)*
	uint32_t	reserved1;	* reserved (for offset or index) *
	uint32_t	reserved2;	* reserved (for count or sizeof) *
	uint32_t	reserved3;	* reserved *
};



struct symtab_command {
	uint_32 cmd;
	uint_32 cmdsize;
	uint_32 symoff;
	uint_32 nsyms;
	uint_32 stroff;
	uint_32 strsize;
};

struct entry_point_command {
    uint32_t  cmd;	* LC_MAIN only used in MH_EXECUTE filetypes *
    uint32_t  cmdsize;	* 24 *
    uint64_t  entryoff;	* file (__TEXT) offset of main() *
    uint64_t  stacksize;* if not zero, initial stack size *
};

struct symtab_command {
	uint32_t	cmd;		* LC_SYMTAB *
	uint32_t	cmdsize;	* sizeof(struct symtab_command) *
	uint32_t	symoff;		* symbol table offset *
	uint32_t	nsyms;		* number of symbol table entries *
	uint32_t	stroff;		* string table offset *
	uint32_t	strsize;	* string table size in bytes *
};

struct dysymtab_command {
    uint32_t cmd;	* LC_DYSYMTAB *
    uint32_t cmdsize;	* sizeof(struct dysymtab_command) *
    uint32_t ilocalsym;	* index to local symbols *
    uint32_t nlocalsym;	* number of local symbols *
    uint32_t iextdefsym;* index to externally defined symbols *
    uint32_t nextdefsym;* number of externally defined symbols *
    uint32_t iundefsym;	* index to undefined symbols *
    uint32_t nundefsym;	* number of undefined symbols *
    uint32_t tocoff;	* file offset to table of contents *
    uint32_t ntoc;	* number of entries in table of contents *
    uint32_t modtaboff;	* file offset to module table *
    uint32_t nmodtab;	* number of module table entries *
    uint32_t extrefsymoff;	* offset to referenced symbol table *
    uint32_t nextrefsyms;	* number of referenced symbol table entries *
    uint32_t indirectsymoff; * file offset to the indirect symbol table *
    uint32_t nindirectsyms;  * number of indirect symbol table entries *
    uint32_t extreloff;	* offset to external relocation entries *
    uint32_t nextrel;	* number of external relocation entries *
    uint32_t locreloff;	* offset to local relocation entries *
    uint32_t nlocrel;	* number of local relocation entries *
};	


struct dylinker_command {
	uint32_t	cmd;		* LC_ID_DYLINKER, LC_LOAD_DYLINKER o
					   LC_DYLD_ENVIRONMENT *
	uint32_t	cmdsize;	* includes pathname string /       8 + 16   with 3 padding zeros.
	union lc_str    name;		* dynamic linker's path name /   8 
};


struct dylib_command {
	uint32_t	cmd;		* LC_ID_DYLIB, LC_LOAD_{,WEAK_}DYLIB,
					   LC_REEXPORT_DYLIB *
	uint32_t	cmdsize;	* includes pathname string *

    	uint32_t  name;			* library's path name *
    	uint32_t timestamp;			* library's build time stamp *
   	uint32_t current_version;		* library's current version number *
   	uint32_t compatibility_version;	/ library's compatibility vers number*
};





#define LC_VERSION_MIN_MACOSX 0x24
struct version_min_command {
    uint32_t	cmd;		
    uint32_t	cmdsize;	
    uint32_t	version;	
    uint32_t	sdk;		
};






static void make_macho_object_file(void) {

	struct mach_header_64 header = {0};
	header.magic = MH_MAGIC_64;
	header.cputype = (int)CPU_TYPE_ARM | (int)CPU_ARCH_ABI64;
	header.cpusubtype = (int) CPU_SUBTYPE_ARM64_ALL;
	header.filetype = MH_OBJECT;
	header.ncmds = 2;
	header.sizeofcmds = 0;
	header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;

	header.sizeofcmds = 	sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	struct segment_command_64 segment = {0};
	strncpy(segment.segname, "__TEXT", 16);
	segment.cmd = LC_SEGMENT_64;
	segment.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64);
	segment.maxprot =  (VM_PROT_READ | VM_PROT_EXECUTE);
	segment.initprot = (VM_PROT_READ | VM_PROT_EXECUTE);
	segment.nsects = 1;
	segment.vmaddr = 0;
	segment.vmsize = byte_count;
	segment.filesize = byte_count;

	segment.fileoff = 	sizeof(struct mach_header_64) + 
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	struct section_64 section = {0};
	strncpy(section.sectname, "__text", 16);
	strncpy(section.segname, "__TEXT", 16);
	section.addr = 0;
	section.size = byte_count;	
	section.align = 3;
	section.reloff = 0;
	section.nreloc = 0;
	section.flags = S_ATTR_PURE_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS;

	section.offset = 	sizeof(struct mach_header_64) + 
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	const char strings[] = "\0_start\0";

	struct symtab_command table  = {0};
	table.cmd = LC_SYMTAB;
	table.cmdsize = sizeof(struct symtab_command);
	table.strsize = sizeof(strings);
	table.nsyms = 1;
	table.stroff = 0;
	
	table.symoff = (uint32_t) (
				sizeof(struct mach_header_64) +
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command) + 
				byte_count
			);

	table.stroff = table.symoff + sizeof(struct nlist_64);

	struct nlist_64 symbols[] = {
	        (struct nlist_64) {
	            .n_un.n_strx = 1,
	            .n_type = N_SECT | N_EXT,
	            .n_sect = 1,
	            .n_desc = REFERENCE_FLAG_DEFINED,
	            .n_value = 0x0000000000,
	        }
	};

	if (preserve_existing_object and not access(object_filename, F_OK)) {
		puts("asm: object_file: file exists"); 
		puts(object_filename);
		exit(1);
	}

	const int flags = O_WRONLY | O_CREAT | O_TRUNC | (preserve_existing_object ? O_EXCL : 0);
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(object_filename, flags, mode);
	if (file < 0) { perror("obj:open"); exit(1); }

	write(file, &header, sizeof(struct mach_header_64));
	write(file, &segment, sizeof (struct segment_command_64));
	write(file, &section, sizeof(struct section_64));
	write(file, &table, sizeof table);
	write(file, bytes, byte_count);
	write(file, symbols, sizeof(struct nlist_64));
	write(file, strings, sizeof strings);
	close(file);
}




*/





















































































/*



//printf("read %llu bytes from file %s...\n", new_text_length, word);
			//printf("again: contents: read %s bytes from file %s...\n", new_text, word);

	//printf("decrementing filestack_count: finished processing filestack_count=%llu...\n", filestack_count);

	//debug_dictionary(names, active, name_count);
	//puts("these instructions were generated.");
	//debug_instructions(ins, ins_count);

		//printf("@ word: (%llu,%llu):   %s\n", word_start, word_length, word);
			//printf("fatal error: %s:%llu: unexpcted word: %s\n", filename, index, word);



			printf("\n\nfile: %s, index: %llu\n", filename, index);
			printf("error: use of undefined word \"%s\", expecting %s\n", word, 
				expecting_var ? "variable" : "operation"
			);
			abort(); 



*/






/*
		printf("executing instruction: %s ", ins_spelling[op]);
		for (nat i = 0; i < isa_arity(op); i++) {
			printf("   %s   ", names[ins[pc].args[1 + i]]);
		}
		puts("");
		usleep(500000);
*/





//printf("(filestack_count=%llu): PROCESSING FILE: { starting_index=%llu : text_length=%llu : filename=%s\n", filestack_count, starting_index, text_length, filename);
	//getchar();


/*  
1202410082.204140

gunna try generating the executable now!!!

found a simple minimal executable example using LC_UNIXTHREAD which should be quite cool! yay 
	then ill try using   LC_THREAD too as well   which doesnt have a stack, which will be super cool lolllll 
		so yeah yay









1202410034.000348

instead of adding postiional syntax, instead lets add the requirement that    names must be defined  by either  the label of a barnch, or by a select few instructions, like at, zero, set.. orrrr uhhhh hmmm

	actually maybe just leave that as is? hmmm idk.. because we can resolve most problems due to this via    unused_variable  checking   and sensibility checks lol. during the backend. so yeah idk actually. hmmm








1202410034.123914

so actually i think we just need to add a way to remove names, i think!











so turns out, we only need to actually add like one more instruction i think!

	to get positional syntax for anything we want    

					(basically, theres a limit, but its reasonable lol)


	as opposed to forcing name based syntax alll the time for every  function call lol 



			basically, the pattern:




				set arg0 X
				set arg1 Y
				set arg2 Z

				at ret
					do my_function_name


				like,   thats really   very automateable, to   allow the user to simply write  the equivalent syntax:   "my_function_name X Y Z" simply.

							(for brievity only, of course, servers no other purpose really, but it makes sense for things like making constants, using the binary literal trick with groups,  or like superrr minimal operations like slt, which are a wrapper around two or one real instructions. 

			basically, the 			idea is that 


			we can just annotate the function body, which still looks like 


			at my_function_name

				...use arg0 arg1 arg2 etc in the body contents....

				incr ret do ret



			like thats still the body, no macro syntax required, 

					BUTTT


						we just need to tellll the compiler    that when you see           my_function_name, it is bothhh an operatin and variable,  the var is the at label, and the operation is now somehting new! 

		its just meaning that the front end should generate the      set arg0 ... set arg1 ... at ret do my)function_name


					like, we just generate those exact statements. simple as that. 


								i think the only problem with this now, is that we need to pass in the ret i think, or something like that, not sure. hmmm



				yeah thats the trick party lol.    i kindaa wan to have like a builtin variable, or somehting, im not sure..h mmmm



					i mean we could also pass it in as an argument, but then its kinda obvious that its not quite like... hmmm idk. 

					like, hmmmmm yeh crappp we don't know the return label lol. thats the thing hmmmmmmm dang itttt



			crappppppp 


	but yeah thats the idae 


				we can attribute the function body with something like 


							atttribute_as_macro my_function_name 3

					and the 3 means arity of 3, the first three variables used in the body are the parameters, basically. thast the idea 

	hm

					idkkkk i mean it hink it need some workkk  but its getting somewhereee 






























1202409231.164909
current language isa:

	zero, incr, decr, set, add, sub, 
	mul, muh, mhs, div, dvs, rem, rms, 
	si, sd, sds, and, or, eor, not, 
	ld, st, sta, bca, sc, at, lf, 
	lt, ge, lts, ges, ne, eq, do, 


just using these instructions, we are able to recreate the macro system. this also allows us to have code more code compression than with macros, too, becuase we can actually compress the code in a way that is still executable, like a loop vs a sequence of statements. but in our case its single function body instead of a macro which pastes it everywhere. butttttt there no real.. function... machinery though. like in typical programming languages lol. so yeah. pretty interesting. hm. 










	plan to make the language better: 1202409216.030428
		
	x	- get rid of everything to do with macros, and any high level abstraction features we added

	x	- figure out how to do arguments for builtin instructions, properly

		- implement the backend for those builtin rt instructions,
	
			- dag and cfg formation

			- unused variable detection

			- inssel, ra, inssched





	the instructions we will keep in the language are:


		zero incr decr 
		set add sub 
		mul muh mhs 
		div rem dvs rms
		si sd sds 
		and or eor not 
		ld st sc la 
		lt ge lts ges ne eq 
		sta bca at lf



	all those (basically) are required and relevant for runtime code. 

		no macro machinery,  and no   anything   that is high level lol.


	also we need to decide if we want to have        multiple symbol tables per type.  

				ie, contexts.



			so yeah, we need to decide that. hmmm



	












todo 1202408283.031842

*	- command line arguments passed in registers?
*	- generating a mach-o executable directly.

**	- add comments
**	- add strings. 

*	- write a print_binary_lsb_number function for the stdlib


// done:

X	- adding system calls to our language, by adding them as instructions!
x **	- implement sta and bca in the language isa. 
x ***	- fix macro and control flow  <------ create new names each macro call.
//					x	implement     lf       please.  then we are done. 





also i want to implement these things in the language, after we do strings/comments         (if we do them lol)




	- label unused

	- label has no definition


	- varaible unused

	- varaible set but not used 

	- variable uninited at use 


	- analysis of types deref'd pointers   loads and stores

	- analysis of valid pointer + offset array indexes

	- analysis of extending register bitcount  value outsite bitcount range for given size bits





outdated
current state:
	
	i think we devised a way to use the existing argument system in order to have unique labels inside a macro, to be able to create a for loop, ie, control flow macros. i think thats where we were...

	and i think we alsooo needed still to figure out how strings would be done. 


	so yeah. 




			zero incr decr sc
			set add sub mul div rem muh mhs dvs rms 
			si sd sds and or eor not ld st la
			lt ge lts ges ne eq
			at lf sta bca





*/






/*static const char* argument_type_spelling[4] = {
	"ARG_variable",
	"ARG_declared", 
	"ARG_label",
	"ARG_undefined",
};*/





/*
	printf("operation symbol table: %llu\n", operation_count);
	for (nat i = 0; i < operation_count; i++) {
		printf("operation #%5llu: .name = %-10s : .arity = %llu : .parent = %llu : .bodycount = %llu\n.args = ", 
			i, operations[i].name, operations[i].arity, operations[i].parent, operations[i].body_count
		);
		printf("[  ");
		for (nat a = 0; a < operations[i].arity; a++) {
			printf("%llu:%s  ", operations[i].arguments[a], argument_type_spelling[operations[i].type[a]]);
		}
		printf("]\n");
		for (nat s = 0; s < 3; s++) {
			printf("sym #%llu: [ ", s);
			for (nat n = 0; n < operations[i].scope_count[s]; n++) {
				printf("%llu ", operations[i].scope[s][n]);
			}
			printf("]\n");
		}
		printf("body = ");
		if (operations[i].body_count) debug_instructions(operations[i].body, operations[i].body_count);
		else printf("{empty}\n");
		puts("");
	}
*/



/*





















static void insert_byte(uint8_t** output_data, nat* output_data_count, uint8_t x) {
	*output_data = realloc(*output_data, *output_data_count + 1);
	(*output_data)[(*output_data_count)++] = x;
}

static void insert_bytes(uint8_t** d, nat* c, char* s, nat len) {
	for (nat i = 0; i < len; i++) insert_byte(d, c, (uint8_t) s[i]);
}

static void insert_u16(uint8_t** d, nat* c, uint16_t x) {
	insert_byte(d, c, (x >> 0) & 0xFF);
	insert_byte(d, c, (x >> 8) & 0xFF);
}

static void insert_u32(uint8_t** d, nat* c, uint32_t x) {
	insert_u16(d, c, (x >> 0) & 0xFFFF);
	insert_u16(d, c, (x >> 16) & 0xFFFF);
}

static void insert_u64(uint8_t** d, nat* c, uint64_t x) {
	insert_u32(d, c, (x >> 0) & 0xFFFFFFFF);
	insert_u32(d, c, (x >> 32) & 0xFFFFFFFF);
}



#define MH_MAGIC_64             0xfeedfacf
#define MH_EXECUTE              2
#define	MH_NOUNDEFS		1
#define	MH_PIE			0x200000
#define LC_UNIXTHREAD           5
#define LC_UUID            	0x1b
#define LC_THREAD            	4
#define	LC_SEGMENT_64		0x19
#define CPU_SUBTYPE_ARM64_ALL   0
#define CPU_TYPE_ARM            12
#define CPU_ARCH_ABI64          0x01000000 
#define VM_PROT_READ       	1
#define VM_PROT_WRITE   	2
#define VM_PROT_EXECUTE 	4
#define ARM_THREAD_STATE 	1
#define ARM_THREAD_STATE64      6
//#define MH_SUBSECTIONS_VIA_SYMBOLS 0x2000




	uint8_t* data = NULL;
	nat count = 0;	

	insert_u32(&data, &count, MH_MAGIC_64);
	insert_u32(&data, &count, CPU_TYPE_ARM | (int)CPU_ARCH_ABI64);
	insert_u32(&data, &count, CPU_SUBTYPE_ARM64_ALL);
	insert_u32(&data, &count, MH_EXECUTE);
	insert_u32(&data, &count, 5);
	insert_u32(&data, &count, 504 + 24);  // command_end - command_start
	insert_u32(&data, &count, MH_NOUNDEFS | MH_PIE);
	insert_u32(&data, &count, 0);

	const nat command_start = count;
	const nat pagezero_start = count;
	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72); 			// pagezero_end - pagezero_start
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'P', 'A', 'G', 'E', 'Z', 'E', 
		'R', 'O',  0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 4096);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	while (count % 4096) insert_byte(&data, &count, 0);
	const nat pagezero_end = count;

	const nat text_start_address = 0x100000000;

	const nat text_start = count;
	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72); 	// text_end - text_start
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'T', 'E', 'X', 'T',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, text_start_address);
	insert_u64(&data, &count, 8192);     //  code_end - 0 (ie, __origin)  // todo: round this up to the nearest page.
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 8192);     //  code_end - 0 (ie, __origin)  // oh also make it dynamically computed.
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_EXECUTE);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_EXECUTE);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	while (count % 4096) insert_byte(&data, &count, 0);
	const nat text_end = count;

	const nat thread_start = count;
	insert_u32(&data, &count, LC_UNIXTHREAD);
	insert_u32(&data, &count, 288);                       // thread_end - thread_start
	insert_u32(&data, &count, ARM_THREAD_STATE64);
	insert_u32(&data, &count, 2 * (32 + 2));
	for (nat i = 0; i < 32; i++) insert_u64(&data, &count, 0xa5a5a5a5a5a5a5a5);
	insert_u64(&data, &count, text_start_address + 464); // pc
	insert_u32(&data, &count, 0); // cpsr
	insert_u32(&data, &count, 0);
	while (count % 4096) insert_byte(&data, &count, 0);
	const nat thread_end = count;

	const nat uuid_start = count;
	insert_u32(&data, &count, LC_UUID);
	insert_u32(&data, &count, 24);    // uuid_end - uuid_start
	insert_u32(&data, &count, rand());
	insert_u32(&data, &count, rand());
	insert_u32(&data, &count, rand());
	insert_u32(&data, &count, rand());
	while (count % 4096) insert_byte(&data, &count, 0);
	const nat uuid_end = count;

	const nat linkedit_start = count;
	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72); 	// linkedit_end - linkedit_start
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'L', 'I', 'N', 'K', 'E', 'D', 
		'I', 'T',  0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_WRITE);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_WRITE);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	const nat linkedit_end = count;

	const nat command_end = count;

	uint8_t* code_bytes = 0;//(uint8_t*) "hello world ashtasht";
	const nat code_byte_count = 4096;//(nat) strlen("hello world ashtasht");
	const nat code_start = count;
	for (nat i = 0; i < code_byte_count; i++) insert_byte(&data, &count, (uint8_t) (rand() % 0xFF));
	while (count < 4096 or count % 4096) insert_byte(&data, &count, 0);
	const nat code_end = count;

	printf("code_start = %llu, code_end = %llu\n", code_start, code_end);
	puts("offsets");
	printf("\t command_end - command_start = %llu\n", command_end - command_start);
	printf("\t pagezero_end - pagezero_start = %llu\n", pagezero_end - pagezero_start);
	printf("\t text_end - text_start = %llu\n", text_end - text_start);
	printf("\t thread_end - thread_start = %llu\n", thread_end - thread_start);
	printf("\t linkedit_end - linkedit_start = %llu\n", linkedit_end - linkedit_start);
	printf("\t uuid_end - uuid_start = %llu\n", uuid_end - uuid_start);
	printf("\t code_end = %llu\n", code_end - 0);	
	puts("");

	puts("bytes: ");
	for (nat i = 0; i < count; i++) {
		if (i % 32 == 0) puts("");
		if (data[i]) printf("\033[32;1m");
		printf("%02hhx ", data[i]);
		if (data[i]) printf("\033[0m");
	}
	puts("");

	if (not access("output_executable", F_OK)) {
		printf("file exists. do you wish to remove the previous one? ");
		fflush(stdout);
		int c = getchar();
		if (c == 'y') {
			puts("file was removed.");
			int r = remove("output_executable");
			if (r < 0) { perror("remove"); exit(1); }
		} else {
			puts("not removed");
		}
	}
	int file = open("output_executable", O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("could not create executable file"); exit(1); }
	int r = fchmod(file, 0777);
	if (r < 0) { perror("could not make the output file executable"); exit(1); }

	write(file, data, count);
	close(file);
	printf("wrote %llu bytes to file %s.\n", count, "output_executable");
}
	exit(1);

  // system("otool -txvVhlL object0.o");
    //            system("otool -txvVhlL executable0.out");
    //            system("objdump object0.o -DSast --disassembler-options=no-aliases");
    //            system("objdump executable0.out -DSast --disassembler-options=no-aliases");






















DD        LC_UNIXTHREAD                                 ; cmd
DD        __UNIX_THREADend - __UNIX_THREADstart         ; cmdsize
DD        x86_THREAD_STATE64                            ; flavor
DD        x86_EXCEPTION_STATE64_COUNT                   ; count

	__uint64_t x[29]; // General purpose registers x0-x28 //
	__uint64_t fp;    // Frame pointer x29 //
	__uint64_t lr;    // Link register x30 //
	__uint64_t sp;    // Stack pointer x31 //
	__uint64_t pc;    // Program counter //
	__uint32_t cpsr;  // Current program status register //
	__uint32_t __pad; // Same size for 32-bit or 64-bit clients //




struct mach_header_64 {
	uint32_t	magic;
	int32_t		cputype;
	int32_t		cpusubtype;
	uint32_t	filetype;
	uint32_t	ncmds;
	uint32_t	sizeofcmds;
	uint32_t	flags;
	uint32_t	reserved;
};

struct segment_command_64 {
	uint32_t	cmd;
	uint32_t	cmdsize;
	char		segname[16];
	uint64_t	vmaddr;
	uint64_t	vmsize;
	uint64_t	fileoff;
	uint64_t	filesize;
	int32_t		maxprot;
	int32_t		initprot;
	uint32_t	nsects;
	uint32_t	flags;
};


static void make_macho_object_file(void) {

	struct mach_header_64 header = {0};
	header.magic = MH_MAGIC_64;
	header.cputype = (int)CPU_TYPE_ARM | (int)CPU_ARCH_ABI64;
	header.cpusubtype = (int) CPU_SUBTYPE_ARM64_ALL;
	header.filetype = MH_OBJECT;
	header.ncmds = 2;
	header.sizeofcmds = 0;
	header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;

	header.sizeofcmds = 	sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	struct segment_command_64 segment = {0};
	strncpy(segment.segname, "__TEXT", 16);
	segment.cmd = LC_SEGMENT_64;
	segment.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64);
	segment.maxprot =  (VM_PROT_READ | VM_PROT_EXECUTE);
	segment.initprot = (VM_PROT_READ | VM_PROT_EXECUTE);
	segment.nsects = 1;
	segment.vmaddr = 0;
	segment.vmsize = byte_count;
	segment.filesize = byte_count;

	segment.fileoff = 	sizeof(struct mach_header_64) + 
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	struct section_64 section = {0};
	strncpy(section.sectname, "__text", 16);
	strncpy(section.segname, "__TEXT", 16);
	section.addr = 0;
	section.size = byte_count;	
	section.align = 3;
	section.reloff = 0;
	section.nreloc = 0;
	section.flags = S_ATTR_PURE_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS;

	section.offset = 	sizeof(struct mach_header_64) + 
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command);

	const char strings[] = "\0_start\0";

	struct symtab_command table  = {0};
	table.cmd = LC_SYMTAB;
	table.cmdsize = sizeof(struct symtab_command);
	table.strsize = sizeof(strings);
	table.nsyms = 1;
	table.stroff = 0;
	
	table.symoff = (uint32_t) (
				sizeof(struct mach_header_64) +
				sizeof(struct segment_command_64) + 
				sizeof(struct section_64) + 
				sizeof(struct symtab_command) + 
				byte_count
			);

	table.stroff = table.symoff + sizeof(struct nlist_64);

	struct nlist_64 symbols[] = {
	        (struct nlist_64) {
	            .n_un.n_strx = 1,
	            .n_type = N_SECT | N_EXT,
	            .n_sect = 1,
	            .n_desc = REFERENCE_FLAG_DEFINED,
	            .n_value = 0x0000000000,
	        }
	};

	if (preserve_existing_object and not access(object_filename, F_OK)) {
		puts("asm: object_file: file exists"); 
		puts(object_filename);
		exit(1);
	}

	const int flags = O_WRONLY | O_CREAT | O_TRUNC | (preserve_existing_object ? O_EXCL : 0);
	const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	const int file = open(object_filename, flags, mode);
	if (file < 0) { perror("obj:open"); exit(1); }

	write(file, &header, sizeof(struct mach_header_64));
	write(file, &segment, sizeof (struct segment_command_64));
	write(file, &section, sizeof(struct section_64));
	write(file, &table, sizeof table);
	write(file, bytes, byte_count);
	write(file, symbols, sizeof(struct nlist_64));
	write(file, strings, sizeof strings);
	close(file);
}



struct segment_command_64 {
	uint32_t	cmd;
	uint32_t	cmdsize;
	char		segname[16];
	uint64_t	vmaddr;
	uint64_t	vmsize;
	uint64_t	fileoff;
	uint64_t	filesize;
	int32_t		maxprot;
	int32_t		initprot;
	uint32_t	nsects;
	uint32_t	flags;
};

*/























/*
//printf("executed AT instruction: assigned labels[%llu].value = %llu...\n", d, pc);
static nat stack_size = 0x1000000;  // we might not need a stack... 

static nat architecture = arm64;
static nat output_format = macho_executable;

static bool preserve_existing_object = false;
static bool preserve_existing_executable = false;

static const char* object_filename = "object0.o";
static const char* executable_filename = "executable0.out";




*/

	/*




static nat isa_type_of_argument(nat op, nat arg) {
	if (op == lf  and arg == 0) return argument_type_label;
	if (op == at  and arg == 0) return argument_type_label;
	if (op == lt  and arg == 2) return argument_type_label;
	if (op == ge  and arg == 2) return argument_type_label;
	if (op == lts and arg == 2) return argument_type_label;
	if (op == ges and arg == 2) return argument_type_label;
	if (op == ne  and arg == 2) return argument_type_label;
	if (op == eq  and arg == 2) return argument_type_label;
	return argument_type_variable;
}




puts("defining builtin operations...");
	for (nat i = 1; i < isa_count; i++) {
		const nat a = isa_arity(i);
		operations = realloc(operations, sizeof(struct operation) * (operation_count + 1));
		operations[operation_count] = (struct operation) {
			.name = strdup(ins_spelling[i]),
			.arity = a,
			.arguments = calloc(a, sizeof(nat)),
			.type = calloc(a, sizeof(nat)),
			.scope = calloc(3, sizeof(nat*)),
			.scope_count = calloc(3, sizeof(nat)),
		};
		for (nat t = 0; t < a; t++) operations[operation_count].type[t] = isa_type_of_argument(i, t);
		operation_count++;

		const nat t = symbol_type_operation;
		operations[0].scope[t] = realloc(operations[0].scope[t], sizeof(nat) * (operations[0].scope_count[t] + 1));
		operations[0].scope[t][operations[0].scope_count[t]++] = i;
	}*/





















/*

		if (op >= isa_count and op != in_scope) goto inline_macro;


		if (op >= isa_count and op == in_scope) {
			puts("warning: recursive macro call ignored");
			this->body_count--;
			goto finish_instruction;
		}




		printf("inline: calling macro op=%llu...\n", op);
		getchar();
		const struct instruction call = *ins;
		this->body_count--;
		const struct operation* macro = operations + op;
		const nat macro_arity = operations[op].arity;
		nat* generated_name = NULL, * generated_type = NULL;
		nat* generated_value = NULL, generated_count = 0;

		printf("bodycount = %llu\n", macro->body_count);
		getchar();

		for (nat b = 0; b < macro->body_count; b++) {
			printf("b = %llu: in macro %llu.\n", b, op);
			getchar();

			const struct instruction bi = macro->body[b];
			struct instruction new = { 
				.count = bi.count, 
				.args = calloc(bi.count, sizeof(nat)) 
			};
			for (nat i = 0; i < new.count; i++) new.args[i] = bi.args[i];
			const nat new_op = new.args[0];
			for (nat j = 1; j < new.count; j++) {
				const nat type = operations[new_op].type[j - 1];
				for (nat a = 0; a < macro_arity; a++) {
					nat type2 = macro->type[a];
					if (type2 == argument_type_declared) type2 = argument_type_variable;
					if (new.args[j] == macro->arguments[a] and type == type2) {
						new.args[j] = call.args[a + 1]; goto next_j;
					}
				}
				const nat type_index = type == argument_type_label 
						? symbol_type_label 
						: symbol_type_variable;
				for (nat i = 0; i < macro->scope_count[type_index]; i++) 
					if (macro->scope[type_index][i] == new.args[j]) goto local_found;
				continue;
			local_found:
				for (nat i = 0; i < generated_count; i++) {
					if (	new.args[j] == generated_name[i] and 
						type_index == generated_type[i]) {
						new.args[j] = generated_value[i];
						goto next_j;
					}
				}
				nat new_generated_name = 0;
				if (type_index == symbol_type_variable) {
					new_generated_name = variable_count;
					variables = realloc(variables, sizeof(struct variable) * (variable_count + 1));
					variables[variable_count++] = (struct variable) { 
						.name = variables[new.args[j]].name, 
						.value = variables[new.args[j]].value 
					};
					macro->scope[1] = realloc(macro->scope[1], sizeof(nat) * (macro->scope_count[1] + 1));
					macro->scope[1][macro->scope_count[1]++] = new_generated_name;

				} else if (type_index == symbol_type_label) {
					new_generated_name = label_count;
					labels = realloc(labels, sizeof(struct label) * (label_count + 1));
					labels[label_count++] = (struct label) { 
						.name = labels[new.args[j]].name, 
						.value = labels[new.args[j]].value 
					};
					macro->scope[2] = realloc(macro->scope[2], sizeof(nat) * (macro->scope_count[2] + 1));
					macro->scope[2][macro->scope_count[2]++] = new_generated_name;
				}
				generated_name = realloc(generated_name, sizeof(nat) * (generated_count + 1));
				generated_name[generated_count] = new.args[j];
				generated_type = realloc(generated_type, sizeof(nat) * (generated_count + 1));
				generated_type[generated_count] = type_index;
				generated_value = realloc(generated_value, sizeof(nat) * (generated_count + 1));
				generated_value[generated_count++] = new_generated_name;
				new.args[j] = new_generated_name;
				next_j: continue;
			} 

		} else if (op == def) {
			this->body_count--;
			operations = realloc(operations, sizeof(struct operation) * (operation_count + 1));
			operations[operation_count++] = (struct operation) { 
				.name = word,
				.parent = in_scope,
				.scope = calloc(3, sizeof(nat*)),
				.scope_count = calloc(3, sizeof(nat)),
			};
			struct operation* this_new = operations + in_scope;
			this_new->scope[0] = realloc(this_new->scope[0], sizeof(nat) * (this_new->scope_count[0] + 1));
			this_new->scope[0][this_new->scope_count[0]++] = calling;
			in_scope = calling;
		} else if (op == ar) {
			this->body_count--;
			this->arguments = realloc(this->arguments, sizeof(nat) * (this->arity + 1));
			this->arguments[this->arity] = calling;
			this->type = realloc(this->type, sizeof(nat) * (this->arity + 1));
			this->type[this->arity] = argument_type_variable;
			this->arity++;
			variables = realloc(variables, sizeof(struct variable) * (variable_count + 1));
			variables[variable_count++] = (struct variable) { .name = word, .value = 0 };
			this->scope[1] = realloc(this->scope[1], sizeof(nat) * (this->scope_count[1] + 1));
			this->scope[1][this->scope_count[1]++] = calling;
		} else if (op == ret) {
			this->body_count--;
			in_scope = this->parent;
		} else if (op == obs) {
			this->body_count--;
			if (not this->arity) {
				if (this->parent == undefined or operations[this->parent].parent == undefined) { 
					puts("error: def obs usage error, no parent."); 
					abort(); 
				}
				const nat type = symbol_type_operation;
				struct operation* m = operations + this->parent;
				struct operation* p = operations + operations[this->parent].parent;
				p->scope[type] = realloc(p->scope[type], sizeof(nat) * (p->scope_count[type] + 1));
				p->scope[type][p->scope_count[type]++] = m->scope[type][m->scope_count[type] - 1];
			} else if (this->type[this->arity - 1] == argument_type_variable) {
				this->type[this->arity - 1] = argument_type_declared;
			} else if (this->type[this->arity - 1] == argument_type_declared) {
				this->type[this->arity - 1] = argument_type_label;
				struct variable w = variables[--variable_count];
				this->scope_count[1]--;
				calling = label_count;
				labels = realloc(labels, sizeof(struct label) * (label_count + 1));
				labels[label_count++] = (struct label) { .name = w.name, .value = w.value };
				const nat type = symbol_type_label;
				this->scope[type] = realloc(this->scope[type], sizeof(nat) * (this->scope_count[type] + 1));
				this->scope[type][this->scope_count[type]++] = calling;
				this->arguments[this->arity - 1] = calling;
			}
		}

*/






























//	puts("this assembler is currently a work in progress,\n"
//		"backend is currently not fully implemented yet..."
//	);



		//printf("op = %llu (%s)\n", op, op < isa_count ? ins_spelling[op] : "bLahhhhhh");
		//printf("ins->count = %llu\n", ins->count);
		//printf("arity = %llu\n", operations[op].arity);		
		//printf("operation_count == %llu\n", operation_count);
		//printf("calling %llu...\n", calling);
		//printf("info: now processing file: %s...\n", filename);





		//printf("%llu:%llu: @ word: %s..\n", word_start, word_length, word);
		//printf("expecting: %s...\n", symbol_type_spelling[expecting_type]);






				//puts("calling a macro!!!!");
				/*printf("heres the call : count=%llu \n", call.count);
				for (nat i = 0; i < call.count; i++) 
					printf("ins[%llu] = %llu\n", i, call.args[i]);
				puts("done call");*/

				/*puts("macro body");
				for (nat i = 0; i < macro->body_count; i++) {
					printf("ins (%llu) { op = %llu     ...args... }\n", 
						macro->body[i].count, macro->body[i].args[0]);
				}*/
				//puts("done macro body");
				//puts("generating body");







/*









								//printf("subsituting %llu with %llu\n", 
								//	new.args[j], call.args[a+1]
								//);
								//printf("didnt subsititute becuase: \n");
								//printf("either: \n");
								//printf("   %llu != %llu \n", new.args[j], macro->arguments[a]);
								//printf("or: \n");
								//printf("   %s != %s \n", argument_type_spelling[type], argument_type_spelling[macro->type[a]]);
								//puts("");





//puts("local variable found, replacing with new generated copy!!!");
						//printf("inside of instruction: %s : %llu\n", ins_spelling[new.args[0]], new.count);
						//printf("used variable %llu: .name=%s\n", new.args[j], type == argument_type_label 
						//		? labels[new.args[j]].name 
						//		: variables[new.args[j]].name);
						//puts("external variable found");
						//printf("inside of instruction: %s : %llu\n", ins_spelling[new.args[0]], new.count);
						//printf("used variable %llu: .name=%s\n", new.args[j], type == argument_type_label 
						//		? labels[new.args[j]].name 
						//		: variables[new.args[j]].name);
						//printf("macro scope index = %llu\n", op);












if (define_on_use) {	
							puts("label redefinition"); 
							printf("label name = %s\n", word); 
							debug_dictionary(operations, operation_count, 
								variables, variable_count, labels, label_count);
							abort(); 
						}








if (op < isa_count) 
			printf("builtin: now expecting argtype=%s for op=%s at position=%llu...\n", 
				argument_type_spelling[argument_type], ins_spelling[op], ins->count - 1);
		else {
			printf("macro: now expecting argtype=%s for op=%llu at position=%llu...\n", 
				argument_type_spelling[argument_type], op, ins->count - 1);
		}



else {
				printf("executing a builtin opcode: ");
				printf("op=%s arity=%llu...\n", ins_spelling[op], ins->count - 1);
			}





*/















		/*if (not this->body_count) { // delete this eventually. 
			puts("error: there is no current instruction, to add an arg to. error"); 
			abort();
		}*/




// const nat t = type_of_argument(op, ins->count - 1);


// puts("obs on ar obs    ");
					// abort(); 


//debug_dictionary(operations, operation_count, variables, variable_count, labels, label_count);
					//getchar();



						// TODO: here, we need to replace the first occurence of this local variable with a new generated one,

						// and then literally replace all other instances of it in the function, with this version. we need to generate a mapping in some array of mappings, and then when we see an element in the mapping, then we replace it, if its indeed local as well. so yeah. this only applies to local variables though. so yeah. cool beans!! niceeee nice. yay. 



// debug_dictionary(operations, operation_count, variables, variable_count, labels, label_count);
				// getchar();



		//nat d = ins[i].count >= 2 ? ins[i].args[1] : 0;
		//nat r = ins[i].count >= 3 ? ins[i].args[2] : 0;
		//nat s = ins[i].count >= 4 ? ins[i].args[3] : 0;



				// debug_dictionary(operations, operation_count, variables, variable_count, labels, label_count);
				// getchar();
				// define into the var st, first. 
				// then if obs, make it declared. 
				// then, if obs again, then move it to label st. 



/*
static void debug_instructions(struct instruction* ins, nat ins_count, struct dictionary d) {
	printf("instructions: (%llu count) \n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("%5llu:", i);
		for (nat a = 0; a < ins[i].count; a++) {
			 printf("  %20s : %-5lld", d.names[ins[i].args[a]], ins[i].args[a]);
		}
		puts("");
	}
	puts("done\n");
}


static void debug_dictionary(struct dictionary d) {
	printf("dictionary: (%llu count)\n", d.count);
	for (nat i = 0; i < d.count; i++) {
		printf("%5llu: .name = %20s, .value = %5llu\n", i, d.names[i], d.values[i]);
	}
	puts("done\n");
}

static void debug_scopes(struct scope* scopes, nat scope_count) {
	printf("scope stack: (%llu count)\n", scope_count);
	for (nat i = 0; i < scope_count; i++) {
		printf("\tscope %5llu: \n", i);
		for (nat t = 0; t < 2; t++) {
			printf("\t\t[%4llu]: ", t);
			for (nat n = 0; n < scopes[i].count[t]; n++) 
				printf("%4llu ", scopes[i].list[t][n]);
			puts("");
		}
		puts("");
	}
	puts("done\n");
}

static void debug_functions(struct function* functions, nat function_count, struct dictionary d) {
	printf("functions: (%llu count)\n", function_count);
	for (nat f = 0; f < function_count; f++) {
		printf("%5llu: .args = (%llu)[ ", f, functions[f].arity);
		for (nat a = 0; a < functions[f].arity; a++) {
			printf("%5llu(%llu) ", functions[f].arguments[a], functions[f].define_on_use[a]);
		}
		puts("]");
		puts("body: ");
		debug_instructions(functions[f].body, functions[f].body_count, d);
		puts("[end-body]");
	}
	puts("done\n");
}

static void debug_registers(nat* r, nat count) {
	printf("registers: (%llu count)\n", count);
	for (nat i = 0; i < count; i++) {
		if (i % 2 == 0) puts("");
		printf("%5llu: 0x%016llx %5lld      ", i, r[i], r[i]);
	}
	puts("\ndone\n");
}



*/




	//exit(0);




























/*



	for (nat i = 0; i < isa_count; i++) {

		const nat a = arity(i);
		functions = realloc(functions, sizeof(struct function) * (function_count + 1));
		functions[function_count++] = (struct function) {
			.arity = a,
			.arguments = calloc(a, sizeof(nat)),
			.define_on_use = calloc(a, sizeof(nat)),
		};

		if (i == zero or i == set or i == at or i == lf or i == ld) 
			functions[function_count - 1].define_on_use[0] = 1;

		if (	i == lt or i == ge or
			i == lts or i == ges or 
			i == ne or i == eq) 
			functions[function_count - 1].define_on_use[2] = 1;

		scopes[0].list[0] = realloc(scopes[0].list[0], sizeof(nat) * (scopes[0].count[0] + 1));
		scopes[0].list[0][scopes[0].count[0]++] = dictionary.count;

		dictionary.names = realloc(dictionary.names, sizeof(char*) * (dictionary.count + 1));
		dictionary.values = realloc(dictionary.values, sizeof(nat) * (dictionary.count + 1));
		dictionary.names[dictionary.count] = strdup(ins_spelling[i]);
		dictionary.values[dictionary.count++] = function_count - 1;
	}

	for (nat i = 0; i < builtin_count; i++) {
		scopes[0].list[1] = realloc(scopes[0].list[1], sizeof(nat) * (scopes[0].count[1] + 1));
		scopes[0].list[1][scopes[0].count[1]++] = dictionary.count;

		dictionary.names = realloc(dictionary.names, sizeof(char*) * (dictionary.count + 1));
		dictionary.values = realloc(dictionary.values, sizeof(nat) * (dictionary.count + 1));
		dictionary.names[dictionary.count] = strdup(builtin_spelling[i]);
		dictionary.values[dictionary.count++] = 0;
	}

	const char* included_files[4096] = {0};
	nat included_file_count = 0;

	debug_dictionary(dictionary);
	debug_functions(functions, function_count, dictionary);
	debug_scopes(scopes, scope_count);
	puts("parsing top level file...");

	// getchar();

process_file:;
	nat word_length = 0, word_start = 0, in_args = 0;

	const nat starting_index = 	stack[stack_count - 1].index;
	const nat text_length = 	stack[stack_count - 1].text_length;
	char* text = 			stack[stack_count - 1].text;
	const char* filename = 		stack[stack_count - 1].filename;

	printf("info: now processing file: %s...\n", filename);

	for (nat index = starting_index; index < text_length; index++) {

		if (not isspace(text[index])) {
			if (not word_length) word_start = index;
			word_length++; 
			if (index + 1 < text_length) continue;
		} else if (not word_length) continue;
		char* word = strndup(text + word_start, word_length);
		
		// printf("%llu:%llu: @ word: %s..\n", word_start, word_length, word);

		nat name = (nat) -1;
		for (nat s = scope_count; s--;) {
			nat* list = scopes[s].list[in_args];
			nat count = scopes[s].count[in_args];
			for (nat i = count; i--;) {
				if (not strcmp(dictionary.names[list[i]], word) and 
					in_args != dictionary.values[list[i]]) {
					name = list[i];
					goto found;
				}
			}
		}

		if (not in_args) {
			printf("file: %s, index: %llu\n", filename, index);
			printf("error: use of undefined operation \"%s\"\n", word);
			abort();

		} else {

			const nat s = scope_count - 1;
			const nat f = scopes[s].function;
			const nat b = functions[f].body_count - 1;
			const nat op = functions[f].body[b].args[0];
			const nat op_f = dictionary.values[op];
			const nat ac = functions[f].body[b].count;
	
			if (op == def or op == ar) goto found;

			if (not functions[op_f].define_on_use[ac - 1]) { 
				printf("file: %s, index: %llu\n", filename, index);
				printf("error: use of undefined variable \"%s\"\n", word);
				abort();
			}

			puts("defining a new name:");
			puts(word);

			const nat t = 1;
			const nat d = dictionary.count;
			const nat ss = scopes[s].count[t];

			scopes[s].list[t] = realloc(scopes[s].list[t], sizeof(nat) * (ss + 1));
			scopes[s].list[t][scopes[s].count[t]++] = d;

			dictionary.names = realloc(dictionary.names, sizeof(char*) * (d + 1));
			dictionary.values = realloc(dictionary.values, sizeof(nat) * (d + 1));
			dictionary.names[d] = word;
			dictionary.values[dictionary.count++] = 0;
			name = d;
		}
	found:
		if (not in_args) {	
			const nat s = scope_count - 1;
			const nat f = scopes[s].function;
			const nat b = functions[f].body_count;

			functions[f].body = realloc(functions[f].body, sizeof(struct instruction) * (b + 1));
			functions[f].body[functions[f].body_count++] = (struct instruction) {0};
			in_args = 1;
		}

		{
		const nat s = scope_count - 1;
		const nat f = scopes[s].function;
		const nat b = functions[f].body_count - 1;
		functions[f].body[b].args = realloc(functions[f].body[b].args, sizeof(nat) * (functions[f].body[b].count + 1));
		functions[f].body[b].args[functions[f].body[b].count++] = name;
		}


		const nat _s = scope_count - 1;
		const nat _f = scopes[_s].function;
		const nat _b = functions[_f].body_count - 1;
		const nat op = functions[_f].body[_b].args[0];
		const nat _c = functions[_f].body[_b].count;
		const nat _a = functions[dictionary.values[op]].arity;

		if (_c >= _a + 1) {

			if (op == lf) {
				functions[scopes[scope_count - 1].function].body_count--;

				for (nat i = 0; i < included_file_count; i++) {
					if (not strcmp(included_files[i], word)) {
						printf("warning: %s: file already included\n", word);
						goto skip_include;
					}
				}

				//printf("including file %s...\n", word);
				included_files[included_file_count++] = word;

				int file = open(word, O_RDONLY);
				if (file < 0) { puts(word); perror("open"); exit(1); }
				const nat new_text_length = (nat) lseek(file, 0, SEEK_END);
				lseek(file, 0, SEEK_SET);
				char* new_text = calloc(new_text_length + 1, 1);
				read(file, new_text, new_text_length);
				close(file);

				const nat s = stack_count;
				stack[s - 1].index = index;
				stack[s].filename = word;
				stack[s].text = new_text;
				stack[s].text_length = new_text_length;
				stack[stack_count++].index = 0;
				goto process_file;
				skip_include:;
			}

			if (op == ret) {
				//puts("executing ret....");
				const nat s = scope_count - 1;
				const nat f = scopes[s].function;
				functions[f].body_count--;
				scope_count--;
				const nat s2 = scope_count - 1;
				const nat f2 = scopes[s2].function;
				functions[f2].body_count--;
			}

			if (op == obs) {
				//puts("executing obs...");
				const nat s = scope_count - 1;
				const nat f2 = scopes[s].function;
				functions[f2].body_count--;
				const nat f = function_count - 1;
				if (functions[f].arity == 0) {
					puts("def obs: making function have public lexical scoping...");	
					if (s < 2) {
						puts("invalid use of obs instruction, aborting...");
						abort();
					}
					const nat t = 0;
					scopes[s - 2].list[t] = realloc(scopes[s - 2].list[t], sizeof(nat) * (scopes[s - 2].count[t] + 1));
					scopes[s - 2].list[t][scopes[s - 2].count[t]++] =  dictionary.count - 1;
				} else {
					printf("info: ar obs: made argument %llu define on use...\n", functions[f].arity - 1);
					functions[f].define_on_use[functions[f].arity - 1] = 1;
				}
			}

			if (op == ar) {
				const nat s = scope_count - 1;
				const nat f2 = scopes[s].function;
				functions[f2].body_count--;

				//puts("executing ar....");

				const nat t = 1;
				const nat d = dictionary.count;

				scopes[s].list[t] = realloc(scopes[s].list[t], sizeof(nat) * (scopes[s].count[t] + 1));
				scopes[s].list[t][scopes[s].count[t]++] = d;

				dictionary.names = realloc(dictionary.names, sizeof(char*) * (d + 1));
				dictionary.values = realloc(dictionary.values, sizeof(nat) * (d + 1));
				dictionary.names[d] = word;
				dictionary.values[dictionary.count++] = 0;

				const nat f = function_count - 1;
				functions[f].arguments = realloc(functions[f].arguments, sizeof(nat) * (functions[f].arity + 1));
				functions[f].arguments[functions[f].arity] = d;

				functions[f].define_on_use = realloc(functions[f].define_on_use, sizeof(nat) * (functions[f].arity + 1));
				functions[f].define_on_use[functions[f].arity++] = 0;
			}

			if (op == def) {
				//puts("EXECUTING DEF!!!");
				//puts(word);

				functions = realloc(functions, sizeof(struct function) * (function_count + 1));
				functions[function_count++] = (struct function) {0};

				const nat t = 0;
				const nat s = scope_count - 1;
				const nat w = scopes[s].function;
				const nat d = dictionary.count;
				const nat f = function_count - 1;

				scopes[s].list[t] = 
				realloc(scopes[s].list[t], 
				sizeof(nat) * (scopes[s].count[t] + 1));
				scopes[s].list[t][
				scopes[s].count[t]++] = d;

				dictionary.names = realloc(dictionary.names, sizeof(char*) * (d + 1));
				dictionary.values = realloc(dictionary.values, sizeof(nat) * (d + 1));
				dictionary.names[d] = word;
				dictionary.values[dictionary.count++] = f;

				const nat b = functions[w].body_count - 1;
				const nat c = functions[w].body[b].count - 1;
				functions[w].body[b].args[c] = d;

				const nat sc = scope_count;
				scopes = realloc(scopes, sizeof(struct scope) * (sc + 1));
				scopes[sc] = (struct scope) {0};
				scopes[sc].list = calloc(2, sizeof(nat*));
				scopes[sc].count = calloc(2, sizeof(nat));
				scopes[sc].function = f;
				scope_count++;
			}
			in_args = 0;
		}
		word_length = 0;
	}
	stack_count--;
	if (not stack_count) {
		puts("processing_file: finished last file.");
	} else {
		puts("processing next file in the stack...");
		goto process_file;
	}

	debug_dictionary(dictionary);
	debug_functions(functions, function_count, dictionary);
	debug_scopes(scopes, scope_count);

	puts("generating inline instructions now...");

	struct instruction* ins = NULL;
	nat ins_count = 0;
	nat call_stack_count = 1;
	nat call_stack[4096] = {0};
	nat return_address_stack[4096] = {0};
	nat* argumentlist_stack[4096] = {0};

generate_function:;
	const nat f = call_stack[call_stack_count - 1];
	const nat init_pc = return_address_stack[call_stack_count - 1];
	nat* call_arguments = argumentlist_stack[call_stack_count - 1];

	for (nat pc = init_pc; pc < functions[f].body_count; pc++) {

		struct instruction new = {0};    // NOTE:   this entire stage should be done during the parser stage. 
						//  we should be inlining all macros as soonnn as we recognize the full
						// argument list of the call.  we should have already parsed the defintition,
						// so we definitely can do this. so yeah. kinda a complicated thing though lol.

		new.count = functions[f].body[pc].count;
		new.args = calloc(new.count, sizeof(nat));
		for (nat i = 0; i < new.count; i++) new.args[i] = functions[f].body[pc].args[i];

		for (nat a = 0; a < functions[f].arity; a++) 
			for (nat b = 0; b < new.count; b++) 
				if (new.args[b] == functions[f].arguments[a]) 
					new.args[b] = call_arguments[a];
		
		printf("generating inline: %4llu:", pc);
		for (nat a = 0; a < functions[f].body[pc].count; a++) {
			  printf("  %10s : %-4lld", 
				dictionary.names[functions[f].body[pc].args[a]], 
				functions[f].body[pc].args[a]
			);
		}
		puts("");

		const nat op = new.args[0];

		if (op >= isa_count) {
			printf("executing user-defined function: %s...\n", dictionary.names[op]);
			return_address_stack[call_stack_count - 1] = pc + 1;
			return_address_stack[call_stack_count] = 0;
			argumentlist_stack[call_stack_count] = new.args + 1;
			call_stack[call_stack_count++] = dictionary.values[op];
			goto generate_function;
		} else {
			ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
			ins[ins_count++] = new;
		}
	}
	call_stack_count--;
	if (call_stack_count) goto generate_function;

	puts("generated these instructions:");
	debug_instructions(ins, ins_count, dictionary);


	puts("finding label attrs...");
	nat* R = calloc(dictionary.count, sizeof(nat));

	for (nat pc = 0; pc < ins_count; pc++) {
		const nat op = ins[pc].args[0];
		nat d = ins[pc].count >= 2 ? ins[pc].args[1] : 0;
		if (op == at) {
			printf("executed at: assigned R[%llu] = %llu...\n", d, pc);
			R[d] = pc;
		}
	}


	puts("starting the DAG formation stage...");
	
	struct node nodes[4096] = {0}; 
	nat node_count = 0;

	nodes[node_count++] = (struct node) { .output_reg = 0, .statically_known = 1 };
	//nodes[node_count++] = (struct node) { .output_reg = var_stacksize, .statically_known = 1 };
	
	puts("stage: constructing data flow DAG...");

	struct basic_block blocks[4096] = {0};
	nat block_count = 0;

	for (nat i = 0; i < ins_count; i++) {

		const nat op = ins[i].args[0];
		nat d = ins[i].count >= 2 ? ins[i].args[1] : 0;
		nat r = ins[i].count >= 3 ? ins[i].args[2] : 0;
		nat s = ins[i].count >= 4 ? ins[i].args[3] : 0;

		printf("generating DAG node for ins: "
			"{ %s  %s  %s  %s }\n",
			ins_spelling[op],
			dictionary.names[d],
			dictionary.names[r],
			dictionary.names[s]
		);
		

		// const nat output_reg = d;

		const nat statically_known = 0; //(nodes[input0].statically_known and nodes[input1].statically_known);

		printf("statically_known = %llu\n", statically_known);

		if (ins[i].args[0] == at and blocks[block_count].dag_count) block_count++;

		struct basic_block* block = blocks + block_count;
		block->dag = realloc(block->dag, sizeof(nat) * (block->dag_count + 1));
		block->dag[block->dag_count++] = node_count;

		nodes[node_count++] = (struct node) {

			.data_outputs = NULL,
			.data_output_count = 0,

			.data_inputs = NULL,
			.data_input_count = 0,

			.op = ins[i].args[0],

			.statically_known = statically_known,
		};

		printf("generated node #%llu into block %llu (of %llu): \n", 
			node_count, block_count, block->dag_count);

		puts("inputs and outputs null for now");
		

		if (	ins[i].args[0] == lt or 
			ins[i].args[0] == ge or 
			ins[i].args[0] == ne or 
			ins[i].args[0] == eq or
			ins[i].args[0] == lts or 
			ins[i].args[0] == ges
		) block_count++;
	}
 	block_count++;
	

	puts("done creating isa nodes... printing nodes:");
	print_nodes(nodes, node_count, dictionary.names);

	puts("done creating basic blocks... printing cfg/dag:");
	print_basic_blocks(blocks, block_count, nodes, dictionary.names);

	puts("executing instructions... ");
	R[isa_count + stacksize] = 65536;
	R[isa_count + stackpointer] = (nat) (void*) malloc(65536);

	for (nat pc = 0; pc < ins_count; pc++) {

		const nat op = ins[pc].args[0];
		nat d = ins[pc].count >= 2 ? ins[pc].args[1] : 0;
		nat r = ins[pc].count >= 3 ? ins[pc].args[2] : 0;
		nat s = ins[pc].count >= 4 ? ins[pc].args[3] : 0;

		if (false) {}
		else if (op == at) {}// { puts("executed at: IGNORING"); }
		else if (op == zero) R[d] = 0;
		else if (op == incr) R[d]++;
		else if (op == decr) R[d]--;
		else if (op == not_) R[d] = ~R[d];
		else if (op == set)  R[d]  = R[r];
		else if (op == add)  R[d] += R[r];
		else if (op == sub)  R[d] -= R[r];
		else if (op == mul)  R[d] *= R[r];
		else if (op == div_) R[d] /= R[r];
		else if (op == rem)  R[d] %= R[r];
		else if (op == si)   R[d]<<= R[r];
		else if (op == sd)   R[d]>>= R[r];
		else if (op == sds)  R[d]>>= R[r];
		else if (op == and_) R[d] &= R[r];
		else if (op == or_)  R[d] |= R[r];
		else if (op == eor)  R[d] ^= R[r];

		else if (op == st) {
			     if (R[s] == 1) { *( uint8_t*)(R[d]) = ( uint8_t)R[r]; }
			else if (R[s] == 2) { *(uint16_t*)(R[d]) = (uint16_t)R[r]; }
			else if (R[s] == 4) { *(uint32_t*)(R[d]) = (uint32_t)R[r]; }
			else if (R[s] == 8) { *(uint64_t*)(R[d]) = (uint64_t)R[r]; }
			else abort();

		} else if (op == ld) {
			     if (R[s] == 1) { R[d] = *( uint8_t*)(R[r]); }
			else if (R[s] == 2) { R[d] = *(uint16_t*)(R[r]); }
			else if (R[s] == 4) { R[d] = *(uint32_t*)(R[r]); }
			else if (R[s] == 8) { R[d] = *(uint64_t*)(R[r]); }
			else abort();
		}
		else if (op == lt)  { if (R[d] < R[r])  { pc = R[s]; } } 
		else if (op == ge)  { if (R[d] >= R[r]) { pc = R[s]; } } 
		else if (op == lts) { if (R[d] < R[r])  { pc = R[s]; } } 
		else if (op == ges) { if (R[d] >= R[r]) { pc = R[s]; } } 
		else if (op == ne)  { if (R[d] != R[r]) { pc = R[s]; } } 
		else if (op == eq)  { if (R[d] == R[r]) { pc = R[s]; } } 
		else if (op == sc) {
			const nat n = R[ins[pc].args[1]];
			const nat a0 = ins[pc].args[2];
			const nat a1 = ins[pc].args[3];
			const nat a2 = ins[pc].args[4];
			const nat a3 = ins[pc].args[5];
			const nat a4 = ins[pc].args[6];
			const nat a5 = ins[pc].args[7];

			     if (n == system_call_undefined) printf("\033[32;1mdebug:   0x%llx : %lld\033[0m\n", R[a0], R[a0]); 
			else if (n == system_exit) 	exit((int) R[a0]);
			else if (n == system_fork) 	R[a0] = (nat) fork(); 
			else if (n == system_openat) 	R[a0] = (nat) openat((int) R[a0], (const char*) R[a1], (int) R[a2], (int) R[a3]);
			else if (n == system_close) 	R[a0] = (nat) close((int) R[a0]);
			else if (n == system_read) 	R[a0] = (nat) read((int) R[a0], (void*) R[a1], (size_t) R[a2]);
			else if (n == system_write) 	R[a0] = (nat) write((int) R[a0], (void*) R[a1], (size_t) R[a2]);
			else if (n == system_lseek) 	R[a0] = (nat) lseek((int) R[a0], (off_t) R[a1], (int) R[a2]);
			else if (n == system_mmap) 	R[a0] = (nat) (void*) mmap((void*) R[a0], (size_t) R[a1], (int) R[a2], (int) R[a3], (int) R[a4], (off_t) R[a5]);
			else if (n == system_munmap) 	R[a0] = (nat) munmap((void*) R[a0], (size_t) R[a1]); 
			else {
				puts("bad system call");
				printf("%llu: system call not supported.\n", n);
				abort();
			}
		}
		else {
			printf("error: executing unknown instruction: %llu (%s)\n", op, dictionary.names[op]);
			abort();
		}
	}

	//debug_registers(R, dictionary.count);

	exit(0);



*/

















































































/*
		else if (op == env) {
			const nat a0 = reg[var_arg0];
			const nat a1 = reg[var_arg1];
			const nat a2 = reg[var_arg2];
			//const nat a3 = reg[var_arg3];
			//const nat a4 = reg[var_arg4];
			//const nat a5 = reg[var_arg5];
			const nat n = reg[var_argn];

			if (n == 0) {
				snprintf(reason, sizeof reason, "%lld (0x%016llx)", a0, a0);
				print_message(user, reason, ins[pc].source[0]);
			} 

			else if (n == 1) exit((int) a0);
			else if (n == 2) fork();
			else if (n == 3) read((int) a0, (void*) a1, (size_t) a2);
			else if (n == 4) write((int) a0, (void*) a1, (size_t) a2);
			else if (n == 5) open((const char*) a0, (int) a1, (int) a2);	
			else if (n == 6) close((int) a0);
			else if (n == 59) execve((char*) a0, (char**) a1, (char**) a2);
			
			else {
				snprintf(reason, sizeof reason, "unknown ct ecl number %llu", n);
				print_message(error, reason, ins[pc].source[0]);
				exit(1);
			}


*/





































					//const nat _b = functions[f2].body_count - 1;
					//const nat op = functions[f2].body[_b].args[0];

				
				//functions[f].arguments = realloc(functions[f].arguments, sizeof(nat) * (functions[f].arity + 1));
				//functions[f].arguments[functions[f].arity] = d;

				//functions[f].define_on_use = realloc(functions[f].define_on_use, sizeof(nat) * (functions[f].arity + 1));
				//functions[f].define_on_use[functions[f].arity++] = 0;










//push:;


	// constant propagation: 	static ct execution:
	// 1. form data flow dag	
	// 2. track which instructions have statically knowable inputs and outputs. (constants)




			//output_reg == var_zero or 
			//input0 == -1 or
			//input1 == -1 or
			//;











			/*
			if (ac != 1 and op == set) goto undecl_error;
			if (ac != 3 and (op == lt or op == ge or
					 op == lts or op == ges or
					 op == ne or op == eq
					)
				) goto undecl_error;

				op != zero and op != set and op != at and op != lf and
				op != lt and op != ge and
				op != lts and op != ges and
				op != ne and op != eq
			*/




















		/*printf("executing: %5llu: %20s : %-5lld %20s : %-5lld %20s : %-5lld %20s : %-5lld\n", 
			pc, 
			dictionary.names[op], op,
			dictionary.names[d], d,
			dictionary.names[r], r,
			dictionary.names[s], s
		);*/






	/*for () {

			}
			printf("parsing:    %5llu: %20s : %-5lld %20s : %-5lld %20s : %-5lld %20s : %-5lld\n", 
				i, 
				dictionary.names[functions[f].body[i].args[0]], functions[f].body[i].args[0],
				dictionary.names[functions[f].body[i].args[1]], functions[f].body[i].args[1],
				dictionary.names[functions[f].body[i].args[2]], functions[f].body[i].args[2],
				dictionary.names[functions[f].body[i].args[3]], functions[f].body[i].args[3]
			);*/

			
			//const nat r  = functions[f].body[i].args[2];
			//const nat s  = functions[f].body[i].args[3];








				// debug_dictionary(dictionary);
				// debug_functions(functions, function_count, dictionary);
				// debug_scopes(scopes, scope_count);








					// puts("found name!");
					// puts(word);
					// printf("in_args = %llu\n", in_args);





		// puts("finished with word");
		// puts(word);
		// debug_dictionary(d);
		// debug_functions(functions, function_count, d);
		// debug_scopes(scopes, scope_count);












			//nodes[n].input0, "[i0]", //names[nodes[nodes[n].input0].output_reg],
			//nodes[n].input1, "[i1]", //names[nodes[nodes[n].input1].output_reg],
			//nodes[n].input0_value,
			//nodes[n].input1_value,
			//nodes[n].output_value,

//".0=%2llu (\"\033[33;1m%-10s\033[0m\") "
			//".1=%2llu (\"\033[33;1m%-10s\033[0m\") "
			//".0v=%2llu "
			//".1v=%2llu "
			//".ov=%2llu "



/*

	dm f obs sb ar x
		zero x
	se f


	dm f sb ar x
		zero x
	se f


	dm f sb
		...
	se f


	dm f
		...
	f
*/





/*



struct instruction {
	nat* args;
	nat count;
};

struct operation {
	char* name;

	nat** scope;
	nat* scope_count;

	nat arity;
	nat* arguments;
	nat* type;

	struct instruction* body;
	nat body_count;

	nat parent;
};

struct label {
	char* name;
	nat value;
};

struct variable {
	char* name;
	nat value;
};


*/




/*static const char* symbol_type_spelling[4] = {
	"symbol_type_operation",
	"symbol_type_variable", 
	"symbol_type_label",
	"symbol_type_undefined", 
};*/




enum system_call_table {
	system_call_undefined, 
	system_exit, system_execve, system_fork, system_wait,
	system_openat, system_close, system_write, system_read,
	system_ioctl, system_poll, system_lseek, 
	system_munmap, system_mprotect, system_mmap, 
	system_call_count
};

/*static const char* system_call_spelling[system_call_count] = {
	"system_call_undefined", 
	"system_exit", "system_execve", "system_fork", "system_wait",
	"system_openat", "system_close", "system_write", "system_read",
	"system_ioctl", "system_poll", "system_lseek", 
	"system_munmap", "system_mprotect", "system_mmap", 
};*/







struct machine_instruction {
	nat args[16];
	nat arg_count;
	nat instructions[16];
	nat ins_count;
	nat op;
};






/*static void print_machine_instructions(struct machine_instruction* mis, const nat mi_count) {
	printf("printing %llu machine instructions...\n", mi_count);
	for (nat i = 0; i < mi_count; i++) {
		printf("machine instruction {.op = %3llu (\"%s\"), .args = (%3llu)[%3llu, %3llu, %3llu, %3llu] }\n", 
			mis[i].op, arm64_spelling[mis[i].op],
			mis[i].arg_count, 
			mis[i].args[0],mis[i].args[1],mis[i].args[2],mis[i].args[3]
		); 
	}
	puts("[done]");
}*/






#define MH_MAGIC_64             0xfeedfacf
#define MH_EXECUTE              2
#define	MH_NOUNDEFS		1
#define	MH_PIE			0x200000
#define MH_DYLDLINK 		0x4
#define MH_TWOLEVEL		0x80
#define	LC_SEGMENT_64		0x19
#define LC_DYSYMTAB		0xb
#define LC_SYMTAB		0x2
#define LC_LOAD_DYLINKER	0xe
#define LC_LOAD_DYLIB		0xc
#define LC_REQ_DYLD		0x80000000
#define LC_MAIN			(0x28 | LC_REQ_DYLD)
#define LC_BUILD_VERSION 	0x32
#define LC_SOURCE_VERSION 	0x2A
#define LC_UUID            	0x1B
#define S_ATTR_PURE_INSTRUCTIONS 0x80000000
#define S_ATTR_SOME_INSTRUCTIONS 0x00000400
#define CPU_SUBTYPE_ARM64_ALL   0
#define CPU_TYPE_ARM            12
#define CPU_ARCH_ABI64          0x01000000 
#define VM_PROT_READ       	1
#define VM_PROT_WRITE   	2
#define VM_PROT_EXECUTE 	4
#define TOOL_LD	3
#define PLATFORM_MACOS 1











{ 
	uint8_t* data = NULL;
	nat count = 0;	

	insert_u32(&data, &count, MH_MAGIC_64);
	insert_u32(&data, &count, CPU_TYPE_ARM | (int)CPU_ARCH_ABI64);
	insert_u32(&data, &count, CPU_SUBTYPE_ARM64_ALL);
	insert_u32(&data, &count, MH_EXECUTE);
	insert_u32(&data, &count, 11);
	insert_u32(&data, &count, 72 + (72 + 80) + 72 + 24 + 80 + 32 + 24 + 32 + 16 + 24 +  (24 + 32) ); 
	insert_u32(&data, &count, MH_NOUNDEFS | MH_PIE | MH_DYLDLINK | MH_TWOLEVEL);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'P', 'A', 'G', 'E', 'Z', 'E', 
		'R', 'O',  0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0x0000000100000000);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72 + 80);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'T', 'E', 'X', 'T',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0x0000000100000000);
	insert_u64(&data, &count, 0x0000000000004000);
	insert_u64(&data, &count, 0);
	insert_u64(&data, &count, 16384);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_EXECUTE);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_EXECUTE);
	insert_u32(&data, &count, 1);
	insert_u32(&data, &count, 0);
//
	insert_bytes(&data, &count, (char[]){
		'_', '_', 't', 'e', 'x', 't',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'T', 'E', 'X', 'T',  0,   0, 
		 0,   0,   0,   0,   0,   0,   0,   0
	}, 16);

	insert_u64(&data, &count, 0x0000000100000000 + 16384 - 16);
	insert_u64(&data, &count, 16); 
	insert_u32(&data, &count, 16384 - 16);
	insert_u32(&data, &count, 4); 
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0); 
	insert_u32(&data, &count, S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS);
	insert_u32(&data, &count, 0); 
	insert_u32(&data, &count, 0); 
	insert_u32(&data, &count, 0); 
//
	insert_u32(&data, &count, LC_SEGMENT_64);
	insert_u32(&data, &count, 72);
	insert_bytes(&data, &count, (char[]){
		'_', '_', 'L', 'I', 'N', 'K', 'E', 'D', 
		'I', 'T',  0,   0,   0,   0,   0,   0
	}, 16);
	insert_u64(&data, &count, 0x0000000100004000);
	insert_u64(&data, &count, 0x0000000000004000);
	insert_u64(&data, &count, 16384);
	insert_u64(&data, &count, 800);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_WRITE);
	insert_u32(&data, &count, VM_PROT_READ | VM_PROT_WRITE);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_SYMTAB);
	insert_u32(&data, &count, 24); 
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_DYSYMTAB);
	insert_u32(&data, &count, 80); 
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_LOAD_DYLINKER);
	insert_u32(&data, &count, 32);
	insert_u32(&data, &count, 12);
	insert_bytes(&data, &count, (char[]){
		'/', 'u', 's', 'r', '/', 'l', 'i', 'b', 
		'/', 'd', 'y',  'l', 'd',  0,   0,   0
	}, 16);
	insert_u32(&data, &count, 0);
//
	insert_u32(&data, &count, LC_UUID);
	insert_u32(&data, &count, 24); 
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
	insert_u32(&data, &count, (uint32_t) rand());
//
	insert_u32(&data, &count, LC_BUILD_VERSION);
	insert_u32(&data, &count, 32);
	insert_u32(&data, &count, PLATFORM_MACOS);
	insert_u32(&data, &count, 13 << 16);
	insert_u32(&data, &count, (13 << 16) | (3 << 8));
	insert_u32(&data, &count, 1);
	insert_u32(&data, &count, TOOL_LD);
	insert_u32(&data, &count, (857 << 16) | (1 << 8));
//
	insert_u32(&data, &count, LC_SOURCE_VERSION);
	insert_u32(&data, &count, 16);
	insert_u64(&data, &count, 0);
//
	insert_u32(&data, &count, LC_MAIN);
	insert_u32(&data, &count, 24);
	insert_u64(&data, &count, 16384 - 16);
	insert_u64(&data, &count, 0);
//
	insert_u32(&data, &count, LC_LOAD_DYLIB);
	insert_u32(&data, &count, 24 + 32);
	insert_u32(&data, &count, 24);
	insert_u32(&data, &count, 0);
	insert_u32(&data, &count, (1319 << 16) | (100 << 8) | 3);
	insert_u32(&data, &count, 1 << 16);
	insert_bytes(&data, &count, (char[]){
		'/', 'u', 's', 'r', '/', 'l', 'i', 'b', 
		'/', 'l', 'i', 'b', 'S', 'y', 's', 't',
		'e', 'm', '.', 'B', '.', 'd', 'y', 'l', 
		'i', 'b',  0,   0,   0,   0,   0,   0
	}, 32);

	const uint8_t my_bytes[] = {

		0x30, 0x00, 0x80, 0xD2, // mov x16, 1
		0xe0, 0x01, 0x80, 0xD2, // mov x0, 15
		0x01, 0x00, 0x00, 0xD4, // svc 0

		0x00, 0x00, 0x00, 0x00, // (padding)
	};
	const nat my_count = sizeof my_bytes;
	while (count < 16384 - my_count) insert_byte(&data, &count, 0);
	for (nat i = 0; i < my_count; i++) insert_byte(&data, &count, my_bytes[i]);
	for (nat i = 0; i < 800; i++) insert_byte(&data, &count, 0);

	puts("");
	puts("bytes: ");
	for (nat i = 0; i < count; i++) {
		if (i % 32 == 0) puts("");
		if (data[i]) printf("\033[32;1m");
		printf("%02hhx ", data[i]);
		if (data[i]) printf("\033[0m");
	}
	puts("");

	puts("preparing for writing out the data.");

	if (not access("output_executable_new", F_OK)) {
		printf("file exists. do you wish to remove the previous one? ");
		fflush(stdout);
		int c = getchar();
		if (c == 'y') {
			puts("file was removed.");
			int r = remove("output_executable_new");
			if (r < 0) { perror("remove"); exit(1); }
		} else {
			puts("not removed");
		}
	}
	int file = open("output_executable_new", O_WRONLY | O_CREAT | O_EXCL, 0777);
	if (file < 0) { perror("could not create executable file"); exit(1); }
	int r = fchmod(file, 0777);
	if (r < 0) { perror("could not make the output file executable"); exit(1); }

	write(file, data, count);
	close(file);
	printf("wrote %llu bytes to file %s.\n", count, "output_executable_new");




	system("codesign -s - output_executable_new");
	//system("codesign -d -vvvvvvv output_executable_new");   // for debugging



	exit(1);

}













	if (argc < 2) {
		char buffer[4096] = {0};
		puts(	"give the input string to process:\n"
			"(press '`' to terminate)\n"
		);
		get_input_string(buffer, sizeof buffer);
		filestack[0].filename = "<top-level>";
		filestack[0].text = strdup(buffer);
		filestack[0].text_length = strlen(buffer);
		filestack[0].index = 0;
	} else {










static void get_input_string(char* string, nat max_length) {

	struct termios term = {0}, restore = {0};
	tcgetattr(0, &term);
	tcgetattr(0, &restore);
	term.c_lflag &= ~(size_t)(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &term);	
	write(1, "\033[6n", 4);
	nat row = 0, column = 0;
	scanf("\033[%llu;%lluR", &row, &column);
	nat length = 0, cursor = 0;
	printf("\033[?25l");
	while (1) {
		printf("\033[%llu;%lluH", row, column);
		for (nat i = 0; i < length; i++) {
			if (string[i] == 10) printf("\033[K");
			if (i == cursor) printf("\033[7m"); 
			putchar(string[i]);
			if (i == cursor) printf("\033[0m");
			if (string[i] == 10) printf("\033[K");
		}
		if (cursor == length) printf("\033[7m \033[0m");
		printf("\033[K");
		fflush(stdout);
		char c = 0;
		read(0, &c, 1);

		if (c == '`') { break; } 
		else if (c == ',') { if (cursor) cursor--; }
		else if (c == '.') { if (cursor < length) cursor++; }
		else if (c == 127) { 
			if (cursor and length) { 
				cursor--; length--;
				memmove(string + cursor, string + cursor + 1, length - cursor);
			}
		} else if (length < max_length - 2) {
			memmove(string + cursor + 1, string + cursor, length - cursor);
			string[cursor] = c;
			cursor++; length++;
		}
	}
	printf("\033[?25h");
	string[length] = 0;
	puts("done. got string: ");
	puts(string);
}










static void insert_byte(uint8_t** output_data, nat* output_data_count, uint8_t x) {
	*output_data = realloc(*output_data, *output_data_count + 1);
	(*output_data)[(*output_data_count)++] = x;
}

static void insert_bytes(uint8_t** d, nat* c, char* s, nat len) {
	for (nat i = 0; i < len; i++) insert_byte(d, c, (uint8_t) s[i]);
}

static void insert_u16(uint8_t** d, nat* c, uint16_t x) {
	insert_byte(d, c, (x >> 0) & 0xFF);
	insert_byte(d, c, (x >> 8) & 0xFF);
}

static void insert_u32(uint8_t** d, nat* c, uint32_t x) {
	insert_u16(d, c, (x >> 0) & 0xFFFF);
	insert_u16(d, c, (x >> 16) & 0xFFFF);
}

static void insert_u64(uint8_t** d, nat* c, uint64_t x) {
	insert_u32(d, c, (x >> 0) & 0xFFFFFFFF);
	insert_u32(d, c, (x >> 32) & 0xFFFFFFFF);
}







static void debug_registers(nat* r, nat count) {
	printf("registers: (%llu count)\n", count);
	for (nat i = 0; i < count; i++) {
		if (i % 2 == 0) puts("");
		printf("%5llu: 0x%016llx %15lld      ", i, r[i], r[i]);
	}
	puts("\ndone\n");
}

// insert ./build
c.c:199:3: error: use of undeclared identifier 'active'
                active[name_count] = 1;
                ^
fatal error: too many errors emitted, stopping now [-ferror-limit=]
2 errors generated.

