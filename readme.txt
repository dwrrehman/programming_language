a compiler for a programming language
written on 1202504115.035207 by dwrr
=======================================

this is an optimizing compiler for a programming language that i am making for fun and for my own use. the language is word-based, statement-based, and is closely modelled off of the hardware RISC-like ISA's which it chooses to target, which include RISC-V (32 and 64 bit), ARM (32 and 64 bit), and the MSP430 ISA. 

there are only 24 builtin operators in the language, which 0, 1, 2, or 3 arguments. all operators are prefix, and fixed arity. 

the builtin operations of the language are the following:

 	halt sc do at lf set add sub mul div rem 
	and or eor si sd rt la ld st lt ge ne eq

thats it! 

a description of the semantics of each instruction is given below:

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



some further notes:
------------------------

these instructions and patterns of them are used to construct all hardware instructions used in the target ISAs, via instruction selection and register allocation. 

an extended form of constant propagation/folding is used in the compiler after parsing to allow for fully turing-complete compile-time execution, and thus the generation of arbitrary data for use during runtime, allowing for further optimizations not possible in languages such as C. 

a graph coloring approach for register allocation is planned to be used. currently unimplemented, as instruction selection is currently in progress. 

the compiler does not use SSA form, or the notion of basic blocks, rather, the above builtin-operation listing/ISA is also the intermediate representation (IR) for the compiler, and all control flow and data flow analysis and optimizations are done globally on the entire program. 

there is no notion of functions, structs, classes (or any other typical high-level abstraction) in this language, as these are not neccessary, and hinder optimizations. 

comments are denoted with parenthesis, and are character based, not word based, and are only allowed between valid instructions. additionally, comments can nest within each other. eg, (something (like this) or that.)

this language also allows the user to define the name of a variable as anything they want. the names of the operations are valid variable names, and any ascii or unicode character can be used in names, except for whitespace (tab, newline, and spaces), which is used for delimiting words. parenthesis are also valid within names, as they only denote comments between valid instructions. 

there is currently no mechanism for allowing the user to define their own macro-operations, and this is not planned to be implemented currently. rather, a macro-like mechanism is emergently acheived via the existing operations such as "at", "do", "set", etc. 

if a given variable name is not defined, and not at a label argument position, or the destination of an instruction which is capable of defining a new variable, then the word is attempted to be interpretted as a little-endian binary literal. little endian, here, means that the least significant bit is first, and the most significant bit is at found at the end of the word. trailing zeros are ignored. 

little-endian binary literals are the only form of immediates/constants present in this language. the digit seperator '_' is allowed in binary literals as well, in addition to the digits '0' and '1'. 

respelling of constants in decimal form is possible, as all digits are valid within identifiers. if a variable is defined which only comprises of 0's and 1's, then this references to this identifier are preferred instead of treating it like the equivalent binary literal. binary literals not valid as the register destination for an instruction. 

if a number cannot be parsed as a binary literal, an "undefined variable" parsing error is displayed/returned. 

additionally, there is delimiter neccessary between instructions besides whitespace, and mulitple instructions can appear on the same line, as long as there is some whitespace between them. 



-----------------------------------------
	code examples:
-----------------------------------------

some examples of code in this language are given below, to illustrate how the language is used in practice!



------------------------------------------

(a simple test of the const prop alg 
written on 1202504093.232238 dwrr)

set x 101         (x has the value 5 after this line.)
set y 001	  (y has the value 4 after this line.)
add x y     	  (x now holds the value 9.)
halt    	  (note, the use of halt is optional, when at the end of the file.)

------------------------------------------

(this is a simple loop from 0 to 9, executed at compiletime)
(these parenthetical thingies are comments, by the way)

set count 0101
set i 0

at loop
	(your code here!)
	add i 1
	lt i count loop


------------------------------------------

(this is a simple loop from 0 to 9 at runtime)

set count 0101
rt i 001   (<---- only difference neccessary, to make this loop happen at runtime.)
set i 0

at loop
	add i 1
	lt i count loop

-------------------------------------------

(a prime number counting program 
 that executes at runtime! 
 written on 1202504104.153543 by dwrr)

rt i 0  rt j 0  rt count 0   (remove this line, 
		to make this code execute at compiletime instead!)

set n 00001
set i 0
set count 0

at loop
	set j 01
	at inner
		ge j i prime
		set r i rem r j eq r 0 composite
		add j 1 do inner
at prime
	add count 1

at composite
	add i 1
	lt i n loop

halt

-------------------------------------------









