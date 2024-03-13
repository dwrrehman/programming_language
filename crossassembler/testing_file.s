library/targets define include
library/ascii define include
library/ecalls define include
arm64 settarget
machoexecutable setoutputformat
object.o define setobjectname 
program.out define setexecutablename
false preserveexistingexecutable
false preserveexistingobject






0 ctf

	01 11 001 addi
	101 11 001 add

	0 0 0 add

	0 0 1 addi

	0001 1 1 addi

	111 0001 1001 addw

	001 01 1 addiw


0 ctstop

1111 0 cc.a0 addi
1 0 cc.a7 addi
ecall




true enabledebugoutput
eof 












