eor 0 0
at _unused
set 1 _unused
div 1 1

set 2 1 add 2 1
set 3 2 add 3 1
set 4 3 add 4 1
set 5 4 add 5 1
set 6 5 add 6 1
set 7 6 add 7 1
set 8 7 add 8 1
set 9 8 add 9 1
set 10 9 add 10 1

set _ 0



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













