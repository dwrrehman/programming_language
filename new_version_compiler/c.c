// programming language compiler 
// written on 2411203.160950 dwrr
// progress on 1202412312.030502
// progress on 1202412312.221307



/*

1202501083.161719

so yeah, the current plan is definitely to:


	1. form the   numeric (index based)   cfg   ds,    while parsing/lexing, 

	2. not have ANYYYYY labels in the code, after parsing is finished. only ins indexes.

	3. not have ANYYYYY implicit ip++'s present in the code. ie, all instructions could be reordered in any way, and the codes meaning wouldnt change AT ALL.

	4. we need to generate a new    rt_ins     array    as the output of   stage1 ct eval.  

		the only instructions we generate are ones which are     containing a rt destination, or    rtk cond br.

	5. 






1202501083.161205
OH MY GOSH    THE LABEL HAS TO BE PART OF THE INSTRUCTIONNNN

	and thennnn

			the goto's for an instruction     are always       label names!!!!    not instruction indexes lol 
		(at least, this would kindaaa solve it lol.  while still allowing things to be editable easily lol 

			)


	idkkk hmm


	i mean we could just as easily   just allow for the   cfg connections to be numbers instead of labels  ie,        ins indexes  

		i feel like thats just better lol 


		hmmm  but yeah then when we see a label, we neeeedddd to like.. interpret it    as      NOT   a variable.   very important, actually that it isnt one. 


		we need to replace it with  like...       the instruction index, instead.   veryyyyyyy important lol. so yeah.. cool.  interesting. 









basicallyyyy we are trying to NOTTTTT rely on the source ordering lolll

LIKE      AT ALLLLLL       ONLYYYYYYYY execution for things.   ONLY.    thats the only way we traverse the code that the user gave.   is via   execution.  ONLY.











1202501083.021325


		i figured out the main issue that we are having with    contradiction between  doing source ordering and execution ordering, 



				the root of the problem     is just  that 



					1. we need to do execution ordering, 

					2. we need to delete the   compute_gotos() function.   we NEEDDD to be notttt using    "at label"  in the representation of the programs cfg,   we need to actually have hardcoded instruction indexes,   for the    .true  and .false  sides of every single instruction.  (for unconditional ones, this will just have the .false side.)  but yeah, each instruction has its   sides    indexes     hardcoded.  when we edit the controlflow, we'll just have to adjust things. its just the way it has to be. alternatively, if you don't want that,   you could generate a million different  do and at instructions lol, for every single implicit "ip++"... buttttt 


							basically what we are tryinggggg to accomplish here  is  to have    some sort of.. invariant  to whereeeee basic blocks are actually placed in the instruction sequence, so thattttt we arent at all sensitive to where those blocks end up in the sequence,

								because of course,   when traversing a graph  via execution, and using a stack to traverse rt branches, 

									we might actually not put    those generated instructions at the right spot. 



					oh, also, we are generating a new list of instructions,   each time we want to edit things. 


						ie, unreachable analysis   actually just   generates a new list of    reachable  (rt_ins)    instructions,


						and ct unrolling    alsooo just generates a new set of instructions,    etc etc.



		now


			to allow for the instruction sequence to be mutable, 


				i'm actually not  totally against    inserting a crap ton of         do label      at label    instructions lol... 



	like 
			its not the best... 

				but i mean  it could work lol. 






			to make it so that reordering    where that block of instructions goes,     never changes anything. 




	its a bit verbose, but it could work lol. 

hmm









so yeah thats the current plan lol 



yay
















//struct instruction mi[4096] = {0};  // arg[0] is in "enum arm64_ins_set". args are in order of assembly format.
//nat mi_count = 0;



		1202501061.175212
		PROBLEM:


				we are like     partiallyyyy following control flow right now, via ct/do branches

				and so 


					it could be the case that some code that we skipped over, is actually 

						accessible      but we won't generate it     despite the fact that we should be generating it 




						we will skip over it,  because we are no longer looking at the  labels of rt branches.

								we either, need to be doing this, 


								orrrr we need to stick purelyyy to source ordering, ie,   top++.



							we can't mix and match these... partially like this lol 


								we need to be doing    full control flow graph analysisss  if we are to try to do that. 


									sooo..... yeah..


					


							honestly, i feel like we should just focus on generating the right code, and then having the    compute_goto's function form the resultant collapsed CFG..     i feel like thatsss what we are looking for here....  to have compute_goto's   actually      just    be able to represetn the new cfg for us.   not needddd any particular statement to be  in a particular order. 

						we need to be relying on the    logicallll connections,    not the source ordering. 


						this is key. 




mhm



	// i think we do generate it actually!!!


	// because we are trying to keep the RT structure in tact, and so we should reduce everything away, until we can easily reduce away the (RT) do itself   in a seperate pass!! cool, okay that makes sense lol. 










1202501061.175200
TODO:
		CURRENT STATE OF INS SEL:


	--->   redo where we are doing the pattern recognition to be in a seperate pass.

		instead of doing it in ct-eval stage, 



		1. generate a new list of ONLYYY RT instructions, 

						 (some of which the opcodes will 
						change to be elements in the  
							 "enum immediate_forms_instructions"!!!)
			{
	
				note, simply move pass (i++)   rt instructions, one generated. don't follow their execution.
					you only do this for compiletime branches. treat rt branches like  single nop instructions. 
			}
		FINE NOTE:

			if you encounter, a "do",  follow it, as its CT known. however, 
			just don't generate any RT instruction you've already visited before.
			this way, we will ignore unreachable rt code, 
			as well as avoid overtraversing the rt cfg.   NICEEEE YAYYYY




		2. then loop over this list, and doing pattern recognition on it. here, i don't think we should take into account the control flow of the RT instructions at all, so far.    
			this should generate a list of machine instructions,     the "mi" array above.  this ds uses the existing    "struct instruction"


		3. print out the generated machine instructions, and rt ins listing.

			then, you should start the process of looking at the reads and writes over those machine instructions. 

			this is when we start the process of register allocation. only here.  once the mi's have been generated in this mi[] list.
			
		
			3.1. we generate the list of reads and writes   based on the instruction semantics, 
					(note: we are doing it based on the mi instructions, in case a variable gets reduced away during ins sel.  very important. 


			3.2. then we go backwards through the reads and writes, generating the live-in lists, 
				keeping track of which variables are found in the same list, 
				and thus constructing the RIG from this information


			3.3.   we then use the RIG to do actual graph coloring based register allocation    ON THE     MI's. 


		4. generate machine code. we have everything we need now lol. op codes, and register numbers.    
			this step should be easy, as its already written lol.
			 
		5. done!!!! yayyyy


// TODO: recognize these three patterns:

ins sel   for      csinc     (conditional select increment)

RT COMPARISON:

ne X Y false
set d n
do done
at false
set d m incr d
at done


USING CONSTANTS IN COMPARISON:

ne X 3 false
set d n
do done
at false
set d m incr d
at done


NEGATING CONDITON:

eq X 3 false
set d m incr d
do done
at false
set d n
at done


	struct instruction mi[4096] = {0};  // arg[0] is in "enum arm64_ins_set". args are in order of assembly format.
	nat mi_count = 0;

ins sel patterns:	
	
	addsr {                 d = n + (m << k)

		set d m
		si d k
		add d n
		
		where k is ct, d, n and m are rt.
			k <= 63		
	}

	addsr (k = 0) {

		set d m
		add d n
		
		where d, n and m are rt. (k = 0)
	}


		if (	top >= head and 
			top + 2 < ins_count and 
			ins[top + 0].args[0] == set and 
			ins[top + 1].args[0] == si and
			ins[top + 2].args[0] == add and
			
			ins[top + 0].args[1] == ins[top + 1].args[1] and
			ins[top + 1].args[1] == ins[top + 2].args[1] and
			ctk[ins[top + 1].args[2]]
		) {
			mi[mi_count++] = top;
			mi[mi_count++] = addsr;
			head += 3;
		}


		if (	top + 1 < ins_count and 
			ins[top + 0].args[0] == set and 
			ins[top + 1].args[0] == add and
			ins[top + 0].args[1] == ins[top + 1].args[1]
		) {
			mi[mi_count++] = top;
			mi[mi_count++] = addsr_k0;
			head += 2;
		}




use this code later:



		if (visited[top] == 1) {

			puts("found this instruction for the first time!!! : ");
			debug_instruction(ins[top], names);
			puts("");
			

			if (state == 0) {
			retry:
				state = 0;
				dest = 0; source1 = 0; source2 = 0; immediate = 0;

				// set d m
				if (op == set and not ctk[arg1] and not ctk[arg2]) { state = 1; dest = arg1; source2 = arg2; } 

				else {}

			} else if (state == 1) {

				// si d k
				      if (op == si and arg1 == dest and not ctk[arg1] and ctk[arg2]) { state = 2; immediate = values[arg2]; }
				else if (op == add and arg1 == dest and not ctk[arg1] and not ctk[arg2]) { immediate = 0; goto generate_addsr; }
				else goto retry;

			} else if (state == 2) {

				// add d n
				if (op == add and arg1 == dest and not ctk[arg1] and not ctk[arg2]) { 
				generate_addsr:
					source1 = arg2;

					printf("FOUND ARM64 MACHINE CODE INSTRUCTION:\n");
					printf("ADD_SR   dest=%llu(%s), source1=%llu(%s), source2=%llu(%s) << immediate=%llu\n",
							dest, names[dest], 
							source1, names[source1], 
							source2, names[source2], 
							immediate
					);				
					state = 0; 
				} 

				else goto retry;

			} else if (state == 3) {

				goto retry;
			}
			
		}




		if (is_branch(op)) {
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if ( true_side < ins_count and visited[true_side] < 1)  stack[stack_count++] = true_side;
			if (false_side < ins_count and visited[false_side] < 1) stack[stack_count++] = false_side;
		} else {
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if ( true_side < ins_count) stack[stack_count++] = true_side;
			if (false_side < ins_count) stack[stack_count++] = false_side;
		} 







			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;

	//nat top = 0;

	//struct instruction rt_ins[4096] = {0};
	//nat rt_count = 0;
	//struct instruction mi[4096] = {0}; 
	//nat mi_count = 0;

        // state =  0, dest = 0, source1 = 0, source2 = 0, immediate = 0;

// top < ins_count

			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;


			//if (not ctk[arg2]) list[list_count++] = arg2;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;


			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg2]) list[list_count++] = arg2;

			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;


			//if (not ctk[arg1]) list[list_count++] = arg1;
			//if (not ctk[arg2]) list[list_count++] = arg2;
			//if (not ctk[arg1]) list[list_count++] = write_access | arg1;

			//rt_ins[rt_count++] = ins[top];
		//top++;		

	 
	printf("found modified  ins instruction sequence {\n");
	for (nat i = 0; i < ins_count; i++) {
		const char* op_name = rt_ins[i].args[0] < isa_count ? 
			ins_spelling[rt_ins[i].args[0]] : 
			ins_imm_spelling[rt_ins[i].args[0] - isa_count];		
		printf("\trt[%llu] = { %llu(%s) %llu(%s) %llu %llu %llu %llu %llu %llu } \n",
			i,  
			rt_ins[i].args[0], op_name,
			rt_ins[i].args[1], names[rt_ins[i].args[1]],
			rt_ins[i].args[2],
			rt_ins[i].args[3],
			rt_ins[i].args[4],
			rt_ins[i].args[5],
			rt_ins[i].args[6],
			rt_ins[i].args[7]			
		);
	}



	
			if (ctk[arg1] and ctk[arg2]) {
				if (condition) {
					ins[top].count = 2;
					ins[top].args[0] = do_;
					ins[top].args[1] = ins[top].args[3];
				} else {
					ins[top].count = 0;
				}
			}







	const nat is_imm = this.args[0] >= isa_count;
	if (is_imm) printf(" %s ", ins_imm_spelling[this.args[0] - isa_count]);
	else printf(" %s ", ins_spelling[this.args[0]]);
	for (nat a = 1; a < this.count; a++) {
		if (a == 2 and is_imm) printf(" IMM=%llu ", this.args[a]);
		else printf(" %s ", names[this.args[a]]);
	}

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
	null_ins, 
	zero, incr, decr, not_, 
	do_, at, ct, lf, 
	set, add, sub, mul, div_, 
	and_, or_, eor, si, sd, rt, 
	lt, ge, ne, eq, 
	ld, st, sc, eoi,
	isa_count
};

static const char* ins_spelling[isa_count] = {
	"__NULL_INSTRUCTION__",
	"zero", "incr", "decr", "not", 
	"do", "at", "ct", "lf",
	"set", "add", "sub", "mul", "div", 
	"and", "or", "eor", "si", "sd", "rt",
	"lt", "ge", "ne", "eq", 
	"ld", "st", "sc", "eoi"
};

enum language_builtins {
	stacksize, stackpointer,
	builtin_count
};

static const char* builtin_spelling[builtin_count] = {
	"_stacksize",
	"_stackpointer", 
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

enum arm64_ins_set {
	addsr, addsr_k0,        //TODO: add more 
	arm_isa_count,
};

enum immediate_forms_of_instructions {
	null_imm_unused = isa_count,

	set_imm,    // set r c
	add_imm,    // add r c	
	sub_imm,  
	mul_imm,   
	div_imm,
	and_imm,
	or_imm, 
	eor_imm, 
	si_imm, 
	sd_imm, 	
	lt_imm,     // lt r c label
	ne_imm,     // ne r c label
	ge_imm,     // ge r c label
	eq_imm,     // eq r c label
};

static const char* ins_imm_spelling[] = {
	"null_imm_unused",
	"set_imm",
	"add_imm", 
	"sub_imm",  
	"mul_imm",   
	"div_imm",
	"and_imm",
	"or_imm", 
	"eor_imm", 
	"si_imm", 
	"sd_imm", 	
	"lt_imm", 
	"ne_imm", 
	"ge_imm",  
	"eq_imm", 
};

static const nat write_access = (nat) (1LLU << 63LLU);

struct instruction {
	nat args[9];   // sc n  0 0 0   0 0 0  l
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};

static nat isa_arity(nat i) {
	if (i == sc) return 7;
	if (i == eoi) return 0;
	if (i >= zero and i <= lf) return 1;
	if (i >= set  and i <= rt) return 2;
	if (i >= lt   and i <= st) return 3;
	abort();
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
	if (n == system_read) return 2;
	if (n == system_write) return 2;
	if (n == system_close) return 1;
	if (n == system_open) return 2;
	abort();
}

static void debug_instruction(struct instruction this, char** names) {

	const nat op = this.args[0];

	if (not op) {
		printf("(null instruction): %llu %llu %llu %llu %llu %llu %llu %llu\n",
			this.args[1], this.args[2], this.args[3], 
			this.args[4], this.args[5], this.args[6], 
			this.args[7], this.args[8]
		);
		
	} else if (
		op == zero or 
		op == incr or 
		op == decr or 
		op == not_ or 
		op == ct
	) {
		const char* name = ins_spelling[op];
		printf(" %s %llu   ---> %llu\n",
			this.args[1], this.args[2]
		);

	} else if (
		op == set or 
		op == add or 
		op == sub or 
		op == mul or 
		op == div_ or
		op == and_ or 
		op == or_ or
		op == eor or
		op == si or
		op == sd or
		op == rt
	) {
		const char* name = ins_spelling[op];
		printf(" %s %llu %llu   ---> %llu\n",
			this.args[1], this.args[2], this.args[3]
		);

	} else {
		printf(" ...UNKNOWN INSTRUCTION...\n");
	}
}


static void debug_instructions(
	struct instruction* ins, 
	nat ins_count, char** names
) {
	printf("instructions: (%llu count) \n", ins_count);
	for (nat i = 0; i < ins_count; i++) {
		//if (not ins[i].count) continue;
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
		//if (not ins[i].count) continue;
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



	for (nat i = 0; i < ins_count; i++) {
		if (ins[i].args[0] == do_) aborT();
		if (ins[i].args[0] == at) aborT();
		if (ins[i].args[0] == lf) aborT();
		if (ins[i].args[0] == eoi) aborT();
		if (ins[i].args[0] == ct) aborT();
		if (ins[i].args[0] == rt) aborT();
	}


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



	bool* ctk = calloc(name_count, sizeof(bool));
	nat* values = calloc(name_count, sizeof(nat));	
	nat* bit_counts = calloc(name_count, sizeof(nat));	
	uint8_t* visited = calloc(ins_count, 1);
	nat* stack = calloc(2 * ins_count, sizeof(nat));
	nat stack_count = 0;
	nat* list = calloc(8 * ins_count, sizeof(nat));
	nat list_count = 0;
	stack[stack_count++] = 0; 
	//const struct instruction nop = {0};	
	ctk[stacksize] = 1;


	struct instruction rt_ins[4096] = {0};
	nat rt_count = 0;


	while (stack_count) { 
		printf("stack: %llu { \n", stack_count);
		for (nat i = 0; i < stack_count; i++) {
			printf("stack[%llu] = %llu\n", i, stack[i]);
		}
		puts("}");

		printf("visited: { ");
		for (nat i = 0; i < ins_count; i++) {
			printf("%hhu ", visited[i]);
		}
		puts("}");

		printf("ctk: { ");
		for (nat i = 0; i < name_count; i++) {
			if (ctk[i]) printf("%s ", names[i]);
		}
		puts("}");

		printf("CT values of variables: {\n");
		for (nat i = 0; i < name_count; i++) {
			if (not ctk[i]) continue;
			printf("\tvalues[%s] = %llu\n", names[i], values[i]);
		}
		puts("}");

		const nat top = stack[--stack_count];
		printf("visiting ins #%llu\n", top);
		print_instruction_index(ins, ins_count, names, top, "here");
		debug_instruction(ins[top], names); puts("");
		getchar();
		visited[top]++;
		const nat op = ins[top].args[0], arg1 = ins[top].args[1], arg2 = ins[top].args[2];
		if (op == ld) { abort();
		} else if (op == st) { abort();
		} else if (op == ct) { ctk[arg1] = 1; ins[top].count = 0;
		} else if (op == rt) {
			if (not ctk[arg2]) { puts("error: rt instruction arg2 (bit count) must be ct."); abort(); }
			bit_counts[arg1] = values[arg2];
			ctk[arg1] = 0;
			ins[top].count = 0;
		} else if (is_branch(op)) {
			if (not ctk[arg1] or not ctk[arg2]) goto generate_rt_branch;
			bool condition = 0;
			if (op == eq and values[arg1] == values[arg2]) condition = 1;
			if (op == ne and values[arg1] != values[arg2]) condition = 1;
			if (op == lt and values[arg1] <  values[arg2]) condition = 1;
			if (op == ge and values[arg1] >= values[arg2]) condition = 1;

			rt_ins[rt_count++] = ins[top];      //TODO


				ERROR ERROR ERROR       we are in the middle of making this pass   generate   rt_ins    never edit main instruction sequence, ins. 

					this is because we want to avoid generating    the at loop's,  and also  get the immediate to be different for each add_imm that we generate. we can't do that since we are overwriting the original instruction, which we used for execution lol. i think. sometihng like that. basically we need to do the rt_ins lol. its a must. don't worry about unreachable instructions, we won't generate them anyways, becuase we are traversing the graph in order to know the right instructions to output. i think. CRAP 


							BUT WHAT ABOUT THE FACT THAT 




									WE WONT BE GENERATING THE INSTRUCTIONS IN THE CORRECT ORDERRRR CRAPPPP



									DUE TO IMPLICIT IP++'s    NOT LINING UPPPP



				BECAUSE OF OUR GRAPH TRAVERSAL ORDERINGGGG   CRAPPPPPP









uh oh lol 


uhh





hmm





			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if (not condition) {
				if (false_side < ins_count) stack[stack_count++] = false_side;
			} else {
				if ( true_side < ins_count) stack[stack_count++] = true_side;
			}
			goto next_instruction;
			generate_rt_branch:;
		} else if (op == do_) {
			nat true_side = (nat) -1;
			const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
			if (false_side < ins_count) stack[stack_count++] = false_side;
			goto next_instruction;
		} else if (op == zero) {
			if (ctk[arg1]) { values[arg1] = 0; ins[top].count = 0; }
		} else if (op == incr) {
			if (ctk[arg1]) { values[arg1]++; ins[top].count = 0; }
		} else if (op == not_) {
			if (ctk[arg1]) { values[arg1] = ~values[arg1]; ins[top].count = 0; }
		} else if (op == add) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] += values[arg2];
		push_rt:;
			if (not ctk[arg1]) {
				if (ctk[arg2]) {
					ins[top].args[2] = values[arg2];
					if (op == set) ins[top].args[0] = set_imm;
					if (op == add) ins[top].args[0] = add_imm;
					if (op == sub) ins[top].args[0] = sub_imm;
					if (op == mul) ins[top].args[0] = mul_imm;
					if (op == div_)ins[top].args[0] = div_imm;
					if (op == and_)ins[top].args[0] = and_imm;
					if (op == or_) ins[top].args[0] = or_imm;
					if (op == eor) ins[top].args[0] = eor_imm;
					if (op == si)  ins[top].args[0] = si_imm;
					if (op == sd)  ins[top].args[0] = sd_imm;
				}
			} else {
				if (ctk[arg2]) {
					ins[top].count = 0;
				} else {
					puts("error: compiletime destination must have compiletime source."); abort();
				}
			}
		} else if (op == set) {
			if (ctk[arg2]) values[arg1] = values[arg2];
			goto push_rt;
		} else if (op == sub) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] -= values[arg2];
			goto push_rt;
		} else if (op == mul) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] *= values[arg2];
			goto push_rt;
		} else if (op == div_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] /= values[arg2];
			goto push_rt;
		} else if (op == and_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] &= values[arg2];
			goto push_rt;
		} else if (op == or_) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] |= values[arg2];
			goto push_rt;
		} else if (op == eor) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] ^= values[arg2];
			goto push_rt;
		} else if (op == si) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] <<= values[arg2];
			goto push_rt;
		} else if (op == sd) {
			if (ctk[arg1] and ctk[arg2]) values[arg1] >>= values[arg2];
			goto push_rt;
		} else if (op == sc) {
			if (not ctk[arg1]) { 
				puts("error: all system calls must be compile time known."); 
				abort(); 
			}
			const nat n = values[arg1];
			//const nat input_count = get_call_input_count(n);
			const nat output_count = get_call_output_count(n);
			//for (nat i = 0; i < input_count; i++) {
			//	//if (ctk[ins[top].args[2 + i]]) { puts("system call ct rt in"); abort(); }
			//	//list[list_count++] = ins[top].args[2 + i];
			//}
			for (nat i = 0; i < output_count; i++) {
				if (ctk[ins[top].args[2 + i]]) { puts("system call ct rt out"); abort(); }
				//list[list_count++] = write_access | ins[top].args[2 + i];
				ctk[ins[top].args[2 + i]] = false;
			}

			rt_ins[rt_count++] = ins[top];

			if (n == system_exit) {
				printf("warning: reached cfg termination point\n");
				print_instruction_index(ins, ins_count, names, top, "CFG termination point here");
				goto next_instruction;

			} else {
				printf("info: found %s system call!\n", systemcall_spelling[n]);
				print_instruction_index(ins, ins_count, names, top, systemcall_spelling[n]);
			}
		}

		nat true_side = (nat) -1;
		const nat false_side = compute_ins_gotos(&true_side, ins, ins_count, top);
		if (is_branch(op)) {
			if ( true_side < ins_count and visited[true_side] < 1)  stack[stack_count++] = true_side;
			if (false_side < ins_count and visited[false_side] < 1) stack[stack_count++] = false_side;
		} else {
			if (false_side < ins_count) stack[stack_count++] = false_side;
		}
		next_instruction:;
	}



	printf("performing unreachable analysis...\n");
	for (nat i = 0; i < rt_count; i++) {

		//const nat op = rt_ins[i].args[0], arg1 = rt_ins[i].args[1], arg2 = rt_ins[i].args[2];

		/*if (not visited[i]) {
			printf("warning: instruction is unreachable\n");
			print_instruction_index(ins, ins_count, names, i, "unreachable");
			puts("");
			ins[i].count = 0;
		}*/

		/*if (ctk[arg1] and op != sc) {
			ins[i].count = 0;
		}*/
	}


	puts("ins:");
	debug_instructions(ins, ins_count, names);

	puts("rt_ins:");
	debug_instructions(rt_ins, rt_count, names);








	puts("}");
	printf("found compiletime values of variables: {\n");
	for (nat i = 0; i < name_count; i++) {
		if (not ctk[i]) continue;
		printf("\tvalues[%s] = %llu\n", names[i], values[i]);
	}
	puts("}");
	printf("found list: (%llu count)\n", list_count);
	for (nat i = 0; i < list_count; i++) {
		nat variable_index = (~(1LLU << 63LLU)) & list[i];
		const char* type = (list[i] >> 63) ? "write" : "read";
		printf("%6llu : %6s of %6s\n", i, type, names[variable_index]);
	}
	puts("done");



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
		//getchar();
	}

	puts("compiled.");
	exit(0);
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
























