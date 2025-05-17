zero 0 
zero 1 incr 1
zero 2 incr 2 incr 2
zero 3 incr 3 incr 3 incr 3
zero 4 incr 4 incr 4 incr 4 incr 4

zero i
at loop
	incr i
	lt i 3 loop

set r i rem r 2
eq r 0 skip
	set i 0
at skip
