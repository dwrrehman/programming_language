(a prime number counting program 
 that executes at runtime! 
 written on 1202504104.153543 by dwrr)

rt i 0  rt j 0  rt count 0

set n 00001
set i 0
set count 0

at loop
	set j 01
	at inner
		ge j i prime
		set r i rem r j eq r 0 composite
		add j 1 do inner
at prime
	add count 1

at composite
	add i 1
	lt i n loop

halt

