// 1202407302.211405  dwrr
// the parser for the programming language,
// the new version with function defs and riscv isa ish...
//




//   todo       to implement still:      obs,    st, ld,     reg, rdo, ctk,   


/*
copyb insert ./build
copya insert ./run
copyb do ,./build
copya do ,./run

*/






	



/*

202408187.002119:
things i have realized recently:
====================================


	- command line arguments passed in registers?
	
	- generating a mach-o executable directly.

	- adding system calls to our language, by adding them as instructions!

	- 












	we alsoooo need to not allocate a stack when we start up,

		anddd we need to like, somehow have some built variables for

			argv and argc, (which we are going to rename!)
				these are usually passed in registers, upon the executable starting up. so yeah. important to preserve these, i think.

		

	this way, we can get arguments from the command-line. very important, actually. well, kinda. not really. but sortt of important lol. 
		sooo yeah. 

				becuase like technically, you donnt needdd cli args, you wan just get stnadard in from the user, or something like taht, ie, using pipes, and standard in/out to do arguments. easy enough. i wish unix did this always actually. but yeah. 











						we are going to generate an executable directly, 

						both for   macho, and elf.  becuase we can lol. 

									shouldnt be thattt bad honestly. 


							the only main problem is that now using an external library will be... 
								difficult lol. but we'll solve taht problem later lol. i think its definitely fixable, i think. honestly it shouldnt even be that bad, i think!   like,  yeah. idk. technically speaking it shouldnt be thattt bad. 

							we can always call out to the linker for doing thatttt linking step lol. so yeah. 

								yay nice. 



exit
execve
fork
wait
open
close
write
read
ioctl
poll
lseek



-------------------- LINUX  -------------------- 

	exit execve fork wait/wait4
	open openat close write read ioctl poll lseek
	mmmap mprotect mremap munmap msync

	readv writev select access
	pipe dup dup2
	nanosleep rt_sigaction kill pause alarm getpid
	
	socket connect accept/accept4
	sendto recvfrom sendmsg recvmsg shutdown
	bind listen getsockname getpeername
	socketpair setsockopt getsockopt
	
	fcntl flock fsync sync fdatasync
	mlock msync munlock
	fdatasync truncate ftruncate
	getdents getcwd chdir fchdir chroot
	rename mkdir mkdirat mknod mknodat rmdir
	link linkat unlink unlinkat symlink readlink
	chmod fchmod chown fchown fchownat

	gettimeofday getrlimit setrlimit getrusage
	getuid setuid getgid setgid utime umask
	getpriority setpriority
	mount unmount
	



-------------------- MACOS -------------------- 


exit fork read write open close wait4 lseek ioctl poll
munmap mprotect mmap 

link unlink chdir fchdir
rename flock  mkdir rmdir utimes 
mknod chmod chown getpid setuid getuid 

recvmsg sendmsg recvfrom accept getpeername getsockname 
sendto socket connect bind setsockopt listen getsockopt 
shutdown socketpair

access sync kill dup dup2 pipe sigaction
getgid symlink readlink execve umask chroot 

fcntl select fsync setpriority getpriority
gettimeofday getrusage  writev readv  fchown fchmod 

setgid truncate ftruncate getdents(getdirentries) 
getrlimit setrlimit 
fstat lstat stat fdatasync
mlock msync munlock
mount unmount











NOTE


	there is a difference between macros and functions:  i just figured it out now:
		   ---------------------------------------


			it has to do with labels:


					if you want to be able to jump from one label defined in a top level scope, for example, 


						but you want to jump to it, FROM a goto inside of a "function" ("macro", really)

								then we need to actually do the instruction subsitution for the call, 


									BEFOREEEE we execute it.  i think this is the only real way to do this. 



				NOTE:   allowing for these types of jumps is quite useful, becuse then it literally means that  you can redefine 


				like,    a branch if equal,    or a less than branch              TO BE called ANYTHING YOU WANT


							ie, these are true macros, now, 



								 that just HAPPENNNN to be veryyyy hygenic becuase of the way we made them lol. 

											so yeah. 





				i think this is the right way to go, i thinkkkkkk





copyb insert ./build
copya insert ./run
copyb do ,./build
copya do ,./run

rename todo:
add these names to the isa:

		incr
		decr
		zero

those are pretty important lol... so yeah. i think i want those names too.





language isa:
===================

			note:   d  ==   r   ==  s  ==  l    all just simply variables.  l must be compiletime known though.
					but o and f are different, and different from those above. 



x	set d r   		<--- d define on use 
x	add d r
x	sub d r

x	mul d r
	muh d r
	mhs d r
x	div d r
	dvs d r
x	rem d r
	rms d r

x	and d r
x	or  d r
x	eor d r
x	sd  d r
x	sds d r
x	si  d r

x	ld  d r l
x	st  d r l

x	lt  r s l   		<--- l define on use
x	ge  r s l   		<--- l define on use
x	lts r s l   		<--- l define on use
x	ges r s l   		<--- l define on use
x	eq  r s l   		<--- l define on use
x	ne  r s l   		<--- l define on use
	
x	incr d
x	zero d   		<--- d define on use
x	decr d
x	not d

x	lf  f   		<--- f is a file, not part of the symbol table.
x	at  l   		<--- l define on use

x	def o  			<--- o must be new
x	ar r   			<--- r must be new
x	ret
x	obs

	sta  d l
	bca  d l

	sc  n  r r r  r r r       <----- n should usuallyyyyyyy be statically known. doesnt have to be though, technically speaking. 




















dm macro 
ar x 
ar y
	tttt....
macro












builtins:
---------------

undefined
builtinstacksize
builtinstackpointer







------------------------w.i.p.------------------------- 

mem r l  <--- r define on use       <----- this wohle system can be simplified:   if l is 0, we know its a register...?.. hm. 
reg r    <--- r define on use

ctk r    <--- r define on use
rtk r    <--- r define on use

rdo r        						<-- i feel like this system can be generalized....
------------------------------------------------- 




202408106.170451:


	OMG!!! after thinking about things a tonn, i think i figured out the fundemental requirements that we need in the programming language to be able to get all possible storage qualifiers and access qualifiers we could possibly want in the language, and have everything work together and be very composable and orthogonal and so on.  its just these simple three instructions/directives:



sto r l                  l == 0  means   r is statically known at compiletime.
			 l == 1  means   r must be in a register.
			 l == 2  means   r must be in memory on the stack.
			 l == 3  means   r can be in memory or a register, or deduced to be compiletime known. (default)

acc r l                  l == 0  means   non-accessible. (neither write or read accessible).
			 l == 1  means   r is read-only.
			 l == 2  means   r is write-only.
			 l == 3  means   r is read accessible and write accessible. (default)

bit r l			 
			 l is the number of bits which r's value takes up at most.  
					(64 or 32 is the default, depending on the target.)




202408106.172516:
actually  got it down to only    a  single instruction... here it is:





				sa r l b                b == 0 means r is inaccessible, 
							b > 0 means b is the bit-count accessible for writing/reading, 
							b < 0 means b is the bit-count accessible for reading
								(b == 64 or 32 is the default, depending on the target)

							l == 0 means compiletime known only.
							l == 1 means r is stored in a register, at runtime.
							l == 2 means r is stored in memory on the stack, at runtime.
							l == 3 means automatic detection/storage selection. (default)

								and these all modify  the variable    r




				sa r 3 64       <----- this use of "sa" is implied to happen on define of r, on 64 bit archs.

			 








	orrr rather, we could do:



			bit r b         b == 0 means inaccessible, 
					b > 0 means writing bits and reading bits   bitcount
					b < 0 means reading bits only   bitcount

					

			sto r l    l == 0 means compiletime only   
				   l == 1 means rt-register storage only
				   l == 2 means rt-memory storage only
				   l == 3 means runtime only (ie, reg or mem)



	perfect. 

	i love it. 

wow 

	so good 				literally a perfect way to solve that aspect of the language, honestly. YAYY













example code, usage of the "obs" instruction


		def local_scope

			def actual_logic ar x

				...something with x...

				ret


			def public_interface obs ar x
				actual_logic x
				ret

		ret

		set my_x 4
		public_interface my_x








	def findintersection  ar set

		....
		ret





	set x 5
	x x 








// todo:  implmeent  printing out a number  using little endian binary! 
		instead of using env.  make env print out a character instead, by calling write(). 
		lets implement printf ourselves basically, and interpret it at complietime. yay. 






copya do ,./run,test1.s








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
#include <sys/mman.h>


typedef uint64_t nat;

enum language_isa {
	nullins, zero, incr, decr, 
	set, add, sub, mul, div_, rem, 
	si, sd, sds, and_, or_, eor, not_, 
	ld, st, sta, bca,
	lt, ge, lts, ges, ne, eq, 
	def, ret, ar, obs, at, lf, sc,
	isa_count
};

static const char* ins_spelling[isa_count] = {
	"()", "zero", "incr", "decr", 
	"set", "add", "sub", "mul", "div", "rem", 
	"si", "sd", "sds", "and", "or", "eor", "not", 
	"ld", "st", "sta", "bca", 
	"lt", "ge", "lts", "ges", "ne", "eq", 
	"def", "ret", "ar", "obs", "at", "lf", "sc", 
};

enum system_call_table {
	system_call_undefined, 
	system_exit, system_execve, system_fork, system_wait,
	system_openat, system_close, system_write, system_read,
	system_ioctl, system_poll, system_lseek, 
	system_munmap, system_mprotect, system_mmap, 
	system_call_count
};

static const char* system_call_spelling[system_call_count] = {
	"system_call_undefined", 
	"system_exit", "system_execve", "system_fork", "system_wait",
	"system_openat", "system_close", "system_write", "system_read",
	"system_ioctl", "system_poll", "system_lseek", 
	"system_munmap", "system_mprotect", "system_mmap", 
};

enum language_builtins {
	nullvar,
	stackpointer, stacksize,
	builtin_count
};

static const char* builtin_spelling[builtin_count] = {
	"(nv)",
	"process_stackpointer", "process_stacksize",
};

enum arm64_isa {
	arm64_mov,  arm64_addi,
	arm64_memiu, arm64_add,
	arm64_adc, arm64_csel,
	arm64_slli, arm64_srli,
	arm64_adr, arm64_blr,
	arm64_bl, arm64_bc,
	arm64_madd,
	arm64_isa_count,
};

static const char* arm64_spelling[arm64_isa_count] = {
	"arm64_mov", "arm64_addi",
	"arm64_memiu", "arm64_add",
	"arm64_adc", "arm64_csel",
	"arm64_slli", "arm64_srli",
	"arm64_adr", "arm64_blr",
	"arm64_bl", "arm64_bc",
	"arm64_madd",
};


struct instruction {
	nat* args;
	nat count;
};

struct function {
	nat arity;
	nat* arguments;
	nat* define_on_use;
	struct instruction* body;
	nat body_count;
};

struct dictionary {
	char** names;
	nat* values;
	nat count;
};

struct scope {
	nat** list;
	nat* count;
	nat function;
};

struct file {
	nat index;
	nat text_length;
	char* text;
	const char* filename;
};


struct machine_instruction {
	nat args[16];
	nat arg_count;
	nat instructions[16];
	nat ins_count;
	nat op;
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

/*

static nat stack_size = 0x1000000;

static nat architecture = arm64;
static nat output_format = macho_executable;

static bool preserve_existing_object = false;
static bool preserve_existing_executable = false;

static const char* object_filename = "object0.o";
static const char* executable_filename = "executable0.out";

*/

