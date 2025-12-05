a macro cross-assembler i wrote for fun and for my own use
---------------------------------------
written on 1202407217.003650 by dwrr
updated on 1202501046.114848
updated on 1202503053.151203

this is a cross-assembler for the arm64, arm32, rv64, rv32, and msp430 architectures, which i wrote for my own use in programming projects requiring maximum control over resources and performance. currently only msp430 and arm64 backends are finished so far. 

this cross-assembler has both a fully-hygenic macro system and a powerful turing-complete compiletime system which allow for complete control over how the machine instructions are translated into the final executable, and these features also allow for much improved readability of code. 

additionally, unlike many assemblers, this assembler is planned to have opt-in optional automatic register allocation, allowing for better debugability when code is not sensitive to the registers used.

the selection of the target architecture can be done programmatically at compiletime through the compiletime execution system, and arbitrary static data can also be generated into the executable to allow for better performance.

the primary goal of this language is to allow for maximum control and performance on the intended target architectures- portability of code across targets is not a design goal. additionally, the language was designed to be minimalist in syntax and semantics, in so much as it does not affect performance at all. because of this, many simplifying design decisions were made, such as treating registers, compiletime-known variables, labels, and immediates/constants all as the exact same entity- just a compile-time data-register value, interpreted in a particular way by an instruction.

despite the langauge's focus on low-level control, and minimalism, due to the macro and compiletime system source code in written for this assembler can still be quite readable, and easily writable. for example, it is in theory possible to write macros to be able to write and use while loops, for loops, and if statements via the generality of the macro system , which could translate down to efficient sequences of machine code instructions. this is unimplemented currently. 


some notes about the assembler's syntax and design:

	0. the language is purely word-based (similar to forth ish)

	1. each operation/directive is always of a fixed arity and in prefix form: the operation name comes before a predefined finite number of operands.

	2. all operands and op-codes are seperated by whitespace.

	3. there is no delimiter neccessary between instructions, due to the fixed arity of each instruction. thus multiple instructions per line are easily permitted.

	4. whitespace is completely ignored, except for the purposes of delimiting words. 

	5. any operation name is itself a valid operand identifier, as operations and operands live in different symbol tables.

	6. any symbols (including unicode) can be used within operation or operand names. 

	7. all operands are treated as either compiletime-known data variables or labels.

	8. all operations are treated as either macros, or macro-creation machinery.

	
ultimately, this language is just meant for my own use, however it might be the case that others could find it useful or interesting possibily. 

thanks for reading!

dwrr






















TRASH/OLD:
---------------------------------


this language is designed to be a simple and easy to learn common subset between multiple risc-like isa's, in such a way to allow for the maximum possible performance on all target isa's without requiring the user to write register numbers or write particular isa op codes. 

rather, the isa of this language is made to facilitate building those constructs (target specific op-codes, and register numbers) emergently via compiler-like instruction selection and register allocation passes, as well as allow for more powerful optimization on the input program. 

the language also has powerful compiletime evaluation semantics which allow for further optimizations.

some notes about the language's syntax and design:

	0. the language is purely word-based (similar to forth ish)

	1. each instruction is always of a fixed arity and in prefix form: the operation name comes before a predefined finite number of operands.

	2. all operands and op codes are seperated by whitespace.

	3. there is no delimiter neccessary between instructions, due to the fixed arity of each instruction. 

	4. whitespace is completely ignored, except for the purposes of delimiting words. 

	5. the operation names themselves are valid operand identifiers.

	6. any symbols (including unicode) can be used within operand names. 

	7. all operands are treated as either virtual registers or labels.

	8. there is no difference internally between a register and a label, except for the fact that labels are registers which are used as the destination/target of a branch or destination argument to label attribution.

	9. syntactically there are no constants or literals built in to this language, as all compiletime-known constants are defined computationally through instructions. 

	10. all built-in operations (which are listed below) are spelt in lowercase, however the language is case sensitive. nocase or occassionally snake_case is the preferred style for names.

this language chooses to use branches as opposed to for/while/if statements, as the latter can be constructed emergently at user lvel from just branches, and branches are the more general construct which is simpler and more general.

the language chooses to have variables represent hardware registers, as the programmer should be in charge of all memory use and allocation. the compiler will not write a load or store on the programmers behalf, by design, as these instructions are the most expensive by far in modern machines. 

