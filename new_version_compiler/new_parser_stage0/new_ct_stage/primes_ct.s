lf constants.s

set max 100
mul max 5

set count 0
set i 2

at loop	
	set j 2
	at inner
		ge j i prime
		set r i div r j
		mul r j sub r i
		eq r 0 composite
		add j 1
		eq 0 0 inner

	at prime 
		add count 1	
		sc 9 i _ _ _ _ _
	at composite


	add i 1
	lt i max loop


rt code 64
set code count
sc system_exit code _ _ _ _ _
eoi

a prime number program   
to exitcode with the number of 
primes less than a given number
written on 1202502204.114717 by dwrr


