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
	"_process_stackpointer", 
	"_process_stacksize",
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

static void debug_instructions(
	struct instruction* ins, 
	nat ins_count, char** names
) {
	printf("instructions: (%llu count) \n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("[%3llu]={", i);
		for (nat a = 0; a < ins[i].count; a++) {
			const char* str = "";
			if (a == 0) str = ins_spelling[ins[i].args[a]];
			else str = names[ins[i].args[a]];
			printf(".%llu=%-4lld %-7s  ", 
				a, ins[i].args[a], str
			);
		}
		puts("");
	}
	puts("done\n");
}

static void debug_dictionary(char** names, nat name_count) {
	printf("dictionary: %llu\n", name_count);
	for (nat i = 0; i < name_count; i++) 
		printf("var #%5llu:   %-10s\n", i, names[i]);
	puts("done");
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

	{int file = open(argv[1], O_RDONLY);
	if (file < 0) { puts(argv[1]); perror("open"); exit(1); }
	const nat text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(text_length + 1, 1);
	read(file, text, text_length);
	close(file);
	filestack[0].filename = argv[1];
	filestack[0].text = text;
	filestack[0].text_length = text_length;
	filestack[0].index = 0;}

	for (nat i = 0; i < builtin_count; i++) {
		names[name_count++] = strdup(builtin_spelling[i]);
	}

process_file:;
	nat word_length = 0, word_start = 0, first = 1;
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
		printf("[first=%llu]: [%llu]: at word = %s\n", first, word_start, word);
		char** list = first ? (char**) ins_spelling : (char**) names;
		const nat count = first ? isa_count : name_count;
		nat calling = 0;
		for (; calling < count; calling++) 
			if (not strcmp(list[calling], word)) goto found;
		if (first) goto next_word;
		names[name_count++] = word;

	found:	if (first) {
			if (not strcmp(word, ins_spelling[eoi])) break;
			ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
			ins[ins_count++] = (struct instruction) {0};
			first = 0;
		}
		struct instruction* this = ins + ins_count - 1;
		this->args = realloc(this->args, sizeof(nat) * (this->count + 1));
		this->args[this->count++] = calling;
		if (this->count != isa_arity(this->args[0]) + 1) goto next_word;
		if (this->args[0] == lf) {
			ins_count--;
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
		} 
		finish_instruction: first = 1;
		next_word: word_length = 0;
	}
	filestack_count--;
	if (filestack_count) goto process_file;

	debug_instructions(ins, ins_count, names);
	debug_dictionary(names, name_count);
	puts("finshed parsing!");


	exit(1);






/*
	struct node nodes[4096] = {0}; 
	nat node_count = 0;
	nodes[node_count++] = (struct node) { .output_reg = 0, .statically_known = 1 };
	nodes[node_count++] = (struct node) { .output_reg = var_stacksize, .statically_known = 1 };

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



	exit(1);
*/




}
























































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



*/











/*

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








*/



