// programming language compiler 
// written on 2411203.160950 dwrr


/*





	state of the art: stages for the backend:


		1. form the cfg blocks, each of which is an instruction. use the original instructions, and just make the cfg bigger i think lol.
		2. connect up these cfg nodes (each, an ins) using the "at" and lt/do/ge/ne/eq instructions and their labels. this forms the cfg.
		3. do SK analysis while doing the data flow / live-in/live-out analysis. 
			3.1. walk the cfg (according to results from step 2), and find which instructions depend on the results of what other instructions. 
			3.2. compute a list of instructions which are the inputs to this instruction, as well as which variables, are required to be alive over the life of this instruction ( uhhh ...?)

			3.3. keep track of which instructions only have compiletime dependancies, via seeing if they are the output of system calls, 
				as well as  if the sta instruction was used on them to cause them to be only be RT known. 



		4. do CT/SK simplification of the CFG and instruction listing. ie, SK optimization. this happens often, and upfront. at this step. 


full isa:

	zero incr
	set add sub mul div rem
	not and or eor si sd
	lt ge ne eq ld st
	do sa sc at lf eoi



	zero light
	incr light

	zero i 
	at L incr i
	lt i 5 L

	
		
*/

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
	nullins, zero, incr,
	set, add, sub, mul, div_, rem,
	not_, and_, or_, eor, si, sd,
	lt, ge, ne, eq, ld, st,
	do_, sba, sc, at, lf, eoi,
	isa_count
};

