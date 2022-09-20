# my programming language
###### created by Daniel Warren Riaz Rehman.

An interpreted and optimized-compiled low-level imperative systems programming language which feels like a mix of Forth and RISC-V assembly, but is faster than C. 

The current implementation is an REPL and interpreter, but an optimizing compiler is currently in progress. 

### Features:

 - extremely small minimalist language.
 - low level language ISA that maps well onto RISC target ISA's.
 - highly flexible macro system. 
 - a powerful compiletime evaluation system
 - unique operand passing syntax/method.
 - word-based, somewhat like Forth.

### Other Notes:

 - the language is currently still very much a work in progress.
 - macros (parse-time function defs/calls) are relied on heavily to make the language high-level.
 - add more stuff

### Example Code:

nestedforloops.txt:

```
include foundation
comment 
	this is an example of nested for loops.
	note, there is no for loop in this language. 
	this for loop is actually constructed at user-level,
	using macros, and language ISA instructions. 
endcomment

01 literal a01 transfer
5 literal a5 transfer

gensym define 
	i print
	gensym define j print endmacro
	5 j for 
endmacro  
01 i for 

```

foundation:

```
comment 
	currently a very work in progress start
	to a standard library, of macros, and 
	helpful constants.
endcomment

donothing define  nop  endmacro
settozero  define  000 xor   endmacro
constant    define gensym literal  endmacro
double define  constant 21  swap1 001 slli endmacro
transfer    define  000 xor addi endmacro
zero	literal 0 transfer
one	literal 21 transfer
incr     define    one swap1 001 add endmacro

for define 
	store0
	store0 store1 store2
	gensym store0
	gensym store0
	load1 settozero 
load4 now here
	load5 load2 load1 beq 
	load3 callsave call
	load1 incr 
	load4 zero one bne
load5 now here
	load4 undefine
	load5 undefine 
	load1 undefine
	load3 undefine
endmacro
```


