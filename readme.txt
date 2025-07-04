a (currently unnamed) compiler for a (currently unnamed) programming language
written on 1202504115.035207 by dwrr
new version of the language made on 1202505154.163659 by dwrr
=======================================

this is an optimizing compiler for a low-level programming language that i am making for fun and for my own use. the language is word-based, statement-based, and is closely modelled off of the hardware RISC-like ISA's which it chooses to target, which include: RISC-V (32 and 64 bit), ARM (32 and 64 bit), and the MSP430 ISA. 

there are only 30 built-in operators/instructions in the language (excluding the machine instructions), all of which take 0, 1, 2, or 3 arguments. also, all these operators are prefix, and fixed arity. 

a description of the built-in instructions and the semantics of each instruction is given below:


full language ISA:
----------------------
	
	set add sub mul div rem and or eor si sd str reg bits 
	ld st adr emit lt ge ne eq at do sc halt ct rt file del 
	


language-specific instructions:
----------------------------------
	rt		: mark instructions and variables from here onwards to be runtime.
	ct		: mark instructions and variables from here onwards to be compiletime.
	del x		: remove x from the symbol table. 
	file f		: load the contents of the file at filepath f,
			  and parse them fully before proceeding. 
			  any edits to the symbol table are persistent/stateful.
	reg x r		: make x stored in the runtime hardware register r. r is CTK.
	bits x b	: make x stored in at least b bits. b is CTK. 
			  note: ...currently this instruction are ignored, lol.
	str s		: emit string data from s to the final executable at this position.
			  is equivalent to a series of emit instructions.

arithmetic operations:
------------------
	set x y	: assignment to destination register x, using the value present in source y.
		  if the name x is not defined, then x is defined as a result of this instruction.
		  if x is defined, the existing definition of variable x is used.

	add x y	: assigns the value x + y to the destination register x.
	sub x y	: assigns the value x - y to the destination register x.
	mul x y	: assigns the value x * y to the destination register x.
	div x y	: assigns the value x / y to the destination register x.
	rem x y	: assigns the value x mod y to the destination register x.

bitwise operations:
------------------
	and x y	: assigns the value x bitwise-and y to the destination register x.
	or x y	: assigns the value x bitwise-or y to the destination register x.
	eor x y	: assigns the value x bitwise-eor y to the destination register x.
	si x y	: shift x up by y bits, and store the result into destination register x.
	sd x y	: shift x down by y bits, and store the result into destination register x.

memory-related operations:
------------------
	ld x y z  : load z bytes from memory address y into destination register x. 
	st x y z  : store z bytes from register y into destination memory at memory address x.
	emit x y  : emit x bytes from constant y to the executable at this position. 
		    x and y are both compiletime-known (CTK).
	adr x	  : puts the following instructions at hardware address x. x is CTK. 
		    this instruction is only valid to use on embedded targets. 

control flow:
------------------
	lt x y l	: if x is less than y, branch to label l. 
	ge x y l	: if x is not less than y, (ie, x is greater than or 
			  equal to y) branch to label l. 
	ne x y l	: if x is not equal to than y, branch to label l. 
	eq x y l	: if x is equal to y, branch to label l. 
	do l		: unconditionally branch to label l. 
			  when executed at compiletime, it also stores the 
			  pc of this instruction to address 0 in compiletime memory.
	at l		: attribute label l at this position. 
	halt		: termination of control flow here. 
	sc 		: system call instruction.






there is also the following machine instructions/encodings which are accessible for all targets, in addition to the above language. here are the machine instructions. they are abstracted from typical assembly, as only the core unique machine-code encodings are provided. it is expected that the compile-time evaulation system will be used to make using these encodings more friendly for programming in assembly in this language. this also should not be required in most circumstances, and providing only the encodings makes decode logic in the compiler simpler, and requires fewer instructions in the language as a whole. 

here are the 3 target hardware ISA's and their machine instructions:
-----------------------------------------------------------------------

