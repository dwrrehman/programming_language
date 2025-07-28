(a program for the riscv virtual machine to print out prime numbers in binary!
buildin up / moving code into the standard library gradually too.
written on 1202507277.191547 by dwrr)

file library/core.s

st target r32_arch
st format hex_array
st overwrite 1

set '0' 000011
set newline 0101

eq 0 0 main

at enabledebug
	ld ra 0
	ld b ctepass 
	st ctedebug b 
	del b
	eq 0 0 ra del ra
	lt 0 0 enabledebug

at cthello 
	ld ra 0
	st ctputc 'h'
	st ctputc 'e'
	st ctputc 'l'
	st ctputc 'l'
	st ctputc 'o'
	st ctputc '!'
	st ctputc newline
	eq 0 0 ra del ra
	lt 0 0 cthello


at ctprintbinary
	ld ra 0
	set n c0
	at loop
		set bit n
		set c0 bit
		set c1 01
		eq 0 0 rem
		set bit c0
		add bit '0'
		st ctputc bit 
		del bit
		div n 01
		lt 0 n loop  
	del loop del n
	eq 0 0 ra del ra
	lt 0 0 ctprintbinary


at ctnl 
	ld ra 0
	st ctputc newline 
	eq 0 0 ra del ra
	lt 0 0 ctnl

at rem
	ld ra 0
	set n c0 set m c1
	set d n div d m
	set r d mul r m set k n sub k r
	set c0 k
	del k del r del n del m del d
	eq 0 0 ra del ra
	lt 0 0 rem

at not 
	ld ra 0
	set r -1 sub r c0
	set c0 r del r
	eq 0 0 ra del ra 
	lt 0 0 not

at si
	ld ra 0
	set value c0
	set shift_amount c1

	lt shift_amount 0000_001 valid
		eq 0 0 -1 str "error: invalid shift amount to si" 
	at valid del valid

	set i 0
	at loop
		eq i shift_amount done 
		mul value 01
		incr i 
		eq 0 0 loop 
		del loop del i del shift_amount
	at done del done

	set c0 value del value 
	eq 0 0 ra del ra
	lt 0 0 si


at sd
	ld ra 0
	set value c0	
	set shift_amount c1

	lt shift_amount 0000_001 valid
		eq 0 0 -1 str "error: invalid shift amount to si" 
	at valid del valid

	set i 0
	at loop
		eq i shift_amount done 
		div value 01
		incr i 
		eq 0 0 loop 
		del loop del i del shift_amount
	at done del done

	set c0 value del value 
	eq 0 0 ra del ra
	lt 0 0 sd

at and
	ld ra 0 set a c0 set b c1
	set c0 1 set c1 111_111 si set msb c0
	zero i zero c
	at loop
		mul c 01
		lt a msb s 
		lt b msb s 
			incr c 
		at s del s
		mul a 01 mul b 01 
		incr i lt i 0000_001 loop del loop del i
	del a del b del msb
	set c0 c del c
	eq 0 0 ra del ra 
	lt 0 0 and

at or
	ld ra 0 set a c0 set b c1
	set c0 1 set c1 111_111 si set msb c0
	zero i zero c
	at loop
		add c c
		lt a msb try_b
			incr c 
			eq 0 0 advance
		at try_b 
		lt b msb advance
			incr c
		at advance 
		add a a 
		add b b
		incr i lt i 0000_001 loop 

	del a del b del msb 
	del advance del try_b 
	del loop del i
	set c0 c del c
	eq 0 0 ra del ra 
	lt 0 0 or


at la
	ld ra 0
	at pc 
	set offset c1 
	sub offset pc 
	del pc
	ru r_auipc c0 0

(todo: do the bitwise operations to extract 
the top 20 bits of the immediate too!!)

	ri r_imm r_add c0 c0 offset 
	del offset
	eq 0 0 ra del ra
	lt 0 0 la

at exit
	ld ra 0
	ri r_imm r_add r_number 0 r_exit
	ri r_imm r_add r_arg0 0 c0
	ri r_ecall 0 0 0 0
	eq 0 0 ra del ra 
	lt 0 0 exit


at readchar
	ld ra 0
	set out c0
	set copy r_arg3
	ri r_imm r_add r_number 0 r_read
	ri r_imm r_add r_arg0 0 stdin
	set c0 r_arg1 set c1 buffer la
	ri r_imm r_add copy r_arg1 0 
	ri r_imm r_add r_arg2 0 1
	ri r_ecall 0 0 0 0
	ri r_load r_lbu out copy 0
	eq 0 0 ra del ra del out del copy
	lt 0 0 readchar

at writestring
	ld ra 0
	set string c0
	set length c1
	ri r_imm r_add r_number 0 r_write
	ri r_imm r_add r_arg0 0 stdout
	set c0 r_arg1 set c1 string la
	ri r_imm r_add r_arg2 0 length
	ri r_ecall 0 0 0 0
	eq 0 0 ra del ra 
	del string del length
	lt 0 0 writestring

at getstringlength
	ld ra 0
	set begin c0
	at here set c0 here sub c0 begin
	del here del begin
	eq 0 0 ra del ra
	lt 0 0 getstringlength

at printbinary
	ld ra 0
	set input c0
	set bit 	r_number
	set data 	r_arg0
	set begin 	r_arg1
	set p 		r_arg2
	
	ri r_imm r_add data input 0
	set c0 begin set c1 buffer la
	ri r_imm r_add p begin 0
	at loop
		ri r_imm r_and bit data 1
		ri r_imm r_add bit bit '0'
		rs r_store r_sb p bit 0
		ri r_imm r_add p p 1
		ri r_imm r_srl data data 1
		rb r_branch r_bne data 0 loop

	ri r_imm r_add bit 0 newline
	rs r_store r_sb p bit 0
	ri r_imm r_add p p 1

	ri r_imm r_add r_number 0 r_write
	ri r_imm r_add r_arg0 0 stdout
	rr r_reg r_add r_arg2 p begin r_signed
	ri r_ecall 0 0 0 0

	del loop 
	del p 
	del bit 
	del data 
	del begin
	del input

	eq 0 0 ra del ra
	lt 0 0 printbinary


at main 

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

set c0 01 exit

del i del j del limit del count del r del loop del inner del composite 

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


