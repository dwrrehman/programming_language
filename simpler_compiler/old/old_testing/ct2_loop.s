(this is a simple loop from 0 to 9 at compiletime    1202504104.151607 dwrr)

rt N 0 set N 001
set count 0101
set i 0
at loop
	ge i count done
	add N 1
	eq N 01 thingy
	do skip
at thingy
	do outofline
at resume
	set N 0
	do l
	div N 11
at l
	do skip
	div N 11
at skip
	add i 1
	do loop
halt


at outofline
	si i 1
	sd i 1 
	do resume