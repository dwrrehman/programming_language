lf foundation.s
set _targetarchitecture no_arch

. a program to print out all the primes at compiletime.
  testing out the compiletime simpler cteval system.
.

df n set n 32
df i set i 2
df count zero count

df loop cat loop
	df j set j 2
	df inner cat inner
		df prime ge j i prime
		df r set r i rem r j 
		df composite eq r 0 composite
		incr j do inner

	cat prime incr count ctprint i emit 8 i
	cat composite incr i lt i n loop

ctprint count 
eoi