static nat arity(nat i) {
	if (not i) return 0;
	if (i == ret or i == obs) return 0; 
	if (i == incr or i == decr or i == zero or i == not_ or i == def or i == ar or i == lf or i == at) return 1;
	if (i == lt or i == ge or i == lts or i == ges or i == ne or i == eq or i == ld or i == st) return 3;
	if (i == sc) return 7;
	return 2;
}

static void print_nodes(struct node* nodes, nat node_count, char** names) {
	printf("printing %3llu nodes...\n", node_count);
	for (nat n = 0; n < node_count; n++) {

		printf("[%s] node #%-5llu: {"
			".opcode=%2llu (\"\033[35;1m%-10s\033[0m\") "
			".outreg=%2llu (\"\033[36;1m%-10s\033[0m\") "
			".oc=%2llu "
			".ic=%2llu "
			".io={ ", 
			nodes[n].statically_known ? "\033[32;1mSK\033[0m" : "  ", n, 
			nodes[n].op, ins_spelling[nodes[n].op],
			nodes[n].output_reg, names[nodes[n].output_reg],
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

static void print_machine_instructions(struct machine_instruction* mis, const nat mi_count) {
	printf("printing %llu machine instructions...\n", mi_count);
	for (nat i = 0; i < mi_count; i++) {
		printf("machine instruction {.op = %3llu (\"%s\"), .args = (%3llu)[%3llu, %3llu, %3llu, %3llu] }\n", 
			mis[i].op, arm64_spelling[mis[i].op],
			mis[i].arg_count, 
			mis[i].args[0],mis[i].args[1],mis[i].args[2],mis[i].args[3]
		); 
	}
	puts("[done]");
}

static void print_basic_blocks(struct basic_block* blocks, nat block_count, 
			struct node* nodes, char** names
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
			printf("\tnode %3llu:   \033[32;1m%7s\033[0m  %3llu(\"%10s\") %3llu %3llu\n\n", 
				blocks[b].dag[d], ins_spelling[nodes[blocks[b].dag[d]].op], 
				nodes[blocks[b].dag[d]].output_reg, 
				names[nodes[blocks[b].dag[d]].output_reg], 
				(nat) -1,//nodes[blocks[b].dag[d]].input0, 
				(nat) -1//nodes[blocks[b].dag[d]].input1 
			);
		}
		puts("}");
	}
	puts("[end of node cfg]");
}

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


int main(int argc, const char** argv) {
	if (argc != 2) exit(puts("compiler: \033[31;1merror:\033[0m usage: ./run <file.s>"));

	puts("this assembler is currently a work in progress, "
		"backend is currently not fully implemented yet..."
	);

	const char* given_filename = argv[1];
	struct function* functions = NULL;
	nat function_count = 0;
	nat scope_count = 1;
	struct scope* scopes = calloc(1, sizeof(struct scope));
	scopes[0].list = calloc(2, sizeof(nat*));
	scopes[0].count = calloc(2, sizeof(nat));
	scopes[0].function = 0;
	struct dictionary dictionary = {0};
	nat stack_count = 1;
	struct file stack[4096] = {0};
{
	int file = open(given_filename, O_RDONLY);
	if (file < 0) { puts(given_filename); perror("open"); exit(1); }
	const nat text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(text_length + 1, 1);
	read(file, text, text_length);
	close(file);
	stack[0].filename = given_filename;
	stack[0].text = text;
	stack[0].text_length = text_length;
	stack[0].index = 0;
}
	for (nat i = 0; i < isa_count; i++) {

		const nat a = arity(i);
		functions = realloc(functions, sizeof(struct function) * (function_count + 1));
		functions[function_count++] = (struct function) {
			.arity = a,
			.arguments = calloc(a, sizeof(nat)),
			.define_on_use = calloc(a, sizeof(nat)),
			.body = NULL,
			.body_count = 0,
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

		struct instruction new = {0};
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
}




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



