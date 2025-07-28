(a program for the riscv virtual machine to print out prime numbers in binary!
buildin up / moving code into the standard library gradually too.
written on 1202507277.191547 by dwrr)

file library/core.s

set '0' 000011
set newline 0101


eq 0 0 main


at hello 
	ld ra 0
	st ctputc 'h'
	st ctputc 'e'
	st ctputc 'l'
	st ctputc 'l'
	st ctputc 'o'
	st ctputc '!'
	st ctputc newline
	eq 0 0 ra del ra
	lt 0 0 hello


at printbinary
	ld ra 0
	set n c0
	at loop
		set bit n
		set c0 bit
		set c1 01
		eq 0 0 mod
		set bit c0
		add bit '0'
		st ctputc bit 
		del bit
		div n 01
		lt 0 n loop  
	del loop del n
	eq 0 0 ra del ra
	lt 0 0 printbinary


at nl 
	ld ra 0
	st ctputc newline 
	eq 0 0 ra del ra
	lt 0 0 nl

at mod
	ld ra 0
	set n c0 set m c1
	set d n div d m
	set r d mul r m set k n sub k r
	set c0 k
	del k del r del n del m del d
	eq 0 0 ra del ra
	lt 0 0 mod


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
		 (todo: do the bitwise operations to extract the top 20 bits of the immediate too!!)
	ri r_imm r_add c0 c0 offset 
	del offset
	eq 0 0 ra del ra
	lt 0 0 la


at main 

st target r32_arch
st format hex_array
st overwrite 1



set c0 101 set c1 0000000001 si






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














ri r_imm r_add r_number 0 r_exit
ri r_imm r_add r_arg0 0 0011
ri r_ecall 0 0 0 0

at string
str "hello
"







