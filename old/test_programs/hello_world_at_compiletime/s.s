file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s

eq 0 0 main

at my_print
	ld ra 0
	set i c0
	set c0 ra function_begin 

	str "the value is: '" ctprintstring
	set n i add n 0101
	set c0 n ctprintbinary 
	del n
	str "', and we are on iteration " ctprintstring
	set c0 i ctprintbinary 
	str " currently." ctprintstring ctnl

	del i
	function_end
	eq 0 0 ra del ra
	lt 0 0 my_print

at main

zero i
at l
	set c0 i my_print
	incr i 
	lt i 0101 l

eoi

a program to print out the numbers 10 trough 19 in binary, 
written on 1202508203.032054 by dwrr

