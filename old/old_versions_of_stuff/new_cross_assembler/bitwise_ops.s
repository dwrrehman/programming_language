file library/core.s
file library/ascii.s

st compiler_target no_arch
st compiler_format no_output

zero c0
zero c1
zero c2

eq 0 0 skip_macros

at hello 
	ld ra 0
	st compiler_putc 'h'
	st compiler_putc 'e'
	st compiler_putc 'l'
	st compiler_putc 'l'
	st compiler_putc 'o'
	st compiler_putc '!'
	st compiler_putc newline
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
		st compiler_putc bit 
		del bit
		div n 01
		lt 0 n loop  
	del loop del n
	eq 0 0 ra del ra
	lt 0 0 printbinary


at nl 
	ld ra 0
	st compiler_putc newline 
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
	eq 0 0 ra del ra lt 0 0 and




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
	eq 0 0 ra del ra lt 0 0 or






at skip_macros del skip_macros

set c0 0011 set c1 101 eq 0 0 mod eq 0 0 printbinary eq 0 0 nl 

set maxcount 0000_01

zero i 
at loop
	set j 01
	at inner
		eq i j prime
		lt i j prime
		set c0 i set c1 j 
		eq 0 0 mod eq c0 0 composite
		incr j eq 0 0 inner
at prime
	set c0 i eq 0 0 printbinary eq 0 0 nl
at composite
	incr i lt i maxcount loop del loop del i del maxcount



(got comments and macro call syntax working! code is a lot more readable now lol) 


hello


set a 011010110101
set b 100111100100

set c0 a  printbinary nl
set c0 b  printbinary nl

zero i
at loop
	set c0 a set c1 i si set x c0 printbinary nl
	incr i
	lt i 00001 loop del loop del i


set c0 x printbinary nl

zero i
at loop
	set c0 x set c1 i sd printbinary nl
	incr i
	lt i 11111 loop del loop del i



hello


set c0 a printbinary nl
set c0 b printbinary nl

set c0 a
set c1 b
and printbinary nl

set c0 a
set c1 b
or printbinary nl


(  not (not a or not b)  ==  a and b   )
set c0 a not
set c0 b not set c1 c0
or not printbinary nl


hello



eoi 

1202507196.230517
revision of the first program we wrote for the ct system, 

	but now with this new revision to the language,
	which now only contains the following    18   ct instructions:


			zero incr set 
			add sub mul div 
			lt eq at ld st 
			emit sect file 
			del str eoi 

 




program header:
 this is the first program to test out the parser and see 
 if this langauges syntax can be simplified at all for the 
 compiletime instructions yay 1202507174.205406 by dwrr


note: comments will usually be now found at the end of the program text. 
ie, we are primarily going to be using eoi for comments, and the trash. 
if you want comments in line with the actual code, we'll use 



















