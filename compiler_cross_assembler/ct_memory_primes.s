(a prime number program 
that executes at compiletime,
and stores the first several primes
into compiletime memory!
written on 1202504104.152442 by dwrr)

set n 0000001 
set i 0 
set count 0

at loop
	set j 01
	at inner
		ge j i prime
		set r i rem r j eq r 0 composite
		add j 1 do inner
at prime
	la at firstprimes
	add at count
	add count 01
	st at i 01
	
at composite
	add i 1
	lt i n loop


at firstprimes
	(allocate some bytes here..?)


halt

