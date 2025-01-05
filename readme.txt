a compiler for a programming language
---------------------------------------
written by dwrr on 202407217.003650
updated on 1202501046.114848

this language is designed to be a simple and easy to learn common subset between multiple risc-like isa's, in such a way to allow for the maximum possible performance without requiring the user to write register numbers or write particular isa op codes. rather, the isa of the language is made to facilitate building those constructs emergently.

the language is word based, and each instruction/statement is always of a fixed arity and in prefix form, ie, the operation name comes before a predefined finite number of operands, listed after the operation name seperated by spaces. there is no delimiter neccessary between instructions, due to the fixed arity of each instruction. whitespace is completely irgnored, except for the purposes of delimiting words. 

additionally, any symbols (including unicode) can be used within user variable names. there are no literal constants in this language, as all constants are defined computationally through instructions. 

this language targets risc-v (32 bit and 64 bit) arm (32 bit and 64 bit) as well as possibly x86-64, in the future. the first two archs and their two subtypes are the main priorities in the language, however.

the isa of the language is the following:

	zero x  : set zero the register x.
	incr x  : increment x by 1.
	decr x  : decrement x by 1.
	not x   : bitwise invert all bits in x.

	set x y : assign the value y into the variable x, ie set x to have value y.
	add x y : set x to be x + y.
	sub x y : set x to be x - y.
	mul x y : set x to be x * y.
	div x y : set x to be x / y.

	and x y : set x to the bitwise AND of x and y.
	or x y  : set x to the bitwise OR of x and y.
	eor x y : set x to the bitwise XOR  of x and y.
	si x y  : shift the value in x up by y bits.
	sd x y  : shift the value in x down by y bits. (unsigned shift.)

	ld x a t : load t bits from address a, and store these bits in register x.
	st a x t : load t bits from register x, and store these bits into t bits starting from address a.

	lt x y l : compare x and y, if x is less than y, branch to label l.	
	ge x y l : compare x and y, if x is not less than y, branch to label l.	
	eq x y l : compare x and y, if x is equal to y, branch to label l.	
	ne x y l : compare x and y, if x is not equal to y, branch to label l.	
	do l     : branch unconditionally to label l.
	at l     : attribute the label l to represent the position of the next instruction after this one. 

	sc n a b c d e f : perform system call with language syscall number n, giving up to arguments a through f.	
	ct x : require x to be known at compiletime.
	rt x b : require x to be known at runtime, and set the bit width of the values stored in x to be b bits.
	lf f : load a file with filename f, and include its source in replace of this instruction. 
	eoi : end of input, causes parsing to stop. all text past this point is ignored. 

	
ultimately, this language is just meant for my own use, however it might be the case that others could find it useful or interesting possibily. 

thanks for reading!

dwrr

