a (currently unnamed) cross-assembler
written on 1202507196.013059 by dwrr
=======================================

this is a cross-assembler that i am making for fun and for my own use. the instruction set architectures (ISA's) which this assembler is able to target currently includes RISC-V 32-bit and 64-bit (RV32IM/RV64IM), ARM 64-bit (Aarch64), and the MSP430 ISA. supporting ARM 32-bit is planned, however not currently implemented. this assembler supports the output file formats: Mach-O executables, Macho-O object files, UF2 files, hex array files, and TI TXT files. supporting ELF executables, and ELF object files is planned, however not currently implemented. 

in addition to the machine instructions for all supported targets, this assembler features a powerful turing-complete compile-time execution system. there are 24 compile-time instructions available for use to perform arbitrary transformations on the output machine code programmatically. they allow for many optimizations to be written at user-level, and the construction of macros at user-level. all of these compile-time instructions take 0, 1, 2, or 3 arguments, and have a simple interface and semantics.

all instructions (both compile-time and machine instructions) are written in a whitespace-delimited, word-based, prefix, fixed-arity, syntax using only alphanumeric characters-- somewhat atypical of most assemblers. whitespace is completely ignored, except for the purposes of separating neighboring words: at least one space (ASCII byte 32), newline (ASCII byte 10) or tab (ASCII byte 9) character must be present between two words for them to be considered different words. (the only exception to this is strings, see the "str" instruction description/semantics below.) finally, there is no delimiter between neighboring instructions except for at least one whitespace character that is neccessary to delimit the neighboring words.

prefix, meaning that the instruction name is always written first. fixed-arity, meaning that the instruction name is always followed by a predefined fixed number of arguments-- all of which must be present. furthermore, the only values available in the assembler are compile-time integer variables and immediates. everything in the assembler is of this form, including registers, labels, etc. 

the way compile-time execution (CTE) works is that while the user's compile-time (CT) instructions are executed, any machine instruction (MI) that is encountered during execution is generated into a runtime instruction list, and these runtime instructions will subsequently be turned into the executable bytes after CTE. the arguments to the MI are the CT values of the variables or constants at the point of the MI being encountered during CTE. for example, if through execution of the CT program, the program's control flow ends up skipping over some MI's, those will not be generated into the final executable. this system allows for arbitrary construction / transformations on the runtime program using a turing complete CT program.

additionally, the only data type present in the entire assembler are natural numbers (also known as non-negative integers) expressed in little-endian binary, of various bit widths. all literals/immediates are written in little-endian binary as well. (see section "reasoning for preferring little-endian binary", for more details). for better readability, underscores ("_") can appear anywhere in literals, and are ignored.  hexadecimal or decimal literals are not supported, and are not planned to be supported ever. however, there are plans to eventually support CT operations on signed integers, and floating point arithmetic / MI's-- this is not currently implemented. 

additionally, several important instructions relating to memory atomic operations, in ARM64, and RISC-V are planned to be implemented eventually as well, but this isnt a major problem at the moment. finally, the entire ARM 32-bit backend/ISA is a major priority which i'll get to soon enough, hopefully. currently i just don't have a use for this ISA yet in my projects, however.

the interface to the assembler's executable itself is simple: exactly one source file (usually with a ".s" extension, although this is not required) is given as the first and only command-line argument to the assembler. this file can include other source files using the "file" instruction (see description and semantics below), and all configuration data about how the output should be formed, and which target is selected is specified programmatically in the main source file (or other included source files) using compile-time instructions. 

a list and description of all available instructions (both runtime and compiletime) and their semantics is given in the "instruction specifications" section. 



[todo: compiletime execution semantics, how labels work, and other cool stuff i'm forgetting to explain..?
notes: execution passing through a runtime label scope duplicates]






-------------------------------------------------------------------------
reasoning for preferring little-endian binary literals only:
-------------------------------------------------------------------------

as stated above, the only data type present in the entire assembler are natural numbers (also known as non-negative integers) expressed in little-endian binary, of various bit widths, and this includes literals/immediate values as well. to be clear, here is an example for the definition of little-endian binary. 

for example, "00011" is the immediate 24 in decimal:

	. the first bit, the least significant bit, is written first, and has index [0]. 
	. the last bit, the MSB, has index [4]. 
	
	00011 is 24 because 

		24 = 0 * 2^[0] + 0 * 2^[1] + 0 * 2^[2] + 1 * 2^[3] + 1 * 2^[4]
		24 = 0 * 2^0 + 0 * 2^1 + 0 * 2^2 + 1 * 2^3 + 1 * 2^4
		24 = 0 * 1 + 0 * 2 + 0 * 4 + 1 * 8 + 1 * 16
		24 = 8 + 16
		24 = 24
	

the reason for preferring a little-endian digit ordering is the following. 

firstly, it is more fundamental at a computational level (as in, software working with little-endian binary numbers is simpler than big-endian), as the bit indexing axis goes in the same direction as array indexing. in the above example, notice how the order of the bit indexes ("[n]") is the same as the ordering for indexes in an array (where array[0] is always the first element). this turns out to be helpful for thinking about arithmetic and numbers when programming in practice, especially in binary. 

secondly, at a hardware level, the bit representations for variable bit-width loads and stores nest properly, because of the fact that all N-bit (where N > 0) binary numbers have a bit [0] defined, and it is in the same place in memory for all N-bit binary numbers no matter what N is. similarly for N > 1, all N-bit (where N > 1) binary numbers have a bit [1] defined, and this bit is in the same place in memory for all of these binary numbers, no matter what N is. etc, for all values of N, and lower limits on N. the representations for binary numbers nest when represented as little-endian. this is also how hardware is simpler with little-endian representations, and why 99.99% of all modern hardware implementations use little-endian over big-endian.

thirdly, all supported targets are little-endian machines: RISC-V is little-endian in its byte ordering as per the specification, and because shifts (e.g. "SLL") address bits little-endian-style (ie, accessing bit [0] uses SLL shift index 0), RISC-V is thus fully little endian. similar reasoning also applied for Aarch64, and MSP430 as well. 

the reason why prefering binary over decimal is due to the fact that writing constants in binary exposes several things which hexadecimal and decimal obfuscate, such as: 

	. binary allows for easier arithmetic and logical operations including, integer division, shifting, bitwise operations, 
	. binary allows the values of hardware peripheral bit fields to be easily written and interpretted when doing embedded programming, and,
	. binary is closer to the actual mental model for how bit-compacted data is stored internally, and thus allows for easier debugging. all modern computers are binary, and completely operate on binary internally. 

furthermore, given the fact we should represent numbers using little-endian, using decimal would perhaps be even more confusing, and the benefits of little-endianness are really only useful when numbers are represented in binary. 







------------------------------------------------------------------------------------
			instruction specifications:
------------------------------------------------------------------------------------

a complete specification of all available instructions (both runtime and compiletime) and their semantics is given below.
note, this section is still very much a work in progress, several instructions are not documented fully yet. WIP. 

instruction listing:

	zero incr set 
	add sub mul div rem 
	and or eor si sd
	ld st emit sect at lt eq 
	file del str eoi 

	rr ri rs rb ru rj

	mo mb

	svc mov bfm	
	adc addx addi addr adr 
	shv clz rev jmp bc br 
	cbz tbz ccmp csel 
	ori orr extr ldrl 
	memp memia memi memr 
	madd divr



instruction set overview:
------------------------------------

compiletime:
---------------
	zero : set to zero
	incr : increment by one
	set  : assignment / copy
	add  : addition
	sub  : subtraction
	mul  : multiplication
	div  : division
	rem  : remainder
	and  : bitwise and
	or   : bitwise or
	eor  : bitwise exclusive or
	si   : shift increase
	sd   : shift decease
	ld   : load from compiletime memory
	st   : store to compiletime memory
	emit : emit data to executable
	sect : section address attribution
	at   : label attribution
	lt   : branch on less-than comparison
	eq   : branch on equal comparison
	file : parse contents of file from the file system
	del  : delete variable from symbol table
	str  : emit bytes for verbatim string literal
	eoi  : end of input

RISC-V:
---------------
	rr : register-register integer operation instruction format
 	ri : register-immediate integer operation instruction format
	rs : store operation instruction format
	rb : register compare and branch to label instruction format
	ru : upper immediate load into register instruction format
	rj : unconditionally jump to label instruction format

MSP430:
---------------
	mo : arithmetic and bitwise operation instruction format, 
		with destination and source addressing modes 
	mb : branch on condition to label instruction format

arm64:
---------------
	svc 	: system call (supervisor call)
	mov 	: set register with 16 bit immediate source
	bfm 	: bit field move instruction
	adc 	: add/subtract with carry 
	addx 	: add/subtract with optionally sign/zero-extended register source 
	addi 	: add/subtract with immediate source
	addr 	: add/subtract with optionally shifted register source
	adr 	: load address of a PC-relative label into a register
	shv 	: variable-shift left or right of a register
	clz 	: count leading zeros of regsiter
	rev 	: reverse bits of register
	jmp 	: unconditional jump to label (with optional link)
	bc 	: conditional branch to label based on flags 
	br 	: branch to register value (with optional link)
	cbz 	: compare and branch if register is nonzero or zero
	tbz 	: test bit in register and branch to label if set
	ccmp 	: conditional compare instruction
	csel 	: conditional select / increment / invert / negate instruction
	ori 	: bitwise or/and/xor with immediate
	orr 	: bitwise or/and/xor with shifted register
	extr 	: extract instruction (?)
	ldrl 	: load register data with from label PC-relative address
	memp 	: load/store memory to/from pair of registers
	memia 	: load/store memory to/from register with post/pre increment addressing mode
	memi 	: load/store memory to/from register with address plus unsigned immediate offset
	memr 	: load/store memory to/from register with address plus register offset
	madd 	: multiply-accumulate / multiply-subtract instruction
	divr 	: divide register with register instruction





---------|---------|---------|---------|---------|---------|



more detailed descriptions and semantics:
-----------------------------------------------------------
a note on the legend for each instruction's documentation: 
first the instruction name/mnemonic is given, followed by a list of whitespace seperated operands/arguments, each of the form:

	A.B

where:
	A is the name of the argument position, refered to in the instruction's description.
	B is the maximum number of bits allowed for this operand. 

for example, lets take the first instruction, "zero x.64". 

for the first and only argument, A would be "x", and B would be 64. 

"zero x.64" denotes that it is named "zero", and takes a single operand/argument internally named "x", and all values (interpretted as a binary unsigned integer) which are passed into argument x for this instruction must be less than 2 to the 64, ie, they could be represented by a 64-bit binary unsigned integer.


compiletime system:
---------------------------------------------------------


---------------------------------------------------------
	eoi
---------------------------------------------------------
	end of input. when this instruction is 
	encountered, parsing stops, and the rest of
	the file after this word is ignored. 
	useful for commenting out code, writing larger
	human-readable comments or documentation, etc.
	always executes at parse-time only.


---------------------------------------------------------
	zero x.64 
---------------------------------------------------------
	set the variable x to 0. if the name x is not defined, 
	then x is defined as a result of this instruction. 
	if x is defined, the existing definition of variable 
	x is used. 
	always executes at compiletime.


---------------------------------------------------------
	incr x.64
---------------------------------------------------------
	increment the variable x by 1. 
	always executes at compiletime.


---------------------------------------------------------
	set x.64 y.64
---------------------------------------------------------
	assignment to destination x, using the value 
	present in source y. if the name x is not 
	defined, then x is defined as a result of this 
	instruction. if x is defined, 
	the existing definition 
	of variable x is used. 
	always executes at compiletime.


---------------------------------------------------------
	add x.64 y.64
---------------------------------------------------------
	assigns the value x + y to the destination 
	variable x. 
	always executes at compiletime.


---------------------------------------------------------
	sub x.64 y.64
---------------------------------------------------------
	assigns the value x - y to the destination 
	variable x. 
	always executes at compiletime.


---------------------------------------------------------
	mul x.64 y.64	
---------------------------------------------------------
	assigns the value x * y to the destination 
	variable x. 
	always executes at compiletime.


---------------------------------------------------------
	div x.64 y.64
---------------------------------------------------------
	assigns the value x / y to the destination 
	variable x. 
	always executes at compiletime.


---------------------------------------------------------
	rem x.64 y.64
---------------------------------------------------------
	assigns the value x modulo y to the destination 
	variable x. 
	always executes at compiletime.


---------------------------------------------------------
	and x.64 y.64
---------------------------------------------------------
	assigns the value x bitwise-and y to the destination 
	variable x. 
	always executes at compiletime.


---------------------------------------------------------
	or x.64 y.64
---------------------------------------------------------
	assigns the value x bitwise-or y to the destination 
	variable x. 
	always executes at compiletime.


---------------------------------------------------------
	eor x.64 y.64
---------------------------------------------------------
	assigns the value x bitwise-exclusive-or y to the destination 
	variable x. 
	always executes at compiletime.


---------------------------------------------------------
	si x.64 y.64
---------------------------------------------------------
	assigns the value x shifted up by y bits to the destination 
	variable x. a shift by 1 bit (where y is 1) is equivalent 
	to multiplying x by 2 and assigning this result to x.
	always executes at compiletime.


---------------------------------------------------------
	sd x.64 y.64
---------------------------------------------------------
	assigns the value x shifted down by y bits to the destination 
	variable x. a shift by 1 bit (where y is 1) is equivalent 
	to dividing x by 2 and assigning this result to x.
	always executes at compiletime.

---------------------------------------------------------
	ld destination.64 address.64
---------------------------------------------------------
	load 8 bytes from source compiletime memory 
	at compiletime memory address "address" 
	into destination variable "destination". 
	this also declares x if x is not already defined. 
	always executes at compiletime.


---------------------------------------------------------
	st address.64 source.64
---------------------------------------------------------
	store 8 bytes from source variable "source" 
	into destination compiletime memory at 
	compiletime memory address "address". 
	always executes at compiletime.


---------------------------------------------------------
	emit size.4 value.64
---------------------------------------------------------
	emit "size" number of least-significant bytes 
	from variable "value" to the executable as raw 
	data, at this position in the instruction stream.
	"value" can never be a value that is unable to be
	represented by a (size * 8) bit unsigned integer.
	always executes at compiletime.



---------------------------------------------------------
	str (string)
---------------------------------------------------------
	emit string data from s to the final executable 
	at this position. str is equivalent to a series 
	of emit byte instructions.
	always executed at compiletime.

	the format of the (string) parameter must 
	be of the form:

		ABA

	where:
		A is any printable ASCII character of 
		  your choice, and	
		B is an arbitrary sequence of 
		  characters, not containing A.

	typically A will be chosen to be a double 
	quote character, thus calls to this look 
	reasonably familiar:

		str "your string here"


---------------------------------------------------------
	sect x.64 
---------------------------------------------------------
	puts the following instructions at hardware
	address x. this instruction is only 
	valid to use on embedded targets, or targets 
	which don't have address layout randomization,
	and where it makes sense to access particular 
	runtime memory addresses.
	always executes at compiletime.


---------------------------------------------------------
	lt x.64 y.64 l.64
---------------------------------------------------------
	if x is less than y, compiletime branch to label l. 
	this is always an unsigned comparison. 
	before the branch occurs, this instruction 
	stores the instruction index of this instruction 
	to address 0 in compiletime memory. 
	address 0 is also known as the compiletime 
	return address, or the link register sometimes.
	this instruction always executes at compiletime.
	always executes at compiletime.

---------------------------------------------------------
	eq x.64 y.64 l.64
---------------------------------------------------------
	if x is equal to y, compiletime branch to label l. 
	before the branch occurs, this instruction 
	stores the instruction index of this instruction 
	to address 0 in compiletime memory. 
	address 0 is also known as the compiletime 
	return address, or the link register sometimes.
	this instruction always executes at compiletime.
	always executes at compiletime.


---------------------------------------------------------
	at l.64
---------------------------------------------------------
	attribute label l at this position. to perform 
	this, the instruction index of this instruction 
	in the input instruction stream is loaded 
	into the compiletime variable associated with l. 
	always executes at compiletime. 

---------------------------------------------------------
	del x
---------------------------------------------------------
	remove x (which can be any defined variable) 
	from the symbol table. executes at parse-time, 
	(which happens before parse time execution)
	and thus does not follow the compiletime 
	execution flow of the program. 

---------------------------------------------------------
	file (file path)
---------------------------------------------------------
	load the contents of the file at filepath f, 
	and parse them fully before proceeding.  
	any edits to the symbol table are 
	persistent/stateful. executes at parse-time, 
	and does not follow the execution flow of 
	the program. currently, the file path cannot
	contain strings, this might be changed soon.
	
	example calls look like:

		file library/directory/my_file_here.s

	if the same file-path is included multiple times 
	in the program, this results in an error. the user
	should arrange the dependancies and use of 
	"file" instructions to never include a file
	multiple times.





---------------------------------------------------------
risc-v:
---------------------------------------------------------



	...TODO: document all risc-v instructions!!!...



---------------------------------------------------------
	rr op.7 funct3.3 destination.5
		source1.5 source2.5 funct7.7
---------------------------------------------------------
	
	work in progress still!

	register-register operation instruction.


	if op == XXXXX and funct3 == XXX
	and funct7 == 0, then destination
	is loaded with source1 + source2.

	if op == XXXXX and funct3 == XXX
	and funct7 == XXXXXXX, then destination 
	is loaded with source1 - source2.



	if op == XXXXX and funct3 == XXX
	and funct7 == XXXXXXX, then destination 
	is loaded with source1 bitwise AND'd 
	with source2.

	if op == XXXXX and funct3 == XXX
	and funct7 == XXXXXXX, then destination 
	is loaded with source1 bitwise OR'd 
	with source2.

	if op == XXXXX and funct3 == XXX
	and funct7 == XXXXXXX, then destination
	is loaded with source1 bitwise
	exclusive OR'd with source2.





---------------------------------------------------------
	ri op.7 funct.3 destination.5 
		source1.5 immediate.12
---------------------------------------------------------
	undocumented so far


---------------------------------------------------------
	rs op.7 funct.3 address.5 
		source.5 immediate.12
---------------------------------------------------------
	undocumented so far


---------------------------------------------------------
	rb op.7 condition.3 source1.5 source2.5 label.12
---------------------------------------------------------
	undocumented so far


---------------------------------------------------------
	ru op.7 destination.5 immediate.20
---------------------------------------------------------
	undocumented so far


---------------------------------------------------------
	rj op.7 destination.5 label.20
---------------------------------------------------------
	undocumented so far




---------------------------------------------------------
msp430:
---------------------------------------------------------

	TODO: document all MSP430 instructions!!!



---------------------------------------------------------
	mo  opcode.4    destination_mode.1    
		destination_register.4     
		destination_immediate.16 
		source_mode.2    source_register.4    
		source_immediate.16     is_8bit.1
---------------------------------------------------------

	undocumented so far


---------------------------------------------------------
	mb cond.3 label.11
---------------------------------------------------------
	undocumented so far


---------------------------------------------------------
arm64:
---------------------------------------------------------

---------------------------------------------------------
	nop
---------------------------------------------------------
	runtime no-operation instruction.
	does nothing.

---------------------------------------------------------
	svc
---------------------------------------------------------
	runtime system call instruction.


---------------------------------------------------------
	mov  Rd.5  imm.16  shift_amount.2  
		mov_type.2
---------------------------------------------------------
	register immediate load.

	shift_amount == 0 means no shift, 
		1 means shift up by 16 bits, 
	2 means shift up by 32 bits, 
		3 means shift up by 64 bits. 

	mov_type == 2 means movz, 
	which zeros all bits except for the ones 
	used by the already-shifted 16 bit immediate.

	mov_type == 0 means movn, 
	which does the same thing as movz, 
	except for the result is inverted 
	after doing the movz. 

	mov_type == 3 means movk, which does 
	the same thing as movz, except that it keeps 
	all existing bits already present 
	in the destination
	besides the ones used by the shifted immediate.


---------------------------------------------------------
	adc   Rd.5  Rn.5  Rm.5  should_setflags.1  
		should_subtract.1
---------------------------------------------------------
	add two source registers with carry flag, 
		and store into destination register.


---------------------------------------------------------
	adr  Rd.5 label.21  is_page_addressed.1 
---------------------------------------------------------
	load pc-rel address into register


		
---------------------------------------------------------
	addi   Rd.5  Rn.5  imm.12  
		should_imm_shift12.1  should_setflags.1  
		should_subtract.1
---------------------------------------------------------
	add source register with immediate and 
	store into destination register. 
	
	Rd/Rn == 31 means the stack pointer, instead of 
	the zero register.


---------------------------------------------------------
	addr    Rd.5  Rn.5  Rm.5  imm.6   
		shift_type.2  should_setflags.1  
		should_subtract.1
---------------------------------------------------------
	add source register with optionally 
	immediate-amount-shifted source register and 
	store into destination register.

	shift_type == 0 means logical left shift, 
	shift_type == 1 means logical right shift, 
	shift_type == 2 means arithmetic right shift.
		


---------------------------------------------------------
	br register.5 type.1
---------------------------------------------------------
	type can be 0, 1 or 2. 
	type == 2 means a return hint,
	type == 1 means it will link with x30 
	type == 0 means just an indirect register branch.



---------------------------------------------------------
	jmp   should_link.1   label.26
---------------------------------------------------------
	unconditional branch to a pc-relative-offset 
	label. 



---------------------------------------------------------
	bc    cond.4   label.19  
---------------------------------------------------------

	conditional branch based on the condition 
	and flags register state to a pc-rel label.

	cond == 15 means always false
	cond == 14 means always true
	cond == 0 means is equal (zero flag is set)
	cond == 1 means is not equal (zero flag is set)
	cond == 4 means is negative 
		(negative flag is set)
	cond == 5 means is non-negative 
		(negative flag is clear)
	cond == 6 means the overflow flag is set
	cond == 7 means the overflow flag is clear		
	cond == 11 means is signed less than
	cond == 12 means is signed greater than
	cond == 13 means is signed less than or equal
	cond == 10 means is signed greater than or equal
	cond == 3 means is unsigned less than (carry set)
	cond == 8 means is unsigned greater than
	cond == 9 means is unsigned less than or equal
	cond == 2 means is unsigned greater 
		than or equal (carry clear)



---------------------------------------------------------
	shv   dest.5  source.5  shift_amount.5  type.2
---------------------------------------------------------
	dest, source, and shift_amount are all registers.

	type == 0 means	shift left logically
	type == 1 means	shift right logically
	type == 2 means	shift right arithmetically
	type == 3 means	rotate right


---------------------------------------------------------
	cbz   source.5  label.23  ifnonzero.1
---------------------------------------------------------
	if ifnonzero is 1, it performs a 
	branch-nonzero on source to label.
	if ifnonzero is 0, it performs a 
	branch-ifzero on source to label.


---------------------------------------------------------
	tbz  source.5  bitindex.6  label.23  ifnonzero.1
---------------------------------------------------------
	checks if the bitindex-th bit is set or not.
	if ifnonzero is 1, then it branches if that bit is 
	1. if ifnonzero is 0, then it branches if that
	bit is 0.



---------------------------------------------------------
	divr  destination.5  source1.5  
		source2.5  issigned.1
---------------------------------------------------------
	performs register division of source1 / source2,
	and stores result into destination.
	if issigned is 1, the division is a signed division.
	else, it is an unsigned division.



---------------------------------------------------------
	ori op.2 destination.5 source1.5 immediate.64
---------------------------------------------------------
	if op is 0, this performs an and, 
	if op is 1, this performs an or,
	if op is 2, this performs an xor,
	if op is 3, this peforms an and, 
		while setting the flags.


TODO:

	work in progress still!!!




---------------------------------------------------------
	orr op.2 destination.5 source1.5 negate.1 
		source2.5 shift_type.2 shift_amount.6
---------------------------------------------------------
	if op is 0, this performs an and, 
	if op is 1, this performs an or,
	if op is 2, this performs an xor,
	if op is 3, this peforms an and, 
		while setting the flags.

	first, an optionally shifted version of the 
	source2 reg is constructed, called k. 
	source2 is not edited. 

	the shift type follows addi's shift type encoding.

	then, this value is optionally bitwise negated.
	if negate is 1, then k is inverted in place.
	else, k is unaffected, and passes on as is.
	
	finally, the source1 and k are combined
	using the bitwise operation seleted by op, 
	and this value is written to destination.
	
		



---------------------------------------------------------
	memia  op.2  size.2  data.5  
		address.5  immediate.9  preincr.1
---------------------------------------------------------
	op == 0 means store
	op == 1 means unsigned load
	op == 2 means signed load
	op == 3 is invalid, and should not be used.

	size == 0 means 8bit load/store
	size == 1 means 16bit load/store
	size == 2 means 32bit load/store
	size == 3 means 64bit load/store

	if size is 3, op can only be 0 or 1.

	address == 31 means the stack pointer, not
	the zero register.

	first, 9-bit immediate is sign-extended to 64 bits.

	the address register value is added with 
	this sign extended immediate, to form the 
	address, called A.
	
	if preincr is 1, then the address register is modified
	at this point by adding this sign extended immediate to it.
	if this happens, A is then set to the resultant 
	modified value of the address register.
	
	for a load, 2 to the size number of bytes 
	are then loaded from memory at A,
	and then stored into the data register.
	if a signed load, then the resultant data is sign extended to 64 bits.

	however, for a store, 2 to the size number of
	bytes are read from the data register,
	and stored to memory starting at A.

	following the load/store's completion, 
	if preincr is 0, then the address register 
	is modified by adding the sign extended immediate to it.





---------------------------------------------------------
	memi  op.2  size.2  data.5  
		address.5  immediate.12
---------------------------------------------------------
	op == 0 means store
	op == 1 means unsigned load
	op == 2 means signed load
	op == 3 is invalid, and should not be used.

	size == 0 means 8bit load/store
	size == 1 means 16bit load/store
	size == 2 means 32bit load/store
	size == 3 means 64bit load/store

	if size is 3, op can only be 0 or 1.

	address == 31 means the stack pointer, not
	the zero register.

	first, the immediate is shifted up by size 
	number of bits.

	the address register value is added with 
	this shifted version of the zero extended 12-bit immediate. 
	this forms the address, called A.

	for a load, 2 to the size number of bytes 
	are then loaded from memory at A,
	and then stored into the data register.
	if a signed load, then the resultant data 
	is sign extended to 64 bits.

	however, for a store, 2 to the size number of
	bytes are read from the data register,
	and stored to memory starting at A.






---------------------------------------------------------
	memr  op.2  size.2  data.5  address.5  
		offset.5  should_scale.1
---------------------------------------------------------
	op == 0 means store
	op == 1 means unsigned load
	op == 2 means signed load
	op == 3 is invalid, and should not be used.

	size == 0 means 8bit load/store
	size == 1 means 16bit load/store
	size == 2 means 32bit load/store
	size == 3 means 64bit load/store

	address == 31 means the stack pointer, not
	the zero register.

	first, the offset register is shifted by up
	size bits, if the should_scale bit is 1.

	next, the address A is calculated by adding 
	the optionally shifted offset register with
	the address register.

	for a load, 2 to the size number of bytes 
	are then loaded from memory at A,
	and then stored into the data register.
	if a signed load, then the resultant data 
	is sign extended to 64 bits.

	however, for a store, 2 to the size number of
	bytes are read from the data register,
	and stored to memory starting at A.



---------------------------------------------------------
	memp   is_load.1 is_64_bit.1  data1.5 data2.5 
		address.5 immediate.7 mode.2
---------------------------------------------------------
	performs a load-pair if is_load, 
	else performs a store-pair.

	address == 31 means the stack pointer, not
	the zero register.

	first, the value k is set to 2 + is_64_bit.

	next, the immediate value is sign extended 
	to 64 bits.

	next, the immediate is shifted up by
	k bits. the value A is then constructed 
	by adding the value in the address register with
	this shifted immediate.

	if mode == 3, then the value A is stored into
	the address register, at this point.

	next, if is_load, 2 to the k bytes are loaded 
	from memory starting from A, and this is stored
	in data1. 

	then, another 2 to the k bytes are loaded
	from memory starting at A + (2 to the k), and these
	bytes are stored into data2. 

	if mode == 1, the value of A is stored into 
	the address register at this point.





---------------------------------------------------------
	csel  	... : not documented yet
---------------------------------------------------------


---------------------------------------------------------
	ori  	... : not documented yet
---------------------------------------------------------


---------------------------------------------------------
	madd  	... : not documented yet
---------------------------------------------------------




---------------------------------------------------------
	ldrl  	... : not documented yet
---------------------------------------------------------

---------------------------------------------------------
	clz  	... : not documented yet
---------------------------------------------------------


i'll do these later:

---------------------------------------------------------
	extr  	... : not documented yet
---------------------------------------------------------
---------------------------------------------------------
	ccmp  	... : not documented yet
---------------------------------------------------------
---------------------------------------------------------
	rev  	... : not documented yet
---------------------------------------------------------
---------------------------------------------------------
	addx  	... : not documented yet
---------------------------------------------------------
---------------------------------------------------------
	bfm  	... : not documented yet
---------------------------------------------------------



final remarks:
------------------------

additional reminder that the assembler is made specifically and primarily for my own use cases, and thus is not tailored for anyone else. additionally, because the assembler is still largely in development and quite drastic changes could be made quite often, is it not currently meant to be used by anyone other than me at all. as such, pull requests or feature suggestions will most likely be rejected, (unless it really knocks my socks off, then i might consider it, but it would have to be something huge that i missed, lol..)

however, that being said, i do plan to make the project publically usable by others for their own purposes, once the assembler and standard library are polished and stable enough to where i am happy with them. 
regardless, i hope you find the project interesting, or possibly learn something or find inspiration from it. thanks for reading!

dwrr
































































































































































































--------------------------------------------------------------------------------------------------
everything from this point on is the trash, feel free to completely ignore it, as its completely irrelevant:
--------------------------------------------------------------------------------------------------


















all arguments to both compile-time and machine instructions are of this form as well. after compile-time execution finishes, all compile-time integer variables reduce to constant integer immediates supplied to the arguments of machine instructions, and compile-time constants or variables which were not given to machine instruction arguments are discarded. all values in the assembler are known at compile-time-- there is no true runtime data.












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

the source code for the compiler is currently a tad under 5,000 lines of code so far, and is expected to grow slightly as more optimization passes are implemented, and instruction selection and register allocation is flushed out further. this source code is located in a single C file, "c.c". most of the code, including parsing, lexing, optimization passes, instruction selection, register allocation, and machine code generation and output generation is all done in the main() function, with minimal use of functions. 

using multiple files besides "c.c", or using widespread use of functions across the code base for readability is not planned or desired, as it significantly increases the friction at which changes can be made to the compiler, and significantly decreases the oppurtunities for new algorithms and common patterns to be discovered in the source code while implementing things. 

also, if the C source code is "unreadable" to you, this is, technically speaking, a "skill issue" on your part. i reccomend improving your programming skills until it is readable for you. 


a notable difference is also that the compiler does not use SSA form, or the notion of basic blocks. rather, all data flow analysis and optimization passes take into account the full global data/control flow, and all control flow and data flow analysis and optimizations are done in a completely stateful (ie, tracing knowledge forwards or backwards through the control-flow-graph (CFG) and data-flow-graph (DFG), statefully) and global (taking into account the entire program's CFG and DFG) manner on the entire program. 

finally, the language itself can be thought of as the same language as the intermediate representation (IR) for the compiler, as the CFG and DFG representation internally is represented in the exact same manner as the users code: using "at" instructions, conditional branches ("lt", "eq", etc) using label names, and operations like "set", "add", etc. this homogeneity between the internal and external (user-facing) representation allows the programmer to have a deeper understanding of how the compiler is interpretting the code, and potentially control the final executable to an even higher degree, for maximum performance. 

this homogeneity also serves to keep the implementation of the compiler itself, simple and straight forward, as the control flow and data flow is never stored or created in data explicitly. rather, it is computationally derived from scratch when needed, allowing for the program in this internal representation to be changed easily and simply, during the process of optimization. 


...see the code examples section for what code in this language generally looks like!...






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

reg z 0011        (variable z is forced to be stored in the hardware register 
			"x12" assuming this exists for the target.)

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






-------------------- [EXAMPLE 4] --------------------

(testing out printing prime numbers in binary using the risc-v backend! 
written on 1202507045.233538 by dwrr)

file library/foundation.s

(...this would all be in the standard library...)

rt 	set a0 a0  
	set a1 a1	
ct 	set c0 c0 
	set c1 c1 
do skip

at rv_backend
	ld ra compiler_return_address nat
	st compiler_target rv32_arch nat
	st compiler_format hex_array nat
	st compiler_should_overwrite true nat
	do ra del ra

at exit
	ld ra compiler_return_address nat
	rt set rv_system_number rv_system_exit
	set rv_system_arg0 0
	sc halt ct do ra del ra

at print
	ld ra compiler_return_address nat
	rt set rv_system_number rv_system_write
	set rv_system_arg0 stdout
	set rv_system_arg1 a0
	set rv_system_arg2 c0
	sc ct do ra del ra

at skip del skip


set newline 0101

(...until here...)

(my code starts here!)

do skip

at print0 ct 
	ld ra compiler_return_address nat
	rt set a0 digitzero
	set c0 01
	do print
	ct do ra del ra

at print1 ct
	ld ra compiler_return_address nat
	rt set a0 digitone
	set c0 01
	do print
	ct do ra del ra

at print_newline ct
	ld ra compiler_return_address nat
	rt set a0 newline_char
	set c0 1
	do print
	ct do ra del ra

at printbinary
	ld ra compiler_return_address nat
	rt set data a0
	at loopb set bit data and bit 1
	eq bit 0 else do print1 do done 
	at else do print0 at done
	sd data 1 ne data 0 loopb
	do print_newline
	ct do ra del ra 
	del loopb
	del bit del data 
	del done del else 
	del print0 
	del print1 
	del print_newline

at footer
	ld ra compiler_return_address nat
	rt at digitzero str "  "
	at digitone str "##"
	at newline_char emit 1 newline
	ct do ra del ra

at skip del skip

do rv_backend rt

ct set count 0000_0000_0000_0000__01 rt
set i 0
at loop set j 01
at inner ge j i prime
set r i rem r j eq r 0 composite
add j 1 do inner
at prime set a0 i do printbinary
at composite add i 1 lt i count loop
del i del loop del count
do exit do footer


-------------------- [EXAMPLE 5] --------------------

(a program to pwm an LED on GPIO 0 using a 
risc-v uf2 file outputted by the compiler,
running on the pico 2 W.
written 1202505272.173200 by dwrr)

file library/foundation.s ct

st compiler_target rv32_arch nat
st compiler_format uf2_executable nat
st compiler_should_overwrite true nat
st compiler_stack_size 0 nat 

(address atomic bitmasks) 
set clear_on_write 	0000_0000_0000_11
set set_on_write 	0000_0000_0000_01
set toggle_on_write 	0000_0000_0000_1

(memory map of rp2350)

set flash_start 	0000_0000_0000_0000__0000_0000_0000_1000
set ram_start 		0000_0000_0000_0000__0000_0000_0000_0100
set powman_base		0000_0000_0000_0000__0000_1000_0000_0010
set clocks_base		0000_0000_0000_0000__1000_0000_0000_0010
set sio_base		0000_0000_0000_0000__0000_0000_0000_1011
set reset_base 		0000_0000_0000_0000__0100_0000_0000_0010
set io_bank0_base 	0000_0000_0000_0001__0100_0000_0000_0010
set pads_bank0_base 	0000_0000_0000_0001__1100_0000_0000_0010

(risc-v op codes)
set addi_op1 	1100100
set addi_op2	000
set sw_op1 	1100010
set sw_op2 	010

set reset_clear reset_base 
add reset_clear clear_on_write

set io_gpio0_ctrl 001
set io_gpio1_ctrl 0011
set io_gpio2_ctrl 00101
set io_gpio3_ctrl 00111

set pads_gpio0 001
set pads_gpio1 0001
set pads_gpio2 0011
set pads_gpio3 00001

set sio_gpio_oe 	0000_11
set sio_gpio_out 	0000_1
set sio_gpio_in 	001

rt set a0 a0
set a1 a1
set a2 a2
set a3 a3

ct set c0 c0
set c1 c1
set c2 c2
set c3 c3

do skip_macros

at setif
	ld ra 0 nat
	set a c0 set b c1 
	set c c2 set d c3
	ne a b l st c d nat
	at l del l del a del b  
	del c del d do ra del ra 

at setup_output
	ld ra 0 nat
	set p compiler_base set c2 p

	set c1 0 set c3 io_gpio0_ctrl do setif
	set c1 1 set c3 io_gpio1_ctrl do setif
	set c1 01 set c3 io_gpio2_ctrl do setif
	set c1 11 set c3 io_gpio3_ctrl do setif
 	ld control p nat

	set c1 0 set c3 pads_gpio0 do setif
	set c1 1 set c3 pads_gpio1 do setif
	set c1 01 set c3 pads_gpio2 do setif
	set c1 11 set c3 pads_gpio3 do setif
	ld pads p nat

	del p
	rt set address io_bank0_base
	set data 101
	r5_s sw_op1 sw_op2 address data control
	set address pads_bank0_base
	set data 0_1_0_0_11_1_0_0
	r5_s sw_op1 sw_op2 address data pads
	del pads del control 
	ct do ra del ra


at delay
	ld ra 0 nat
	rt set i 0
	at L ge i c0 done
	add i 1 do L at done
	del i del L del done
	ct do ra del ra


at delayr
	ld ra 0 nat
	rt set ii 0
	at LL ge ii a0 donee
	add ii 1 do LL at donee
	del ii del LL del donee
	ct do ra del ra

at skip_macros del skip_macros
rt adr flash_start

do skip
(rp2350 image_def marker)
emit  001  1100_1011_0111_1011__1111_1111_1111_1111
emit  001  0100_0010_1000_0000__1000_0000_1000_1000
emit  001  1111_1111_1000_0000__0000_0000_0000_0000
emit  001  0000_0000_0000_0000__0000_0000_0000_0000
emit  001  1001_1110_1010_1100__0100_1000_1101_0101
at skip del skip

reg address 101
reg data 011

set address 	reset_clear
set data 	0000_0010_01
r5_s sw_op1 sw_op2 address data 0

set c0 0 do setup_output

set address	sio_base
set data 	1
r5_s sw_op1 sw_op2 address data sio_gpio_oe

set data 1
r5_s sw_op1 sw_op2 address data sio_gpio_out

at loop
	ct 
		set millisecond 		0000_0000_0000_1
		set half_millisecond 		0000_000_1

		set 10_milliseconds millisecond 
		mul 10_milliseconds 0101

		set 5_milliseconds millisecond 
		mul 5_milliseconds 101

		set 3_milliseconds millisecond 
		mul 3_milliseconds 11
	rt
	reg increment 1   set increment half_millisecond
	reg iterator_limit 01  set iterator_limit 10_milliseconds
	reg iterator_limit2 111 set iterator_limit2 3_milliseconds
	reg iterator 11

	set iterator increment
	at inner
		set data 1
		r5_s sw_op1 sw_op2 address data sio_gpio_out
		reg i 001 set i iterator at d sub i 1 ne i 0 d del d del i
		set data 0
		r5_s sw_op1 sw_op2 address data sio_gpio_out
		reg i 001 set i iterator_limit sub i iterator at d sub i 1 ne i 0 d del d del i
		add iterator increment
		lt iterator iterator_limit2 inner del inner

	set iterator iterator_limit2
	at inner
		sub iterator increment
		set data 1
		r5_s sw_op1 sw_op2 address data sio_gpio_out
		reg i 001 set i iterator at d sub i 1 ne i 0 d del d del i
		set data 0
		r5_s sw_op1 sw_op2 address data sio_gpio_out
		reg i 001 set i iterator_limit sub i iterator at d sub i 1 ne i 0 d del d del i
		lt increment iterator inner del inner

	set data 0
	r5_s sw_op1 sw_op2 address data sio_gpio_out
	reg i 001 set i 0000_0000_0000_0000_0000_01 at d sub i 1 ne i 0 d del d del i
do loop















------------------------- reference code ------------------------


for reference, here is the current standard library file, "foundation.s" which is used in some of the above examples:



------------------------- foundation.s code ------------------------

(
	the core standard library for the language: foundation.s
	written on 1202505294.221753 by dwrr.
)

ct 

(numbers)
set -1 0 sub -1 1

(booleans)
set false 0
set true 1

(unix file descriptors)
set stdin  0
set stdout 1
set stderr 01

(unsigned integer sizes)
set byte 	1
set nat16 	01
set nat32 	001
set nat 	0001

(memory mapped ctsc address)
set x 0000 set compiler_return_address x
add x 0001 set compiler_target x
add x 0001 set compiler_format x 

add x 0001 set compiler_should_overwrite x
add x 0001 set compiler_should_debug x

add x 0001 set compiler_stack_size x
add x 0001 set compiler_get_length x
add x 0001 set compiler_is_compiletime x

add x 0001 set compiler_arg0 x
add x 0001 set compiler_arg1 x
add x 0001 set compiler_arg2 x
add x 0001 set compiler_arg3 x
add x 0001 set compiler_arg4 x
add x 0001 set compiler_arg5 x
add x 0001 set compiler_arg6 x
add x 0001 set compiler_arg7 x

add x 0001 set compiler_base x

(compiletime system call interface : call numbers)
set x 0 set compiler_system_debug x
add x 1 set compiler_system_exit x
add x 1 set compiler_system_read x 
add x 1 set compiler_system_write x
add x 1 set compiler_system_open x
add x 1 set compiler_system_close x

(valid arguments to ctsc compiler_target)
set x 0 set no_arch x
add x 1 set arm64_arch x
add x 1 set arm32_arch x
add x 1 set rv64_arch x
add x 1 set rv32_arch x
add x 1 set msp430_arch x
add x 1 set c_arch x

(valid arguments to ctsc compiler_format)
set x 0 set no_output x
add x 1 set macho_executable x
add x 1 set macho_object x
add x 1 set elf_executable x
add x 1 set elf_object x
add x 1 set ti_txt_executable x
add x 1 set uf2_executable x
add x 1 set hex_array x
add x 1 set c_source x

(---------------- c backend -------------------)

(system calls suppported by the c backend) 

set x 0 set c_system_debug x
add x 1 set c_system_exit x
add x 1 set c_system_read x
add x 1 set c_system_write x
add x 1 set c_system_open x
add x 1 set c_system_close x
add x 1 set c_system_mmap x
add x 1 set c_system_munmap x


(constants for the mmap system call interface: )

set prot_read 1
set prot_write 01
set map_private 01
set map_anonymous 0000_0000_0000_1
set map_failed -1

rt 
reg c_system_number 0
reg c_system_arg0 1
reg c_system_arg1 01
reg c_system_arg2 11
reg c_system_arg3 001
reg c_system_arg4 101
reg c_system_arg5 011 
reg c_system_arg6 111 
ct



(--------------------- msp430 -------------------)

((msp430 registers)
reg pc_reg 0
reg sp_reg 1
reg sr_reg 01
reg cg_reg 11
reg r4_reg 001
reg r5_reg 101
reg r6_reg 011
reg r7_reg 111
reg r8_reg 0001
reg r9_reg 1001
reg r10_reg 0101
reg r11_reg 1101
reg r12_reg 0011
reg r13_reg 1011
reg r14_reg 0111
reg r15_reg 1111
)

(msp430 register index constants)
set pc 0
set sp 1
set sr 01
set cg 11
set r4 001
set r5 101
set r6 011
set r7 111
set r8 0001
set r9 1001
set r10 0101
set r11 1101
set r12 0011
set r13 1011
set r14 0111
set r15 1111

(m4_op: op codes)
set msp_mov 001
set msp_add 101
set msp_addc 011
set msp_sub 111
set msp_subc 0001
set msp_cmp 1001
set msp_dadd 0101
set msp_bit 1101
set msp_bic 0011
set msp_bis 1011
set msp_xor 0111
set msp_and 1111

(m4_br: branch conditions)
set condjnz 0
set condjz 1
set condjnc 01
set condjc 11
set condjn 001
set condjge 101
set condjl 011
set condjmp 111

(m4_op: size parameter)
set size_byte 1
set size_word 0

(m4_op: addressing modes)
set reg_mode 0
set index_mode 1
set deref_mode 01
set incr_mode 11

(specific addressing modes)
set imm_mode incr_mode
set imm_reg pc
set literal_mode index_mode
set constant_1 cg
set fixed_reg sr
set fixed_mode index_mode

(msp430 bit position constants)
set bit0 10000000
set bit1 01000000
set bit2 00100000
set bit3 00010000
set bit4 00001000
set bit5 00000100
set bit6 00000010
set bit7 00000001


( ---------------- risc-v -----------------)

(risc-v op codes)
set r5_addi_op1 	1100100
set r5_addi_op2		000
set r5_sw_op1 		1100010
set r5_sw_op2 		010

(risc-v registers)
set r5_zr 0
set r5_ra 1

(rv32 system call abi)
reg rv_system_arg0 0101
reg rv_system_arg1 1101
reg rv_system_arg2 0011
reg rv_system_number 10001 ct

(specific to the rv32 virtual machine running in my website)
set x 1 set rv_system_exit x
add x 1 set rv_system_read x
add x 1 set rv_system_write x

del x

(end of standard library code)































































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





TRASH from the old readme:


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




arm64:

	nop 	: no operation
	svc 	: system call (supervisor call)
	mov 	: set register with 16 bit immediate source
	bfm 	: bit field move instruction
	adc 	: add/subtract with carry 
	addx 	: add/subtract with optionally sign/zero-extended register source 
	addi 	: add/subtract with immediate source
	addr 	: add/subtract with optionally shifted register source
	adr 	: load address of a PC-relative label into a register
	shv 	: variable-shift left or right of a register
	clz 	: count leading zeros of regsiter
	rev 	: reverse bits of register
	jmp 	: unconditional jump to label (with optional link)
	bc 	: conditional branch to label based on flags 
	br 	: branch to register value (with optional link)
	cbz 	: compare and branch if register is nonzero or zero
	tbz 	: test bit in register and branch to label if set
	ccmp 	: conditional compare instruction
	csel 	: conditional select / increment / invert / negate instruction
	ori 	: bitwise or/and/xor with immediate
	orr 	: bitwise or/and/xor with shifted register
	extr 	: extract instruction (?)
	ldrl 	: load register data with from label PC-relative address
	memp 	: load/store memory to/from pair of registers
	memia 	: load/store memory to/from register with post/pre increment addressing mode
	memi 	: load/store memory to/from register with address plus unsigned immediate offset
	memr 	: load/store memory to/from register with address plus register offset
	madd 	: multiply-accumulate / multiply-subtract instruction
	divr 	: divide register with register instruction

MSP430:
	mo : arithmetic and bitwise operation instruction, with destination and source addressing modes 
	mb : branch on condition to label instruction

RISC-V
	rr : register-register integer operation instruction format
 	ri : register-immediate integer operation insrtuction format
	rs : store operation instruction format
	rb : compare and branch with register-register condition to label instruction format
	ru : large immediate (possibly PC-relative) load into register instruction format
	rj : unconditionally jump to label instruction format

