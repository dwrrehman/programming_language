=========================================
          an arm 64-bit assembler
=========================================

this is an arm 64-bit assembler for the M1 max macbook pro (or other arm 64-bit Mach-O machines) with a syntax similar to forth, and C in some ways. this assembler is intended to be the primary programming language that most of my command-line programs will be written in, assuming i can master the language to a sufficient degree. the source code for the assember is less than 900 lines of C code.

the language intentionally only includes the arm64 runtime instructions which have a unique binary encoding- that is, there are no alias instructions. additionally, a set of compiletime-executed instructions are provided, which are used for everything from constructing more complex immediate values, to calculating branch address targets for an instruction, to generating arbitrary data at compiletime in the .text section, to creating compile-time function calls, which essentially provides a sort of macro-like functionality in the assembler. 

the language does not provide any way for the user to create their own names/words/symbols in the program whatsoever- there are a predefined number of symbols in the language which does not change, and thus no dynamic dictionary is neccessary. instead, a set of numeric register names are provided for referencing control flow or data flow points. these could be runtime or compiletime- a given register number supplied to an instruction could be interpreted as a runtime register or compiletime register, depending on the semantics of the instruction. only the hexadecimal numeric index itself is used to identify the register. there are 32 runtime registers, and 4096 compiletime registers.

whitespace is entirely ignored in this language, except for the existence of it to delimit words from one another. there must be at least one whitespace character (defined by isspace(c)) between two given words to parse them as seperate words. 




heres an example program in the language currently:
=====================================================


0 ctbr 

	....this is just a comment btw. (ie, a section of code which never gets executed.)

	this program simply demonstrates the ability for the language to
	have macros using the compiletime system, 

	and create a simple hello world program, using the 
	currently implemented runtime instructions.
	
	also calculating pc relative offsets is tested, along with generating 
	constant data at compiletime to the .text section.

	this language is still a major work in progress!


0 ctstop

5 4 ctldi
14 7 ctldi

1 ctzero


128 ctpc
ctdel 0 ctsta 2 ctzero 1 0 ctbeq 

	7 ctprint
	4 ctprint
	4 ctprint

	2 ctzero 0 0 movzx

0 ctlda ctbr 0 ctstop

129 ctpc 
ctdel 0 ctsta 2 ctzero 1 0 ctbeq 

	77 2 ctldi ctprint
	77 2 ctldi ctprint
	77 2 ctldi ctprint
	nop
	nop

0 ctlda ctbr 0 ctstop

1 ctincr

0 ctprint

64 ctpc 128 ctgoto
64 ctstop

65 ctpc 128 ctgoto
65 ctstop

66 ctpc 129 ctgoto
66 ctstop

41 2 ctldi ctput
20 2 ctldi ctput
42 2 ctldi ctput
a 2 ctldi ctput

2 ctget

43 2 ctldi ctput
42 2 ctldi ctput
41 2 ctldi ctput
a 2 ctldi ctput


1 2 ctldi ctimm 0 0 movzx
0 4 ctldi 0 1 ctadd ctat ctimm 1 adr
3 2 ctldi ctimm 0 2 movzx
4 2 ctldi ctimm 0 10 movzw
svc


2 ctzero ctimm 0 0 movzx
20 9 ctldi ctimm 1f 1f subix
2 ctzero ctimm 1f 1 addix
9 ctimm 0 2 movzx
3 2 ctldi ctimm 0 10 movzw
svc

2 ctzero ctimm 0 6 addix

1 2 ctldi ctimm 0 0 movzx
2 ctzero ctimm 1f 1 addix
2 ctzero ctimm 6 2 addix
4 2 ctldi ctimm 0 10 movzw
svc

1 2 ctldi ctimm 0 0 movzx
4 4 ctldi 0 1 ctadd ctat ctimm 1 adr
6 2 ctldi ctimm 0 2 movzx
4 2 ctldi ctimm 0 10 movzw
svc

2 ctzero ctimm 6 0 addix
1 2 ctldi ctimm 0 10 movzw 
svc

0 4 ctldi 0 1 ctadd 
3 ctld4
1 ctat 2 ctld4
3 2 3 ctsub
2 2 ctldi 3 3 ctshl
3 1 ctst4

3 ctzero

68 4 ctldi 
0 8 ctldi 4 4 ctshl 4 3 3 ctor

69 4 ctldi
8 8 ctldi 4 4 ctshl 4 3 3 ctor

a 4 ctldi
10 8 ctldi 4 4 ctshl 4 3 3 ctor

0 4 ctldi 
18 8 ctldi 4 4 ctshl 4 3 3 ctor

3 ctimm dw

4 4 ctldi 0 1 ctadd 
3 ctld4
1 ctat 2 ctld4
3 2 3 ctsub
2 2 ctldi 3 3 ctshl
3 1 ctst4

3 ctzero

3a 4 ctldi 
0 8 ctldi 4 4 ctshl 4 3 3 ctor

64 4 ctldi 
8 8 ctldi 4 4 ctshl 4 3 3 ctor

6f 4 ctldi 
10 8 ctldi 4 4 ctshl 4 3 3 ctor

6e 4 ctldi 
18 8 ctldi 4 4 ctshl 4 3 3 ctor

3 ctimm dw

3 ctzero

65 4 ctldi 
0 8 ctldi 4 4 ctshl 4 3 3 ctor

a 4 ctldi 
8 8 ctldi 4 4 ctshl 4 3 3 ctor

0 4 ctldi 
10 8 ctldi 4 4 ctshl 4 3 3 ctor

0 4 ctldi 
18 8 ctldi 4 4 ctshl 4 3 3 ctor

3 ctimm dw


eof


write: w16=4, x0 fd    x1 buf    x2 len
1 for stdout

