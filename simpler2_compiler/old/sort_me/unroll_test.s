(testing out unrolling of loops in the compiler, currently buggy i think  1202504196.225351 dwrr)


rt x 0
set x 0
set i 0 
at l 
	add x 101 
	add i 1 
	lt i 11 l

(this shouldd be unrolled, but lets see if it will be!)




