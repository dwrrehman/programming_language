file library/core.s
file library/ascii.s

st compiler_target no_arch
st compiler_format no_output

set -2 -1 add -2 -1

set c0 c0
set c1 c1
set c2 c2

do skip_macros

at hello 
	ld ra 0
	st compiler_putc 'H'
	st compiler_putc 'E'
	st compiler_putc 'L'
	st compiler_putc 'L'
	st compiler_putc 'O'
	st compiler_putc '!'
	st compiler_putc newline
	do ra del ra


at printbinary
	ld ra 0
	set n c0
	at loop
		set bit n 
		nor bit 0 
		nor bit -2 
		add bit '0'
		st compiler_putc bit 
		del bit
		sd n 1 
		lt 0 n loop  
	del loop del n
	do ra del ra


at nl 
	ld ra 0
	st compiler_putc newline 
	do ra del ra

at mod
	ld ra 0
	set n c0 set m c1
	set d n div d m
	set r d mul r m set k n sub k r
	set c0 k
	del k del r del n del m del d
	do ra del ra

at skip_macros del skip_macros

set c0 0011 set c1 101 do mod do printbinary do nl 
do hello

set maxcount 0000_0000_01

do s st compiler_should_debug true at s del s

zero i 
at loop
	set j 01
	at inner
		eq i j prime
		lt i j prime
		set c0 i set c1 j 
		do mod eq c0 0 composite
		incr j do inner
at prime
	set c0 i do printbinary do nl
at composite
	incr i lt i maxcount loop del loop del i

eoi 




program header:
 this is the first program to test out the parser and see 
 if this langauges syntax can be simplified at all for the 
 compiletime instructions yay 1202507174.205406 by dwrr


note: comments will usually be now found at the end of the program text. 
ie, we are primarily going to be using eoi for comments, and the trash. 
if you want comments in line with the actual code, we'll use 



