ARM64:
	a6_nop 	: no operation
	a6_svc 	: system call (supervisor call)
	a6_mov 	: set register with 16 bit immediate source
	a6_bfm 	: bit field move instruction
	a6_adc 	: add/subtract with carry 
	a6_addx : add/subtract with optionally sign/zero-extended register source 
	a6_addi : add/subtract with immediate source
	a6_addr : add/subtract with optionally shifted register source
	a6_adr 	: load address of a PC-relative label into a register
	a6_shv 	: variable-shift left or right of a register
	a6_clz 	: count leading zeros of regsiter
	a6_rev 	: reverse bits of register
	a6_jmp 	: unconditional jump to label (with optional link)
	a6_bc 	: conditional branch to label based on flags 
	a6_br 	: branch to register value (with optional link)
	a6_cbz 	: compare and branch if register is nonzero or zero
	a6_tbz 	: test bit in register and branch to label if set
	a6_ccmp : conditional compare instruction
	a6_csel : conditional select / increment / invert / negate instruction
	a6_ori 	: bitwise or/and/xor with immediate
	a6_orr 	: bitwise or/and/xor with shifted register
	a6_extr : extract instruction (?)
	a6_ldrl : load register data with from label PC-relative address
	a6_memp : load/store memory to/from pair of registers
	a6_memia : load/store memory to/from register with post/pre increment addressing mode
	a6_memi : load/store memory to/from register with address plus unsigned immediate offset
	a6_memr : load/store memory to/from register with address plus register offset
	a6_madd : multiply-accumulate / multiply-subtract instruction
	a6_divr : divide register with register instruction

MSP430:
	m4_sect : create new section in executable, and set physical memory address of this section
	m4_op : arithmetic and bitwise operation instruction, with destination and source addressing modes 
	m4_br : branch on condition to label instruction


RISC-V
	r5_r : register-register integer operation instruction format
 	r5_i : register-immediate integer operation insrtuction format
	r5_s : store operation instruction format
	r5_b : compare and branch with register-register condition to label instruction format
	r5_u : large immediate (possibly PC-relative) load into register instruction format
	r5_j : unconditionally jump to label instruction format




the "why" behind the language:
----------------------------------

in my particular use cases, i have two use cases which i think really shaped the language, and its goals. the first is that i want to run code in my language on these MSP430 microcontrollers which have only 512 bytes of SRAM, meaning that code needs to be incredibly memory and time efficient. not a single instruction can be wasted, as this could cause the program to not function at all, potentially.

My second use case is different: i am making an extremely high performance system running on an ARM64 machine, and in this setting, performance and efficiency are absolutely paramount. having the low level control to be able to guarantee performance and efficiency at the language level is highly advantageous in this scenario, as it very much does make the difference between finding the solution in a week, or never finding the solution for years.

so these are the use cases i had in mind. thus the programming experience i am aiming to create is based around low-level control, and efficiency. additionally, it helps to minimize the complexity of the language as much as possible, to make the compiler itself as simple as possible, which allows better reasoning about the translation process itself. further making it easier to get a working solution in resource constrained environments.

the trade-offs which i am consciously making revolve around user-friendliness, vs expert-friendliness / fine control. the language strives to give fine control over things when its advantageous to performance, and thus, the language loses much in user-friendliness, and ease of use, most of the time. additionally, terseness is lost as well in some ways, as even small tasks take many instructions to complete. luckily, however, the standard library might try to help this problem slightly, by providing solutions to common problems encountered in programming. 

portability is also not a true goal of this language: for example, spill code will never be generated, as if register allocation fails to fit all variables into registers, a compiler error is generated, and the programmer must fix this by manually allocating stack memory for some memory variables, or some how compressing their use of registers until things fit into the register file, and RA can succeed. ergonomics are seen as something only given when it does not come at the expense of performance.

the paradigms which are promoted in this language include only: imperative programming, and procedural programming. all other programming paradigms, including functional programming, and object oriented programming, are seen as completely antithetical to the goal and use cases of the language, and thus are highly, highly discouraged.

the intended feel of this language is to feel like you are as close to the metal as you can be, while still programming in a way where you are able to specify intent better, (ie, not always using the machine instructions directly!) and where you are able to dynamically change how low level you are, based on what you want to do. in some places, you choose to use machine instructions, in other places, you choose to use the more abstract language, which expresses intent better.

the long term aspirations of this language are to replace my using of C for heavily resource constrained, performance-critical applications, specifically when the hardware target is either MSP430, RISC-V, or ARM. for these applications, this language hopes to do a better job at attaining peak performance than C code. :)





some further notes about the language:
-------------------------------------------

a goal of this language is generally speaking to use the language ISA instructions (and various patterns of them) to construct all useful hardware instructions/registers present in the target machine ISAs, via instruction selection and register allocation. in cases where this is not feasible, direct access to both hardware registers, and machine instructions is provided. 