the language chooses to expose the system call instruction directly, as all external physical effects of a program ultimately boil down to just system calls on modern operating systems (only linux, WSL and macos will be supported, windows is not intended to be supported ever) and thus to give the maximum control, performance, and flexibiltiy, a system call interface is given which is roughly portable across the various targets. unfortunately MacOS does not make the system call interface stable, but this will be manually delt with as problems arise. 

this language targets risc-v (32 bit and 64 bit) arm (32 bit and 64 bit).  additionally, possibly x86-64 in the future, however this is not a priority, as x86/x64 is considered legacy. the first two archs and their two subtypes are the main priorities in the language. an msp430 backend and interpreter will also be written for my own use cases.

the language's compiler stages are as follows: first parsing/lexing/CFG-formation is done in a single pass, then compiletime evaluation occurs, then instruction selection, then register allocation, and finally machine code generation. this compiler only generates MacOS Mach-O position-independent executables currently, and does not depend on the assembler or linker of the system. use of external libraries in user code is not supported currently, but is planned to be eventually.

the language chooses to use the two operand form of all arithmetic and logical operations, for example, "add d s " as opposed to "add d r s", including providing "set" as a seperate instruction, as "add d s" and "set d s" are simpler and more fundamental and primitive than the composite instruction found on most risc-like isa's, "add d r s". forming these composite instructions is done reliably and easily via the instructions selection stage.

the language's choice for which instructions should and should not be in the language was determined based on the following set of competing constraints: first, the instruction must be useful, ie occur very often in real code, and is useful in solving real problems. second, the instruction most correspond very easily/directly to an instruction (or maybe two instructions) in all targets. third, it should either be directly useful for creating runtime computation, or if its a compiletime directive only, it must be extremely helpful for assisting in runtime optimizations. fourth, the interface to the instruction must be as simple and minimalist as possible. 

note: finding the minimal turing complete isa is not required here. the property of mapping directly to the target isa's instructions, and being useful for actual code is more important than trying to minimize the isa to the simplset smallest possible set of instructions. 

there are only a couple exceptions to this reasoning: the "eoi" instruction is useful in practice for commenting out code, or providing notes, as there is currently no other mechanism for writing comments, and the implementation for "eoi" is a single line of code. 

the isa of the language is the following:

	set x y : assign the value y into the variable x, ie set x to have value y.
	add x y : set x to be x + y.
	sub x y : set x to be x - y.
	mul x y : set x to be x * y.
	div x y : set x to be x / y.
	and x y : set x to the bitwise AND of x and y.
	or x y  : set x to the bitwise OR of x and y.
	eor x y : set x to the bitwise XOR (exclusive OR) of x and y.
	si x y  : shift the value in x up by y bits.
	sd x y  : shift the value in x down by y bits. (unsigned shift.)

	ld x a t : load t bits from memory address a, and store these bits in register x.
	st a x t : load t bits from register x, and store these into t bits starting from memory address a.

	lt x y l : compare x and y, if x is less than y, branch to label l. if not, then ignore this instruction.
	ge x y l : compare x and y, if x is not less than y, branch to label l.	if not, then ignore this instruction.
	eq x y l : compare x and y, if x is equal to y, branch to label l. if not, then ignore this instruction.
	ne x y l : compare x and y, if x is not equal to y, branch to label l. if not, then ignore this instruction.
	do l     : branch unconditionally to label l.
	at l     : attribute the label l to represent the position of the next instruction after this instruction. 
	sc n a b c d e f : perform system call with language syscall number n, giving up to 6 arguments, a through f.	

	rt x b : require x to be known at runtime, and set the bit width of the values stored in x to be at most b bits.
	lf f   : load a file with filename f, and include its source in replace of this instruction. 
	eoi    : end of input, causes parsing to stop. all text past this point is ignored. this instruction is optional.


examples of code in the language:
-------------------------------------


	. a simple for loop: .

	lf foundation
	set i 0
	at loop
		print i
		add i 1
		lt i 5 loop


-------------------------------------

	. a simple if condition: .

	lf foundation
	set condition true
	eq condition 0 skip
		print hello
	at skip


-------------------------------------

	
ultimately, this language is just meant for my own use, however it might be the case that others could find it useful or interesting possibily. 

thanks for reading!

dwrr









