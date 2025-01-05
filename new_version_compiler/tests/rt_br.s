zero 0
set 1 0 incr 1
set 2 1 incr 2
set 3 2 incr 3
set 4 3 incr 4

zero i
set x 0
sc 1 x 1 1 2 2 2

at loop
	incr i
	lt i x loop
