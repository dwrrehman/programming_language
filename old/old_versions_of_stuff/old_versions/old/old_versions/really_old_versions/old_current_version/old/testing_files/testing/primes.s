lf library/foundation

set limit 5
si limit 1
mul limit limit
mul limit limit

zero i
at loop
	set j 2
	at inner
		ge j i prime 
		set r i rem r j
		eq r 0 composite
		incr j
		eq 0 0 inner

at prime
	debug_number i
at composite
	incr i
	lt i limit loop