static const char* ins_spelling[isa_count] = {
	"", "zero", "incr",
	"set", "add", "sub", "mul", "div", "rem", 
	"not", "and", "or", "eor", "si", "sd", 
	"lt", "ge", "ne", "eq", "ld", "st", 
	"do", "sba", "sc", "at", "lf", "eoi",
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
	nat args[8];
	nat count;

// cfg:
	nat gotos[2];
	nat* pred;
	nat pred_count;

// dfg:
	nat inputs[3];
	nat output;
	nat sk;

// ra:
	nat* live_in;  // wip
	nat* live_out; // wip
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

static nat isa_arity(nat i) {
	if (i == sc) return 7;
	if (not i or i == eoi) return 0;
	if (i == incr or i == zero or i == not_ or i == lf or i == at or i == do_) return 1;
	if (i == lt or i == ge or i == ne or i == eq or i == ld or i == st) return 3;
	return 2;
}


static void debug_instruction(struct instruction this, char** names) {
	printf("ins( (%llu){", this.count);
	for (nat a = 0; a < this.count; a++) {
		const char* str = "";
		if (a == 0) str = ins_spelling[this.args[a]]; else str = names[this.args[a]];
		printf(".%llu=%-4llu %-7s  ", a, this.args[a], str);
	}
	for (nat a = 0; a < 4 - this.count; a++) printf(".%u=%-4d %-7s  ", 0, 0, "");
	printf("}, \n\t(%llu){ ", this.pred_count);
	for (nat i = 0; i < this.pred_count; i++) printf("%llu ", this.pred[i]);
	printf("}--->gotos={%llu, %llu}, [%s] {%llu, %llu, %llu}-->[%llu] .. LO / LI )\n", 
		this.gotos[0], this.gotos[1], this.sk ? "SK" : "  ",
		this.inputs[0], this.inputs[1], this.inputs[2], this.output
	);
}

static void debug_instructions(
	struct instruction* ins, 
	nat ins_count, char** names
) {
	printf("instructions: (%llu count) \n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("[%3llu] = ", i);
		debug_instruction(ins[i], names);
	}
	puts("done\n");
}

static void debug_dictionary(char** names, nat* locations, nat name_count) {
	printf("dictionary: %llu\n", name_count);
	for (nat i = 0; i < name_count; i++)
		printf("var #%5llu:   %-25s   ---->    %llu\n", i, names[i], locations[i]);
	puts("done");
}





struct stack_entry {
	nat* defs;  // a name_count-sized array  of instruction indicies
	nat side; // 0 or 1 	of branch side
	nat visited_count; // the height of the visited stack at the time of the branch.
};



/*
nat* visited; // stack of instruction indicies
nat visited_count;

struct stackentry {
	nat* defs;  // a name_count-sized array  of instruction indicies
	nat side; // 0 or 1 	of branch side
	nat visited_count; // the height of the visited stack at the time of the branch.
};


the fundemental intuition behind our approach is the following:


	1. we need to start data flow analysis starting from instruction #0. 

	2. we must traverse the entire cfg, and along an execution path, keeping track constantly  of what variables are alive, 
			anddd if they areee alive, (defs[dict_index_for_variable] != -1) then we take note of the latest instruction which produces its latest value.

	3. the idea here is that we are finding actual instruction indicies    for a given instruction, who is the producer of the latest value of a variable. thenn, when we see that a name is used again, we set the    .inputs[X] = <instruction index Y>   where X ranges in {0, 1, 2} depending on which argument of the instruction we are dealing with, and the instruction's arity, and Y ranges in LRS(ins_count), and corresponds to the most recent prior instruction ALONG THIS EXECUTION PATH in the cfg, which produces the latest value of this variable. Y is the ins index of this instruction which produces the latest value of this variable. 


	4. in order to accomplish this, we treat the cfg like a binary tree. 

	5. to traverse this tree, we need to have a treestack, of stackentry's.   we push a new entry onto this stack upon encountering a binary branch. (all branches are binary in this language!)

	6. we also keep track of the side of the branch we went on, inside the stackentry,  as well as the current state of the def's mapping, at this point in the program. 
	7. note: arguably we should have a full def's array, for every single instruction.. we might do that. idk. 
	
		7.5 in order to cope with that, we would push a new stack entry, on each instruction, 
			instead of each branch. .side would be 0 for unconditional instructions. 

	8. in order to deal with the fact that the cfg is not ACTUALLLY a binary tree, we keep track of a list of visited nodes, seperate from the treestack.

	9. this visited node list  is a simple list of instruction indicies, in which every instruction we execute, we push the current index for this instruction to that list. when we push a new treestack entry, we also include the current size of the visited node list, in that stack entry. this helps us to revert the list of visited nodes to the right place, when we CHANGE SIDES of the branch. 


	10. finally, we begin the backtracking process (ie, switching sides, or popping off the current TOS (top of stack)  when we reach an instruction which is either a CFG termination point, (sc ins, with 0 syscall number), or an instruction we have already encountered. 


	11. 

*/





static void print_nats(nat* a, nat c) {
	printf("(%llu){ ", c);
	for (nat i = 0; i < c; i++) {
		printf("%lld ", a[i]);
	}
	puts("}");
}



static void print_nats_indicies(nat* a, nat c) {
	printf("(%llu){ ", c);
	for (nat i = 0; i < c; i++) {
		if (a[i] != (nat) -1) printf("%llu:%lld ", i, a[i]);
	}
	puts("}");
}



int main(int argc, const char** argv) {
	if (argc > 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));

	printf("isa_count = %u\n", isa_count);
	
	char* names[4096] = {0};
	nat locations[4096] = {0};
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
	filestack[0].index = 0;
	printf("file: (%llu chars)\n<<<%s>>>\n", text_length, text);}

	for (nat i = 0; i < builtin_count; i++) names[name_count++] = strdup(builtin_spelling[i]);

