0 ctf
	this is a program that runs in a 
	rv32-like virtual machine, inside
	of my terminal emulator website.

	written by dwrr on 202402132.234551.
0 ctstop

1 systemexit define
01 systemread define
11 systemwrite define 

0101 '\n' define
000001 ___ define
1000001 'A' define
0100001 'B' define
1100001 'C' define
0010001 'D' define
1010001 'E' define
0110001 'F' define
1110001 'G' define
0001001 'H' define

101001 codestart define
0 0 codestart jal

	0001 stringstart define
	'C' db
	'D' db
	'A' db
	'G' db
	___ db
	'H' db
	'E' db
	'A' db

	'D' db
	'\n' db
	0 db 
	0 db
	0 db
	0 db
	0 db 
	0 db

	0101 stringcount define


codestart ctat

	0 ecall
	0 ecall
	
	stringcount 01 01 addi
	stringstart 1 1 addi

	systemwrite ecall

	systemexit ecall

eof


011 1001 1001 addi
101 0101 0101 addi
0101 1001 1011 mul
1111111111111111 01 01 addi

