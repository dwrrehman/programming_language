"include" 

"library/foundation.s" include

"
	this is a program to test the ct/rt language, and generating arm64 machine code. 
	everything is working really well so far! we got the first working rt program yayyyy
	on 202403251.044244.  
	dwrr
"

set compiletime 
"assembler: loaded foundation successfully..." ctprint
set runtime


arm 64 			set architecture
macho executable 	set output format
"object.o"		set object name
"program.out"		set executable name
0 = preserve object
0 = preserve executable

set compiletime

011 = "sum"
4 zr sum addi 
5 sum sum addi

zr zr 101 = "a" addi
5 zr 011 = "count" addi


0000001 = "my loop label"
my loop label ctat
	a ctdebug ctincr
	my loop label count a bltu


1= zr 111 = "should skip" addi
1= zr 001 = "one" addi

1000001 = "skip over getchar"

skip over getchar   should skip  one bltu

	set runtime 

	standard in= 	zr 	system read fd 		addi
	sp= 		zr 	system read buffer 	addi
	1= 		zr 	system read length 	addi
	system read 	zr 	system call number 	addi 
	ecall

skip over getchar ctat 


set runtime 
standard in= 	zr 	system read fd 		addi
sp= 		zr 	system read buffer 	addi
1= 		zr 	system read length 	addi
system read 	zr 	system call number 	addi 
ecall

11111= 		zr 	system exit code 	addi
system exit 	zr 	system call number 	addi 
ecall










enable debug 
eof










0= zr 001= addi
1= zr 101= addi
"runtime skip label" 001= 101= beq





runtime skip label ctat













how to comment out code currently.. its not the best:

	set compiletime 1111111= zr jal

		set compiletime 
		"ending assembly program!" ctprint
		ctabort

	1111111= ctat










