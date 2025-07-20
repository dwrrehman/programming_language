file library/core.s
file library/ascii.s

st compiler_target no_arch
st compiler_format no_output

set c0 c0
set c1 c1
set c2 c2

eq 0 0 skip_macros

at hello 
	ld ra 0
	st compiler_putc 'H'
	st compiler_putc 'E'
	st compiler_putc 'L'
	st compiler_putc 'L'
	st compiler_putc 'O'
	st compiler_putc '!'
	st compiler_putc newline
	eq 0 0 ra del ra


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


at nl 
	ld ra 0
	st compiler_putc newline 
	eq 0 0 ra del ra

at mod
	ld ra 0
	set n c0 set m c1
	set d n div d m
	set r d mul r m set k n sub k r
	set c0 k
	del k del r del n del m del d
	eq 0 0 ra del ra


at not 
	ld ra 0
	set r -1 sub r c0
	set c0 r del r
	eq 0 0 ra del ra 
	lt 0 0 not




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


eq 0 0 hello


set a 011010110101
set b 100111100100

set c0 a eq 0 0 printbinary eq 0 0 nl
set c0 b eq 0 0 printbinary eq 0 0 nl



set c0 a 

not printbinary nl

set c0 b

not printbinary nl



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



















