(
a prime number program to compute prime numbers. 
uses a debug system call number to print out the results.
written by dwrr on 1202504067.015642
)
ct set debug 0

ct set count 0
ct set n 0101    (this is a compile-time set)
ct set i 0 

at loop
	ct set j 01
	at inner
		ge j i prime
		ct set r i rem r j
		eq r 0 composite
		add j 1
		ct do inner
at prime

	ct sc 0 i   0 0 0 0 0 
	add count 1

at composite
	add i 1
	lt i n loop

	
set debug 1
ct set finaldata count
si finaldata 1
ct sc 0 finaldata   0 0 0 0 0 
sd finaldata 1
ct sc 0 finaldata   0 0 0 0 0 
halt

ct st 101 1111000101100110 01
ct ld my_value 101 01
ct sc 01  0 0 0 0 0 0 
set debug 0
ct halt