needless to say, there is no notion of structs, generics, classes in this language, (or any other typical high-level abstraction found in most languages), as these are not neccessary for programming, and generally hinder optimizations, and also in a lot of cases, hinder programming as well.

a compiletime execution system is used in the compiler after parsing to allow for fully turing-complete compile-time execution, and thus the generation of arbitrary data and runtime instructions for use during the runtime program, allowing for further optimizations not possible in languages such as C. after this step is performed, a constant propagation/folding optimization stage (akin to SCCP in SSA compilers) is also performed as well, on the generated runtime program which resulted from the first compiletime execution stage. 

the first CT execution stage is quite powerful, allowing for a derived feature of compiletime function calls, aka "macros" (implemented as effectively assembly=like function invocations) to be constructed at user level from this compiletime execution system. 

currently, there is no built-in mechanism for allowing the user to define their own functions, or macro operations, and this is not planned to be implemented, at least for now. rather, the user can use the macro-like mechanism that is emergently acheived via the existing builtin language operations such as "at", "do", "set", etc., where these are executed at compiletime.

a graph coloring approach for register allocation is currently used. as stated, spill code, and automatic stack memory management will not take place, ever, by design. if register allocation (RA) fails to allocate all the program's variables into the hardware registers, an error is generated, and the programmer must fix this error by manually managing stack memory or storing variables in memory somehow, or somehow compressing the data variables into registers better. 

instruction scheduling is currently not implemented, however, when it is implemented, minimizing register pressure will always be a paramount goal of the scheduler, unless there are available registers for use.

comments are denoted with parenthesis, and are character based, not word based, and are only allowed between valid instructions. additionally, comments can nest within each other. eg, (something (like this) or that.)

this language also allows the user to define the name of a variable as anything they want. the names of the operations are valid variable names, and any ascii or unicode character can be used in names, except for whitespace (tab, newline, and spaces), which is used for delimiting words. parenthesis are also valid within names, as they only denote comments between valid instructions. 

if a given variable name is not defined, and not at a label argument position, or the destination of an instruction which is capable of defining a new variable, then the word is attempted to be interpretted as a little-endian binary literal. little endian, here, means that the least significant bit is first, and the most significant bit is at found at the end of the word. trailing zeros are ignored. 

little-endian binary literals are the only form of immediates/constants present in this language. the digit seperator '_' is allowed in binary literals as well, in addition to the digits '0' and '1'. little endian binary was chosen, as it is more fundamental than big endian binary compuationally speaking, as there is no "bit-reversing" that needs to be done in the mathematics/code describing the representation. binary is used instead of decimal, as math and programming is much easier done in binary, once you get used to it, and comfortable with it. binary arithmetic exposes several patterns in numbers which are inaccessible to programmers using decimal or hexadecimal only. also, friendly reminder that our computers literally run on binary, in virtually all respects. 

respelling of constants in decimal form is possible, as all digits are valid within identifiers. if a variable is defined which only comprises of 0's and 1's, then this references to this identifier are preferred instead of treating it like the equivalent binary literal. binary literals not valid as the register destination for an instruction. 

if a number cannot be parsed as a binary literal, an "undefined variable" parsing error is displayed/returned. 

additionally, there is no delimiter neccessary between instructions besides whitespace. furthermore, it is completely valid for mulitple instructions to appear on the same line, as long as there is some whitespace between them. this is often used to group instructions to make written code more compact or readable, as each instruction is usually quite short. 

as for data types and type systems: this language actually does not have a notion of any data types- except for floats or integers of various bit widths. no type checking is performed, and it is assumed the programmer needs to correct these bugs manually. however, the compiler will do a runtime value bit-width analysis on the code as part of its translation process and thus type errors might be able to surface here to help the programmer catch them. the "bits" instruction can be thought of as providing a "type" to an existing variable, but it is not said what information could be stored in that number of bits (besides possibly whether it is a float or integer data).

the system call instruction will use the fact that the standard library defines numerous useful constants containing the appropriate system call numbers and register indexes used by the platforms system calls, thus we don't need to make an abstract system call interface because the user will just include a library file which defines correct values to be able to use the "system" instruction.



notes about the compiler's development process:
-----------------------------------------------------

