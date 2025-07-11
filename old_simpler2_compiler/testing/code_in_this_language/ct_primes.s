(a prime number counting program 

	 that executes   at compile-time!!!

 written on 1202504104.153543 by dwrr)

file library/foundation.s 
file library/ascii.s

string " is a prime!"
string " ----> there were "
string " primes less than the total amount!"

constant prime 
constant composite 
constant n set n 0000000000000000001  
constant i set i 0
constant count set count 0

constant loop at loop
	constant j set j 01
	constant inner at inner
		ge j i prime
		constant r set r i rem r j eq r 0 composite
		add j 1 do inner

at prime
	compiler ctsc_printdec i
	compiler ctsc_print 0
	compiler ctsc_putchar char_newline
	add count 1

at composite
	add i 1
	lt i n loop


compiler ctsc_print 1
compiler ctsc_printdec count
compiler ctsc_print 01
compiler ctsc_putchar char_newline


compiler ctsc_exit 010101




























