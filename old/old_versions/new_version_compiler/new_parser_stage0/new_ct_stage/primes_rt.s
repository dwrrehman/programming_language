lf constants.s

set max 100
mul max 5

rt count nat 
set count 0

rt i nat set i 2

at loop
	rt j nat set j 2
	at inner
		ge j i prime
		set r i div r j
		mul r j sub r i
		eq r 0 composite
		add j 1
		do inner

	at prime add count 1	
	at composite
	add i 1
	lt i max loop


sc system_exit count _ _ _ _ _

eoi

a runtime executed prime number program   
to exitcode with the number of 
primes less than a given number
written on 1202502252.161655 by dwrr













