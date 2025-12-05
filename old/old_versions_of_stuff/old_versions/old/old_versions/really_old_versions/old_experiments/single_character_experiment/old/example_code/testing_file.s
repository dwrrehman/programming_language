library/targets define include
library/ascii define include
library/ecalls define include
arm64 settarget
machoexecutable setoutputformat
object.o define setobjectname 
program.out define setexecutablename
false preserveexistingexecutable
false preserveexistingobject

0 addi
101 001 11 or
101 001 11 sub
101 001 11 subw

1 0 11 jalr

101 001 11 addi
101 001 11 add
101 001 11 addiw
101 001 11 addw
0 0 0 add
0 0 1 addi
1111 0 1 addi
0001 1 1 addi


1111 	0 	cc.a0 	addi
1 	0 	cc.a7 	addi
			ecall

true enabledebugoutput

eof 