process_file:;
	nat word_length = 0, word_start = 0, first = 1, comment = 0;
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
		printf("[first=%llu,com=%llu]: [%llu]: at word = %s\n", first, comment, word_start, word);
		if (comment) { if (not strcmp(word, ".")) comment = 0; goto next_word; }
		char** list = first ? (char**) ins_spelling : (char**) names;
		const nat count = first ? isa_count : name_count;
		nat calling = 0;
		for (; calling < count; calling++) 
			if (not strcmp(list[calling], word)) goto found;
		if (first) { 
			if (not strcmp(word, ".")) { comment = 1; goto next_word; }
			else { printf("unknown word: %s\n", word); abort(); }
		}
		names[name_count++] = word;

	found:	if (first) {
			if (not strcmp(word, ins_spelling[eoi])) break;
			ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
			ins[ins_count++] = (struct instruction) {0};
			first = 0;
		}
		struct instruction* this = ins + ins_count - 1;
		this->args[this->count++] = calling;
		if (this->count != isa_arity(this->args[0]) + 1) goto next_word;
		if (this->args[0] == at) {
			locations[this->args[1]] = --ins_count;
		} else if (this->args[0] == lf) {
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
	debug_dictionary(names, locations, name_count);
	puts("finshed parsing!");

	for (nat i = 0; i < ins_count; i++) {
		printf("finding cfg connections for: #%llu: ", i); debug_instruction(ins[i], names);

		nat dest = ins[i].args[0] == do_ ? locations[ins[i].args[1]] : i + 1;
		ins[i].gotos[0] = dest;
		if (dest < ins_count) {
			ins[dest].pred = realloc(ins[dest].pred, sizeof(nat) * (ins[dest].pred_count + 1));
			ins[dest].pred[ins[dest].pred_count++] = i;
		}

		if (ins[i].args[0] >= lt and ins[i].args[0] <= eq) {
			dest = locations[ins[i].args[3]];
			ins[i].gotos[1] = dest;
			if (dest < ins_count) {
				ins[dest].pred = realloc(ins[dest].pred, sizeof(nat) * (ins[dest].pred_count + 1));
				ins[dest].pred[ins[dest].pred_count++] = i;
			}
		}

		const nat op = ins[i].args[0];

		if (op == zero) {
			ins[i].output = ins[i].args[1];
			ins[i].sk = 1;
		} else ins[i].output = ins[i].args[1];
	}


	puts("\n\n\n\nstarting DFG formation pass...");

	nat* defs = calloc(name_count, sizeof(nat));
	memset(defs, 255, name_count * sizeof(nat));

	nat* visited = NULL;
	nat visited_count = 0;

	struct stack_entry stack[4096] = {0};
	nat stack_count = 0;


	nat* starting_def = calloc(name_count, sizeof(nat));
	memcpy(starting_def, defs, name_count * sizeof(nat)); 

	stack[stack_count++] = (struct stack_entry) {.defs = starting_def};

	for (nat pc = 0; pc < ins_count; ) {
		printf("traversing ins #%llu:  ", pc); debug_instruction(ins[pc], names);
		const nat op = ins[pc].args[0];

		printf("op def was: "); 
		print_nats_indicies(defs, name_count);


		if (op == zero) {
			defs[ins[pc].args[1]] = pc;
		}
		if (op == set) {
			defs[ins[pc].args[1]] = pc;
		}
		if (op == incr) {
			defs[ins[pc].args[1]] = pc;
		}
		if (op == add) {
			defs[ins[pc].args[1]] = pc;
		}
		if (op == sub) {
			defs[ins[pc].args[1]] = pc;
		}


		printf("op def now: "); 
		print_nats_indicies(defs, name_count);

		printf("stack: (%llu) { \n", stack_count);
		for (nat i = 0; i < stack_count; i++) {
			printf("\t#%llu: entry(.side=%llu, .visited=%llu, .def={ ", i, stack[i].side, stack[i].visited_count);
			for (nat n = 0; n < name_count; n++) {
				if (stack[i].defs[n] != (nat) -1) printf("%llu:%llu ", n, stack[i].defs[n]);
			}
			printf("})\n");
		}
		puts("}");
			
		for (nat i = 0; i < visited_count; i++) {
			if (visited[i] == pc) {
				printf("@ %llu: found loopback point in CFG!\n", pc);
				const struct stack_entry top = stack[--stack_count];
				visited_count = top.visited_count;

				printf("loopback defs was: "); 
				print_nats_indicies(defs, name_count);

				memcpy(defs, top.defs, name_count * sizeof(nat));

				printf("loopback defs now: "); 
				print_nats_indicies(defs, name_count);


				if (not stack_count) goto done_traversal;
				goto node_found;
			}
		}

		printf("no loopback detected, pushing instruction...\n");

		visited = realloc(visited, sizeof(nat) * (visited_count + 1));
		visited[visited_count++] = pc;

		stack[stack_count - 1].visited_count = visited_count;

		puts("visited nodes: "); print_nats(visited, visited_count);

		node_found:;

		memcpy(stack[stack_count - 1].defs, defs, name_count * sizeof(nat));
		
		if (op >= lt and op <= eq) {
			nat* copy = calloc(name_count, sizeof(nat));
			memcpy(copy, defs, name_count * sizeof(nat)); 
			stack[stack_count++] = (struct stack_entry) {.visited_count = visited_count, .defs = copy};
		}

		

		pc = ins[pc].gotos[0];
		//sleep(1);
	}

done_traversal:

	print_nats_indicies(defs, name_count);
	debug_instructions(ins, ins_count, names);
	puts("finshed cfg!");

	exit(1);
}


