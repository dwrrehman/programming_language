lf library/foundation

set limit 5
sl limit 1
mul limit limit
mul limit limit

zero i
at loop
	set j 2
	at inner
		ge prime j i
		set r i rem r j
		eq composite r 0
		incr j
		eq inner 0 0 

at prime
	add i 0 env
at composite
	incr i
	lt loop i limit


