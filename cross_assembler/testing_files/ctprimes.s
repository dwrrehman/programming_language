.
	a file which tests printing out
	prime numbers at compiletime.
	written on 1202502285.012928 dwrr
.

lf foundation.s

set n 10000    . set this to some number! . 
set i 2 
set count 0

at loop	
	set j 2	
	at inner
		ge j i prime
		set r i rem r j eq r 0 composite
		incr j do inner

	at prime print i incr count
	at composite incr i lt i n loop

print count

eoi





