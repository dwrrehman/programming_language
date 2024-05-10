"eof"
 
"0" "1" "=" 

"set architecture"  
"set output format" 
"preserve executable" 
"preserve object"
"set object name" 
"set executable name"

"ct debug arguments" 
"ct debug instructions" 
"ct debug registers" 
"ct debug dictionary"

"ctdebug" 
"ctget" 
"ctput"
"ctdel"
"ctlast"
"ctarg"
"ctset"
"ctabort"
"ctprint"

"ctat"
"add"  "sub"  "addi" 
"beq"  "bne" 
"bltu" "blt" 
"bgeu" "bge" 
"jalr" "jal"
"ecall"


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

0 = "zr"
1 = "ra"
01 = "sp"

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

0 = "false"
1 = "true"

11 = "is compiletime"
001 = "is debugging"


10001111 = "skip to next function"
11111100 = "function execute"

10011100 = "argument0"
01011100 = "argument1"
11011100 = "argument2"

0000 0001 0000 0001  = "enable debugging"
0100 0001 0000 0001  = "disable debugging"

1000 0001 0000 0001  = "ct set to zero"
1100 0001 0000 0001  = "ct increment"

0100 0000 0000 0001  = "exit"

1100 0000 0000 0001  = "const"
0010 0000 0000 0001  = "set"
1010 0000 0000 0001  = "zero"
0110 0000 0000 0001  = "incr"

true is compiletime ctset

zr zr function execute addi




set enable debugging ctat 	skip to next function zr function execute beq

	0= ra zr true is debugging ctset jalr 

skip to next function ctat zr zr skip to next function addi


set disable debugging ctat 	skip to next function zr function execute beq

	0= ra zr false is debugging ctset jalr 

skip to next function ctat zr zr skip to next function addi





ct set to zero ctat 	skip to next function zr function execute beq

	argument0 ctset
	zr zr argument0 ctarg addi 
	ctdel ctdel ctdel
	0= ra zr jalr

skip to next function ctat zr zr skip to next function addi


ct increment ctat 	skip to next function zr function execute beq

	argument0 ctset
	1= argument0 ctarg argument0 ctarg addi 
	ctdel ctdel ctdel
	0= ra zr jalr 

skip to next function ctat zr zr skip to next function addi









exit ctat 	skip to next function zr function execute beq

	argument0 ctset
	false is compiletime ctset
	argument0 ctarg zr a0 addi 				ctdel ctdel ctdel
	system exit zr system call number addi 			ctdel ctdel ctdel
	ecall
	true is compiletime ctset
	0= ra zr jalr 

skip to next function ctat zr zr skip to next function addi





const ctat 	skip to next function zr function execute beq

	argument0 ctset
	argument1 ctset
	false is compiletime ctset
	argument1 ctarg zr argument0 ctarg addi 	ctdel ctdel ctdel
	true is compiletime ctset
	0= ra zr jalr 

skip to next function ctat zr zr skip to next function addi






set ctat 	skip to next function zr function execute beq

	argument0 ctset
	argument1 ctset
	false is compiletime ctset
	zr argument1 ctarg argument0 ctarg add 	  ctdel ctdel ctdel
	true is compiletime ctset
	0= ra zr jalr 

skip to next function ctat zr zr skip to next function addi





zero ctat 	skip to next function zr function execute beq

	argument0 ctset ctlast
	false is compiletime ctset
	zr zr argument0 ctarg addi 	ctdel ctdel ctdel
	true is compiletime ctset
	0= ra zr jalr 

skip to next function ctat zr zr skip to next function addi




incr ctat 	skip to next function zr function execute beq

	argument0 ctset ctlast
	false is compiletime ctset
	1= argument0 ctarg argument0 ctarg addi 	ctdel ctdel ctdel
	true is compiletime ctset
	0= ra zr jalr 

skip to next function ctat zr zr skip to next function addi






	0= ra zr jalr 				<--------- theres a problem with this code:


							in order to do a compiletime return, we need to set compiletime to true,

								but we alsoooo want to preserve the previous compiletime state. thats the problem. 


								or at the least,,,, always set the enviornment to be runtime, becuase that really should be the default, technically.    butttt, we need some sort of jalr, (ie, a return statement of sorts) which actually sets the enviornment back to runtime.  like a set-runtime-jalr or something.  i don't know how to do this yet... hm... 



	







1= zr function execute addi



false is compiletime ctset







