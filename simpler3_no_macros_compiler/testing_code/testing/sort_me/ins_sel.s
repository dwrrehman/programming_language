(1202505106.200618 working out instruction selection a bit more! dwrr)



(this would be in the standard library) 
	set hr 1 si hr 111_111
	set x0 hr or x0 0  set zr x0
	set x1 hr or x1 1
	set x2 hr or x2 01
	set x3 hr or x3 11
	set x4 hr or x4 001 
	set x5 hr or x5 101 



set zero 0 set one 1

set limit one
si limit one
si limit one
si limit one

rt i x3 

set i zero 
at loop add i one lt i limit loop
halt