the source code for the compiler is currently a tad under 3,000 lines of code so far, and is expected to grow slightly as more optimization passes are implemented, and instruction selection and register allocation is flushed out further. this source code is located in a single C file, "c.c". most of the code, including parsing, lexing, optimization passes, instruction selection, register allocation, and machine code generation and output generation is all done in the main() function, with minimal use of functions. 

using multiple files besides "c.c", or using widespread use of functions across the code base for readability is not planned or desired, as it significantly increases the friction at which changes can be made to the compiler, and significantly decreases the oppurtunities for new algorithms and common patterns to be discovered in the source code while implementing things. 

also, if the C source code is "unreadable" to you, this is, technically speaking, a "skill issue" on your part. i reccomend improving your programming skills until it is readable for you. 


a notable difference is also that the compiler does not use SSA form, or the notion of basic blocks. rather, all data flow analysis and optimization passes take into account the full global data/control flow, and all control flow and data flow analysis and optimizations are done in a completely stateful (ie, tracing knowledge forwards or backwards through the control-flow-graph (CFG) and data-flow-graph (DFG), statefully) and global (taking into account the entire program's CFG and DFG) manner on the entire program. 

finally, the language itself can be thought of as the same language as the intermediate representation (IR) for the compiler, as the CFG and DFG representation internally is represented in the exact same manner as the users code: using "at" instructions, conditional branches ("lt", "eq", etc) using label names, and operations like "set", "add", etc. this homogeneity between the internal and external (user-facing) representation allows the programmer to have a deeper understanding of how the compiler is interpretting the code, and potentially control the final executable to an even higher degree, for maximum performance. 

this homogeneity also serves to keep the implementation of the compiler itself, simple and straight forward, as the control flow and data flow is never stored or created in data explicitly. rather, it is computationally derived from scratch when needed, allowing for the program in this internal representation to be changed easily and simply, during the process of optimization. 


...see the code examples section for what code in this language generally looks like!...




final remarks:
------------------------

additional reminder that the language is made specifically and primarily for my own use cases, and thus is not tailored for anyone else. additionally, because the language is still largely in development and quite drastic changes could be made quite often, is it not currently meant to be used by anyone other than me at all. as such, pull requests or feature suggestions will most likely be rejected, (unless it really knocks my socks off, then i might consider it, but it would have to be something huge that i missed, lol..)

however, that being said, i do plan to make the project publically usable by others for their own purposes, once the compiler and standard library are polished and stable enough to where i am happy with them. 

regardless, i hope you find the project interesting, or possibly learn something or find inspiration from it. thanks for reading!

dwrr










-----------------------------------------
	code examples:
-----------------------------------------


some examples of code in this language are given below, to illustrate how the language is used in practice!

note that these are subject to change! some examples may be out of date, as the language goes through several revisions over time. 



------------------[EXAMPLE 1]------------------------

(a simple test of the const prop alg 
written on 1202504093.232238 dwrr)

set x 101         (x has the value 5 after this line.)
set y 001	  (y has the value 4 after this line.)

register z 0011   (variable z is forced to be stored in the hardware register "x12" assuming this exists for the target.)

add x y     	  (x now holds the value 9.)

set z x           (z now contains the value 9, and this store to this 
		   register cannot be elided/eliminated by the compiler.)

halt    	  (note, the use of halt is optional/implied, when at the end of the file.)






--------------------[EXAMPLE 2]----------------------

(this is a simple loop from 0 to 9, executed at runtime)

set count 0101
set i 0
at loop
	(your code here!)
	add i 1
	lt i count loop












-------------------- [EXAMPLE 3] --------------------


(computing the number of primes less than a given number ("limit")
 at runtime, using the c backend! also testing out the macro system further.
 things are still a bit rough right now, but it works kinda lol. 
written on 1202507023.214455 by dwrr)

file library/foundation.s

(...this would all be in the standard library...)
rt 
	set a0 a0  
	set a1 a1	
ct 
	set c0 c0 
	set c1 c1 
do skip

at c_backend
	ld ra compiler_return_address nat
	st compiler_target c_arch nat
	st compiler_format c_source nat
	st compiler_should_overwrite true nat
	do ra del ra

at exit
	ld ra compiler_return_address nat
	rt set c_system_number c_system_exit
	set c_system_arg0 a0
	sc halt ct 
	do ra del ra

at skip del skip
set newline 0101
(...until here...)




(my code starts here!)

do c_backend 
set limit 0101
rt set i 0 set count 0
at loop
	set j 01
at inner 
	ge j i prime
	set r i rem r j 
	eq r 0 composite
	add j 1 do inner
at prime
	add count 1
at composite
	add i 1 lt i limit loop

set a0 count 
do exit 


































































TRASH:
-------------------------------------------------------




	la x l	  : load the PC-relative address of label l into destination register x. 





old code examples:



--------------------[EXAMPLE 3]----------------------


(this is a simple loop from 0 to 9 at compiletime)

constant count    (<---- only difference is these three lines, to make this loop happen at compiletime.)
constant i 
constant loop

set count 0101      (the value 10 in little endian binary)
set i 0
at loop
	(your code here!)
	add i 1
	lt i count loop


-------------------[EXAMPLE 4]------------------------

(a prime number counting program 

	 that executes   at compile-time!!!

 written on 1202504104.153543 by dwrr)

constant prime 
constant composite 
constant n set n 00001    (the value 16 in little endian binary)
constant i set i 0
constant count set count 0

constant loop at loop
	constant j set j 01
	constant inner at inner
		ge j i prime
		constant r set r i rem r j eq r 0 composite
		add j 1 do inner
at prime
	add count 1

at composite
	add i 1
	lt i n loop

halt

----------------------[EXAMPLE 5]---------------------------


(testing out macros/functions in the language (aka compiletime macros/function calls lol) 1202505106.141237)

			(also yes, you can nest comments!)


runtime sum 0 set sum 0

constant lr  set lr 0

constant a0  set a0 0

constant skip
do skip

constant mymacro 
at mymacro                (this is effectively a compiletime function body!)
	a6_nop
	add sum a0
	system
	
	add lr 1 do lr         (this is effectively a compiletime function return!  ...note we are incrementing a compiletime label.)

at skip


set a0 101 at lr do mymacro       (these are effectively compiletime function calls!   note we are reattributing a compiletime label.)

set a0 11 at lr do mymacro        (here, we pass in 3 for argument 0.)

set a0 01 at lr do mymacro        (and here, we pass in 2 for argument 0 instead)

set a0 0 at lr do mymacro           (etc)

halt




---------------------[EXAMPLE 6]-----------------------


(
	core standard library for the language:     "library/foundation.s"
	written on 1202505165.132635 by dwrr.
)

(rv32 system call abi)

register rv_sc_arg0 0101
register rv_sc_arg1 1101
register rv_sc_arg2 0011
register rv_sc_number 10001

(specific to the rv32 virtual machine running in my website: )

constant rv_system_exit   set rv_system_exit   1
constant rv_system_read   set rv_system_read   01
constant rv_system_write  set rv_system_write  11 

(compiler interface)

constant ctsc_abort 		set ctsc_abort 0
constant ctsc_exit 		set ctsc_exit 1
constant ctsc_getchar 		set ctsc_getchar 01
constant ctsc_putchar 		set ctsc_putchar 11
constant ctsc_printhex 		set ctsc_printhex 001
constant ctsc_printdec 		set ctsc_printdec 101
constant ctsc_set_debug 	set ctsc_set_debug 011
constant ctsc_print 		set ctsc_print 111
constant ctsc_set_target 	set ctsc_set_target 0001
constant ctsc_set_format 	set ctsc_set_format 1001
constant ctsc_overwrite 	set ctsc_overwrite 0101
constant ctsc_get_length 	set ctsc_get_length 1101

constant no_arch 	set no_arch 0
constant arm64_arch 	set arm64_arch 1
constant arm32_arch 	set arm32_arch 01
constant rv64_arch 	set rv64_arch 11
constant rv32_arch 	set rv32_arch 001
constant msp430_arch 	set msp430_arch 101

constant debug_output_only 	set debug_output_only 0
constant macho_executable 	set macho_executable 1
constant macho_object 		set macho_object 01
constant elf_executable 	set elf_executable 11
constant elf_object 		set elf_object 001
constant ti_txt_executable 	set ti_txt_executable 101
constant hex_array_txt_executable  set hex_array_txt_executable 011

constant true set true 1
constant false set false 0

(end of standard library code) 


----------------------[EXAMPLE 7]---------------------------


(the first hello world program for the language, 
running in the risc-v virtual machine!
written on 1202505165.132734 by dwrr )

file library/foundation.s        (includes the standard library file for the language. still a work in progress)


compiler ctsc_set_target rv32_arch                     (these lines set the target architecture, and output format)
compiler ctsc_set_format hex_array_txt_executable         (and whether the output file is allowed to be overwritten, if it exists.)
compiler ctsc_overwrite true

set rv_sc_arg0 1
la rv_sc_arg1 string
constant l set l 0
compiler ctsc_get_length l 
set rv_sc_arg2 l 
del l         				(here,  we remove l from the current scope, to keep it from being used elsewhere)
set rv_sc_number rv_system_write
system

set rv_sc_arg0 0011
set rv_sc_number rv_system_exit
system halt

at string
string "hello, world!
"                         ( <----- note, you can have newlines in strings! they are multiline, 
					and there are no escaped characters. additionally,
					you can use any character to delimit strings, 
					use of the character double quote specifically was not required.)




