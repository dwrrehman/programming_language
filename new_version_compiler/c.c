// programming language compiler 
// written on 2411203.160950 dwrr
// progress on 1202412312.030502
// progress on 1202412312.221307

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
	do_, bc, sc, at, lf, eoi,
	isa_count
};

static const char* ins_spelling[isa_count] = {
	"", "zero", "incr",
	"set", "add", "sub", "mul", "div", "rem", 
	"not", "and", "or", "eor", "si", "sd", 
	"lt", "ge", "ne", "eq", "ld", "st", 
	"do", "bc", "sc", "at", "lf", "eoi",
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

enum language_systemcalls {
	system_exit,
	system_read, system_write, 
	system_open, system_close,
	systemcall_count
};

static const char* systemcall_spelling[systemcall_count] = {
	"system_exit",
	"system_read", "system_write", 
	"system_open", "system_close",
};

struct instruction {
	nat args[8];
	nat count;
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

static nat get_call_input_count(nat n) {
	if (n == system_exit) return 1;
	if (n == system_read) return 3;
	if (n == system_write) return 3;
	if (n == system_close) return 1;
	if (n == system_open) return 3;
	abort();
}

static nat get_call_output_count(nat n) {
	if (n == system_exit) return 0;
	if (n == system_read) return 1;
	if (n == system_write) return 1;
	if (n == system_close) return 1;
	if (n == system_open) return 2;
	abort();
}

static void debug_instruction(struct instruction this, char** names) {
	for (nat a = 0; a < this.count; a++) {
		const char* str = "";
		if (a == 0) str = ins_spelling[this.args[a]]; else str = names[this.args[a]];
		printf(" %s ", str);
	}
}

static void debug_instructions(
	struct instruction* ins, 
	nat ins_count, char** names
) {
	printf("instructions: (%llu count) \n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("[%3llu] = ins(\" ", i);
		debug_instruction(ins[i], names); puts("\")");
	}
	puts("done\n");
}

static void print_instruction_index(
	struct instruction* ins, 
	nat ins_count, char** names,
	nat this, const char* note
) {
	printf("(%llu instructions)\n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		printf("%5llu:\t", i);
		debug_instruction(ins[i], names);
		if (i == this) printf("   <---- %s\n", note); else puts("");
	}
	puts("");
}


static void debug_dictionary(char** names, nat name_count) {
	printf("dictionary: %llu\n", name_count);
	for (nat i = 0; i < name_count; i++)
		printf("var #%5llu:   %-25s   ---->    %llu\n", i, names[i], 0LLU);
	puts("done");
}

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

static void debug_nats_indicies(nat* a, nat c, char** names) {
	printf("(%llu){ ", c);
	for (nat i = 0; i < c; i++) {
		if (a[i] != (nat) -1) printf(" %s:i%lld ", names[i], a[i]);
	}
	puts("}");
}

static bool is_branch(nat op) {
	return op == lt or op == ge or op == ne or op == eq;
}

static nat compute_ins_gotos(nat* side, struct instruction* ins, nat ins_count, nat this) {
	const nat op = ins[this].args[0];	
	if (op == do_) {
		const nat label = ins[this].args[1];
		for (nat i = 0; i < ins_count; i++) {
			if (ins[i].args[0] == at and ins[i].args[1] == label) {
				return i;
			}
		}
		puts("error: branch destination not attributed");
		abort();

	} else if (is_branch(ins[this].args[0])) {
		const nat label = ins[this].args[3];
		for (nat i = 0; i < ins_count; i++) {
			if (ins[i].args[0] == at and ins[i].args[1] == label) {
				*side = i;
				return this + 1;
			}
		}
		puts("error: branch destination not attributed");
		abort();

	} else return this + 1;
}

static void compute_all(
	struct instruction* ins, nat ins_count, 
	char** names, nat name_count
) {
	bool* ctk = calloc(name_count, sizeof(bool));
	nat* values = calloc(name_count, sizeof(nat));	
	bool* visited = calloc(ins_count, sizeof(bool));
	nat* stack = calloc(ins_count, sizeof(nat));
	nat stack_count = 0;
	nat* list = calloc(8 * ins_count, sizeof(nat));
	nat list_count = 0;

	stack[stack_count++] = 0;

	const nat write_access = (nat) (1LLU << 63LLU);

	while (stack_count) {
		const nat top = stack[--stack_count];
		printf("visiting ins #%llu\n", top);
		print_instruction_index(ins, ins_count, names, top, "here");
		visited[top] = true;

		const nat op = ins[top].args[0], arg1 = ins[top].args[1], arg2 = ins[top].args[2];

		if (op == zero) {
			ctk[arg1] = true;
			values[arg1] = 0;

			list[list_count++] = write_access | arg1;

		} else if (op == set) {
			if (ctk[arg2]) {
				ctk[arg1] = true; 
				values[arg1] = values[arg2]; 
			}

			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;


		} else if (op == ld) { // todo: do these.
			// 1. enforce that the loadsize is CT.
			abort();
		} else if (op == st) {
			// 1. enforce that the loadsize is CT.
			abort();
		} else if (op == bc) {
			// 1. enforce that the bit_count is CT.
			abort();

		} else if (op == eq) {

			list[list_count++] = arg1;
			list[list_count++] = arg2;
		
			if (not ctk[arg1] or not ctk[arg2]) goto skip_br;
			const bool condition = values[arg1] == values[arg2];
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if (not condition) {
				if (false_side < ins_count and not visited[false_side]) 
					stack[stack_count++] = false_side;
			} else {
				if ( true_side < ins_count and not visited[true_side])  
					stack[stack_count++] = true_side;
			}
			continue;
			skip_br:;

		} else if (op == incr) {
			if (ctk[arg1]) values[arg1]++;
			list[list_count++] = arg1;
			list[list_count++] = write_access | arg1;			

		} else if (op == not_) {
			if (ctk[arg1]) values[arg1] = ~values[arg1];
			list[list_count++] = arg1;
			list[list_count++] = write_access | arg1;

		} else if (op == add) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] += values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;

		} else if (op == sub) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] -= values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;

		} else if (op == mul) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] *= values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;

		} else if (op == div_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] /= values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;

		} else if (op == rem) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] %= values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;

		} else if (op == and_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] &= values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;

		} else if (op == or_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] |= values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;

		} else if (op == eor) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] ^= values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;

		} else if (op == si) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] <<= values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;

		} else if (op == sd) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] >>= values[arg2];

			list[list_count++] = arg1;
			list[list_count++] = arg2;
			list[list_count++] = write_access | arg1;



		} else if (op == sc) {
			if (not ctk[arg1]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}

			const nat n = values[arg1];
			const nat input_count = get_call_input_count(n);
			const nat output_count = get_call_output_count(n);

			for (nat i = 0; i < input_count; i++) 
				list[list_count++] = ins[top].args[2 + i];
		
			for (nat i = 0; i < output_count; i++) {
				list[list_count++] = write_access | ins[top].args[2 + i];
				ctk[ins[top].args[2 + i]] = false;
			}

			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(ins, ins_count, names, top, "CFG termination point here");
				continue;
			}
		}

		nat true_side = (nat) -1;
		const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
		if (false_side < ins_count and not visited[false_side]) stack[stack_count++] = false_side;
		if ( true_side < ins_count and not visited[true_side])  stack[stack_count++] = true_side;
	}


	for (nat i = 0; i < ins_count; i++) {
		if (not visited[i]) {
			printf("warning: instruction is unreachable\n");
			print_instruction_index(ins, ins_count, names, i, "unreachable");
			puts("");
		}
	}

	printf("found list: (%llu count)\n", list_count);
	for (nat i = 0; i < list_count; i++) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);
	}
	puts("ready for forming register interference graph yay");


	bool* alive = calloc(name_count, sizeof(bool));

	for (nat i = list_count; i--;) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);

		const bool is_write = !!(list[i] >> 63);

		if (is_write) {			
			alive[variable_index] = 0;
		} else {
			alive[variable_index] = 1;			
		}
		
		printf("alive = { ");
		for (nat n = 0; n < name_count; n++) {
			if (alive[n]) printf("%s  ", names[n]);
		} 
		printf(" }\n");
		getchar();
	}

}


