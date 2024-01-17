=========================================
          an arm 64-bit assembler
=========================================

this is an arm 64-bit macro assembler for the M1 max macbook pro (or other arm 64-bit Mach-O machines) with a syntax similar to forth, and C in some ways. this assembler is intended to be the primary programming language that most of the command-line programs that i write will be written in.

the language intentionally only includes the arm64 runtime instructions which have a unique binary encoding- that is, there are no alias instructions, as these are provided by macros if present at all. additionally, a set of custom compile-time instructions which are used to construct arbitrary data used for immediate values and branch address targets for each instruction. there is no way to write a literal or constant in this assembly language, as it is not neccessary due to the compile-time system existing. 










heres some example code from the hello_world.s program:






. examples/foundation . include

enable comment.0192840918234

202310146.180557:

	this is a comment! 

	this file is for testing out the system calls and the compiletime system in the language. 


	kinda messy right now because we are debugging strings at the moment. 



comment.0192840918234 disable



macros

	65 
		15 r1 r2 +
		2 r2 r2 *
		5 r2 r2 +
		r1 ctzero
		r1 r2 r1 +
		r1
	65 

	address
		28
	address

	last    r3 r3 +  r3 dw  	last
	next    r3 r3 +  8 r3 r3 ctshl  next

	mystring
		r3 ctzero

		'L' next
		'L'  next
		'E'  next
		'H'  last

		r3 ctzero

		'I' next
		'H'  next
		'\n'  next
		'O'  last
		
	mystring

	then string emit remove then
	newline 10 db newline

	mystring2
		enable 
			" Hello there from space! this is my cool string. yay. " 

				then newline

			" i still can't beleive this works lol... " 

				then newline
		disable
	mystring2

endmacros



1 r0 r0 movzx   	enable ;41234 	1 = stdout 						;41234 disable

address r1 adr  	enable ;41235 	string address  points to after __text section. 	;41235 disable

100 r0 r2 movzx   	enable ;41236 	4 characters in string. 				;41236 disable

4 r0 r16 movzw  	enable ;41237 	4 = write() system call. 				;41237 disable

svc

2 r0 r0 movzx
1 r0 r16 movzw
svc


address

	mystring2











foundation library file:










. examples/constants . include 
. examples/characters . include 

print ctprint print

exit cthalt exit

. ctincr .
+ ctadd +
- ctsub -
* ctmul *

enable r31 ctzero r31 ctincr enable
disable r31 ctzero disable

macros enable macros
endmacros disable endmacros

disable




constants library file:




comment.constants

	this is a file to hold commonly used constants for foundation library.

comment.constants

0 r1 ctzero r1 0

1 0 ctincr r1 1
2 1 ctincr r1 2
3 2 ctincr r1 3
4 3 ctincr r1 4
5 4 ctincr r1 5
6 5 ctincr r1 6
7 6 ctincr r1 7
8 7 ctincr r1 8
9 8 ctincr r1 9
10 9 ctincr r1 10




	...etc...










character library file:




'\n' 10 '\n'

'A' 65 'A'
'B' 66 'B'
'C' 67 'C'
'D' 68 'D'
'E' 69 'E'
'F' 70 'F'
'G' 71 'G'
'H' 72 'H'
'I' 73 'I'
'J' 74 'J'
'K' 75 'K'
'L' 76 'L'
'M' 77 'M'
'N' 78 'N'
'O' 79 'O'
'P' 80 'P'




