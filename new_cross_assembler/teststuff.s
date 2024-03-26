"
	this is a program to test the ct/rt language, and generating arm64 machine code. 
	everything is working really well so far! we got the first working rt program yayyyy
	on 202403251.044244.  
	dwrr
"

"eof" "0" "1" "=" 
"debug arguments" "debug instructions" "debug registers" "debug dictionary"
"setarchitecture"  "setoutputformat" 
"preserveexecutable" "preserveobject"
"set object name" "set executable name"
"setcompiletime" "setruntime" 
"ctdebug" "set debug" "ctget" "ctput" "ctabort"

"ctat"
"ctincr"
"ctzero"

"add" 
"sub" 
"addi" 
"blt"
"bltu"
"jalr"
"jal"
"ecall"



0 = set debug




0 = "no runtime"
1 = "riscv 32"
01 = "riscv 64"
11 = "arm 32"
001 = "arm 64"

0 = "print binary"
1 = "elf object"
01 = "elf executable"
11 = "macho object"
001 = "macho executable"

0101 = "a0"
1101 = "a1"
0011 = "a2"
1011 = "a3"
0111 = "a4"
1111 = "a5"
00001 = "a6"
10001 = "a7" "system call number"

1 = "system exit"     a0 "system exit code"
01 = "system fork"
11 = "system read"    a0 "system read fd"     a1 "system read buffer"      a2 "system read length"
001 = "system write"
101 = "system open"
011 = "system close"

0 = "standard in"
1 = "standard out"

0 = "zr"
1 = "ra"
01 = "sp"

01 = "2"
11 = "3"
001 = "4"
101 = "5"
011 = "6"
111 = "7"
0001 = "8"
1001 = "9"

011 = "sum"
set compiletime
4 zr sum addi 
5 sum sum addi
ct debug

arm 64 			set architecture
macho executable 	set output format
"object.o"		set object name
"program.out"		set executable name
0 = preserve object
0 = preserve executable






001 = "one"
101 = "a"
011 = "count"

0000001 = "my loop label" 	ctzero
1000001 = "skip to exit call" 	ctzero

set compiletime

zr zr a addi
2 zr count addi

"
my loop label ctat
	debug registers

	zr ctget

	a ctincr
	my loop label count a bltu
"



1 = set debug

one ctzero ctincr ctincr

debug registers zr ctget

skip to exit call count one bltu

debug registers zr ctget


set runtime 
standard in= 	zr 	system read fd 		addi
sp= 		zr 	system read buffer 	addi
1= 		zr 	system read length 	addi
system read 	zr 	system call number 	addi 
ecall

skip to exit call ctat 

set runtime 
11111= 		zr 	system exit code 	addi
system exit 	zr 	system call number 	addi 
ecall


eof