int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run [file.s]"));
	printf("isa_count = %u\n", isa_count);
	
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
		const char*const*const list = first ? ins_spelling : (const char*const*const) names;
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

	debug_dictionary(names, name_count);
	debug_instructions(ins, ins_count, names);

	puts("computing CFG...");
	for (nat i = 0; i < ins_count; i++) {
		nat true_side = (nat) -1;
		const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, i);

		printf("[%llu]: ", i);
		debug_instruction(ins[i], names);
		printf(" --- gotos = [.0 = %lld, .1 = %lld]\n", false_side, true_side);
	}
	puts("done");

	compute_all(ins, ins_count, names, name_count);


}



























































// TODO: 1202412312.030917
// please figure out   system calls,   knowing their sysnumber arg at ct. 
// use that to know cfg termination poitns, instead of the "at done do done" mech we put it.
// and also use sc's to know the data flow effects (inputs and outputs)  of sc's  so taht we can detect unused variables better!
// from there, i think we are going to walk the cfg backwards, starting from the sc termination points,
// and then we are going to keep track of the live variables, our "liveset" array,
// and then we'll be actively forming the RIG on the fly while walking the cfg, based on this live set. 
// if the live set ever becomes 32 elements long, we know that we cannot do RA.
// note, sc's are the sink of usages. its critical we know these final usages of data in the program, to work backwards from!

 
//  X   1. constant propogation / CT evaulation     ---->   compute_argument_value();

