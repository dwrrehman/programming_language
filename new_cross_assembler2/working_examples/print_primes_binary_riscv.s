(a program for the riscv virtual machine to print out prime numbers in binary!
buildin up / moving code into the standard library gradually too.
written on 1202507277.191547 by dwrr)

file library/core.s
file library/useful.s

riscv_hex

set c 1

set c0 longstring set c1 longstring.length writestring
set c0 c readchar
set c0 string2 set c1 string2.length writestring
set c0 buffer set c1 1 writestring
set c0 string3 set c1 string3.length writestring
set c0 c printbinary

set myvariable c
set myconstant 0001_01
ri r_imm r_add myvariable 0 myconstant
set c0 myvariable printbinary
del myvariable del myconstant del c

(1202507277.233929 we are going to print binary primes now!! YAYY)

set i 1
set j 01
set limit 11
set r 001
set count 101
ri r_imm r_add limit 0 0000_0000_1
ri r_imm r_add i 0 0
ri r_imm r_add count 0 0
at loop
	ri r_imm r_add j 0 01
	at inner
		rb r_branch r_bgeu j i prime
		rr r_remu_op1 r_remu_op2 r i j r_remu_op3
		rb r_branch r_beq r 0 composite
		ri r_imm r_add j j 1
		rj r_jal 0 inner
	at prime
		set c0 i printbinary
		ri r_imm r_add count count 1
	at composite
		ri r_imm r_add i i 1	
		rb r_branch r_bltu i limit loop

	
set c0 string4 set c1 string4.length writestring
set c0 count printbinary

del i del j del limit del count del r del loop del inner del composite



set c0 01 exit


at buffer 
emit 0001 0
emit 0001 0
emit 0001 0
emit 0001 0
emit 0001 0

at longstring 
str 
"this my really long and interesting string!
i am going to print out a lot, all at runtime! 
lets see how it goes lol. i think it should go
pretty well, considering that we have strings
setup in a way that makes things easy lol.

hopefully this goes well! :)
"
(str "hi lol")
set c0 longstring getstringlength
set longstring.length c0

at string2 str 
"the user pressed the character '"
set c0 string2 getstringlength 
set string2.length c0

at string3 str 
"', and they pressed exactly one character!
" set c0 string3 getstringlength
set string3.length c0

at string4 str 
"---> there were exactly this many primes less than the limit: " 
set c0 string4 getstringlength
set string4.length c0


eoi 

a program to test out how the risc-v backend works with the rv website vm, and basically making to get input and output working,
to be able to have the user give a number to print out prime binary numbers less than that number lol. should be cool. 
1202507277.222702



yayyy, got printbinary working on 1202507277.233941 
































(this will cause a compiletime error!
set c0 101 set c1 0000000001 si)








(
set limit 1
set i 01

ri r_imm r_add i 0 0
ri r_imm r_add limit 0 101

at loop

rb r_branch r_equal i limit done

ri r_imm r_add r_number 0 r_write
ri r_imm r_add r_arg0 0 1

set c0 r_arg1 
set c1 string  la

ri r_imm r_add r_arg2 0 011
ri r_ecall 0 0 0 0

ri r_imm r_add i i 1 del i
rb r_branch r_equal 0 0 loop del loop

at done del done
)


