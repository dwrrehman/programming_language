lf constants.s

set n 10 
set i 0 
set sum 0
at loop
	add sum i
	add i 1
	lt i n loop

rt x 8
set x sum
sc 0 sum _ _ _ _ _ 


eoi

written on 1202502123.130918 
by dwrr

this is a test of the language ct system which doesnt  have incr or zero as those were replaced by computational immediates used with add and set. 
the computational immediates  1 and 0 are obtained using:

	eor x x
	at l
	set y l 
	div y y

x will be 0 and y will be 1. 