// -->  2. system call identification / inputs/outputs  ---> compute_system_call_type();

// 3. cfg termination point identification   ---> compute_sc_is_halt();
// 4. liveness data flow analysis, cfg traversal algorithm backwards using pred constructing RIG ---> compute_RIG();
// 5. register allocation. ----> allocate_registers(RIG);




/*

 we should not execute at  ct    branches which are compiletime known  to never execute on one of their sides.
 we shouldnt even neccessarily warn the user about this.   unless they ask? lolollll

 note:   all ct variable, and ct known instructions  (basically speaking)         should not appear in the exucetable. EVER.
 ie,

	    they don't have any affect on RA!!!    ct variables    don't take up any room!!! thats the cool part. 

	    note this!!! this is improtant. 





 to properly test this system,  (and not have the compiler optimize away and eval the whole program lol) 

	  we NEEDDD to introduce   the        sba ins!             

					                    sba  0/1/2/3  bit_count 


	lets do that now! 






todo:

	- introduce sba    into const prop
	- introduce loads and stores into const prop
	- make ct branches actually happen! of course, there will always be the property that we never execute/visit a node we have already visited though. thats the thing. but, we should only visit branches which actually are executable, and ct known to possibly happen lol. shouldnt be that bad. lets start with eq/ne branches. 
	- we need to print out all  the sc argument sizes for inputs/outputs. make that a pass to test out the sc identification and arity deduction lol. 




*/


























/*static bool compute_ins_is_halt(
	struct instruction* ins, nat ins_count, 
	nat this, char** names, nat name_count
) {
	const nat op = ins[this].args[0];
	if (op != sc) return false;
	nat value = (nat) -1;
	bool ct = compute_argument_value(&value, ins, ins_count, this, 1, names, name_count);
	return ct and value == system_exit;
}*/

/*static nat* compute_ins_pred(
	nat* pred_count, 
	struct instruction* ins, nat ins_count, 
	nat this, char** names, nat name_count
) {	
	if (ins[this].args[0] != at) {
		if (not this or ins[this - 1].args[0] == do_ or 
			compute_ins_is_halt(ins, ins_count, this - 1, names, name_count)
		) {
			*pred_count = 0;
			return NULL;
		}
		*pred_count = 1;
		nat* result = malloc(sizeof(nat));
		result[0] = this - 1;
		return result;
	} 
	
	nat* result = NULL;
	nat count = 0;
	const nat label = ins[this].args[1];

	if (this and 
		((ins[this - 1].args[0] == do_ and ins[this - 1].args[1] == label) 
		or (ins[this - 1].args[0] != do_ and 
		not compute_ins_is_halt(ins, ins_count, this - 1, names, name_count)))
	) {
		(*pred_count)++;
		result = malloc(sizeof(nat));
		result[count++] = this - 1;			
	}

	for (nat i = 0; i < ins_count; i++) {
		const nat op = ins[i].args[0];
		if (((is_branch(op) and ins[i].args[3] == label) or 
			 op == do_ and ins[i].args[1] == label) and 
			(i != this - 1)) {
			result = realloc(result, sizeof(nat) * (count + 1));
			result[count++] = i;
		}
	}
	*pred_count = count; 
	return result;
}
*/









		/*if (top == this) {
			const nat n = ins[this].args[arg];
			*out_value = values[n];
			return ctk[n];
		}*/


























/*	puts("computing precedents of CFG...");
	for (nat i = 0; i < ins_count; i++) {

		nat pred_count = 0;		
		nat* pred = compute_ins_pred(&pred_count, ins, ins_count, i, names, name_count);

		printf("[%llu]: ", i);
		debug_instruction(ins[i], names);
		printf(" --- pred = { ");
		for (nat a = 0; a < pred_count; a++) {
			printf("%llu ", pred[a]);
		}
		puts("}");
	}
	puts("done");

	for (nat i = 0; i < ins_count; i++) {
		nat pred_count = 0;		
		compute_ins_pred(&pred_count, ins, ins_count, i, names, name_count);

		if (i and not pred_count) {
			printf("error: instruction is unreachable\n");
			print_instruction_index(ins, ins_count, names, i, "unreachable");
			//abort();
		}
	}

	// constant propogation:

	for (nat i = 0; i < ins_count; i++) {

		const nat op = ins[i].args[0];
		printf("%llu:  \t .op = %s : ", i, ins_spelling[op]);

		for (nat a = 0; a < isa_arity(op); a++) {
			nat value = (nat) -1;
			bool ct = compute_argument_value(&value, ins, ins_count, i, a + 1, names, name_count);

			printf(".%llu={arg=\"%s\", ct:[%s], .val=%lld}  ",
				a, names[ins[i].args[a + 1]], 
				ct ? "CT" : "RT", value
			);
		}
		puts("");		
	}

	for (nat i = 0; i < ins_count; i++) {
		bool h = compute_ins_is_halt(ins, ins_count, i, names, name_count);
		if (h) {
			printf("error: found halt instruction\n");
			print_instruction_index(ins, ins_count, names, i, "treating as cfg termination point");
		}
	}


	*/