/*


if (op == incr) {
			ins[pc].sk = ins[pc].args[1];
		}


*/





































































		//printf("generated node #%llu into block %llu (of %llu): \n", 
		//	node_count, block_count, block->dag_count);
		//puts("inputs and outputs null for now");
		//printf("generating DAG node for ins: { %s ", ins_spelling[op]);
		//for (nat a = 0; a < ins[i].count; a++) printf(" %llu   ", ins[i].args[a]);
		//printf("statically_known = %llu\n", statically_known);













































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





/*

	nat* data_outputs;
	nat data_output_count;
	nat* data_inputs;
	nat data_input_count;
	nat* predecessors;
	nat predecessor_count;
	nat successor;
	nat dag_count;
	nat* dag;





static void print_nodes(struct node* nodes, nat node_count) {
	printf("printing %3llu nodes...\n", node_count);
	for (nat n = 0; n < node_count; n++) {

		printf("[%s] node #%-5llu: {"
			".op=%2llu (\"%-10s\") "
			".or=%2llu (\"%-4llu\") "
			".oc=%2llu "
			".ic=%2llu "
			".io={ ", 
			nodes[n].statically_known ? "SK" : "  ", n, 
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
			printf("\tnode %3llu:  %7s  %3llu\n\n", 
				blocks[b].dag[d], ins_spelling[nodes[blocks[b].dag[d]].op], 
				nodes[blocks[b].dag[d]].output_reg
			);
		}
		puts("}");
	}
	puts("[end of node cfg]");
}




*/



		/*

	nat args[3];
	nat count;
	nat child[2];
	nat inputs[3];
	nat output;
	nat sk;
	nat* live_in;  // wip
	nat* live_out; // wip

*/















/*
	struct node nodes[4096] = {0}; 
	nat node_count = 0;
	nodes[node_count++] = (struct node) { 
		.output_reg = 0, 
		.statically_known = 1 
	};


	puts("stage: constructing data flow DAG...");

	struct basic_block blocks[4096] = {0};
	nat block_count = 0;
	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].args[0];
		if (op == at and blocks[block_count].dag_count) block_count++;
		struct basic_block* block = blocks + block_count;
		block->dag = realloc(block->dag, sizeof(nat) * (block->dag_count + 1));
		block->dag[block->dag_count++] = node_count;
		nodes[node_count++] = (struct node) { .op = op };

		if (	ins[i].args[0] == lt or ins[i].args[0] == ge or 
			ins[i].args[0] == ne or ins[i].args[0] == eq
		) block_count++;
	}
 	if (blocks[block_count].dag_count) block_count++;
	
	puts("done creating isa nodes... printing nodes:");
	print_nodes(nodes, node_count);

	puts("done creating basic blocks... printing cfg/dag:");
	print_basic_blocks(blocks, block_count, nodes);









##############################
##.#.#...#.###.###...#########
##.#.#.###.###.###.#.#########
##...#...#.###.###.#.#########
##.#.#.###.###.###.#.#########
##.#.#...#...#...#...#########
##############################
##############################
##############################
##############################
##############################
##############################
##############################
##############################
##############################
##############################
##############################
##############################
##############################
##############################
##############################
##############################







*/





