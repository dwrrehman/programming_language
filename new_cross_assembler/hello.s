"include"
"library/foundation.s" include

set runtime 
arm 64 			set architecture
macho executable 	set output format
"object.o"		set object name		false preserve object
"program.out"		set executable name	false preserve executable

0 = zr 001 = "n" addi
0 = zr 101 = "i" addi
0 = zr 011 = "sum" addi
0 = zr 111 = "exit code" addi

0000001 = "loop" 
1000001 = "set exit code"

loop ctat
	set exit code n i bgeu
	"i sum sum add"
	1= sum sum addi
	1= i i addi
loop n i bltu

set exit code ctat zr sum exit code add

standard in= zr system read fd addi
zr zr system read buffer addi
1= zr system read length addi
system read zr system call number addi 
ecall

exit code zr system exit code add
system exit zr system call number addi 
ecall

debug instructions
enable debug
eof
