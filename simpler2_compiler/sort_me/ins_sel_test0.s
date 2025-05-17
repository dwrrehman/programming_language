(testing out instruction selection for rv32. written on 1202505036.190953 by dwrr)


(this would be in the standard library) 
	set hr 1 si hr 111_111
	set x0 hr or x0 0  set zr x0
	set x1 hr or x1 1
	set x2 hr or x2 01
	set x3 hr or x3 11
	set x4 hr or x4 001 
	set x5 hr or x5 101 

rt a x5  rt x x1  rt y x2  rt d x3

r5_i 1100100 x 0 0 101
set x 101

add x 011
set d x

set y 11
set y d
add y x