-----------------------[EXAMPLE 8]--------------------------

(a simple test of the ctsc system:  
	a compiletime hello world program 
	written on 1202505176.034945 by dwrr
)


constant ctsc_print    (defined as in foundation.s)
set ctsc_print 111       (the value 7)


compiler ctsc_print 0     (calling the compiletime system call interface for printing a string at compiletime)


halt                  (this instruction is not technically speaking required lol)


string "hello world
"


-------------------------------------------------




OLD instruction set description:




	set x y : assignment to destination register x, using the value present in source y.
		if the name x is not defined, then x is defined as a result of this instruction.
		if x is defined, the existing definition of variable x is used.
	add x y : assigns the value x + y to the destination register x.
	sub x y : assigns the value x - y to the destination register x.
	mul x y : assigns the value x * y to the destination register x.
	div x y : assigns the value x / y to the destination register x.
	rem x y : assigns the value x modulo y to the destination register x.
	and x y : assigns the value x bitwise-and y to the destination register x.
	or x y : assigns the value x bitwise-or y to the destination register x.
	eor x y : assigns the value x bitwise-xor y to the destination register x.
	si x y : shifts the bits in x up by y bits. 
	sd x y : shifts the bits in x down by y bits. (always an unsigned shift)
	la x k : loads a PC-relative address given by a label k into a destination register x.
	ld x y z : load z bytes from memory address y into destination register x. 
	st x y z : store z bytes from the soruce register y into the memory at address x.
	rt x y : force the variable x to be runtime known. 
		if y > 0, this sets the number of bits allocated to x, and 
		if y == 0, x is forced to be runtime known, with no further constraints, and
		if y < 0, this denotes the hardware register x should be allocated in. 
	halt : termination point in the control flow graph.
	do k : unconditional branch to label k. 
	at k : attribute label k to this position in the code. 
		k should be used as the destination of branches, 
		or as the source of an la instruction.
	lt x y k : if x is less than y, control flow branches to label k. 
	ge x y k : if x is not less than y, control flow branches to label k. 
	ne x y k : if x is not equal to y, control flow branches to label k. 
	eq x y k : if x is equal to y, control flow branches to label k. 
	sc : system call, target specific, and triggers a context switch on 
		targets with an operating system, to perform a specialized task.
	lf f : load file f from the filesystem, and include its parsed contents here.
		f denotes a relative path from the directory the compiler invocation is executed. 
		...due to the strict word-based nature of the language, f cannot contain spaces.








ct system call interface:
-------------------------------
consider:

	compiler a b 		: compiletime system call interface to the compiler


for this "compiler" instruction, the inputs "a" and "b" to it are quite nuanced, and control various important settings in the compiler, including which target is being targeted. 

the parameters a and b do various useful functions:

	if a == 0     ctsc_set_debug    debug = b
	if a == 1     ctsc_exit     exit(b)
	if a == 2     ctsc_putchar   putchar(b)
	if a == 3     ctsc_getchar  b = getchar()
	if a == 4     ctsc_abort    abort()
	if a == 5     ctsc_length    string length calculation for string b, length stored in b
	if a == 6     ctsc_print      print string with string-index b
	if a == 7     ctsc_printd     print the value currently in compiletime variable b, in decimal
	if a == 8     ctsc_printh     print the value currently in compiletime variable b, in hex 
	if a == 9     ctsc_target   target = b
	if a == 10    ctsc_get_target   b = target
	if a == 11    ctsc_output_format   output_format = b
	if a == 12    ctsc_output_name   output_name = string with index b
	if a == 13    ctsc_overwrite   should_overwrite = b


examples of the compiler compile-time interface in action are given in the examples section. 





