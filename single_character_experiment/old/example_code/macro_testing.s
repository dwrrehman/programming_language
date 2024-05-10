"include"
"library/foundation.s" include
"library/arm.s" include

" no runtime set architecture "

set compiletime

10001111 = "skip"
11111100 = "e"
10011100 = "this"

10011111 = "please print the message"
0000 0000 0000 0001  = "set to zero"       "<---- this defines a call on use macro!  r >= 32768 "
1000 0000 0000 0001  = "increment"
0100 0000 0000 0001  = "set to zero at runtime"




zr zr e addi


please print the message ctat skip zr e beq
	"hello world!" ctprint 
	0= ra zr jalr
skip ctat zr zr skip addi


set to zero ctat skip zr e beq
	this cttop
	zr zr this ctarg addi 
	ctdel ctdel ctdel
	0= ra zr jalr
skip ctat zr zr skip addi


increment ctat skip zr e beq
	this cttop
	1= this ctarg this ctarg addi 
	ctdel ctdel ctdel
	0= ra zr jalr 
skip ctat zr zr skip addi


set to zero at runtime ctat skip zr e beq
	this cttop
	zr zr this ctarg set runtime addi set compiletime
	ctdel ctdel ctdel
	0= ra zr jalr 
skip ctat zr zr skip addi


1= zr e addi






1000001 = "A"
10001 = "my variable"
1001000001 = "i"
1010000001 = "n"

A zr my variable addi
101= zr n addi
0= zr i addi

"calling the function once..." ctprint
please print the message 	ra jal

"calling the function again: " ctprint
please print the message 	ra jal

n set to zero increment increment increment increment ctdebug
a0 set to zero at runtime

my variable ctput

1= i i addi


















eof 



"	if we ever need it, we can enable     enable debug    

		for a section of code.

"