/*

			// 1. enforce storage_type and bit_count to be compiletime known.
			// 2. the register first arg   is only required to be CT known if   
					storage_type is CT and storage_type == 0, 
			// 3. modify a variable to be runtime known always, if storage_type is nonzero.
				
				type == 0 : compiletime known  (neither register, nor memory storage)
				type == 1 : must be a register variable
				type == 2 : must be a memory variable
				type == 3 : must be runtime known.

					we could typothetically simplify this to be only 


					type == 0 : CT 
					type == 1 : RT, register 



						why not?...      why do we care about using memory or not lol. like.. that just seems odd to me. hmm. 


						in which case, we can actually simplify this to just include the bit count, and then have bit count of 0 to be compiletime known. that makes a million times more sense.    yeah, lets do that lol.  




			*/
























/*


static nat* compute_ins_live_in(nat* live_count, struct instruction* ins, nat ins_count, nat this) {

	nat pc = this;

	while (pc or stack_count) {

		
	}


}


*/












/*

static bool is_halt(struct instruction* ins, nat ins_count, nat this) {
	if (this == ins_count - 1) return false;
	const nat op = ins[this].args[0];	
	if (op != at) return false;
	const nat op2 = ins[this + 1].args[0];
	if (op2 != do_) return false;
	const nat label = ins[this].args[1];
	const nat label2 = ins[this + 1].args[1];	
	return label == label2;
}



	for (nat i = 0; i < ins_count; i++) {
		const bool h = is_halt(ins, ins_count, i);

		if (h) {
			printf("info: instruction is a halt instruction\n");
			print_instruction_index(ins, ins_count, names, i, "treating as CFG termination point");
			//abort();
		}
	}
*/



// do this      wayyyy later plz:





	// now walk the cfg backwards, starting from the cfg termination points, 
	// using a stack to see other decisions you must consider, 
	
	// and all the while    you are actually constructing the RIG, while walking, keeping track of edges between rig nodes, by looking at the live in lists for each instruction you encounter along the execution path, if you see a pair of variables live at the same time, then you know that those vars must have an edge between them (they interfere) in the rig. 

	// note, a variable is no longer live   before its definition,  (ie, after, becuase you are going backwards)

	// and note, you never need to store the live in lists. they are implicitly constructed and discarded during this process, just to know the RIG. thats their whole point. 






	// also, via this method, you can see if a variable is set but unused, ie, it has a defintion, but no use. basically, there will always be some instruction, where the value doesnt go anywhere. note, if a value is implicated in a system call, thats a use-sink. (depends on the system call, though...)

	// you can also detect conditional initialization of variables, another type of warning! this is done in the process of tracing the possible definitions for a given variable, and seeing that on some execution paths, the variable is not set. 



	// maybe we should be going forwards in this analysis though.. hmmmmmmmmm

















































/*struct vec {
	nat* data;
	nat count;
};*/


/*
nat* visited; // stack of instruction indicies
nat visited_count;

struct stackentry {
	nat* defs;  // a name_count-sized array  of instruction indicies
	nat side; // 0 or 1 	of branch side
	nat visited_count; // the height of the visited stack at the time of the branch.
	nat pc;
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


	11. done?...




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
	do sba sc at lf eoi





the data we need to generate: 


	nat* pred;
	nat pred_count;
// dfg:
	nat* inputs[8];
	nat input_count[8];
	nat arity;

	nat output;

	nat sk;
// ra:
	nat** live_in;
	nat** live_out;
	nat live_in_count;
	nat live_out_count;








static bool compute_argument_value(nat* value, struct instruction* ins, nat ins_count, nat this, nat arg, char** names) {

	bool* visited = calloc(ins_count, sizeof(bool));
	nat* stack = calloc(ins_count, sizeof(nat));
	nat stack_count = 0;
	stack[stack_count++] = 0;

	while (stack_count) {
		const nat top = stack[--stack_count];

		if (not visited[top]) {
			printf("visiting ins #%llu\n", top);
			print_instruction_index(ins, ins_count, names, top, "here");
			visited[top] = true;
		}

		nat true_side = (nat) -1;
		const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
		if (not visited[false_side]) stack[stack_count++] = false_side;
		if (true_side != (nat) -1 and not visited[true_side]) stack[stack_count++] = true_side;
	}

	return false;
}




*/
























