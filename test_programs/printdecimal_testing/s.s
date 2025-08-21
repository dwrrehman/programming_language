file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s

eq 0 0 main

at exponentiate
	ld ra 0
	set base c0 
	set power c1
	set c0 ra function_begin

	set c0 1
	zero i
	at loop
		mul c0 base incr i
		lt i power loop del loop

	del base del power
	function_end
	eq 0 0 ra del ra
	lt 0 0 exponentiate


at ctprintdecimal
	ld ra 0
	set n c0
	set c0 ra function_begin

	set array 0000_0000_1
	set count 0101

	set p array
	zero i at l	
		set data i
		st p data incr p
		incr i lt i count l del l

	set p array add p count sub p 1
	zero i at l 
		ld b p sub p 1 add b '0'
		set c0 b ctputchar del b
		incr i lt i count l del l

	ctnl

	del i del p
	del count
	del array

	del n
	function_end
	eq 0 0 ra del ra
	lt 0 0 ctprintdecimal



at main

(zero i
at l
	str "the value is: '" ctprintstring
	set c0 i ctprintbinary 
	str "', and we are on iteration " ctprintstring
	set c0 i ctprintbinary 
	str " currently." ctprintstring ctnl
	incr i lt i 0101 l


set operation_count 101

set 0sp_hole_count 00101
sub 0sp_hole_count 101
sub 0sp_hole_count 1

set c0 operation_count 
set c1 0sp_hole_count 
exponentiate 
set 0sp_size c0

set product 0sp_size

str "the size of 0-space is: '" ctprintstring 
set c0 product ctprintbinary ctnl
)



set c0 0011 ctprintdecimal





eoi

a program to test out using my editor for writing actual programs not in the assemblers directory.
this test is an read-only editor like program!
written on 1202508203.021226


























	
