lf library/constants

zero limit
add limit 5
add limit limit
mul limit limit
mul limit limit
add limit limit
add limit limit


zero i
at loop	
	zero j add j 2
	at inner
		ge prime j i
		zero r add r i rem r j
		eq composite r 0
		incr j
		eq inner 0 0 
at prime
	add i 0 env
at composite
	incr i
	lt loop i limit


